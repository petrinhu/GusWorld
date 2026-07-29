// SPDX-License-Identifier: GPL-3.0-or-later
// gus/platform/src/render2d/render2d_glintfx.cpp
//
// Implementacao do Render2dGlintfx (3a impl. de IRenderer, delega ao glintfx::Draw2d
// v0.23.0). Ver header pro contrato completo (camera, texto, headless). Travado por
// platform/tests/render2d_glintfx_test.cpp (TEST-FIRST, equivalencia de camera +
// caminho headless) + smoke visual do app (--battle com contexto GL real, quando
// GUSWORLD_RENDER2D_BACKEND=glintfx).
//
// TEXTO (pipeline GL propria, NAO Draw2d::draw_text - ver header, secao TEXTO): reusa a
// MESMA receita do Render2dGl3 (compilar 1 programa de quad texturizado, VAO/VBO/EBO
// proprios, atlas de glifo grayscale->RGBA premultiplied, projecao via
// viewport_transform::build_quad_screen) - duplicada aqui de proposito (NAO extraida pro
// Render2dGl3 nesta fatia: risco de regressao no backend em producao, fora do escopo do
// D2D-2-SENTINELA). O premultiplied-alpha blend (GL_ONE, GL_ONE_MINUS_SRC_ALPHA) e o
// MESMO que o Draw2d usa nas proprias texturas (D7 do header deles), entao os dois
// convivem sem estado GL conflitante (D9).

#include "gus/platform/render2d/render2d_glintfx.hpp"

#include <cstddef>  // offsetof
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include <glintfx/draw2d.hpp>

#include "gus/core/asset_paths.hpp"                // nomes de arquivo de fonte
#include "gus/platform/render2d/font_atlas.hpp"    // bake_font_atlas/FontAtlas/resolve_font_path
#include "gus/platform/render2d/text_metrics.hpp"  // glyph_advance + decode_utf8
#include "gus/platform/render2d/viewport_transform.hpp"

// glad (GL 3.3 core) - so o header; a impl vem do gl3_loader.cpp (mesma TU-set que o
// Render2dGl3 ja compartilha, ADR-010 F3). Precisamos das chamadas GL cruas so pra texto
// (ver o comentario de cabecalho deste arquivo).
#include "RmlUi_Include_GL3.h"

namespace gus::platform::render2d {

namespace {

constexpr float k1_255 = 1.0f / 255.0f;

// Vertice do quad de TEXTO: posicao em clip-space + uv + cor (premultiplied alpha) -
// MESMO layout do Vtx do Render2dGl3 (duplicado de proposito, ver cabecalho do arquivo).
struct TextVtx {
    float x, y;   // clip-space [-1,1]
    float u, v;   // uv [0,1]
    float r, g, b, a;
};

const char* kTextVertSrc = R"GLSL(#version 330 core
layout(location=0) in vec2 a_pos;
layout(location=1) in vec2 a_uv;
layout(location=2) in vec4 a_col;
out vec2 v_uv;
out vec4 v_col;
void main(){ v_uv = a_uv; v_col = a_col; gl_Position = vec4(a_pos, 0.0, 1.0); }
)GLSL";

const char* kTextFragSrc = R"GLSL(#version 330 core
in vec2 v_uv;
in vec4 v_col;
out vec4 frag;
uniform sampler2D u_tex;
void main(){ frag = texture(u_tex, v_uv) * v_col; }
)GLSL";

}  // namespace

// ---------------------------------------------------------------------------
// Impl: Draw2d (5 primitivas de mundo) + pipeline GL propria de texto. So existe quando
// gl_active. Em headless impl_ == nullptr (espelha o Render2dGl3).
// ---------------------------------------------------------------------------
struct Render2dGlintfx::Impl {
    glintfx::Draw2d draw2d;

    GLuint text_program = 0;
    GLuint text_vao = 0;
    GLuint text_vbo = 0;
    GLuint text_ebo = 0;
    GLint text_loc_tex = -1;

    struct FontFace {
        FontAtlas atlas;
        GLuint texture = 0;
        bool tried = false;
    };
    FontFace font_regular;
    FontFace font_bold;

    // Texturas de sprite: indexadas por TextureId (1-based; 0 invalido), espelhando o
    // vetor by-id do Render2dGl3 - so que aqui o "handle" e o proprio glintfx::Texture2d
    // (copiavel, trivialmente pequeno - Draw2d e quem possui o recurso GL de verdade).
    std::vector<glintfx::Texture2d> textures{glintfx::Texture2d{}};  // slot 0 = invalido
    std::unordered_map<std::string, TextureId> by_path;

    bool init_text_gl() {
        auto compile = [](GLenum type, const char* src) -> GLuint {
            GLuint s = glCreateShader(type);
            glShaderSource(s, 1, &src, nullptr);
            glCompileShader(s);
            GLint status = 0;
            glGetShaderiv(s, GL_COMPILE_STATUS, &status);
            if (!status) {
                glDeleteShader(s);
                return 0;
            }
            return s;
        };
        GLuint vs = compile(GL_VERTEX_SHADER, kTextVertSrc);
        GLuint fs = compile(GL_FRAGMENT_SHADER, kTextFragSrc);
        if (!vs || !fs) {
            if (vs) glDeleteShader(vs);
            if (fs) glDeleteShader(fs);
            return false;
        }
        text_program = glCreateProgram();
        glAttachShader(text_program, vs);
        glAttachShader(text_program, fs);
        glLinkProgram(text_program);
        glDeleteShader(vs);
        glDeleteShader(fs);
        GLint linked = 0;
        glGetProgramiv(text_program, GL_LINK_STATUS, &linked);
        if (!linked) {
            glDeleteProgram(text_program);
            text_program = 0;
            return false;
        }
        text_loc_tex = glGetUniformLocation(text_program, "u_tex");

        glGenVertexArrays(1, &text_vao);
        glGenBuffers(1, &text_vbo);
        glGenBuffers(1, &text_ebo);
        glBindVertexArray(text_vao);
        glBindBuffer(GL_ARRAY_BUFFER, text_vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, text_ebo);
        const GLsizei stride = sizeof(TextVtx);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride,
                              reinterpret_cast<void*>(offsetof(TextVtx, x)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride,
                              reinterpret_cast<void*>(offsetof(TextVtx, u)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, stride,
                              reinterpret_cast<void*>(offsetof(TextVtx, r)));
        glBindVertexArray(0);
        return true;
    }

    // Sobe um quad de texto (4 vtx + 6 idx) e desenha - MESMA conversao px->clip do
    // Render2dGl3::Impl::draw_quad (duplicada, ver cabecalho do arquivo).
    void draw_text_quad(const QuadScreen& q, const UvRect& uv, GLuint tex, int pixel_w,
                        int pixel_h, float r, float g, float b, float a) {
        const float W = static_cast<float>(pixel_w);
        const float H = static_cast<float>(pixel_h);
        const float x0 = 2.0f * q.x / W - 1.0f;
        const float x1 = 2.0f * (q.x + q.w) / W - 1.0f;
        const float y0 = 1.0f - 2.0f * q.y / H;
        const float y1 = 1.0f - 2.0f * (q.y + q.h) / H;
        const TextVtx verts[4] = {
            {x0, y0, uv.u, uv.v, r, g, b, a},
            {x1, y0, uv.u + uv.w, uv.v, r, g, b, a},
            {x1, y1, uv.u + uv.w, uv.v + uv.h, r, g, b, a},
            {x0, y1, uv.u, uv.v + uv.h, r, g, b, a},
        };
        const GLuint idx[6] = {0, 1, 2, 0, 2, 3};
        glBindVertexArray(text_vao);
        glBindBuffer(GL_ARRAY_BUFFER, text_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STREAM_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, text_ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STREAM_DRAW);
        glUseProgram(text_program);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex);
        glUniform1i(text_loc_tex, 0);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);  // premultiplied, mesmo blend do Draw2d
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
    }

    FontFace* ensure_font(bool bold) {
        FontFace& face = bold ? font_bold : font_regular;
        if (face.tried) {
            return face.atlas.valid() && face.texture != 0 ? &face : nullptr;
        }
        face.tried = true;
        const std::string file(bold ? gus::core::assets::kFontMonoBoldFile
                                    : gus::core::assets::kFontMonoRegularFile);
        face.atlas = bake_font_atlas(resolve_font_path(file), /*cell_px=*/16);
        if (!face.atlas.valid()) {
            return nullptr;
        }
        // grayscale (alpha) -> RGBA premultiplied (branco * alpha; tint vem no draw) -
        // MESMO truque do Render2dGl3::Impl::ensure_font.
        const int w = face.atlas.atlas_w;
        const int h = face.atlas.atlas_h;
        std::vector<std::uint8_t> rgba(static_cast<std::size_t>(w) * h * 4);
        for (std::size_t i = 0; i < static_cast<std::size_t>(w) * h; ++i) {
            const std::uint8_t a = face.atlas.pixels[i];
            rgba[i * 4 + 0] = a;
            rgba[i * 4 + 1] = a;
            rgba[i * 4 + 2] = a;
            rgba[i * 4 + 3] = a;
        }
        GLuint tex = 0;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     rgba.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);
        face.texture = tex;
        return face.texture != 0 ? &face : nullptr;
    }

    ~Impl() {
        if (font_regular.texture) glDeleteTextures(1, &font_regular.texture);
        if (font_bold.texture) glDeleteTextures(1, &font_bold.texture);
        if (text_vbo) glDeleteBuffers(1, &text_vbo);
        if (text_ebo) glDeleteBuffers(1, &text_ebo);
        if (text_vao) glDeleteVertexArrays(1, &text_vao);
        if (text_program) glDeleteProgram(text_program);
        // draw2d (membro por valor): o proprio ~Draw2d() chama shutdown() (D11),
        // liberando as texturas/VBO/VAO/programa dele - nada a fazer aqui.
    }
};

Render2dGlintfx::Render2dGlintfx(bool gl_active) noexcept : gl_active_(gl_active) {
    if (gl_active_) {
        impl_ = std::make_unique<Impl>();
        const bool draw2d_ok = impl_->draw2d.init();
        const bool text_ok = impl_->init_text_gl();
        if (!draw2d_ok || !text_ok) {
            // Falha de init (programa GL nao compila/linka, ou contexto invalido):
            // degrada pra headless (no-op contabilizado), mesmo contrato do Render2dGl3.
            impl_.reset();
            gl_active_ = false;
        }
    }
}

Render2dGlintfx::~Render2dGlintfx() = default;

void Render2dGlintfx::begin_frame(const gus::core::spatial::Rect& camera_world,
                                  int pixel_w, int pixel_h) {
    draw_count_ = 0;
    camera_world_ = camera_world;
    pixel_w_ = pixel_w;
    pixel_h_ = pixel_h;
    if (!impl_) {
        return;  // headless
    }

    // O Draw2d NAO limpa o framebuffer (D9: so seta o estado GL de que depende a cada
    // flush, nunca glClear) - quem abre o frame e dono do clear, igual ao Render2dGl3.
    // NOTA: a vinheta/glow radial de fundo do Render2dGl3 (efeito decorativo, fora do
    // contrato das 6 primitivas do IRenderer) NAO e reproduzida aqui - flag no relatorio
    // D2D-2-SENTINELA pro lider decidir se importa portar.
    glViewport(0, 0, pixel_w, pixel_h);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_SCISSOR_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(24.0f * k1_255, 26.0f * k1_255, 34.0f * k1_255, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Rect de mundo -> Camera2d (ancora CENTRO + zoom uniforme) via o helper PURO do
    // proprio glintfx - ver a secao CAMERA do header pra prova de equivalencia com o
    // world_to_screen do Render2dGl3 sob aspecto uniforme.
    const glintfx::RectF world_rect{camera_world.x, camera_world.y, camera_world.w,
                                    camera_world.h};
    const glintfx::Camera2d cam =
        glintfx::camera_from_world_rect(world_rect, pixel_w, pixel_h);
    impl_->draw2d.begin(pixel_w, pixel_h);
    impl_->draw2d.set_camera(cam);
}

void Render2dGlintfx::draw_filled_rect(const gus::core::spatial::Rect& world_rect,
                                       const DrawColor& color) {
    ++draw_count_;
    if (!impl_) return;
    const glintfx::RectF dst{world_rect.x, world_rect.y, world_rect.w, world_rect.h};
    // ColorF do glintfx e STRAIGHT (nao-premultiplicado, D8) - o mesmo formato do nosso
    // DrawColor; o Draw2d premultiplica internamente (sem premultiplicar aqui, ao
    // contrario do Render2dGl3, que premultiplica na mao pro proprio shader dele).
    const glintfx::ColorF col{color.r, color.g, color.b, color.a};
    impl_->draw2d.draw_filled_rect(dst, col);
}

void Render2dGlintfx::draw_rect_outline(const gus::core::spatial::Rect& world_rect,
                                        const DrawColor& color, float thickness_world) {
    ++draw_count_;
    if (!impl_) return;
    const glintfx::RectF dst{world_rect.x, world_rect.y, world_rect.w, world_rect.h};
    const glintfx::ColorF col{color.r, color.g, color.b, color.a};
    // NOTA (divergencia POSITIVA vs. Render2dGl3): o Gl3 aproxima o contorno pra 1px de
    // TELA sempre (ignora thickness_world - contrato do IRenderer permite aproximar). O
    // Draw2d::draw_rect_outline respeita thickness_world de verdade (unidades do espaco
    // ATIVO, escala com o zoom da camera por projecao) - mais fiel ao contrato, nao
    // menos. Documentado no relatorio D2D-2-SENTINELA.
    impl_->draw2d.draw_rect_outline(dst, thickness_world, col);
}

TextureId Render2dGlintfx::load_texture(const char* path) {
    if (path == nullptr || path[0] == '\0') {
        return kInvalidTexture;
    }
    if (!impl_) {
        return kInvalidTexture;  // headless
    }
    const std::string key(path);
    auto it = impl_->by_path.find(key);
    if (it != impl_->by_path.end()) {
        return it->second;
    }
    glintfx::Texture2d tex = impl_->draw2d.load_texture(path);
    if (!tex.ok()) {
        return kInvalidTexture;
    }
    const TextureId id = static_cast<TextureId>(impl_->textures.size());
    impl_->textures.push_back(tex);
    impl_->by_path[key] = id;
    return id;
}

void Render2dGlintfx::draw_textured_rect(const gus::core::spatial::Rect& world_rect,
                                         TextureId texture, const UvRect& uv,
                                         const DrawColor& tint) {
    if (texture == kInvalidTexture) {
        return;  // no-op: cabe ao app desenhar o fallback (mesmo contrato do Gl3/SDL)
    }
    ++draw_count_;
    if (!impl_ || texture >= impl_->textures.size()) return;
    const glintfx::Texture2d& tex = impl_->textures[texture];
    if (!tex.ok()) return;

    const glintfx::RectF dst{world_rect.x, world_rect.y, world_rect.w, world_rect.h};
    // O nosso UvRect e NORMALIZADO [0,1] (fracao da textura); o src_px do Draw2d e em
    // TEXEL (mesma origem topo-esquerda/y-pra-baixo, D5) - converte multiplicando pelas
    // dimensoes reais da textura (Texture2d::width()/height(), decodificadas no load).
    const glintfx::RectF src{uv.u * static_cast<float>(tex.width()),
                             uv.v * static_cast<float>(tex.height()),
                             uv.w * static_cast<float>(tex.width()),
                             uv.h * static_cast<float>(tex.height())};
    const glintfx::ColorF col{tint.r, tint.g, tint.b, tint.a};
    impl_->draw2d.draw_sprite(tex, dst, src, col);
}

ContentBbox Render2dGlintfx::texture_content_bbox(TextureId texture) const {
    if (!impl_ || texture == kInvalidTexture || texture >= impl_->textures.size()) {
        return ContentBbox{};
    }
    const glintfx::Texture2d& tex = impl_->textures[texture];
    if (!tex.ok()) {
        return ContentBbox{};
    }
    // Draw2d::texture_content_bbox nao e const (Draw2d por baixo cacheia no registry
    // proprio) - chamar por um ponteiro membro (impl_) continua legal dentro de um
    // metodo const desta classe (o ponteiro e const-top-level, o apontado nao).
    const glintfx::TextureBbox bbox = impl_->draw2d.texture_content_bbox(tex);
    if (!bbox.found) {
        return ContentBbox{};  // mesma definicao "alpha>0" do nosso scan_alpha_content_bbox
    }
    ContentBbox out;
    out.canvas_w = tex.width();
    out.canvas_h = tex.height();
    out.left = bbox.x;
    out.top = bbox.y;
    out.width = bbox.w;
    out.height = bbox.h;
    return out;
}

void Render2dGlintfx::draw_text(const char* text, float x, float y, float px_size,
                                const DrawColor& color, bool bold) {
    if (text == nullptr || px_size <= 0.0f) {
        return;
    }
    if (!impl_) return;  // headless
    Impl::FontFace* face = impl_->ensure_font(bold);
    if (face == nullptr) {
        return;  // sem fonte: no-op (caller ja tem fallback)
    }

    // Fecha o bracket do Draw2d ANTES de desenhar texto na pipeline GL propria: forca o
    // flush do que estava pendente no batcher, preservando a ORDEM DE DESENHO
    // (painter's order, contrato do IRenderer) entre os draws batchados e o texto cru.
    // Reabre depois - a camera fica sticky entre brackets (D13), nada se perde.
    impl_->draw2d.end();

    const float advance = glyph_advance(px_size);
    float pen_x = x;
    // ColorF straight (nao-premultiplicado) seria o formato do Draw2d, mas aqui e a
    // pipeline PROPRIA (mesma conta do Render2dGl3): a textura do glifo ja e
    // premultiplied (branco*alpha), entao o tint tambem precisa ser premultiplicado pro
    // shader de texto (glDrawElements acima nao faz a conta - so multiplica textura*cor).
    const float pr = color.r * color.a, pg = color.g * color.a, pb = color.b * color.a;
    for (const std::uint32_t cp : decode_utf8(text)) {
        const UvRect uv = face->atlas.glyph_uv(static_cast<int>(cp));
        if (uv.w > 0.0f) {
            const gus::core::spatial::Rect cell_world{pen_x, y, px_size, px_size};
            const QuadScreen q =
                build_quad_screen(cell_world, camera_world_, pixel_w_, pixel_h_);
            impl_->draw_text_quad(q, uv, face->texture, pixel_w_, pixel_h_, pr, pg, pb,
                                  color.a);
            ++draw_count_;
        }
        pen_x += advance;
    }

    impl_->draw2d.begin(pixel_w_, pixel_h_);
}

void Render2dGlintfx::end_frame() {
    last_draw_count_ = draw_count_;
    if (!impl_) return;  // headless
    impl_->draw2d.end();
    // O swap real e SDL_GL_SwapWindow, feito pela casca app/ (mesmo contrato do
    // Render2dGl3::end_frame) - aqui so fecha o ultimo bracket do Draw2d.
}

void Render2dGlintfx::present() {
    // Simetria de API com Render2dGl3::present() - o swap nao pertence a este backend.
}

}  // namespace gus::platform::render2d

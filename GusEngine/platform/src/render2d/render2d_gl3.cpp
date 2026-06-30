// gus/platform/src/render2d/render2d_gl3.cpp
//
// Implementacao do Render2dGl3 (backend de IRenderer sobre OpenGL 3.3 core, ADR-009
// adendo GL3). Ver header. Travado por platform/tests/render2d_gl3_test.cpp (TEST-FIRST,
// caminho headless) + smoke visual do app (--battle com contexto GL real).
//
// DESENHO: um unico programa GL (vertex+fragment) desenha quads. Cada quad e 4 vertices
// (pos em clip-space [-1,1] + uv + cor RGBA premultiplied) e 6 indices (2 triangulos).
// A projecao mundo->pixel reusa build_quad_screen (viewport_transform POCO, testada);
// converto os pixels resultantes para clip-space na CPU (x_clip = 2*px/W - 1; y_clip =
// 1 - 2*py/H, +Y para baixo em pixel vira +Y para cima em clip). Quad colorido = sem
// textura (uniform use_texture=0). Quad de sprite/fonte = textura NEAREST * tint.
//
// O .cpp inclui o loader glad via RmlUi_Include_GL3.h (header-only). A IMPLEMENTACAO do
// glad (GLAD_GL_IMPLEMENTATION) ja e definida pelo RmlUi_Renderer_GL3.cpp - aqui so o
// header (declaracoes + ponteiros de funcao globais que aquele .cpp resolve). stb_image
// idem: so o header (impl no render2d_sdl.cpp).

#include "gus/platform/render2d/render2d_gl3.hpp"

#include <array>
#include <cstddef>  // offsetof
#include <cstdint>
#include <cstdlib>  // std::getenv
#include <string>
#include <vector>

#include "gus/core/asset_paths.hpp"               // nomes de arquivo de fonte
#include "gus/platform/render2d/alpha_bbox.hpp"   // scan_alpha_content_bbox (POCO)
#include "gus/platform/render2d/text_metrics.hpp"  // glyph_advance + decode_utf8
#include "gus/platform/render2d/viewport_transform.hpp"

// glad (GL 3.3 core) - so o header; a impl vem do RmlUi_Renderer_GL3.cpp (mesma TU-set).
#include "RmlUi_Include_GL3.h"

// stb_image - so o header (impl em render2d_sdl.cpp).
#include "stb_image.h"

namespace gus::platform::render2d {

namespace {

constexpr float k1_255 = 1.0f / 255.0f;

// O resolver de caminho de fonte (env GUSWORLD_FONTS > macro) JA existe publico em
// font_atlas.hpp (gus::platform::render2d::resolve_font_path); reusamos ele (evita
// duplicar a logica e a ambiguidade de overload). Mesma fonte que o Render2dSdl.

// Vertice do quad: posicao em clip-space + uv + cor (premultiplied alpha).
struct Vtx {
    float x, y;   // clip-space [-1,1]
    float u, v;   // uv [0,1]
    float r, g, b, a;
};

const char* kVertSrc = R"GLSL(#version 330 core
layout(location=0) in vec2 a_pos;
layout(location=1) in vec2 a_uv;
layout(location=2) in vec4 a_col;
out vec2 v_uv;
out vec4 v_col;
void main(){ v_uv = a_uv; v_col = a_col; gl_Position = vec4(a_pos, 0.0, 1.0); }
)GLSL";

const char* kFragSrc = R"GLSL(#version 330 core
in vec2 v_uv;
in vec4 v_col;
out vec4 frag;
uniform sampler2D u_tex;
uniform int u_use_texture;
void main(){
  if (u_use_texture == 1) { frag = texture(u_tex, v_uv) * v_col; }
  else { frag = v_col; }
}
)GLSL";

// ---------------------------------------------------------------------------
// Vinheta / glow radial de fundo da arena (ADR-009 adendo). Quad full-window
// desenhado LOGO APOS o clear e ANTES dos sprites/HUD => fica ATRAS de tudo,
// dando profundidade (bordas mais escuras + leve clareamento no centro). NAO
// lava os sprites (e fundo opaco; os sprites compoem por cima depois).
//
// AJUSTE PELO LIDER: os 6 valores abaixo sao as alavancas (cor de centro/borda
// + foco + raio + suavidade). Paleta canonica: base #0c0f1a; centro = leve lift
// (#141a2c); bordas mais escuras (#06080f). Tudo SUTIL: profundidade, nao holofote.
constexpr float kVigCenterRgb[3] = {0x14, 0x1a, 0x2c};  // #141a2c lift no centro
constexpr float kVigEdgeRgb[3] = {0x06, 0x08, 0x0f};    // #06080f bordas escuras
constexpr float kVigFocusX = 0.50f;   // centro do glow em X (0=esq, 1=dir)
constexpr float kVigFocusY = 0.46f;   // levemente ACIMA do meio (onde ficam os atores)
constexpr float kVigRadius = 0.85f;   // distancia (uv) onde chega na cor de borda
constexpr float kVigSoftness = 0.65f;  // largura do falloff (maior = mais suave)

const char* kVigVertSrc = R"GLSL(#version 330 core
layout(location=0) in vec2 a_pos;
out vec2 v_uv;
void main(){ v_uv = a_pos * 0.5 + 0.5; gl_Position = vec4(a_pos, 0.0, 1.0); }
)GLSL";

const char* kVigFragSrc = R"GLSL(#version 330 core
in vec2 v_uv;
out vec4 frag;
uniform vec3 u_center;
uniform vec3 u_edge;
uniform vec2 u_focus;
uniform float u_radius;
uniform float u_softness;
void main(){
  float d = distance(v_uv, u_focus);
  float t = smoothstep(u_radius - u_softness, u_radius, d);
  frag = vec4(mix(u_center, u_edge, t), 1.0);
}
)GLSL";

}  // namespace

// ---------------------------------------------------------------------------
// Impl: handles GL (so existe quando gl_active). Em headless impl_ == nullptr.
// ---------------------------------------------------------------------------
struct Render2dGl3::Impl {
    GLuint program = 0;
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0;
    GLint loc_use_texture = -1;
    GLint loc_tex = -1;

    // Vinheta de fundo (programa proprio; reusa o vao/vbo/ebo do quad principal).
    GLuint vig_program = 0;
    GLint vig_loc_center = -1;
    GLint vig_loc_edge = -1;
    GLint vig_loc_focus = -1;
    GLint vig_loc_radius = -1;
    GLint vig_loc_softness = -1;

    int pixel_w = 0;
    int pixel_h = 0;
    gus::core::spatial::Rect camera{};

    // Texturas de sprite: indexadas por TextureId (1-based; 0 invalido). Cache por path +
    // alpha-bbox paralelo (mesmo id), igual ao Render2dSdl.
    std::vector<GLuint> textures{0};               // slot 0 = invalido
    std::vector<ContentBbox> bboxes{ContentBbox{}};
    std::unordered_map<std::string, TextureId> by_path;

    // Atlas de fonte por face (regular/bold): textura GL + atlas (metricas).
    struct FontFace {
        FontAtlas atlas;
        GLuint texture = 0;
        bool tried = false;
    };
    FontFace font_regular;
    FontFace font_bold;

    bool ok = false;

    bool init_gl() {
        // Compila o programa.
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
        GLuint vs = compile(GL_VERTEX_SHADER, kVertSrc);
        GLuint fs = compile(GL_FRAGMENT_SHADER, kFragSrc);
        if (!vs || !fs) {
            if (vs) glDeleteShader(vs);
            if (fs) glDeleteShader(fs);
            return false;
        }
        program = glCreateProgram();
        glAttachShader(program, vs);
        glAttachShader(program, fs);
        glLinkProgram(program);
        glDeleteShader(vs);
        glDeleteShader(fs);
        GLint linked = 0;
        glGetProgramiv(program, GL_LINK_STATUS, &linked);
        if (!linked) {
            glDeleteProgram(program);
            program = 0;
            return false;
        }
        loc_use_texture = glGetUniformLocation(program, "u_use_texture");
        loc_tex = glGetUniformLocation(program, "u_tex");

        // Programa da vinheta de fundo (radial-gradient). Falha de compile/link aqui NAO e
        // fatal: degrada pra "sem vinheta" (vig_program == 0 => draw_vignette e no-op), a
        // arena continua com o clear chapado.
        GLuint vvs = compile(GL_VERTEX_SHADER, kVigVertSrc);
        GLuint vfs = compile(GL_FRAGMENT_SHADER, kVigFragSrc);
        if (vvs && vfs) {
            vig_program = glCreateProgram();
            glAttachShader(vig_program, vvs);
            glAttachShader(vig_program, vfs);
            glLinkProgram(vig_program);
            GLint vlinked = 0;
            glGetProgramiv(vig_program, GL_LINK_STATUS, &vlinked);
            if (!vlinked) {
                glDeleteProgram(vig_program);
                vig_program = 0;
            } else {
                vig_loc_center = glGetUniformLocation(vig_program, "u_center");
                vig_loc_edge = glGetUniformLocation(vig_program, "u_edge");
                vig_loc_focus = glGetUniformLocation(vig_program, "u_focus");
                vig_loc_radius = glGetUniformLocation(vig_program, "u_radius");
                vig_loc_softness = glGetUniformLocation(vig_program, "u_softness");
            }
        }
        if (vvs) glDeleteShader(vvs);
        if (vfs) glDeleteShader(vfs);

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        const GLsizei stride = sizeof(Vtx);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride,
                              reinterpret_cast<void*>(offsetof(Vtx, x)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride,
                              reinterpret_cast<void*>(offsetof(Vtx, u)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, stride,
                              reinterpret_cast<void*>(offsetof(Vtx, r)));
        glBindVertexArray(0);
        ok = true;
        return true;
    }

    // Sobe um quad (4 vtx + 6 idx) e desenha. cor ja em premultiplied alpha.
    void draw_quad(const QuadScreen& q, const UvRect& uv, GLuint tex, float r, float g,
                   float b, float a) {
        // px -> clip. +Y px (para baixo) vira -Y clip.
        const float W = static_cast<float>(pixel_w);
        const float H = static_cast<float>(pixel_h);
        const float x0 = 2.0f * q.x / W - 1.0f;
        const float x1 = 2.0f * (q.x + q.w) / W - 1.0f;
        const float y0 = 1.0f - 2.0f * q.y / H;
        const float y1 = 1.0f - 2.0f * (q.y + q.h) / H;
        const Vtx verts[4] = {
            {x0, y0, uv.u, uv.v, r, g, b, a},
            {x1, y0, uv.u + uv.w, uv.v, r, g, b, a},
            {x1, y1, uv.u + uv.w, uv.v + uv.h, r, g, b, a},
            {x0, y1, uv.u, uv.v + uv.h, r, g, b, a},
        };
        const GLuint idx[6] = {0, 1, 2, 0, 2, 3};
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STREAM_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STREAM_DRAW);
        glUseProgram(program);
        if (tex != 0) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, tex);
            glUniform1i(loc_tex, 0);
            glUniform1i(loc_use_texture, 1);
        } else {
            glUniform1i(loc_use_texture, 0);
        }
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
    }

    // Vinheta de fundo: 1 quad full-window (NDC -1..1) com radial-gradient no frag.
    // Reusa o vao/vbo/ebo do quad principal (so o atributo de posicao importa aqui; o
    // vertex shader da vinheta le apenas location=0). Opaco (alpha=1): substitui o clear.
    void draw_vignette() {
        if (vig_program == 0) return;  // degradou no init: arena fica com o clear chapado
        const Vtx verts[4] = {
            {-1.0f, -1.0f, 0, 0, 0, 0, 0, 0},
            {1.0f, -1.0f, 0, 0, 0, 0, 0, 0},
            {1.0f, 1.0f, 0, 0, 0, 0, 0, 0},
            {-1.0f, 1.0f, 0, 0, 0, 0, 0, 0},
        };
        const GLuint idx[6] = {0, 1, 2, 0, 2, 3};
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STREAM_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STREAM_DRAW);
        glUseProgram(vig_program);
        glUniform3f(vig_loc_center, kVigCenterRgb[0] * k1_255, kVigCenterRgb[1] * k1_255,
                    kVigCenterRgb[2] * k1_255);
        glUniform3f(vig_loc_edge, kVigEdgeRgb[0] * k1_255, kVigEdgeRgb[1] * k1_255,
                    kVigEdgeRgb[2] * k1_255);
        glUniform2f(vig_loc_focus, kVigFocusX, kVigFocusY);
        glUniform1f(vig_loc_radius, kVigRadius);
        glUniform1f(vig_loc_softness, kVigSoftness);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
    }

    GLuint upload_rgba(const std::uint8_t* pixels, int w, int h, bool nearest) {
        GLuint tex = 0;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     pixels);
        const GLint filter = nearest ? GL_NEAREST : GL_LINEAR;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);
        return tex;
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
        // grayscale (alpha) -> RGBA premultiplied (branco * alpha; tint vem no draw).
        const int w = face.atlas.atlas_w;
        const int h = face.atlas.atlas_h;
        std::vector<std::uint8_t> rgba(static_cast<std::size_t>(w) * h * 4);
        for (std::size_t i = 0; i < static_cast<std::size_t>(w) * h; ++i) {
            const std::uint8_t a = face.atlas.pixels[i];
            rgba[i * 4 + 0] = a;  // premultiplied: branco*alpha = alpha
            rgba[i * 4 + 1] = a;
            rgba[i * 4 + 2] = a;
            rgba[i * 4 + 3] = a;
        }
        face.texture = upload_rgba(rgba.data(), w, h, /*nearest=*/true);
        return face.texture != 0 ? &face : nullptr;
    }

    ~Impl() {
        for (std::size_t i = 1; i < textures.size(); ++i) {
            if (textures[i] != 0) glDeleteTextures(1, &textures[i]);
        }
        if (font_regular.texture) glDeleteTextures(1, &font_regular.texture);
        if (font_bold.texture) glDeleteTextures(1, &font_bold.texture);
        if (vbo) glDeleteBuffers(1, &vbo);
        if (ebo) glDeleteBuffers(1, &ebo);
        if (vao) glDeleteVertexArrays(1, &vao);
        if (program) glDeleteProgram(program);
        if (vig_program) glDeleteProgram(vig_program);
    }
};

Render2dGl3::Render2dGl3(bool gl_active) noexcept : gl_active_(gl_active) {
    if (gl_active_) {
        impl_ = std::make_unique<Impl>();
        if (!impl_->init_gl()) {
            // Falha de programa GL: degrada pra headless (no-op contabilizado).
            impl_.reset();
            gl_active_ = false;
        }
    }
}

Render2dGl3::~Render2dGl3() = default;

void Render2dGl3::begin_frame(const gus::core::spatial::Rect& camera_world, int pixel_w,
                              int pixel_h) {
    draw_count_ = 0;
    if (!impl_) {
        return;  // headless
    }
    impl_->camera = camera_world;
    impl_->pixel_w = pixel_w;
    impl_->pixel_h = pixel_h;
    // A arena e dona do clear (o HUD compoe por cima depois). Premultiplied alpha blend.
    glViewport(0, 0, pixel_w, pixel_h);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_SCISSOR_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(24.0f * k1_255, 26.0f * k1_255, 34.0f * k1_255, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    // Vinheta/glow radial de fundo LOGO APOS o clear (ANTES de qualquer sprite/HUD): da
    // profundidade (bordas escuras + leve lift no centro). E quad full-window opaco; os
    // sprites compoem por cima sem serem lavados. Composicao arena->UI->swap inalterada.
    impl_->draw_vignette();
}

void Render2dGl3::draw_filled_rect(const gus::core::spatial::Rect& world_rect,
                                   const DrawColor& color) {
    ++draw_count_;
    if (!impl_) return;
    const QuadScreen q =
        build_quad_screen(world_rect, impl_->camera, impl_->pixel_w, impl_->pixel_h);
    // premultiplica a cor pelo alpha (o blend e premultiplied).
    impl_->draw_quad(q, UvRect{0, 0, 0, 0}, 0, color.r * color.a, color.g * color.a,
                     color.b * color.a, color.a);
}

void Render2dGl3::draw_rect_outline(const gus::core::spatial::Rect& world_rect,
                                    const DrawColor& color, float /*thickness_world*/) {
    ++draw_count_;
    if (!impl_) return;
    const QuadScreen q =
        build_quad_screen(world_rect, impl_->camera, impl_->pixel_w, impl_->pixel_h);
    // Contorno de 1px: 4 quads finos (topo/baixo/esq/dir) na borda do retangulo de tela.
    const float pr = color.r * color.a, pg = color.g * color.a, pb = color.b * color.a;
    const float t = 1.0f;  // 1px
    const UvRect none{0, 0, 0, 0};
    impl_->draw_quad(QuadScreen{q.x, q.y, q.w, t}, none, 0, pr, pg, pb, color.a);
    impl_->draw_quad(QuadScreen{q.x, q.y + q.h - t, q.w, t}, none, 0, pr, pg, pb, color.a);
    impl_->draw_quad(QuadScreen{q.x, q.y, t, q.h}, none, 0, pr, pg, pb, color.a);
    impl_->draw_quad(QuadScreen{q.x + q.w - t, q.y, t, q.h}, none, 0, pr, pg, pb, color.a);
}

TextureId Render2dGl3::load_texture(const char* path) {
    if (path == nullptr || path[0] == '\0') {
        return kInvalidTexture;
    }
    const std::string key(path);
    if (!impl_) {
        return kInvalidTexture;  // headless
    }
    auto it = impl_->by_path.find(key);
    if (it != impl_->by_path.end()) {
        return it->second;
    }
    int w = 0, h = 0, channels = 0;
    stbi_uc* pixels = stbi_load(path, &w, &h, &channels, 4);
    if (pixels == nullptr || w <= 0 || h <= 0) {
        if (pixels != nullptr) stbi_image_free(pixels);
        return kInvalidTexture;
    }
    // Alpha-bbox medido com os pixels NAO premultiplicados (alpha original).
    const ContentBbox bbox =
        scan_alpha_content_bbox(static_cast<const std::uint8_t*>(pixels), w, h);
    // Premultiplica para o blend premultiplied alpha.
    const std::size_t n = static_cast<std::size_t>(w) * h * 4;
    for (std::size_t i = 0; i < n; i += 4) {
        const unsigned int a = pixels[i + 3];
        pixels[i + 0] = static_cast<stbi_uc>(pixels[i + 0] * a / 255);
        pixels[i + 1] = static_cast<stbi_uc>(pixels[i + 1] * a / 255);
        pixels[i + 2] = static_cast<stbi_uc>(pixels[i + 2] * a / 255);
    }
    const GLuint tex = impl_->upload_rgba(pixels, w, h, /*nearest=*/true);
    stbi_image_free(pixels);
    if (tex == 0) {
        return kInvalidTexture;
    }
    const TextureId id = static_cast<TextureId>(impl_->textures.size());
    impl_->textures.push_back(tex);
    impl_->bboxes.push_back(bbox);
    impl_->by_path[key] = id;
    return id;
}

void Render2dGl3::draw_textured_rect(const gus::core::spatial::Rect& world_rect,
                                     TextureId texture, const UvRect& uv,
                                     const DrawColor& tint) {
    if (texture == kInvalidTexture) {
        return;  // no-op: cabe ao app desenhar o fallback (mesmo contrato do SDL)
    }
    ++draw_count_;
    if (!impl_ || texture >= impl_->textures.size()) return;
    const GLuint tex = impl_->textures[texture];
    if (tex == 0) return;
    const QuadScreen q =
        build_quad_screen(world_rect, impl_->camera, impl_->pixel_w, impl_->pixel_h);
    // tint multiplica a textura (que ja esta premultiplied). tint tambem premultiplicado.
    impl_->draw_quad(q, uv, tex, tint.r * tint.a, tint.g * tint.a, tint.b * tint.a,
                     tint.a);
}

ContentBbox Render2dGl3::texture_content_bbox(TextureId texture) const {
    if (!impl_ || texture == kInvalidTexture || texture >= impl_->bboxes.size()) {
        return ContentBbox{};
    }
    return impl_->bboxes[texture];
}

void Render2dGl3::draw_text(const char* text, float x, float y, float px_size,
                            const DrawColor& color, bool bold) {
    if (text == nullptr || px_size <= 0.0f) {
        return;
    }
    if (!impl_) return;  // headless
    Impl::FontFace* face = impl_->ensure_font(bold);
    if (face == nullptr) {
        return;  // sem fonte: no-op (caller ja tem fallback)
    }
    const float advance = glyph_advance(px_size);
    float pen_x = x;
    // tint premultiplicado (a textura de glifo ja e premultiplied: alpha do glifo).
    const float pr = color.r * color.a, pg = color.g * color.a, pb = color.b * color.a;
    for (const std::uint32_t cp : decode_utf8(text)) {
        const UvRect uv = face->atlas.glyph_uv(static_cast<int>(cp));
        if (uv.w > 0.0f) {
            const gus::core::spatial::Rect cell_world{pen_x, y, px_size, px_size};
            const QuadScreen q = build_quad_screen(cell_world, impl_->camera,
                                                   impl_->pixel_w, impl_->pixel_h);
            impl_->draw_quad(q, uv, face->texture, pr, pg, pb, color.a);
            ++draw_count_;
        }
        pen_x += advance;
    }
}

void Render2dGl3::end_frame() {
    last_draw_count_ = draw_count_;
    // No GL o SWAP (SDL_GL_SwapWindow) e do contexto da janela, feito pela casca app/.
    // Aqui nao apresentamos (defer ou nao): o present()/swap fica na casca. end_frame so
    // congela a contagem. O HUD GL3 compoe por cima ANTES do swap.
}

void Render2dGl3::present() {
    // O swap real e SDL_GL_SwapWindow (na casca app/, que tem o SDL_Window). Mantido como
    // ponto de simetria com o Render2dSdl; aqui e no-op (o swap nao pertence ao renderer).
}

}  // namespace gus::platform::render2d

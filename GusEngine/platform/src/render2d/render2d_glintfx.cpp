// SPDX-License-Identifier: Apache-2.0
// gus/platform/src/render2d/render2d_glintfx.cpp
//
// Implementacao do Render2dGlintfx (3a impl. de IRenderer, delega ao glintfx::Draw2d
// v0.24.0). Ver header pro contrato completo (camera, texto, headless). Travado por
// platform/tests/render2d_glintfx_test.cpp (TEST-FIRST, equivalencia de camera +
// caminho headless) + smoke visual do app (--battle com contexto GL real, quando
// GUSWORLD_RENDER2D_BACKEND=glintfx).
//
// TEXTO (Draw2d::draw_text/load_font/measure_text, D2D-TEXT - ver header, secao TEXTO):
// a pipeline GL propria + FontAtlas/stb_truetype que este arquivo usava ate v0.24.0 foi
// REMOVIDA (nao mais duplicada do Render2dGl3 - esse backend continua com a pipeline
// antiga por conta propria, intocado). O texto batcha junto dos sprites no mesmo
// Draw2d::draw2d; a camera/scissor/layer sao as MESMAS ja setadas por begin_frame.

#include "gus/platform/render2d/render2d_glintfx.hpp"

#include <string>
#include <unordered_map>
#include <vector>

#include <glintfx/draw2d.hpp>

#include "gus/core/asset_paths.hpp"              // nomes de arquivo de fonte
#include "gus/platform/render2d/font_atlas.hpp"  // resolve_font_path (POCO de caminho)

// glad (GL 3.3 core) - so o header; a impl vem do gl3_loader.cpp (mesma TU-set que o
// Render2dGl3 ja compartilha, ADR-010 F3). Precisamos das chamadas GL cruas so pro
// glClear/glViewport/glBlendFunc do begin_frame (o Draw2D NAO limpa o framebuffer, D9) -
// nao mais pra texto (migrado pro Draw2d::draw_text, ver cabecalho do arquivo).
#include "RmlUi_Include_GL3.h"

namespace gus::platform::render2d {

namespace {

constexpr float k1_255 = 1.0f / 255.0f;

}  // namespace

// ---------------------------------------------------------------------------
// Impl: Draw2d (6 primitivas de mundo, incluindo texto) + Font2d regular/bold
// cacheadas. So existe quando gl_active. Em headless impl_ == nullptr (espelha o
// Render2dGl3).
// ---------------------------------------------------------------------------
struct Render2dGlintfx::Impl {
    glintfx::Draw2d draw2d;

    // Font2d e um HANDLE opaco pro registry interno do Draw2d (load_font parseia o SFNT
    // uma vez; o atlas de glifo POR-TAMANHO e criado preguicosamente pelo PROPRIO Draw2d
    // no 1o draw_text()/measure_text() daquele tamanho - nao ha mais cache por-cell_px
    // mantido aqui, ver header). unavailable trava a falha (arquivo ausente/corrompido):
    // nunca reoferece o load_font (mesmo contrato fail-high do ensure_font antigo).
    glintfx::Font2d font_regular;
    glintfx::Font2d font_bold;
    bool font_regular_unavailable = false;
    bool font_bold_unavailable = false;

    // Garante a Font2d da face pedida (carrega no 1o uso). Devolve nullptr se
    // indisponivel (headless sem draw2d.ok() / arquivo ausente / SFNT invalido).
    glintfx::Font2d* ensure_font(bool bold) {
        glintfx::Font2d& font = bold ? font_bold : font_regular;
        bool& unavailable = bold ? font_bold_unavailable : font_regular_unavailable;
        if (font.ok()) {
            return &font;
        }
        if (unavailable) {
            return nullptr;  // ja falhou uma vez: nunca retenta (mesmo arquivo falharia)
        }
        const std::string file(bold ? gus::core::assets::kFontMonoBoldFile
                                    : gus::core::assets::kFontMonoRegularFile);
        font = draw2d.load_font(resolve_font_path(file).c_str());
        if (!font.ok()) {
            unavailable = true;
            return nullptr;
        }
        return &font;
    }

    // Texturas de sprite: indexadas por TextureId (1-based; 0 invalido), espelhando o
    // vetor by-id do Render2dGl3 - so que aqui o "handle" e o proprio glintfx::Texture2d
    // (copiavel, trivialmente pequeno - Draw2d e quem possui o recurso GL de verdade).
    std::vector<glintfx::Texture2d> textures{glintfx::Texture2d{}};  // slot 0 = invalido
    std::unordered_map<std::string, TextureId> by_path;

    ~Impl() {
        // draw2d (membro por valor): o proprio ~Draw2d() chama shutdown() (D11),
        // liberando as texturas/VBO/VAO/programa/registry-de-fonte dele - nada a fazer
        // aqui (font_regular/font_bold sao handles, nao donos RAII).
    }
};

Render2dGlintfx::Render2dGlintfx(bool gl_active) noexcept : gl_active_(gl_active) {
    if (gl_active_) {
        impl_ = std::make_unique<Impl>();
        if (!impl_->draw2d.init()) {
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
    glintfx::Font2d* font = impl_->ensure_font(bold);
    if (font == nullptr) {
        return;  // sem fonte: no-op (caller ja tem fallback)
    }
    // Draw2d::draw_text batcha junto dos sprites/retangulos ja abertos no bracket
    // begin()/end() deste frame (ver begin_frame/end_frame) - nao ha mais bracket
    // proprio nem pipeline GL a parte pro texto (ver cabecalho do arquivo). ColorF e
    // STRAIGHT (nao-premultiplicado, D8), o mesmo formato do nosso DrawColor - o Draw2d
    // premultiplica internamente, igual draw_filled_rect/draw_textured_rect acima.
    const glintfx::ColorF col{color.r, color.g, color.b, color.a};
    impl_->draw2d.draw_text(*font, text, glintfx::Vec2F{x, y}, px_size, col);
    ++draw_count_;  // 1 emissao por chamada (a string inteira e 1 primitivo pro batcher)
}

float Render2dGlintfx::measure_text_width(const char* text, float px_size, bool bold) {
    if (text == nullptr || px_size <= 0.0f) {
        return 0.0f;
    }
    if (!impl_) return 0.0f;  // headless: sem draw2d/fonte
    glintfx::Font2d* font = impl_->ensure_font(bold);
    if (font == nullptr) {
        return 0.0f;  // sem fonte: mesmo fallback de draw_text
    }
    const glintfx::TextMetrics m = impl_->draw2d.measure_text(*font, text, px_size);
    return m.ok ? m.width : 0.0f;
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

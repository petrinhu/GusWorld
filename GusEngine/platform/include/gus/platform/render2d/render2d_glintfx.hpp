// SPDX-License-Identifier: GPL-3.0-or-later
// gus/platform/render2d/render2d_glintfx.hpp
//
// Render2dGlintfx: TERCEIRA implementacao de IRenderer (D2D-2-SENTINELA), delegando ao
// glintfx::Draw2d (pin v0.24.0, GLINTFX_MODULE_DRAW2D=ON - ver GusEngine/CMakeLists.txt)
// pras 6 primitivas de MUNDO (begin/end de frame, retangulo preenchido, contorno, sprite
// texturizado, bbox de conteudo, E TEXTO - ver secao TEXTO abaixo, migracao 2026-07-29
// autorizada pelo lider). Zero FreeType, zero stb_truetype neste backend.
//
// NAO E O DEFAULT: selecionada em tempo de COMPILACAO pela opcao de CMake
// GUSWORLD_RENDER2D_BACKEND (valores "gl3" [default] | "glintfx", ver
// platform/CMakeLists.txt) - reversivel (flip da cache var + reconfigure + rebuild),
// comparavel lado a lado com Render2dGl3. Os dois backends expoem a MESMA superficie
// publica (ctor(bool gl_active) + IRenderer + present()/set_defer_present()/
// defer_present()/last_draw_count() de simetria), entao app/sdl_window.hpp troca so o
// tipo concreto por tras de um alias (`using ActiveRenderer2d = ...`), zero mudanca nos
// call sites. Selecao em TEMPO DE COMPILACAO (nao runtime/env var) de proposito: o campo
// render2d_ do SdlWindow e tipado concreto (nao IRenderer*) e chama metodos que NAO
// pertencem ao IRenderer (present/set_defer_present/last_draw_count) - um switch runtime
// exigiria ou alargar o contrato do IRenderer (bate em Render2dSdl tambem, fora do escopo
// desta fatia) ou um wrapper/variant tocando todo call site (nao seria "mudanca minima").
// Um alias de tipo resolvido em CMake mantem os dois backends 1:1 substituiveis com UMA
// linha de diff no campo do SdlWindow.
//
// CAMERA (a parte delicada desta fatia): begin_frame recebe um RETANGULO de mundo
// (contrato do IRenderer, canto+tamanho); o Draw2D usa Camera2d (ancora no CENTRO +
// zoom UNIFORME, mesmo fator nos dois eixos). A conversao usa o proprio helper PURO do
// glintfx, `camera_from_world_rect(world_rect, pixel_w, pixel_h)` (D17 do header deles:
// zoom = pixel_w / world_rect.w, centro = centro do retangulo) - EQUIVALENTE ao nosso
// world_to_screen (viewport_transform.hpp, usado pelo Render2dGl3) sempre que a camera
// mantiver aspecto uniforme, ou seja `camera_world.w / pixel_w == camera_world.h /
// pixel_h` - o invariante que clamp_camera()+world_span_from_pixels() (core/spatial/
// camera_clamp.*) JA garantem em todo caminho de render deste jogo (os dois eixos usam o
// MESMO ppu). Prova por igualdade ponto-a-ponto contra o world_to_screen real do glintfx
// e contra o build_quad_screen do Render2dGl3: platform/tests/render2d_glintfx_test.cpp.
// FORA desse invariante (camera com aspecto NAO-uniforme - nenhum caller atual do
// IRenderer produz isso, mas o CONTRATO da interface nao proibe) o Draw2D diverge do
// Render2dGl3: zoom uniforme (Camera2d) vs. escala independente por eixo
// (world_to_screen do Gl3) - a projecao usada aqui escala por LARGURA (zoom = pixel_w /
// camera_world.w), entao o eixo vertical estica/encolhe diferente do Gl3 quando o
// aspecto diverge. Documentado, nao escondido; nao ha caller real hoje que exercite isso.
//
// TEXTO (decisao do lider, 2026-07-29, one-way na pratica - docs/tech/
// plano-fim-dos-workarounds.md secao 4/9-D1 + docs/tech/fontflip-draw2d-dossie.md):
// draw_text() CHAMA Draw2d::draw_text/load_font (D2D-TEXT), o motor de fonte SOBERANO
// proprio do glintfx (zero FreeType, zero stb_truetype neste caminho) - substitui a
// pipeline GL propria + FontAtlas/stb_truetype que este arquivo usava ate v0.24.0 (ver
// historico em font_atlas.cpp/render2d_gl3.cpp/render2d_sdl.cpp, que CONTINUAM na
// pipeline antiga - nao sao backends desta fatia). O texto agora BATCHA junto dos
// sprites/retangulos no MESMO Draw2d::draw2d (nao ha mais bracket end()/begin() ao
// redor do texto: o Draw2D preserva o painter's order sozinho, o mesmo motivo que
// eliminou W2/D2D-FLUSH do censo de workarounds).
//
// LAYOUT: monospace fixo (kMonoAdvanceRatio) -> PROPORCIONAL + kerning-se-a-fonte-tiver
// (tabela `kern` classica; confirmado por leitura direta do SFNT que PixelOperatorMono*
// NAO TEM `kern` - kern_px() sempre 0.f, contrato documentado do glintfx, nao um bug).
// Por isso measure_text_width() (i_renderer.hpp) SOBRESCREVE o default monospace com
// Draw2d::measure_text() - medir com a matematica errada desalinha centralizacao/
// alinhamento-a-direita (achado do dossie fontflip-draw2d, secao 0.6).
//
// FONTE: Font2d e carregada UMA VEZ por face (regular/bold) via load_font() e cacheada
// no Impl - o atlas de glifo POR-TAMANHO e criado preguicosamente PELO PROPRIO Draw2d no
// 1o draw_text()/measure_text() daquele tamanho (nao mais um cache por-(face,cell_px)
// mantido aqui, como fazia o D2D-2-SENTINELA original: essa responsabilidade migrou pro
// glintfx). unavailable trava a falha (arquivo ausente/corrompido: nunca reoferece o
// load, mesmo contrato fail-high do ensure_font antigo).
//
// HEADLESS (gl_active == false): espelha Render2dGl3(false) - nenhuma chamada GL nem
// Draw2D, todo draw vira no-op contabilizado, load_texture devolve kInvalidTexture. Prova
// que a cadeia monta e roda sem GPU (CI).

#ifndef GUS_PLATFORM_RENDER2D_RENDER2D_GLINTFX_HPP
#define GUS_PLATFORM_RENDER2D_RENDER2D_GLINTFX_HPP

#include <memory>

#include "gus/platform/render2d/i_renderer.hpp"

namespace gus::platform::render2d {

class Render2dGlintfx : public IRenderer {
public:
    // gl_active: true = ha um contexto GL corrente (a janela ja criou e fez make-current
    // ANTES de construir o renderer); false = headless (CI/smoke sem GPU): tudo no-op.
    // Mesma semantica/assinatura do Render2dGl3 (simetria de API, ver comentario acima).
    explicit Render2dGlintfx(bool gl_active) noexcept;
    ~Render2dGlintfx() override;

    Render2dGlintfx(const Render2dGlintfx&) = delete;
    Render2dGlintfx& operator=(const Render2dGlintfx&) = delete;

    // IRenderer.
    void begin_frame(const gus::core::spatial::Rect& camera_world, int pixel_w,
                     int pixel_h) override;
    void draw_filled_rect(const gus::core::spatial::Rect& world_rect,
                          const DrawColor& color) override;
    void draw_rect_outline(const gus::core::spatial::Rect& world_rect,
                           const DrawColor& color, float thickness_world) override;
    [[nodiscard]] TextureId load_texture(const char* path) override;
    void draw_textured_rect(const gus::core::spatial::Rect& world_rect,
                            TextureId texture, const UvRect& uv,
                            const DrawColor& tint) override;
    [[nodiscard]] ContentBbox texture_content_bbox(
        TextureId texture) const override;
    void draw_text(const char* text, float x, float y, float px_size,
                   const DrawColor& color, bool bold) override;
    [[nodiscard]] float measure_text_width(const char* text, float px_size,
                                           bool bold) override;
    void end_frame() override;

    [[nodiscard]] int last_draw_count() const noexcept { return last_draw_count_; }

    // Simetria de API com Render2dGl3 (ver comentario de cabecalho): permite que
    // app/sdl_window.hpp troque o backend so via um alias de tipo, sem mudar nenhum call
    // site. NO-OP aqui pela MESMA razao do Render2dGl3: o swap real e SDL_GL_SwapWindow,
    // na casca app/ (nem o Draw2D nem este backend sao donos da janela/do swap).
    void set_defer_present(bool defer) noexcept { defer_present_ = defer; }
    [[nodiscard]] bool defer_present() const noexcept { return defer_present_; }
    void present();

private:
    struct Impl;                  // Draw2d + Font2d regular/bold cacheadas; PImpl
    std::unique_ptr<Impl> impl_;  // nullptr em headless (gl_active == false)
    bool gl_active_ = false;

    int draw_count_ = 0;
    int last_draw_count_ = 0;
    bool defer_present_ = false;
};

}  // namespace gus::platform::render2d

#endif  // GUS_PLATFORM_RENDER2D_RENDER2D_GLINTFX_HPP

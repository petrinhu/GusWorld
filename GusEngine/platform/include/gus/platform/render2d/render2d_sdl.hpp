// SPDX-License-Identifier: Apache-2.0
// gus/platform/render2d/render2d_sdl.hpp
//
// Render2dSdl: a implementacao concreta de IRenderer sobre SDL_Renderer (pos
// repivot ADR-008). ESTE e o UNICO ponto que toca a API de render do SDL. O resto
// do jogo fala so com IRenderer (a abstracao que isola o backend - trocar de
// backend e ~1 arquivo, a mesma licao do antigo Render2dRhi).
//
// COMO FUNCIONA: SDL_Renderer ja desenha em PIXELS de tela (origem topo-esquerda,
// +Y para baixo). A projecao mundo->pixel e a tested viewport_transform
// (build_quad_screen, na CPU). begin_frame guarda a camera e limpa; cada draw
// projeta e emite o primitivo SDL correspondente (SDL_RenderFillRect para parede,
// 4 linhas finas para contorno, SDL_RenderTexture para sprite); end_frame
// apresenta (SDL_RenderPresent).
//
// TEXTURAS: load_texture le o PNG via glintfx::decode_png_file (RGBA8 straight; NAO
// somos nos que decodificamos - o stb_image vive DENTRO do glintfx desde a
// STB-IMAGE-PLATFORM, e a variante so-PNG entrou na PNG-DECODE-ADOPT) e cria um SDL_Texture com
// SDL_SCALEMODE_NEAREST (pixel-art crisp) e blend alpha. Cache por caminho. Se o
// arquivo faltar ou o renderer for nulo, devolve kInvalidTexture (o app degrada
// para o contorno). Idempotente por caminho.
//
// MODO HEADLESS: com renderer == nullptr (smoke SDL dummy sem janela), todos os
// draws sao no-op contabilizado e load_texture devolve kInvalidTexture - prova que
// a cadeia monta e roda sem display nem GPU.
//
// Inclui <SDL3/SDL.h> (camada platform/, SDL permitido). O HEADER do IRenderer
// permanece limpo (sem <SDL...>): o app nao ve SDL.

#ifndef GUS_PLATFORM_RENDER2D_RENDER2D_SDL_HPP
#define GUS_PLATFORM_RENDER2D_RENDER2D_SDL_HPP

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include <SDL3/SDL.h>

#include "gus/platform/render2d/font_atlas.hpp"
#include "gus/platform/render2d/i_renderer.hpp"

namespace gus::platform::render2d {

class Render2dSdl : public IRenderer {
public:
    // renderer: o SDL_Renderer (NAO assumido owner; vive enquanto o Render2dSdl
    // viver). nullptr e valido e legal: modo headless (smoke dummy) - tudo vira
    // no-op contabilizado, sem tocar GPU/display.
    explicit Render2dSdl(SDL_Renderer* renderer) noexcept;
    ~Render2dSdl() override;

    Render2dSdl(const Render2dSdl&) = delete;
    Render2dSdl& operator=(const Render2dSdl&) = delete;

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
    void end_frame() override;

    // Numero de primitivos (quads/sprites) emitidos no ultimo frame (debug/teste).
    [[nodiscard]] int last_draw_count() const noexcept { return last_draw_count_; }

    // --- COMPOSICAO COM RmlUi (ADR-009): present diferido -------------------------
    // Por padrao end_frame() apresenta (SDL_RenderPresent) o frame. Quando o HUD do
    // RmlUi vai compor POR CIMA da arena, o present precisa acontecer DEPOIS do RmlUi,
    // nao no fim do desenho da arena. set_defer_present(true) faz end_frame() PARAR de
    // apresentar; o dono do frame (app/) chama compose do RmlUi e depois present()
    // manualmente. Sem isso, a arena daria present antes do HUD (o HUD nao apareceria).
    void set_defer_present(bool defer) noexcept { defer_present_ = defer; }
    [[nodiscard]] bool defer_present() const noexcept { return defer_present_; }

    // Apresenta o frame manualmente (swap). Usado quando defer_present()==true: chamar
    // depois da arena (end_frame sem present) E do compose do RmlUi. No-op headless.
    void present();

private:
    SDL_Renderer* renderer_ = nullptr;  // nao-owner; nullptr = headless

    // Texturas carregadas, indexadas por TextureId (1-based; 0 = invalido). O cache
    // por caminho evita recarregar o mesmo arquivo. As SDL_Texture sao owned aqui.
    std::vector<SDL_Texture*> textures_;
    std::unordered_map<std::string, TextureId> texture_by_path_;

    // Alpha-bbox de cada textura, medido no decode do PNG (load_texture) e cacheado
    // PARALELO a textures_ (mesmo TextureId indexa os dois). Slot 0 = invalido. Serve
    // pra ancorar o sprite pelos PES (texture_content_bbox); ver alpha_bbox.hpp.
    std::vector<ContentBbox> bboxes_;

    // FONTE (D2D-2-SENTINELA): atlas bakeado SOB DEMANDA no TAMANHO REALMENTE DESENHADO,
    // nao mais um unico bake fixo de 16px com downscale na GPU (o defeito original: HUD a
    // 8px saia pontilhado - ver docs/tech/fontflip-visuals/). Uma entrada BakedFontSize por
    // (face, cell_px) INTEIRO (px_size fracionario e arredondado por quantize_cell_px -
    // font_atlas.hpp - so pra escolher a RESOLUCAO do bake; o layout continua float).
    //
    // FontFace agrupa o cache POR TAMANHO de uma face (regular/bold): mapa cell_px ->
    // BakedFontSize, um relogio logico (tick, incrementado a cada ensure_font) pra
    // eviccao LRU quando o cache estoura kMaxCachedFontSizesPerFace, e uma flag
    // "unavailable" (a fonte falhou uma vez - arquivo ausente/stb recusou - entao TODO
    // tamanho falharia igual; evita re-ler o .ttf do disco a cada tamanho novo pedido).
    struct BakedFontSize {
        FontAtlas atlas;                 // bitmap + metricas bakeados NESTE cell_px
        SDL_Texture* texture = nullptr;  // owned; textura RGBA do atlas deste tamanho
        std::uint64_t last_used = 0;     // tick da FontFace no ultimo uso (LRU)
    };
    struct FontFace {
        std::unordered_map<int, BakedFontSize> by_cell_px;  // cache por tamanho
        bool unavailable = false;  // fonte ausente/invalida: nao retenta bake nenhum
        std::uint64_t tick = 0;    // relogio logico (incrementa a cada ensure_font)
    };
    // Garante o bake do TAMANHO pedido (cell_px, ja arredondado por quantize_cell_px).
    // Devolve nullptr se indisponivel (headless sem renderer / fonte ausente). Evicta a
    // entrada LRU do cache antes de inserir se o teto (kMaxCachedFontSizesPerFace) for
    // atingido - nunca cresce sem limite, nunca crasha (fail-high).
    BakedFontSize* ensure_font(bool bold, int cell_px);

    FontFace font_regular_{};
    FontFace font_bold_{};

    gus::core::spatial::Rect camera_{};
    int pixel_w_ = 0;
    int pixel_h_ = 0;
    int draw_count_ = 0;       // acumulado no frame corrente
    int last_draw_count_ = 0;  // congelado em end_frame
    bool defer_present_ = false;  // true = end_frame nao apresenta (HUD RmlUi compoe antes)
};

}  // namespace gus::platform::render2d

#endif  // GUS_PLATFORM_RENDER2D_RENDER2D_SDL_HPP

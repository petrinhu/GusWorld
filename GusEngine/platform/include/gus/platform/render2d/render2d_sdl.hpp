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
// TEXTURAS: load_texture le o PNG via stb_image (RGBA8) e cria um SDL_Texture com
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

#include <string>
#include <unordered_map>
#include <vector>

#include <SDL3/SDL.h>

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
    void end_frame() override;

    // Numero de primitivos (quads/sprites) emitidos no ultimo frame (debug/teste).
    [[nodiscard]] int last_draw_count() const noexcept { return last_draw_count_; }

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

    gus::core::spatial::Rect camera_{};
    int pixel_w_ = 0;
    int pixel_h_ = 0;
    int draw_count_ = 0;       // acumulado no frame corrente
    int last_draw_count_ = 0;  // congelado em end_frame
};

}  // namespace gus::platform::render2d

#endif  // GUS_PLATFORM_RENDER2D_RENDER2D_SDL_HPP

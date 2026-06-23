// gus/platform/render2d/viewport_transform.hpp
//
// Projecao mundo -> tela para a camera ortografica fixa do render2d. Esta e a
// LOGICA do render que NAO precisa de GPU (a matematica de vertice fica testavel
// sem device/backend). O HEADER e limpo (sem <Q...>, sem <SDL...>): so usa Rect de
// core/spatial, pra rodar em Catch2 headless.
//
// CAMERA ORTOGRAFICA FIXA: o CameraView.rect (mundo, de clamp_camera) e a janela
// visivel; mapeia para a viewport. Sem zoom dinamico nem lerp (isso e RF-3).
//
// DOIS ALVOS DE PROJECAO (POCO, ambos testados):
//   - world_to_screen / build_quad_screen -> PIXELS de tela (origem topo-esquerda,
//     +Y para BAIXO), que e o espaco do SDL_Renderer (backend atual, Render2dSdl).
//     Mundo +Y = baixo e tela +Y = baixo: SEM inversao de Y. E a projecao em uso.
//   - world_to_ndc / build_quad_ndc -> cubo NDC [-1,1] (com inversao de Y), que era
//     o espaco do Qt RHI. MANTIDO como utilidade POCO (pos-repivot SDL3, o backend
//     RHI saiu, mas a matematica fica disponivel para um eventual backend de GPU
//     que projete em NDC; e ja testada). Cross-ref: ADR-008.

#ifndef GUS_PLATFORM_RENDER2D_VIEWPORT_TRANSFORM_HPP
#define GUS_PLATFORM_RENDER2D_VIEWPORT_TRANSFORM_HPP

#include "gus/core/spatial/camera_clamp.hpp"  // gus::core::spatial::Rect

namespace gus::platform::render2d {

// --- Projecao em PIXELS de tela (espaco do SDL_Renderer, backend atual) -------

// Ponto em PIXELS de tela. Origem (0,0) = canto superior-esquerdo da viewport;
// +X direita, +Y BAIXO (igual ao mundo e ao SDL_Renderer). Float pra subpixel.
struct ScreenPoint {
    float x = 0.0f;
    float y = 0.0f;
};

// Quad ja em pixels de tela (canto sup-esq + tamanho), pronto pra SDL_FRect.
// Eixos alinhados (sem rotacao): basta a origem e o tamanho. Tamanho negativo
// nunca ocorre para world_rect com w/h >= 0 e camera nao-degenerada.
struct QuadScreen {
    float x = 0.0f;  // canto superior-esquerdo, em pixels
    float y = 0.0f;
    float w = 0.0f;
    float h = 0.0f;
};

// Projeta um ponto de mundo para PIXELS de tela, dada a janela visivel da camera
// (cam_rect, em mundo) e o tamanho da viewport em pixels. SEM inversao de Y (mundo
// e tela tem +Y para baixo). cam_rect degenerado (w<=0/h<=0) nao divide por zero
// (devolve 0 naquele eixo), nunca NaN/inf. Pura, deterministica.
[[nodiscard]] ScreenPoint world_to_screen(float world_x, float world_y,
                                          const gus::core::spatial::Rect& cam_rect,
                                          int viewport_px_w,
                                          int viewport_px_h) noexcept;

// Projeta um retangulo de mundo para um retangulo em pixels de tela (canto + w/h).
// Usa world_to_screen no canto sup-esq e no inf-dir. Pura, deterministica.
[[nodiscard]] QuadScreen build_quad_screen(const gus::core::spatial::Rect& world_rect,
                                           const gus::core::spatial::Rect& cam_rect,
                                           int viewport_px_w,
                                           int viewport_px_h) noexcept;

// --- Projecao em NDC [-1,1] (utilidade POCO; era o espaco do Qt RHI) ----------
// Mantida pos-repivot SDL3 (ADR-008) como matematica testada disponivel para um
// eventual backend de GPU em NDC. NAO usada pelo Render2dSdl.

// Ponto em Normalized Device Coordinates (cubo [-1,1]). z implicito = 0.
struct NdcPoint {
    float x = 0.0f;
    float y = 0.0f;
};

// Quad (retangulo) ja em NDC. Ordem dos cantos:
//   [0] superior-esquerdo, [1] superior-direito, [2] inferior-direito,
//   [3] inferior-esquerdo (sentido horario partindo do topo-esq). Um backend de
// GPU monta dois triangulos (0,1,2) e (0,2,3) a partir disso.
struct QuadNdc {
    NdcPoint corners[4];
};

// Projeta um ponto de mundo para NDC (com inversao de Y: topo do mundo -> +1).
// cam_rect degenerado nao divide por zero. Pura, deterministica.
[[nodiscard]] NdcPoint world_to_ndc(float world_x, float world_y,
                                    const gus::core::spatial::Rect& cam_rect) noexcept;

// Projeta um retangulo de mundo para os 4 cantos NDC (ver QuadNdc). Usa
// world_to_ndc nos cantos. Pura, deterministica.
[[nodiscard]] QuadNdc build_quad_ndc(const gus::core::spatial::Rect& world_rect,
                                     const gus::core::spatial::Rect& cam_rect) noexcept;

}  // namespace gus::platform::render2d

#endif  // GUS_PLATFORM_RENDER2D_VIEWPORT_TRANSFORM_HPP

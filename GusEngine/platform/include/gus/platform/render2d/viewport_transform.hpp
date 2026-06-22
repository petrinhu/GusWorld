// gus/platform/render2d/viewport_transform.hpp
//
// Projecao mundo -> NDC para a camera ortografica fixa do render2d (M1). Esta e
// a LOGICA do render que NAO precisa de GPU (decisao do brief: a matematica de
// vertice fica testavel sem device). O HEADER e limpo (sem <Q...>): so usa Rect
// de core/spatial, pra rodar em Qt Test headless.
//
// CAMERA ORTOGRAFICA FIXA: o CameraView.rect (mundo, de clamp_camera) e a janela
// visivel; mapeia para o cubo NDC do Qt RHI ([-1,1] x [-1,1]). Sem zoom dinamico
// nem lerp (isso e RF-3). O backend QRhi consome estes NDC direto no vertex buffer
// (z fixo, sem profundidade: 2D puro).
//
// INVERSAO DE Y: mundo +Y = baixo (tile_grid.hpp); NDC +Y = cima. A projecao
// inverte Y (topo do mundo visivel -> +1; fundo -> -1). Ver o teste.

#ifndef GUS_PLATFORM_RENDER2D_VIEWPORT_TRANSFORM_HPP
#define GUS_PLATFORM_RENDER2D_VIEWPORT_TRANSFORM_HPP

#include "gus/core/spatial/camera_clamp.hpp"  // gus::core::spatial::Rect

namespace gus::platform::render2d {

// Ponto em Normalized Device Coordinates (cubo [-1,1] do Qt RHI). z implicito = 0.
struct NdcPoint {
    float x = 0.0f;
    float y = 0.0f;
};

// Quad (retangulo) ja em NDC, pronto pro vertex buffer. Ordem dos cantos:
//   [0] superior-esquerdo, [1] superior-direito, [2] inferior-direito,
//   [3] inferior-esquerdo (sentido horario partindo do topo-esq). O backend monta
// dois triangulos (0,1,2) e (0,2,3) a partir disso.
struct QuadNdc {
    NdcPoint corners[4];
};

// Projeta um ponto de mundo para NDC, dada a janela visivel da camera (em mundo).
// cam_rect com w<=0 ou h<=0 (estado degenerado) e tratado sem divisao por zero
// (devolve 0 naquele eixo), nunca NaN/inf. Pura, deterministica.
[[nodiscard]] NdcPoint world_to_ndc(float world_x, float world_y,
                                    const gus::core::spatial::Rect& cam_rect) noexcept;

// Projeta um retangulo de mundo para os 4 cantos NDC (ver QuadNdc). Usa
// world_to_ndc nos cantos. Pura, deterministica.
[[nodiscard]] QuadNdc build_quad_ndc(const gus::core::spatial::Rect& world_rect,
                                     const gus::core::spatial::Rect& cam_rect) noexcept;

}  // namespace gus::platform::render2d

#endif  // GUS_PLATFORM_RENDER2D_VIEWPORT_TRANSFORM_HPP

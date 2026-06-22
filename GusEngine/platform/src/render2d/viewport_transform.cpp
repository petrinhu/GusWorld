// gus/platform/src/render2d/viewport_transform.cpp
//
// Implementacao da projecao mundo -> NDC (M1). Ver header. Travado por
// platform/tests/viewport_transform_test.cpp (TEST-FIRST).

#include "gus/platform/render2d/viewport_transform.hpp"

namespace gus::platform::render2d {

NdcPoint world_to_ndc(float world_x, float world_y,
                      const gus::core::spatial::Rect& cam_rect) noexcept {
    NdcPoint p;

    // X: normaliza [cam.x, cam.x+cam.w] -> [-1, +1].
    if (cam_rect.w > 0.0f) {
        p.x = (world_x - cam_rect.x) / cam_rect.w * 2.0f - 1.0f;
    } else {
        p.x = 0.0f;  // camera degenerada: nao divide por zero
    }

    // Y: normaliza [cam.y, cam.y+cam.h] -> [+1, -1] (INVERTE: mundo +Y e baixo,
    // NDC +Y e cima). y == cam.y (topo) -> +1; y == cam.y+cam.h (fundo) -> -1.
    if (cam_rect.h > 0.0f) {
        p.y = 1.0f - (world_y - cam_rect.y) / cam_rect.h * 2.0f;
    } else {
        p.y = 0.0f;
    }

    return p;
}

QuadNdc build_quad_ndc(const gus::core::spatial::Rect& world_rect,
                       const gus::core::spatial::Rect& cam_rect) noexcept {
    const float left = world_rect.x;
    const float top = world_rect.y;
    const float right = world_rect.x + world_rect.w;
    const float bottom = world_rect.y + world_rect.h;

    QuadNdc q;
    q.corners[0] = world_to_ndc(left, top, cam_rect);      // superior-esquerdo
    q.corners[1] = world_to_ndc(right, top, cam_rect);     // superior-direito
    q.corners[2] = world_to_ndc(right, bottom, cam_rect);  // inferior-direito
    q.corners[3] = world_to_ndc(left, bottom, cam_rect);   // inferior-esquerdo
    return q;
}

}  // namespace gus::platform::render2d

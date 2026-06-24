// gus/core/spatial/camera_clamp.cpp
// Clamp de camera ao mapa (M4). Ver camera_clamp.hpp para o contrato (clamp nas
// 4 bordas; mapa menor que viewport => centraliza). Matematica pura.

#include "gus/core/spatial/camera_clamp.hpp"

namespace gus::core::spatial {

namespace {

// Resolve o CENTRO da camera em um eixo.
//   half      = metade do viewport nesse eixo
//   target    = centro desejado nesse eixo
//   map_size  = tamanho do mapa nesse eixo (mapa vai de 0 a map_size)
// Se o mapa cabe inteiro no viewport (map_size <= 2*half), centraliza no meio do
// mapa. Senao, clampa o centro ao intervalo [half, map_size - half] para o
// retangulo nunca sair do mapa.
float clamp_axis(float target, float half, float map_size) noexcept {
    const float viewport = 2.0f * half;
    if (map_size <= viewport) {
        return map_size * 0.5f;  // mapa menor que a visao: centraliza
    }
    const float lo = half;
    const float hi = map_size - half;
    if (target < lo) {
        return lo;
    }
    if (target > hi) {
        return hi;
    }
    return target;
}

}  // namespace

float world_span_from_pixels(float pixels, float px_per_world_unit) noexcept {
    if (px_per_world_unit <= 0.0f) {
        return pixels;  // guarda: zoom invalido => 1 px == 1 unidade (sem divisao ruim)
    }
    return pixels / px_per_world_unit;
}

CameraView clamp_camera(Vec2 target_center, float viewport_w, float viewport_h,
                        float map_w, float map_h) noexcept {
    const float half_w = viewport_w * 0.5f;
    const float half_h = viewport_h * 0.5f;

    const float cx = clamp_axis(target_center.x, half_w, map_w);
    const float cy = clamp_axis(target_center.y, half_h, map_h);

    CameraView view;
    view.center = Vec2{cx, cy};
    view.rect = Rect{cx - half_w, cy - half_h, viewport_w, viewport_h};
    return view;
}

}  // namespace gus::core::spatial

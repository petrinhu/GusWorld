// gus/core/spatial/camera_clamp.hpp
//
// Clamp de camera ao mapa (M4) - POCO C++ puro, ZERO Qt, ZERO I/O. Matematica
// pura: alvo cru -> clamp nas 4 bordas -> retangulo de visao.
//
// ESCOPO: SO o clamp. NAO ha feel de camera aqui (zoom dinamico, suavizacao/lerp,
// deadzone, follow com atraso) - isso e o RF-3 (brainstorm pendente do lider). A
// porta fica aberta: o alvo entra cru; quem quiser suavizar passa um alvo ja
// suavizado.
//
// CONTRATO:
//   - target_center: centro desejado da camera (ex. centro do personagem), em
//     unidades de mundo.
//   - viewport_w/h: tamanho da visao em unidades de mundo.
//   - map_w/h: tamanho do mapa em unidades de mundo; o mapa vai de (0,0) a
//     (map_w, map_h). Eixo +X direita, +Y baixo.
//   - A visao NUNCA mostra fora do mapa: o retangulo e clampado nas 4 bordas.
//   - Se o mapa for MENOR que o viewport num eixo (map < viewport), CENTRALIZA
//     nesse eixo (centro = meio do mapa); nesse caso o retangulo se estende para
//     fora do mapa simetricamente (nao da para preencher o viewport so com
//     mundo). Decisao documentada: centralizar > prender numa borda.
//
// CameraView devolve o centro final e o retangulo (canto superior-esquerdo + w/h).

#ifndef GUS_CORE_SPATIAL_CAMERA_CLAMP_HPP
#define GUS_CORE_SPATIAL_CAMERA_CLAMP_HPP

namespace gus::core::spatial {

struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;
};

struct Rect {
    float x = 0.0f;  // canto superior-esquerdo
    float y = 0.0f;
    float w = 0.0f;
    float h = 0.0f;
};

struct CameraView {
    Vec2 center;
    Rect rect;
};

// Calcula a visao da camera clampada ao mapa. Matematica pura, deterministica.
CameraView clamp_camera(Vec2 target_center, float viewport_w, float viewport_h,
                        float map_w, float map_h) noexcept;

// ZOOM (px por unidade de mundo) -> tamanho da visao EM UNIDADES DE MUNDO, dado o
// tamanho da viewport em PIXELS. Esta e a peca que faltava: a camera mostra
// (pixels / px_por_unidade) unidades de mundo, e nao "pixels unidades" (que fazia o
// mapa de 60x40 unidades virar um retangulo de 60x40 px no centro da tela).
//   px_per_world_unit > 1  => cada unidade de mundo ocupa mais de 1 px = APROXIMA
//                             (mostra MENOS mundo, tudo maior na tela).
//   px_per_world_unit <= 0 => guarda: devolve os proprios pixels (zoom 1, sem
//                             divisao por zero/negativo). Pura, deterministica.
[[nodiscard]] float world_span_from_pixels(float pixels,
                                           float px_per_world_unit) noexcept;

}  // namespace gus::core::spatial

#endif  // GUS_CORE_SPATIAL_CAMERA_CLAMP_HPP

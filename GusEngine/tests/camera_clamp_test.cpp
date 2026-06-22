// GusEngine/tests/camera_clamp_test.cpp
//
// Spec executavel (Catch2 v3) do clamp de camera ao mapa (M4). TEST-FIRST.
//
// CONTRATO exercitado:
//   - clamp_camera(target_center, viewport_w, viewport_h, map_w, map_h)
//     -> CameraView { center{x,y}, rect{x,y,w,h} (canto superior-esquerdo) };
//   - a visao NUNCA mostra fora do mapa: clamp nas 4 bordas;
//   - se o mapa for MENOR que o viewport num eixo, CENTRALIZA nesse eixo
//     (centro do mapa), em vez de prender numa borda;
//   - mapa em unidades de mundo de (0,0) a (map_w, map_h); viewport em unidades
//     de mundo. SEM feel (zoom/lerp/deadzone): alvo cru -> clamp -> retangulo.
//
// NAO ha suavizacao/follow aqui (isso e o RF-3, brainstorm pendente do lider).

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "gus/core/spatial/camera_clamp.hpp"

using gus::core::spatial::CameraView;
using gus::core::spatial::clamp_camera;
using gus::core::spatial::Vec2;
using Catch::Matchers::WithinAbs;

namespace {
constexpr float kEps = 1e-4f;
}

TEST_CASE("clamp_camera: alvo no meio de mapa grande nao move", "[core][spatial][camera]") {
    // Mapa 200x200, viewport 100x80, alvo no centro.
    CameraView v = clamp_camera(Vec2{100.0f, 100.0f}, 100.0f, 80.0f, 200.0f, 200.0f);
    REQUIRE_THAT(v.center.x, WithinAbs(100.0f, kEps));
    REQUIRE_THAT(v.center.y, WithinAbs(100.0f, kEps));
    REQUIRE_THAT(v.rect.x, WithinAbs(50.0f, kEps));   // 100 - 100/2
    REQUIRE_THAT(v.rect.y, WithinAbs(60.0f, kEps));   // 100 - 80/2
    REQUIRE_THAT(v.rect.w, WithinAbs(100.0f, kEps));
    REQUIRE_THAT(v.rect.h, WithinAbs(80.0f, kEps));
}

TEST_CASE("clamp_camera: borda esquerda prende o retangulo em x=0",
          "[core][spatial][camera]") {
    // Alvo perto da borda esquerda: o retangulo nao pode comecar antes de 0.
    CameraView v = clamp_camera(Vec2{10.0f, 100.0f}, 100.0f, 80.0f, 200.0f, 200.0f);
    REQUIRE_THAT(v.rect.x, WithinAbs(0.0f, kEps));
    REQUIRE_THAT(v.center.x, WithinAbs(50.0f, kEps));  // centro = metade do viewport
}

TEST_CASE("clamp_camera: borda direita prende o retangulo no fim do mapa",
          "[core][spatial][camera]") {
    CameraView v = clamp_camera(Vec2{195.0f, 100.0f}, 100.0f, 80.0f, 200.0f, 200.0f);
    REQUIRE_THAT(v.rect.x, WithinAbs(100.0f, kEps));   // 200 - 100
    REQUIRE_THAT(v.center.x, WithinAbs(150.0f, kEps)); // 200 - 100/2
}

TEST_CASE("clamp_camera: borda topo prende o retangulo em y=0",
          "[core][spatial][camera]") {
    CameraView v = clamp_camera(Vec2{100.0f, 5.0f}, 100.0f, 80.0f, 200.0f, 200.0f);
    REQUIRE_THAT(v.rect.y, WithinAbs(0.0f, kEps));
    REQUIRE_THAT(v.center.y, WithinAbs(40.0f, kEps));  // 80/2
}

TEST_CASE("clamp_camera: borda baixo prende o retangulo no fim do mapa",
          "[core][spatial][camera]") {
    CameraView v = clamp_camera(Vec2{100.0f, 198.0f}, 100.0f, 80.0f, 200.0f, 200.0f);
    REQUIRE_THAT(v.rect.y, WithinAbs(120.0f, kEps));   // 200 - 80
    REQUIRE_THAT(v.center.y, WithinAbs(160.0f, kEps)); // 200 - 80/2
}

TEST_CASE("clamp_camera: canto inferior-direito prende nos dois eixos",
          "[core][spatial][camera]") {
    CameraView v = clamp_camera(Vec2{999.0f, 999.0f}, 100.0f, 80.0f, 200.0f, 200.0f);
    REQUIRE_THAT(v.rect.x, WithinAbs(100.0f, kEps));
    REQUIRE_THAT(v.rect.y, WithinAbs(120.0f, kEps));
}

TEST_CASE("clamp_camera: mapa menor que viewport em X centraliza nesse eixo",
          "[core][spatial][camera]") {
    // Mapa 60 de largura, viewport 100: o mundo cabe inteiro -> centraliza no
    // meio do mapa (30), independente do alvo.
    CameraView v = clamp_camera(Vec2{0.0f, 100.0f}, 100.0f, 80.0f, 60.0f, 200.0f);
    REQUIRE_THAT(v.center.x, WithinAbs(30.0f, kEps));   // meio do mapa
    REQUIRE_THAT(v.rect.x, WithinAbs(-20.0f, kEps));    // 30 - 50 (mapa centrado)
    // Eixo Y continua com clamp normal (mapa maior que viewport).
    REQUIRE_THAT(v.center.y, WithinAbs(100.0f, kEps));
}

TEST_CASE("clamp_camera: mapa menor que viewport em Y centraliza nesse eixo",
          "[core][spatial][camera]") {
    CameraView v = clamp_camera(Vec2{100.0f, 999.0f}, 100.0f, 80.0f, 200.0f, 40.0f);
    REQUIRE_THAT(v.center.y, WithinAbs(20.0f, kEps));   // meio do mapa
    REQUIRE_THAT(v.center.x, WithinAbs(100.0f, kEps));  // X clampa normal
}

TEST_CASE("clamp_camera: mapa menor nos dois eixos centraliza nos dois",
          "[core][spatial][camera]") {
    CameraView v = clamp_camera(Vec2{0.0f, 0.0f}, 100.0f, 80.0f, 60.0f, 40.0f);
    REQUIRE_THAT(v.center.x, WithinAbs(30.0f, kEps));
    REQUIRE_THAT(v.center.y, WithinAbs(20.0f, kEps));
}

// GusEngine/platform/tests/viewport_transform_test.cpp
//
// Catch2 da matematica de projecao do render2d (POCO, sem GPU/SDL). TEST-FIRST.
// Pos repivot ADR-008 cobre os DOIS alvos:
//   - world_to_screen / build_quad_screen -> PIXELS (espaco do SDL_Renderer, EM
//     USO). Mundo +Y baixo e tela +Y baixo: SEM inversao de Y.
//   - world_to_ndc / build_quad_ndc -> NDC [-1,1] com inversao de Y (utilidade
//     POCO mantida; era o espaco do Qt RHI).

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <cmath>

#include "gus/core/spatial/camera_clamp.hpp"  // Rect (mundo)
#include "gus/platform/render2d/viewport_transform.hpp"

using Catch::Matchers::WithinAbs;
using gus::core::spatial::Rect;
using gus::platform::render2d::build_quad_ndc;
using gus::platform::render2d::build_quad_screen;
using gus::platform::render2d::NdcPoint;
using gus::platform::render2d::QuadNdc;
using gus::platform::render2d::QuadScreen;
using gus::platform::render2d::ScreenPoint;
using gus::platform::render2d::world_to_ndc;
using gus::platform::render2d::world_to_screen;

namespace {
constexpr double kEps = 1e-4;
}

// --- PIXELS de tela (em uso pelo Render2dSdl) --------------------------------

TEST_CASE("world_to_screen: canto sup-esq da camera vira (0,0)", "[viewport]") {
    Rect cam{100.0f, 50.0f, 200.0f, 200.0f};
    ScreenPoint p = world_to_screen(100.0f, 50.0f, cam, 800, 600);
    CHECK_THAT(p.x, WithinAbs(0.0, kEps));
    CHECK_THAT(p.y, WithinAbs(0.0, kEps));  // SEM inversao: topo do mundo -> y=0
}

TEST_CASE("world_to_screen: canto inf-dir vira (w,h) em pixels", "[viewport]") {
    Rect cam{100.0f, 50.0f, 200.0f, 200.0f};
    ScreenPoint p = world_to_screen(300.0f, 250.0f, cam, 800, 600);
    CHECK_THAT(p.x, WithinAbs(800.0, kEps));
    CHECK_THAT(p.y, WithinAbs(600.0, kEps));  // fundo do mundo -> base da tela
}

TEST_CASE("world_to_screen: centro da camera vira o centro da tela", "[viewport]") {
    Rect cam{0.0f, 0.0f, 100.0f, 100.0f};
    ScreenPoint p = world_to_screen(50.0f, 50.0f, cam, 640, 480);
    CHECK_THAT(p.x, WithinAbs(320.0, kEps));
    CHECK_THAT(p.y, WithinAbs(240.0, kEps));
}

TEST_CASE("build_quad_screen: retangulo de mundo vira rect em pixels", "[viewport]") {
    Rect cam{0.0f, 0.0f, 100.0f, 100.0f};
    Rect world{10.0f, 10.0f, 10.0f, 20.0f};
    QuadScreen q = build_quad_screen(world, cam, 100, 100);
    // 1 unidade de mundo == 1 pixel (viewport 100 sobre camera 100).
    CHECK_THAT(q.x, WithinAbs(10.0, kEps));
    CHECK_THAT(q.y, WithinAbs(10.0, kEps));
    CHECK_THAT(q.w, WithinAbs(10.0, kEps));
    CHECK_THAT(q.h, WithinAbs(20.0, kEps));
}

TEST_CASE("world_to_screen: camera degenerada nao gera NaN/inf", "[viewport]") {
    Rect cam{0.0f, 0.0f, 0.0f, 0.0f};
    ScreenPoint p = world_to_screen(5.0f, 5.0f, cam, 100, 100);
    CHECK(std::isfinite(p.x));
    CHECK(std::isfinite(p.y));
}

// --- NDC [-1,1] (utilidade POCO mantida) -------------------------------------

TEST_CASE("world_to_ndc: centro vira origem NDC", "[viewport][ndc]") {
    Rect cam{100.0f, 50.0f, 200.0f, 200.0f};
    NdcPoint c = world_to_ndc(200.0f, 150.0f, cam);
    CHECK_THAT(c.x, WithinAbs(0.0, kEps));
    CHECK_THAT(c.y, WithinAbs(0.0, kEps));
}

TEST_CASE("world_to_ndc: canto sup-esq vira (-1,+1) com Y invertido",
          "[viewport][ndc]") {
    Rect cam{100.0f, 50.0f, 200.0f, 200.0f};
    NdcPoint p = world_to_ndc(100.0f, 50.0f, cam);
    CHECK_THAT(p.x, WithinAbs(-1.0, kEps));
    CHECK_THAT(p.y, WithinAbs(1.0, kEps));  // topo do mundo -> +1 (invertido)
}

TEST_CASE("world_to_ndc: canto inf-dir vira (+1,-1)", "[viewport][ndc]") {
    Rect cam{100.0f, 50.0f, 200.0f, 200.0f};
    NdcPoint p = world_to_ndc(300.0f, 250.0f, cam);
    CHECK_THAT(p.x, WithinAbs(1.0, kEps));
    CHECK_THAT(p.y, WithinAbs(-1.0, kEps));
}

TEST_CASE("build_quad_ndc: 4 cantos na ordem documentada", "[viewport][ndc]") {
    Rect cam{0.0f, 0.0f, 100.0f, 100.0f};
    Rect world{10.0f, 10.0f, 10.0f, 20.0f};
    QuadNdc q = build_quad_ndc(world, cam);
    // ndc_x: 10 -> -0.8 ; 20 -> -0.6 ; ndc_y(10)->0.8 ; ndc_y(30)->0.4
    CHECK_THAT(q.corners[0].x, WithinAbs(-0.8, kEps));
    CHECK_THAT(q.corners[0].y, WithinAbs(0.8, kEps));
    CHECK_THAT(q.corners[2].x, WithinAbs(-0.6, kEps));
    CHECK_THAT(q.corners[2].y, WithinAbs(0.4, kEps));
}

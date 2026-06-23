// GusEngine/app/tests/sprite_anchor_test.cpp
//
// Catch2 da ANCORAGEM do sprite pelos pes reais (sprite_anchor.hpp), POCO puro.
// M1-BUG.SUL: descontar a margem inferior TRANSPARENTE do PNG pra o PE REAL do
// desenho coincidir com a base da AABB de colisao (sem isso, no SUL sobra um vao
// entre o pe e a parede de baixo). Tudo aritmetica - sem GPU, sem I/O.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "gus/app/screens/sprite_anchor.hpp"

using Catch::Matchers::WithinAbs;
using gus::app::screens::bottom_margin_fraction;
using gus::app::screens::Direction;
using gus::app::screens::FootInset;
using gus::app::screens::sprite_top_y;

namespace {
constexpr double kEps = 1e-4;
}

TEST_CASE("sprite_anchor: fracao da margem inferior (caso medido do Caua)",
          "[sprite_anchor]") {
    // south.png: canvas 68, conteudo termina em y=57 => margem 11 px.
    REQUIRE_THAT(bottom_margin_fraction(11, 68), WithinAbs(11.0 / 68.0, kEps));
    // east/west: margem 9 px.
    REQUIRE_THAT(bottom_margin_fraction(9, 68), WithinAbs(9.0 / 68.0, kEps));
}

TEST_CASE("sprite_anchor: margens degeneradas caem no anchor legado (0)",
          "[sprite_anchor]") {
    REQUIRE_THAT(bottom_margin_fraction(0, 68), WithinAbs(0.0, kEps));   // sem margem
    REQUIRE_THAT(bottom_margin_fraction(-3, 68), WithinAbs(0.0, kEps));  // negativa
    REQUIRE_THAT(bottom_margin_fraction(11, 0), WithinAbs(0.0, kEps));   // canvas 0
    REQUIRE_THAT(bottom_margin_fraction(80, 68), WithinAbs(1.0, kEps));  // tudo vazio
}

TEST_CASE("sprite_anchor: sem margem e sem ajuste = comportamento legado",
          "[sprite_anchor]") {
    // base do canvas na base da AABB: sy = aabb_bottom - sprite_h.
    const float aabb_bottom = 44.0f;
    const float sprite_h = 44.0f;  // 2.75 tiles * 16
    const float sy = sprite_top_y(aabb_bottom, sprite_h, 0.0f, 0.0f);
    REQUIRE_THAT(sy, WithinAbs(aabb_bottom - sprite_h, kEps));
    // A base do CANVAS coincide com a base da AABB.
    REQUIRE_THAT(sy + sprite_h, WithinAbs(aabb_bottom, kEps));
}

TEST_CASE("sprite_anchor: a margem inferior SOBE a base do canvas pra colar o pe",
          "[sprite_anchor]") {
    const float aabb_bottom = 44.0f;
    const float sprite_h = 44.0f;
    const float frac = 11.0f / 68.0f;  // south
    const float sy = sprite_top_y(aabb_bottom, sprite_h, frac, 0.0f);
    // A base do CANVAS fica ABAIXO da base da AABB exatamente pela margem em mundo,
    // de modo que o PE REAL (canvas - margem) caia EM CIMA da base da AABB.
    const float canvas_bottom = sy + sprite_h;
    const float foot_world = frac * sprite_h;
    REQUIRE_THAT(canvas_bottom - foot_world, WithinAbs(aabb_bottom, kEps));
    // E o canvas desceu (canvas_bottom > aabb_bottom): o desenho avanca pra parede.
    REQUIRE(canvas_bottom > aabb_bottom);
}

TEST_CASE("sprite_anchor: ajuste manual soma por cima do automatico",
          "[sprite_anchor]") {
    const float aabb_bottom = 44.0f;
    const float sprite_h = 44.0f;
    const float frac = 11.0f / 68.0f;
    const float sy0 = sprite_top_y(aabb_bottom, sprite_h, frac, 0.0f);
    const float sy1 = sprite_top_y(aabb_bottom, sprite_h, frac, 8.0f);
    REQUIRE_THAT(sy1 - sy0, WithinAbs(8.0, kEps));  // +0.5 tile desce 8 u
}

TEST_CASE("sprite_anchor: FootInset.for_direction le por direcao e protege limite",
          "[sprite_anchor]") {
    FootInset fi;
    fi.bottom_fraction[static_cast<int>(Direction::South)] = 0.16f;
    fi.bottom_fraction[static_cast<int>(Direction::East)] = 0.13f;
    REQUIRE_THAT(fi.for_direction(Direction::South), WithinAbs(0.16, kEps));
    REQUIRE_THAT(fi.for_direction(Direction::East), WithinAbs(0.13, kEps));
    REQUIRE_THAT(fi.for_direction(Direction::North), WithinAbs(0.0, kEps));
}

// GusEngine/tests/anim_clock_test.cpp
//
// Catch2 do AnimClock (core/anim), POCO puro sem SDL/IO/GPU. TEST-FIRST.
// Cobre o que o VIEWER de animacao consome: avanco de quadro ciclico por TEMPO,
// ajuste de fps, troca de animacao (reset), e a robustez a dt grande/negativo.

#include <catch2/catch_test_macros.hpp>

#include "gus/core/anim/anim_clock.hpp"

using gus::core::anim::AnimClock;

TEST_CASE("anim_clock: comeca no quadro 0", "[anim_clock]") {
    AnimClock c(7, 10.0f);
    REQUIRE(c.frame() == 0);
    REQUIRE(c.frame_count() == 7);
    REQUIRE(c.fps() == 10.0f);
}

TEST_CASE("anim_clock: avanca 1 quadro a cada 1/fps", "[anim_clock]") {
    AnimClock c(4, 10.0f);  // 0.1s por quadro
    c.advance(0.05f);
    REQUIRE(c.frame() == 0);  // metade do passo: ainda no 0
    c.advance(0.05f);
    REQUIRE(c.frame() == 1);  // cruzou 0.1s -> 1 quadro
    c.advance(0.1f);
    REQUIRE(c.frame() == 2);
}

TEST_CASE("anim_clock: faz wrap ciclico em frame_count", "[anim_clock]") {
    AnimClock c(3, 10.0f);
    c.advance(0.1f);  // -> 1
    c.advance(0.1f);  // -> 2
    c.advance(0.1f);  // -> 0 (wrap)
    REQUIRE(c.frame() == 0);
}

TEST_CASE("anim_clock: dt grande avanca varios quadros de uma vez", "[anim_clock]") {
    AnimClock c(10, 10.0f);
    c.advance(0.35f);  // 3.5 passos -> +3 quadros
    REQUIRE(c.frame() == 3);
}

TEST_CASE("anim_clock: dt negativo ou zero nao avanca", "[anim_clock]") {
    AnimClock c(5, 10.0f);
    c.advance(-1.0f);
    c.advance(0.0f);
    REQUIRE(c.frame() == 0);
    REQUIRE(c.accumulated() == 0.0f);
}

TEST_CASE("anim_clock: set_frame_count reseta a animacao", "[anim_clock]") {
    AnimClock c(4, 10.0f);
    c.advance(0.25f);  // -> quadro 2, com sobra acumulada
    REQUIRE(c.frame() == 2);
    c.set_frame_count(7);
    REQUIRE(c.frame_count() == 7);
    REQUIRE(c.frame() == 0);
    REQUIRE(c.accumulated() == 0.0f);
}

TEST_CASE("anim_clock: nudge_fps respeita o piso e nao para a anim", "[anim_clock]") {
    AnimClock c(4, 5.0f);
    c.nudge_fps(5.0f);
    REQUIRE(c.fps() == 10.0f);
    c.nudge_fps(-1000.0f);  // tentaria zerar/negativar
    REQUIRE(c.fps() >= AnimClock::kMinFps);
}

TEST_CASE("anim_clock: fps maior troca quadro mais rapido", "[anim_clock]") {
    AnimClock c(10, 20.0f);  // 0.05s por quadro
    c.advance(0.05f);
    REQUIRE(c.frame() == 1);
}

TEST_CASE("anim_clock: saneia argumentos invalidos no construtor", "[anim_clock]") {
    AnimClock zero_frames(0, 10.0f);
    REQUIRE(zero_frames.frame_count() >= 1);
    AnimClock zero_fps(4, 0.0f);
    REQUIRE(zero_fps.fps() >= AnimClock::kMinFps);
}

TEST_CASE("anim_clock: reset volta ao quadro 0 sem trocar de anim", "[anim_clock]") {
    AnimClock c(6, 10.0f);
    c.advance(0.3f);  // -> 3
    REQUIRE(c.frame() == 3);
    c.reset();
    REQUIRE(c.frame() == 0);
    REQUIRE(c.frame_count() == 6);
}

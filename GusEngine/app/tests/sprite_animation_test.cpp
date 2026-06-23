// GusEngine/app/tests/sprite_animation_test.cpp
//
// Catch2 da LOGICA de animacao de locomocao (app/screens/sprite_animation), POCO
// sem Qt/SDL/GPU. TEST-FIRST. Cobre as duas decisoes que o render so consome:
//   (1) vetor de movimento -> direcao cardinal (com regra de diagonal e parado);
//   (2) distancia percorrida -> quadro de walk ciclico; parado -> neutro; run com
//       passada mais longa (troca menos vezes que walk na mesma distancia).
//
// CANON: docs/design/mecanicas/locomotion.md.

#include <catch2/catch_test_macros.hpp>

#include "gus/app/screens/sprite_animation.hpp"

using gus::app::screens::Direction;
using gus::app::screens::direction_from_move;
using gus::app::screens::kWalkFrameCount;
using gus::app::screens::WalkCycle;

TEST_CASE("direction: cardinais puras mapeiam direto", "[sprite_anim]") {
    REQUIRE(direction_from_move(0, 1, Direction::North) == Direction::South);
    REQUIRE(direction_from_move(0, -1, Direction::South) == Direction::North);
    REQUIRE(direction_from_move(1, 0, Direction::West) == Direction::East);
    REQUIRE(direction_from_move(-1, 0, Direction::East) == Direction::West);
}

TEST_CASE("direction: parado mantem a direcao anterior", "[sprite_anim]") {
    REQUIRE(direction_from_move(0, 0, Direction::East) == Direction::East);
    REQUIRE(direction_from_move(0, 0, Direction::North) == Direction::North);
}

TEST_CASE("direction: diagonal horizontal vence", "[sprite_anim]") {
    REQUIRE(direction_from_move(1, 1, Direction::North) == Direction::East);
    REQUIRE(direction_from_move(1, -1, Direction::South) == Direction::East);
    REQUIRE(direction_from_move(-1, 1, Direction::North) == Direction::West);
    REQUIRE(direction_from_move(-1, -1, Direction::South) == Direction::West);
}

TEST_CASE("walk: parado fica neutro", "[sprite_anim]") {
    WalkCycle c;
    c.advance(0.0f, false);
    REQUIRE_FALSE(c.is_moving());
    REQUIRE(c.current_frame() == WalkCycle::kNeutralFrame);
}

TEST_CASE("walk: anda sai do neutro no primeiro passo", "[sprite_anim]") {
    WalkCycle c;
    c.advance(2.0f, false);  // andou um pouco (< 8 px)
    REQUIRE(c.is_moving());
    REQUIRE(c.current_frame() == 0);
}

TEST_CASE("walk: troca de quadro a cada passo de distancia", "[sprite_anim]") {
    WalkCycle c(WalkCycle::Config{/*walk*/ 8.0f, /*run*/ 11.0f});
    c.advance(8.0f, false);
    REQUIRE(c.current_frame() == 1);
    c.advance(8.0f, false);
    REQUIRE(c.current_frame() == 2);
    c.advance(8.0f, false);
    REQUIRE(c.current_frame() == 3);
    c.advance(8.0f, false);
    REQUIRE(c.current_frame() == 0);  // ciclico
}

TEST_CASE("walk: quadro e sempre valido", "[sprite_anim]") {
    WalkCycle c;
    for (int i = 0; i < 100; ++i) {
        c.advance(3.3f, false);
        const int f = c.current_frame();
        REQUIRE(f >= 0);
        REQUIRE(f < kWalkFrameCount);
    }
}

TEST_CASE("walk: parar no meio volta ao neutro", "[sprite_anim]") {
    WalkCycle c;
    c.advance(10.0f, false);
    REQUIRE(c.is_moving());
    c.advance(0.0f, false);
    REQUIRE_FALSE(c.is_moving());
    REQUIRE(c.current_frame() == WalkCycle::kNeutralFrame);
}

TEST_CASE("walk: run troca menos que walk na mesma distancia", "[sprite_anim]") {
    WalkCycle walk(WalkCycle::Config{8.0f, 11.0f});
    WalkCycle run(WalkCycle::Config{8.0f, 11.0f});
    auto count_changes = [](WalkCycle& c, bool running) {
        int changes = 0;
        int last = c.current_frame();
        for (int i = 0; i < 33; ++i) {
            c.advance(1.0f, running);
            if (c.current_frame() != last) {
                ++changes;
                last = c.current_frame();
            }
        }
        return changes;
    };
    const int walk_changes = count_changes(walk, false);
    const int run_changes = count_changes(run, true);
    REQUIRE(run_changes < walk_changes);
}

TEST_CASE("walk: reset volta ao neutro", "[sprite_anim]") {
    WalkCycle c;
    c.advance(20.0f, false);
    REQUIRE(c.is_moving());
    c.reset();
    REQUIRE_FALSE(c.is_moving());
    REQUIRE(c.current_frame() == WalkCycle::kNeutralFrame);
    REQUIRE(c.accumulated() == 0.0f);
}

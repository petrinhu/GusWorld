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

TEST_CASE("direction: diagonal horizontal vence (politica legada/default explicito)",
          "[sprite_anim]") {
    using gus::app::screens::DiagonalFacing;
    // A sobrecarga sem politica mantem o comportamento legado (horizontal vence).
    REQUIRE(direction_from_move(1, 1, Direction::North) == Direction::East);
    REQUIRE(direction_from_move(1, -1, Direction::South) == Direction::East);
    REQUIRE(direction_from_move(-1, 1, Direction::North) == Direction::West);
    REQUIRE(direction_from_move(-1, -1, Direction::South) == Direction::West);
    // E a politica HorizontalWins explicita faz o mesmo.
    REQUIRE(direction_from_move(1, 1, Direction::North,
                                DiagonalFacing::HorizontalWins) == Direction::East);
}

TEST_CASE("direction: politica VerticalWins faz Norte/Sul ganhar na diagonal",
          "[sprite_anim]") {
    using gus::app::screens::DiagonalFacing;
    const auto P = DiagonalFacing::VerticalWins;
    REQUIRE(direction_from_move(1, 1, Direction::East, P) == Direction::South);
    REQUIRE(direction_from_move(1, -1, Direction::East, P) == Direction::North);
    REQUIRE(direction_from_move(-1, 1, Direction::West, P) == Direction::South);
    REQUIRE(direction_from_move(-1, -1, Direction::West, P) == Direction::North);
    // Cardinal puro nao muda com a politica.
    REQUIRE(direction_from_move(1, 0, Direction::North, P) == Direction::East);
    REQUIRE(direction_from_move(0, -1, Direction::South, P) == Direction::North);
}

TEST_CASE("direction: politica LastAxisWins vira para o eixo recem-acionado",
          "[sprite_anim]") {
    using gus::app::screens::DiagonalFacing;
    const auto P = DiagonalFacing::LastAxisWins;
    // Andando pro LADO (prev horizontal) e acionando vertical: vira pro VERTICAL
    // (o eixo novo). Esse e o pedido do lider: andar pro lado + W/S -> vira N/S.
    REQUIRE(direction_from_move(1, 1, Direction::East, P) == Direction::South);
    REQUIRE(direction_from_move(1, -1, Direction::East, P) == Direction::North);
    REQUIRE(direction_from_move(-1, -1, Direction::West, P) == Direction::North);
    // Andando pra CIMA/BAIXO (prev vertical) e acionando horizontal: vira pro LADO.
    REQUIRE(direction_from_move(1, 1, Direction::South, P) == Direction::East);
    REQUIRE(direction_from_move(-1, 1, Direction::South, P) == Direction::West);
    // Se prev ja era um dos eixos da diagonal corrente, mantem (sem flicker):
    //   prev=East, diagonal (1,1) tem East como um dos eixos -> NAO ha eixo novo
    //   forcado; cai pro tie-break (horizontal) e segue East.
    REQUIRE(direction_from_move(1, 0, Direction::East, P) == Direction::East);
}

TEST_CASE("direction: LastAxisWins com memoria de input nao OSCILA na diagonal "
          "sustentada",
          "[sprite_anim]") {
    using gus::app::screens::DiagonalFacing;
    const auto P = DiagonalFacing::LastAxisWins;
    // BUG do flicker: derivando do facing ANTERIOR, uma diagonal SUSTENTADA fazia o
    // boneco alternar (East->North->East...). Com a memoria de input (dx_prev,dy_prev),
    // segurar a MESMA diagonal por varios ticks tem que ESTABILIZAR numa direcao so.
    // Cenario: ja andava pro lado (dx_prev=1, dy_prev=0) e acionou W (dy=-1): vira N.
    int dx_prev = 1, dy_prev = 0;
    Direction f = Direction::East;  // estava indo pro lado
    Direction first = direction_from_move(1, -1, dx_prev, dy_prev, f, P);
    REQUIRE(first == Direction::North);  // virou pro eixo recem-acionado (vertical)
    // Agora as duas teclas seguem apertadas (input sustentado, nada novo): NAO oscila.
    f = first;
    dx_prev = 1;
    dy_prev = -1;
    for (int i = 0; i < 10; ++i) {
        Direction g = direction_from_move(1, -1, dx_prev, dy_prev, f, P);
        REQUIRE(g == Direction::North);  // estavel: continua Norte, sem piscar
        f = g;
    }
}

TEST_CASE("direction: LastAxisWins lado-depois-vertical vira para o vertical",
          "[sprite_anim]") {
    using gus::app::screens::DiagonalFacing;
    const auto P = DiagonalFacing::LastAxisWins;
    // So andava pro lado (dx_prev != 0, dy_prev == 0) e adicionou vertical agora.
    REQUIRE(direction_from_move(1, 1, 1, 0, Direction::East, P) == Direction::South);
    REQUIRE(direction_from_move(1, -1, 1, 0, Direction::East, P) == Direction::North);
    REQUIRE(direction_from_move(-1, 1, -1, 0, Direction::West, P) == Direction::South);
    REQUIRE(direction_from_move(-1, -1, -1, 0, Direction::West, P) == Direction::North);
}

TEST_CASE("direction: LastAxisWins cima-depois-horizontal vira para o horizontal",
          "[sprite_anim]") {
    using gus::app::screens::DiagonalFacing;
    const auto P = DiagonalFacing::LastAxisWins;
    // So andava pra cima/baixo (dy_prev != 0, dx_prev == 0) e adicionou horizontal.
    REQUIRE(direction_from_move(1, 1, 0, 1, Direction::South, P) == Direction::East);
    REQUIRE(direction_from_move(-1, 1, 0, 1, Direction::South, P) == Direction::West);
    REQUIRE(direction_from_move(1, -1, 0, -1, Direction::North, P) == Direction::East);
    REQUIRE(direction_from_move(-1, -1, 0, -1, Direction::North, P) == Direction::West);
}

TEST_CASE("direction: LastAxisWins ambos sustentados mantem prev se for eixo da "
          "diagonal",
          "[sprite_anim]") {
    using gus::app::screens::DiagonalFacing;
    const auto P = DiagonalFacing::LastAxisWins;
    // Nenhum eixo recem-acionado (dx_prev e dy_prev ja != 0). prev=North e um dos
    // eixos da diagonal (1,-1) -> mantem North (estavel). prev=East idem -> East.
    REQUIRE(direction_from_move(1, -1, 1, -1, Direction::North, P) == Direction::North);
    REQUIRE(direction_from_move(1, -1, 1, -1, Direction::East, P) == Direction::East);
    // Se prev NAO e nenhum dos eixos da diagonal corrente (ex.: prev=West numa
    // diagonal (1,-1) = Leste+Norte): fallback = vertical da diagonal.
    REQUIRE(direction_from_move(1, -1, 1, -1, Direction::West, P) == Direction::North);
    REQUIRE(direction_from_move(1, 1, 1, 1, Direction::North, P) == Direction::South);
}

TEST_CASE("direction: nova sobrecarga preserva cardinal puro e parado", "[sprite_anim]") {
    using gus::app::screens::DiagonalFacing;
    const auto P = DiagonalFacing::LastAxisWins;
    // Cardinal puro: a propria direcao, independente de dx_prev/dy_prev e da politica.
    REQUIRE(direction_from_move(1, 0, 0, 0, Direction::North, P) == Direction::East);
    REQUIRE(direction_from_move(0, -1, 0, 0, Direction::South, P) == Direction::North);
    // Parado: mantem prev.
    REQUIRE(direction_from_move(0, 0, 1, 1, Direction::West, P) == Direction::West);
    // VerticalWins e HorizontalWins inalteradas na nova sobrecarga (ignoram memoria).
    REQUIRE(direction_from_move(1, 1, 0, 0, Direction::East,
                                DiagonalFacing::VerticalWins) == Direction::South);
    REQUIRE(direction_from_move(1, 1, 0, 0, Direction::North,
                                DiagonalFacing::HorizontalWins) == Direction::East);
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

// --- GENERALIZACAO N quadros (Gus tem 7 por direcao) -------------------------

TEST_CASE("walk: frame_count default e kWalkFrameCount (compat Caua)", "[sprite_anim]") {
    WalkCycle c;
    REQUIRE(c.frame_count() == kWalkFrameCount);
}

TEST_CASE("walk: frame_count saneia entradas invalidas", "[sprite_anim]") {
    WalkCycle zero(WalkCycle::Config{}, 0);
    REQUIRE(zero.frame_count() == kWalkFrameCount);  // < 1 cai no default
    WalkCycle neg(WalkCycle::Config{}, -3);
    REQUIRE(neg.frame_count() == kWalkFrameCount);
    WalkCycle ok(WalkCycle::Config{}, 7);
    REQUIRE(ok.frame_count() == 7);
}

TEST_CASE("walk: cicla os 7 quadros do Gus e volta a 0", "[sprite_anim]") {
    // Gus: 7 quadros por direcao. O wrap tem que ser em 7 (nao no kWalkFrameCount=4).
    WalkCycle c(WalkCycle::Config{/*walk*/ 8.0f, /*run*/ 11.0f}, /*frames*/ 7);
    for (int expected = 1; expected < 7; ++expected) {
        c.advance(8.0f, false);
        REQUIRE(c.current_frame() == expected);  // 1,2,3,4,5,6
    }
    c.advance(8.0f, false);
    REQUIRE(c.current_frame() == 0);  // ciclico no 7o passo, nao no 4o
}

TEST_CASE("walk: 7 quadros - quadro sempre valido no intervalo", "[sprite_anim]") {
    WalkCycle c(WalkCycle::Config{8.0f, 11.0f}, 7);
    for (int i = 0; i < 200; ++i) {
        c.advance(3.7f, false);
        const int f = c.current_frame();
        REQUIRE(f >= 0);
        REQUIRE(f < 7);
    }
}

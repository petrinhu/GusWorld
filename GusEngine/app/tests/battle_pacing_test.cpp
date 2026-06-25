// GusEngine/app/tests/battle_pacing_test.cpp
//
// Catch2 (headless) do DIRETOR DE PACING (M5, incremento 6, D8/D10). Prova, SEM SDL: a
// maquina de estados do ritmo (Intro -> EsperaInput / Animando / EsperaDelay), o avanco
// por dt, o pulo por tecla, e os sinais que a BattleScene consome pra resolver UM turno
// de cada vez. POCO puro: o diretor NAO toca a FSM nem SDL - so dita QUANDO o proximo
// passo pode acontecer.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "gus/app/screens/battle_pacing.hpp"

using Catch::Matchers::WithinAbs;
using gus::app::screens::kPacingIntroSeconds;
using gus::app::screens::kPacingStepDelaySeconds;
using gus::app::screens::PacingDirector;
using gus::app::screens::PacingState;

TEST_CASE("pacing: comeca em INTRO parada (D10: ninguem agiu)", "[battle_pacing]") {
    PacingDirector d;
    REQUIRE(d.state() == PacingState::Intro);
    // Na intro, nao libera passo (a arena monta, mostra BATALHA!, ninguem age).
    REQUIRE_FALSE(d.ready_to_step());
}

TEST_CASE("pacing: a intro termina por tempo e libera o 1o passo", "[battle_pacing]") {
    PacingDirector d;
    d.tick(kPacingIntroSeconds * 0.5f);
    REQUIRE(d.state() == PacingState::Intro);  // ainda na intro
    REQUIRE_FALSE(d.ready_to_step());
    d.tick(kPacingIntroSeconds);  // passa do tempo de intro
    REQUIRE(d.ready_to_step());   // libera o 1o turno
}

TEST_CASE("pacing: skip pula a intro na hora (1a tecla)", "[battle_pacing]") {
    PacingDirector d;
    REQUIRE_FALSE(d.ready_to_step());
    d.skip();
    REQUIRE(d.ready_to_step());  // a tecla acelerou: libera ja
}

TEST_CASE("pacing: ao animar um turno de inimigo, entra em EsperaDelay e segura ~0.8s",
          "[battle_pacing]") {
    PacingDirector d;
    d.skip();  // sai da intro
    REQUIRE(d.ready_to_step());
    // A cena resolveu 1 turno de inimigo e avisa o diretor (begin_enemy_step): entra em
    // EsperaDelay (o numero/log ficam na tela pelo delay).
    d.begin_enemy_step();
    REQUIRE(d.state() == PacingState::WaitingDelay);
    REQUIRE_FALSE(d.ready_to_step());  // segura
    d.tick(kPacingStepDelaySeconds * 0.5f);
    REQUIRE_FALSE(d.ready_to_step());  // ainda no delay
    d.tick(kPacingStepDelaySeconds);   // passa o delay
    REQUIRE(d.ready_to_step());         // libera o proximo turno
}

TEST_CASE("pacing: skip encurta o delay do passo (jogador acelera)", "[battle_pacing]") {
    PacingDirector d;
    d.skip();
    d.begin_enemy_step();
    REQUIRE_FALSE(d.ready_to_step());
    d.skip();  // tecla durante o delay
    REQUIRE(d.ready_to_step());  // pula pro proximo na hora
}

TEST_CASE("pacing: turno do JOGADOR entra em EsperaInput (nao auto-avanca)",
          "[battle_pacing]") {
    PacingDirector d;
    d.skip();
    // A cena detectou que o ator ativo e o jogador e avisa (begin_player_turn).
    d.begin_player_turn();
    REQUIRE(d.state() == PacingState::WaitingPlayerInput);
    REQUIRE_FALSE(d.ready_to_step());   // espera o menu; o tempo NAO libera passo
    d.tick(100.0f);                     // muito tempo
    REQUIRE_FALSE(d.ready_to_step());   // continua esperando o input (D9: sua vez)
    REQUIRE(d.waiting_player_input());
    // skip NAO pula o turno do jogador (so o delay/intro): continua esperando.
    d.skip();
    REQUIRE(d.waiting_player_input());
}

TEST_CASE("pacing: o jogador confirma a acao e o ritmo retoma", "[battle_pacing]") {
    PacingDirector d;
    d.skip();
    d.begin_player_turn();
    REQUIRE(d.waiting_player_input());
    // A cena resolveu a acao do jogador e avisa (player_acted): entra em EsperaDelay (o
    // resultado do golpe do jogador tambem respeita o ritmo) e depois libera.
    d.player_acted();
    REQUIRE(d.state() == PacingState::WaitingDelay);
    d.tick(kPacingStepDelaySeconds + 0.01f);
    REQUIRE(d.ready_to_step());
}

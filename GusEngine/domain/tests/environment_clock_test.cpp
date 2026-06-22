// environment_clock_test.cpp
//
// Spec executavel (Catch2 v3) da roda temporal de periodo (secao 18.3):
//   Dia(5) -> Crepusculo(2) -> Noite(5) -> Aurora(2) -> Dia...
// Portada de engine/foundation/turn_combat/EnvironmentClock.cs (origem canonica) e dos
// casos de engine/tests/turn_combat/environments/EnvironmentClockTests.cs (xUnit = SPEC).
// Deterministica, sem RNG. POCO puro, ZERO Qt, headless. Marco M5 (chunk 3).
//
// Cobre contagem de turnos, telegraph N turnos antes, projecao e wrap. Paridade 1:1 com
// o C# (mapeamento de excecoes: ArgumentException -> std::invalid_argument;
// ArgumentOutOfRangeException -> std::out_of_range).
//
// Cross-ref: engine/foundation/turn_combat/EnvironmentClock.cs;
//            engine/tests/turn_combat/environments/EnvironmentClockTests.cs;
//            docs/design/mecanicas/combat.md secao 18.1 (telegraph)/18.3 (roda).

#include <catch2/catch_test_macros.hpp>

#include <stdexcept>
#include <vector>

#include "gus/domain/combat/environment_clock.hpp"
#include "gus/domain/combat/environment_enums.hpp"

using namespace gus::domain::combat;

// ---- Ciclo 5/2/5/2 - contagem de turnos correta -----------------------------------

TEST_CASE("environment_clock: comeca em Dia com 5 turnos", "[domain][combat][environment]") {
    const EnvironmentClock clock;
    REQUIRE(clock.current_phase() == EnvironmentId::Dia);
    REQUIRE(clock.phase_duration() == 5);
    REQUIRE(clock.turns_remaining() == 5);
    REQUIRE(clock.next_phase() == EnvironmentId::Crepusculo);
}

TEST_CASE("environment_clock: roda completa segue a ordem canonica 5/2/5/2",
          "[domain][combat][environment]") {
    const EnvironmentClock clock;
    const auto projecao = clock.project(14);  // 5 + 2 + 5 + 2 = 14 = um ciclo completo

    std::vector<EnvironmentId> esperado;
    for (int i = 0; i < 5; ++i) esperado.push_back(EnvironmentId::Dia);
    for (int i = 0; i < 2; ++i) esperado.push_back(EnvironmentId::Crepusculo);
    for (int i = 0; i < 5; ++i) esperado.push_back(EnvironmentId::Noite);
    for (int i = 0; i < 2; ++i) esperado.push_back(EnvironmentId::Aurora);

    REQUIRE(projecao == esperado);
}

TEST_CASE("environment_clock: projecao de um ciclo e meio faz wrap para Dia",
          "[domain][combat][environment]") {
    const EnvironmentClock clock;
    const auto projecao = clock.project(15);  // 14 + 1 => volta pro Dia
    REQUIRE(projecao[14] == EnvironmentId::Dia);
}

TEST_CASE("environment_clock: advance troca de fase apos esgotar a duracao",
          "[domain][combat][environment]") {
    EnvironmentClock clock;

    bool trocou = false;
    for (int i = 0; i < 5; ++i) trocou = clock.advance();

    REQUIRE(trocou);
    REQUIRE(clock.current_phase() == EnvironmentId::Crepusculo);
    REQUIRE(clock.turns_remaining() == 2);
}

TEST_CASE("environment_clock: advance dentro da fase nao troca",
          "[domain][combat][environment]") {
    EnvironmentClock clock;
    for (int i = 0; i < 4; ++i) REQUIRE_FALSE(clock.advance());
    REQUIRE(clock.current_phase() == EnvironmentId::Dia);
    REQUIRE(clock.turns_remaining() == 1);
}

TEST_CASE("environment_clock: ciclo completo volta ao Dia", "[domain][combat][environment]") {
    EnvironmentClock clock;
    for (int i = 0; i < 14; ++i) clock.advance();  // 5+2+5+2
    REQUIRE(clock.current_phase() == EnvironmentId::Dia);
    REQUIRE(clock.turns_remaining() == 5);
}

// ---- Telegraph N turnos antes (secao 18.1) - proposta N=2 -------------------------

TEST_CASE("environment_clock: telegraph dispara quando faltam N turnos ou menos",
          "[domain][combat][environment]") {
    EnvironmentClock clock(EnvironmentId::Dia, /*telegraph_turns=*/2);

    REQUIRE_FALSE(clock.transition_telegraphed());  // restam 5
    clock.advance();
    REQUIRE_FALSE(clock.transition_telegraphed());  // restam 4
    clock.advance();
    REQUIRE_FALSE(clock.transition_telegraphed());  // restam 3
    clock.advance();
    REQUIRE(clock.transition_telegraphed());  // restam 2 -> telegrafa
    clock.advance();
    REQUIRE(clock.transition_telegraphed());  // restam 1
}

TEST_CASE("environment_clock: fase curta esta sempre telegrafada com N=2",
          "[domain][combat][environment]") {
    const EnvironmentClock clock(EnvironmentId::Crepusculo, /*telegraph_turns=*/2);
    REQUIRE(clock.transition_telegraphed());
}

TEST_CASE("environment_clock: telegraph zero nunca avisa antes",
          "[domain][combat][environment]") {
    EnvironmentClock clock(EnvironmentId::Dia, /*telegraph_turns=*/0);
    for (int i = 0; i < 4; ++i) {
        REQUIRE_FALSE(clock.transition_telegraphed());
        clock.advance();
    }
}

// ---- Construcao / validacao -------------------------------------------------------

TEST_CASE("environment_clock: pode comecar em qualquer fase de periodo",
          "[domain][combat][environment]") {
    const EnvironmentClock clock(EnvironmentId::Noite);
    REQUIRE(clock.current_phase() == EnvironmentId::Noite);
    REQUIRE(clock.phase_duration() == 5);
    REQUIRE(clock.next_phase() == EnvironmentId::Aurora);
}

TEST_CASE("environment_clock: comecar em ambiente que nao e periodo lanca",
          "[domain][combat][environment]") {
    REQUIRE_THROWS_AS(EnvironmentClock(EnvironmentId::Chuva), std::invalid_argument);
}

TEST_CASE("environment_clock: telegraph negativo lanca", "[domain][combat][environment]") {
    REQUIRE_THROWS_AS(EnvironmentClock(EnvironmentId::Dia, /*telegraph_turns=*/-1),
                      std::out_of_range);
}

TEST_CASE("environment_clock: project com turns negativo lanca",
          "[domain][combat][environment]") {
    const EnvironmentClock clock;
    REQUIRE_THROWS_AS(clock.project(-1), std::out_of_range);
}

// ---- Current le do catalogo -------------------------------------------------------

TEST_CASE("environment_clock: current devolve o ambiente da fase ativa",
          "[domain][combat][environment]") {
    const EnvironmentClock clock(EnvironmentId::Noite);
    REQUIRE(clock.current().id == EnvironmentId::Noite);
    REQUIRE(clock.current().period_duration == 5);
}

// ---- Vetor caotico Perlin (utilitario, secao 18.4) --------------------------------

TEST_CASE("environment_clock: is_chaotic_vector so e true para AnomaliaPerlin",
          "[domain][combat][environment]") {
    REQUIRE(EnvironmentClock::is_chaotic_vector(EnvironmentId::AnomaliaPerlin));
    REQUIRE_FALSE(EnvironmentClock::is_chaotic_vector(EnvironmentId::Chuva));
    REQUIRE(EnvironmentClock::any_chaotic({EnvironmentId::Chuva, EnvironmentId::AnomaliaPerlin}));
    REQUIRE_FALSE(EnvironmentClock::any_chaotic({EnvironmentId::Chuva, EnvironmentId::Noite}));
}

// random_source_test.cpp
//
// Spec executavel (Catch2 v3) da porta de RNG injetavel do combate
// (gus/domain/combat/random_source.hpp) e do seu duplo deterministico FixedRandom
// (fixed_random.hpp), portados de engine/tests/turn_combat/FixedRandom.cs.
//
// PORTE FIEL da porta que o C# ja injeta: a CombatStateMachine recebe um IRandomSource
// por injecao (NUNCA RNG global no dominio). A superficie espelha o que a FSM consome
// de System.Random: NextDouble() -> next_double(); Next(maxValue) -> next(max_value).
// A semente real (data+hora+ms, ADR-006 item 5) e injetada na fronteira app/ depois,
// NAO neste porte (dominio puro/deterministico).
//
// Cross-ref: engine/tests/turn_combat/FixedRandom.cs; docs/design/mecanicas/combat.md secao 11.

#include <catch2/catch_test_macros.hpp>

#include "fixed_random.hpp"
#include "gus/domain/combat/random_source.hpp"

using gus::domain::combat::IRandomSource;
using gus::domain::tests::FixedRandom;

TEST_CASE("random_source: FixedRandom devolve o next_double cravado",
          "[domain][combat][rng]") {
    FixedRandom meio;  // default 0.5 (sem variancia)
    REQUIRE(meio.next_double() == 0.5);

    FixedRandom topo(1.0);
    REQUIRE(topo.next_double() == 1.0);

    FixedRandom fundo(0.0);
    REQUIRE(fundo.next_double() == 0.0);
}

TEST_CASE("random_source: FixedRandom.next clampa no intervalo semiaberto",
          "[domain][combat][rng]") {
    // next_int 0 => sempre 0 (crita quando crit_chance>0).
    FixedRandom zero(0.5, 0);
    REQUIRE(zero.next(100) == 0);

    // next_int dentro do range volta o proprio valor.
    FixedRandom cinquenta(0.5, 50);
    REQUIRE(cinquenta.next(100) == 50);

    // next_int >= max_value satura em max_value-1 (espelha Math.Min do C#).
    FixedRandom grande(0.5, 9999);
    REQUIRE(grande.next(100) == 99);

    // max_value <= 0 => 0 (defensivo, identico ao C#).
    FixedRandom q(0.5, 7);
    REQUIRE(q.next(0) == 0);
    REQUIRE(q.next(-3) == 0);
}

TEST_CASE("random_source: a porta e polimorfica (injetavel via ponteiro base)",
          "[domain][combat][rng]") {
    FixedRandom impl(0.25, 3);
    IRandomSource& port = impl;
    REQUIRE(port.next_double() == 0.25);
    REQUIRE(port.next(10) == 3);
}

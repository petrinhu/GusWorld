// combat_constants_test.cpp
//
// Spec executavel (Catch2 v3) das constantes canonicas do combate, portadas de
// engine/foundation/turn_combat/CombatConstants.cs (origem canonica). POCO puro,
// ZERO Qt, headless. Marco M5 (chunk 1: fundacoes do combate).
//
// Sao parametros macro (secao 2/5/6/11/18). Travar os valores aqui evita magic
// numbers divergentes quando o resto do motor (formula de dano, roda, ambientes)
// for portado nos chunks 2-4.
//
// Cross-ref: engine/foundation/turn_combat/CombatConstants.cs;
//            docs/design/mecanicas/combat.md secao 2/5/6/11/18; ADR-006.

#include <catch2/catch_test_macros.hpp>

#include "gus/domain/combat/combat_constants.hpp"

namespace cc = gus::domain::combat::combat_constants;

// ---- Parametros macro de recurso (secao 2/5) --------------------------------------

TEST_CASE("combat_constants: parametros de recurso (secao 2/5)",
          "[domain][combat][constants]") {
    REQUIRE(cc::kBaseApPerTurn == 3);
    REQUIRE(cc::kBaseMana == 2);
    REQUIRE(cc::kManaCap == 8);
    REQUIRE(cc::kMinPartySize == 1);
    REQUIRE(cc::kMaxPartySize == 4);
    REQUIRE(cc::kMinDamage == 1);
}

// ---- Multiplicadores da roda de fraqueza (secao 6) --------------------------------

TEST_CASE("combat_constants: multiplicadores da roda de fraqueza (secao 6)",
          "[domain][combat][constants]") {
    REQUIRE(cc::kMultFraco == 1.5f);
    REQUIRE(cc::kMultNeutro == 1.0f);
    REQUIRE(cc::kMultResistente == 0.66f);
    REQUIRE(cc::kMultImune == 0.0f);
}

// ---- multAmbiente: caps + default (secao 11) --------------------------------------

TEST_CASE("combat_constants: caps de multAmbiente (secao 11)",
          "[domain][combat][constants]") {
    REQUIRE(cc::kMultAmbienteCapMin == 0.44f);
    REQUIRE(cc::kMultAmbienteCapMax == 2.25f);
    REQUIRE(cc::kMultAmbienteDefault == 1.0f);
}

// ---- Faixas de multiplicador de familia de ambiente (secao 18) --------------------

TEST_CASE("combat_constants: multiplicadores de ambiente por familia (secao 18)",
          "[domain][combat][constants]") {
    REQUIRE(cc::kEnvMultPico == 1.5f);
    REQUIRE(cc::kEnvMultAlto == 1.3f);
    REQUIRE(cc::kEnvMultHostilLeve == 0.85f);
    REQUIRE(cc::kEnvMultHostilForte == 0.66f);
}

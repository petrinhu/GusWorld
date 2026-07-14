// weakness_wheel_test.cpp
//
// Spec executavel (Catch2 v3) da roda de fraqueza deterministica, portada de
// engine/foundation/turn_combat/WeaknessWheel.cs (origem canonica) e dos casos de
// engine/tests/turn_combat/WeaknessWheelTests.cs (xUnit = SPEC). POCO puro, ZERO Qt,
// headless. Marco M5 (chunk 2: atores/filas do combate).
//
// Ciclo (forte contra a seguinte):
//   Eletrico -> Cinetico -> Criptografico -> Sonico -> Bioquimico -> Eletrico
// Paridade de comportamento 1:1 com o C#.
//
// Cross-ref: engine/foundation/turn_combat/WeaknessWheel.cs;
//            engine/tests/turn_combat/WeaknessWheelTests.cs;
//            docs/design/mecanicas/combat.md secao 6/11/17.

#include <catch2/catch_test_macros.hpp>

#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/combat/weakness_wheel.hpp"

using namespace gus::domain::combat;

// ---- Forte contra (1.5) -- sentido da seta (Forte_contra_retorna_1_5) -------------

TEST_CASE("weakness_wheel: atacante forte contra alvo retorna 1.5",
          "[domain][combat][weakness]") {
    REQUIRE(WeaknessWheel::multiplier(CardFamily::Eletrico, CardFamily::Cinetico) == 1.5f);
    REQUIRE(WeaknessWheel::multiplier(CardFamily::Cinetico, CardFamily::Criptografico) == 1.5f);
    REQUIRE(WeaknessWheel::multiplier(CardFamily::Criptografico, CardFamily::Sonico) == 1.5f);
    REQUIRE(WeaknessWheel::multiplier(CardFamily::Sonico, CardFamily::Bioquimico) == 1.5f);
    REQUIRE(WeaknessWheel::multiplier(CardFamily::Bioquimico, CardFamily::Eletrico) == 1.5f);
}

// ---- Resistente (0.66) -- sentido inverso (Resistente_retorna_0_66) ---------------

TEST_CASE("weakness_wheel: alvo forte contra atacante retorna 0.66",
          "[domain][combat][weakness]") {
    REQUIRE(WeaknessWheel::multiplier(CardFamily::Cinetico, CardFamily::Eletrico) == 0.66f);
    REQUIRE(WeaknessWheel::multiplier(CardFamily::Criptografico, CardFamily::Cinetico) == 0.66f);
    REQUIRE(WeaknessWheel::multiplier(CardFamily::Sonico, CardFamily::Criptografico) == 0.66f);
    REQUIRE(WeaknessWheel::multiplier(CardFamily::Bioquimico, CardFamily::Sonico) == 0.66f);
    REQUIRE(WeaknessWheel::multiplier(CardFamily::Eletrico, CardFamily::Bioquimico) == 0.66f);
}

// ---- Neutro (1.0) -- mesma familia ou par sem relacao (Neutro_retorna_1_0) --------

TEST_CASE("weakness_wheel: mesma familia ou par sem relacao retorna 1.0",
          "[domain][combat][weakness]") {
    // Mesma familia.
    REQUIRE(WeaknessWheel::multiplier(CardFamily::Eletrico, CardFamily::Eletrico) == 1.0f);
    REQUIRE(WeaknessWheel::multiplier(CardFamily::Cinetico, CardFamily::Cinetico) == 1.0f);
    REQUIRE(WeaknessWheel::multiplier(CardFamily::Sonico, CardFamily::Sonico) == 1.0f);
    // Pares sem relacao na roda.
    REQUIRE(WeaknessWheel::multiplier(CardFamily::Eletrico, CardFamily::Criptografico) == 1.0f);
    REQUIRE(WeaknessWheel::multiplier(CardFamily::Eletrico, CardFamily::Sonico) == 1.0f);
    REQUIRE(WeaknessWheel::multiplier(CardFamily::Cinetico, CardFamily::Sonico) == 1.0f);
    REQUIRE(WeaknessWheel::multiplier(CardFamily::Cinetico, CardFamily::Bioquimico) == 1.0f);
}

// ---- TierFor (Classifica_tier_corretamente) ---------------------------------------

TEST_CASE("weakness_wheel: classifica tier corretamente",
          "[domain][combat][weakness]") {
    REQUIRE(WeaknessWheel::tier_for(CardFamily::Eletrico, CardFamily::Cinetico) == WeaknessTier::Fraco);
    REQUIRE(WeaknessWheel::tier_for(CardFamily::Cinetico, CardFamily::Eletrico) == WeaknessTier::Resistente);
    REQUIRE(WeaknessWheel::tier_for(CardFamily::Eletrico, CardFamily::Eletrico) == WeaknessTier::Neutro);
}

// ---- Universal (PS-R1): fora da roda, curto-circuito Neutro/1.0 nos dois sentidos ----
//
// Decisao do criador 2026-07-14 (docs/design/mecanicas/combat.md secao 20): Universal
// e a familia das cartas que NAO competem na roda de fraqueza. multFraqueza SEMPRE 1.0,
// SEM Fraco/Resistente/Imune, em qualquer combinacao com qualquer uma das 5 familias
// (e consigo mesma). So-cartas: templates de personagem/inimigo continuam na roda de 5
// (character_template_test.cpp / enemy_template_test rejeitam ordinal 5).

TEST_CASE("weakness_wheel: Universal como atacante e sempre Neutro/1.0",
          "[domain][combat][weakness][universal]") {
    REQUIRE(WeaknessWheel::tier_for(CardFamily::Universal, CardFamily::Eletrico) == WeaknessTier::Neutro);
    REQUIRE(WeaknessWheel::tier_for(CardFamily::Universal, CardFamily::Bioquimico) == WeaknessTier::Neutro);
    REQUIRE(WeaknessWheel::tier_for(CardFamily::Universal, CardFamily::Sonico) == WeaknessTier::Neutro);
    REQUIRE(WeaknessWheel::tier_for(CardFamily::Universal, CardFamily::Cinetico) == WeaknessTier::Neutro);
    REQUIRE(WeaknessWheel::tier_for(CardFamily::Universal, CardFamily::Criptografico) == WeaknessTier::Neutro);
    REQUIRE(WeaknessWheel::tier_for(CardFamily::Universal, CardFamily::Universal) == WeaknessTier::Neutro);

    REQUIRE(WeaknessWheel::multiplier(CardFamily::Universal, CardFamily::Eletrico) == 1.0f);
    REQUIRE(WeaknessWheel::multiplier(CardFamily::Universal, CardFamily::Bioquimico) == 1.0f);
    REQUIRE(WeaknessWheel::multiplier(CardFamily::Universal, CardFamily::Sonico) == 1.0f);
    REQUIRE(WeaknessWheel::multiplier(CardFamily::Universal, CardFamily::Cinetico) == 1.0f);
    REQUIRE(WeaknessWheel::multiplier(CardFamily::Universal, CardFamily::Criptografico) == 1.0f);
    REQUIRE(WeaknessWheel::multiplier(CardFamily::Universal, CardFamily::Universal) == 1.0f);
}

TEST_CASE("weakness_wheel: Universal como alvo e sempre Neutro/1.0",
          "[domain][combat][weakness][universal]") {
    REQUIRE(WeaknessWheel::tier_for(CardFamily::Eletrico, CardFamily::Universal) == WeaknessTier::Neutro);
    REQUIRE(WeaknessWheel::tier_for(CardFamily::Bioquimico, CardFamily::Universal) == WeaknessTier::Neutro);
    REQUIRE(WeaknessWheel::tier_for(CardFamily::Sonico, CardFamily::Universal) == WeaknessTier::Neutro);
    REQUIRE(WeaknessWheel::tier_for(CardFamily::Cinetico, CardFamily::Universal) == WeaknessTier::Neutro);
    REQUIRE(WeaknessWheel::tier_for(CardFamily::Criptografico, CardFamily::Universal) == WeaknessTier::Neutro);

    REQUIRE(WeaknessWheel::multiplier(CardFamily::Eletrico, CardFamily::Universal) == 1.0f);
    REQUIRE(WeaknessWheel::multiplier(CardFamily::Bioquimico, CardFamily::Universal) == 1.0f);
    REQUIRE(WeaknessWheel::multiplier(CardFamily::Sonico, CardFamily::Universal) == 1.0f);
    REQUIRE(WeaknessWheel::multiplier(CardFamily::Cinetico, CardFamily::Universal) == 1.0f);
    REQUIRE(WeaknessWheel::multiplier(CardFamily::Criptografico, CardFamily::Universal) == 1.0f);
}

// environment_modifier_test.cpp
//
// Spec executavel (Catch2 v3) do record de ambiente, portada de
// engine/foundation/turn_combat/EnvironmentModifier.cs (origem canonica). Cobre o
// FacilitatedStatus.ApplyTo (canal 2, secao 18.1), o HardwareHook (canal 4) e o
// EnvironmentModifier.MultFor (canal 1, secao 18). POCO puro, ZERO Qt, headless.
// Marco M5 (chunk 3: subsistema de ambiente do combate).
//
// Paridade de comportamento 1:1 com o C# e com os casos espelhados de
// EnvironmentStatusFacilitationTests.cs.
//
// Cross-ref: engine/foundation/turn_combat/EnvironmentModifier.cs;
//            docs/design/mecanicas/combat.md secao 18 (canais)/9 (status).

#include <catch2/catch_test_macros.hpp>

#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/combat/combat_records.hpp"
#include "gus/domain/combat/environment_enums.hpp"
#include "gus/domain/combat/environment_modifier.hpp"

using namespace gus::domain::combat;

// ---- FacilitatedStatus.ApplyTo soma os deltas no status do MESMO id (canal 2) ----

TEST_CASE("environment_modifier: facilitated_status soma magnitude no mesmo id",
          "[domain][combat][environment]") {
    const FacilitatedStatus fs{StatusId::Disrupt, /*magnitude_delta=*/1, /*duration_delta=*/0};
    StatusEffect base{};
    base.id = StatusId::Disrupt;
    base.magnitude = 20;
    base.duration = 1;

    const StatusEffect out = fs.apply_to(base);
    REQUIRE(out.id == StatusId::Disrupt);
    REQUIRE(out.magnitude == 21);  // +1 mag
    REQUIRE(out.duration == 1);    // dur inalterada
}

TEST_CASE("environment_modifier: facilitated_status soma duracao no mesmo id",
          "[domain][combat][environment]") {
    const FacilitatedStatus fs{StatusId::Stun, /*magnitude_delta=*/0, /*duration_delta=*/1};
    StatusEffect base{};
    base.id = StatusId::Stun;
    base.magnitude = 0;
    base.duration = 1;

    const StatusEffect out = fs.apply_to(base);
    REQUIRE(out.duration == 2);  // +1 dur
    REQUIRE(out.magnitude == 0);
}

TEST_CASE("environment_modifier: facilitated_status e no-op para id diferente",
          "[domain][combat][environment]") {
    // Neblina facilita Disrupt; um Poison nao e afetado.
    const FacilitatedStatus fs{StatusId::Disrupt, /*magnitude_delta=*/1, /*duration_delta=*/0};
    StatusEffect poison{};
    poison.id = StatusId::Poison;
    poison.magnitude = 5;
    poison.duration = 3;

    const StatusEffect out = fs.apply_to(poison);
    REQUIRE(out == poison);  // inalterado
}

// ---- HardwareHook: defaults neutros + campos ----

TEST_CASE("environment_modifier: hardware_hook default e neutro",
          "[domain][combat][environment]") {
    const HardwareHook h{};
    REQUIRE(h.scan_ap_delta == 0);
    REQUIRE(h.scan_free == false);
    REQUIRE(h.prever_turn_delta == 0);
    REQUIRE(h.party_spd_delta == 0);
}

// ---- EnvironmentModifier.MultFor: listada retorna o mult; nao-listada 1.0 ----

TEST_CASE("environment_modifier: mult_for retorna o mult listado",
          "[domain][combat][environment]") {
    EnvironmentModifier env{};
    env.id = EnvironmentId::Chuva;
    env.layer = EnvironmentLayer::Clima;
    env.family_mults = {{CardFamily::Eletrico, 1.5f}, {CardFamily::Bioquimico, 0.66f}};

    REQUIRE(env.mult_for(CardFamily::Eletrico) == 1.5f);
    REQUIRE(env.mult_for(CardFamily::Bioquimico) == 0.66f);
}

TEST_CASE("environment_modifier: familia nao-listada e neutra 1.0",
          "[domain][combat][environment]") {
    EnvironmentModifier env{};
    env.id = EnvironmentId::Chuva;
    env.family_mults = {{CardFamily::Eletrico, 1.5f}};

    REQUIRE(env.mult_for(CardFamily::Sonico) == 1.0f);  // nao listada
}

TEST_CASE("environment_modifier: defaults do record (tier visivel, sem mults)",
          "[domain][combat][environment]") {
    EnvironmentModifier env{};
    env.id = EnvironmentId::None;
    env.layer = EnvironmentLayer::Terreno;

    REQUIRE(env.tier == EnvironmentTier::Visivel);
    REQUIRE(env.family_mults.empty());
    REQUIRE_FALSE(env.facilitated_status.has_value());
    REQUIRE(env.period_duration == 0);
    REQUIRE(env.mult_for(CardFamily::Eletrico) == 1.0f);
}

// environment_damage_test.cpp
//
// Spec executavel (Catch2 v3) da integracao de mult_ambiente como ULTIMO fator da formula
// secao 11 dentro da CombatStateMachine. Portado de
// engine/tests/turn_combat/environments/EnvironmentDamageTests.cs. POCO puro, ZERO Qt.
//
// Cobre: retrocompat (sem ambiente => dano identico); ambiente up/down; cap; mult_ambiente
// NUNCA toca mult_fraqueza; T6 Anomalia Perlin nao muda o dano.
//
// RNG fixado em 0.5 (sem variancia, sem crit) pra dano deterministico (FixedRandom).
//
// Cross-ref: engine/tests/turn_combat/environments/EnvironmentDamageTests.cs;
//            docs/design/mecanicas/combat.md secao 11/18.

#include <catch2/catch_test_macros.hpp>

#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "fixed_random.hpp"
#include "gus/domain/combat/combat_actor.hpp"
#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/combat/combat_records.hpp"
#include "gus/domain/combat/combat_state.hpp"
#include "gus/domain/combat/combat_state_machine.hpp"
#include "gus/domain/combat/environment_enums.hpp"

using namespace gus::domain::combat;
using gus::domain::tests::FixedRandom;
using Catch::Matchers::WithinAbs;

namespace {

CombatActor hero(int atk = 8, CardFamily family = CardFamily::Eletrico) {
    return CombatActor("gus", "gus", 50, atk, 2, 20, family, /*player=*/true);
}

CombatActor foe(int def = 1, CardFamily family = CardFamily::Eletrico) {
    return CombatActor("enemy", "enemy", 5000, 6, def, 10, family, /*player=*/false);
}

Card make_card(const std::string& id, CardFamily family, int power) {
    Card c;
    c.id = id;
    c.display_name = id;
    c.family = family;
    c.base_type = CardBaseType::Pulso;
    c.mana_cost = 1;
    c.ap_cost = 1;
    c.power = power;
    c.target_shape = TargetShape::Single;
    return c;
}

CombatActionProvider play_once(CombatAction action) {
    auto done = std::make_shared<bool>(false);
    return [action, done](CombatActor& a, const CombatState&) -> CombatAction {
        if (!a.is_player_side() || *done) return CombatAction::pass();
        *done = true;
        return action;
    };
}

int damage_dealt(CombatActor& h, CombatActor& f, const Card& card,
                 const std::vector<EnvironmentId>& environment) {
    std::unordered_map<std::string, Card> reg;
    reg.emplace(card.id, card);
    FixedRandom rng(0.5);
    CombatStateMachine sm({&h, &f}, play_once(CombatAction::use_card(card.id, f.id())), &reg,
                          nullptr, &rng);
    if (!environment.empty())
        sm.set_environment(environment);
    sm.begin_turn();
    sm.run_active_turn_to_end();
    return f.max_hp() - f.hp();
}

}  // namespace

// ===== Retrocompat =====

TEST_CASE("env-dmg: sem ambiente dano identico ao baseline", "[domain][combat][env]") {
    CombatActor h = hero(/*atk=*/0, CardFamily::Eletrico);
    CombatActor f = foe(/*def=*/1, CardFamily::Eletrico);
    Card card = make_card("pulso", CardFamily::Eletrico, 10);
    REQUIRE(damage_dealt(h, f, card, {}) == 10);
}

TEST_CASE("env-dmg: SetEnvironment None equivale a sem ambiente",
          "[domain][combat][env]") {
    Card card = make_card("pulso", CardFamily::Eletrico, 10);
    CombatActor h1 = hero(0, CardFamily::Eletrico);
    CombatActor f1 = foe(1, CardFamily::Eletrico);
    const int sem = damage_dealt(h1, f1, card, {});
    CombatActor h2 = hero(0, CardFamily::Eletrico);
    CombatActor f2 = foe(1, CardFamily::Eletrico);
    const int com_none = damage_dealt(h2, f2, card, {EnvironmentId::None});
    REQUIRE(sem == com_none);
}

// ===== Ambiente up/down =====

TEST_CASE("env-dmg: ambiente casa aumenta o dano da familia", "[domain][combat][env]") {
    CombatActor h = hero(0, CardFamily::Eletrico);
    CombatActor f = foe(1, CardFamily::Eletrico);
    Card card = make_card("pulso", CardFamily::Eletrico, 10);
    REQUIRE(damage_dealt(h, f, card, {EnvironmentId::Chuva}) == 15);  // 9.90 * 1.5
}

TEST_CASE("env-dmg: ambiente hostil reduz o dano da familia", "[domain][combat][env]") {
    CombatActor h = hero(0, CardFamily::Eletrico);
    CombatActor f = foe(1, CardFamily::Eletrico);
    Card card = make_card("pulso", CardFamily::Eletrico, 10);
    REQUIRE(damage_dealt(h, f, card, {EnvironmentId::Calor}) == 7);  // 9.90 * 0.66
}

TEST_CASE("env-dmg: ambiente que nao afeta a familia nao muda o dano",
          "[domain][combat][env]") {
    CombatActor h = hero(0, CardFamily::Sonico);
    CombatActor f = foe(1, CardFamily::Sonico);
    Card card = make_card("eco", CardFamily::Sonico, 10);
    REQUIRE(damage_dealt(h, f, card, {EnvironmentId::Chuva}) == 10);
}

// ===== mult_ambiente NUNCA toca mult_fraqueza =====

TEST_CASE("env-dmg: ambiente multiplica por cima da fraqueza sem alterar a roda",
          "[domain][combat][env]") {
    CombatActor h = hero(0, CardFamily::Eletrico);
    CombatActor f = foe(1, CardFamily::Cinetico);
    Card card = make_card("pulso", CardFamily::Eletrico, 10);
    REQUIRE(damage_dealt(h, f, card, {EnvironmentId::Chuva}) == 22);  // 9.90 * 2.25
}

TEST_CASE("env-dmg: ambiente nao resgata imunidade (mult_ambiente sozinho > 0)",
          "[domain][combat][env]") {
    CombatActor h = hero(8, CardFamily::Eletrico);
    CombatActor f = foe(1, CardFamily::Eletrico);
    CombatStateMachine sm({&h, &f}, [](CombatActor&, const CombatState&) { return CombatAction::pass(); });
    sm.set_environment({EnvironmentId::Chuva});
    REQUIRE(sm.mult_ambiente_for(CardFamily::Eletrico) > 0.0f);
}

// ===== Cap =====

TEST_CASE("env-dmg: cap de combate limita o dano a 2.25x", "[domain][combat][env]") {
    CombatActor h = hero(0, CardFamily::Eletrico);
    CombatActor f = foe(1, CardFamily::Eletrico);
    Card card = make_card("pulso", CardFamily::Eletrico, 10);
    // Lamacento (1.3) + Chuva (1.5) + Aurora (1.3) = 2.535 -> clamp 2.25.
    REQUIRE(damage_dealt(h, f, card,
                         {EnvironmentId::Lamacento, EnvironmentId::Chuva,
                          EnvironmentId::Aurora}) == 22);
}

// ===== T6 Anomalia Perlin =====

TEST_CASE("env-dmg: anomalia perlin nao altera o dano", "[domain][combat][env]") {
    Card card = make_card("glifo", CardFamily::Criptografico, 10);
    CombatActor h1 = hero(0, CardFamily::Criptografico);
    CombatActor f1 = foe(1, CardFamily::Criptografico);
    const int sem = damage_dealt(h1, f1, card, {});
    CombatActor h2 = hero(0, CardFamily::Criptografico);
    CombatActor f2 = foe(1, CardFamily::Criptografico);
    const int com_perlin = damage_dealt(h2, f2, card, {EnvironmentId::AnomaliaPerlin});
    REQUIRE(sem == com_perlin);
}

TEST_CASE("env-dmg: mult_ambiente_for default e 1.0", "[domain][combat][env]") {
    CombatActor h = hero();
    CombatActor f = foe();
    CombatStateMachine sm({&h, &f}, [](CombatActor&, const CombatState&) { return CombatAction::pass(); });
    for (const CardFamily fam : {CardFamily::Eletrico, CardFamily::Bioquimico, CardFamily::Sonico,
                                 CardFamily::Cinetico, CardFamily::Criptografico})
        REQUIRE_THAT(sm.mult_ambiente_for(fam), WithinAbs(1.0f, 1e-4));
}

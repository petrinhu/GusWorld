// combat_formula_test.cpp
//
// Spec executavel (Catch2 v3) das decisoes canonizadas pelo criador (2026-05-26, secao 11):
//   1. Formula divisiva no UseCard (Def reduz por fracao, nunca zera por subtracao).
//   2. Variancia Knowledge Decay (+-30% no 1o encontro -> +-5% floor conforme farm).
//   3. Critico por carta (Card.crit_chance; *1.5 pos-variancia; log "[CRITICO]").
//   4. Ataque basico permanece subtrativo clamp(atk - def, 1) - inalterado.
// Portado de engine/tests/turn_combat/CombatFormulaTests.cs. POCO puro, ZERO Qt.
//
// RNG injetado (FixedRandom) pra determinismo independente de plataforma.
//
// Cross-ref: engine/tests/turn_combat/CombatFormulaTests.cs; docs/design/mecanicas/combat.md secao 7/11.

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <unordered_map>
#include <vector>

#include "fixed_random.hpp"
#include "gus/domain/combat/combat_actor.hpp"
#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/combat/combat_records.hpp"
#include "gus/domain/combat/combat_state.hpp"
#include "gus/domain/combat/combat_state_machine.hpp"

using namespace gus::domain::combat;
using gus::domain::tests::FixedRandom;

namespace {

CombatActor hero(const std::string& id = "gus", int hp = 50, int spd = 20, int atk = 8,
                 int def = 2, CardFamily family = CardFamily::Eletrico) {
    return CombatActor(id, id, hp, atk, def, spd, family, /*player=*/true);
}

CombatActor foe(const std::string& id = "enemy", int hp = 500, int spd = 10, int atk = 6,
                int def = 1, CardFamily family = CardFamily::Cinetico, int kills = 0) {
    return CombatActor(id, id, hp, atk, def, spd, family, /*player=*/false, /*boss=*/false, kills);
}

Card make_card(const std::string& id, CardFamily family, int power, int mana_cost = 1,
               int crit_chance = 0, TargetShape shape = TargetShape::Single) {
    Card c;
    c.id = id;
    c.display_name = id;
    c.family = family;
    c.base_type = CardBaseType::Pulso;
    c.mana_cost = mana_cost;
    c.ap_cost = 1;
    c.power = power;
    c.target_shape = shape;
    c.crit_chance = crit_chance;
    return c;
}

std::unordered_map<std::string, Card> registry(const Card& c) {
    std::unordered_map<std::string, Card> d;
    d.emplace(c.id, c);
    return d;
}

// Provider que joga UMA acao player-side e depois passa; inimigo sempre passa.
CombatActionProvider play_once(CombatAction action) {
    auto done = std::make_shared<bool>(false);
    return [action, done](CombatActor& a, const CombatState&) -> CombatAction {
        if (!a.is_player_side() || *done) return CombatAction::pass();
        *done = true;
        return action;
    };
}

bool log_has(const CombatStateMachine& sm, const std::string& needle) {
    for (const auto& e : sm.log())
        if (e.message.find(needle) != std::string::npos)
            return true;
    return false;
}

// Roda o turno corrente e devolve o dano causado (max_hp - hp do alvo).
int damage_dealt(CombatActor& h, CombatActor& f, const Card& card, IRandomSource& rng) {
    auto reg = registry(card);
    CombatStateMachine sm({&h, &f}, play_once(CombatAction::use_card(card.id, f.id())), &reg,
                          nullptr, &rng);
    sm.begin_turn();
    sm.run_active_turn_to_end();
    return f.max_hp() - f.hp();
}

}  // namespace

// ===== 1. Formula divisiva =====

TEST_CASE("formula: divisiva def alto ainda causa dano (sem subtrativa)",
          "[domain][combat][formula]") {
    CombatActor h = hero("gus", 50, 20, /*atk=*/0, 2, CardFamily::Eletrico);
    CombatActor f = foe("enemy", 500, 10, 6, /*def=*/80, CardFamily::Eletrico);
    Card card = make_card("pulso.eletrico", CardFamily::Eletrico, 10);
    FixedRandom rng;
    REQUIRE(damage_dealt(h, f, card, rng) == 6);  // 10*(100/180) = 5.5556 -> 6
}

TEST_CASE("formula: divisiva def zero nao reduz dano", "[domain][combat][formula]") {
    CombatActor h = hero("gus", 50, 20, 0, 2, CardFamily::Eletrico);
    CombatActor f = foe("enemy", 500, 10, 6, /*def=*/0, CardFamily::Eletrico);
    Card card = make_card("pulso.eletrico", CardFamily::Eletrico, 10);
    FixedRandom rng;
    REQUIRE(damage_dealt(h, f, card, rng) == 10);
}

TEST_CASE("formula: divisiva sem clamp minimo arredonda pra zero",
          "[domain][combat][formula]") {
    CombatActor h = hero("gus", 50, 20, 0, 2, CardFamily::Eletrico);
    CombatActor f = foe("enemy", 500, 10, 6, /*def=*/9000, CardFamily::Eletrico);
    Card card = make_card("pulso.fraco", CardFamily::Eletrico, 1);
    FixedRandom rng;
    REQUIRE(damage_dealt(h, f, card, rng) == 0);  // 0.01099 -> 0
}

// ===== 2. Variancia Knowledge Decay =====

TEST_CASE("formula: variancia kills zero aplica 30pct max no rng topo",
          "[domain][combat][formula]") {
    CombatActor h = hero("gus", 50, 20, 0, 2, CardFamily::Eletrico);
    CombatActor f = foe("enemy", 500, 10, 6, 0, CardFamily::Eletrico, /*kills=*/0);
    Card card = make_card("pulso.eletrico", CardFamily::Eletrico, 10);
    FixedRandom rng(/*next_double=*/1.0);
    REQUIRE(damage_dealt(h, f, card, rng) == 13);  // 10 * 1.30
}

TEST_CASE("formula: variancia kills zero aplica 30pct min no rng fundo",
          "[domain][combat][formula]") {
    CombatActor h = hero("gus", 50, 20, 0, 2, CardFamily::Eletrico);
    CombatActor f = foe("enemy", 500, 10, 6, 0, CardFamily::Eletrico, /*kills=*/0);
    Card card = make_card("pulso.eletrico", CardFamily::Eletrico, 10);
    FixedRandom rng(/*next_double=*/0.0);
    REQUIRE(damage_dealt(h, f, card, rng) == 7);  // 10 * 0.70
}

TEST_CASE("formula: variancia kills altos decai pra 5pct floor",
          "[domain][combat][formula]") {
    Card card = make_card("pulso.forte", CardFamily::Eletrico, 100);

    CombatActor ht = hero("gus", 50, 20, 0, 2, CardFamily::Eletrico);
    CombatActor ft = foe("enemy", 500, 10, 6, 0, CardFamily::Eletrico, /*kills=*/100);
    FixedRandom topo(1.0);
    REQUIRE(damage_dealt(ht, ft, card, topo) == 105);  // +5%

    CombatActor hf = hero("gus", 50, 20, 0, 2, CardFamily::Eletrico);
    CombatActor ff = foe("enemy", 500, 10, 6, 0, CardFamily::Eletrico, /*kills=*/100);
    FixedRandom fundo(0.0);
    REQUIRE(damage_dealt(hf, ff, card, fundo) == 95);  // -5%
}

TEST_CASE("formula: variancia range encolhe conforme kills sobem",
          "[domain][combat][formula]") {
    Card card = make_card("pulso.forte", CardFamily::Eletrico, 100);

    auto roll = [&](int kills, double nd) {
        CombatActor h = hero("gus", 50, 20, 0, 2, CardFamily::Eletrico);
        CombatActor f = foe("enemy", 500, 10, 6, 0, CardFamily::Eletrico, kills);
        FixedRandom rng(nd);
        return damage_dealt(h, f, card, rng);
    };

    const int spread0 = roll(0, 1.0) - roll(0, 0.0);
    const int spread100 = roll(100, 1.0) - roll(100, 0.0);
    REQUIRE(spread0 == 60);
    REQUIRE(spread100 == 10);
    REQUIRE(spread100 < spread0);
}

// ===== 3. Critico por carta =====

TEST_CASE("formula: crit 100 sempre multiplica por 1.5 e loga critico",
          "[domain][combat][formula]") {
    CombatActor h = hero("gus", 50, 20, 0, 2, CardFamily::Eletrico);
    CombatActor f = foe("enemy", 500, 10, 6, 0, CardFamily::Eletrico);
    Card card = make_card("pulso.crit", CardFamily::Eletrico, 10, 1, /*crit=*/100);
    auto reg = registry(card);
    FixedRandom rng(0.5, 0);
    CombatStateMachine sm({&h, &f}, play_once(CombatAction::use_card(card.id, f.id())), &reg,
                          nullptr, &rng);
    sm.begin_turn();
    sm.run_active_turn_to_end();
    REQUIRE(f.max_hp() - f.hp() == 15);
    REQUIRE(log_has(sm, "[CRITICO]"));
}

TEST_CASE("formula: crit 0 nunca crita e log sem critico", "[domain][combat][formula]") {
    CombatActor h = hero("gus", 50, 20, 0, 2, CardFamily::Eletrico);
    CombatActor f = foe("enemy", 500, 10, 6, 0, CardFamily::Eletrico);
    Card card = make_card("pulso.nocrit", CardFamily::Eletrico, 10, 1, /*crit=*/0);
    auto reg = registry(card);
    FixedRandom rng(0.5, 0);
    CombatStateMachine sm({&h, &f}, play_once(CombatAction::use_card(card.id, f.id())), &reg,
                          nullptr, &rng);
    sm.begin_turn();
    sm.run_active_turn_to_end();
    REQUIRE(f.max_hp() - f.hp() == 10);
    REQUIRE_FALSE(log_has(sm, "[CRITICO]"));
}

TEST_CASE("formula: crit roll acima da chance nao crita", "[domain][combat][formula]") {
    CombatActor h = hero("gus", 50, 20, 0, 2, CardFamily::Eletrico);
    CombatActor f = foe("enemy", 500, 10, 6, 0, CardFamily::Eletrico);
    Card card = make_card("pulso.crit30", CardFamily::Eletrico, 10, 1, /*crit=*/30);
    auto reg = registry(card);
    FixedRandom rng(0.5, /*next_int=*/50);  // 50 >= 30 => nao crita
    CombatStateMachine sm({&h, &f}, play_once(CombatAction::use_card(card.id, f.id())), &reg,
                          nullptr, &rng);
    sm.begin_turn();
    sm.run_active_turn_to_end();
    REQUIRE(f.max_hp() - f.hp() == 10);
    REQUIRE_FALSE(log_has(sm, "[CRITICO]"));
}

// ===== 4. Ataque basico subtrativo (inalterado) =====

TEST_CASE("formula: ataque basico usa subtrativa clamp 1 inalterado",
          "[domain][combat][formula]") {
    CombatActor h = hero("gus", 50, 20, /*atk=*/8);
    CombatActor f = foe("enemy", 500, 10, 6, /*def=*/2);
    FixedRandom rng(1.0, 0);  // RNG nao deve afetar o basico
    CombatStateMachine sm({&h, &f}, play_once(CombatAction::attack(f.id())), nullptr, nullptr,
                          &rng);
    sm.begin_turn();
    sm.run_active_turn_to_end();
    REQUIRE(f.max_hp() - f.hp() == 6);  // 8 - 2
}

TEST_CASE("formula: ataque basico clamp minimo 1 quando def supera atk",
          "[domain][combat][formula]") {
    CombatActor h = hero("gus", 50, 20, /*atk=*/3);
    CombatActor f = foe("enemy", 500, 10, 6, /*def=*/50);
    FixedRandom rng(0.0);
    CombatStateMachine sm({&h, &f}, play_once(CombatAction::attack(f.id())), nullptr, nullptr,
                          &rng);
    sm.begin_turn();
    sm.run_active_turn_to_end();
    REQUIRE(f.max_hp() - f.hp() == 1);
}

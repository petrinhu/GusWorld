// combat_inc2_test.cpp
//
// Spec executavel (Catch2 v3) do incremento 2 (secao 5..12, 14): StackRule, status
// tick/expire, Scan, UseCard (formula de dano completa, mana, combo, status), Defend,
// Flee + IsBoss. Portado de engine/tests/turn_combat/CombatInc2Tests.cs. POCO puro, ZERO Qt.
//
// MAPEAMENTO de excecoes: InvalidOperationException -> std::logic_error;
//                         KeyNotFoundException      -> std::out_of_range.
//
// Cross-ref: engine/tests/turn_combat/CombatInc2Tests.cs;
//            docs/design/mecanicas/combat.md secao 5/6/7/8/9/10/11/12/14.

#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <stdexcept>
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

CombatActor foe(const std::string& id = "enemy", int hp = 50, int spd = 10, int atk = 6,
                int def = 1, CardFamily family = CardFamily::Cinetico, bool boss = false) {
    return CombatActor(id, id, hp, atk, def, spd, family, /*player=*/false, boss);
}

Card make_card(const std::string& id, CardFamily family, int power, int mana_cost = 1,
               TargetShape shape = TargetShape::Single,
               std::optional<StatusEffect> status_applied = std::nullopt,
               CardBaseType base_type = CardBaseType::Pulso) {
    Card c;
    c.id = id;
    c.display_name = id;
    c.family = family;
    c.base_type = base_type;
    c.mana_cost = mana_cost;
    c.ap_cost = 1;
    c.power = power;
    c.target_shape = shape;
    c.status_applied = std::move(status_applied);
    return c;
}

std::unordered_map<std::string, Card> registry(const Card& c) {
    std::unordered_map<std::string, Card> d;
    d.emplace(c.id, c);
    return d;
}

StatusEffect effect(StatusId id, int mag, int dur, CardFamily origin) {
    return StatusEffect{id, mag, dur, StackRule::Replace, origin};
}

CombatActionProvider play_once(CombatAction action) {
    auto done = std::make_shared<bool>(false);
    return [action, done](CombatActor& a, const CombatState&) -> CombatAction {
        if (!a.is_player_side() || *done) return CombatAction::pass();
        *done = true;
        return action;
    };
}

CombatActionProvider play_once_at_round(CombatAction action, int min_round) {
    auto done = std::make_shared<bool>(false);
    return [action, done, min_round](CombatActor& a, const CombatState& s) -> CombatAction {
        if (!a.is_player_side() || *done || s.round_index() < min_round)
            return CombatAction::pass();
        *done = true;
        return action;
    };
}

const StatusEffect* find_status(const CombatActor& a, StatusId id) {
    for (const auto& s : a.status_effects())
        if (s.id == id) return &s;
    return nullptr;
}

bool log_has(const CombatStateMachine& sm, const std::string& needle) {
    for (const auto& e : sm.log())
        if (e.message.find(needle) != std::string::npos) return true;
    return false;
}

}  // namespace

// ===== StackRule (CombatActor::add_status) =====

TEST_CASE("inc2: StackRule Replace descarta anterior", "[domain][combat][inc2]") {
    CombatActor a = hero();
    a.add_status({StatusId::Poison, 3, 2, StackRule::Replace, CardFamily::Bioquimico});
    a.add_status({StatusId::Poison, 7, 5, StackRule::Replace, CardFamily::Bioquimico});
    REQUIRE(a.status_effects().size() == 1);
    REQUIRE(a.status_effects()[0].magnitude == 7);
    REQUIRE(a.status_effects()[0].duration == 5);
}

TEST_CASE("inc2: StackRule Refresh atualiza duration pra maior", "[domain][combat][inc2]") {
    CombatActor a = hero();
    a.add_status({StatusId::Poison, 3, 4, StackRule::Refresh, CardFamily::Bioquimico});
    a.add_status({StatusId::Poison, 9, 2, StackRule::Refresh, CardFamily::Bioquimico});
    REQUIRE(a.status_effects().size() == 1);
    REQUIRE(a.status_effects()[0].duration == 4);
}

TEST_CASE("inc2: StackRule StackMagnitude soma magnitude", "[domain][combat][inc2]") {
    CombatActor a = hero();
    a.add_status({StatusId::Poison, 3, 2, StackRule::StackMagnitude, CardFamily::Bioquimico});
    a.add_status({StatusId::Poison, 5, 9, StackRule::StackMagnitude, CardFamily::Bioquimico});
    REQUIRE(a.status_effects().size() == 1);
    REQUIRE(a.status_effects()[0].magnitude == 8);
}

TEST_CASE("inc2: StackRule StackDuration soma duration", "[domain][combat][inc2]") {
    CombatActor a = hero();
    a.add_status({StatusId::Poison, 3, 2, StackRule::StackDuration, CardFamily::Bioquimico});
    a.add_status({StatusId::Poison, 99, 5, StackRule::StackDuration, CardFamily::Bioquimico});
    REQUIRE(a.status_effects().size() == 1);
    REQUIRE(a.status_effects()[0].duration == 7);
}

// ===== Status tick =====

TEST_CASE("inc2: tick Poison aplica dano igual magnitude", "[domain][combat][inc2]") {
    CombatActor h = hero("gus", 50);
    CombatActor e = foe();
    h.add_status(effect(StatusId::Poison, 4, 3, CardFamily::Bioquimico));
    CombatStateMachine sm({&h, &e}, [](CombatActor&, const CombatState&) { return CombatAction::pass(); });
    sm.begin_turn();
    REQUIRE(h.hp() == 46);
}

TEST_CASE("inc2: tick Regen cura igual magnitude", "[domain][combat][inc2]") {
    CombatActor h = hero("gus", 50);
    CombatActor e = foe();
    h.take_damage(20);
    h.add_status(effect(StatusId::Regen, 5, 3, CardFamily::Bioquimico));
    CombatStateMachine sm({&h, &e}, [](CombatActor&, const CombatState&) { return CombatAction::pass(); });
    sm.begin_turn();
    REQUIRE(h.hp() == 35);
}

TEST_CASE("inc2: tick Corrode aplica dano e reduz def", "[domain][combat][inc2]") {
    CombatActor h = hero("gus", 50, 20, 8, /*def=*/10);
    CombatActor e = foe();
    h.add_status(effect(StatusId::Corrode, 3, 3, CardFamily::Bioquimico));
    CombatStateMachine sm({&h, &e}, [](CombatActor&, const CombatState&) { return CombatAction::pass(); });
    sm.begin_turn();
    REQUIRE(h.hp() == 47);
    REQUIRE(h.def() == 7);
}

TEST_CASE("inc2: tick Stun faz pular turno sem gastar ap", "[domain][combat][inc2]") {
    CombatActor h = hero("gus", 50, 20, /*atk=*/10);
    CombatActor e = foe("enemy", 100, 10, 6, /*def=*/0);
    h.add_status(effect(StatusId::Stun, 0, 1, CardFamily::Eletrico));
    int calls = 0;
    CombatStateMachine sm({&h, &e}, [&](CombatActor&, const CombatState&) {
        ++calls;
        return CombatAction::attack(e.id());
    });
    const bool stunned = sm.begin_turn();
    REQUIRE(stunned);
    if (!stunned) sm.run_active_turn_to_end();
    REQUIRE(calls == 0);
    REQUIRE(h.ap() == 3);
    REQUIRE(e.hp() == 100);
}

TEST_CASE("inc2: tick decrementa duration de todos os status", "[domain][combat][inc2]") {
    CombatActor h = hero("gus", 50);
    CombatActor e = foe();
    h.add_status(effect(StatusId::Poison, 1, 3, CardFamily::Bioquimico));
    h.add_status(effect(StatusId::Regen, 1, 5, CardFamily::Bioquimico));
    CombatStateMachine sm({&h, &e}, [](CombatActor&, const CombatState&) { return CombatAction::pass(); });
    sm.begin_turn();
    REQUIRE(find_status(h, StatusId::Poison)->duration == 2);
    REQUIRE(find_status(h, StatusId::Regen)->duration == 4);
}

// ===== Status expire =====

TEST_CASE("inc2: expire remove status com duration zero no TurnEnd",
          "[domain][combat][inc2]") {
    CombatActor h = hero("gus", 50);
    CombatActor e = foe();
    h.add_status(effect(StatusId::Poison, 1, 1, CardFamily::Bioquimico));
    CombatStateMachine sm({&h, &e}, [](CombatActor&, const CombatState&) { return CombatAction::pass(); });
    sm.begin_turn();
    sm.run_active_turn_to_end();
    REQUIRE(find_status(h, StatusId::Poison) == nullptr);
}

TEST_CASE("inc2: expire mantem status com duration positivo", "[domain][combat][inc2]") {
    CombatActor h = hero("gus", 50);
    CombatActor e = foe();
    h.add_status(effect(StatusId::Poison, 1, 3, CardFamily::Bioquimico));
    CombatStateMachine sm({&h, &e}, [](CombatActor&, const CombatState&) { return CombatAction::pass(); });
    sm.begin_turn();
    sm.run_active_turn_to_end();
    const StatusEffect* p = find_status(h, StatusId::Poison);
    REQUIRE(p != nullptr);
    REQUIRE(p->duration == 2);
}

// ===== Scan =====

TEST_CASE("inc2: scan seta is_scanned e loga hp e familia", "[domain][combat][inc2]") {
    CombatActor h = hero();
    CombatActor e = foe("enemy", 42, 10, 6, 1, CardFamily::Sonico);
    REQUIRE_FALSE(e.is_scanned());
    CombatStateMachine sm({&h, &e}, [&](CombatActor& a, const CombatState&) {
        return a.is_player_side() ? CombatAction::scan(e.id()) : CombatAction::pass();
    });
    sm.begin_turn();
    sm.run_active_turn_to_end();
    REQUIRE(e.is_scanned());
    REQUIRE(log_has(sm, "42"));
    REQUIRE(log_has(sm, "Sonico"));
}

// ===== UseCard - formula de dano =====

TEST_CASE("inc2: usecard dano fraco aplica mult 1.5", "[domain][combat][inc2]") {
    CombatActor h = hero("gus", 50, 20, /*atk=*/8, 2, CardFamily::Eletrico);
    CombatActor e = foe("enemy", 100, 10, 6, /*def=*/1, CardFamily::Cinetico);
    Card card = make_card("pulso.eletrico", CardFamily::Eletrico, 10);
    auto reg = registry(card);
    FixedRandom rng;
    CombatStateMachine sm({&h, &e}, play_once(CombatAction::use_card(card.id, e.id())), &reg,
                          nullptr, &rng);
    sm.begin_turn();
    sm.run_active_turn_to_end();
    REQUIRE(e.hp() == 100 - 27);
}

TEST_CASE("inc2: usecard dano neutro aplica mult 1.0", "[domain][combat][inc2]") {
    CombatActor h = hero("gus", 50, 20, 8, 2, CardFamily::Eletrico);
    CombatActor e = foe("enemy", 100, 10, 6, 1, CardFamily::Eletrico);
    Card card = make_card("pulso.eletrico", CardFamily::Eletrico, 10);
    auto reg = registry(card);
    FixedRandom rng;
    CombatStateMachine sm({&h, &e}, play_once(CombatAction::use_card(card.id, e.id())), &reg,
                          nullptr, &rng);
    sm.begin_turn();
    sm.run_active_turn_to_end();
    REQUIRE(e.hp() == 100 - 18);
}

TEST_CASE("inc2: usecard resistente aplica mult 0.66", "[domain][combat][inc2]") {
    CombatActor h = hero("gus", 50, 20, /*atk=*/0, 2, CardFamily::Cinetico);
    CombatActor e = foe("enemy", 100, 10, 6, /*def=*/0, CardFamily::Eletrico);
    Card card = make_card("pulso.cinetico", CardFamily::Cinetico, 10);
    auto reg = registry(card);
    FixedRandom rng;
    CombatStateMachine sm({&h, &e}, play_once(CombatAction::use_card(card.id, e.id())), &reg,
                          nullptr, &rng);
    sm.begin_turn();
    sm.run_active_turn_to_end();
    REQUIRE(e.hp() == 93);  // round(6.6) = 7
}

TEST_CASE("inc2: usecard divisiva sem clamp def alto ainda causa dano",
          "[domain][combat][inc2]") {
    CombatActor h = hero("gus", 50, 20, /*atk=*/0, 2, CardFamily::Eletrico);
    CombatActor e = foe("enemy", 100, 10, 6, /*def=*/50, CardFamily::Eletrico);
    Card card = make_card("pulso.fraco", CardFamily::Eletrico, 1);
    auto reg = registry(card);
    FixedRandom rng;
    CombatStateMachine sm({&h, &e}, play_once(CombatAction::use_card(card.id, e.id())), &reg,
                          nullptr, &rng);
    sm.begin_turn();
    sm.run_active_turn_to_end();
    REQUIRE(e.hp() == 99);  // round(0.6667) = 1
}

// ===== UseCard - mana =====

TEST_CASE("inc2: usecard consome mana igual ManaCost sem modificador",
          "[domain][combat][inc2]") {
    CombatActor h = hero("gus", 50, 20, 8, 2, CardFamily::Eletrico);
    CombatActor e = foe("enemy", 100, 10, 6, 0);
    Card card = make_card("pulso.eletrico", CardFamily::Eletrico, 5, /*mana=*/2);
    auto reg = registry(card);
    CombatStateMachine sm({&h, &e}, play_once(CombatAction::use_card(card.id, e.id())), &reg);
    sm.begin_turn();
    sm.run_active_turn_to_end();
    REQUIRE(h.mana() == 0);
}

TEST_CASE("inc2: usecard modificador Object soma 1 mana", "[domain][combat][inc2]") {
    CombatActor h = hero("gus", 50, 20, 8, 2, CardFamily::Eletrico);
    CombatActor e = foe("enemy", 100, 10, 6, 0);
    Card card = make_card("pulso.eletrico", CardFamily::Eletrico, 5, /*mana=*/1);
    auto reg = registry(card);
    CombatAction act = CombatAction::use_card(card.id, e.id());
    act.modifier = CardModifier::Object;
    CombatStateMachine sm({&h, &e}, play_once(act), &reg);
    sm.begin_turn();
    sm.run_active_turn_to_end();
    REQUIRE(h.mana() == 0);  // 2 - (1 + 1)
}

TEST_CASE("inc2: usecard modificador Stream soma 2 mana", "[domain][combat][inc2]") {
    CombatActor h = hero("gus", 50, 20, 8, 2, CardFamily::Eletrico);
    CombatActor e = foe("enemy", 100, 10, 6, 0);
    Card card = make_card("pulso.eletrico", CardFamily::Eletrico, 5, /*mana=*/1);
    auto reg = registry(card);
    CombatAction act = CombatAction::use_card(card.id, e.id());
    act.modifier = CardModifier::Stream;
    CombatStateMachine sm({&h, &e}, play_once_at_round(act, /*min_round=*/1), &reg);
    sm.begin_turn(); sm.run_active_turn_to_end(); sm.advance_to_next_actor();  // hero r0
    sm.begin_turn(); sm.run_active_turn_to_end(); sm.advance_to_next_actor();  // foe
    sm.begin_turn(); sm.run_active_turn_to_end();                              // hero r1
    REQUIRE(h.mana() == 0);  // 3 - (1 + 2)
}

TEST_CASE("inc2: usecard Null sem scan lanca", "[domain][combat][inc2]") {
    CombatActor h = hero("gus", 50, 20, 8, 2, CardFamily::Eletrico);
    CombatActor e = foe("enemy", 100, 10, 6, 0);
    Card card = make_card("pulso.eletrico", CardFamily::Eletrico, 5, /*mana=*/1);
    auto reg = registry(card);
    CombatStateMachine sm({&h, &e}, [&](CombatActor& a, const CombatState&) {
        if (!a.is_player_side()) return CombatAction::pass();
        CombatAction act = CombatAction::use_card(card.id, e.id());
        act.modifier = CardModifier::Null;
        return act;
    }, &reg);
    sm.begin_turn();
    REQUIRE_THROWS_AS(sm.run_active_turn_to_end(), std::logic_error);
}

TEST_CASE("inc2: usecard Null com scan previo funciona", "[domain][combat][inc2]") {
    CombatActor h = hero("gus", 50, 20, 8, 2, CardFamily::Eletrico);
    CombatActor e = foe("enemy", 100, 10, 6, 0);
    e.set_scanned(true);
    Card card = make_card("pulso.eletrico", CardFamily::Eletrico, 5, /*mana=*/1);
    auto reg = registry(card);
    CombatAction act = CombatAction::use_card(card.id, e.id());
    act.modifier = CardModifier::Null;
    CombatStateMachine sm({&h, &e}, play_once(act), &reg);
    sm.begin_turn();
    sm.run_active_turn_to_end();
    REQUIRE(h.mana() == 0);  // 2 - (1 + 1 Null)
}

TEST_CASE("inc2: usecard mana insuficiente lanca", "[domain][combat][inc2]") {
    CombatActor h = hero("gus", 50, 20, 8, 2, CardFamily::Eletrico);
    CombatActor e = foe("enemy", 100, 10, 6, 0);
    Card card = make_card("pulso.caro", CardFamily::Eletrico, 5, /*mana=*/99);
    auto reg = registry(card);
    CombatStateMachine sm({&h, &e}, [&](CombatActor& a, const CombatState&) {
        return a.is_player_side() ? CombatAction::use_card(card.id, e.id()) : CombatAction::pass();
    }, &reg);
    sm.begin_turn();
    REQUIRE_THROWS_AS(sm.run_active_turn_to_end(), std::logic_error);
}

TEST_CASE("inc2: usecard carta inexistente lanca out_of_range", "[domain][combat][inc2]") {
    CombatActor h = hero("gus", 50, 20, 8, 2, CardFamily::Eletrico);
    CombatActor e = foe("enemy", 100, 10, 6, 0);
    CombatStateMachine sm({&h, &e}, [&](CombatActor& a, const CombatState&) {
        return a.is_player_side() ? CombatAction::use_card("nao_existe", e.id()) : CombatAction::pass();
    });
    sm.begin_turn();
    REQUIRE_THROWS_AS(sm.run_active_turn_to_end(), std::out_of_range);
}

// ===== UseCard - combo =====

TEST_CASE("inc2: usecard pulso.eletrico + Stream casa combo e aplica mult",
          "[domain][combat][inc2]") {
    CombatActor h = hero("gus", 50, 20, /*atk=*/0, 2, CardFamily::Eletrico);
    CombatActor e = foe("enemy", 100, 10, 6, /*def=*/0, CardFamily::Cinetico);
    Card card = make_card("pulso.eletrico", CardFamily::Eletrico, 10, /*mana=*/1);
    auto reg = registry(card);
    FixedRandom rng;
    CombatAction act = CombatAction::use_card(card.id, e.id());
    act.modifier = CardModifier::Stream;
    CombatStateMachine sm({&h, &e}, play_once_at_round(act, 1), &reg, nullptr, &rng);
    sm.begin_turn(); sm.run_active_turn_to_end(); sm.advance_to_next_actor();
    sm.begin_turn(); sm.run_active_turn_to_end(); sm.advance_to_next_actor();
    sm.begin_turn(); sm.run_active_turn_to_end();
    REQUIRE(e.hp() == 70);  // 100 - 30
    REQUIRE(log_has(sm, "COMPILADO: COMBO_PULSO_STREAM_NAME"));
}

TEST_CASE("inc2: usecard pipeline sem receita nao aplica combo",
          "[domain][combat][inc2]") {
    CombatActor h = hero("gus", 50, 20, /*atk=*/0, 2, CardFamily::Eletrico);
    CombatActor e = foe("enemy", 100, 10, 6, /*def=*/0, CardFamily::Cinetico);
    Card card = make_card("pulso.eletrico", CardFamily::Eletrico, 10, /*mana=*/1);
    auto reg = registry(card);
    FixedRandom rng;
    CombatStateMachine sm({&h, &e}, play_once(CombatAction::use_card(card.id, e.id())), &reg,
                          nullptr, &rng);
    sm.begin_turn();
    sm.run_active_turn_to_end();
    REQUIRE(e.hp() == 85);  // 100 - 15
    REQUIRE_FALSE(log_has(sm, "COMPILADO"));
}

// ===== UseCard - status aplicado =====

TEST_CASE("inc2: usecard com StatusApplied aplica status no alvo",
          "[domain][combat][inc2]") {
    StatusEffect poison{StatusId::Poison, 4, 3, StackRule::Replace, CardFamily::Bioquimico};
    CombatActor h = hero("gus", 50, 20, /*atk=*/0, 2, CardFamily::Bioquimico);
    CombatActor e = foe("enemy", 100, 10, 6, /*def=*/0, CardFamily::Cinetico);
    Card card = make_card("raiz.toxica", CardFamily::Bioquimico, 5, 1, TargetShape::Single, poison);
    auto reg = registry(card);
    CombatStateMachine sm({&h, &e}, play_once(CombatAction::use_card(card.id, e.id())), &reg);
    sm.begin_turn();
    sm.run_active_turn_to_end();
    const StatusEffect* p = find_status(e, StatusId::Poison);
    REQUIRE(p != nullptr);
    REQUIRE(p->magnitude == 4);
}

TEST_CASE("inc2: usecard combo com ResultStatus aplica status adicional",
          "[domain][combat][inc2]") {
    CombatActor h = hero("gus", 50, 20, /*atk=*/0, 2, CardFamily::Bioquimico);
    CombatActor e = foe("enemy", 100, 10, 6, /*def=*/0, CardFamily::Cinetico);
    e.set_scanned(true);
    Card card = make_card("raiz.bioquimico", CardFamily::Bioquimico, 5, /*mana=*/1);
    auto reg = registry(card);
    CombatAction act = CombatAction::use_card(card.id, e.id());
    act.modifier = CardModifier::Null;
    CombatStateMachine sm({&h, &e}, play_once(act), &reg);
    sm.begin_turn();
    sm.run_active_turn_to_end();
    const StatusEffect* r = find_status(e, StatusId::Regen);
    REQUIRE(r != nullptr);
    REQUIRE(r->magnitude == 5);
    REQUIRE(log_has(sm, "COMPILADO: COMBO_RAIZ_NULL_NAME"));
}

// ===== Defend =====

TEST_CASE("inc2: defend aplica shield duration 1 no proprio ator",
          "[domain][combat][inc2]") {
    CombatActor h = hero("gus", 50, 20, 8, /*def=*/5);
    CombatActor e = foe();
    CombatStateMachine sm({&h, &e}, play_once(CombatAction::defend()));
    sm.begin_turn();
    sm.run_active_turn_to_end();
    const StatusEffect* shield = find_status(h, StatusId::Shield);
    REQUIRE(shield != nullptr);
    REQUIRE(shield->duration == 1);
    REQUIRE(shield->magnitude == 5);
}

// ===== Flee + IsBoss =====

TEST_CASE("inc2: flee falha com excecao quando ha inimigo boss",
          "[domain][combat][inc2]") {
    CombatActor h = hero("gus", 50, /*spd=*/100);
    CombatActor boss = foe("boss", 50, /*spd=*/1, 6, 1, CardFamily::Cinetico, /*boss=*/true);
    CombatStateMachine sm({&h, &boss}, [&](CombatActor& a, const CombatState&) {
        return a.is_player_side() ? CombatAction::flee() : CombatAction::pass();
    });
    sm.begin_turn();
    REQUIRE_THROWS_AS(sm.run_active_turn_to_end(), std::logic_error);
}

TEST_CASE("inc2: flee funciona normal sem boss", "[domain][combat][inc2]") {
    CombatActor h = hero("gus", 50, /*spd=*/100);
    CombatActor e = foe("enemy", 50, /*spd=*/1);
    CombatStateMachine sm({&h, &e}, [&](CombatActor& a, const CombatState&) {
        return a.is_player_side() ? CombatAction::flee() : CombatAction::pass();
    });
    const CombatResult result = sm.run_until_end();
    REQUIRE(result.outcome == CombatOutcome::Fled);
}

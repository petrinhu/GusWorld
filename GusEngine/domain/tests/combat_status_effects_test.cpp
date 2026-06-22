// combat_status_effects_test.cpp
//
// Spec executavel (Catch2 v3) dos status de combate (F2-E.5b): Silence, Disrupt, Break,
// Decrypt, Haste/Slow, Knockback. Portado de
// engine/tests/turn_combat/CombatStatusEffectsTests.cs. POCO puro, ZERO Qt.
//
// RNG cravado (FixedRandom 0.5) zera a variancia => dano deterministico.
//
// Cross-ref: engine/tests/turn_combat/CombatStatusEffectsTests.cs;
//            docs/design/mecanicas/combat.md secao 8/9/10/15.

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

CombatActor hero(const std::string& id = "gus", int hp = 100, int spd = 20, int atk = 8,
                 int def = 4, CardFamily family = CardFamily::Eletrico) {
    return CombatActor(id, id, hp, atk, def, spd, family, /*player=*/true);
}

CombatActor foe(const std::string& id = "enemy", int hp = 500, int spd = 10, int atk = 6,
                int def = 4, CardFamily family = CardFamily::Cinetico) {
    return CombatActor(id, id, hp, atk, def, spd, family, /*player=*/false);
}

Card make_card(const std::string& id, CardFamily family, int power, int mana_cost = 1,
               std::optional<StatusEffect> status_applied = std::nullopt) {
    Card c;
    c.id = id;
    c.display_name = id;
    c.family = family;
    c.base_type = CardBaseType::Pulso;
    c.mana_cost = mana_cost;
    c.ap_cost = 1;
    c.power = power;
    c.target_shape = TargetShape::Single;
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

const StatusEffect* find_status(const CombatActor& a, StatusId id) {
    for (const auto& s : a.status_effects())
        if (s.id == id) return &s;
    return nullptr;
}

std::vector<std::string> order_ids(const CombatStateMachine& sm) {
    std::vector<std::string> ids;
    for (const CombatActor* a : sm.queue().order()) ids.push_back(a->id());
    return ids;
}

}  // namespace

// ===== SILENCE =====

TEST_CASE("status: silence bloqueia usecard com erro de compilacao",
          "[domain][combat][status]") {
    CombatActor h = hero("gus", 100, 20, 8, 4, CardFamily::Eletrico);
    CombatActor e = foe("enemy", 200, 10, 6, /*def=*/0);
    h.add_status(effect(StatusId::Silence, 0, 2, CardFamily::Sonico));
    Card card = make_card("pulso.eletrico", CardFamily::Eletrico, 10);
    auto reg = registry(card);
    FixedRandom rng;
    CombatStateMachine sm({&h, &e}, play_once(CombatAction::use_card(card.id, e.id())), &reg,
                          nullptr, &rng);
    sm.begin_turn();
    REQUIRE_THROWS_AS(sm.run_active_turn_to_end(), std::logic_error);
    REQUIRE(e.hp() == 200);
}

TEST_CASE("status: silence permite ataque basico", "[domain][combat][status]") {
    CombatActor h = hero("gus", 100, 20, /*atk=*/10, 4, CardFamily::Eletrico);
    CombatActor e = foe("enemy", 200, 10, 6, /*def=*/2);
    h.add_status(effect(StatusId::Silence, 0, 2, CardFamily::Sonico));
    FixedRandom rng;
    CombatStateMachine sm({&h, &e}, play_once(CombatAction::attack(e.id())), nullptr, nullptr,
                          &rng);
    sm.begin_turn();
    sm.run_active_turn_to_end();
    REQUIRE(e.max_hp() - e.hp() == 8);
}

TEST_CASE("status: silence permite defender", "[domain][combat][status]") {
    CombatActor h = hero("gus", 100, 20, 8, /*def=*/7, CardFamily::Eletrico);
    CombatActor e = foe();
    h.add_status(effect(StatusId::Silence, 0, 2, CardFamily::Sonico));
    FixedRandom rng;
    CombatStateMachine sm({&h, &e}, play_once(CombatAction::defend()), nullptr, nullptr, &rng);
    sm.begin_turn();
    sm.run_active_turn_to_end();
    REQUIRE(find_status(h, StatusId::Shield) != nullptr);
}

TEST_CASE("status: silence permite flee", "[domain][combat][status]") {
    CombatActor h = hero("gus", 100, /*spd=*/100, 8, 4, CardFamily::Eletrico);
    CombatActor e = foe("enemy", 500, /*spd=*/1);
    h.add_status(effect(StatusId::Silence, 0, 2, CardFamily::Sonico));
    FixedRandom rng;
    CombatStateMachine sm({&h, &e}, play_once(CombatAction::flee()), nullptr, nullptr, &rng);
    sm.begin_turn();
    sm.run_active_turn_to_end();
    REQUIRE(sm.outcome() == CombatOutcome::Fled);
}

TEST_CASE("status: silence expira e libera usecard", "[domain][combat][status]") {
    CombatActor h = hero("gus", 100, 20, /*atk=*/0, 4, CardFamily::Eletrico);
    CombatActor e = foe("enemy", 200, 10, 6, /*def=*/0, CardFamily::Eletrico);
    h.add_status(effect(StatusId::Silence, 0, 1, CardFamily::Sonico));
    Card card = make_card("pulso.eletrico", CardFamily::Eletrico, 10);
    auto reg = registry(card);
    FixedRandom rng;

    auto card_played = std::make_shared<bool>(false);
    CombatStateMachine sm({&h, &e}, [card_played, &card, &e](CombatActor& a, const CombatState&) -> CombatAction {
        if (!a.is_player_side()) return CombatAction::pass();
        for (const auto& s : a.status_effects())
            if (s.id == StatusId::Silence) return CombatAction::pass();
        if (*card_played) return CombatAction::pass();
        *card_played = true;
        return CombatAction::use_card(card.id, e.id());
    }, &reg, nullptr, &rng);

    sm.begin_turn(); sm.run_active_turn_to_end(); sm.advance_to_next_actor();  // hero silenciado
    sm.begin_turn(); sm.run_active_turn_to_end(); sm.advance_to_next_actor();  // foe
    REQUIRE(find_status(h, StatusId::Silence) == nullptr);
    sm.begin_turn(); sm.run_active_turn_to_end();  // hero joga carta
    REQUIRE(e.max_hp() - e.hp() == 10);
}

// ===== DISRUPT =====

TEST_CASE("status: disrupt 30 reduz dano da proxima carta em 30pct",
          "[domain][combat][status]") {
    CombatActor h = hero("gus", 100, 20, /*atk=*/0, 4, CardFamily::Eletrico);
    CombatActor e = foe("enemy", 200, 10, 6, /*def=*/0, CardFamily::Eletrico);
    h.add_status(effect(StatusId::Disrupt, 30, 3, CardFamily::Sonico));
    Card card = make_card("pulso.eletrico", CardFamily::Eletrico, 10);
    auto reg = registry(card);
    FixedRandom rng;
    CombatStateMachine sm({&h, &e}, play_once(CombatAction::use_card(card.id, e.id())), &reg,
                          nullptr, &rng);
    sm.begin_turn();
    sm.run_active_turn_to_end();
    REQUIRE(e.max_hp() - e.hp() == 7);  // 10 * 0.70
}

TEST_CASE("status: disrupt consome na primeira carta e some", "[domain][combat][status]") {
    CombatActor h = hero("gus", 100, 20, /*atk=*/0, 4, CardFamily::Eletrico);
    CombatActor e = foe("enemy", 200, 10, 6, /*def=*/0, CardFamily::Eletrico);
    h.add_status(effect(StatusId::Disrupt, 50, 3, CardFamily::Sonico));
    Card card = make_card("pulso.eletrico", CardFamily::Eletrico, 10);
    auto reg = registry(card);
    FixedRandom rng;
    auto plays = std::make_shared<int>(0);
    CombatStateMachine sm({&h, &e}, [plays, &card, &e](CombatActor& a, const CombatState&) -> CombatAction {
        if (!a.is_player_side()) return CombatAction::pass();
        ++(*plays);
        if (*plays <= 2) return CombatAction::use_card(card.id, e.id());
        return CombatAction::pass();
    }, &reg, nullptr, &rng);
    sm.begin_turn();
    sm.run_active_turn_to_end();
    REQUIRE(e.max_hp() - e.hp() == 15);  // 5 + 10
    REQUIRE(find_status(h, StatusId::Disrupt) == nullptr);
}

TEST_CASE("status: disrupt nao afeta ataque basico", "[domain][combat][status]") {
    CombatActor h = hero("gus", 100, 20, /*atk=*/10, 4, CardFamily::Eletrico);
    CombatActor e = foe("enemy", 200, 10, 6, /*def=*/2);
    h.add_status(effect(StatusId::Disrupt, 50, 3, CardFamily::Sonico));
    FixedRandom rng;
    CombatStateMachine sm({&h, &e}, play_once(CombatAction::attack(e.id())), nullptr, nullptr,
                          &rng);
    sm.begin_turn();
    sm.run_active_turn_to_end();
    REQUIRE(e.max_hp() - e.hp() == 8);  // 10 - 2
}

// ===== BREAK =====

TEST_CASE("status: break reduz def ao aplicar", "[domain][combat][status]") {
    CombatActor f = foe("enemy", 500, 10, 6, /*def=*/10);
    CombatActor h = hero("gus", 100, /*spd=*/5);
    f.add_status(effect(StatusId::Break, 6, 2, CardFamily::Cinetico));
    FixedRandom rng;
    CombatStateMachine sm({&h, &f}, [](CombatActor&, const CombatState&) { return CombatAction::pass(); },
                          nullptr, nullptr, &rng);
    sm.begin_turn();  // foe age primeiro (spd 10 > hero 5)
    REQUIRE(f.def() == 4);
}

TEST_CASE("status: break restaura def ao expirar", "[domain][combat][status]") {
    CombatActor f = foe("enemy", 500, /*spd=*/10, 6, /*def=*/10);
    CombatActor h = hero("gus", 100, /*spd=*/5);
    f.add_status(effect(StatusId::Break, 6, 1, CardFamily::Cinetico));
    FixedRandom rng;
    CombatStateMachine sm({&h, &f}, [](CombatActor&, const CombatState&) { return CombatAction::pass(); },
                          nullptr, nullptr, &rng);
    sm.begin_turn();
    REQUIRE(f.def() == 4);
    sm.run_active_turn_to_end();
    REQUIRE(f.def() == 10);
    REQUIRE(find_status(f, StatusId::Break) == nullptr);
}

TEST_CASE("status: break def reduzida persiste durante a duracao",
          "[domain][combat][status]") {
    CombatActor f = foe("enemy", 500, /*spd=*/10, 6, /*def=*/10);
    CombatActor h = hero("gus", 100, /*spd=*/5);
    f.add_status(effect(StatusId::Break, 6, 3, CardFamily::Cinetico));
    FixedRandom rng;
    CombatStateMachine sm({&h, &f}, [](CombatActor&, const CombatState&) { return CombatAction::pass(); },
                          nullptr, nullptr, &rng);
    sm.begin_turn(); sm.run_active_turn_to_end(); sm.advance_to_next_actor();  // foe
    sm.begin_turn(); sm.run_active_turn_to_end(); sm.advance_to_next_actor();  // hero
    REQUIRE(f.def() == 4);
    REQUIRE(find_status(f, StatusId::Break) != nullptr);
}

// ===== HASTE / SLOW =====

TEST_CASE("status: haste aumenta spd e adianta na fila", "[domain][combat][status]") {
    CombatActor h = hero("gus", 100, /*spd=*/10);
    CombatActor e = foe("enemy", 500, /*spd=*/20);
    h.add_status(effect(StatusId::Haste, 15, 3, CardFamily::Eletrico));
    FixedRandom rng;
    CombatStateMachine sm({&e, &h}, [](CombatActor&, const CombatState&) { return CombatAction::pass(); },
                          nullptr, nullptr, &rng);
    REQUIRE(order_ids(sm)[0] == "enemy");
    sm.begin_turn(); sm.run_active_turn_to_end(); sm.advance_to_next_actor();  // foe
    sm.begin_turn();  // hero tick aplica Haste
    REQUIRE(h.spd() == 25);
    REQUIRE(order_ids(sm)[0] == "gus");
}

TEST_CASE("status: slow reduz spd e atrasa na fila", "[domain][combat][status]") {
    CombatActor h = hero("gus", 100, /*spd=*/30);
    CombatActor e = foe("enemy", 500, /*spd=*/20);
    h.add_status(effect(StatusId::Slow, 15, 3, CardFamily::Eletrico));
    FixedRandom rng;
    CombatStateMachine sm({&h, &e}, [](CombatActor&, const CombatState&) { return CombatAction::pass(); },
                          nullptr, nullptr, &rng);
    REQUIRE(order_ids(sm)[0] == "gus");
    sm.begin_turn();  // hero tick aplica Slow
    REQUIRE(h.spd() == 15);
    REQUIRE(order_ids(sm)[0] == "enemy");
}

TEST_CASE("status: slow clamp spd em zero", "[domain][combat][status]") {
    CombatActor h = hero("gus", 100, /*spd=*/10);
    CombatActor e = foe("enemy", 500, /*spd=*/5);
    h.add_status(effect(StatusId::Slow, 50, 3, CardFamily::Eletrico));
    FixedRandom rng;
    CombatStateMachine sm({&h, &e}, [](CombatActor&, const CombatState&) { return CombatAction::pass(); },
                          nullptr, nullptr, &rng);
    sm.begin_turn();
    REQUIRE(h.spd() == 0);
}

TEST_CASE("status: haste restaura spd ao expirar", "[domain][combat][status]") {
    CombatActor h = hero("gus", 100, /*spd=*/10);
    CombatActor e = foe("enemy", 500, /*spd=*/5);
    h.add_status(effect(StatusId::Haste, 15, 1, CardFamily::Eletrico));
    FixedRandom rng;
    CombatStateMachine sm({&h, &e}, [](CombatActor&, const CombatState&) { return CombatAction::pass(); },
                          nullptr, nullptr, &rng);
    sm.begin_turn();
    REQUIRE(h.spd() == 25);
    sm.run_active_turn_to_end();
    REQUIRE(h.spd() == 10);
    REQUIRE(find_status(h, StatusId::Haste) == nullptr);
}

// ===== KNOCKBACK =====

TEST_CASE("status: knockback empurra alvo uma posicao na fila",
          "[domain][combat][status]") {
    CombatActor a = hero("a", 100, /*spd=*/30);
    CombatActor b = foe("b", 500, /*spd=*/20);
    CombatActor c = foe("c", 500, /*spd=*/10);
    b.add_status(effect(StatusId::Knockback, 0, 2, CardFamily::Cinetico));
    FixedRandom rng;
    CombatStateMachine sm({&a, &b, &c}, [](CombatActor&, const CombatState&) { return CombatAction::pass(); },
                          nullptr, nullptr, &rng);
    REQUIRE(order_ids(sm) == std::vector<std::string>{"a", "b", "c"});
    sm.begin_turn(); sm.run_active_turn_to_end(); sm.advance_to_next_actor();  // a
    sm.begin_turn();  // b tick empurra +1
    REQUIRE(order_ids(sm) == std::vector<std::string>{"a", "c", "b"});
}

TEST_CASE("status: knockback no proprio tick nao faz o ator perder o turno",
          "[domain][combat][status]") {
    CombatActor a = hero("a", 100, /*spd=*/30, /*atk=*/0);
    CombatActor b = foe("b", 500, /*spd=*/20, /*atk=*/10);
    CombatActor c = foe("c", 200, /*spd=*/10, /*atk=*/0, /*def=*/0);
    b.add_status(effect(StatusId::Knockback, 0, 2, CardFamily::Cinetico));
    FixedRandom rng;
    auto b_acted = std::make_shared<bool>(false);
    CombatStateMachine sm({&a, &b, &c}, [b_acted](CombatActor& act, const CombatState&) -> CombatAction {
        if (act.id() == "b" && !*b_acted) { *b_acted = true; return CombatAction::attack("c"); }
        return CombatAction::pass();
    }, nullptr, nullptr, &rng);
    sm.begin_turn(); sm.run_active_turn_to_end(); sm.advance_to_next_actor();  // a passa
    sm.begin_turn();  // b tick empurra -> [a, c, b]; cursor segue b
    REQUIRE(sm.active_actor()->id() == "b");
    sm.run_active_turn_to_end();  // b ataca c
    REQUIRE(order_ids(sm) == std::vector<std::string>{"a", "c", "b"});
    REQUIRE(*b_acted);
    REQUIRE(c.max_hp() - c.hp() == 10);
}

TEST_CASE("status: haste no proprio tick nao faz o ator perder o turno",
          "[domain][combat][status]") {
    CombatActor h = hero("gus", 100, /*spd=*/30, /*atk=*/10);
    CombatActor e = foe("enemy", 200, /*spd=*/20, 6, /*def=*/0);
    h.add_status(effect(StatusId::Haste, 15, 3, CardFamily::Eletrico));
    FixedRandom rng;
    CombatStateMachine sm({&h, &e}, play_once(CombatAction::attack(e.id())), nullptr, nullptr,
                          &rng);
    sm.begin_turn();  // tick Haste recomputa; cursor segue hero
    REQUIRE(sm.active_actor()->id() == "gus");
    sm.run_active_turn_to_end();
    REQUIRE(e.max_hp() - e.hp() == 10);
}

// ===== DECRYPT =====

TEST_CASE("status: decrypt remove buffs existentes no tick", "[domain][combat][status]") {
    CombatActor f = foe("enemy", 500, /*spd=*/10);
    CombatActor h = hero("gus", 100, /*spd=*/5);
    f.add_status(effect(StatusId::Shield, 20, 5, CardFamily::Eletrico));
    f.add_status(effect(StatusId::Regen, 3, 5, CardFamily::Bioquimico));
    f.add_status(effect(StatusId::Decrypt, 0, 2, CardFamily::Criptografico));
    FixedRandom rng;
    CombatStateMachine sm({&h, &f}, [](CombatActor&, const CombatState&) { return CombatAction::pass(); },
                          nullptr, nullptr, &rng);
    sm.begin_turn();  // foe tick
    REQUIRE(find_status(f, StatusId::Shield) == nullptr);
    REQUIRE(find_status(f, StatusId::Regen) == nullptr);
    REQUIRE(find_status(f, StatusId::Decrypt) != nullptr);
}

TEST_CASE("status: decrypt remove TODOS os buffs do alvo", "[domain][combat][status]") {
    CombatActor f = foe("enemy", 500, /*spd=*/10);
    CombatActor h = hero("gus", 100, /*spd=*/5);
    f.add_status(effect(StatusId::Shield, 20, 5, CardFamily::Eletrico));
    f.add_status(effect(StatusId::Regen, 3, 5, CardFamily::Bioquimico));
    f.add_status(effect(StatusId::Haste, 7, 5, CardFamily::Eletrico));
    f.add_status(effect(StatusId::Decrypt, 0, 2, CardFamily::Criptografico));
    FixedRandom rng;
    CombatStateMachine sm({&h, &f}, [](CombatActor&, const CombatState&) { return CombatAction::pass(); },
                          nullptr, nullptr, &rng);
    sm.begin_turn();
    for (const auto& s : f.status_effects())
        REQUIRE_FALSE(CombatActor::is_buff(s.id));
    REQUIRE(find_status(f, StatusId::Shield) == nullptr);
    REQUIRE(find_status(f, StatusId::Regen) == nullptr);
    REQUIRE(find_status(f, StatusId::Haste) == nullptr);
    REQUIRE(find_status(f, StatusId::Decrypt) != nullptr);
    REQUIRE(f.spd() == 10);  // Haste dispelado: SPD restaurado
}

TEST_CASE("status: decrypt nao bloqueia reaplicacao de buff", "[domain][combat][status]") {
    CombatActor f = foe("enemy", 500, /*spd=*/10);
    CombatActor h = hero("gus", 100, /*spd=*/5);
    f.add_status(effect(StatusId::Shield, 20, 5, CardFamily::Eletrico));
    f.add_status(effect(StatusId::Decrypt, 0, 3, CardFamily::Criptografico));
    FixedRandom rng;
    CombatStateMachine sm({&h, &f}, [](CombatActor& a, const CombatState&) {
        return !a.is_player_side() ? CombatAction::defend() : CombatAction::pass();
    }, nullptr, nullptr, &rng);
    sm.begin_turn();
    REQUIRE(find_status(f, StatusId::Shield) == nullptr);
    sm.run_active_turn_to_end();  // foe defende -> Shield reaplicado
    REQUIRE(find_status(f, StatusId::Shield) != nullptr);
    REQUIRE(find_status(f, StatusId::Decrypt) != nullptr);
}

TEST_CASE("status: decrypt nao bloqueia debuffs", "[domain][combat][status]") {
    CombatActor f = foe("enemy", 500, /*spd=*/10, 6, 4, CardFamily::Eletrico);
    CombatActor h = hero("gus", 100, /*spd=*/20, /*atk=*/0, 4, CardFamily::Bioquimico);
    f.add_status(effect(StatusId::Decrypt, 0, 3, CardFamily::Criptografico));
    StatusEffect poison = effect(StatusId::Poison, 4, 3, CardFamily::Bioquimico);
    Card card = make_card("raiz.toxica", CardFamily::Bioquimico, 5, 1, poison);
    auto reg = registry(card);
    FixedRandom rng;
    CombatStateMachine sm({&h, &f}, play_once(CombatAction::use_card(card.id, f.id())), &reg,
                          nullptr, &rng);
    sm.begin_turn();  // hero age primeiro (spd 20)
    sm.run_active_turn_to_end();
    REQUIRE(find_status(f, StatusId::Poison) != nullptr);
}

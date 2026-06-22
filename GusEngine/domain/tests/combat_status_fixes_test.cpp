// combat_status_fixes_test.cpp
//
// Spec executavel (Catch2 v3) das 4 correcoes de combate (criador 2026-05-26):
//   FIX 1 (BUG-1): ator morto por tick (Poison/Corrode letal) NAO age no ramo nao-stunned.
//   FIX 2 (BUG-3): Shield absorve TODO dano antes do HP; pool decrementa; expira ao zerar.
//   FIX 3 (BUG-4): Expose multiplica dano de UseCard por (1 + Mag/100), no fim da cadeia.
//   FIX 4 (secao 16): FSM acumula StatusEffectChange (Applied/Expired/Absorbed) pos-turno.
// Portado de engine/tests/turn_combat/CombatStatusFixesTests.cs. POCO puro, ZERO Qt.
//
// Cross-ref: engine/tests/turn_combat/CombatStatusFixesTests.cs;
//            docs/design/mecanicas/combat.md secao 9/11/16.

#include <catch2/catch_test_macros.hpp>

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

using namespace gus::domain::combat;
using gus::domain::tests::FixedRandom;

namespace {

CombatActor hero(const std::string& id = "gus", int hp = 50, int spd = 20, int atk = 8,
                 int def = 2, CardFamily family = CardFamily::Eletrico) {
    return CombatActor(id, id, hp, atk, def, spd, family, /*player=*/true);
}

CombatActor foe(const std::string& id = "enemy", int hp = 500, int spd = 10, int atk = 6,
                int def = 1, CardFamily family = CardFamily::Cinetico) {
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

bool has_change(const CombatStateMachine& sm, const std::string& actor, StatusId id,
                StatusChangeKind kind) {
    for (const auto& c : sm.status_changes())
        if (c.actor_id == actor && c.id == id && c.kind == kind) return true;
    return false;
}

}  // namespace

// ===== FIX 1 (BUG-1) =====

TEST_CASE("fix1: ator morto por poison no tick nao executa acao",
          "[domain][combat][fixes]") {
    CombatActor h = hero("gus", 50, 20, /*atk=*/10);
    CombatActor e = foe("enemy", 100, 10, 6, /*def=*/0);
    h.add_status({StatusId::Poison, 50, 3, StackRule::Replace, CardFamily::Bioquimico});
    int calls = 0;
    CombatStateMachine sm({&h, &e}, [&](CombatActor&, const CombatState&) {
        ++calls;
        return CombatAction::attack(e.id());
    });
    const bool stunned = sm.begin_turn();
    REQUIRE_FALSE(stunned);
    REQUIRE_FALSE(h.is_alive());
    if (!stunned && h.is_alive()) sm.run_active_turn_to_end();
    REQUIRE(calls == 0);
    REQUIRE(e.hp() == 100);
}

TEST_CASE("fix1: run_until_end ator morto por tick nao age e combate resolve",
          "[domain][combat][fixes]") {
    CombatActor h = hero("gus", 30, 20, /*atk=*/99);
    CombatActor e = foe("enemy", 100, 10, 6, /*def=*/0);
    h.add_status({StatusId::Poison, 30, 5, StackRule::Replace, CardFamily::Bioquimico});
    int player_calls = 0;
    CombatStateMachine sm({&h, &e}, [&](CombatActor& a, const CombatState&) {
        if (a.is_player_side()) ++player_calls;
        return CombatAction::attack(a.is_player_side() ? e.id() : h.id());
    });
    const CombatResult result = sm.run_until_end();
    REQUIRE(player_calls == 0);
    REQUIRE(e.hp() == 100);
    REQUIRE(result.outcome == CombatOutcome::Defeat);
}

// ===== FIX 2 (BUG-3): Shield (logica em CombatActor, exercitada aqui) =====

TEST_CASE("fix2: shield absorve parcial quando magnitude maior que dano",
          "[domain][combat][fixes]") {
    CombatActor h = hero("gus", 100);
    h.add_status({StatusId::Shield, 20, 5, StackRule::Replace, CardFamily::Eletrico});
    h.take_damage(8);
    REQUIRE(h.hp() == 100);
    REQUIRE(find_status(h, StatusId::Shield)->magnitude == 12);
}

TEST_CASE("fix2: shield absorve total e overflow vai pro hp", "[domain][combat][fixes]") {
    CombatActor h = hero("gus", 100);
    h.add_status({StatusId::Shield, 5, 5, StackRule::Replace, CardFamily::Eletrico});
    h.take_damage(12);
    REQUIRE(h.hp() == 93);
    REQUIRE(find_status(h, StatusId::Shield) == nullptr);
}

TEST_CASE("fix2: shield depleao exata remove o shield", "[domain][combat][fixes]") {
    CombatActor h = hero("gus", 100);
    h.add_status({StatusId::Shield, 10, 5, StackRule::Replace, CardFamily::Eletrico});
    h.take_damage(10);
    REQUIRE(h.hp() == 100);
    REQUIRE(find_status(h, StatusId::Shield) == nullptr);
}

TEST_CASE("fix2: shield multiplos hits drenam o pool", "[domain][combat][fixes]") {
    CombatActor h = hero("gus", 100);
    h.add_status({StatusId::Shield, 10, 5, StackRule::Replace, CardFamily::Eletrico});
    h.take_damage(3);
    h.take_damage(4);
    REQUIRE(h.hp() == 100);
    REQUIRE(find_status(h, StatusId::Shield)->magnitude == 3);
    h.take_damage(5);
    REQUIRE(h.hp() == 98);
    REQUIRE(find_status(h, StatusId::Shield) == nullptr);
}

TEST_CASE("fix2: defend deixa de ser no-op absorve dano recebido",
          "[domain][combat][fixes]") {
    CombatActor h = hero("gus", 100, /*spd=*/100, 8, /*def=*/5);
    CombatActor e = foe("enemy", 100, /*spd=*/1, /*atk=*/20, /*def=*/0);
    auto hero_acted = std::make_shared<bool>(false);
    auto foe_acted = std::make_shared<bool>(false);
    CombatStateMachine sm({&h, &e}, [hero_acted, foe_acted, &h](CombatActor& a, const CombatState&) -> CombatAction {
        if (a.is_player_side()) {
            if (*hero_acted) return CombatAction::pass();
            *hero_acted = true;
            return CombatAction::defend();
        }
        if (*foe_acted) return CombatAction::pass();
        *foe_acted = true;
        return CombatAction::attack(h.id());
    });
    sm.begin_turn(); sm.run_active_turn_to_end(); sm.advance_to_next_actor();  // hero defende
    sm.begin_turn(); sm.run_active_turn_to_end();                              // foe ataca
    REQUIRE(h.hp() == 90);  // dano bruto 15, shield 5 absorve, 10 ao HP
}

TEST_CASE("fix2: shield absorve dano de tick dot", "[domain][combat][fixes]") {
    CombatActor h = hero("gus", 100);
    CombatActor e = foe();
    h.add_status({StatusId::Shield, 10, 5, StackRule::Replace, CardFamily::Eletrico});
    h.add_status({StatusId::Poison, 6, 5, StackRule::Replace, CardFamily::Bioquimico});
    CombatStateMachine sm({&h, &e}, [](CombatActor&, const CombatState&) { return CombatAction::pass(); });
    sm.begin_turn();  // tick Poison absorvido pelo Shield
    REQUIRE(h.hp() == 100);
    REQUIRE(find_status(h, StatusId::Shield)->magnitude == 4);
}

// ===== FIX 3 (BUG-4): Expose =====

TEST_CASE("fix3: expose 30 multiplica dano de usecard por 1.3",
          "[domain][combat][fixes]") {
    CombatActor h = hero("gus", 50, 20, /*atk=*/0, 2, CardFamily::Eletrico);
    CombatActor e = foe("enemy", 100, 10, 6, /*def=*/0, CardFamily::Eletrico);
    e.add_status({StatusId::Expose, 30, 5, StackRule::Replace, CardFamily::Criptografico});
    Card card = make_card("pulso.eletrico", CardFamily::Eletrico, 10);
    auto reg = registry(card);
    FixedRandom rng;
    CombatStateMachine sm({&h, &e}, play_once(CombatAction::use_card(card.id, e.id())), &reg,
                          nullptr, &rng);
    sm.begin_turn();
    sm.run_active_turn_to_end();
    REQUIRE(e.max_hp() - e.hp() == 13);  // 10 * 1.3
}

TEST_CASE("fix3: sem expose dano normal x1.0", "[domain][combat][fixes]") {
    CombatActor h = hero("gus", 50, 20, /*atk=*/0, 2, CardFamily::Eletrico);
    CombatActor e = foe("enemy", 100, 10, 6, /*def=*/0, CardFamily::Eletrico);
    Card card = make_card("pulso.eletrico", CardFamily::Eletrico, 10);
    auto reg = registry(card);
    FixedRandom rng;
    CombatStateMachine sm({&h, &e}, play_once(CombatAction::use_card(card.id, e.id())), &reg,
                          nullptr, &rng);
    sm.begin_turn();
    sm.run_active_turn_to_end();
    REQUIRE(e.max_hp() - e.hp() == 10);
}

TEST_CASE("fix3: expose interage com mult_fraqueza na ordem do produto",
          "[domain][combat][fixes]") {
    CombatActor h = hero("gus", 50, 20, /*atk=*/0, 2, CardFamily::Eletrico);
    CombatActor e = foe("enemy", 100, 10, 6, /*def=*/0, CardFamily::Cinetico);
    e.add_status({StatusId::Expose, 40, 5, StackRule::Replace, CardFamily::Criptografico});
    Card card = make_card("pulso.eletrico", CardFamily::Eletrico, 10);
    auto reg = registry(card);
    FixedRandom rng;
    CombatStateMachine sm({&h, &e}, play_once(CombatAction::use_card(card.id, e.id())), &reg,
                          nullptr, &rng);
    sm.begin_turn();
    sm.run_active_turn_to_end();
    REQUIRE(e.max_hp() - e.hp() == 21);  // 10 * 1.5 * 1.4
}

TEST_CASE("fix3: expose nao afeta ataque basico", "[domain][combat][fixes]") {
    CombatActor h = hero("gus", 50, 20, /*atk=*/8);
    CombatActor e = foe("enemy", 100, 10, 6, /*def=*/2);
    e.add_status({StatusId::Expose, 50, 5, StackRule::Replace, CardFamily::Criptografico});
    FixedRandom rng;
    CombatStateMachine sm({&h, &e}, play_once(CombatAction::attack(e.id())), nullptr, nullptr,
                          &rng);
    sm.begin_turn();
    sm.run_active_turn_to_end();
    REQUIRE(e.max_hp() - e.hp() == 6);  // 8 - 2
}

// ===== FIX 4 (secao 16): StatusEffectChange =====

TEST_CASE("fix4: aplicar status via carta registra Applied", "[domain][combat][fixes]") {
    StatusEffect poison{StatusId::Poison, 4, 3, StackRule::Replace, CardFamily::Bioquimico};
    CombatActor h = hero("gus", 50, 20, /*atk=*/0, 2, CardFamily::Bioquimico);
    CombatActor e = foe("enemy", 100, 10, 6, /*def=*/0, CardFamily::Cinetico);
    Card card = make_card("raiz.toxica", CardFamily::Bioquimico, 5, 1, poison);
    auto reg = registry(card);
    FixedRandom rng;
    CombatStateMachine sm({&h, &e}, play_once(CombatAction::use_card(card.id, e.id())), &reg,
                          nullptr, &rng);
    sm.begin_turn();
    sm.run_active_turn_to_end();
    bool ok = false;
    for (const auto& c : sm.status_changes())
        if (c.actor_id == e.id() && c.id == StatusId::Poison &&
            c.kind == StatusChangeKind::Applied && c.magnitude == 4 && c.duration == 3)
            ok = true;
    REQUIRE(ok);
}

TEST_CASE("fix4: defend registra shield Applied", "[domain][combat][fixes]") {
    CombatActor h = hero("gus", 50, 20, 8, /*def=*/7);
    CombatActor e = foe();
    CombatStateMachine sm({&h, &e}, play_once(CombatAction::defend()));
    sm.begin_turn();
    sm.run_active_turn_to_end();
    bool ok = false;
    for (const auto& c : sm.status_changes())
        if (c.actor_id == h.id() && c.id == StatusId::Shield &&
            c.kind == StatusChangeKind::Applied && c.magnitude == 7)
            ok = true;
    REQUIRE(ok);
}

TEST_CASE("fix4: expiracao no TurnEnd registra Expired", "[domain][combat][fixes]") {
    CombatActor h = hero("gus", 50);
    CombatActor e = foe();
    h.add_status({StatusId::Poison, 1, 1, StackRule::Replace, CardFamily::Bioquimico});
    CombatStateMachine sm({&h, &e}, [](CombatActor&, const CombatState&) { return CombatAction::pass(); });
    sm.begin_turn();
    sm.run_active_turn_to_end();
    REQUIRE(has_change(sm, h.id(), StatusId::Poison, StatusChangeKind::Expired));
}

TEST_CASE("fix4: shield absorcao registra Absorbed", "[domain][combat][fixes]") {
    CombatActor h = hero("gus", 100);
    CombatActor e = foe();
    h.add_status({StatusId::Shield, 10, 5, StackRule::Replace, CardFamily::Eletrico});
    h.add_status({StatusId::Poison, 4, 5, StackRule::Replace, CardFamily::Bioquimico});
    CombatStateMachine sm({&h, &e}, [](CombatActor&, const CombatState&) { return CombatAction::pass(); });
    sm.begin_turn();
    REQUIRE(has_change(sm, h.id(), StatusId::Shield, StatusChangeKind::Absorbed));
}

TEST_CASE("fix4: shield depleao registra Expired", "[domain][combat][fixes]") {
    CombatActor h = hero("gus", 100);
    CombatActor e = foe();
    h.add_status({StatusId::Shield, 4, 5, StackRule::Replace, CardFamily::Eletrico});
    h.add_status({StatusId::Poison, 10, 5, StackRule::Replace, CardFamily::Bioquimico});
    CombatStateMachine sm({&h, &e}, [](CombatActor&, const CombatState&) { return CombatAction::pass(); });
    sm.begin_turn();
    REQUIRE(h.hp() == 94);  // 100 - 6
    REQUIRE(has_change(sm, h.id(), StatusId::Shield, StatusChangeKind::Expired));
}

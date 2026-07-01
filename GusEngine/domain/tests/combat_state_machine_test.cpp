// combat_state_machine_test.cpp
//
// Spec executavel (Catch2 v3) da FSM de combate (secao 3): transicoes, loop de AP em
// ActionSelect, recarga de AP/mana em TurnStart (ramp + cap + sem carry-over), CheckEnd
// (3 condicoes), CombatEnd outcome. Portado de
// engine/tests/turn_combat/CombatStateMachineTests.cs. POCO puro, ZERO Qt, headless.
//
// A FSM e POCO testavel sem Godot. A selecao de acao e injetada via std::function
// (CombatActionProvider) pra exercitar o loop interno de forma deterministica. Os atores
// vivem no escopo do teste (locais); a FSM/fila guardam ponteiros NAO-DONOS (mesmo padrao
// de InitiativeQueue/CombatState ja portados).
//
// Cross-ref: engine/tests/turn_combat/CombatStateMachineTests.cs;
//            docs/design/mecanicas/combat.md secao 3/4/5.

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <string>
#include <vector>

#include "gus/domain/combat/combat_actor.hpp"
#include "gus/domain/combat/combat_constants.hpp"  // kMinDamage (piso do dano/previa)
#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/combat/combat_records.hpp"
#include "gus/domain/combat/combat_state.hpp"
#include "gus/domain/combat/combat_state_machine.hpp"

using namespace gus::domain::combat;

namespace {

CombatActor hero(const std::string& id = "gus", int hp = 30, int spd = 20, int atk = 8,
                 int def = 2) {
    return CombatActor(id, id, hp, atk, def, spd, CardFamily::Eletrico, /*player=*/true);
}

CombatActor foe(const std::string& id = "enemy", int hp = 30, int spd = 10, int atk = 6,
                int def = 1) {
    return CombatActor(id, id, hp, atk, def, spd, CardFamily::Cinetico, /*player=*/false);
}

CombatAction always_pass(CombatActor&, const CombatState&) { return CombatAction::pass(); }

std::vector<std::string> order_ids(const CombatStateMachine& sm) {
    std::vector<std::string> ids;
    for (const CombatActor* a : sm.queue().order())
        ids.push_back(a->id());
    return ids;
}

bool log_has(const CombatStateMachine& sm, CombatActionType action,
             const std::string& needle) {
    for (const auto& e : sm.log()) {
        if (e.action == action && e.message.find(needle) != std::string::npos)
            return true;
    }
    return false;
}

}  // namespace

// ----- Setup / fase inicial -----

TEST_CASE("fsm: SetupPhase monta fila por spd e inicia em TurnStart resolvido",
          "[domain][combat][fsm]") {
    CombatActor h = hero("gus", 30, 20);
    CombatActor e = foe("enemy", 30, 10);
    CombatStateMachine sm({&h, &e}, always_pass);

    REQUIRE(order_ids(sm) == std::vector<std::string>{"gus", "enemy"});
    REQUIRE(sm.outcome() == CombatOutcome::Ongoing);
}

// ----- TurnStart: AP e mana -----

TEST_CASE("fsm: TurnStart recarrega ap para 3", "[domain][combat][fsm]") {
    CombatActor h = hero();
    CombatActor e = foe();
    CombatStateMachine sm({&h, &e}, always_pass);

    sm.begin_turn();
    REQUIRE(sm.active_actor()->ap() == 3);
    REQUIRE(sm.active_actor()->max_ap() == 3);
}

TEST_CASE("fsm: TurnStart mana ramp 2 mais turno_index", "[domain][combat][fsm]") {
    CombatActor h = hero();
    CombatActor e = foe();
    CombatStateMachine sm({&h, &e}, always_pass);

    sm.begin_turn();
    REQUIRE(sm.active_actor()->max_mana() == 2);
    REQUIRE(sm.active_actor()->mana() == 2);
}

TEST_CASE("fsm: mana sobe com o avanco das rodadas", "[domain][combat][fsm]") {
    CombatActor h = hero();
    CombatStateMachine sm({&h}, always_pass);

    sm.begin_turn();
    REQUIRE(h.max_mana() == 2);
    sm.run_active_turn_to_end();
    sm.advance_to_next_actor();

    sm.begin_turn();
    REQUIRE(h.max_mana() == 3);
    sm.run_active_turn_to_end();
    sm.advance_to_next_actor();

    sm.begin_turn();
    REQUIRE(h.max_mana() == 4);
}

TEST_CASE("fsm: mana capa em 8", "[domain][combat][fsm]") {
    CombatActor h = hero();
    CombatStateMachine sm({&h}, always_pass);

    for (int i = 0; i < 8; ++i) {
        sm.begin_turn();
        sm.run_active_turn_to_end();
        sm.advance_to_next_actor();
    }
    sm.begin_turn();
    REQUIRE(h.max_mana() == 8);
}

TEST_CASE("fsm: mana nao tem carry-over recarrega ao maximo", "[domain][combat][fsm]") {
    CombatActor h = hero();
    CombatStateMachine sm({&h}, always_pass);

    sm.begin_turn();
    h.spend_mana(2);
    REQUIRE(h.mana() == 0);
    sm.run_active_turn_to_end();
    sm.advance_to_next_actor();

    sm.begin_turn();
    REQUIRE(h.mana() == h.max_mana());
    REQUIRE(h.mana() == 3);
}

// ----- ActionSelect: loop de AP -----

TEST_CASE("fsm: ActionSelect consome 3 attacks ate ap zerar", "[domain][combat][fsm]") {
    CombatActor h = hero("gus", 30, 20, /*atk=*/10);
    CombatActor e = foe("enemy", 100, 10, 6, /*def=*/0);
    int attacks = 0;

    CombatStateMachine sm({&h, &e}, [&](CombatActor&, const CombatState&) {
        ++attacks;
        return CombatAction::attack(e.id());
    });
    sm.begin_turn();
    sm.run_active_turn_to_end();

    REQUIRE(attacks == 3);
    REQUIRE(h.ap() == 0);
    REQUIRE(e.hp() == 70);
}

TEST_CASE("fsm: pass encerra turno imediatamente sem gastar ap",
          "[domain][combat][fsm]") {
    CombatActor h = hero();
    CombatActor e = foe();
    CombatStateMachine sm({&h, &e}, always_pass);
    sm.begin_turn();
    sm.run_active_turn_to_end();
    REQUIRE(h.ap() == 3);
}

TEST_CASE("fsm: para quando provider devolve pass no meio", "[domain][combat][fsm]") {
    CombatActor h = hero("gus", 30, 20, 10);
    CombatActor e = foe("enemy", 100, 10, 6, 0);
    int count = 0;
    CombatStateMachine sm({&h, &e}, [&](CombatActor&, const CombatState&) {
        ++count;
        return count == 1 ? CombatAction::attack(e.id()) : CombatAction::pass();
    });
    sm.begin_turn();
    sm.run_active_turn_to_end();

    REQUIRE(h.ap() == 2);
    REQUIRE(e.hp() == 90);
}

// ----- Ataque basico stub (clamp min 1) -----

TEST_CASE("fsm: attack stub dano e atk menos def clamp minimo 1",
          "[domain][combat][fsm]") {
    CombatActor h = hero("gus", 30, 20, /*atk=*/5);
    CombatActor e = foe("enemy", 50, 10, 6, /*def=*/99);
    CombatStateMachine sm({&h, &e}, [&](CombatActor& a, const CombatState&) {
        return &a == &h ? CombatAction::attack(e.id()) : CombatAction::pass();
    });
    sm.begin_turn();
    sm.run_active_turn_to_end();
    REQUIRE(e.hp() == 47);  // 3 ataques de clamp 1
}

// ----- CheckEnd: 3 condicoes -----

TEST_CASE("fsm: CheckEnd victory quando todos inimigos mortos",
          "[domain][combat][fsm]") {
    CombatActor h = hero("gus", 30, 20, /*atk=*/1000);
    CombatActor e = foe("enemy", 5, 10, 6, /*def=*/0);
    CombatStateMachine sm({&h, &e}, [&](CombatActor& a, const CombatState&) {
        return &a == &h ? CombatAction::attack(e.id()) : CombatAction::pass();
    });
    sm.run_until_end();
    REQUIRE(sm.outcome() == CombatOutcome::Victory);
}

TEST_CASE("fsm: CheckEnd defeat quando party inteira morta",
          "[domain][combat][fsm]") {
    CombatActor h = hero("gus", /*hp=*/1, /*spd=*/1, 8, /*def=*/0);
    CombatActor e = foe("enemy", 30, /*spd=*/99, /*atk=*/50, 1);
    CombatStateMachine sm({&h, &e}, [&](CombatActor& a, const CombatState&) {
        return a.is_player_side() ? CombatAction::pass() : CombatAction::attack(h.id());
    });
    sm.run_until_end();
    REQUIRE(sm.outcome() == CombatOutcome::Defeat);
}

TEST_CASE("fsm: CheckEnd fled quando fuga bem-sucedida", "[domain][combat][fsm]") {
    CombatActor h = hero("gus", 30, /*spd=*/100);
    CombatActor e = foe("enemy", 30, /*spd=*/1);
    CombatStateMachine sm({&h, &e}, [&](CombatActor& a, const CombatState&) {
        return a.is_player_side() ? CombatAction::flee() : CombatAction::pass();
    });
    sm.run_until_end();
    REQUIRE(sm.outcome() == CombatOutcome::Fled);
}

// ----- CombatEnd -----

TEST_CASE("fsm: CombatEnd produz resultado com outcome e log",
          "[domain][combat][fsm]") {
    CombatActor h = hero("gus", 30, 20, /*atk=*/1000);
    CombatActor e = foe("enemy", 5, 10, 6, /*def=*/0);
    CombatStateMachine sm({&h, &e}, [&](CombatActor& a, const CombatState&) {
        return &a == &h ? CombatAction::attack(e.id()) : CombatAction::pass();
    });
    const CombatResult result = sm.run_until_end();
    REQUIRE(result.outcome == CombatOutcome::Victory);
    REQUIRE_FALSE(result.log.empty());
    REQUIRE(sm.phase() == CombatPhase::CombatEnd);
}

TEST_CASE("fsm: inimigo morto e removido da fila", "[domain][combat][fsm]") {
    CombatActor h = hero("gus", 30, 20, /*atk=*/1000);
    CombatActor e = foe("enemy", 5, 10, 6, /*def=*/0);
    CombatStateMachine sm({&h, &e}, [&](CombatActor& a, const CombatState&) {
        return &a == &h ? CombatAction::attack(e.id()) : CombatAction::pass();
    });
    sm.run_until_end();
    const auto ids = order_ids(sm);
    REQUIRE(std::find(ids.begin(), ids.end(), "enemy") == ids.end());
    REQUIRE_FALSE(log_has(sm, CombatActionType::Pass, "nunca"));  // sanity (no-op)
}

// ----- Previa de dano do ataque basico (modo-mira §3.5): PURA e read-only -----
// Espelha resolve_basic_attack (dano bruto = max(kMinDamage, atk - def)) + a absorcao de
// Shield (absorb_with_shield) SEM mutar nada. A UI mostra este numero no modo-mira antes
// de confirmar. Ver preview_basic_attack_damage.

namespace {
// Aplica um Shield de pool `magnitude` num ator (mesmo shape do resolve_defend).
void give_shield(CombatActor& a, int magnitude) {
    a.add_status(StatusEffect{StatusId::Shield, magnitude, /*duration=*/1,
                              StackRule::Replace, CardFamily::Eletrico});
}
}  // namespace

TEST_CASE("preview: (a) SEM shield o dano previsto e atk - def", "[domain][combat][fsm]") {
    CombatActor h = hero("gus", 30, 20, /*atk=*/8, /*def=*/2);
    CombatActor e = foe("enemy", 30, 10, /*atk=*/6, /*def=*/3);
    CombatStateMachine sm({&h, &e}, always_pass);

    // raw = max(1, 8 - 3) = 5; sem Shield => perda de HP prevista = 5.
    REQUIRE(sm.preview_basic_attack_damage(h, e) == 5);

    // READ-ONLY: a previa NAO tocou o alvo nem a maquina (HP cheio, sem status, sem log).
    REQUIRE(e.hp() == 30);
    REQUIRE(e.status_effects().empty());
    REQUIRE(sm.log().empty());
}

TEST_CASE("preview: (b) shield PARCIAL reduz a perda prevista", "[domain][combat][fsm]") {
    CombatActor h = hero("gus", 30, 20, /*atk=*/10, /*def=*/2);
    CombatActor e = foe("enemy", 30, 10, /*atk=*/6, /*def=*/2);
    give_shield(e, /*magnitude=*/3);
    CombatStateMachine sm({&h, &e}, always_pass);

    // raw = max(1, 10 - 2) = 8; Shield 3 absorve => perda prevista = 8 - 3 = 5.
    REQUIRE(sm.preview_basic_attack_damage(h, e) == 5);

    // READ-ONLY: o Shield NAO foi consumido pela previa (magnitude segue 3) e o HP segue cheio.
    const int idx = e.index_of_status(StatusId::Shield);
    REQUIRE(idx >= 0);
    REQUIRE(e.status_effects()[static_cast<std::size_t>(idx)].magnitude == 3);
    REQUIRE(e.hp() == 30);
}

TEST_CASE("preview: (c) shield que ABSORVE TUDO zera a perda prevista",
          "[domain][combat][fsm]") {
    CombatActor h = hero("gus", 30, 20, /*atk=*/7, /*def=*/2);
    CombatActor e = foe("enemy", 30, 10, /*atk=*/6, /*def=*/2);
    give_shield(e, /*magnitude=*/50);
    CombatStateMachine sm({&h, &e}, always_pass);

    // raw = max(1, 7 - 2) = 5; Shield 50 >= 5 => perda prevista = max(0, 5 - 50) = 0.
    REQUIRE(sm.preview_basic_attack_damage(h, e) == 0);

    // READ-ONLY: Shield intacto (magnitude 50), HP cheio.
    const int idx = e.index_of_status(StatusId::Shield);
    REQUIRE(idx >= 0);
    REQUIRE(e.status_effects()[static_cast<std::size_t>(idx)].magnitude == 50);
    REQUIRE(e.hp() == 30);
}

TEST_CASE("preview: (d) piso kMinDamage quando atk < def", "[domain][combat][fsm]") {
    CombatActor h = hero("gus", 30, 20, /*atk=*/5, /*def=*/2);
    CombatActor e = foe("enemy", 30, 10, /*atk=*/6, /*def=*/99);
    CombatStateMachine sm({&h, &e}, always_pass);

    // atk 5 < def 99 => raw = max(kMinDamage, 5 - 99) = kMinDamage (1); sem Shield => 1.
    REQUIRE(sm.preview_basic_attack_damage(h, e) == combat_constants::kMinDamage);
    REQUIRE(sm.preview_basic_attack_damage(h, e) == 1);

    // Piso + Shield: raw 1 absorvido por um Shield de 1 => perda prevista = 0.
    give_shield(e, /*magnitude=*/1);
    REQUIRE(sm.preview_basic_attack_damage(h, e) == 0);
}

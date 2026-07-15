// combat_inc3_test.cpp
//
// Spec executavel (Catch2 v3) do incremento 3 (secao 12): Gambito Prever (1 AP, le
// IntentPreview via brain_registry) e Gambito Reordenar (2 AP, chama reorder_actor), mais
// a infra de brain_registry na FSM. Portado de engine/tests/turn_combat/CombatInc3Tests.cs.
// POCO puro, ZERO Qt.
//
// MAPEAMENTO de excecoes: InvalidOperationException -> std::logic_error;
//                         KeyNotFoundException      -> std::out_of_range.
//
// Cross-ref: engine/tests/turn_combat/CombatInc3Tests.cs;
//            docs/design/mecanicas/combat.md secao 12/13.

#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "gus/domain/combat/combat_actor.hpp"
#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/combat/combat_records.hpp"
#include "gus/domain/combat/combat_state.hpp"
#include "gus/domain/combat/combat_state_machine.hpp"
#include "gus/domain/combat/enemy_brain.hpp"

using namespace gus::domain::combat;

namespace {

CombatActor hero(const std::string& id = "gus", int hp = 50, int spd = 20, int atk = 8,
                 int def = 2, CardFamily family = CardFamily::Eletrico) {
    return CombatActor(id, id, hp, atk, def, spd, family, /*player=*/true);
}

CombatActor foe(const std::string& id = "enemy", int hp = 50, int spd = 10, int atk = 6,
                int def = 1, CardFamily family = CardFamily::Cinetico) {
    return CombatActor(id, id, hp, atk, def, spd, family, /*player=*/false);
}

// Brain stub configuravel: devolve o IntentPreview que o teste mandar.
class FakeBrain final : public IEnemyBrain {
public:
    explicit FakeBrain(IntentPreview preview) : preview_(std::move(preview)) {}
    IntentPreview preview_intent(const CombatState&, const CombatActor&) override {
        return preview_;
    }
    CombatAction decide_action(const CombatState&, const CombatActor&) override {
        return CombatAction::pass();
    }

private:
    IntentPreview preview_;
};

CombatActionProvider play_once(CombatAction action) {
    auto done = std::make_shared<bool>(false);
    return [action, done](CombatActor& a, const CombatState&) -> CombatAction {
        if (!a.is_player_side() || *done) return CombatAction::pass();
        *done = true;
        return action;
    };
}

std::vector<std::string> order_ids(const CombatStateMachine& sm) {
    std::vector<std::string> ids;
    for (const CombatActor* a : sm.queue().order()) ids.push_back(a->id());
    return ids;
}

bool log_has(const CombatStateMachine& sm, CombatActionType action,
             const std::string& needle) {
    for (const auto& e : sm.log())
        if (e.action == action && e.message.find(needle) != std::string::npos) return true;
    return false;
}

}  // namespace

// ===== GambitPredict =====

TEST_CASE("inc3: gambit predict com brain seta LastPrediction e gasta 1 ap",
          "[domain][combat][inc3]") {
    CombatActor h = hero();
    CombatActor e = foe("enemy");
    FakeBrain brain(IntentPreview{"enemy", "attack", 7, TargetShape::Single, "gus", false});
    std::unordered_map<std::string, IEnemyBrain*> brains{{e.id(), &brain}};

    CombatStateMachine sm({&h, &e}, play_once(CombatAction::gambit_predict(e.id())), nullptr,
                          &brains);
    sm.begin_turn();
    sm.run_active_turn_to_end();

    REQUIRE(sm.last_prediction().has_value());
    REQUIRE(sm.last_prediction()->actor_id == "enemy");
    REQUIRE(sm.last_prediction()->predicted_action_id == "attack");
    REQUIRE(sm.last_prediction()->predicted_damage == 7);
    REQUIRE(sm.last_prediction()->predicted_target_id == "gus");
    REQUIRE_FALSE(sm.last_prediction()->is_chaotic);
    REQUIRE(h.ap() == 2);
    REQUIRE(log_has(sm, CombatActionType::GambitPredict, "Gambito Prever"));
    REQUIRE(log_has(sm, CombatActionType::GambitPredict, "[LEGIVEL]"));
}

TEST_CASE("inc3: gambit predict sem brain lanca out_of_range", "[domain][combat][inc3]") {
    CombatActor h = hero();
    CombatActor e = foe("enemy");
    CombatStateMachine sm({&h, &e}, [&](CombatActor& a, const CombatState&) {
        return a.is_player_side() ? CombatAction::gambit_predict(e.id()) : CombatAction::pass();
    });
    sm.begin_turn();
    REQUIRE_THROWS_AS(sm.run_active_turn_to_end(), std::out_of_range);
}

TEST_CASE("inc3: gambit predict intent caotico loga caotico", "[domain][combat][inc3]") {
    CombatActor h = hero();
    CombatActor e = foe("miniboss");
    FakeBrain brain(IntentPreview{"miniboss", "noise", 0, TargetShape::Single, "gus", true});
    std::unordered_map<std::string, IEnemyBrain*> brains{{e.id(), &brain}};
    CombatStateMachine sm({&h, &e}, play_once(CombatAction::gambit_predict(e.id())), nullptr,
                          &brains);
    sm.begin_turn();
    sm.run_active_turn_to_end();
    REQUIRE(sm.last_prediction().has_value());
    REQUIRE(sm.last_prediction()->is_chaotic);
    REQUIRE(log_has(sm, CombatActionType::GambitPredict, "[CAOTICO"));
}

TEST_CASE("inc3: LastPrediction e nulo antes de qualquer predict",
          "[domain][combat][inc3]") {
    CombatActor h = hero();
    CombatActor e = foe();
    CombatStateMachine sm({&h, &e}, [](CombatActor&, const CombatState&) { return CombatAction::pass(); });
    REQUIRE_FALSE(sm.last_prediction().has_value());
}

TEST_CASE("inc3: gambit predict sem target lanca", "[domain][combat][inc3]") {
    CombatActor h = hero();
    CombatActor e = foe("enemy");
    FakeBrain brain(IntentPreview{"enemy", "attack", 5, TargetShape::Single, "gus", false});
    std::unordered_map<std::string, IEnemyBrain*> brains{{e.id(), &brain}};
    CombatStateMachine sm({&h, &e}, [&](CombatActor& a, const CombatState&) {
        if (!a.is_player_side()) return CombatAction::pass();
        CombatAction act = CombatAction::gambit_predict(e.id());
        act.target_id = std::nullopt;
        return act;
    }, nullptr, &brains);
    sm.begin_turn();
    REQUIRE_THROWS_AS(sm.run_active_turn_to_end(), std::logic_error);
}

TEST_CASE("inc3: gambit predict de novo atualiza LastPrediction",
          "[domain][combat][inc3]") {
    CombatActor h = hero();
    CombatActor a = foe("a", 50, 5);
    CombatActor b = foe("b", 50, 4);
    FakeBrain ba(IntentPreview{"a", "attack", 3, TargetShape::Single, "gus", false});
    FakeBrain bb(IntentPreview{"b", "defend", 0, TargetShape::Self, "b", false});
    std::unordered_map<std::string, IEnemyBrain*> brains{{"a", &ba}, {"b", &bb}};

    auto step = std::make_shared<int>(0);
    CombatStateMachine sm({&h, &a, &b}, [step](CombatActor& act, const CombatState&) -> CombatAction {
        if (!act.is_player_side()) return CombatAction::pass();
        switch ((*step)++) {
            case 0: return CombatAction::gambit_predict("a");
            case 1: return CombatAction::gambit_predict("b");
            default: return CombatAction::pass();
        }
    }, nullptr, &brains);
    sm.begin_turn();
    sm.run_active_turn_to_end();

    REQUIRE(sm.last_prediction()->actor_id == "b");
    REQUIRE(sm.last_prediction()->predicted_action_id == "defend");
    REQUIRE(h.ap() == 1);  // 3 - 1 - 1
}

// ===== GambitReorder =====

TEST_CASE("inc3: gambit reorder +1 atrasa alvo na fila", "[domain][combat][inc3]") {
    CombatActor h = hero("gus", 50, 30);
    CombatActor a = foe("a", 50, 20);
    CombatActor b = foe("b", 50, 10);
    CombatStateMachine sm({&h, &a, &b}, play_once(CombatAction::gambit_reorder("a", 1)));
    sm.begin_turn();
    sm.run_active_turn_to_end();
    REQUIRE(order_ids(sm) == std::vector<std::string>{"gus", "b", "a"});
}

TEST_CASE("inc3: gambit reorder -1 adianta alvo na fila", "[domain][combat][inc3]") {
    CombatActor h = hero("gus", 50, 30);
    CombatActor a = foe("a", 50, 20);
    CombatActor b = foe("b", 50, 10);
    CombatStateMachine sm({&h, &a, &b}, play_once(CombatAction::gambit_reorder("b", -1)));
    sm.begin_turn();
    sm.run_active_turn_to_end();
    REQUIRE(order_ids(sm) == std::vector<std::string>{"gus", "b", "a"});
}

TEST_CASE("inc3: gambit reorder gasta 2 ap", "[domain][combat][inc3]") {
    CombatActor h = hero("gus", 50, 30);
    CombatActor a = foe("a", 50, 20);
    CombatStateMachine sm({&h, &a}, play_once(CombatAction::gambit_reorder("a", 1)));
    sm.begin_turn();
    REQUIRE(h.ap() == 3);
    sm.run_active_turn_to_end();
    REQUIRE(h.ap() == 1);
}

TEST_CASE("inc3: gambit reorder delta 0 gasta 2 ap e loga no-op", "[domain][combat][inc3]") {
    CombatActor h = hero("gus", 50, 30);
    CombatActor a = foe("a", 50, 20);
    CombatStateMachine sm({&h, &a}, play_once(CombatAction::gambit_reorder("a", 0)));
    sm.begin_turn();
    sm.run_active_turn_to_end();
    REQUIRE(h.ap() == 1);
    REQUIRE(log_has(sm, CombatActionType::GambitReorder, "no-op"));
}

TEST_CASE("inc3: gambit reorder sem target lanca", "[domain][combat][inc3]") {
    CombatActor h = hero("gus", 50, 30);
    CombatActor a = foe("a", 50, 20);
    CombatStateMachine sm({&h, &a}, [&](CombatActor& act, const CombatState&) {
        if (!act.is_player_side()) return CombatAction::pass();
        CombatAction r = CombatAction::gambit_reorder("a", 1);
        r.target_id = std::nullopt;
        return r;
    });
    sm.begin_turn();
    REQUIRE_THROWS_AS(sm.run_active_turn_to_end(), std::logic_error);
}

TEST_CASE("inc3: gambit reorder alvo inexistente lanca", "[domain][combat][inc3]") {
    CombatActor h = hero("gus", 50, 30);
    CombatActor a = foe("a", 50, 20);
    CombatStateMachine sm({&h, &a}, play_once(CombatAction::gambit_reorder("nao_existe", 1)));
    sm.begin_turn();
    REQUIRE_THROWS_AS(sm.run_active_turn_to_end(), std::logic_error);
}

// ===== Integracao =====

TEST_CASE("inc3: ator com ap 2 usa reorder e encerra turno sem segundo reorder",
          "[domain][combat][inc3]") {
    CombatActor h = hero("gus", 50, 30);
    CombatActor a = foe("a", 50, 20);
    auto calls = std::make_shared<int>(0);
    CombatStateMachine sm({&h, &a}, [calls](CombatActor& act, const CombatState&) -> CombatAction {
        if (!act.is_player_side()) return CombatAction::pass();
        ++(*calls);
        return CombatAction::gambit_reorder("a", 1);
    });
    sm.begin_turn();
    sm.run_active_turn_to_end();
    REQUIRE(h.ap() == 1);
    REQUIRE(*calls == 2);  // 1 executado, 1 inviavel
    int reorder_logs = 0;
    for (const auto& e : sm.log())
        if (e.action == CombatActionType::GambitReorder) ++reorder_logs;
    REQUIRE(reorder_logs == 1);
}

// ===== QA-1: repro do bug GambitReorder-duplo (achado QA 2026-07-15, ==============
// =====        COMBATE-FILA-CURSOR-FIX/A1) - reorder_actor cru cruzava o cursor, =====
// =====        fazendo o CASTER agir 2x e um vizinho ser pulado inteiro. ============

TEST_CASE("inc3 QA-1: gambit reorder adianta um alvo bem atras sem cruzar o cursor - "
          "cada ator age exatamente 1x na rodada, o caster NAO repete",
          "[domain][combat][inc3][qa]") {
    CombatActor h = hero("h", 50, 40);
    CombatActor a = foe("a", 50, 30);
    CombatActor b = foe("b", 50, 20);
    CombatActor c = foe("c", 50, 10);
    CombatStateMachine sm({&h, &a, &b, &c}, play_once(CombatAction::gambit_reorder("c", -3)));
    REQUIRE(order_ids(sm) == std::vector<std::string>{"h", "a", "b", "c"});

    std::vector<std::string> act_order;
    for (int i = 0; i < 4; ++i) {
        act_order.push_back(sm.queue().current()->id());
        sm.begin_turn();
        sm.run_active_turn_to_end();
        sm.advance_to_next_actor();
    }

    // c pede adiantar 3 (indice 3 -> 0), clampado em cursor()+1=1: [h, c, a, b]. Cada id
    // aparece EXATAMENTE 1x - h NAO repete e nenhum vizinho e pulado (o bug antigo cruzava
    // o cursor com reorder_actor cru e produzia exatamente esses 2 sintomas).
    REQUIRE(act_order == std::vector<std::string>{"h", "c", "a", "b"});
    REQUIRE(order_ids(sm) == std::vector<std::string>{"h", "c", "a", "b"});
}

TEST_CASE("inc3 QA-1: gambit reorder no alvo == current() (em acao agora) dissipa - AP "
          "gasto, log de dissipacao, ordem intacta",
          "[domain][combat][inc3][qa]") {
    CombatActor h = hero("h", 50, 30);
    CombatActor a = foe("a", 50, 20);
    CombatStateMachine sm({&h, &a}, play_once(CombatAction::gambit_reorder("h", 1)));  // auto-alvo.
    sm.begin_turn();
    const std::vector<std::string> before = order_ids(sm);
    sm.run_active_turn_to_end();
    REQUIRE(h.ap() == 1);  // AP gasto mesmo dissipando (janela de acao normal).
    REQUIRE(order_ids(sm) == before);
    REQUIRE(log_has(sm, CombatActionType::GambitReorder, "dissipa"));
}

TEST_CASE("inc3 QA-1: gambit reorder num alvo que ja agiu nesta rodada (indice < cursor) "
          "dissipa - AP gasto, log de dissipacao, ordem intacta",
          "[domain][combat][inc3][qa]") {
    CombatActor a = foe("a", 50, 30);   // abre a rodada (SPD maior).
    CombatActor h = hero("h", 50, 20);  // age depois de a.
    CombatStateMachine sm({&a, &h}, [](CombatActor& act, const CombatState&) -> CombatAction {
        if (act.id() == "h") return CombatAction::gambit_reorder("a", 1);
        return CombatAction::pass();
    });
    sm.begin_turn(); sm.run_active_turn_to_end(); sm.advance_to_next_actor();  // a age (pass).
    REQUIRE(sm.active_actor()->id() == "h");
    const std::vector<std::string> before = order_ids(sm);

    sm.begin_turn();
    sm.run_active_turn_to_end();  // h mira a, que ja agiu -> dissipa.

    REQUIRE(h.ap() == 1);
    REQUIRE(order_ids(sm) == before);
    REQUIRE(log_has(sm, CombatActionType::GambitReorder, "dissipa"));
}

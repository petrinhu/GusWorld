// scripted_brain_test.cpp
//
// Spec executavel (Catch2 v3) do contrato de AI inimiga (enemy_brain.hpp / IEnemyBrain)
// e do ScriptedBrain (AI nivel Trash, deterministica, intent 100% legivel), portados de
// engine/foundation/turn_combat/IEnemyBrain.cs + ScriptedBrain.cs. POCO puro, ZERO Qt.
//
// Estes arquivos C# nao tem teste xUnit dedicado (a FSM os exercita em CombatInc3Tests
// via FakeBrain); aqui validamos o ScriptedBrain isolado: ataca o primeiro player vivo,
// intent reflete exatamente a decisao (telegraph honesto), e pass quando nao ha alvo.
//
// Cross-ref: engine/foundation/turn_combat/IEnemyBrain.cs / ScriptedBrain.cs;
//            docs/design/mecanicas/combat.md secao 13.

#include <catch2/catch_test_macros.hpp>

#include "gus/domain/combat/combat_actor.hpp"
#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/combat/combat_records.hpp"
#include "gus/domain/combat/combat_state.hpp"
#include "gus/domain/combat/initiative_queue.hpp"
#include "gus/domain/combat/scripted_brain.hpp"

using namespace gus::domain::combat;

TEST_CASE("scripted_brain: intent ataca o primeiro player vivo (telegraph honesto)",
          "[domain][combat][brain]") {
    CombatActor gus("gus", "gus", 30, 8, 2, 20, CardFamily::Eletrico, /*player=*/true);
    CombatActor foe("e", "e", 30, 9, 1, 10, CardFamily::Cinetico, /*player=*/false);
    InitiativeQueue q({&gus, &foe});
    const CombatState s(q, &foe, q.round_index());

    ScriptedBrain brain;
    const IntentPreview intent = brain.preview_intent(s, foe);

    REQUIRE(intent.actor_id == "e");
    REQUIRE(intent.predicted_action_id == "attack");
    REQUIRE(intent.predicted_shape == TargetShape::Single);
    REQUIRE(intent.predicted_target_id == "gus");
    REQUIRE_FALSE(intent.is_chaotic);
    // dano previsto = clamp(Atk - Def, MinDamage): 9 - 2 = 7.
    REQUIRE(intent.predicted_damage == 7);
}

TEST_CASE("scripted_brain: dano previsto clampa em MinDamage quando def supera atk",
          "[domain][combat][brain]") {
    CombatActor gus("gus", "gus", 30, 8, 50, 20, CardFamily::Eletrico, /*player=*/true);
    CombatActor foe("e", "e", 30, 3, 1, 10, CardFamily::Cinetico, /*player=*/false);
    InitiativeQueue q({&gus, &foe});
    const CombatState s(q, &foe, q.round_index());

    ScriptedBrain brain;
    const IntentPreview intent = brain.preview_intent(s, foe);
    REQUIRE(intent.predicted_damage == 1);  // clamp MinDamage
}

TEST_CASE("scripted_brain: decide_action ataca o primeiro player vivo",
          "[domain][combat][brain]") {
    CombatActor gus("gus", "gus", 30, 8, 2, 20, CardFamily::Eletrico, /*player=*/true);
    CombatActor foe("e", "e", 30, 9, 1, 10, CardFamily::Cinetico, /*player=*/false);
    InitiativeQueue q({&gus, &foe});
    const CombatState s(q, &foe, q.round_index());

    ScriptedBrain brain;
    const CombatAction action = brain.decide_action(s, foe);
    REQUIRE(action.type == CombatActionType::Attack);
    REQUIRE(action.target_id.has_value());
    REQUIRE(*action.target_id == "gus");
}

TEST_CASE("scripted_brain: sem player vivo, intent e decisao viram pass",
          "[domain][combat][brain]") {
    CombatActor gus("gus", "gus", 30, 8, 2, 20, CardFamily::Eletrico, /*player=*/true);
    CombatActor foe("e", "e", 30, 9, 1, 10, CardFamily::Cinetico, /*player=*/false);
    InitiativeQueue q({&gus, &foe});
    gus.take_damage(1000);  // unico player morto
    const CombatState s(q, &foe, q.round_index());

    ScriptedBrain brain;
    const IntentPreview intent = brain.preview_intent(s, foe);
    REQUIRE(intent.predicted_action_id == "pass");
    REQUIRE(intent.predicted_shape == TargetShape::Self);
    REQUIRE(intent.predicted_target_id == "e");

    const CombatAction action = brain.decide_action(s, foe);
    REQUIRE(action.type == CombatActionType::Pass);
}

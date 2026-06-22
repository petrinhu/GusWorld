// gus/domain/src/combat/scripted_brain.cpp
//
// Implementacao do ScriptedBrain (AI Trash, secao 13). Ver header para o contrato.
// Espelha ScriptedBrain.cs 1:1: ataca o primeiro player vivo; pass se nao houver.
// POCO puro, ZERO Qt.

#include "gus/domain/combat/scripted_brain.hpp"

#include <algorithm>

#include "gus/domain/combat/combat_actor.hpp"
#include "gus/domain/combat/combat_constants.hpp"
#include "gus/domain/combat/combat_state.hpp"

namespace gus::domain::combat {

IntentPreview ScriptedBrain::preview_intent(const CombatState& state,
                                            const CombatActor& self) {
    const auto players = state.alive_players();
    if (players.empty()) {
        return IntentPreview{self.id(), "pass", 0, TargetShape::Self, self.id(),
                             /*is_chaotic=*/false};
    }

    const CombatActor* target = players.front();
    const int predicted_damage =
        std::max(combat_constants::kMinDamage, self.atk() - target->def());
    return IntentPreview{self.id(), "attack", predicted_damage, TargetShape::Single,
                         target->id(), /*is_chaotic=*/false};
}

CombatAction ScriptedBrain::decide_action(const CombatState& state,
                                          const CombatActor& self) {
    (void)self;  // o roteiro Trash nao usa self para decidir (mira o 1o player vivo)
    const auto players = state.alive_players();
    if (players.empty())
        return CombatAction::pass();
    return CombatAction::attack(players.front()->id());
}

}  // namespace gus::domain::combat

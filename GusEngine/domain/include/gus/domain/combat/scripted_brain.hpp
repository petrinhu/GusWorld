// gus/domain/combat/scripted_brain.hpp
//
// AI nivel Trash: deterministica, roteiro fixo. Intent 100% legivel (secao 13). Portado
// de engine/foundation/turn_combat/ScriptedBrain.cs. POCO puro, ZERO Qt.
//
// Esqueleto funcional minimo do slice: ataca o primeiro membro vivo da party. O intent
// reflete exatamente essa decisao (telegraph honesto). UtilityBrain (Elite) + ruido de
// mini-boss ficam pra jogo posterior.
//
// Cross-ref: engine/foundation/turn_combat/ScriptedBrain.cs; docs/design/mecanicas/combat.md secao 13.

#ifndef GUS_DOMAIN_COMBAT_SCRIPTED_BRAIN_HPP
#define GUS_DOMAIN_COMBAT_SCRIPTED_BRAIN_HPP

#include "gus/domain/combat/combat_records.hpp"
#include "gus/domain/combat/enemy_brain.hpp"

namespace gus::domain::combat {

class CombatActor;
class CombatState;

// Brain deterministico de inimigo trash (secao 13).
class ScriptedBrain final : public IEnemyBrain {
public:
    [[nodiscard]] IntentPreview preview_intent(const CombatState& state,
                                               const CombatActor& self) override;

    [[nodiscard]] CombatAction decide_action(const CombatState& state,
                                             const CombatActor& self) override;
};

}  // namespace gus::domain::combat

#endif  // GUS_DOMAIN_COMBAT_SCRIPTED_BRAIN_HPP

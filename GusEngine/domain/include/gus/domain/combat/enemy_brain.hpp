// gus/domain/combat/enemy_brain.hpp
//
// Contrato de AI inimiga (IEnemyBrain), portado de
// engine/foundation/turn_combat/IEnemyBrain.cs. POCO puro, ZERO Qt.
//
// TODA AI e obrigada a expor IntentPreview (contrato de telegraph): Scan e Gambito leem
// o intent. Um inimigo sem intent legivel quebraria o pillar de informacao (secao 13).
//
// PORTE DE REFERENCIA: o C# passa CombatState (view) + CombatActor self. No C++ a view
// e const CombatState& e o self e const CombatActor& (preview/decide nao mutam o self;
// a FSM aplica a acao escolhida). O delegate CombatActionProvider do C# vira
// std::function<CombatAction(CombatActor&, const CombatState&)> na FSM (combat_state_machine).
//
// Cross-ref: engine/foundation/turn_combat/IEnemyBrain.cs; docs/design/mecanicas/combat.md secao 13.

#ifndef GUS_DOMAIN_COMBAT_ENEMY_BRAIN_HPP
#define GUS_DOMAIN_COMBAT_ENEMY_BRAIN_HPP

#include "gus/domain/combat/combat_records.hpp"

namespace gus::domain::combat {

class CombatActor;
class CombatState;

// Contrato central de AI inimiga (secao 13).
class IEnemyBrain {
public:
    virtual ~IEnemyBrain() = default;

    // Telegraph obrigatorio: o que o inimigo pretende fazer no proximo turno.
    [[nodiscard]] virtual IntentPreview preview_intent(const CombatState& state,
                                                       const CombatActor& self) = 0;

    // Acao efetiva escolhida no turno.
    [[nodiscard]] virtual CombatAction decide_action(const CombatState& state,
                                                     const CombatActor& self) = 0;
};

}  // namespace gus::domain::combat

#endif  // GUS_DOMAIN_COMBAT_ENEMY_BRAIN_HPP

// gus/domain/combat/card_integrity_ledger.hpp
//
// PONTE instancia->combate (CARDS-HW-2 fatia 1, VIRUS EM COMBATE; docs/design/mecanicas/
// cartas-spec-logica.md secao 1/4/5.2): a CombatStateMachine so enxerga o Card de catalogo
// (card_registry, id->Card) + CombatAction::card_id (string) - ela NAO conhece CardInstance
// nem CardCollection (esses vivem em gus/domain/deck/, fora do gate de camadas do combate).
// Este header e o UNICO ponto de contato: um snapshot READ-ONLY, montado pelo host (app/) a
// cada UseCard, mapeando instance_id (u64, deck/card_collection.hpp) -> ponteiro NAO-DONO
// pro IntegrityState MUTAVEL da instancia (gus/domain/infection/integrity_state.hpp).
//
// GATE DE CAMADAS (invariante): este header inclui SO infection/integrity_state.hpp, NUNCA
// deck/ - o combate nao pode depender de deck/ (evita ciclo deck->combat->deck). A mutacao do
// worm/logic-bomb escreve DIRETO no IntegrityState* apontado aqui; o save (deck/card_hardware.hpp
// -> CardPhysicalState herda IntegrityState) serializa de graca, sem o combate saber de save.
//
// OWNERSHIP: ponteiros NAO-DONOS (mesmo padrao de CombatActor*/IRandomSource* na FSM). O
// vetor de CardIntegrityRef e as instancias apontadas vivem no escopo do host (app/) ou do
// teste - a CombatStateMachine so LE/MUTA atraves do ponteiro, nunca aloca/libera.
//
// Cross-ref: gus/domain/infection/integrity_state.hpp (IntegrityState/VirusKind);
//            gus/domain/combat/combat_state_machine.hpp (ctor integrity_ledger, CardAction::
//            card_instance_id); docs/design/mecanicas/cartas-spec-logica.md secao 1/2/4/5.2;
//            docs/design/mecanicas/deck-mao-sistema.md secao 7 (CardInstance::instance_id).

#ifndef GUS_DOMAIN_COMBAT_CARD_INTEGRITY_LEDGER_HPP
#define GUS_DOMAIN_COMBAT_CARD_INTEGRITY_LEDGER_HPP

#include <cstdint>
#include <string>

#include "gus/domain/infection/integrity_state.hpp"

namespace gus::domain::combat {

// Uma entrada do ledger: qual instancia (`instance_id`), qual carta de catalogo ela
// representa (`card_id`, chave do card_registry - resolve tier/ESPECIAL-SUPER pro guard de
// classe protegida do Worm, secao 5.2), de quem e a posse (`owner_actor_id`, chave OPACA
// interna ao ledger - agrupa "mesmo dono" pra distinguir OwnDeck/EnemyDeck na propagacao;
// NAO precisa casar com CombatActor::id() nesta fatia) e o ponteiro pro estado mutavel
// (`state`, nullptr = instancia sem peca de integridade, fail-safe no consumidor).
struct CardIntegrityRef {
    std::uint64_t instance_id = 0;
    std::string card_id;
    int owner_actor_id = 0;
    infection::IntegrityState* state = nullptr;
};

}  // namespace gus::domain::combat

#endif  // GUS_DOMAIN_COMBAT_CARD_INTEGRITY_LEDGER_HPP

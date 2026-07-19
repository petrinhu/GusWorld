// gus/domain/combat/card_collection_snapshot.hpp
//
// Snapshot READ-ONLY da coleçao (deck ativo) de um caster (CARDS-HW-2 fatia B, carta
// `urandom`; docs/design/mecanicas/cartas-spec-logica.md secao 7): a CombatStateMachine so
// enxerga o Card de catalogo (card_registry, id->Card) - ela NAO conhece CardCollection nem
// CardInstance (esses vivem em gus/domain/deck/, fora do gate de camadas do combate, mesmo
// racional de card_integrity_ledger.hpp). Este header e o 2o ponto de contato desse tipo:
// um vetor read-only, montado pelo host (app/) a cada combate, listando TODAS as instancias
// possuidas por CADA participante (nao so a carta jogada) - o urandom precisa da COLEÇAO
// INTEIRA do caster pra classificar o pool por faixa (fraco/medio/forte/jackpot), algo que
// nenhum outro consumidor do executor techMagic precisou ate agora (TechMagicContext nunca
// carregou um card_registry nem uma colecao - so o card_id/instance da carta EM EXECUÇAO).
//
// GATE DE CAMADAS (invariante, mesmo do integrity ledger): este header inclui SO
// hardware/card_provenance.hpp (POCO sem dependencia de combat/ nem deck/), NUNCA deck/ - o
// combate nao pode depender de deck/ (evita ciclo deck->combat->deck, ja que CardCollection
// inclui combat/combat_enums.hpp pro CardTier).
//
// OWNERSHIP: nao ha ponteiro mutavel aqui (ao contrario de CardIntegrityRef::state) - o
// snapshot e 100% read-only, urandom so LE origem/card_id/dono pra classificar e escolher,
// nunca muta a colecao. O vetor inteiro vive no escopo do host (app/) ou do teste.
//
// Cross-ref: gus/domain/combat/card_integrity_ledger.hpp (MESMO padrao de snapshot, pra
//            vírus); gus/domain/combat/urandom_algorithm.hpp (tabelas de peso/faixa);
//            gus/domain/combat/combat_state_machine.hpp (ctor collection_snapshot,
//            resolve_urandom); gus/domain/hardware/card_provenance.hpp (CardOrigin);
//            docs/design/mecanicas/cartas-spec-logica.md secao 7;
//            docs/design/mecanicas/cartas-numeros-proposta.md secao 4.

#ifndef GUS_DOMAIN_COMBAT_CARD_COLLECTION_SNAPSHOT_HPP
#define GUS_DOMAIN_COMBAT_CARD_COLLECTION_SNAPSHOT_HPP

#include <cstdint>
#include <string>

#include "gus/domain/hardware/card_provenance.hpp"

namespace gus::domain::combat {

// Uma entrada do snapshot: qual instancia (`instance_id`), qual carta de catalogo ela
// representa (`card_id`, chave do card_registry - resolve tier/mana pro classificador de
// faixa do urandom), de quem e a posse (`owner_actor_id`, chave OPACA interna ao snapshot,
// MESMO racional de CardIntegrityRef::owner_actor_id - agrupa "mesma colecao"; nao precisa
// casar com CombatActor::id()) e a origem fisica (`origin`, ROM/EPROM/pirata - decide qual
// tabela de pesos o urandom usa quando ESTA e a instancia jogada, hardware/card_provenance.hpp).
struct CardCollectionEntry {
    std::uint64_t instance_id = 0;
    std::string card_id;
    int owner_actor_id = 0;
    hardware::CardOrigin origin = hardware::CardOrigin::OriginalRom;
};

}  // namespace gus::domain::combat

#endif  // GUS_DOMAIN_COMBAT_CARD_COLLECTION_SNAPSHOT_HPP

// gus/domain/deck/deck_records.hpp
//
// Record de dados puros do sistema de deck/mao (DECK-1). POCO puro, ZERO
// SDL/glintfx (invariante de domain/, engine-design.md secao 2).
//
// CardInstance e a UNIDADE do agregado CardCollection (card_collection.hpp): cada carta
// POSSUIDA por um personagem vira uma instancia com ID proprio, deterministico (contador
// sequencial persistido, comeca em 1, NUNCA reusa - sem RNG, sem timestamp/Date). O
// campo card_id so REFERENCIA o catalogo (gus::domain::combat::Card, resolvido via
// PlaceholderCards::all()/MasterCards) - este struct NAO duplica dado de carta (mana,
// power, efeitos, tier ficam SO no catalogo, fonte unica).
//
// Cross-ref: docs/design/mecanicas/deck-mao-sistema.md secao 7 (invariantes
// anti-exploit, inv.1: "carta = instancia unica com ID, vive em EXATAMENTE UM
// container"); card_collection.hpp (o agregado que garante o invariante).

#ifndef GUS_DOMAIN_DECK_DECK_RECORDS_HPP
#define GUS_DOMAIN_DECK_DECK_RECORDS_HPP

#include <cstdint>
#include <string>

#include "gus/domain/deck/card_hardware.hpp"  // CardPhysicalState (CARDS-HW-1)

namespace gus::domain::deck {

// Instancia unica de uma carta possuida por um personagem. instance_id e o identificador
// deterministico do CONTAINER (deck ativo/morto); card_id e a chave pro catalogo (
// gus::domain::combat::Card::id). Sem mastery/skin aqui - sem consumidor ainda (fica
// pra quando existir progressao de instancia, fora do MVP).
struct CardInstance {
    std::uint64_t instance_id = 0;
    std::string card_id;

    // NOVO (CARDS-HW-1, cartas-spec-dados.md secao 4). Estado fisico MUTAVEL desta
    // copia especifica (bateria/degradacao/integridade sao por-EXEMPLAR, nao
    // por-catalogo: duas copias da MESMA carta podem ter cargas de bateria e
    // status de infeccao diferentes). Default = CardPhysicalState{} = "ROM
    // original legitima, bateria cheia, sem infeccao" - o estado mais SEGURO,
    // preservando TODO CardInstance{id, card_id} existente intacto (campo
    // ADITIVO ao fim do struct, mesmo padrao de EffectSpec::side_filter).
    CardPhysicalState physical;

    [[nodiscard]] bool operator==(const CardInstance&) const = default;
};

}  // namespace gus::domain::deck

#endif  // GUS_DOMAIN_DECK_DECK_RECORDS_HPP

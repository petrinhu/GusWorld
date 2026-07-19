// gus/domain/src/combat/urandom_algorithm.cpp
//
// Implementacao das 2 funcoes PURAS do motor da carta `urandom`. Ver header para o
// contrato. POCO puro, ZERO Qt.

#include "gus/domain/combat/urandom_algorithm.hpp"

namespace gus::domain::combat {

UrandomFaixa weighted_pick_urandom_faixa(const UrandomWeightEntry* table, std::size_t count,
                                         IRandomSource& rng) {
    int total = 0;
    for (std::size_t i = 0; i < count; ++i) total += table[i].weight;
    if (total <= 0 || count == 0)
        return count > 0 ? table[0].faixa : UrandomFaixa::Fraco;  // defensivo, nunca alcancado
                                                                    // pelas tabelas canonicas.

    const int roll = rng.next(total);
    int cumulative = 0;
    for (std::size_t i = 0; i < count; ++i) {
        cumulative += table[i].weight;
        if (roll < cumulative) return table[i].faixa;
    }
    return table[count - 1].faixa;  // fallback defensivo (arredondamento), matematicamente
                                     // inalcancavel: roll < total sempre por construcao do rng.
}

std::optional<UrandomFaixa> classify_urandom_faixa(const cards::Card& card) {
    if (card.tier != cards::CardTier::Comum) return UrandomFaixa::Jackpot;
    if (card.mana_cost == kUrandomFaixaFracoManaCost) return UrandomFaixa::Fraco;
    if (card.mana_cost == kUrandomFaixaMedioManaCost) return UrandomFaixa::Medio;
    if (card.mana_cost == kUrandomFaixaForteManaCost) return UrandomFaixa::Forte;
    return std::nullopt;
}

}  // namespace gus::domain::combat

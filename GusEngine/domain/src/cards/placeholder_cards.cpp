// gus/domain/src/cards/placeholder_cards.cpp
//
// Implementacao do registry de cartas placeholder (secao 9/10). Ver header para o
// contrato. Espelha PlaceholderCards.cs 1:1: 5 cartas, uma por familia, construidas uma
// vez e congeladas. POCO puro, ZERO Qt.
//
// ATOM-2: movido de gus/domain/src/combat/placeholder_cards.cpp.

#include "gus/domain/cards/placeholder_cards.hpp"

#include <stdexcept>

#include "gus/domain/cards/card_enums.hpp"

namespace gus::domain::cards::PlaceholderCards {

namespace {

Card make_card(std::string id,
               std::string display_name,
               CardFamily family,
               CardBaseType base_type,
               int mana_cost,
               int power,
               TargetShape shape,
               StatusEffect status) {
    Card c;
    c.id = std::move(id);
    c.display_name = std::move(display_name);
    c.family = family;
    c.base_type = base_type;
    c.mana_cost = mana_cost;
    c.ap_cost = 1;
    c.power = power;
    c.target_shape = shape;
    c.status_applied = status;
    // modifiers vazio, mastery 0, crit_chance 0 (defaults do struct).
    return c;
}

std::unordered_map<std::string, Card> build_registry() {
    const Card cards[] = {
        // Eletrico - Pulso: dano direto + Stun (controle curto). secao 6, 9.
        make_card("pulso_eletrico", "CARD_PULSO_ELETRICO_NAME", CardFamily::Eletrico,
                  CardBaseType::Pulso, /*mana=*/1, /*power=*/6, TargetShape::Single,
                  StatusEffect{StatusId::Stun, 2, 1, StackRule::Replace, CardFamily::Eletrico}),

        // Bioquimico - Raiz: sem dano, aplica Regen no proprio caster. secao 6, 9.
        make_card("raiz_cura", "CARD_RAIZ_CURA_NAME", CardFamily::Bioquimico,
                  CardBaseType::Raiz, /*mana=*/2, /*power=*/0, TargetShape::Self,
                  StatusEffect{StatusId::Regen, 8, 2, StackRule::Refresh, CardFamily::Bioquimico}),

        // Sonico - Eco: dano leve + Disrupt (debuff de magnitude). secao 6, 9.
        make_card("eco_sonico", "CARD_ECO_SONICO_NAME", CardFamily::Sonico,
                  CardBaseType::Eco, /*mana=*/1, /*power=*/4, TargetShape::Single,
                  StatusEffect{StatusId::Disrupt, 20, 1, StackRule::Replace, CardFamily::Sonico}),

        // Criptografico - Fenda: dano + Expose (abre o alvo a mais dano). secao 6, 9.
        make_card("fenda_criptica", "CARD_FENDA_CRIPTICA_NAME", CardFamily::Criptografico,
                  CardBaseType::Fenda, /*mana=*/2, /*power=*/5, TargetShape::Single,
                  StatusEffect{StatusId::Expose, 30, 2, StackRule::Refresh, CardFamily::Criptografico}),

        // Cinetico - Pulso: maior dano direto + Knockback (reposiciona). secao 6, 9.
        make_card("impacto_cinetico", "CARD_IMPACTO_CINETICO_NAME", CardFamily::Cinetico,
                  CardBaseType::Pulso, /*mana=*/1, /*power=*/7, TargetShape::Single,
                  StatusEffect{StatusId::Knockback, 1, 1, StackRule::Replace, CardFamily::Cinetico}),
    };

    std::unordered_map<std::string, Card> registry;
    registry.reserve(5);
    for (const auto& card : cards) {
        // emplace (nao indexer) falha-cedo se algum id duplicar (espelha Add() do C#).
        const auto [it, inserted] = registry.emplace(card.id, card);
        (void)it;
        if (!inserted)
            throw std::logic_error("PlaceholderCards: id de carta duplicado: " + card.id);
    }
    return registry;
}

}  // namespace

const std::unordered_map<std::string, Card>& all() {
    static const std::unordered_map<std::string, Card> kCards = build_registry();
    return kCards;
}

}  // namespace gus::domain::cards::PlaceholderCards

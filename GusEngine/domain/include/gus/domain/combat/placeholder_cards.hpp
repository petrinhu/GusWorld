// gus/domain/combat/placeholder_cards.hpp
//
// Registry in-memory de cartas placeholder do vertical slice (1 por familia), portado de
// engine/foundation/turn_combat/PlaceholderCards.cs. POCO puro, ZERO Qt. Serve so para
// exercitar UseCard na cena de combate; NAO e o CardRepository final (~200 cartas).
//
// Cada carta segue a gramatica "tipo.familia" (secao 7) e aplica um unico status
// representativo da familia (secao 9). Sem modificadores, sem mastery e sem crit.
//
// O C# expoe IReadOnlyDictionary<string,Card> All(). Aqui PlaceholderCards::all() devolve
// const std::unordered_map<string,Card>& (mesma instancia imutavel toda chamada),
// pronto para passar como card_registry da CombatStateMachine.
//
// Cross-ref: engine/foundation/turn_combat/PlaceholderCards.cs; docs/design/mecanicas/combat.md secao 9/10.

#ifndef GUS_DOMAIN_COMBAT_PLACEHOLDER_CARDS_HPP
#define GUS_DOMAIN_COMBAT_PLACEHOLDER_CARDS_HPP

#include <string>
#include <unordered_map>

#include "gus/domain/combat/combat_records.hpp"

namespace gus::domain::combat::PlaceholderCards {

// Dicionario id->Card com as 5 cartas do slice. Imutavel; mesma instancia toda chamada.
[[nodiscard]] const std::unordered_map<std::string, Card>& all();

}  // namespace gus::domain::combat::PlaceholderCards

#endif  // GUS_DOMAIN_COMBAT_PLACEHOLDER_CARDS_HPP

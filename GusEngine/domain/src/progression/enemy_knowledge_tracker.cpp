// gus/domain/progression/enemy_knowledge_tracker.cpp
//
// Implementacao da logica pura de Knowledge por tipo de inimigo. Ver header para
// o contrato, a semantica canonica (knowledge-progression.md secao 1/3) e a nota
// FRONTEIRA sobre DefeatedActor.

#include "gus/domain/progression/enemy_knowledge_tracker.hpp"

#include <stdexcept>
#include <string>
#include <string_view>

namespace gus::domain::progression {

namespace {

constexpr std::string_view kWhitespace = " \t\r\n\f\v";

// True se a string e vazia ou so whitespace (equivalente a
// string.IsNullOrWhiteSpace do .NET para o conjunto ASCII relevante). Mesma
// regra usada pelo validador i18n (is_blank).
bool is_blank(std::string_view s) {
    return s.find_first_not_of(kWhitespace) == std::string_view::npos;
}

}  // namespace

int knowledge_for(const KnowledgeStore& store, std::string_view enemy_type_id) {
    if (is_blank(enemy_type_id)) {
        throw std::invalid_argument("knowledge_for: enemy_type_id nao pode ser vazio.");
    }

    // std::map nao aceita string_view como chave de find ate o transparent
    // comparator; materializa a chave (catalogo pequeno, custo irrelevante).
    const auto it = store.find(std::string{enemy_type_id});
    return it != store.end() ? it->second : 0;
}

KnowledgeStore apply_victory(const KnowledgeStore& store,
                             const std::vector<std::string>& defeated_enemy_types) {
    // Clona o store (forward-only: o input nao e mutado).
    KnowledgeStore result = store;

    for (const auto& type_id : defeated_enemy_types) {
        if (is_blank(type_id)) {
            throw std::invalid_argument(
                "apply_victory: defeated_enemy_types nao pode conter id vazio.");
        }
        // operator[] insere com 0 se ausente; ++ aplica o incremento do ciclo.
        ++result[type_id];
    }

    return result;
}

std::vector<std::string> defeated_enemy_types(CombatOutcome outcome,
                                              const std::vector<DefeatedActor>& actors) {
    std::vector<std::string> types;
    if (outcome != CombatOutcome::Victory) {
        return types;  // Fled/Defeat: nenhum credito (secao 3).
    }

    for (const auto& actor : actors) {
        // So inimigos (nao-party) que terminaram mortos (HP 0) contam.
        if (!actor.is_player_side && !actor.is_alive) {
            types.push_back(actor.id);
        }
    }
    return types;
}

}  // namespace gus::domain::progression

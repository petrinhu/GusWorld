// gus/domain/src/templates/enemy_template.cpp
//
// Validacao fail-fast do EnemyTemplate. Ver header para o contrato. Espelha
// EnemyTemplate.Validate() do C#. POCO puro, ZERO Qt.

#include "gus/domain/templates/enemy_template.hpp"

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <string>

namespace gus::domain::templates {

namespace {

// Espelha string.IsNullOrWhiteSpace do C#.
[[nodiscard]] bool is_null_or_whitespace(const std::string& s) {
    return std::all_of(s.begin(), s.end(), [](unsigned char c) {
        return std::isspace(c) != 0;
    });
}

}  // namespace

void EnemyTemplate::validate() const {
    if (is_null_or_whitespace(id)) {
        throw std::invalid_argument("EnemyTemplate.id nao pode ser vazio.");
    }
    if (max_hp <= 0) {
        throw std::invalid_argument("EnemyTemplate.max_hp deve ser > 0.");
    }
    if (atk < 0) {
        throw std::invalid_argument("EnemyTemplate.atk deve ser >= 0.");
    }
    if (def < 0) {
        throw std::invalid_argument("EnemyTemplate.def deve ser >= 0.");
    }
    if (spd < 0) {
        throw std::invalid_argument("EnemyTemplate.spd deve ser >= 0.");
    }
    for (const auto& card_id : base_deck) {
        if (is_null_or_whitespace(card_id)) {
            throw std::invalid_argument(
                "EnemyTemplate.base_deck nao pode conter Card.Id vazio.");
        }
    }
}

}  // namespace gus::domain::templates

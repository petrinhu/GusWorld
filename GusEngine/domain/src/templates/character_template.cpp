// gus/domain/src/templates/character_template.cpp
//
// Validacao fail-fast do CharacterTemplate. Ver header para o contrato. Espelha
// CharacterTemplate.Validate() do C# (ArgumentException/ArgumentOutOfRange viram
// std::invalid_argument). POCO puro, ZERO Qt.

#include "gus/domain/templates/character_template.hpp"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <stdexcept>
#include <string>

namespace gus::domain::templates {

namespace {

// Espelha string.IsNullOrWhiteSpace do C#: vazio OU so espacos em branco.
[[nodiscard]] bool is_null_or_whitespace(const std::string& s) {
    return std::all_of(s.begin(), s.end(), [](unsigned char c) {
        return std::isspace(c) != 0;
    });
}

}  // namespace

void CharacterTemplate::validate() const {
    if (is_null_or_whitespace(id)) {
        throw std::invalid_argument("CharacterTemplate.id nao pode ser vazio.");
    }
    if (max_hp <= 0) {
        throw std::invalid_argument("CharacterTemplate.max_hp deve ser > 0.");
    }
    if (atk < 0) {
        throw std::invalid_argument("CharacterTemplate.atk deve ser >= 0.");
    }
    if (def < 0) {
        throw std::invalid_argument("CharacterTemplate.def deve ser >= 0.");
    }
    if (spd < 0) {
        throw std::invalid_argument("CharacterTemplate.spd deve ser >= 0.");
    }
    // A1 (auditoria M3): rejeita ordinal de family fora do dominio canonico {0..4}. O C#
    // Validate() NAO cobria isto; e hardening alem da paridade (um .gdt selado mas
    // schema-divergente, family=9999, deixa de ser aceito silenciosamente). Como
    // templates::CardFamily agora e a fonte canonica do combate (religacao A1), o range
    // valido e [0, kCardFamilyCount).
    if (static_cast<std::uint32_t>(family) >= kCardFamilyCount) {
        throw std::invalid_argument("CharacterTemplate.family fora do dominio (ordinal invalido).");
    }
    for (const auto& card_id : base_deck) {
        if (is_null_or_whitespace(card_id)) {
            throw std::invalid_argument(
                "CharacterTemplate.base_deck nao pode conter Card.Id vazio.");
        }
    }
}

}  // namespace gus::domain::templates

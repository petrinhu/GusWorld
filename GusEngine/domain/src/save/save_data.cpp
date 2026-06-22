// gus/domain/src/save/save_data.cpp
//
// Validacao de invariantes (fail-fast) do estado de save. POCO puro, ZERO Qt.
// Espelha o Validate() do C# (CharacterSaveState.cs). Ver header.

#include "gus/domain/save/save_data.hpp"

#include <stdexcept>
#include <string>

namespace gus::domain::save {

void CharacterSaveState::validate() const {
    if (current_hp < 0)
        throw std::invalid_argument("CharacterSaveState: current_hp deve ser >= 0.");
    if (xp < 0)
        throw std::invalid_argument("CharacterSaveState: xp deve ser >= 0.");
    for (const auto& card : deck) {
        if (card.empty())
            throw std::invalid_argument(
                "CharacterSaveState: deck nao pode conter Card.Id vazio.");
    }
}

void SaveData::validate() const {
    if (timestamp_ms < 0)
        throw std::invalid_argument("SaveData: timestamp_ms deve ser >= 0.");
    if (playtime_seconds < 0.0)
        throw std::invalid_argument("SaveData: playtime_seconds deve ser >= 0.");
    for (const auto& [id, state] : character_states) {
        if (id.empty())
            throw std::invalid_argument(
                "SaveData: chave de character_states nao pode ser vazia.");
        state.validate();
    }
}

}  // namespace gus::domain::save

// gus/domain/save/new_game.cpp
//
// Implementacao. Ver o header para o contrato e o racional de existir.

#include "gus/domain/save/new_game.hpp"

namespace gus::domain::save {

SaveData fresh_new_game_save_data(DifficultyLevel difficulty) noexcept {
    // Default-construction JA e "fresco" para todo campo (schema_version=6=
    // kSaveSchemaVersion, playtime 0, mapas/vetores vazios, credits 0,
    // slot_id=-1) - so `difficulty` nao tem default universal (e a escolha do
    // jogador na tela de selecao, ver o comentario do header).
    SaveData data{};
    data.difficulty = difficulty;
    return data;
}

}  // namespace gus::domain::save

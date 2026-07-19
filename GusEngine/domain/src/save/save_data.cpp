// gus/domain/src/save/save_data.cpp
//
// Validacao de invariantes (fail-fast) do estado de save. POCO puro, ZERO Qt.
// Espelha o Validate() do C# (CharacterSaveState.cs). Ver header.

#include "gus/domain/save/save_data.hpp"

#include <cstdint>
#include <stdexcept>
#include <string>
#include <unordered_set>

namespace gus::domain::save {

void CardCollectionState::validate() const {
    std::unordered_set<std::uint64_t> seen_ids;
    std::uint64_t max_id = 0;
    bool any_id = false;
    for (const std::vector<gus::domain::deck::CardInstance>* container :
         {&active, &dead}) {
        for (const auto& inst : *container) {
            if (inst.instance_id == 0)
                throw std::invalid_argument(
                    "CardCollectionState: instance_id deve ser >= 1 (0 e "
                    "reservado/nao-instanciado).");
            if (inst.card_id.empty())
                throw std::invalid_argument(
                    "CardCollectionState: card_id nao pode ser vazio.");
            // V7 (CARDS-HW-1): propaga o invariante de forma da camada FISICA
            // (virus_kind != None => is_infected; is_diagnosed => is_infected;
            // ordinais de origin/virus_kind - cartas-spec-dados.md secao 6 inv.1).
            // Fail-fast na fronteira do save, mesmo espirito do resto do arquivo.
            inst.physical.validate();
            if (!seen_ids.insert(inst.instance_id).second)
                throw std::invalid_argument(
                    "CardCollectionState: instance_id duplicado entre ativo e "
                    "morto (uma instancia deve viver em EXATAMENTE um "
                    "container).");
            any_id = true;
            if (inst.instance_id > max_id) max_id = inst.instance_id;
        }
    }
    if (next_instance_id == 0)
        throw std::invalid_argument(
            "CardCollectionState: next_instance_id deve ser >= 1.");
    if (any_id && next_instance_id <= max_id)
        throw std::invalid_argument(
            "CardCollectionState: next_instance_id deve ser MAIOR que qualquer "
            "instance_id existente (o contador nunca reusa um id emitido).");
}

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
    // V6 (DECK-4): CardCollectionState (deck ativo/morto). credits NAO e checado
    // aqui - mora em SaveData::credits (carteira UNICA da party), ver
    // SaveData::validate() abaixo.
    card_collection.validate();
    // V6 (DECK-4): a mao so pode referenciar instancias PRESENTES no deck ativo
    // (inv.6 de deck-mao-sistema.md secao 7 - "mao so puxa do ativo"), sem
    // duplicata (cada instancia ocupa no maximo 1 slot da mao).
    std::unordered_set<std::uint64_t> active_ids;
    active_ids.reserve(card_collection.active.size());
    for (const auto& inst : card_collection.active) active_ids.insert(inst.instance_id);
    std::unordered_set<std::uint64_t> hand_seen;
    for (const auto id : hand_selection) {
        if (!active_ids.count(id))
            throw std::invalid_argument(
                "CharacterSaveState: hand_selection referencia instance_id fora "
                "do deck ativo.");
        if (!hand_seen.insert(id).second)
            throw std::invalid_argument(
                "CharacterSaveState: hand_selection contem instance_id "
                "duplicado.");
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
    for (const auto& [enemy_type_id, kills] : enemy_knowledge) {
        if (enemy_type_id.empty())
            throw std::invalid_argument(
                "SaveData: chave de enemy_knowledge (enemy_type_id) nao pode ser "
                "vazia.");
        if (kills < 0)
            throw std::invalid_argument(
                "SaveData: kills de enemy_knowledge deve ser >= 0.");
    }
    // V5 (MODOS-MORTE Fase 0): hardening de ordinal, MESMO padrao de
    // EnemyTemplate::validate() (A1, auditoria M3) - um payload selado mas
    // schema-divergente nao e aceito silenciosamente.
    if (static_cast<std::uint32_t>(difficulty) >= kDifficultyLevelCount)
        throw std::invalid_argument(
            "SaveData: difficulty fora do dominio (ordinal invalido).");
    if (difficult_recovery_stage < 0 || difficult_recovery_stage > 4)
        throw std::invalid_argument(
            "SaveData: difficult_recovery_stage deve estar em [0, 4].");
    // V6 (DECK-4): carteira UNICA da party (docs/design/mecanicas/economia.md -
    // single-currency, nao per-character). Sem credito negativo.
    if (credits < 0)
        throw std::invalid_argument("SaveData: credits deve ser >= 0.");
}

}  // namespace gus::domain::save

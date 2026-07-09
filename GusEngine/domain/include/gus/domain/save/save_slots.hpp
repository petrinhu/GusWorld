// gus/domain/save/save_slots.hpp
//
// POLITICA DE SLOTS do save (camada PURA, ZERO Qt, ZERO disco). Decisao do lider
// (ADR-006, 2026-06-21): 1 slot de AUTO-SAVE + 5 slots de save MANUAL = 6 no total.
//
// BUMP ADITIVO (SAVE-LOAD-UI etapa 6, decisao do lider): 5->6 slots manuais (7 no
// total), pra fechar a divergencia sinalizada em save_load_menu.hpp contra o mock
// docs/design/mockups/07-save-load.html ("6 espacos + Auto"). Aditivo por
// construcao: NAO muda o formato do save nem o serializer (save_serializer.hpp/
// save_backup.hpp nao referenciam kManualSlotCount) - so o TETO de slot valido
// (is_valid_slot/kSlotCount) cresce; saves ja gravados nos slots 1..5 continuam
// lendo normalmente, o slot 6 novo nasce vazio.
//
// IDs de slot (int): 0 = autosave; 1..6 = manuais. O auto-save sobrescreve o
// proprio slot (0); os 6 manuais o jogador escolhe. Mapeamento slot -> NOME LOGICO
// ("autosave", "save_1".."save_6") e a fronteira estavel: a camada de I/O (platform/,
// futura) traduz o nome logico para caminho de arquivo. Aqui NAO ha path de disco.
//
// ADAPTACAO do C#: game/scripts/foundation/save_system/SaveManager.cs usava 1
// autosave (slot 0) + 4 manuais (slots 1..4), nomes "slot_autosave"/"slot_N". Aqui
// e 1 + 6 (slots 1..6), nomes "autosave"/"save_N" (sem o prefixo "slot_", que era
// detalhe de arquivo do C#).
//
// Cross-ref: game/scripts/foundation/save_system/SaveManager.cs (ref, adaptada),
//            ADR-006.

#ifndef GUS_DOMAIN_SAVE_SAVE_SLOTS_HPP
#define GUS_DOMAIN_SAVE_SAVE_SLOTS_HPP

#include <string>

namespace gus::domain::save {

// Slot canonico do auto-save (sobrescreve a si mesmo).
inline constexpr int kAutosaveSlot = 0;

// Quantidade de slots manuais que o jogador escolhe (1..6).
inline constexpr int kManualSlotCount = 6;

// Total de slots = 1 auto + 6 manuais.
inline constexpr int kSlotCount = 1 + kManualSlotCount;

// true se id e um slot valido (0..6).
[[nodiscard]] constexpr bool is_valid_slot(int slot) noexcept {
    return slot >= 0 && slot < kSlotCount;
}

// true se o slot e o auto-save (slot 0).
[[nodiscard]] constexpr bool is_autosave(int slot) noexcept {
    return slot == kAutosaveSlot;
}

// Nome logico do slot: "autosave" (0) ou "save_1".."save_5" (1..5). Estavel: a
// camada de I/O traduz para path. Lanca std::out_of_range se slot invalido.
[[nodiscard]] std::string slot_logical_name(int slot);

}  // namespace gus::domain::save

#endif  // GUS_DOMAIN_SAVE_SAVE_SLOTS_HPP

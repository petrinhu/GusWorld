// gus/domain/save/save_backup.hpp
//
// POLITICA DE BACKUP CHAIN do save (camada PURA, ZERO Qt, ZERO disco). Opera sobre
// o port SaveStore (nomes logicos), nao sobre arquivos. Portada de
// game/scripts/foundation/save_system/SaveManager.cs::RotateBackups (C#: renomeia
// arquivos no disco) para uma funcao pura sobre o store abstrato.
//
// Politica: ao gravar o slot S com bytes novos, o conteudo anterior de S e
// preservado como backup. Rotaciona em cadeia ate a profundidade kBackupChainDepth
// (N=3, igual ao C#): backupN-1 -> backupN, ..., backup1 -> backup2, primary ->
// backup1, e entao escreve o primario novo. A geracao mais antiga (alem de N) cai.
//
// Nomes logicos: primary_logical_name(slot) = nome do slot (save_slots.hpp);
// backup_logical_name(slot, k) = "<slot>.backupK". A traducao para path e do
// platform/, fora daqui.
//
// Cross-ref: game/scripts/foundation/save_system/SaveManager.cs (RotateBackups,
//            ref), gus/domain/save/save_store.hpp, save_slots.hpp, ADR-006.

#ifndef GUS_DOMAIN_SAVE_SAVE_BACKUP_HPP
#define GUS_DOMAIN_SAVE_SAVE_BACKUP_HPP

#include <cstdint>
#include <string>
#include <vector>

#include "gus/domain/save/save_store.hpp"

namespace gus::domain::save {

// Profundidade da cadeia de backup (N=3, igual ao SaveManager.cs do C#).
inline constexpr int kBackupChainDepth = 3;

// Nome logico do primario de um slot (= slot_logical_name). Lanca std::out_of_range
// se slot invalido.
[[nodiscard]] std::string primary_logical_name(int slot);

// Nome logico do backup k (1..kBackupChainDepth) de um slot: "<slot>.backupK".
// Lanca std::out_of_range se slot ou k invalidos.
[[nodiscard]] std::string backup_logical_name(int slot, int backup_index);

// Grava bytes no slot, rotacionando a cadeia de backup antes (primary->b1->b2->b3,
// o mais antigo cai). Funcao pura sobre o store injetado. Lanca std::out_of_range
// se slot invalido.
void write_with_backup_rotation(SaveStore& store, int slot,
                                const std::vector<std::uint8_t>& bytes);

}  // namespace gus::domain::save

#endif  // GUS_DOMAIN_SAVE_SAVE_BACKUP_HPP

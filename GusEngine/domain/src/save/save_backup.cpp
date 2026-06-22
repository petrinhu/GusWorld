// gus/domain/src/save/save_backup.cpp
//
// Politica de backup chain (pura) sobre o port SaveStore. Portada de
// SaveManager.cs::RotateBackups (C#). POCO puro, ZERO Qt, ZERO disco. Ver header.

#include "gus/domain/save/save_backup.hpp"

#include <stdexcept>
#include <string>

#include "gus/domain/save/save_slots.hpp"

namespace gus::domain::save {

std::string primary_logical_name(int slot) {
    return slot_logical_name(slot);  // valida slot (lanca std::out_of_range)
}

std::string backup_logical_name(int slot, int backup_index) {
    if (backup_index < 1 || backup_index > kBackupChainDepth)
        throw std::out_of_range("backup_logical_name: backup_index fora de 1.." +
                                std::to_string(kBackupChainDepth) + ": " +
                                std::to_string(backup_index));
    return slot_logical_name(slot) + ".backup" + std::to_string(backup_index);
}

void write_with_backup_rotation(SaveStore& store, int slot,
                                const std::vector<std::uint8_t>& bytes) {
    const std::string primary = primary_logical_name(slot);  // valida slot

    // Rotaciona do mais antigo para o mais novo (espelha SaveManager.cs):
    //   backupN-1 -> backupN, ..., backup1 -> backup2, primary -> backup1.
    // O move() sobre o nome backupN sobrescreve a geracao mais antiga (ela cai).
    for (int k = kBackupChainDepth; k >= 2; --k)
        store.move(backup_logical_name(slot, k - 1), backup_logical_name(slot, k));
    store.move(primary, backup_logical_name(slot, 1));

    store.write(primary, bytes);
}

}  // namespace gus::domain::save

// gus/domain/src/save/save_slots.cpp
//
// Politica de slots do save (1 auto + 5 manuais). POCO puro, ZERO Qt. Ver header.

#include "gus/domain/save/save_slots.hpp"

#include <stdexcept>
#include <string>

namespace gus::domain::save {

std::string slot_logical_name(int slot) {
    if (!is_valid_slot(slot))
        throw std::out_of_range("slot_logical_name: slot fora de 0.." +
                                std::to_string(kSlotCount - 1) + ": " +
                                std::to_string(slot));
    if (is_autosave(slot)) return "autosave";
    return "save_" + std::to_string(slot);
}

}  // namespace gus::domain::save

// gus/app/src/screens/battle_cockpit_verb_ids.cpp
//
// Implementacao do mapeamento id->indice de verbo (ver header). strcmp puro, sem
// SDL/glintfx.

#include "gus/app/screens/battle_cockpit_verb_ids.hpp"

#include <cstring>

namespace gus::app::screens {

int cockpit_verb_index_for_click_id(const char* element_id) noexcept {
    if (element_id == nullptr || element_id[0] == '\0') {
        return -1;
    }
    for (int i = 0; i < kBattleVerbCount; ++i) {
        if (std::strcmp(element_id, kCockpitVerbElementIds[i]) == 0) {
            return i;
        }
    }
    return -1;
}

}  // namespace gus::app::screens

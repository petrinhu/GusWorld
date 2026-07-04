// gus/app/screens/ui_hover.cpp
//
// Implementacao da fatia PURA do "som de hover" de botoes de UI (ver header).
// Travada por app/tests/ui_hover_test.cpp (TEST-FIRST, MESMO padrao do menu).

#include "gus/app/screens/ui_hover.hpp"

namespace gus::app::screens {

int ui_hover_index(float mouse_x, float mouse_y, const UiHoverBox* boxes,
                   int count) noexcept {
    if (boxes == nullptr || count <= 0) {
        return -1;
    }
    for (int i = 0; i < count; ++i) {
        const UiHoverBox& box = boxes[i];
        if (!box.found) continue;  // MESMO contrato do hit_test de clique
        if (mouse_x >= box.x && mouse_x <= box.x + box.w && mouse_y >= box.y &&
            mouse_y <= box.y + box.h) {
            return i;
        }
    }
    return -1;
}

bool ui_hover_entered_new_item(int previous_index, int current_index) noexcept {
    return current_index >= 0 && current_index != previous_index;
}

}  // namespace gus::app::screens

// gus/domain/input/action_registry.cpp
//
// Implementacao do registry canonico de actions. Ver header para o contrato e o
// mapeamento C# -> C++. A lista de 30 actions e portada 1:1 de
// engine/foundation/input_remap/ActionRegistry.cs (mesma ordem, mesmos nomes,
// categorias e label keys), MENOS as 7 actions de camera orbital 3/4 (era Godot),
// removidas na higiene M2/higiene-controles-godot: o C# espelhado ainda as tem
// (submodule Godot legado, fora de escopo), mas o jogo 2D top-down pos-ADR-008
// nunca teve essa camera -- eram mortas aqui.

#include "gus/domain/input/action_registry.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace gus::domain::input {

namespace {

// Singleton imutavel da lista canonica. Construido na primeira chamada (Meyers
// singleton): inicializacao thread-safe e ordem determinada, sem static-init
// fiasco. Espelha a ordem EXATA do ActionRegistry.Actions do C#.
const std::vector<ActionDefinition>& canonical_actions() {
    static const std::vector<ActionDefinition> kActions = {
        // 1. Movement
        {"move_forward", ActionCategory::Movement, "ACTION_MOVE_FORWARD"},
        {"move_backward", ActionCategory::Movement, "ACTION_MOVE_BACKWARD"},
        {"move_left", ActionCategory::Movement, "ACTION_MOVE_LEFT"},
        {"move_right", ActionCategory::Movement, "ACTION_MOVE_RIGHT"},
        {"move_run", ActionCategory::Movement, "ACTION_MOVE_RUN"},

        // 2. Interact
        {"interact", ActionCategory::Interact, "ACTION_INTERACT"},

        // 3. Menu (UI navigation)
        {"menu_open", ActionCategory::Menu, "ACTION_MENU_OPEN"},
        {"menu_close", ActionCategory::Menu, "ACTION_MENU_CLOSE"},
        {"menu_confirm", ActionCategory::Menu, "ACTION_MENU_CONFIRM"},
        {"menu_cancel", ActionCategory::Menu, "ACTION_MENU_CANCEL"},
        {"menu_nav_up", ActionCategory::Menu, "ACTION_MENU_NAV_UP"},
        {"menu_nav_down", ActionCategory::Menu, "ACTION_MENU_NAV_DOWN"},
        {"menu_nav_left", ActionCategory::Menu, "ACTION_MENU_NAV_LEFT"},
        {"menu_nav_right", ActionCategory::Menu, "ACTION_MENU_NAV_RIGHT"},

        // 4. Combat
        {"combat_attack_basic", ActionCategory::Combat, "ACTION_COMBAT_ATTACK_BASIC"},
        {"combat_defend", ActionCategory::Combat, "ACTION_COMBAT_DEFEND"},
        {"combat_cast", ActionCategory::Combat, "ACTION_COMBAT_CAST"},
        {"combat_card_1", ActionCategory::Combat, "ACTION_COMBAT_CARD_1"},
        {"combat_card_2", ActionCategory::Combat, "ACTION_COMBAT_CARD_2"},
        {"combat_card_3", ActionCategory::Combat, "ACTION_COMBAT_CARD_3"},
        {"combat_end_turn", ActionCategory::Combat, "ACTION_COMBAT_END_TURN"},

        // 5. Dialogue
        {"dialogue_continue", ActionCategory::Dialogue, "ACTION_DIALOGUE_CONTINUE"},
        {"dialogue_skip", ActionCategory::Dialogue, "ACTION_DIALOGUE_SKIP"},
        {"dialogue_choice_1", ActionCategory::Dialogue, "ACTION_DIALOGUE_CHOICE_1"},
        {"dialogue_choice_2", ActionCategory::Dialogue, "ACTION_DIALOGUE_CHOICE_2"},
        {"dialogue_choice_3", ActionCategory::Dialogue, "ACTION_DIALOGUE_CHOICE_3"},
        {"dialogue_choice_4", ActionCategory::Dialogue, "ACTION_DIALOGUE_CHOICE_4"},

        // 6. Inventory
        {"inventory_open", ActionCategory::Inventory, "ACTION_INVENTORY_OPEN"},
        {"inventory_close", ActionCategory::Inventory, "ACTION_INVENTORY_CLOSE"},

        // 7. Diary
        {"diary_open", ActionCategory::Diary, "ACTION_DIARY_OPEN"},
    };
    return kActions;
}

}  // namespace

const std::vector<ActionDefinition>& ActionRegistry::actions() {
    return canonical_actions();
}

int ActionRegistry::count() {
    return static_cast<int>(canonical_actions().size());
}

const ActionDefinition* ActionRegistry::get_by_name(std::string_view action_name) {
    for (const auto& a : canonical_actions()) {
        if (a.action_name == action_name) {
            return &a;
        }
    }
    return nullptr;
}

std::vector<const ActionDefinition*> ActionRegistry::get_by_category(ActionCategory category) {
    std::vector<const ActionDefinition*> result;
    for (const auto& a : canonical_actions()) {
        if (a.category == category) {
            result.push_back(&a);
        }
    }
    return result;
}

}  // namespace gus::domain::input

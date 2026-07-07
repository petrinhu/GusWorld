// input_remap_test.cpp
//
// Spec executavel do subsistema input_remap (marco M2, metade de logica pura).
// Porte 1:1 de engine/tests/input_remap/InputRemapTests.cs (xUnit -> Catch2 v3).
// Paridade total de casos.
//
// ESCOPO (fronteira POCO vs backend de evento Qt): igual ao C#.
//   - ActionRegistry (POCO static, 30 actions): contagem, lookup, categorias.
//     (higiene M2/higiene-controles-godot removeu as 7 actions de camera orbital
//     3/4 da era Godot, mortas pos-ADR-008: jogo 2D top-down nunca teve essa
//     camera. O ActionCategory::Camera correspondente tambem foi removido.)
//   - InputBinding records (POCO): igualdade por valor sensivel a modifier
//     (Ctrl/Shift/Alt) -- a PRIMITIVA em que a deteccao de conflito se apoia.
//   - InputRemapConfig: estrutura/versao.
//
//   FORA daqui (vivem no futuro platform/input/, que usa eventos Qt ->
//   exige runtime, NAO testavel headless): DetectConflict, ResetActionToDefault,
//   ResetAllToDefaults, ApplyTo<Backend>InputMap. Integration/runtime, nao unit POCO.
//
// Cross-ref: GusEngine/domain/include/gus/domain/input/{action_registry,input_binding}.hpp;
//            engine/foundation/input_remap/{ActionRegistry,InputBinding}.cs.

#include <algorithm>
#include <set>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include "gus/domain/input/action_registry.hpp"
#include "gus/domain/input/input_binding.hpp"

using namespace gus::domain::input;

// ---- ActionRegistry: contagem e integridade ----

TEST_CASE("Registry_Has30CanonicalActions", "[input_remap][registry]") {
    REQUIRE(ActionRegistry::count() == 30);
    REQUIRE(ActionRegistry::actions().size() == 30);
}

TEST_CASE("Registry_ActionNames_AreUnique", "[input_remap][registry]") {
    std::set<std::string> names;
    for (const auto& a : ActionRegistry::actions()) {
        names.insert(a.action_name);
    }
    REQUIRE(names.size() == ActionRegistry::actions().size());
}

TEST_CASE("Registry_EveryAction_HasNameAndLabelKey", "[input_remap][registry]") {
    for (const auto& a : ActionRegistry::actions()) {
        INFO("ActionName vazio");
        REQUIRE_FALSE(a.action_name.empty());
        INFO("LabelI18nKey vazio para " << a.action_name);
        REQUIRE_FALSE(a.label_i18n_key.empty());
    }
}

TEST_CASE("Registry_EveryLabelKey_IsUpperSnakeWithActionPrefix", "[input_remap][registry]") {
    // Convencao: ACTION_* UPPER_SNAKE (casa com chaves de traducao).
    using Catch::Matchers::StartsWith;
    using Catch::Matchers::Matches;
    for (const auto& a : ActionRegistry::actions()) {
        REQUIRE_THAT(a.label_i18n_key, StartsWith("ACTION_"));
        REQUIRE_THAT(a.label_i18n_key, Matches("[A-Z][A-Z0-9_]*"));
    }
}

// ---- ActionRegistry: lookup ----

TEST_CASE("GetByName_KnownAction_ReturnsCorrectCategory", "[input_remap][registry]") {
    struct Case {
        std::string name;
        ActionCategory category;
    };
    const std::vector<Case> cases = {
        {"move_forward", ActionCategory::Movement},
        {"interact", ActionCategory::Interact},
        {"combat_card_1", ActionCategory::Combat},
        {"dialogue_continue", ActionCategory::Dialogue},
        {"inventory_open", ActionCategory::Inventory},
        {"diary_open", ActionCategory::Diary},
    };
    for (const auto& c : cases) {
        INFO("action: " << c.name);
        const auto* def = ActionRegistry::get_by_name(c.name);
        REQUIRE(def != nullptr);
        REQUIRE(def->action_name == c.name);
        REQUIRE(def->category == c.category);
    }
}

TEST_CASE("GetByName_UnknownAction_ReturnsNull", "[input_remap][registry]") {
    REQUIRE(ActionRegistry::get_by_name("does_not_exist") == nullptr);
}

// ---- ActionRegistry: categorias ----

TEST_CASE("GetByCategory_Movement_Returns5Actions", "[input_remap][registry]") {
    const auto movement = ActionRegistry::get_by_category(ActionCategory::Movement);
    REQUIRE(movement.size() == 5);
    for (const auto* a : movement) {
        REQUIRE(a->category == ActionCategory::Movement);
    }
}

TEST_CASE("GetByCategory_Combat_Returns7Actions", "[input_remap][registry]") {
    const auto combat = ActionRegistry::get_by_category(ActionCategory::Combat);
    REQUIRE(combat.size() == 7);
}

TEST_CASE("GetByCategory_CoversAllActions_SumEquals30", "[input_remap][registry]") {
    // Soma das categorias = total (cada action em exatamente uma categoria).
    int total = 0;
    for (const auto cat : all_action_categories()) {
        total += static_cast<int>(ActionRegistry::get_by_category(cat).size());
    }
    REQUIRE(total == 30);
}

TEST_CASE("GetByCategory_DiaryValue_ReturnsSingle", "[input_remap][registry]") {
    // Categoria Diary tem exatamente 1 action (diary_open).
    const auto diary = ActionRegistry::get_by_category(ActionCategory::Diary);
    REQUIRE(diary.size() == 1);
}

// ---- KeyBinding: igualdade sensivel a modifier (primitiva de conflito) ----

TEST_CASE("KeyBinding_SameKeycodeNoModifiers_AreEqual", "[input_remap][binding]") {
    const KeyBinding a{.keycode = 65};
    const KeyBinding b{.keycode = 65};
    REQUIRE(a == b);
    REQUIRE(std::hash<KeyBinding>{}(a) == std::hash<KeyBinding>{}(b));
}

TEST_CASE("KeyBinding_DifferentKeycode_AreNotEqual", "[input_remap][binding]") {
    const KeyBinding a{.keycode = 65};
    const KeyBinding b{.keycode = 66};
    REQUIRE(a != b);
}

TEST_CASE("KeyBinding_SameKeyDifferentCtrl_AreNotEqual", "[input_remap][binding]") {
    // Ctrl+A != A: deteccao de conflito DEVE distinguir por modifier.
    const KeyBinding plain{.keycode = 65, .ctrl_pressed = false};
    const KeyBinding with_ctrl{.keycode = 65, .ctrl_pressed = true};
    REQUIRE(plain != with_ctrl);
}

TEST_CASE("KeyBinding_ShiftAndAltDistinguished", "[input_remap][binding]") {
    const KeyBinding shift{.keycode = 65, .shift_pressed = true};
    const KeyBinding alt{.keycode = 65, .alt_pressed = true};
    REQUIRE(shift != alt);
}

TEST_CASE("KeyBinding_AllModifiersMatching_AreEqual", "[input_remap][binding]") {
    const KeyBinding a{
        .keycode = 88, .ctrl_pressed = true, .shift_pressed = true, .alt_pressed = false};
    const KeyBinding b{
        .keycode = 88, .ctrl_pressed = true, .shift_pressed = true, .alt_pressed = false};
    REQUIRE(a == b);
}

// ---- ActionBindings / GamepadButtonBinding / MouseButtonBinding / Axis ----

TEST_CASE("ActionBindings_DefaultDeadzone_IsHalf", "[input_remap][binding]") {
    const ActionBindings ab{.action_name = "move_forward"};
    REQUIRE_THAT(ab.deadzone, Catch::Matchers::WithinULP(0.5f, 0));
}

TEST_CASE("ActionBindings_DefaultLists_AreEmptyNotNull", "[input_remap][binding]") {
    // C++: vetores membros sao sempre validos (nao ha null); confere que sao vazios.
    const ActionBindings ab{.action_name = "x"};
    REQUIRE(ab.keys.empty());
    REQUIRE(ab.gamepad_buttons.empty());
    REQUIRE(ab.mouse_buttons.empty());
    REQUIRE(ab.gamepad_axes.empty());
}

TEST_CASE("ActionBindings_CanHoldMultipleBindings_WasdPlusArrows", "[input_remap][binding]") {
    // Action MAY ter multiplos bindings (ex: W + Up arrow + left stick).
    ActionBindings ab{.action_name = "move_forward"};
    ab.keys = {
        KeyBinding{.keycode = 87},        // W
        KeyBinding{.keycode = 16777217},  // Up (dummy)
    };
    ab.gamepad_axes = {
        GamepadAxisBinding{.axis = 1, .axis_value = -1.0f},
    };
    REQUIRE(ab.keys.size() == 2);
    REQUIRE(ab.gamepad_axes.size() == 1);
}

TEST_CASE("GamepadButtonBinding_EqualityByValue", "[input_remap][binding]") {
    REQUIRE(GamepadButtonBinding{.button_index = 0} == GamepadButtonBinding{.button_index = 0});
    REQUIRE(GamepadButtonBinding{.button_index = 0} != GamepadButtonBinding{.button_index = 1});
}

TEST_CASE("GamepadAxisBinding_DistinguishesAxisAndValue", "[input_remap][binding]") {
    const GamepadAxisBinding left{.axis = 0, .axis_value = -1.0f};
    const GamepadAxisBinding right{.axis = 0, .axis_value = 1.0f};
    REQUIRE(left != right);  // mesmo eixo, direcoes opostas
}

// ---- InputRemapConfig ----

TEST_CASE("InputRemapConfig_DefaultVersion_IsOne", "[input_remap][config]") {
    const InputRemapConfig cfg{};
    REQUIRE(cfg.config_version == 1);
    REQUIRE(cfg.actions.empty());
}

TEST_CASE("InputRemapConfig_HoldsActionBindings", "[input_remap][config]") {
    InputRemapConfig cfg{};
    ActionBindings interact{.action_name = "interact"};
    interact.keys = {KeyBinding{.keycode = 70}};
    cfg.actions = {interact};
    REQUIRE(cfg.actions.size() == 1);
    REQUIRE(cfg.actions[0].action_name == "interact");
}

// controls_restore_test.cpp
//
// Spec executavel (Catch2 v3) da restauracao de controles e da fabrica de defaults
// (ADR-007 item 5). POCO puro, ZERO Qt, ZERO disco.
//
// Contrato:
//   - default_controls() -> InputRemapConfig: fonte UNICA do esquema de fabrica.
//     Determinismo (mesma fabrica sempre); cobre as 30 actions do ActionRegistry
//     (higiene M2/higiene-controles-godot removeu as 7 actions de camera orbital
//     3/4 da era Godot, mortas pos-ADR-008); config_version = 1.
//   - restore_from_save(save) -> save.input_remap_backup (o backup embutido no save).
//
// O backup viaja em TODO save (ADR-007 item 3): restore_from_save apenas devolve o
// que esta no save mais recente. A reescrita do arquivo (I/O) e platform.
//
// Cross-ref: docs/tech/adr/ADR-007-controls-json-hash128-save-v4.md (item 5),
//            gus/domain/input/controls_restore.hpp, save_data.hpp.

#include <catch2/catch_test_macros.hpp>

#include <set>
#include <string>

#include "gus/domain/input/action_registry.hpp"
#include "gus/domain/input/controls_restore.hpp"
#include "gus/domain/input/input_binding.hpp"
#include "gus/domain/save/save_data.hpp"

using gus::domain::input::ActionBindings;
using gus::domain::input::ActionRegistry;
using gus::domain::input::default_controls;
using gus::domain::input::InputRemapConfig;
using gus::domain::input::KeyBinding;
using gus::domain::input::restore_from_save;
using gus::domain::save::SaveData;

// ---- default_controls: fonte unica de fabrica ------------------------------

TEST_CASE("controls_restore: default_controls e deterministico",
          "[domain][input][controls_restore]") {
    REQUIRE(default_controls() == default_controls());
}

TEST_CASE("controls_restore: default_controls tem config_version 1",
          "[domain][input][controls_restore]") {
    REQUIRE(default_controls().config_version == 1);
}

TEST_CASE("controls_restore: default cobre todas as 30 actions canonicas",
          "[domain][input][controls_restore]") {
    const auto cfg = default_controls();
    REQUIRE(cfg.actions.size() == static_cast<std::size_t>(ActionRegistry::count()));
    // Cada action do default e uma action canonica conhecida; nomes unicos.
    std::set<std::string> names;
    for (const auto& a : cfg.actions) {
        INFO("action: " << a.action_name);
        REQUIRE(ActionRegistry::get_by_name(a.action_name) != nullptr);
        names.insert(a.action_name);
    }
    REQUIRE(names.size() == cfg.actions.size());
}

TEST_CASE("controls_restore: default tem pelo menos um binding por action",
          "[domain][input][controls_restore]") {
    for (const auto& a : default_controls().actions) {
        INFO("action sem binding: " << a.action_name);
        const bool has_any = !a.keys.empty() || !a.gamepad_buttons.empty() ||
                             !a.mouse_buttons.empty() || !a.gamepad_axes.empty();
        REQUIRE(has_any);
    }
}

TEST_CASE("controls_restore: default define WASD em move_*",
          "[domain][input][controls_restore]") {
    // Sanidade do esquema de fabrica (Godot keycodes W/A/S/D = 87/65/83/68).
    const auto cfg = default_controls();
    auto first_key = [&](const std::string& action) -> long long {
        for (const auto& a : cfg.actions)
            if (a.action_name == action && !a.keys.empty()) return a.keys[0].keycode;
        return -1;
    };
    REQUIRE(first_key("move_forward") == 87);   // W
    REQUIRE(first_key("move_left") == 65);       // A
    REQUIRE(first_key("move_backward") == 83);   // S
    REQUIRE(first_key("move_right") == 68);      // D
}

// ---- restore_from_save devolve o backup embutido ---------------------------

TEST_CASE("controls_restore: restore_from_save devolve o backup do save",
          "[domain][input][controls_restore]") {
    SaveData save;
    InputRemapConfig custom = default_controls();
    // Altera um binding no backup pra distinguir de um default puro.
    custom.actions[0].keys = {KeyBinding{.keycode = 12345}};
    save.input_remap_backup = custom;

    const auto restored = restore_from_save(save);
    REQUIRE(restored == custom);
}

TEST_CASE("controls_restore: restore de save com backup default devolve default",
          "[domain][input][controls_restore]") {
    SaveData save;
    save.input_remap_backup = default_controls();
    REQUIRE(restore_from_save(save) == default_controls());
}

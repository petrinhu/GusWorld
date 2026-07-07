// gus/domain/src/input/controls_restore.cpp
//
// Fabrica de controles default + restauracao a partir do backup do save (ADR-007
// item 5). POCO puro, ZERO Qt, ZERO disco. Ver header. Travado por
// tests/controls_restore_test.cpp.
//
// O CONTEUDO concreto dos defaults (quais teclas) e design de input; este arquivo e
// a UNICA fonte canonica pura do esquema de fabrica. Cada action canonica recebe ao
// menos um binding. Keycodes seguem o Godot 4 (Key enum): letras = ASCII maiusculo,
// teclas nomeadas com os valores do enum.

#include "gus/domain/input/controls_restore.hpp"

#include <string>

#include "gus/domain/input/action_registry.hpp"
#include "gus/domain/save/save_data.hpp"

namespace gus::domain::input {

namespace {

// Godot 4 Key enum (subconjunto usado nos defaults).
constexpr long long kKeyEnter = 4194309;
constexpr long long kKeyEscape = 4194305;
constexpr long long kKeyTab = 4194308;
constexpr long long kKeyLeft = 4194319;
constexpr long long kKeyRight = 4194321;
constexpr long long kKeyUp = 4194320;
constexpr long long kKeyDown = 4194322;
constexpr long long kKeyShift = 4194325;
constexpr long long kKeySpace = 32;

ActionBindings key_action(const std::string& name, long long keycode,
                          bool ctrl = false, bool shift = false, bool alt = false) {
    ActionBindings a;
    a.action_name = name;
    a.keys = {KeyBinding{.keycode = keycode,
                         .ctrl_pressed = ctrl,
                         .shift_pressed = shift,
                         .alt_pressed = alt}};
    return a;
}

// Mapa canonico action_name -> binding default. Toda action do ActionRegistry DEVE
// estar aqui (o teste exige >=1 binding por action). Mantem WASD em move_*.
ActionBindings default_for(const std::string& name) {
    // Movimento: WASD (+ correr no Shift).
    if (name == "move_forward") return key_action(name, 'W');
    if (name == "move_backward") return key_action(name, 'S');
    if (name == "move_left") return key_action(name, 'A');
    if (name == "move_right") return key_action(name, 'D');
    if (name == "move_run") return key_action(name, kKeyShift);

    // Interact.
    if (name == "interact") return key_action(name, kKeyEnter);

    // Menu.
    if (name == "menu_open") return key_action(name, kKeyEscape);
    if (name == "menu_close") return key_action(name, kKeyEscape);
    if (name == "menu_confirm") return key_action(name, kKeyEnter);
    if (name == "menu_cancel") return key_action(name, kKeyEscape);
    if (name == "menu_nav_up") return key_action(name, kKeyUp);
    if (name == "menu_nav_down") return key_action(name, kKeyDown);
    if (name == "menu_nav_left") return key_action(name, kKeyLeft);
    if (name == "menu_nav_right") return key_action(name, kKeyRight);

    // Combat.
    if (name == "combat_attack_basic") return key_action(name, 'Z');
    if (name == "combat_defend") return key_action(name, 'X');
    if (name == "combat_cast") return key_action(name, kKeySpace);
    if (name == "combat_card_1") return key_action(name, '1');
    if (name == "combat_card_2") return key_action(name, '2');
    if (name == "combat_card_3") return key_action(name, '3');
    if (name == "combat_end_turn") return key_action(name, kKeyTab);

    // Dialogue.
    if (name == "dialogue_continue") return key_action(name, kKeyEnter);
    if (name == "dialogue_skip") return key_action(name, kKeyEscape);
    if (name == "dialogue_choice_1") return key_action(name, '1');
    if (name == "dialogue_choice_2") return key_action(name, '2');
    if (name == "dialogue_choice_3") return key_action(name, '3');
    if (name == "dialogue_choice_4") return key_action(name, '4');

    // Inventory.
    if (name == "inventory_open") return key_action(name, 'I');
    if (name == "inventory_close") return key_action(name, kKeyEscape);

    // Diary.
    if (name == "diary_open") return key_action(name, 'J');

    // Defesa: action canonica sem default explicito ainda recebe um binding neutro
    // (o teste exige >=1 binding por action). Nunca deve acontecer em producao (o
    // mapa acima cobre as 30); se uma action nova entrar no registry sem default
    // aqui, ela cai neste fallback e o dev deve corrigir.
    return key_action(name, kKeySpace);
}

}  // namespace

InputRemapConfig default_controls() {
    InputRemapConfig cfg;
    cfg.config_version = 1;
    cfg.actions.reserve(ActionRegistry::actions().size());
    // Na ordem do registry (determinismo do canonico).
    for (const auto& def : ActionRegistry::actions()) {
        cfg.actions.push_back(default_for(def.action_name));
    }
    return cfg;
}

InputRemapConfig restore_from_save(const gus::domain::save::SaveData& most_recent) {
    return most_recent.input_remap_backup;
}

}  // namespace gus::domain::input

// GusEngine/domain/tests/controls_remap_apply_test.cpp
//
// Catch2 (TEST-FIRST) de apply_key_remap (tela Controles/M2): swap-on-conflict
// PURO. Ver gus/domain/input/controls_remap_apply.hpp para o contrato completo.
// Fixtures partem de default_controls() (WASD + setas + Shift/Enter/Esc/Tab/
// Espaco/digitos, ver controls_restore.cpp) - mesmas fixtures usadas por
// controls_restore_test.cpp/controls_diff_test.cpp.

#include <catch2/catch_test_macros.hpp>

#include "gus/domain/input/action_registry.hpp"
#include "gus/domain/input/controls_remap_apply.hpp"
#include "gus/domain/input/controls_restore.hpp"

using namespace gus::domain::input;

namespace {

const ActionBindings* find_action(const InputRemapConfig& cfg, const std::string& name) {
    for (const auto& a : cfg.actions) {
        if (a.action_name == name) return &a;
    }
    return nullptr;
}

KeyBinding key(long long keycode) {
    return KeyBinding{.keycode = keycode, .ctrl_pressed = false, .shift_pressed = false,
                      .alt_pressed = false};
}

}  // namespace

TEST_CASE("apply_key_remap: tecla LIVRE - so a action-alvo muda, sem swap",
          "[controls_remap_apply]") {
    const InputRemapConfig before = default_controls();
    // 'K' nao e default de nenhuma action -> livre.
    const KeyRemapResult result = apply_key_remap(before, "diary_open", key('K'));

    REQUIRE(result.changed);
    REQUIRE_FALSE(result.swapped);
    REQUIRE(result.swapped_with_action_name.empty());

    const ActionBindings* diary = find_action(result.config, "diary_open");
    REQUIRE(diary != nullptr);
    REQUIRE(diary->keys.size() == 1);
    REQUIRE(diary->keys.front() == key('K'));

    // Nenhuma outra action foi tocada (spot-check: move_forward continua W).
    const ActionBindings* move_forward = find_action(result.config, "move_forward");
    REQUIRE(move_forward != nullptr);
    REQUIRE(move_forward->keys.front() == key('W'));
}

TEST_CASE("apply_key_remap: no-op quando a tecla nova E a tecla ATUAL da action",
          "[controls_remap_apply]") {
    const InputRemapConfig before = default_controls();
    // diary_open ja e 'J' por default (controls_restore.cpp).
    const KeyRemapResult result = apply_key_remap(before, "diary_open", key('J'));

    REQUIRE_FALSE(result.changed);
    REQUIRE_FALSE(result.swapped);
    REQUIRE(result.config == before);  // config INTACTO
}

TEST_CASE("apply_key_remap: CONFLITO -> TROCA (decisao do lider, nao recusa)",
          "[controls_remap_apply]") {
    const InputRemapConfig before = default_controls();
    // move_forward = 'W' por default. Rebind diary_open ('J') para 'W'.
    const KeyRemapResult result = apply_key_remap(before, "diary_open", key('W'));

    REQUIRE(result.changed);
    REQUIRE(result.swapped);
    REQUIRE(result.swapped_with_action_name == "move_forward");
    REQUIRE(result.swapped_with_label_i18n_key == "ACTION_MOVE_FORWARD");

    const ActionBindings* diary = find_action(result.config, "diary_open");
    const ActionBindings* move_forward = find_action(result.config, "move_forward");
    REQUIRE(diary != nullptr);
    REQUIRE(move_forward != nullptr);
    // diary_open ganhou a tecla nova.
    REQUIRE(diary->keys.size() == 1);
    REQUIRE(diary->keys.front() == key('W'));
    // move_forward recebeu a tecla ANTIGA de diary_open ('J') - ninguem fica
    // sem tecla.
    REQUIRE(move_forward->keys.size() == 1);
    REQUIRE(move_forward->keys.front() == key('J'));
}

TEST_CASE("apply_key_remap: tecla compartilhada por VARIAS actions (ex.: Esc) so "
          "troca com a PRIMEIRA (ordem do ActionRegistry) - as demais ficam intocadas",
          "[controls_remap_apply]") {
    const InputRemapConfig before = default_controls();
    constexpr long long kKeyEscape = 4194305;

    // Esc e default de menu_open/menu_close/menu_cancel/dialogue_skip/
    // inventory_close (ver controls_restore.cpp) - um compartilhamento
    // PRE-EXISTENTE e intencional (nao e este remap que criou). Rebind
    // move_forward ('W') para Esc.
    const KeyRemapResult result = apply_key_remap(before, "move_forward", key(kKeyEscape));

    REQUIRE(result.changed);
    REQUIRE(result.swapped);
    // menu_open e o PRIMEIRO na ordem do ActionRegistry entre os que tem Esc.
    REQUIRE(result.swapped_with_action_name == "menu_open");

    const ActionBindings* move_forward = find_action(result.config, "move_forward");
    const ActionBindings* menu_open = find_action(result.config, "menu_open");
    REQUIRE(move_forward->keys.front() == key(kKeyEscape));
    REQUIRE(menu_open->keys.front() == key('W'));  // recebeu a tecla antiga de move_forward

    // As DEMAIS actions que compartilhavam Esc continuam com Esc (so a
    // PRIMEIRA foi trocada, o resto do compartilhamento pre-existente fica
    // como estava).
    for (const char* name :
         {"menu_close", "menu_cancel", "dialogue_skip", "inventory_close"}) {
        const ActionBindings* a = find_action(result.config, name);
        REQUIRE(a != nullptr);
        REQUIRE(a->keys.front() == key(kKeyEscape));
    }
}

TEST_CASE("apply_key_remap: action_name DESCONHECIDA e no-op defensivo (config intacto)",
          "[controls_remap_apply]") {
    const InputRemapConfig before = default_controls();
    const KeyRemapResult result = apply_key_remap(before, "nao_existe_essa_action", key('K'));

    REQUIRE_FALSE(result.changed);
    REQUIRE_FALSE(result.swapped);
    REQUIRE(result.config == before);
}

TEST_CASE("apply_key_remap: verifica ANTES que a ordem do ActionRegistry realmente "
          "coloca menu_open antes das demais actions que usam Esc por default "
          "(pre-condicao do teste de swap-com-varios-conflitantes acima)",
          "[controls_remap_apply]") {
    bool seen_menu_open = false;
    for (const auto& def : ActionRegistry::actions()) {
        if (def.action_name == "menu_open") {
            seen_menu_open = true;
        }
        if (def.action_name == "menu_close" || def.action_name == "menu_cancel" ||
            def.action_name == "dialogue_skip" || def.action_name == "inventory_close") {
            REQUIRE(seen_menu_open);
        }
    }
    REQUIRE(seen_menu_open);
}

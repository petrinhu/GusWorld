// gus/domain/src/input/controls_diff.cpp
//
// Diff puro de controles + tabela keycode->rotulo legivel (ADR-007 item 4). POCO
// puro, ZERO Qt, ZERO UI. Ver header. Travado por tests/controls_diff_test.cpp.
//
// A tabela de keycodes segue os valores do Godot 4 (Key enum): letras A..Z = 65..90
// (ASCII maiusculo), digitos 0..9 = 48..57, alguns nomeados. Como o rotulo final
// passa pela i18n da UI (via label_i18n_key da action), aqui devolvemos um rotulo
// legivel ESTAVEL da tecla (em pt-br para o dev); a UI MAY re-traduzir a tecla se
// quiser, mas o caso do lider ("era Espaco, esta D") ja fica resolvido.

#include "gus/domain/input/controls_diff.hpp"

#include <string>

#include "gus/domain/input/action_registry.hpp"

namespace gus::domain::input {

namespace {

// Procura uma action por nome num config; nullptr se ausente.
const ActionBindings* find_action(const InputRemapConfig& cfg,
                                  const std::string& name) {
    for (const auto& a : cfg.actions)
        if (a.action_name == name) return &a;
    return nullptr;
}

// Compara duas listas de bindings de uma action (semantica). nullptr = "action
// ausente naquele config" (lista efetivamente vazia, sem deadzone customizado).
bool bindings_differ(const ActionBindings* a, const ActionBindings* b) {
    static const ActionBindings kEmpty{};
    const ActionBindings& x = a ? *a : kEmpty;
    const ActionBindings& y = b ? *b : kEmpty;
    // operator== de ActionBindings compara action_name tambem; aqui comparamos so o
    // conteudo de binding (mesma action), entao olhamos os campos diretamente.
    return x.keys != y.keys || x.gamepad_buttons != y.gamepad_buttons ||
           x.mouse_buttons != y.mouse_buttons || x.gamepad_axes != y.gamepad_axes ||
           x.deadzone != y.deadzone;
}

// Prefixo de modifiers (Ctrl+/Shift+/Alt+) do primeiro key-binding.
std::string modifier_prefix(const KeyBinding& k) {
    std::string p;
    if (k.ctrl_pressed) p += "Ctrl+";
    if (k.shift_pressed) p += "Shift+";
    if (k.alt_pressed) p += "Alt+";
    return p;
}

// Rotulo humano do primeiro key-binding de uma action (vazio se nao tem key).
std::string first_key_human(const ActionBindings* a) {
    if (a == nullptr || a->keys.empty()) return "";
    const KeyBinding& k = a->keys.front();
    return modifier_prefix(k) + human_label_for_keycode(k.keycode);
}

}  // namespace

std::string human_label_for_keycode(long long keycode) {
    // Nomeadas comuns (subconjunto do Godot Key enum, pt-br para o dev).
    switch (keycode) {
        case 32: return "Espaco";        // KEY_SPACE
        case 4194309: return "Enter";    // KEY_ENTER (Godot 4)
        case 13: return "Enter";         // tambem aceito (teste usa 13)
        case 4194308: return "Tab";      // KEY_TAB
        case 9: return "Tab";
        case 4194305: return "Esc";      // KEY_ESCAPE
        case 27: return "Esc";
        case 4194319: return "Esquerda"; // KEY_LEFT
        case 4194321: return "Direita";  // KEY_RIGHT
        case 4194320: return "Cima";     // KEY_UP
        case 4194322: return "Baixo";    // KEY_DOWN
        case 4194306: return "Backspace";
        default: break;
    }
    // Letras A..Z (Godot usa o ASCII maiusculo).
    if (keycode >= 'A' && keycode <= 'Z')
        return std::string(1, static_cast<char>(keycode));
    // Letras minusculas a..z (defensivo): apresenta em maiuscula.
    if (keycode >= 'a' && keycode <= 'z')
        return std::string(1, static_cast<char>(keycode - 32));
    // Digitos 0..9.
    if (keycode >= '0' && keycode <= '9')
        return std::string(1, static_cast<char>(keycode));
    // Fallback nunca-vazio para teclas exoticas/sem rotulo.
    return "Tecla " + std::to_string(keycode);
}

ControlsDiff diff_controls(const InputRemapConfig& was, const InputRemapConfig& now) {
    ControlsDiff diff;
    // Itera na ORDEM do ActionRegistry (ordem estavel do diff). Cada action canonica
    // que mudou de binding entre was/now vira um BindingChange.
    for (const auto& def : ActionRegistry::actions()) {
        const ActionBindings* a = find_action(was, def.action_name);
        const ActionBindings* b = find_action(now, def.action_name);
        if (a == nullptr && b == nullptr) continue;  // nem antes nem depois
        if (!bindings_differ(a, b)) continue;        // igual: nao entra no diff

        diff.changes.push_back(BindingChange{
            def.action_name,
            def.label_i18n_key,
            first_key_human(a),
            first_key_human(b),
        });
    }
    return diff;
}

}  // namespace gus::domain::input

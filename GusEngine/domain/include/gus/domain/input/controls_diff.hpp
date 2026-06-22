// gus/domain/input/controls_diff.hpp
//
// Diff PURO entre dois InputRemapConfig (ADR-007 item 4). POCO puro, ZERO Qt, ZERO
// UI. Devolve a lista de actions cujo binding mudou, com dados + chave i18n + um
// rotulo humano legivel do binding ("Espaco", "D", "Ctrl+A"). A TRADUCAO final do
// rotulo da action e da camada de apresentacao via label_i18n_key; aqui o domain
// entrega o necessario (dados + key + representacao legivel da TECLA).
//
// Exemplo do lider: "magia: era Espaco, esta D" =
//   BindingChange{action_name="combat_cast", label_i18n_key="ACTION_COMBAT_CAST",
//                 was_human="Espaco", now_human="D"}.
//
// Cross-ref: docs/tech/adr/ADR-007-controls-json-hash128-save-v4.md (item 4),
//            gus/domain/input/input_binding.hpp, action_registry.hpp.

#ifndef GUS_DOMAIN_INPUT_CONTROLS_DIFF_HPP
#define GUS_DOMAIN_INPUT_CONTROLS_DIFF_HPP

#include <string>
#include <vector>

#include "gus/domain/input/input_binding.hpp"

namespace gus::domain::input {

// Uma action que mudou de binding entre dois configs.
struct BindingChange {
    std::string action_name;     // ex.: "combat_cast"
    std::string label_i18n_key;  // do ActionRegistry, pra UI traduzir o rotulo
    std::string was_human;       // rotulo legivel do binding anterior ("Espaco")
    std::string now_human;       // rotulo legivel do binding atual ("D")

    [[nodiscard]] bool operator==(const BindingChange&) const = default;
};

// Resultado do diff: so as actions que mudaram, na ordem do ActionRegistry.
struct ControlsDiff {
    std::vector<BindingChange> changes;

    [[nodiscard]] bool empty() const { return changes.empty(); }
    [[nodiscard]] bool operator==(const ControlsDiff&) const = default;
};

// Calcula o diff entre 'was' e 'now'. Pura. Considera mudou-de-binding qualquer
// action cuja lista de bindings (keys/gamepad/mouse/axes) ou deadzone difira.
[[nodiscard]] ControlsDiff diff_controls(const InputRemapConfig& was,
                                         const InputRemapConfig& now);

// Rotulo legivel de um keycode (Godot). Tabela de dado puro. Fallback nao-vazio
// ("Tecla <codigo>") para teclas sem rotulo conhecido. Exposta para teste/UI.
[[nodiscard]] std::string human_label_for_keycode(long long keycode);

}  // namespace gus::domain::input

#endif  // GUS_DOMAIN_INPUT_CONTROLS_DIFF_HPP

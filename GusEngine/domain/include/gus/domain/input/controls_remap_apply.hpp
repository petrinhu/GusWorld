// gus/domain/input/controls_remap_apply.hpp
//
// Aplica um novo KEYBOARD binding a uma action do InputRemapConfig, com
// RESOLUCAO DE CONFLITO POR TROCA (decisao do lider, tela Controles/M2: "avisa e
// troca", NAO "avisa e recusa") - se a tecla nova ja pertence a OUTRA action, as
// duas TROCAM de tecla entre si; ninguem fica sem tecla. POCO puro, ZERO Qt, ZERO
// SDL, ZERO I/O (mesma familia de controls_diff.hpp/controls_restore.hpp).
//
// ESCOPO (MVP tela Controles, mock docs/design/mockups/06-controles-remap.html):
//   - So o KEYBOARD e editado aqui (decisao 2 do lider: gamepad fica read-only
//     nesta onda, remap de gamepad e peca futura). gamepad_buttons/mouse_buttons/
//     gamepad_axes de `cfg` NUNCA sao tocados por esta funcao.
//   - Cada action passa a ter EXATAMENTE 1 key binding apos a chamada (`keys`
//     vira sempre {new_key} ou permanece {old_key} em caso de no-op) - a tela so
//     mostra/edita 1 keycap de teclado por acao (nao uma lista).
//   - Modifiers (Ctrl/Shift/Alt) NAO sao capturados nesta onda (mesmo escopo M1 ja
//     documentado em platform/input/input_mapper.hpp - "considera apenas o
//     KEYCODE"): o KeyBinding aplicado tem os 3 modifiers em false; a deteccao de
//     conflito/troca tambem compara so o valor pleno de KeyBinding (keycode +
//     modifiers, todos false aqui), consistente com o operator== do struct.
//
// Cross-ref: docs/design/mockups/06-controles-remap.html (decisao 1: swap),
//            gus/domain/input/action_registry.hpp (ordem canonica usada para
//            desempate quando mais de 1 action ja compartilha a tecla-alvo -
//            ex.: Esc e default de varias actions de Menu/Dialogue/Inventory),
//            gus/domain/input/input_binding.hpp, gus/domain/input/controls_restore.hpp.

#ifndef GUS_DOMAIN_INPUT_CONTROLS_REMAP_APPLY_HPP
#define GUS_DOMAIN_INPUT_CONTROLS_REMAP_APPLY_HPP

#include <string>

#include "gus/domain/input/input_binding.hpp"

namespace gus::domain::input {

// Resultado de aplicar um remap: o config resultante + o que aconteceu (a UI
// consome swapped/swapped_with_* pra montar o aviso "trocou com X", decisao 1 do
// lider "avisa e troca").
struct KeyRemapResult {
    InputRemapConfig config;
    bool changed = false;                     // false = no-op (ver regras abaixo)
    bool swapped = false;                     // true = houve TROCA com outra action
    std::string swapped_with_action_name;      // nome da OUTRA action que perdeu a tecla (vazio se !swapped)
    std::string swapped_with_label_i18n_key;   // label i18n dela, pronto pro aviso da UI

    [[nodiscard]] bool operator==(const KeyRemapResult&) const = default;
};

// Aplica `new_key` como o UNICO key binding de `action_name` dentro de `cfg`.
// Regras (decisao do lider, "avisa e troca"):
//   - `action_name` desconhecida (fora do ActionRegistry) OU ausente de
//     `cfg.actions`: no-op defensivo, `cfg` devolvido intacto (changed=false,
//     swapped=false).
//   - `new_key` ja e a tecla ATUAL de `action_name` (jogador re-selecionou a
//     mesma tecla): no-op semantico, `cfg` devolvido intacto (changed=false).
//   - `new_key` ja pertence a OUTRA action presente em `cfg.actions` (a PRIMEIRA
//     encontrada na ORDEM do ActionRegistry, excluindo `action_name` - desempate
//     deterministico quando mais de 1 action ja compartilha essa tecla por
//     default): as duas TROCAM - a outra action recebe a tecla ANTIGA de
//     `action_name`; swapped=true, swapped_with_* preenchidos. Nenhuma OUTRA
//     action que porventura tambem compartilhe a tecla-alvo e tocada (so a
//     PRIMEIRA - o resto do compartilhamento pre-existente, se houver, fica como
//     estava).
//   - Caso contrario (tecla livre): `action_name` recebe `new_key`; swapped=false.
// Pura, deterministica, nunca lanca.
[[nodiscard]] KeyRemapResult apply_key_remap(const InputRemapConfig& cfg,
                                              const std::string& action_name,
                                              const KeyBinding& new_key);

}  // namespace gus::domain::input

#endif  // GUS_DOMAIN_INPUT_CONTROLS_REMAP_APPLY_HPP

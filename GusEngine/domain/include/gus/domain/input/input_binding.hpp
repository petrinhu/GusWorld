// gus/domain/input/input_binding.hpp
//
// POCO records de binding de InputAction, portados de
// engine/foundation/input_remap/InputBinding.cs. Header-only: dados puros com
// igualdade por valor (operator== = default), mesmo padrao de combat_records.hpp.
//
// POCO puro, ZERO Qt (invariante de domain/, engine-design.md secao 2). E o MAPA
// LOGICO acao->tecla: o "mapa acao->tecla e puro; so o backend de evento muda"
// (engine-design.md secao 3). O backend de evento Qt vive em platform/input/ e
// NAO faz parte deste porte (M2, metade de logica pura).
//
// MAPEAMENTO C# -> C++:
//   sealed record       -> struct de campos publicos + operator== = default
//                          (igualdade estrutural por valor, igual ao record C#)
//   long                -> long long (keycode; o C# usa long de 64 bits, e os
//                          keycodes Godot com bits de modifier excedem 32 bits)
//   List<T>             -> std::vector<T>
//   float Deadzone=0.5f -> float deadzone = 0.5f (mesmo default)
//   string=""           -> std::string (default vazio)
//
// NOTA DE PORTE (serializacao): o C# documenta "Serializavel JSON" mas NAO
// implementa nenhum serializer (a persistencia .NET seria por reflexao/System.
// Text.Json no game-side). Nao ha codigo de serializacao a portar; portamos so
// o modelo logico, fiel ao C#. Quando a persistencia do remap virar necessaria,
// o serializer entra junto do backend platform/ (decisao de design para o lider).
//
// Cross-ref: engine/foundation/input_remap/InputBinding.cs;
//            docs/tech/engine-modules.md secao 2.4; engine-design.md secao 2/3.

#ifndef GUS_DOMAIN_INPUT_INPUT_BINDING_HPP
#define GUS_DOMAIN_INPUT_INPUT_BINDING_HPP

#include <cstddef>
#include <functional>
#include <string>
#include <vector>

namespace gus::domain::input {

// Binding de teclado. Modifier support: Ctrl/Shift/Alt opcional. A igualdade por
// valor (com os modifiers) e a PRIMITIVA em que a deteccao de conflito se apoia:
// Ctrl+A != A. Espelha o record KeyBinding do C#.
struct KeyBinding {
    long long keycode = 0;
    bool ctrl_pressed = false;
    bool shift_pressed = false;
    bool alt_pressed = false;

    [[nodiscard]] bool operator==(const KeyBinding&) const = default;
};

// Binding de gamepad button. Espelha GamepadButtonBinding do C#.
struct GamepadButtonBinding {
    int button_index = 0;

    [[nodiscard]] bool operator==(const GamepadButtonBinding&) const = default;
};

// Binding de mouse button. Espelha MouseButtonBinding do C#.
struct MouseButtonBinding {
    int button_index = 0;

    [[nodiscard]] bool operator==(const MouseButtonBinding&) const = default;
};

// Binding de gamepad axis (left stick X/Y, triggers, etc). Distingue eixo E
// direcao: mesmo eixo com valores opostos sao bindings distintos. Espelha
// GamepadAxisBinding do C#.
struct GamepadAxisBinding {
    int axis = 0;
    float axis_value = 0.0f;

    [[nodiscard]] bool operator==(const GamepadAxisBinding&) const = default;
};

// Conjunto de bindings de uma action (keyboard + gamepad + mouse + axis). Uma
// action MAY ter multiplos bindings (ex: WASD + arrow keys + left stick).
// Espelha ActionBindings do C# (incluindo o default Deadzone = 0.5f e as listas
// vazias por default, que em C++ sao sempre validas, nunca null).
struct ActionBindings {
    std::string action_name;
    std::vector<KeyBinding> keys;
    std::vector<GamepadButtonBinding> gamepad_buttons;
    std::vector<MouseButtonBinding> mouse_buttons;
    std::vector<GamepadAxisBinding> gamepad_axes;
    float deadzone = 0.5f;

    [[nodiscard]] bool operator==(const ActionBindings&) const = default;
};

// Config completo de input remap. Versionado pra migration futura (espelha
// ConfigVersion = 1 do C#). InputRemapConfig do C#.
struct InputRemapConfig {
    int config_version = 1;
    std::vector<ActionBindings> actions;

    [[nodiscard]] bool operator==(const InputRemapConfig&) const = default;
};

}  // namespace gus::domain::input

// Hash de KeyBinding sensivel a TODOS os campos (keycode + modifiers), coerente
// com operator==. Espelha o GetHashCode gerado pelo record C# (sensivel ao
// modifier), exercitado pela spec (KeyBinding_SameKeycodeNoModifiers_AreEqual).
// Permite usar KeyBinding em containers hash (ex.: deteccao de conflito O(1) no
// backend futuro), mesmo que a regra de conflito viva fora de domain/.
template <>
struct std::hash<gus::domain::input::KeyBinding> {
    [[nodiscard]] std::size_t operator()(
        const gus::domain::input::KeyBinding& k) const noexcept {
        std::size_t h = std::hash<long long>{}(k.keycode);
        // Combina os 3 bits de modifier num nibble e mistura (boost-style spread).
        const std::size_t mods = (static_cast<std::size_t>(k.ctrl_pressed) << 0) |
                                 (static_cast<std::size_t>(k.shift_pressed) << 1) |
                                 (static_cast<std::size_t>(k.alt_pressed) << 2);
        h ^= mods + 0x9e3779b9U + (h << 6) + (h >> 2);
        return h;
    }
};

#endif  // GUS_DOMAIN_INPUT_INPUT_BINDING_HPP

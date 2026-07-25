// gus/platform/src/input/glintfx_gamepad_input.cpp
//
// Implementacao do GlintfxGamepadInput. Ver header. Travado por
// platform/tests/glintfx_gamepad_input_test.cpp (TEST-FIRST) - providers FAKEs, headless.

#include "gus/platform/input/glintfx_gamepad_input.hpp"

#include <utility>  // std::move

namespace gus::platform::input {

namespace {

// Converte um eixo NORMALIZADO do glintfx ([-1,1] float) pra faixa SIMETRICA que
// axis_sign()/kStickDeadzone (gamepad_mapping.cpp) ja assumem ([-32767,32767] - ver a nota
// FAIXA DE EIXO no header). Clampa ANTES de escalar: uma entrada fora de [-1,1] (bug
// upstream, ou hipoteticamente hostil) nao pode estourar o int nem inverter o sinal por
// truncamento. 32767 (nao 32768): mantem SIMETRICA a faixa que gamepad_mapping.cpp usa - ele
// so compara <= -kStickDeadzone / >= kStickDeadzone, nunca depende da assimetria tecnica do
// Sint16 real do SDL (-32768).
int to_stick_axis(float normalized) noexcept {
    if (normalized < -1.0f) normalized = -1.0f;
    if (normalized > 1.0f) normalized = 1.0f;
    return static_cast<int>(normalized * 32767.0f);
}

}  // namespace

GlintfxGamepadInput::GlintfxGamepadInput(ConnectedProvider connected, AxisProvider axis,
                                          ButtonProvider button) noexcept
    : connected_(std::move(connected)), axis_(std::move(axis)), button_(std::move(button)) {}

GlintfxGamepadInput::GlintfxGamepadInput(const glintfx::Gamepads& pads, int pad_index) noexcept
    : connected_([&pads, pad_index] { return pads.connected(pad_index); }),
      axis_([&pads, pad_index](glintfx::GamepadAxis a) { return pads.axis(pad_index, a); }),
      button_([&pads, pad_index](glintfx::GamepadButton b) { return pads.button(pad_index, b); }) {}

GamepadState GlintfxGamepadInput::read() const noexcept {
    GamepadState state;
    state.connected = connected_();
    if (!state.connected) {
        return state;  // GamepadState{} ja zerado - fail-high, nunca consulta eixo/botao
    }
    state.stick_x = to_stick_axis(axis_(glintfx::GamepadAxis::LeftX));
    state.stick_y = to_stick_axis(axis_(glintfx::GamepadAxis::LeftY));
    state.dpad_left = button_(glintfx::GamepadButton::DpadLeft);
    state.dpad_right = button_(glintfx::GamepadButton::DpadRight);
    state.dpad_up = button_(glintfx::GamepadButton::DpadUp);
    state.dpad_down = button_(glintfx::GamepadButton::DpadDown);
    // Run no gamepad: botao leste (B/Circle) - MESMO mapeamento fisico que
    // SdlInput::pump_events ja usa (SDL_GAMEPAD_BUTTON_EAST).
    state.run_button = button_(glintfx::GamepadButton::East);
    return state;
}

}  // namespace gus::platform::input

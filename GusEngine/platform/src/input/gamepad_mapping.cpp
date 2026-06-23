// gus/platform/src/input/gamepad_mapping.cpp
//
// Implementacao do mapeamento gamepad -> cardinal. Ver header. Travado por
// platform/tests/gamepad_mapping_test.cpp (TEST-FIRST). POCO puro, sem SDL.

#include "gus/platform/input/gamepad_mapping.hpp"

namespace gus::platform::input {

namespace {

// Componente digital de um eixo analogico com deadzone: -1/0/+1.
int axis_sign(int value) noexcept {
    if (value <= -kStickDeadzone) return -1;
    if (value >= kStickDeadzone) return 1;
    return 0;
}

// Clampa um inteiro em {-1,0,1}.
int clamp_cardinal(int v) noexcept {
    if (v < 0) return -1;
    if (v > 0) return 1;
    return 0;
}

}  // namespace

int gamepad_dx(const GamepadState& g) noexcept {
    int v = 0;
    if (g.dpad_left) --v;
    if (g.dpad_right) ++v;
    v += axis_sign(g.stick_x);
    return clamp_cardinal(v);
}

int gamepad_dy(const GamepadState& g) noexcept {
    int v = 0;
    if (g.dpad_up) --v;    // cima = -Y (frente)
    if (g.dpad_down) ++v;  // baixo = +Y
    v += axis_sign(g.stick_y);
    return clamp_cardinal(v);
}

}  // namespace gus::platform::input

// GusEngine/platform/tests/gamepad_mapping_test.cpp
//
// Catch2 da LOGICA pura de mapeamento de gamepad -> intencao cardinal (platform/
// input, pos repivot ADR-008: o SDL trouxe gamepad nativo, que o Qt6 nao tinha).
// TEST-FIRST.
//
// O bombeamento de eventos do SDL (abrir o gamepad, ler SDL_Event) e irredutivel e
// fica no SdlInput (coberto pelo smoke). Aqui travamos a parte testavel sem
// hardware: o d-pad e os eixos analogicos do stick esquerdo viram (dx,dy) em
// {-1,0,1}, com DEADZONE (ruido do stick parado nao move), e empate/opostos
// cancelam - exatamente o mesmo contrato cardinal do teclado.

#include <catch2/catch_test_macros.hpp>

#include "gus/platform/input/gamepad_mapping.hpp"

using gus::platform::input::GamepadState;
using gus::platform::input::gamepad_dx;
using gus::platform::input::gamepad_dy;

namespace {
// Faixa de um eixo analogico do SDL (Sint16): [-32768, 32767].
constexpr int kAxisMax = 32767;
constexpr int kAxisMin = -32768;
}  // namespace

TEST_CASE("Gamepad parado (sticks no centro) nao move", "[gamepad]") {
    GamepadState g;  // tudo zerado
    REQUIRE(gamepad_dx(g) == 0);
    REQUIRE(gamepad_dy(g) == 0);
}

TEST_CASE("Ruido dentro da deadzone nao move", "[gamepad]") {
    GamepadState g;
    g.stick_x = 5000;   // < deadzone tipica (~8000)
    g.stick_y = -4000;
    REQUIRE(gamepad_dx(g) == 0);
    REQUIRE(gamepad_dy(g) == 0);
}

TEST_CASE("Stick para a direita move +X", "[gamepad]") {
    GamepadState g;
    g.stick_x = kAxisMax;
    REQUIRE(gamepad_dx(g) == 1);
    REQUIRE(gamepad_dy(g) == 0);
}

TEST_CASE("Stick para cima move -Y (frente)", "[gamepad]") {
    // No SDL o eixo Y do stick e +baixo; cima = valor negativo. O mundo tambem usa
    // +Y baixo, entao o sinal passa direto: stick para cima -> dy negativo.
    GamepadState g;
    g.stick_y = kAxisMin;
    REQUIRE(gamepad_dy(g) == -1);
}

TEST_CASE("D-pad tem prioridade e e digital", "[gamepad]") {
    GamepadState g;
    g.dpad_left = true;
    REQUIRE(gamepad_dx(g) == -1);
    g.dpad_right = true;  // ambos -> cancelam
    REQUIRE(gamepad_dx(g) == 0);
}

TEST_CASE("D-pad e stick combinam sem duplicar (clamp em {-1,0,1})", "[gamepad]") {
    GamepadState g;
    g.dpad_right = true;
    g.stick_x = kAxisMax;  // mesmo sentido: nao vira +2
    REQUIRE(gamepad_dx(g) == 1);
}

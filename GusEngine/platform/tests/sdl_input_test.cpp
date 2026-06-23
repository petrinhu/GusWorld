// GusEngine/platform/tests/sdl_input_test.cpp
//
// Catch2 do SdlInput (platform/input): a ponte que junta teclado (via InputMapper
// + key_translation) e gamepad (GamepadState) numa intencao cardinal unica
// (dx,dy,run) para alimentar o overworld. TEST-FIRST.
//
// O bombeamento de SDL_Event de hardware e irredutivel (coberto pelo smoke); o que
// travamos aqui sem display/joystick e a parte testavel: process_key traduz o
// SDL_Keycode e atualiza o estado; o gamepad pode ser injetado; a FUSAO clampa em
// {-1,0,1} (teclado e gamepad no mesmo sentido nao viram 2; opostos cancelam); e
// run vem do Shift no teclado OU do botao de run do gamepad.

#include <catch2/catch_test_macros.hpp>

#include "gus/platform/input/sdl_input.hpp"

using gus::platform::input::SdlInput;

namespace {
// SDL_Keycode das teclas exercitadas (ASCII coincide; setas/shift sao nomeadas).
constexpr int kKeyD = 'd';        // SDLK_d
constexpr int kKeyA = 'a';        // SDLK_a
constexpr int kKeyW = 'w';        // SDLK_w
constexpr int kSdlLShift = 0x400000E1;  // SDLK_LSHIFT
constexpr int kAxisMax = 32767;
}  // namespace

TEST_CASE("SdlInput sem nada nao move", "[sdl_input]") {
    SdlInput in;
    REQUIRE(in.dx() == 0);
    REQUIRE(in.dy() == 0);
    REQUIRE(in.run() == false);
}

TEST_CASE("SdlInput teclado D move +X", "[sdl_input]") {
    SdlInput in;
    in.process_key(kKeyD, /*pressed=*/true);
    REQUIRE(in.dx() == 1);
    in.process_key(kKeyD, /*pressed=*/false);
    REQUIRE(in.dx() == 0);
}

TEST_CASE("SdlInput W move -Y (frente)", "[sdl_input]") {
    SdlInput in;
    in.process_key(kKeyW, true);
    REQUIRE(in.dy() == -1);
}

TEST_CASE("SdlInput Shift ativa run", "[sdl_input]") {
    SdlInput in;
    in.process_key(kSdlLShift, true);
    REQUIRE(in.run() == true);
}

TEST_CASE("SdlInput teclas opostas cancelam", "[sdl_input]") {
    SdlInput in;
    in.process_key(kKeyA, true);
    in.process_key(kKeyD, true);
    REQUIRE(in.dx() == 0);
}

TEST_CASE("SdlInput funde gamepad e clampa", "[sdl_input]") {
    SdlInput in;
    // Gamepad injetado: stick para a direita.
    auto& g = in.mutable_gamepad();
    g.stick_x = kAxisMax;
    REQUIRE(in.dx() == 1);
    // Teclado no MESMO sentido nao vira 2.
    in.process_key(kKeyD, true);
    REQUIRE(in.dx() == 1);
    // Teclado no sentido OPOSTO cancela o gamepad.
    in.process_key(kKeyD, false);
    in.process_key(kKeyA, true);
    REQUIRE(in.dx() == 0);
}

TEST_CASE("SdlInput clear solta tudo", "[sdl_input]") {
    SdlInput in;
    in.process_key(kKeyW, true);
    in.process_key(kSdlLShift, true);
    in.clear();
    REQUIRE(in.dy() == 0);
    REQUIRE(in.run() == false);
}

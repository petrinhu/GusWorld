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
constexpr int kSdlEscape = 0x1B;        // SDLK_ESCAPE ('\x1B' == 27, ver SDL_keycode.h)
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

// MENU-PAUSA-CONFIG-SOM: consume_escape_pressed() e o gancho da CIDADE pro menu de
// pausa (Esc == abrir, sem pilha de modais como a batalha). EDGE + consumo: 1
// press = 1 true, e so 1 - a proxima chamada sem novo press devolve false.
TEST_CASE("SdlInput Esc arma o edge e consume_escape_pressed consome uma vez",
          "[sdl_input]") {
    SdlInput in;
    REQUIRE(in.consume_escape_pressed() == false);  // nada pressionado ainda
    in.process_key(kSdlEscape, /*pressed=*/true);
    REQUIRE(in.consume_escape_pressed() == true);   // 1o consumo: pega o press
    REQUIRE(in.consume_escape_pressed() == false);  // 2o consumo: ja foi drenado
}

TEST_CASE("SdlInput Esc release nao arma o edge", "[sdl_input]") {
    SdlInput in;
    in.process_key(kSdlEscape, /*pressed=*/false);
    REQUIRE(in.consume_escape_pressed() == false);
}

TEST_CASE("SdlInput Esc nao interfere no movimento (aditivo)", "[sdl_input]") {
    SdlInput in;
    in.process_key(kSdlEscape, /*pressed=*/true);
    REQUIRE(in.dx() == 0);
    REQUIRE(in.dy() == 0);
    REQUIRE(in.run() == false);
    REQUIRE(in.consume_escape_pressed() == true);  // o edge ainda foi armado
}

// FIX BUG (playtest ao vivo do lider, M7-DIALOGO NPC-MVP: "Gus anda sozinho apos
// fechar o dialogo com o Bertoldo") - reproduz a causa raiz EXATA: o jogador segura
// D (leste) ao esbarrar no NPC; o loop MODAL do dialogo (npc_dialogue_loop.cpp/
// npc_dialogue_loop_gl.cpp) faz o proprio SDL_PollEvent, independente de
// SdlInput::pump_events - o SDL_EVENT_KEY_UP de D, se acontecer DURANTE a
// conversa, e descartado (nunca chega em process_key). Sem o fix (SdlWindow::
// clear_input(), que so repassa pra este SdlInput::clear() ja testado acima em
// "SdlInput clear solta tudo"), D continuaria "pressionada" pra sempre - o Gus
// retomaria andando na cidade sozinho ate esbarrar em algo. Este caso E O MESMO
// mecanismo (KEY_DOWN SEM o KEY_UP correspondente chegar), so nomeado pelo bug
// real para deixar a regressao rastreavel.
TEST_CASE("SdlInput: KEY_DOWN sem KEY_UP correspondente (tecla 'presa' durante um "
          "loop modal) nao sobrevive a um clear() na saida",
          "[sdl_input][regressao-dialogo]") {
    SdlInput in;
    in.process_key(kKeyD, /*pressed=*/true);  // segura D antes de entrar no modal
    REQUIRE(in.dx() == 1);
    // ... o loop modal roda aqui: o KEY_UP fisico de D (se acontecer) e descartado,
    // NUNCA chega em process_key (e o que reproduz o bug relatado ao vivo) ...
    in.clear();  // o que SdlWindow::clear_input() faz ao ENTRAR/SAIR do modal
    REQUIRE(in.dx() == 0);  // D nao fica "presa": o Gus nao anda sozinho ao retomar
}

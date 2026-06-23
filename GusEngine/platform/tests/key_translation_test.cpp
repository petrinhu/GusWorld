// GusEngine/platform/tests/key_translation_test.cpp
//
// Catch2 da traducao SDL_Keycode -> keycode Godot (platform/input). TEST-FIRST.
// Headless: nao abre janela, so exercita a tabela de traducao.
//
// POR QUE EXISTE: o esquema de fabrica (domain/input/controls_restore.cpp) usa os
// keycodes do enum Godot Key (congelados nos saves). O backend SDL produz
// SDL_Keycode, cujos valores DIVERGEM para teclas nomeadas (setas, Shift, ...). As
// letras ASCII coincidem (SDLK_a == 'a'), mas a tabela traduz explicitamente. E a
// "1 peca" que casa o backend de evento com o mapa logico puro (ADR-008).
//
// Os literais SDL espelham <SDL3/SDL_keycode.h>; os Godot espelham
// controls_restore.cpp (fonte canonica). Se um valor mudar la, este teste pega.

#include <catch2/catch_test_macros.hpp>

#include "gus/platform/input/key_translation.hpp"

using gus::platform::input::sdl_key_to_godot_keycode;

namespace {
// Esquema Godot Key enum (espelho de controls_restore.cpp).
constexpr long long kGodotLeft = 4194319;
constexpr long long kGodotRight = 4194321;
constexpr long long kGodotUp = 4194320;
constexpr long long kGodotDown = 4194322;
constexpr long long kGodotShift = 4194325;
constexpr long long kGodotEnter = 4194309;
constexpr long long kGodotEscape = 4194305;
constexpr long long kGodotTab = 4194308;

// Valores SDL_Keycode (SDL3 SDL_keycode.h) das teclas exercitadas. Letras = ASCII
// minusculo (SDL entrega minuscula). Nomeadas usam SDLK_SCANCODE_MASK | scancode.
constexpr int kSdlW = 'w';   // SDLK_w
constexpr int kSdlA = 'a';
constexpr int kSdlS = 's';
constexpr int kSdlD = 'd';
constexpr int kSdl1 = '1';
constexpr int kSdlLeft = 0x40000050;   // SDLK_LEFT
constexpr int kSdlRight = 0x4000004F;  // SDLK_RIGHT
constexpr int kSdlUp = 0x40000052;     // SDLK_UP
constexpr int kSdlDown = 0x40000051;   // SDLK_DOWN
constexpr int kSdlLShift = 0x400000E1; // SDLK_LSHIFT
constexpr int kSdlRShift = 0x400000E5; // SDLK_RSHIFT
constexpr int kSdlReturn = 0x0D;       // SDLK_RETURN ('\r')
constexpr int kSdlKpEnter = 0x40000058; // SDLK_KP_ENTER
constexpr int kSdlEscape = 0x1B;       // SDLK_ESCAPE
constexpr int kSdlTab = 0x09;          // SDLK_TAB
}  // namespace

TEST_CASE("key_translation: letras minusculas normalizam para maiuscula",
          "[key_translation]") {
    // SDLK_w == 'w' (minusculo); o esquema de fabrica usa 'W' (maiusculo).
    REQUIRE(sdl_key_to_godot_keycode(kSdlW) == static_cast<long long>('W'));
    REQUIRE(sdl_key_to_godot_keycode(kSdlA) == static_cast<long long>('A'));
    REQUIRE(sdl_key_to_godot_keycode(kSdlS) == static_cast<long long>('S'));
    REQUIRE(sdl_key_to_godot_keycode(kSdlD) == static_cast<long long>('D'));
}

TEST_CASE("key_translation: numeros ASCII passam inalterados", "[key_translation]") {
    REQUIRE(sdl_key_to_godot_keycode(kSdl1) == static_cast<long long>('1'));
}

TEST_CASE("key_translation: setas mapeiam para o Godot exato", "[key_translation]") {
    REQUIRE(sdl_key_to_godot_keycode(kSdlLeft) == kGodotLeft);
    REQUIRE(sdl_key_to_godot_keycode(kSdlRight) == kGodotRight);
    REQUIRE(sdl_key_to_godot_keycode(kSdlUp) == kGodotUp);
    REQUIRE(sdl_key_to_godot_keycode(kSdlDown) == kGodotDown);
}

TEST_CASE("key_translation: ambos Shift viram o Shift Godot", "[key_translation]") {
    REQUIRE(sdl_key_to_godot_keycode(kSdlLShift) == kGodotShift);
    REQUIRE(sdl_key_to_godot_keycode(kSdlRShift) == kGodotShift);
}

TEST_CASE("key_translation: teclas nomeadas de UI mapeiam", "[key_translation]") {
    // Return e KP-Enter ambos viram o Enter Godot (o esquema usa um so codigo).
    REQUIRE(sdl_key_to_godot_keycode(kSdlReturn) == kGodotEnter);
    REQUIRE(sdl_key_to_godot_keycode(kSdlKpEnter) == kGodotEnter);
    REQUIRE(sdl_key_to_godot_keycode(kSdlEscape) == kGodotEscape);
    REQUIRE(sdl_key_to_godot_keycode(kSdlTab) == kGodotTab);
}

TEST_CASE("key_translation: tecla desconhecida vira zero", "[key_translation]") {
    // Um SDL_Keycode fora da tabela e nao-ASCII -> 0 (sentinela "sem binding").
    REQUIRE(sdl_key_to_godot_keycode(0x40000099) == 0LL);
    REQUIRE(sdl_key_to_godot_keycode(-1) == 0LL);
}

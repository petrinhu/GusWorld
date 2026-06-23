// gus/platform/src/input/key_translation.cpp
//
// Tabela de traducao SDL_Keycode -> keycode Godot. Ver header. Travado por
// platform/tests/key_translation_test.cpp (TEST-FIRST).
//
// Os literais Godot espelham domain/src/input/controls_restore.cpp (fonte unica
// do esquema de fabrica). As constantes SDL vem de <SDL3/SDL_keycode.h> (camada
// platform/, SDL permitido); referencia-las documenta a intencao e blinda contra
// mudanca de valor entre versoes do SDL.

#include "gus/platform/input/key_translation.hpp"

#include <SDL3/SDL_keycode.h>  // SDLK_* (camada platform/, SDL permitido)

namespace gus::platform::input {

namespace {

// Esquema Godot Key enum (mesmos valores de controls_restore.cpp).
constexpr long long kGodotEnter = 4194309;
constexpr long long kGodotEscape = 4194305;
constexpr long long kGodotTab = 4194308;
constexpr long long kGodotLeft = 4194319;
constexpr long long kGodotRight = 4194321;
constexpr long long kGodotUp = 4194320;
constexpr long long kGodotDown = 4194322;
constexpr long long kGodotShift = 4194325;
constexpr long long kGodotSpace = 32;  // ASCII, coincide

}  // namespace

long long sdl_key_to_godot_keycode(int sdl_keycode) noexcept {
    // 1) Teclas nomeadas: mapeamento explicito (os valores divergem do Godot).
    switch (static_cast<SDL_Keycode>(sdl_keycode)) {
        case SDLK_LEFT:
            return kGodotLeft;
        case SDLK_RIGHT:
            return kGodotRight;
        case SDLK_UP:
            return kGodotUp;
        case SDLK_DOWN:
            return kGodotDown;
        // Os dois Shift (esquerdo e direito) viram o mesmo Shift Godot (o esquema
        // de fabrica usa um codigo so para move_run).
        case SDLK_LSHIFT:
        case SDLK_RSHIFT:
            return kGodotShift;
        // Return (teclado principal) e Enter (numerico) viram o mesmo Enter Godot.
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
            return kGodotEnter;
        case SDLK_ESCAPE:
            return kGodotEscape;
        case SDLK_TAB:
            return kGodotTab;
        case SDLK_SPACE:
            return kGodotSpace;
        default:
            break;
    }

    // 2) ASCII imprimivel (0x20..0x7E): passa direto (SDL == ASCII == Godot).
    //    SDL entrega minuscula para letras (SDLK_a == 'a'); o esquema de fabrica
    //    usa o codigo MAIUSCULO ('W','A',...), entao normalizamos a..z -> A..Z.
    if (sdl_keycode >= 0x20 && sdl_keycode <= 0x7E) {
        if (sdl_keycode >= 'a' && sdl_keycode <= 'z') {
            return static_cast<long long>(sdl_keycode - ('a' - 'A'));
        }
        return static_cast<long long>(sdl_keycode);
    }

    // 3) Desconhecida (nao-ASCII fora da tabela): sentinela "sem binding".
    return 0;
}

}  // namespace gus::platform::input

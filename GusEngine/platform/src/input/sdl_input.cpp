// gus/platform/src/input/sdl_input.cpp
//
// Implementacao do SdlInput. Ver header. A FUSAO teclado+gamepad e a traducao de
// tecla sao travadas por sdl_input_test.cpp (TEST-FIRST); pump_events/open_gamepads
// (que tocam SDL) sao o caminho de hardware, coberto pelo smoke do app.

#include "gus/platform/input/sdl_input.hpp"

#include <SDL3/SDL.h>

#include <utility>  // std::move

#include "gus/domain/input/controls_restore.hpp"
#include "gus/platform/input/key_translation.hpp"

namespace gus::platform::input {

namespace {

int clamp_cardinal(int v) noexcept {
    if (v < 0) return -1;
    if (v > 0) return 1;
    return 0;
}

SDL_Gamepad* as_pad(void* p) noexcept { return static_cast<SDL_Gamepad*>(p); }

}  // namespace

SdlInput::SdlInput() : SdlInput(gus::domain::input::default_controls()) {}

SdlInput::SdlInput(gus::domain::input::InputRemapConfig config)
    : mapper_(std::move(config)) {}

SdlInput::~SdlInput() {
    if (gamepad_ != nullptr) {
        SDL_CloseGamepad(as_pad(gamepad_));
        gamepad_ = nullptr;
    }
}

void SdlInput::process_key(int sdl_keycode, bool pressed) {
    // MENU-PAUSA-CONFIG-SOM: arma o EDGE do Esc ANTES da traducao pro InputMapper
    // (Esc nao e mapeado a movimento nenhum - default_controls() nao o usa - entao
    // isto e ADITIVO, nunca interfere no dx/dy/run). So no PRESS (release nao
    // dispara o menu).
    if (pressed && sdl_keycode == SDLK_ESCAPE) {
        escape_pressed_ = true;
    }
    const long long code = sdl_key_to_godot_keycode(sdl_keycode);
    if (pressed) {
        mapper_.press(code);
    } else {
        mapper_.release(code);
    }
}

void SdlInput::set_controls(gus::domain::input::InputRemapConfig config) {
    // Reconstroi o InputMapper com o esquema novo - pressed_ comeca vazio (nenhuma
    // tecla "presa" sobrevive a troca), independente do que estava fisicamente
    // pressionado no esquema anterior. gamepad_/pad_ nao mudam (o InputMapper so
    // cobre o teclado).
    mapper_ = InputMapper(std::move(config));
}

bool SdlInput::consume_escape_pressed() noexcept {
    const bool was_pressed = escape_pressed_;
    escape_pressed_ = false;
    return was_pressed;
}

void SdlInput::clear() noexcept {
    mapper_.clear();
    pad_ = GamepadState{};
    // Mantem connected: re-sondado no proximo pump. clear so solta o movimento.
    pad_.connected = (gamepad_ != nullptr);
    // RUN-CAPSLOCK: zera aqui (nao "gruda" correndo ao perder foco); o pump_events
    // real re-le o Caps Lock do SO logo no proximo frame (ver corpo de pump_events
    // abaixo), entao o estado real volta a valer sem exigir soltar/apertar a tecla.
    caps_lock_active_ = false;
}

void SdlInput::open_gamepads() {
    if (gamepad_ != nullptr) {
        return;  // ja temos um aberto (M1: 1 jogador, 1 gamepad)
    }
    int count = 0;
    SDL_JoystickID* ids = SDL_GetGamepads(&count);
    if (ids != nullptr) {
        if (count > 0) {
            gamepad_ = SDL_OpenGamepad(ids[0]);
            pad_.connected = (gamepad_ != nullptr);
        }
        SDL_free(ids);
    }
}

bool SdlInput::handle_event(const SDL_Event& e) {
    switch (e.type) {
        case SDL_EVENT_QUIT:
            return false;  // fechar janela / Ctrl-C: encerra o loop

        case SDL_EVENT_KEY_DOWN:
            if (!e.key.repeat) {  // auto-repeat e idempotente; ignora
                process_key(static_cast<int>(e.key.key), /*pressed=*/true);
            }
            break;
        case SDL_EVENT_KEY_UP:
            process_key(static_cast<int>(e.key.key), /*pressed=*/false);
            break;

        // Perder foco: solta tudo pra nao "grudar" movimento.
        case SDL_EVENT_WINDOW_FOCUS_LOST:
            clear();
            break;

        // Hot-plug: abre o gamepad recem-conectado; fecha o removido.
        case SDL_EVENT_GAMEPAD_ADDED:
            open_gamepads();
            break;
        case SDL_EVENT_GAMEPAD_REMOVED:
            if (gamepad_ != nullptr) {
                SDL_CloseGamepad(as_pad(gamepad_));
                gamepad_ = nullptr;
            }
            pad_ = GamepadState{};
            break;

        default:
            break;
    }
    return true;
}

bool SdlInput::pump_events() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (!handle_event(e)) {
            return false;  // MESMO comportamento de sempre: retorna NA HORA no
                            // QUIT, sem ler Caps Lock/gamepad deste frame (os
                            // eventos restantes da fila ficam pro proximo pump).
        }
    }

    // RUN-CAPSLOCK: le o estado ATUAL do modificador Caps Lock do SO (nao um
    // evento de tecla - SDL_GetModState() e uma leitura de ESTADO, chamada a CADA
    // pump_events(), igual ao stick do gamepad logo abaixo). "Enquanto Caps Lock ON,
    // corre" e literalmente isto: nenhum press/release a rastrear, so o bit do
    // modificador a cada frame. Em headless/driver dummy (--smoke, SDL_VIDEODRIVER=
    // dummy) SDL_GetModState() devolve SDL_KMOD_NONE (sem teclado fisico nenhum
    // modificador fica "preso" ligado) - degrada seguro pra "nao corre".
    caps_lock_active_ = (SDL_GetModState() & SDL_KMOD_CAPS) != 0;

    // Le o estado continuo do gamepad aberto (stick + d-pad + botao de run).
    if (gamepad_ != nullptr) {
        SDL_Gamepad* g = as_pad(gamepad_);
        pad_.connected = true;
        pad_.stick_x = SDL_GetGamepadAxis(g, SDL_GAMEPAD_AXIS_LEFTX);
        pad_.stick_y = SDL_GetGamepadAxis(g, SDL_GAMEPAD_AXIS_LEFTY);
        pad_.dpad_left = SDL_GetGamepadButton(g, SDL_GAMEPAD_BUTTON_DPAD_LEFT);
        pad_.dpad_right = SDL_GetGamepadButton(g, SDL_GAMEPAD_BUTTON_DPAD_RIGHT);
        pad_.dpad_up = SDL_GetGamepadButton(g, SDL_GAMEPAD_BUTTON_DPAD_UP);
        pad_.dpad_down = SDL_GetGamepadButton(g, SDL_GAMEPAD_BUTTON_DPAD_DOWN);
        // Run no gamepad: botao leste (B/Circle), segurar para correr.
        pad_.run_button = SDL_GetGamepadButton(g, SDL_GAMEPAD_BUTTON_EAST);
    }
    return true;
}

int SdlInput::dx() const noexcept {
    return clamp_cardinal(mapper_.movement_dx() + gamepad_dx(pad_));
}

int SdlInput::dy() const noexcept {
    return clamp_cardinal(mapper_.movement_dy() + gamepad_dy(pad_));
}

bool SdlInput::run() const noexcept {
    // RUN-CAPSLOCK: Caps Lock ON (should_run_caps_lock()) e mais uma fonte de
    // "correr", aditiva as ja existentes (Shift segurado, botao do gamepad) -
    // nenhuma substitui a outra.
    return mapper_.run_active() || pad_.run_button || should_run_caps_lock();
}

}  // namespace gus::platform::input

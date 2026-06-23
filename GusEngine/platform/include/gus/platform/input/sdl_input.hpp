// gus/platform/input/sdl_input.hpp
//
// SdlInput (platform/input, pos repivot ADR-008): a ponte entre o backend de
// evento SDL e a intencao cardinal (dx,dy,run) que o overworld consome. Junta:
//   - TECLADO: SDL_Keycode -> keycode Godot (key_translation) -> InputMapper (o
//     mapa logico puro de domain/, que decide qual tecla e qual acao);
//   - GAMEPAD: GamepadState (stick + d-pad + botao de run) -> gamepad_mapping.
// As duas fontes sao FUNDIDAS e clampadas em {-1,0,1}: mesmo sentido nao vira 2,
// opostos cancelam. run = Shift (teclado) OU botao de run (gamepad).
//
// HEADER LIMPO (sem <SDL...>): inclui so InputMapper + gamepad_mapping (POCO). O
// processamento de SDL_Event de hardware (pump_events, open/close de gamepad) vive
// no .cpp - a unica parte que toca SDL. Isso mantem a FUSAO e a traducao de tecla
// testaveis headless (sdl_input_test.cpp): process_key e mutable_gamepad sao a
// porta de injecao do teste; pump_events e o caminho real (coberto pelo smoke).

#ifndef GUS_PLATFORM_INPUT_SDL_INPUT_HPP
#define GUS_PLATFORM_INPUT_SDL_INPUT_HPP

#include "gus/platform/input/gamepad_mapping.hpp"
#include "gus/platform/input/input_mapper.hpp"

namespace gus::platform::input {

class SdlInput {
public:
    // Constroi com o esquema de fabrica (default_controls()). NAO abre gamepad
    // aqui (precisa do subsistema SDL inicializado): chame open_gamepads() depois.
    SdlInput();
    ~SdlInput();

    SdlInput(const SdlInput&) = delete;
    SdlInput& operator=(const SdlInput&) = delete;

    // --- caminho REAL (toca SDL; coberto pelo smoke) -----------------------
    // Drena a fila de eventos do SDL: teclado -> process_key, gamepad
    // (conexao/desconexao/botoes) -> GamepadState. Devolve false se o SDL pediu
    // para sair (janela fechada / SDL_EVENT_QUIT) - o loop encerra. true continua.
    [[nodiscard]] bool pump_events();
    // Abre os gamepads ja conectados e arma o hot-plug (chamar apos SDL_Init).
    void open_gamepads();

    // --- caminho TESTAVEL (sem SDL) ----------------------------------------
    // Aplica uma transicao de tecla (SDL_Keycode cru -> traducao -> InputMapper).
    // pressed=true e press; false e release. Auto-repeat e idempotente (conjunto).
    void process_key(int sdl_keycode, bool pressed);
    // Acesso ao estado do gamepad (o teste injeta; o pump_events preenche).
    [[nodiscard]] GamepadState& mutable_gamepad() noexcept { return pad_; }
    // Solta tudo (teclado + gamepad), ex.: ao perder foco da janela.
    void clear() noexcept;

    // --- intencao FUNDIDA (teclado + gamepad), clampada em {-1,0,1} ---------
    [[nodiscard]] int dx() const noexcept;
    [[nodiscard]] int dy() const noexcept;
    [[nodiscard]] bool run() const noexcept;

private:
    InputMapper mapper_;  // teclado -> acao (mapa logico puro)
    GamepadState pad_;    // estado cru do gamepad
    void* gamepad_ = nullptr;  // SDL_Gamepad* opaco (so o .cpp conhece o tipo)
};

}  // namespace gus::platform::input

#endif  // GUS_PLATFORM_INPUT_SDL_INPUT_HPP

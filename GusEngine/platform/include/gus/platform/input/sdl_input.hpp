// gus/platform/input/sdl_input.hpp
//
// SdlInput (platform/input, pos repivot ADR-008): a ponte entre o backend de
// evento SDL e a intencao cardinal (dx,dy,run) que o overworld consome. Junta:
//   - TECLADO: SDL_Keycode -> keycode Godot (key_translation) -> InputMapper (o
//     mapa logico puro de domain/, que decide qual tecla e qual acao);
//   - GAMEPAD: GamepadState (stick + d-pad + botao de run) -> gamepad_mapping;
//   - CAPS LOCK (RUN-CAPSLOCK, pedido do lider): estado do modificador Caps Lock
//     do SO, lido via SDL_GetModState() a CADA pump_events() - NAO e um evento de
//     tecla (KEY_DOWN/UP), e um ESTADO PERSISTENTE: enquanto ON, corre; ao
//     desligar, volta a andar, sem precisar segurar nada. Ver should_run_caps_
//     lock()/set_caps_lock_active() abaixo.
// As fontes sao FUNDIDAS e clampadas em {-1,0,1}: mesmo sentido nao vira 2,
// opostos cancelam. run = Shift (teclado, segurar) OU botao de run (gamepad) OU
// Caps Lock ON (teclado, toggle por estado) - qualquer uma basta.
//
// HEADER LIMPO (sem <SDL...>): inclui so InputMapper + gamepad_mapping (POCO). O
// processamento de SDL_Event de hardware (pump_events, open/close de gamepad) vive
// no .cpp - a unica parte que toca SDL. Isso mantem a FUSAO e a traducao de tecla
// testaveis headless (sdl_input_test.cpp): process_key, mutable_gamepad e
// set_caps_lock_active sao a porta de injecao do teste; pump_events e o caminho
// real (coberto pelo smoke; SDL_GetModState degrada pra KMOD_NONE sem teclado
// fisico/driver dummy, ou seja Caps Lock OFF - seguro em headless).

#ifndef GUS_PLATFORM_INPUT_SDL_INPUT_HPP
#define GUS_PLATFORM_INPUT_SDL_INPUT_HPP

#include "gus/platform/input/gamepad_mapping.hpp"
#include "gus/platform/input/input_mapper.hpp"

namespace gus::platform::input {

class SdlInput {
public:
    // Constroi com o esquema de fabrica (default_controls()). NAO abre gamepad
    // aqui (precisa do subsistema SDL inicializado): chame open_gamepads() depois.
    // Delega pro ctor explicito abaixo (mesmo esquema, so a fonte muda) - NENHUM
    // call-site existente (produção OU testes) precisa mudar.
    SdlInput();

    // M2 (ligando a tela Controles ao input REAL): constroi com um esquema de
    // controles EXPLICITO (ex.: carregado do disco via gus::platform::fs::
    // load_controls, ou default_controls() se preferir ser explicito). Permite ao
    // MAESTRO alimentar o remap persistido do jogador no BOOT, em vez de sempre
    // cair no hardcoded default_controls() - sem duplicar a logica de fusao
    // teclado+gamepad/traducao daqui.
    explicit SdlInput(gus::domain::input::InputRemapConfig config);

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
    // MENU-PAUSA-CONFIG-SOM: alem da traducao pro InputMapper (movimento), tambem
    // arma o EDGE do Esc (ver consume_escape_pressed) quando sdl_keycode ==
    // SDLK_ESCAPE && pressed - a MESMA porta de injecao dos testes, sem duplicar
    // logica entre pump_events() (caminho real) e o teste.
    void process_key(int sdl_keycode, bool pressed);
    // Acesso ao estado do gamepad (o teste injeta; o pump_events preenche).
    [[nodiscard]] GamepadState& mutable_gamepad() noexcept { return pad_; }
    // RUN-CAPSLOCK: injeta o estado do Caps Lock (o teste chama direto, sem SDL; o
    // pump_events() real chama isto com SDL_GetModState() & SDL_KMOD_CAPS a cada
    // frame - MESMO padrao de porta de injecao do mutable_gamepad() acima, so que
    // ESTADO puro em vez de referencia mutavel, porque nao ha "botoes" pra o teste
    // mexer individualmente, so um bool.
    void set_caps_lock_active(bool active) noexcept { caps_lock_active_ = active; }
    // Solta tudo (teclado + gamepad + caps lock), ex.: ao perder foco da janela. O
    // Caps Lock volta a refletir o estado real do SO no PROXIMO pump_events() (nao
    // fica "grudado" nem "vazio" por mais de um frame).
    void clear() noexcept;

    // MENU-PAUSA-CONFIG-SOM: EDGE (press unico, nao auto-repeat - pump_events ja
    // filtra e.key.repeat antes de chamar process_key) do Esc desde a ULTIMA
    // chamada a este metodo. Devolve true UMA vez por press (consome o flag: a
    // 2a chamada seguida, sem novo press, devolve false) - o padrao "consume"
    // evita que o chamador (SdlWindow/Maestro) precise gerenciar o proprio
    // estado de "ja processei este press". A CIDADE (sem pilha de modais como a
    // batalha) usa isto como o gancho unico do MENU DE PAUSA: Esc == abrir.
    [[nodiscard]] bool consume_escape_pressed() noexcept;

    // M2 (ligando a tela Controles ao input REAL): troca o esquema de controles em
    // RUNTIME (reconstroi o InputMapper interno com `config`) - usado pelo MAESTRO
    // ao voltar do menu de pausa (Esc -> Controles -> remapear -> Voltar): sem
    // isto, o remap so valeria depois de reiniciar o jogo. Solta toda tecla
    // pressionada de propósito (o InputMapper novo comeca com pressed_ vazio) -
    // evita "tecla presa" caso o keycode que estava fisicamente pressionado nao
    // tenha mais o mesmo papel no esquema novo. NAO mexe no gamepad (pad_/
    // gamepad_ sao independentes do InputMapper).
    void set_controls(gus::domain::input::InputRemapConfig config);

    // --- intencao FUNDIDA (teclado + gamepad), clampada em {-1,0,1} ---------
    [[nodiscard]] int dx() const noexcept;
    [[nodiscard]] int dy() const noexcept;
    // true se DEVE correr agora: Shift segurado (mapper_.run_active()) OU botao de
    // run do gamepad (pad_.run_button) OU Caps Lock ligado (should_run_caps_lock()).
    // Qualquer fonte basta (OR aditivo - nenhuma desliga a outra).
    [[nodiscard]] bool run() const noexcept;

    // RUN-CAPSLOCK (GANCHO PRO GAMEPAD, TODO lider decide o binding depois): a
    // UNICA funcao que decide "correr" por ESTADO PERSISTENTE (nao por segurar uma
    // tecla) - hoje so olha o Caps Lock. Quando o gamepad ganhar um binding de
    // "correr por toggle" (alem do run_button de segurar que ja existe), plugar
    // AQUI (ex.: `caps_lock_active_ || pad_toggle_run_`), sem mexer em run() nem no
    // resto do fluxo (run() so chama should_run_caps_lock(), nao conhece a fonte).
    [[nodiscard]] bool should_run_caps_lock() const noexcept { return caps_lock_active_; }

private:
    InputMapper mapper_;  // teclado -> acao (mapa logico puro)
    GamepadState pad_;    // estado cru do gamepad
    void* gamepad_ = nullptr;  // SDL_Gamepad* opaco (so o .cpp conhece o tipo)
    // MENU-PAUSA-CONFIG-SOM: flag EDGE do Esc, setada em process_key (press) e
    // consumida (lida + zerada) por consume_escape_pressed().
    bool escape_pressed_ = false;
    // RUN-CAPSLOCK: estado ATUAL (nao edge) do Caps Lock, refeito a cada
    // pump_events() real (SDL_GetModState) ou injetado por set_caps_lock_active
    // (teste). Default false = anda (headless/antes do 1o pump).
    bool caps_lock_active_ = false;
};

}  // namespace gus::platform::input

#endif  // GUS_PLATFORM_INPUT_SDL_INPUT_HPP

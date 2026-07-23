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

#include "gus/domain/input/controls_restore.hpp"
#include "gus/domain/input/input_binding.hpp"
#include "gus/platform/input/sdl_input.hpp"

using gus::domain::input::default_controls;
using gus::domain::input::InputRemapConfig;
using gus::domain::input::KeyBinding;
using gus::platform::input::SdlInput;

namespace {
// SDL_Keycode das teclas exercitadas (ASCII coincide; setas/shift sao nomeadas).
constexpr int kKeyD = 'd';        // SDLK_d
constexpr int kKeyA = 'a';        // SDLK_a
constexpr int kKeyW = 'w';        // SDLK_w
constexpr int kKeyJ = 'j';        // SDLK_j (usado no remap de move_forward abaixo)
constexpr int kSdlLShift = 0x400000E1;  // SDLK_LSHIFT
constexpr int kSdlEscape = 0x1B;        // SDLK_ESCAPE ('\x1B' == 27, ver SDL_keycode.h)
constexpr int kAxisMax = 32767;

// Devolve default_controls() com move_forward REMAPEADO de W (default de
// fabrica) pra J - unico binding, sem manter W (prova que o remap SUBSTITUI, nao
// so ADICIONA). Espelha o que a tela Controles faz (system_menu_controls_capture_
// key) e o que persist_controls grava em controls.json - aqui construido direto,
// sem depender de I/O/app/ (teste headless, so platform+domain).
InputRemapConfig config_with_move_forward_remapped_to_j() {
    InputRemapConfig cfg = default_controls();
    for (auto& action : cfg.actions) {
        if (action.action_name == "move_forward") {
            action.keys = {KeyBinding{.keycode = 74}};  // 'J' maiusculo (esquema Godot)
        }
    }
    return cfg;
}
}  // namespace

// M2 (GAP FINAL: liga a tela Controles ao input REAL) - prova o WIRING que
// Maestro::init()/open_pause_from_city() dependem: um SdlInput construido com um
// InputRemapConfig DIFERENTE do default traduz a tecla certa pra acao certa (e
// NAO mais a tecla de fabrica). Sem isto (o bug fechado aqui), SdlInput SEMPRE
// usava gus::domain::input::default_controls() hardcoded, ignorando qualquer
// remap persistido/carregado.
TEST_CASE("SdlInput(config): config remapeado muda tecla->acao (J move_forward, "
          "nao mais W)",
          "[sdl_input][m2-controles]") {
    SdlInput in(config_with_move_forward_remapped_to_j());

    // A tecla NOVA (J) agora aciona move_forward (dy=-1).
    in.process_key(kKeyJ, /*pressed=*/true);
    REQUIRE(in.dy() == -1);
    in.process_key(kKeyJ, /*pressed=*/false);
    REQUIRE(in.dy() == 0);

    // A tecla de FABRICA (W) NAO aciona mais move_forward - prova que o remap
    // SUBSTITUIU o binding, nao ficou aditivo/fantasma.
    in.process_key(kKeyW, /*pressed=*/true);
    REQUIRE(in.dy() == 0);
}

// Construtor default (SdlInput()) continua EQUIVALENTE a construir explicitamente
// com default_controls() - nao-regressao dos call-sites existentes (produção via
// SdlWindow's member default-initializer, e todo o resto desta suite acima/abaixo
// que usa `SdlInput in;` sem argumento).
TEST_CASE("SdlInput(): construtor default equivale a SdlInput(default_controls())",
          "[sdl_input][m2-controles]") {
    SdlInput default_ctor;
    SdlInput explicit_ctor(default_controls());

    default_ctor.process_key(kKeyW, /*pressed=*/true);
    explicit_ctor.process_key(kKeyW, /*pressed=*/true);
    REQUIRE(default_ctor.dy() == explicit_ctor.dy());

    default_ctor.process_key(kKeyD, /*pressed=*/true);
    explicit_ctor.process_key(kKeyD, /*pressed=*/true);
    REQUIRE(default_ctor.dx() == explicit_ctor.dx());

    default_ctor.process_key(kSdlLShift, /*pressed=*/true);
    explicit_ctor.process_key(kSdlLShift, /*pressed=*/true);
    REQUIRE(default_ctor.run() == explicit_ctor.run());
}

// set_controls() (troca em RUNTIME, sem reconstruir o SdlInput) - o gancho que
// Maestro::open_pause_from_city() usa ao FECHAR o menu de pausa (rele o
// controls.json e realimenta o SdlInput vivo da cidade, aplicando o remap SEM
// exigir restart).
TEST_CASE("SdlInput::set_controls troca o mapa em runtime (mesma instancia)",
          "[sdl_input][m2-controles]") {
    SdlInput in;  // nasce com default_controls() (W move_forward)
    in.process_key(kKeyW, /*pressed=*/true);
    REQUIRE(in.dy() == -1);

    in.set_controls(config_with_move_forward_remapped_to_j());

    // W (ainda fisicamente "pressionado" antes da troca) NAO fica "preso": o
    // InputMapper novo comeca com pressed_ vazio.
    REQUIRE(in.dy() == 0);

    // A tecla nova (J) agora move; W sozinho nao move mais.
    in.process_key(kKeyJ, /*pressed=*/true);
    REQUIRE(in.dy() == -1);
    in.process_key(kKeyJ, /*pressed=*/false);
    in.process_key(kKeyW, /*pressed=*/true);
    REQUIRE(in.dy() == 0);
}

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

// RUN-CAPSLOCK: Caps Lock e ESTADO (nao evento) - set_caps_lock_active() e a porta
// de injecao headless (o pump_events() real chama o mesmo setter com
// SDL_GetModState() & SDL_KMOD_CAPS a cada frame, ver sdl_input.cpp). "Enquanto
// ON, corre" = nao precisa de nenhum process_key.
TEST_CASE("SdlInput Caps Lock ON ativa run (estado, sem tecla segurada)",
          "[sdl_input][run-capslock]") {
    SdlInput in;
    REQUIRE(in.run() == false);
    REQUIRE(in.should_run_caps_lock() == false);

    in.set_caps_lock_active(true);
    REQUIRE(in.run() == true);
    REQUIRE(in.should_run_caps_lock() == true);

    // Desligar o Caps Lock volta a andar - sem soltar nenhuma tecla.
    in.set_caps_lock_active(false);
    REQUIRE(in.run() == false);
}

// Aditivo: Caps Lock ON + Shift OFF ainda corre; Shift ON + Caps Lock OFF idem -
// nenhuma fonte desliga a outra (OR puro), igual ao gamepad ja testado acima.
TEST_CASE("SdlInput Caps Lock e Shift sao fontes independentes de run (OR)",
          "[sdl_input][run-capslock]") {
    SdlInput in;
    in.set_caps_lock_active(true);
    in.process_key(kSdlLShift, /*pressed=*/false);
    REQUIRE(in.run() == true);  // so o Caps Lock ja basta

    in.set_caps_lock_active(false);
    in.process_key(kSdlLShift, /*pressed=*/true);
    REQUIRE(in.run() == true);  // so o Shift ja basta
}

// clear() (perder foco) zera o Caps Lock injetado - o pump_events() REAL re-le o
// estado do SO no proximo frame (nao fica "grudado" nem "vazio" por mais de 1
// frame); aqui, sem novo pump, fica false ate o proximo set/pump.
TEST_CASE("SdlInput clear() zera o Caps Lock injetado", "[sdl_input][run-capslock]") {
    SdlInput in;
    in.set_caps_lock_active(true);
    REQUIRE(in.run() == true);
    in.clear();
    REQUIRE(in.run() == false);
    REQUIRE(in.should_run_caps_lock() == false);
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

// F4-1a (onda F4, fatia 1 - loops modais -> maquina de estados com 1 unico pump de
// eventos): handle_event() e a peca NOVA que gus::app::SdlWindow::sync_input_event
// repassa pro SdlInput da cidade quando um evento e pumpado por um CHAMADOR
// EXTERNO (ex.: NpcDialogueScreen, dentro do modal) - MESMO switch que
// pump_events() ja fazia por dentro do proprio poll, so exposto pra ser chamado
// por fora. Prova de EQUIVALENCIA: alimentar handle_event() com os MESMOS 2
// eventos (KEY_DOWN d, KEY_UP d) que um SDL_PollEvent real geraria produz o MESMO
// resultado que process_key() direto (o caminho ja coberto acima) - a extracao de
// pump_events() NAO mudou comportamento nenhum.
TEST_CASE("SdlInput::handle_event reproduz process_key para KEY_DOWN/KEY_UP",
          "[sdl_input][f4-1a]") {
    SdlInput in;

    SDL_Event down{};
    down.type = SDL_EVENT_KEY_DOWN;
    down.key.key = static_cast<SDL_Keycode>(kKeyD);
    down.key.repeat = false;
    REQUIRE(in.handle_event(down) == true);  // true = "segue rodando" (nao e QUIT)
    REQUIRE(in.dx() == 1);

    SDL_Event up{};
    up.type = SDL_EVENT_KEY_UP;
    up.key.key = static_cast<SDL_Keycode>(kKeyD);
    REQUIRE(in.handle_event(up) == true);
    REQUIRE(in.dx() == 0);
}

// Auto-repeat (e.key.repeat=true) e ignorado por handle_event() EXATAMENTE como
// ja era por pump_events() (idempotente) - nao-regressao da extracao.
TEST_CASE("SdlInput::handle_event ignora KEY_DOWN com repeat=true (auto-repeat)",
          "[sdl_input][f4-1a]") {
    SdlInput in;
    SDL_Event down{};
    down.type = SDL_EVENT_KEY_DOWN;
    down.key.key = static_cast<SDL_Keycode>(kKeyD);
    down.key.repeat = true;
    REQUIRE(in.handle_event(down) == true);
    REQUIRE(in.dx() == 0);  // repeat nunca chega em process_key
}

// SDL_EVENT_QUIT devolve false (MESMO contrato de pump_events() no QUIT).
TEST_CASE("SdlInput::handle_event devolve false em SDL_EVENT_QUIT",
          "[sdl_input][f4-1a]") {
    SdlInput in;
    SDL_Event quit{};
    quit.type = SDL_EVENT_QUIT;
    REQUIRE(in.handle_event(quit) == false);
}

// PROVA CENTRAL da F4-1a: o mecanismo NOVO (pump unico - handle_event chamado por
// um CHAMADOR EXTERNO enquanto um modal "tem o foco", ver gus/app/sdl_window.hpp::
// sync_input_event) fecha o MESMO bug do teste "regressao-dialogo" acima SEM
// depender de um clear() defensivo na SAIDA do modal: a tecla segurada ANTES do
// modal e solta DURANTE o modal (o handle_event acontece ao vivo, no MESMO
// instante da solta - nao um flush retroativo ao sair).
TEST_CASE("SdlInput::handle_event ao vivo DURANTE o modal solta a tecla sem "
          "precisar de clear() na saida (F4-1a, prova da onda dos loops modais)",
          "[sdl_input][f4-1a][regressao-dialogo]") {
    SdlInput in;
    in.process_key(kKeyD, /*pressed=*/true);  // segura D antes de entrar no modal
    REQUIRE(in.dx() == 1);

    // ... o modal abre aqui (ScreenState::enter()) - SEM clear_input() de entrada
    // nesta prova (isolando o efeito SO do sync ao vivo, nao do freeze de
    // entrada que Maestro::to_npc_dialogue() ainda aplica por decisao de
    // produto separada - ver o comentario de sync_input_event) ...

    // o KEY_UP de D acontece DURANTE o modal - o pump UNICO da F4-1a entrega
    // este MESMO evento tanto pro ScreenState (via handle_event da tela) quanto,
    // por este EventSyncHook, pro handle_event do SdlInput da cidade - no MESMO
    // frame em que acontece.
    SDL_Event up{};
    up.type = SDL_EVENT_KEY_UP;
    up.key.key = static_cast<SDL_Keycode>(kKeyD);
    REQUIRE(in.handle_event(up) == true);

    // SEM clear() nenhum aqui (nem de entrada nem de saida) - o estado JA esta
    // correto porque o evento foi visto ao vivo, nao porque foi varrido depois.
    REQUIRE(in.dx() == 0);
}

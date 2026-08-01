// SPDX-License-Identifier: Apache-2.0
// GusEngine/app/tests/system_screen_step_test.cpp
//
// Catch2 (headless, SEM SDL_Init/janela/GL/glintfx::UiLayer real) de
// gus::app::screens::system_screen_step + gus::app::screens::pause_flow_next
// (F4-1b.4, onda F4 "casca SDL -> App mode do glintfx", fatia 1b.4 - SEGUNDA
// tela PAI da onda, apos F4-1b.3/title_menu_loop.cpp - reusa o padrao do
// MINI-DRIVER, ver o comentario grande no topo de system_menu_loop.cpp). Ambas
// as funcoes sao PURAS o suficiente pra serem exercitadas com SDL_Event/
// glintfx::ElementBox construidos a mao (MESMA tecnica de
// title_screen_step_test.cpp) - nunca consultam a UiLayer, as boxes/o
// desfecho de save-load entram como parametro. O CONSUMIDOR real
// (SystemMenuLoopScreen, GL-heavy, e o MINI-DRIVER dentro de
// run_system_menu_loop_gl_current) continua sem teste direto aqui - ver
// system_menu_loop_interaction_test.cpp (harness GL/SDL_PushEvent).

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <SDL3/SDL.h>
#include <glintfx/element_box.hpp>

#include "gus/app/screens/save_load_menu_loop.hpp"
#include "gus/app/screens/system_menu.hpp"
#include "gus/app/screens/system_menu_loop.hpp"

using gus::app::screens::AudioItem;
using gus::app::screens::PauseItem;
using gus::app::screens::SaveLoadLoopExit;
using gus::app::screens::SystemMenuFlowStep;
using gus::app::screens::SystemMenuScreen;
using gus::app::screens::SystemMenuScreenExit;
using gus::app::screens::SystemMenuSfxKind;
using gus::app::screens::SystemMenuState;
using gus::app::screens::SystemMenuStepBoxes;
using gus::app::screens::SystemMenuStepResult;
using gus::app::screens::pause_flow_next;
using gus::app::screens::system_menu_open;
using gus::app::screens::system_screen_step;

namespace {

glintfx::ElementBox make_box(float x, float y, float w, float h) {
    glintfx::ElementBox box;
    box.found = true;
    box.x = x;
    box.y = y;
    box.w = w;
    box.h = h;
    return box;
}

SDL_Event key_down_event(SDL_Keycode key, bool repeat = false) {
    SDL_Event ev{};
    ev.type = SDL_EVENT_KEY_DOWN;
    ev.key.key = key;
    ev.key.repeat = repeat ? 1 : 0;
    return ev;
}

SDL_Event mouse_button_down_event(float x, float y) {
    SDL_Event ev{};
    ev.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
    ev.button.button = SDL_BUTTON_LEFT;
    ev.button.x = x;
    ev.button.y = y;
    return ev;
}

SDL_Event mouse_button_up_event(float x, float y) {
    SDL_Event ev{};
    ev.type = SDL_EVENT_MOUSE_BUTTON_UP;
    ev.button.button = SDL_BUTTON_LEFT;
    ev.button.x = x;
    ev.button.y = y;
    return ev;
}

}  // namespace

// ---------------------------------------------------------------- QUIT

TEST_CASE("system_screen_step: SDL_EVENT_QUIT vira window_closed=true, sem "
          "tocar estado/sfx/reload/exit",
          "[system_screen_step][f4-1b4]") {
    SystemMenuState state;
    system_menu_open(state);
    const SystemMenuState before = state;

    SDL_Event ev{};
    ev.type = SDL_EVENT_QUIT;
    const SystemMenuStepBoxes boxes;
    const SystemMenuStepResult result = system_screen_step(state, ev, boxes);

    REQUIRE(result.window_closed);
    REQUIRE_FALSE(result.resize);
    REQUIRE_FALSE(result.reload);
    REQUIRE_FALSE(result.exit.has_value());
    REQUIRE(result.sfx == SystemMenuSfxKind::None);
    REQUIRE(state.screen == before.screen);
    REQUIRE(state.pause_selected == before.pause_selected);
}

// ---------------------------------------------------------------- RESIZE

TEST_CASE("system_screen_step: WINDOW_RESIZED/PIXEL_SIZE_CHANGED viram "
          "resize=true e sao NO-OP DE ESTADO",
          "[system_screen_step][f4-1b4]") {
    SystemMenuState state;
    system_menu_open(state);
    const SystemMenuScreen screen_before = state.screen;
    const SystemMenuStepBoxes boxes;

    SDL_Event resized{};
    resized.type = SDL_EVENT_WINDOW_RESIZED;
    const SystemMenuStepResult r1 = system_screen_step(state, resized, boxes);
    REQUIRE(r1.resize);
    REQUIRE_FALSE(r1.window_closed);
    REQUIRE_FALSE(r1.reload);
    REQUIRE(state.screen == screen_before);

    SDL_Event pixel_changed{};
    pixel_changed.type = SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED;
    const SystemMenuStepResult r2 = system_screen_step(state, pixel_changed, boxes);
    REQUIRE(r2.resize);
}

// ---------------------------------------------------------------- PONTO CRITICO #1: ordem da captura de Controles

TEST_CASE("system_screen_step: PONTO CRITICO - a interceptacao de captura de "
          "tecla da tela Controles (state.controls_capturing) vence o "
          "roteamento generico de KEY_DOWN (mutante classico: inverter essa "
          "ordem deixaria controls_capturing PRESO em true pra sempre)",
          "[system_screen_step][f4-1b4][ordem-controles]") {
    SystemMenuState state;
    state.screen = SystemMenuScreen::Controls;
    state.controls_capturing = true;
    state.controls_selected = 0;  // move_forward (default 'W')

    const SystemMenuStepBoxes boxes;
    // Uma tecla comum (nao ESC) enquanto capturando: se a ORDEM estivesse
    // ERRADA (roteamento generico ANTES), cairia em system_menu_key_down, que
    // e um no-op TOTAL quando controls_capturing==true (devolve None sem
    // tocar controls_capturing) - o modo de captura ficaria PRESO pra sempre.
    // Com a ORDEM CORRETA (esta implementacao), system_menu_controls_capture_
    // key roda e SEMPRE desliga controls_capturing (capturou um binding novo
    // OU cancelou) - ver o comentario de kControlsBackIndex/system_menu.hpp.
    const SDL_Event ev = key_down_event(SDLK_A);
    const SystemMenuStepResult result = system_screen_step(state, ev, boxes);

    REQUIRE_FALSE(state.controls_capturing);  // capturado - NUNCA fica preso
    REQUIRE(result.reload);
    REQUIRE_FALSE(result.exit.has_value());
    REQUIRE_FALSE(result.flash.has_value());  // captura nunca dispara o flash de PRESS
}

TEST_CASE("system_screen_step: captura de Controles com ESCAPE cancela (NAO "
          "vira binding) e tambem desliga controls_capturing",
          "[system_screen_step][f4-1b4][ordem-controles]") {
    SystemMenuState state;
    state.screen = SystemMenuScreen::Controls;
    state.controls_capturing = true;
    state.controls_selected = 0;

    const SystemMenuStepBoxes boxes;
    const SDL_Event ev = key_down_event(SDLK_ESCAPE);
    const SystemMenuStepResult result = system_screen_step(state, ev, boxes);

    REQUIRE_FALSE(state.controls_capturing);
    REQUIRE_FALSE(result.exit.has_value());
}

// ---------------------------------------------------------------- navegacao (hover teclado)

TEST_CASE("system_screen_step: navegar (seta Baixo) no Pause toca o SFX de "
          "HOVER e liga reload - a SELECAO muda de fato",
          "[system_screen_step][f4-1b4]") {
    SystemMenuState state;
    system_menu_open(state);
    REQUIRE(state.pause_selected == static_cast<int>(PauseItem::Continue));

    const SystemMenuStepBoxes boxes;
    const SDL_Event ev = key_down_event(SDLK_DOWN);
    const SystemMenuStepResult result = system_screen_step(state, ev, boxes);

    REQUIRE(state.pause_selected == static_cast<int>(PauseItem::Save));
    REQUIRE(result.sfx == SystemMenuSfxKind::Hover);
    REQUIRE(result.reload);
    REQUIRE_FALSE(result.exit.has_value());
    REQUIRE_FALSE(result.flash.has_value());
}

TEST_CASE("system_screen_step: LEFT no Pause e NO-OP de navegacao - hover_sfx "
          "NAO toca",
          "[system_screen_step][f4-1b4]") {
    SystemMenuState state;
    system_menu_open(state);
    const int before = state.pause_selected;

    const SystemMenuStepBoxes boxes;
    const SDL_Event ev = key_down_event(SDLK_LEFT);
    const SystemMenuStepResult result = system_screen_step(state, ev, boxes);

    REQUIRE(state.pause_selected == before);
    REQUIRE(result.sfx == SystemMenuSfxKind::None);
}

// ---------------------------------------------------------------- Continuar (confirm key)

TEST_CASE("system_screen_step: Enter em Continuar devolve exit=Continue COM "
          "flash (is_confirming inclui Continue)",
          "[system_screen_step][f4-1b4]") {
    SystemMenuState state;
    system_menu_open(state);
    REQUIRE(state.pause_selected == static_cast<int>(PauseItem::Continue));
    const SystemMenuState pre_action_state = state;

    const SystemMenuStepBoxes boxes;
    const SDL_Event ev = key_down_event(SDLK_RETURN);
    const SystemMenuStepResult result = system_screen_step(state, ev, boxes);

    REQUIRE(result.exit.has_value());
    REQUIRE(*result.exit == SystemMenuScreenExit::Continue);
    REQUIRE(result.flash.has_value());
    REQUIRE(result.flash->item_index == static_cast<int>(PauseItem::Continue));
    REQUIRE(result.flash->pre_action_state.pause_selected == pre_action_state.pause_selected);
    REQUIRE_FALSE(result.reload);  // exit nao passa por reload - o CHAMADOR encerra a rodada.
}

// ---------------------------------------------------------------- Sair (RequestQuit) TAMBEM flasheia

TEST_CASE("system_screen_step: Enter em Sair devolve exit=RequestQuit COM "
          "flash (is_confirming inclui RequestQuit - o jogador VE o botao "
          "piscar antes do jogo fechar)",
          "[system_screen_step][f4-1b4]") {
    SystemMenuState state;
    system_menu_open(state);
    state.pause_selected = static_cast<int>(PauseItem::Quit);

    const SystemMenuStepBoxes boxes;
    const SDL_Event ev = key_down_event(SDLK_RETURN);
    const SystemMenuStepResult result = system_screen_step(state, ev, boxes);

    REQUIRE(result.exit.has_value());
    REQUIRE(*result.exit == SystemMenuScreenExit::RequestQuit);
    REQUIRE(result.flash.has_value());
    REQUIRE(result.flash->item_index == static_cast<int>(PauseItem::Quit));
}

// ---------------------------------------------------------------- Salvar/Carregar NAO flasheiam

TEST_CASE("system_screen_step: Enter em Salvar devolve exit=OpenSaveLoadSave "
          "SEM flash (OpenSaveLoadSave nao esta em is_confirming - abrir a "
          "tela de save/load nao pisca)",
          "[system_screen_step][f4-1b4]") {
    SystemMenuState state;
    system_menu_open(state);
    state.pause_selected = static_cast<int>(PauseItem::Save);

    const SystemMenuStepBoxes boxes;
    const SDL_Event ev = key_down_event(SDLK_RETURN);
    const SystemMenuStepResult result = system_screen_step(state, ev, boxes);

    REQUIRE(result.exit.has_value());
    REQUIRE(*result.exit == SystemMenuScreenExit::OpenSaveLoadSave);
    REQUIRE_FALSE(result.flash.has_value());
    REQUIRE(state.screen == SystemMenuScreen::Pause);  // nao navega, so sinaliza o exit
}

TEST_CASE("system_screen_step: Enter em Carregar devolve exit=OpenSaveLoadLoad "
          "SEM flash",
          "[system_screen_step][f4-1b4]") {
    SystemMenuState state;
    system_menu_open(state);
    state.pause_selected = static_cast<int>(PauseItem::Load);

    const SystemMenuStepBoxes boxes;
    const SDL_Event ev = key_down_event(SDLK_RETURN);
    const SystemMenuStepResult result = system_screen_step(state, ev, boxes);

    REQUIRE(result.exit.has_value());
    REQUIRE(*result.exit == SystemMenuScreenExit::OpenSaveLoadLoad);
    REQUIRE_FALSE(result.flash.has_value());
}

// ---------------------------------------------------------------- VolumeChanged (tecla)

TEST_CASE("system_screen_step: RIGHT na tela Audio (item Musica) devolve "
          "volume_changed=true + reload=true, SEM flash/exit",
          "[system_screen_step][f4-1b4]") {
    SystemMenuState state;
    state.screen = SystemMenuScreen::Audio;
    state.audio_selected = static_cast<int>(AudioItem::Music);
    state.music_volume = 0.5f;

    const SystemMenuStepBoxes boxes;
    const SDL_Event ev = key_down_event(SDLK_RIGHT);
    const SystemMenuStepResult result = system_screen_step(state, ev, boxes);

    REQUIRE(result.volume_changed);
    REQUIRE(result.reload);
    REQUIRE_FALSE(result.flash.has_value());
    REQUIRE_FALSE(result.exit.has_value());
    REQUIRE(state.music_volume > 0.5f);
}

// ---------------------------------------------------------------- clique DENTRO vs FORA (Pause)

TEST_CASE("system_screen_step: clique DENTRO da box de Continuar devolve "
          "exit=Continue COM flash - clique FORA de qualquer box e NO-OP "
          "TOTAL",
          "[system_screen_step][f4-1b4]") {
    SystemMenuState state;
    system_menu_open(state);

    SystemMenuStepBoxes boxes;
    boxes.pause_items[static_cast<std::size_t>(PauseItem::Continue)] =
        make_box(10.0f, 10.0f, 100.0f, 40.0f);

    SECTION("clique DENTRO da box") {
        const SDL_Event ev = mouse_button_down_event(30.0f, 25.0f);
        const SystemMenuStepResult result = system_screen_step(state, ev, boxes);
        REQUIRE(result.exit.has_value());
        REQUIRE(*result.exit == SystemMenuScreenExit::Continue);
        REQUIRE(result.flash.has_value());
    }

    SECTION("clique FORA de qualquer box (canto vazio)") {
        const int before = state.pause_selected;
        const SDL_Event ev = mouse_button_down_event(500.0f, 500.0f);
        const SystemMenuStepResult result = system_screen_step(state, ev, boxes);
        REQUIRE_FALSE(result.exit.has_value());
        REQUIRE_FALSE(result.flash.has_value());
        REQUIRE_FALSE(result.reload);
        REQUIRE(state.pause_selected == before);  // no-op TOTAL - nem o foco muda
    }
}

// ---------------------------------------------------------------- PONTO CRITICO #2: track do slider VENCE o campo/rotulo

TEST_CASE("system_screen_step: clique no TRACK do slider Musica inicia o "
          "arrasto (start_drag_item) e muda o volume pela FRACAO exata - "
          "vence mesmo quando a caixa do CAMPO/ROTULO tambem cobre o ponto "
          "(o mais especifico vence, PONTO CRITICO #2)",
          "[system_screen_step][f4-1b4][drag]") {
    SystemMenuState state;
    state.screen = SystemMenuScreen::Audio;
    state.music_volume = 0.0f;

    SystemMenuStepBoxes boxes;
    // Track do item 0 (Musica): x=[100,200]. Campo/rotulo (audio_items[0])
    // cobre uma area MAIOR que INCLUI o track - mesma sobreposicao real do
    // RML de producao (o campo engloba o track visualmente).
    boxes.audio_tracks[0] = make_box(100.0f, 50.0f, 100.0f, 20.0f);
    boxes.audio_items[0] = make_box(0.0f, 0.0f, 400.0f, 80.0f);

    // Clique no meio do track (x=150 -> ratio=0.5).
    const SDL_Event ev = mouse_button_down_event(150.0f, 60.0f);
    const SystemMenuStepResult result = system_screen_step(state, ev, boxes);

    REQUIRE(result.start_drag_item.has_value());
    REQUIRE(*result.start_drag_item == 0);
    REQUIRE(result.volume_changed);
    REQUIRE(result.reload);
    REQUIRE(state.audio_selected == 0);
    REQUIRE(state.music_volume == Catch::Approx(0.5f).margin(0.01f));
    REQUIRE_FALSE(result.flash.has_value());  // arrasto de slider nunca flasheia
}

TEST_CASE("system_screen_step: MOUSE_BUTTON_UP (LEFT) devolve end_drag=true - "
          "o CHAMADOR deve zerar o membro drag_item_",
          "[system_screen_step][f4-1b4][drag]") {
    SystemMenuState state;
    state.screen = SystemMenuScreen::Audio;
    const SystemMenuStepBoxes boxes;

    const SDL_Event ev = mouse_button_up_event(0.0f, 0.0f);
    const SystemMenuStepResult result = system_screen_step(state, ev, boxes);

    REQUIRE(result.end_drag);
    REQUIRE_FALSE(result.window_closed);
    REQUIRE_FALSE(result.exit.has_value());
}

// ---------------------------------------------------------------- ControlsApplied (Aplicar) flasheia + sinaliza persistencia

TEST_CASE("system_screen_step: Enter em Aplicar (Controles) devolve "
          "controls_applied=true + flash (is_confirming inclui "
          "ControlsApplied)",
          "[system_screen_step][f4-1b4]") {
    SystemMenuState state;
    state.screen = SystemMenuScreen::Controls;
    state.controls_selected = gus::app::screens::kControlsApplyIndex;
    state.controls_dirty = true;  // ha uma mudanca staged a aplicar

    const SystemMenuStepBoxes boxes;
    const SDL_Event ev = key_down_event(SDLK_RETURN);
    const SystemMenuStepResult result = system_screen_step(state, ev, boxes);

    REQUIRE(result.controls_applied);
    REQUIRE(result.reload);
    REQUIRE(result.flash.has_value());
    REQUIRE_FALSE(result.exit.has_value());
    REQUIRE_FALSE(state.controls_dirty);  // ja promovido a baseline (efeito em MEMORIA)
}

// ---------------------------------------------------------------- MOUSE_MOTION

TEST_CASE("system_screen_step: MOUSE_MOTION vira mouse_move=true com (x,y) "
          "repassados, sem tocar estado/sfx/reload/exit",
          "[system_screen_step][f4-1b4]") {
    SystemMenuState state;
    system_menu_open(state);
    const int before = state.pause_selected;

    SDL_Event ev{};
    ev.type = SDL_EVENT_MOUSE_MOTION;
    ev.motion.x = 42.0f;
    ev.motion.y = 84.0f;
    const SystemMenuStepBoxes boxes;
    const SystemMenuStepResult result = system_screen_step(state, ev, boxes);

    REQUIRE(result.mouse_move);
    REQUIRE(result.mouse_x == 42.0f);
    REQUIRE(result.mouse_y == 84.0f);
    REQUIRE_FALSE(result.reload);
    REQUIRE(result.sfx == SystemMenuSfxKind::None);
    REQUIRE_FALSE(result.exit.has_value());
    REQUIRE(state.pause_selected == before);
}

// ---------------------------------------------------------------- MOUSE_WHEEL (fora do escopo desta funcao)

TEST_CASE("system_screen_step: SDL_EVENT_MOUSE_WHEEL e NO-OP TOTAL (exige "
          "SDL_GetMouseState, impuro - tratado direto pelo CHAMADOR)",
          "[system_screen_step][f4-1b4]") {
    SystemMenuState state;
    system_menu_open(state);

    SDL_Event ev{};
    ev.type = SDL_EVENT_MOUSE_WHEEL;
    ev.wheel.y = 1.0f;
    const SystemMenuStepBoxes boxes;
    const SystemMenuStepResult result = system_screen_step(state, ev, boxes);

    REQUIRE_FALSE(result.mouse_move);
    REQUIRE_FALSE(result.reload);
    REQUIRE_FALSE(result.exit.has_value());
    REQUIRE(result.sfx == SystemMenuSfxKind::None);
}

// ---------------------------------------------------------------- evento nao roteado / repeat

TEST_CASE("system_screen_step: tipo de evento que esta tela nao roteia (ex.: "
          "TEXT_INPUT) e NO-OP TOTAL (resultado default)",
          "[system_screen_step][f4-1b4]") {
    SystemMenuState state;
    system_menu_open(state);

    SDL_Event ev{};
    ev.type = SDL_EVENT_TEXT_INPUT;
    const SystemMenuStepBoxes boxes;
    const SystemMenuStepResult result = system_screen_step(state, ev, boxes);

    REQUIRE_FALSE(result.window_closed);
    REQUIRE_FALSE(result.resize);
    REQUIRE_FALSE(result.mouse_move);
    REQUIRE_FALSE(result.reload);
    REQUIRE(result.sfx == SystemMenuSfxKind::None);
    REQUIRE_FALSE(result.exit.has_value());
}

TEST_CASE("system_screen_step: SDL_EVENT_KEY_DOWN com key.repeat!=0 (auto-"
          "repeat do SO) e NO-OP TOTAL - so a 1a borda (repeat=0) e roteada",
          "[system_screen_step][f4-1b4]") {
    SystemMenuState state;
    system_menu_open(state);
    const int before = state.pause_selected;

    const SystemMenuStepBoxes boxes;
    const SDL_Event ev = key_down_event(SDLK_DOWN, /*repeat=*/true);
    const SystemMenuStepResult result = system_screen_step(state, ev, boxes);

    REQUIRE_FALSE(result.reload);
    REQUIRE(result.sfx == SystemMenuSfxKind::None);
    REQUIRE_FALSE(result.exit.has_value());
    REQUIRE(state.pause_selected == before);
}

// ================================================================== pause_flow_next

// F4-1b.4: driver PURO - os 3 ramos de OpenSaveLoadSave/OpenSaveLoadLoad
// (BackToPause->RetryPause, ClosedAfterLoad->Continue, QuitApp->RequestQuit)
// + a IGNORANCIA de saveload_exit fora desses 2 casos (Continue/RequestQuit/
// RequestToTitle sempre devolvem o MESMO FlowStep, qualquer que seja
// saveload_exit).

TEST_CASE("pause_flow_next: Continue -> SystemMenuFlowStep::Continue, "
          "IGNORANDO saveload_exit (as 3 variantes devolvem o MESMO)",
          "[pause_flow_next][f4-1b4]") {
    REQUIRE(pause_flow_next(SystemMenuScreenExit::Continue, SaveLoadLoopExit::BackToPause) ==
            SystemMenuFlowStep::Continue);
    REQUIRE(pause_flow_next(SystemMenuScreenExit::Continue, SaveLoadLoopExit::ClosedAfterLoad) ==
            SystemMenuFlowStep::Continue);
    REQUIRE(pause_flow_next(SystemMenuScreenExit::Continue, SaveLoadLoopExit::QuitApp) ==
            SystemMenuFlowStep::Continue);
}

TEST_CASE("pause_flow_next: RequestQuit (no PROPRIO Pause) -> SystemMenuFlowStep::"
          "RequestQuit, IGNORANDO saveload_exit",
          "[pause_flow_next][f4-1b4]") {
    REQUIRE(pause_flow_next(SystemMenuScreenExit::RequestQuit, SaveLoadLoopExit::BackToPause) ==
            SystemMenuFlowStep::RequestQuit);
    REQUIRE(pause_flow_next(SystemMenuScreenExit::RequestQuit, SaveLoadLoopExit::ClosedAfterLoad) ==
            SystemMenuFlowStep::RequestQuit);
    REQUIRE(pause_flow_next(SystemMenuScreenExit::RequestQuit, SaveLoadLoopExit::QuitApp) ==
            SystemMenuFlowStep::RequestQuit);
}

TEST_CASE("pause_flow_next: RequestToTitle -> SystemMenuFlowStep::"
          "RequestToTitle, IGNORANDO saveload_exit",
          "[pause_flow_next][f4-1b4]") {
    REQUIRE(pause_flow_next(SystemMenuScreenExit::RequestToTitle, SaveLoadLoopExit::BackToPause) ==
            SystemMenuFlowStep::RequestToTitle);
    REQUIRE(pause_flow_next(SystemMenuScreenExit::RequestToTitle, SaveLoadLoopExit::ClosedAfterLoad) ==
            SystemMenuFlowStep::RequestToTitle);
    REQUIRE(pause_flow_next(SystemMenuScreenExit::RequestToTitle, SaveLoadLoopExit::QuitApp) ==
            SystemMenuFlowStep::RequestToTitle);
}

TEST_CASE("pause_flow_next: OpenSaveLoadSave + BackToPause -> RetryPause (RE-"
          "ENTRA no MESMO SystemMenuLoopScreen)",
          "[pause_flow_next][f4-1b4]") {
    REQUIRE(pause_flow_next(SystemMenuScreenExit::OpenSaveLoadSave, SaveLoadLoopExit::BackToPause) ==
            SystemMenuFlowStep::RetryPause);
}

TEST_CASE("pause_flow_next: OpenSaveLoadSave + ClosedAfterLoad -> Continue "
          "(MESMO efeito pratico de Continuar - o Load ja aplicou no jogo VIVO)",
          "[pause_flow_next][f4-1b4]") {
    REQUIRE(pause_flow_next(SystemMenuScreenExit::OpenSaveLoadSave, SaveLoadLoopExit::ClosedAfterLoad) ==
            SystemMenuFlowStep::Continue);
}

TEST_CASE("pause_flow_next: OpenSaveLoadSave + QuitApp (janela fechada DENTRO "
          "da tela de save/load) -> RequestQuit (propaga)",
          "[pause_flow_next][f4-1b4]") {
    REQUIRE(pause_flow_next(SystemMenuScreenExit::OpenSaveLoadSave, SaveLoadLoopExit::QuitApp) ==
            SystemMenuFlowStep::RequestQuit);
}

// MESMOS 3 ramos, agora via OpenSaveLoadLoad - prova que o modo (Save x Load)
// NAO muda a matriz de decisao (so o `mode` passado a
// run_save_load_menu_loop_gl_current, decidido pelo CHAMADOR, ver o .cpp).

TEST_CASE("pause_flow_next: OpenSaveLoadLoad + BackToPause -> RetryPause",
          "[pause_flow_next][f4-1b4]") {
    REQUIRE(pause_flow_next(SystemMenuScreenExit::OpenSaveLoadLoad, SaveLoadLoopExit::BackToPause) ==
            SystemMenuFlowStep::RetryPause);
}

TEST_CASE("pause_flow_next: OpenSaveLoadLoad + ClosedAfterLoad -> Continue",
          "[pause_flow_next][f4-1b4]") {
    REQUIRE(pause_flow_next(SystemMenuScreenExit::OpenSaveLoadLoad, SaveLoadLoopExit::ClosedAfterLoad) ==
            SystemMenuFlowStep::Continue);
}

TEST_CASE("pause_flow_next: OpenSaveLoadLoad + QuitApp -> RequestQuit",
          "[pause_flow_next][f4-1b4]") {
    REQUIRE(pause_flow_next(SystemMenuScreenExit::OpenSaveLoadLoad, SaveLoadLoopExit::QuitApp) ==
            SystemMenuFlowStep::RequestQuit);
}

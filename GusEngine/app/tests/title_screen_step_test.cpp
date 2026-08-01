// SPDX-License-Identifier: Apache-2.0
// GusEngine/app/tests/title_screen_step_test.cpp
//
// Catch2 (headless, SEM SDL_Init/janela/GL/glintfx::UiLayer real) de
// gus::app::screens::title_screen_step + gus::app::screens::title_flow_next
// (F4-1b.3, onda F4 "casca SDL -> App mode do glintfx", fatia 1b.3 - PRIMEIRA
// tela PAI da onda, apos F4-1b.1/difficulty_menu_loop.cpp e F4-1b.2/
// save_load_menu_loop.cpp - introduz o padrao do MINI-DRIVER, ver o comentario
// grande no topo de title_menu_loop.cpp). Ambas as funcoes sao PURAS o
// suficiente pra serem exercitadas com SDL_Event/glintfx::ElementBox
// construidos a mao (MESMA tecnica de difficulty_screen_step_test.cpp) - nunca
// consultam a UiLayer, as boxes/o desfecho de dificuldade entram como
// parametro. O CONSUMIDOR real (TitleScreen, GL-heavy, e o MINI-DRIVER dentro
// de run_title_menu_loop_gl_current) continua sem teste direto aqui - ver
// title_menu_loop_interaction_test.cpp (harness GL/SDL_PushEvent).

#include <array>

#include <catch2/catch_test_macros.hpp>

#include <SDL3/SDL.h>
#include <glintfx/element_box.hpp>

#include "gus/app/screens/difficulty_menu_loop.hpp"
#include "gus/app/screens/title_menu.hpp"
#include "gus/app/screens/title_menu_loop.hpp"

using gus::app::screens::DifficultyLoopExit;
using gus::app::screens::kTitleItemCount;
using gus::app::screens::title_flow_next;
using gus::app::screens::title_menu_open;
using gus::app::screens::title_screen_step;
using gus::app::screens::TitleFlowStep;
using gus::app::screens::TitleMenuItem;
using gus::app::screens::TitleMenuState;
using gus::app::screens::TitleScreenExit;
using gus::app::screens::TitleSfxKind;
using gus::app::screens::TitleStepResult;

namespace {

// Boxes TODAS "nao encontradas" (found=false) - MESMO default de
// glintfx::ElementBox - representa "nenhuma box resolvida" (a maioria dos
// tipos de evento nem consulta `boxes`).
std::array<glintfx::ElementBox, kTitleItemCount> no_boxes() { return {}; }

glintfx::ElementBox make_box(float x, float y, float w, float h) {
    glintfx::ElementBox box;
    box.found = true;
    box.x = x;
    box.y = y;
    box.w = w;
    box.h = h;
    return box;
}

SDL_Event key_down_event(SDL_Keycode key) {
    SDL_Event ev{};
    ev.type = SDL_EVENT_KEY_DOWN;
    ev.key.key = key;
    ev.key.repeat = 0;
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

bool state_eq(const TitleMenuState& a, const TitleMenuState& b) {
    return a.selected == b.selected && a.any_save_exists == b.any_save_exists &&
           a.confirming_new_game == b.confirming_new_game &&
           a.confirm_selected == b.confirm_selected;
}

}  // namespace

// ---------------------------------------------------------------- QUIT

TEST_CASE("title_screen_step: SDL_EVENT_QUIT vira window_closed=true, sem "
          "tocar estado/sfx/reload/exit",
          "[title_screen_step][f4-1b3]") {
    TitleMenuState state;
    title_menu_open(state, /*any_save_exists=*/true);
    const TitleMenuState before = state;

    SDL_Event ev{};
    ev.type = SDL_EVENT_QUIT;
    const auto boxes = no_boxes();
    const TitleStepResult result = title_screen_step(state, ev, boxes.data());

    REQUIRE(result.window_closed);
    REQUIRE_FALSE(result.resize);
    REQUIRE_FALSE(result.reload);
    REQUIRE_FALSE(result.exit.has_value());
    REQUIRE(result.sfx == TitleSfxKind::None);
    REQUIRE(state_eq(state, before));
}

// ---------------------------------------------------------------- RESIZE

TEST_CASE("title_screen_step: WINDOW_RESIZED/PIXEL_SIZE_CHANGED viram "
          "resize=true e sao NO-OP DE ESTADO - o CHAMADOR e quem reposiciona a "
          "UiLayer",
          "[title_screen_step][f4-1b3]") {
    TitleMenuState state;
    title_menu_open(state, /*any_save_exists=*/true);
    const TitleMenuState before = state;
    const auto boxes = no_boxes();

    SDL_Event resized{};
    resized.type = SDL_EVENT_WINDOW_RESIZED;
    const TitleStepResult r1 = title_screen_step(state, resized, boxes.data());
    REQUIRE(r1.resize);
    REQUIRE_FALSE(r1.window_closed);
    REQUIRE_FALSE(r1.reload);
    REQUIRE_FALSE(r1.exit.has_value());
    REQUIRE(r1.sfx == TitleSfxKind::None);
    REQUIRE(state_eq(state, before));

    SDL_Event pixel_changed{};
    pixel_changed.type = SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED;
    const TitleStepResult r2 = title_screen_step(state, pixel_changed, boxes.data());
    REQUIRE(r2.resize);
    REQUIRE(state_eq(state, before));
}

// ---------------------------------------------------------------- navegacao (hover teclado)

TEST_CASE("title_screen_step: navegar (seta Baixo) entre 2 itens toca o SFX de "
          "HOVER e liga reload - a SELECAO muda de fato",
          "[title_screen_step][f4-1b3]") {
    TitleMenuState state;
    title_menu_open(state, /*any_save_exists=*/true);  // foco inicial = Continuar (0)
    REQUIRE(state.selected == static_cast<int>(TitleMenuItem::Continue));

    const auto boxes = no_boxes();
    const SDL_Event ev = key_down_event(SDLK_DOWN);
    const TitleStepResult result = title_screen_step(state, ev, boxes.data());

    REQUIRE(state.selected == static_cast<int>(TitleMenuItem::NewGame));
    REQUIRE(result.sfx == TitleSfxKind::Hover);
    REQUIRE(result.reload);
    REQUIRE_FALSE(result.exit.has_value());
}

// ---------------------------------------------------------------- Continuar (confirm key)

TEST_CASE("title_screen_step: Enter em Continuar (selecionavel, any_save_exists) "
          "toca Click e devolve exit=ContinueGame (sem reload)",
          "[title_screen_step][f4-1b3]") {
    TitleMenuState state;
    title_menu_open(state, /*any_save_exists=*/true);
    REQUIRE(state.selected == static_cast<int>(TitleMenuItem::Continue));

    const auto boxes = no_boxes();
    const SDL_Event ev = key_down_event(SDLK_RETURN);
    const TitleStepResult result = title_screen_step(state, ev, boxes.data());

    REQUIRE(result.sfx == TitleSfxKind::Click);
    REQUIRE(result.exit.has_value());
    REQUIRE(*result.exit == TitleScreenExit::ContinueGame);
    REQUIRE_FALSE(result.reload);  // ContinueGame nao passa por reload - o
                                   // CHAMADOR retorna na hora.
}

// ---------------------------------------------------------------- (B2) EFEITO DE PRESS

TEST_CASE("title_screen_step (B2): Enter num item seta result.flash (pre_action_state "
          "+ item_index), MESMO quando o resultado e abrir o mini-dialogo (None)",
          "[title_screen_step][b2-flash]") {
    TitleMenuState state;
    title_menu_open(state, /*any_save_exists=*/true);
    state.selected = static_cast<int>(TitleMenuItem::NewGame);

    const auto boxes = no_boxes();
    const SDL_Event ev = key_down_event(SDLK_RETURN);
    const TitleStepResult result = title_screen_step(state, ev, boxes.data());

    REQUIRE(result.flash.has_value());
    REQUIRE(result.flash->item_index == static_cast<int>(TitleMenuItem::NewGame));
    REQUIRE_FALSE(result.flash->pre_action_state.confirming_new_game);  // ANTES da mutacao
    REQUIRE(state.confirming_new_game);  // DEPOIS - o dialogo ja abriu de fato
}

TEST_CASE("title_screen_step (B2): navegar (seta) NUNCA seta flash - so Enter/clique "
          "confirmam",
          "[title_screen_step][b2-flash]") {
    TitleMenuState state;
    title_menu_open(state, /*any_save_exists=*/true);

    const auto boxes = no_boxes();
    const SDL_Event ev = key_down_event(SDLK_DOWN);
    const TitleStepResult result = title_screen_step(state, ev, boxes.data());

    REQUIRE_FALSE(result.flash.has_value());
}

TEST_CASE("title_screen_step (B2): clicar num item DESABILITADO nao seta flash "
          "(MESMA semantica de nao tocar som)",
          "[title_screen_step][b2-flash]") {
    TitleMenuState state;
    title_menu_open(state, /*any_save_exists=*/false);  // Continuar desabilitado

    std::array<glintfx::ElementBox, kTitleItemCount> boxes{};
    boxes[static_cast<std::size_t>(TitleMenuItem::Continue)] = make_box(10, 10, 100, 30);
    SDL_Event ev{};
    ev.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
    ev.button.button = SDL_BUTTON_LEFT;
    ev.button.x = 30;
    ev.button.y = 20;
    const TitleStepResult result = title_screen_step(state, ev, boxes.data());

    REQUIRE_FALSE(result.flash.has_value());
    REQUIRE(result.sfx == TitleSfxKind::None);
}

TEST_CASE("title_screen_step (B2): clicar numa pill do mini-dialogo seta flash "
          "(item_index = indice da pill)",
          "[title_screen_step][b2-flash]") {
    TitleMenuState state;
    title_menu_open(state, /*any_save_exists=*/true);
    state.selected = static_cast<int>(TitleMenuItem::NewGame);
    (void)title_screen_step(state, key_down_event(SDLK_RETURN), no_boxes().data());
    REQUIRE(state.confirming_new_game);

    std::array<glintfx::ElementBox, kTitleItemCount> boxes{};
    boxes[0] = make_box(10, 10, 100, 30);  // pill "Sim" (indice 0)
    SDL_Event ev{};
    ev.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
    ev.button.button = SDL_BUTTON_LEFT;
    ev.button.x = 30;
    ev.button.y = 20;
    const TitleStepResult result = title_screen_step(state, ev, boxes.data());

    REQUIRE(result.flash.has_value());
    REQUIRE(result.flash->item_index == 0);
    REQUIRE(result.exit.has_value());
    REQUIRE(*result.exit == TitleScreenExit::NewGameRequested);
}

// ---------------------------------------------------------------- Novo Jogo SEM save existente

TEST_CASE("title_screen_step: Enter em Novo Jogo SEM nenhum save existente "
          "confirma DIRETO (sem mini-dialogo) - exit=NewGameRequested",
          "[title_screen_step][f4-1b3]") {
    TitleMenuState state;
    title_menu_open(state, /*any_save_exists=*/false);  // Continuar desabilitado
    REQUIRE(state.selected == static_cast<int>(TitleMenuItem::NewGame));  // foco pulou Continuar

    const auto boxes = no_boxes();
    const SDL_Event ev = key_down_event(SDLK_RETURN);
    const TitleStepResult result = title_screen_step(state, ev, boxes.data());

    REQUIRE(result.sfx == TitleSfxKind::Click);
    REQUIRE(result.exit.has_value());
    REQUIRE(*result.exit == TitleScreenExit::NewGameRequested);
    REQUIRE_FALSE(state.confirming_new_game);  // nunca abriu o mini-dialogo
}

// ---------------------------------------------------------------- Novo Jogo COM save existente (mini-dialogo)

TEST_CASE("title_screen_step: Enter em Novo Jogo COM save existente abre o "
          "mini-dialogo (action=None, reload=true, sem exit) - Enter de novo "
          "com confirm_selected=Sim (0) devolve exit=NewGameRequested; ESC "
          "fecha o dialogo (Nao) sem exit",
          "[title_screen_step][f4-1b3]") {
    TitleMenuState state;
    title_menu_open(state, /*any_save_exists=*/true);
    state.selected = static_cast<int>(TitleMenuItem::NewGame);

    const auto boxes = no_boxes();

    SECTION("abre o dialogo, confirma Sim") {
        const SDL_Event open_ev = key_down_event(SDLK_RETURN);
        const TitleStepResult open_result = title_screen_step(state, open_ev, boxes.data());
        REQUIRE(open_result.sfx == TitleSfxKind::Click);
        REQUIRE(open_result.reload);
        REQUIRE_FALSE(open_result.exit.has_value());
        REQUIRE(state.confirming_new_game);
        REQUIRE(state.confirm_selected == 1);  // default seguro = Nao

        // move pra "Sim" (indice 0) antes de confirmar.
        const SDL_Event move_ev = key_down_event(SDLK_LEFT);
        (void)title_screen_step(state, move_ev, boxes.data());
        REQUIRE(state.confirm_selected == 0);

        const SDL_Event confirm_ev = key_down_event(SDLK_RETURN);
        const TitleStepResult confirm_result =
            title_screen_step(state, confirm_ev, boxes.data());
        REQUIRE(confirm_result.sfx == TitleSfxKind::Click);
        REQUIRE(confirm_result.exit.has_value());
        REQUIRE(*confirm_result.exit == TitleScreenExit::NewGameRequested);
    }

    SECTION("abre o dialogo, ESC fecha (Nao) - sem exit, volta pra lista") {
        const SDL_Event open_ev = key_down_event(SDLK_RETURN);
        (void)title_screen_step(state, open_ev, boxes.data());
        REQUIRE(state.confirming_new_game);

        const SDL_Event esc_ev = key_down_event(SDLK_ESCAPE);
        const TitleStepResult esc_result = title_screen_step(state, esc_ev, boxes.data());
        REQUIRE_FALSE(esc_result.exit.has_value());
        REQUIRE(esc_result.reload);
        REQUIRE_FALSE(state.confirming_new_game);
        // ESC NAO e is_confirm_key (so Enter/KP_Enter/Espaco) - cai no ramo de
        // NAVEGACAO, que so toca Hover se o MODO nao mudou; aqui o MODO MUDOU
        // (fechou o mini-dialogo), entao NENHUM som toca - MESMO guard do
        // while(true) antigo ("Trocar de MODO NAO conta como hover num item
        // novo").
        REQUIRE(esc_result.sfx == TitleSfxKind::None);
    }
}

// ---------------------------------------------------------------- Sair (RequestQuit)

TEST_CASE("title_screen_step: Enter em Sair devolve exit=QuitApp",
          "[title_screen_step][f4-1b3]") {
    TitleMenuState state;
    title_menu_open(state, /*any_save_exists=*/true);
    state.selected = static_cast<int>(TitleMenuItem::Quit);

    const auto boxes = no_boxes();
    const SDL_Event ev = key_down_event(SDLK_RETURN);
    const TitleStepResult result = title_screen_step(state, ev, boxes.data());

    REQUIRE(result.sfx == TitleSfxKind::Click);
    REQUIRE(result.exit.has_value());
    REQUIRE(*result.exit == TitleScreenExit::QuitApp);
}

// ---------------------------------------------------------------- clique DENTRO vs FORA

TEST_CASE("title_screen_step: clique DENTRO da box de Continuar (selecionavel) "
          "toca Click e devolve exit=ContinueGame - clique FORA de qualquer box "
          "e NO-OP TOTAL",
          "[title_screen_step][f4-1b3]") {
    TitleMenuState state;
    title_menu_open(state, /*any_save_exists=*/true);

    std::array<glintfx::ElementBox, kTitleItemCount> boxes{};
    boxes[static_cast<std::size_t>(TitleMenuItem::Continue)] =
        make_box(/*x=*/10.0f, /*y=*/10.0f, /*w=*/100.0f, /*h=*/40.0f);

    SECTION("clique DENTRO da box") {
        const SDL_Event ev = mouse_button_down_event(30.0f, 25.0f);
        const TitleStepResult result = title_screen_step(state, ev, boxes.data());
        REQUIRE(result.sfx == TitleSfxKind::Click);
        REQUIRE(result.exit.has_value());
        REQUIRE(*result.exit == TitleScreenExit::ContinueGame);
    }

    SECTION("clique FORA de qualquer box (canto vazio)") {
        const TitleMenuState before = state;
        const SDL_Event ev = mouse_button_down_event(500.0f, 500.0f);
        const TitleStepResult result = title_screen_step(state, ev, boxes.data());
        REQUIRE(result.sfx == TitleSfxKind::None);
        REQUIRE_FALSE(result.reload);
        REQUIRE_FALSE(result.exit.has_value());
        REQUIRE(state_eq(state, before));  // no-op TOTAL - nem o foco muda
    }
}

TEST_CASE("title_screen_step: clique DENTRO da box de Continuar DESABILITADO "
          "(sem save existente) e no-op TOTAL, inclusive de som (nao ha SFX "
          "bloqueado na tela de titulo)",
          "[title_screen_step][f4-1b3]") {
    TitleMenuState state;
    title_menu_open(state, /*any_save_exists=*/false);
    const TitleMenuState before = state;

    std::array<glintfx::ElementBox, kTitleItemCount> boxes{};
    boxes[static_cast<std::size_t>(TitleMenuItem::Continue)] =
        make_box(10.0f, 10.0f, 100.0f, 40.0f);

    const SDL_Event ev = mouse_button_down_event(30.0f, 25.0f);
    const TitleStepResult result = title_screen_step(state, ev, boxes.data());

    REQUIRE(result.sfx == TitleSfxKind::None);
    REQUIRE_FALSE(result.exit.has_value());
    REQUIRE(state_eq(state, before));  // estado imutavel, nem o foco muda
}

TEST_CASE("title_screen_step: dentro do mini-dialogo, clique na pill 0 (Sim) "
          "devolve exit=NewGameRequested; clique na pill 1 (Cancelar) fecha o "
          "dialogo (reload=true, sem exit)",
          "[title_screen_step][f4-1b3]") {
    TitleMenuState state;
    title_menu_open(state, /*any_save_exists=*/true);
    state.selected = static_cast<int>(TitleMenuItem::NewGame);
    state.confirming_new_game = true;
    state.confirm_selected = 1;

    std::array<glintfx::ElementBox, kTitleItemCount> boxes{};
    boxes[0] = make_box(0.0f, 0.0f, 50.0f, 50.0f);    // pill "Sim"
    boxes[1] = make_box(100.0f, 0.0f, 50.0f, 50.0f);  // pill "Cancelar"

    SECTION("Sim") {
        const SDL_Event ev = mouse_button_down_event(10.0f, 10.0f);
        const TitleStepResult result = title_screen_step(state, ev, boxes.data());
        REQUIRE(result.sfx == TitleSfxKind::Click);
        REQUIRE(result.exit.has_value());
        REQUIRE(*result.exit == TitleScreenExit::NewGameRequested);
    }

    SECTION("Cancelar") {
        const SDL_Event ev = mouse_button_down_event(110.0f, 10.0f);
        const TitleStepResult result = title_screen_step(state, ev, boxes.data());
        REQUIRE(result.sfx == TitleSfxKind::Click);
        REQUIRE(result.reload);
        REQUIRE_FALSE(result.exit.has_value());
        REQUIRE_FALSE(state.confirming_new_game);
    }
}

// ---------------------------------------------------------------- MOUSE_MOTION

TEST_CASE("title_screen_step: MOUSE_MOTION vira mouse_move=true com (x,y) "
          "repassados, sem tocar estado/sfx/reload/exit",
          "[title_screen_step][f4-1b3]") {
    TitleMenuState state;
    title_menu_open(state, /*any_save_exists=*/true);
    const TitleMenuState before = state;

    SDL_Event ev{};
    ev.type = SDL_EVENT_MOUSE_MOTION;
    ev.motion.x = 42.0f;
    ev.motion.y = 84.0f;
    const auto boxes = no_boxes();
    const TitleStepResult result = title_screen_step(state, ev, boxes.data());

    REQUIRE(result.mouse_move);
    REQUIRE(result.mouse_x == 42.0f);
    REQUIRE(result.mouse_y == 84.0f);
    REQUIRE_FALSE(result.reload);
    REQUIRE(result.sfx == TitleSfxKind::None);
    REQUIRE_FALSE(result.exit.has_value());
    REQUIRE(state_eq(state, before));
}

// ---------------------------------------------------------------- evento nao roteado / repeat

TEST_CASE("title_screen_step: tipo de evento que esta tela nao roteia (ex.: "
          "TEXT_INPUT) e NO-OP TOTAL (resultado default)",
          "[title_screen_step][f4-1b3]") {
    TitleMenuState state;
    title_menu_open(state, /*any_save_exists=*/true);
    const TitleMenuState before = state;

    SDL_Event ev{};
    ev.type = SDL_EVENT_TEXT_INPUT;
    const auto boxes = no_boxes();
    const TitleStepResult result = title_screen_step(state, ev, boxes.data());

    REQUIRE_FALSE(result.window_closed);
    REQUIRE_FALSE(result.resize);
    REQUIRE_FALSE(result.mouse_move);
    REQUIRE_FALSE(result.reload);
    REQUIRE(result.sfx == TitleSfxKind::None);
    REQUIRE_FALSE(result.exit.has_value());
    REQUIRE(state_eq(state, before));
}

TEST_CASE("title_screen_step: SDL_EVENT_KEY_DOWN com key.repeat!=0 (auto-repeat "
          "do SO) e NO-OP TOTAL - so a 1a borda (repeat=0) e roteada",
          "[title_screen_step][f4-1b3]") {
    TitleMenuState state;
    title_menu_open(state, /*any_save_exists=*/true);
    const TitleMenuState before = state;

    SDL_Event ev{};
    ev.type = SDL_EVENT_KEY_DOWN;
    ev.key.key = SDLK_DOWN;
    ev.key.repeat = 1;
    const auto boxes = no_boxes();
    const TitleStepResult result = title_screen_step(state, ev, boxes.data());

    REQUIRE_FALSE(result.reload);
    REQUIRE(result.sfx == TitleSfxKind::None);
    REQUIRE_FALSE(result.exit.has_value());
    REQUIRE(state_eq(state, before));
}

// ================================================================== title_flow_next

// F4-1b.3: driver PURO - as 3x3 combinacoes de TitleScreenExit x
// DifficultyLoopExit, provando EXPLICITAMENTE que difficulty_exit so importa
// quando title_exit==NewGameRequested (nos outros 2 casos, QUALQUER
// DifficultyLoopExit devolve o MESMO FlowStep - o mini-driver nem chega a
// rodar a tela de dificuldade nesses casos, ver run_title_menu_loop_gl_current).

TEST_CASE("title_flow_next: ContinueGame -> TitleFlowStep::ContinueGame, "
          "IGNORANDO difficulty_exit (as 3 variantes devolvem o MESMO)",
          "[title_flow_next][f4-1b3]") {
    REQUIRE(title_flow_next(TitleScreenExit::ContinueGame, DifficultyLoopExit::Cancelled) ==
            TitleFlowStep::ContinueGame);
    REQUIRE(title_flow_next(TitleScreenExit::ContinueGame, DifficultyLoopExit::Chosen) ==
            TitleFlowStep::ContinueGame);
    REQUIRE(title_flow_next(TitleScreenExit::ContinueGame, DifficultyLoopExit::QuitApp) ==
            TitleFlowStep::ContinueGame);
}

TEST_CASE("title_flow_next: QuitApp (na LISTA de titulo) -> TitleFlowStep::"
          "QuitApp, IGNORANDO difficulty_exit (as 3 variantes devolvem o MESMO)",
          "[title_flow_next][f4-1b3]") {
    REQUIRE(title_flow_next(TitleScreenExit::QuitApp, DifficultyLoopExit::Cancelled) ==
            TitleFlowStep::QuitApp);
    REQUIRE(title_flow_next(TitleScreenExit::QuitApp, DifficultyLoopExit::Chosen) ==
            TitleFlowStep::QuitApp);
    REQUIRE(title_flow_next(TitleScreenExit::QuitApp, DifficultyLoopExit::QuitApp) ==
            TitleFlowStep::QuitApp);
}

TEST_CASE("title_flow_next: NewGameRequested + Cancelled -> RetryTitle (RE-"
          "ENTRA no mesmo TitleScreen)",
          "[title_flow_next][f4-1b3]") {
    REQUIRE(title_flow_next(TitleScreenExit::NewGameRequested, DifficultyLoopExit::Cancelled) ==
            TitleFlowStep::RetryTitle);
}

TEST_CASE("title_flow_next: NewGameRequested + Chosen -> StartNewGame",
          "[title_flow_next][f4-1b3]") {
    REQUIRE(title_flow_next(TitleScreenExit::NewGameRequested, DifficultyLoopExit::Chosen) ==
            TitleFlowStep::StartNewGame);
}

TEST_CASE("title_flow_next: NewGameRequested + QuitApp (janela fechada DENTRO "
          "da tela de dificuldade) -> TitleFlowStep::QuitApp (propaga)",
          "[title_flow_next][f4-1b3]") {
    REQUIRE(title_flow_next(TitleScreenExit::NewGameRequested, DifficultyLoopExit::QuitApp) ==
            TitleFlowStep::QuitApp);
}

// GusEngine/app/tests/difficulty_screen_step_test.cpp
//
// Catch2 (headless, SEM SDL_Init/janela/GL/glintfx::UiLayer real) de
// gus::app::screens::difficulty_screen_step (F4-1b, onda F4 "casca SDL -> App
// mode do glintfx", fatia 1b - PRIMEIRA tela convertida ao contrato ScreenState
// apos F4-1a/npc_dialogue_loop_gl.cpp). A funcao e PURA o suficiente pra ser
// exercitada com SDL_Event/glintfx::ElementBox construidos a mao (SDL_Event e
// glintfx::ElementBox sao structs simples - MESMA tecnica ja usada em
// screen_state_test.cpp, nao precisa de SDL_Init/janela/display/GL nenhum) - ela
// NUNCA consulta a UiLayer, as boxes entram como parametro. O CONSUMIDOR real
// (DifficultyScreen, GL-heavy) continua sem teste direto - ver o topo de
// gus/app/src/screens/difficulty_menu_loop.cpp.

#include <array>

#include <catch2/catch_test_macros.hpp>

#include <SDL3/SDL.h>
#include <glintfx/element_box.hpp>

#include "gus/app/screens/difficulty_menu.hpp"
#include "gus/app/screens/difficulty_menu_loop.hpp"

using gus::app::screens::DifficultyLoopExit;
using gus::app::screens::difficulty_screen_step;
using gus::app::screens::DifficultyMenuItem;
using gus::app::screens::DifficultyMenuState;
using gus::app::screens::DifficultySfxKind;
using gus::app::screens::DifficultyStepResult;
using gus::app::screens::kDifficultyItemCount;

namespace {

// Boxes TODAS "nao encontradas" (found=false) - MESMO default de
// glintfx::ElementBox (kInvalidTexture-like) - representa "nenhuma box
// resolvida" (a maioria dos tipos de evento nem consulta `boxes`).
std::array<glintfx::ElementBox, kDifficultyItemCount> no_boxes() { return {}; }

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

bool state_eq(const DifficultyMenuState& a, const DifficultyMenuState& b) {
    return a.selected == b.selected && a.confirming == b.confirming &&
           a.confirm_selected == b.confirm_selected &&
           a.hardcore_unlocked == b.hardcore_unlocked;
}

}  // namespace

// ---------------------------------------------------------------- QUIT

TEST_CASE("difficulty_screen_step: SDL_EVENT_QUIT vira window_closed=true, "
          "sem tocar estado/sfx/reload/exit",
          "[difficulty_screen_step][f4-1b]") {
    DifficultyMenuState state;
    difficulty_menu_open(state);
    const DifficultyMenuState before = state;

    SDL_Event ev{};
    ev.type = SDL_EVENT_QUIT;
    const auto boxes = no_boxes();
    const DifficultyStepResult result = difficulty_screen_step(state, ev, boxes.data());

    REQUIRE(result.window_closed);
    REQUIRE_FALSE(result.resize);
    REQUIRE_FALSE(result.reload);
    REQUIRE_FALSE(result.exit.has_value());
    REQUIRE(result.sfx == DifficultySfxKind::None);
    REQUIRE(state_eq(state, before));
}

// ---------------------------------------------------------------- RESIZE

TEST_CASE("difficulty_screen_step: WINDOW_RESIZED/PIXEL_SIZE_CHANGED viram "
          "resize=true e sao NO-OP DE ESTADO (a tela nao muda foco/splash) - o "
          "CHAMADOR e quem reposiciona a UiLayer",
          "[difficulty_screen_step][f4-1b]") {
    DifficultyMenuState state;
    difficulty_menu_open(state);
    state.selected = static_cast<int>(DifficultyMenuItem::Dificil);
    const DifficultyMenuState before = state;

    const auto boxes = no_boxes();

    SDL_Event resized{};
    resized.type = SDL_EVENT_WINDOW_RESIZED;
    const DifficultyStepResult r1 = difficulty_screen_step(state, resized, boxes.data());
    REQUIRE(r1.resize);
    REQUIRE_FALSE(r1.window_closed);
    REQUIRE_FALSE(r1.reload);
    REQUIRE_FALSE(r1.exit.has_value());
    REQUIRE(r1.sfx == DifficultySfxKind::None);
    REQUIRE(state_eq(state, before));

    SDL_Event pixel_changed{};
    pixel_changed.type = SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED;
    const DifficultyStepResult r2 =
        difficulty_screen_step(state, pixel_changed, boxes.data());
    REQUIRE(r2.resize);
    REQUIRE(state_eq(state, before));
}

// ---------------------------------------------------------------- confirm no Hardcore bloqueado

TEST_CASE("difficulty_screen_step: Enter/Espaco com o foco no Hardcore "
          "BLOQUEADO toca o SFX de bloqueio e deixa o ESTADO imutavel (no-op "
          "TOTAL de difficulty_menu_key_down, so o reload flag liga - MESMO "
          "comportamento do route_action antigo)",
          "[difficulty_screen_step][f4-1b]") {
    DifficultyMenuState state;
    difficulty_menu_open(state);  // hardcore_unlocked=false (Fase 0)
    state.selected = static_cast<int>(DifficultyMenuItem::Hardcore);
    REQUIRE_FALSE(state.confirming);
    const DifficultyMenuState before = state;

    const auto boxes = no_boxes();
    const SDL_Event ev = key_down_event(SDLK_RETURN);
    const DifficultyStepResult result = difficulty_screen_step(state, ev, boxes.data());

    REQUIRE(result.sfx == DifficultySfxKind::Blocked);
    REQUIRE(result.reload);  // route_action antigo recarrega mesmo em no-op
    REQUIRE_FALSE(result.exit.has_value());
    REQUIRE_FALSE(result.window_closed);
    REQUIRE(state_eq(state, before));  // ESTADO IMUTAVEL - nao abriu o splash

    // Espaco tem o MESMO comportamento (2o canal de confirmacao).
    const SDL_Event ev_space = key_down_event(SDLK_SPACE);
    const DifficultyStepResult result2 =
        difficulty_screen_step(state, ev_space, boxes.data());
    REQUIRE(result2.sfx == DifficultySfxKind::Blocked);
    REQUIRE(state_eq(state, before));
}

TEST_CASE("difficulty_screen_step: navegar (seta Baixo) ATE o Hardcore "
          "BLOQUEADO toca o MESMO SFX de bloqueio (paridade teclado x clique) - "
          "so o FOCO muda (reload=true), a selecao continua imutavel ate "
          "confirmar",
          "[difficulty_screen_step][f4-1b]") {
    DifficultyMenuState state;
    difficulty_menu_open(state);
    state.selected = static_cast<int>(DifficultyMenuItem::Dificil);  // 2 -> 3 (Hardcore)

    const auto boxes = no_boxes();
    const SDL_Event ev = key_down_event(SDLK_DOWN);
    const DifficultyStepResult result = difficulty_screen_step(state, ev, boxes.data());

    REQUIRE(state.selected == static_cast<int>(DifficultyMenuItem::Hardcore));
    REQUIRE(result.sfx == DifficultySfxKind::Blocked);
    REQUIRE(result.reload);
    REQUIRE_FALSE(result.exit.has_value());
}

TEST_CASE("difficulty_screen_step: navegar (seta Baixo) entre 2 itens "
          "SELECIONAVEIS toca o SFX de HOVER normal (nao Blocked)",
          "[difficulty_screen_step][f4-1b]") {
    DifficultyMenuState state;
    difficulty_menu_open(state);
    state.selected = static_cast<int>(DifficultyMenuItem::Facil);  // 0 -> 1 (Medio)

    const auto boxes = no_boxes();
    const SDL_Event ev = key_down_event(SDLK_DOWN);
    const DifficultyStepResult result = difficulty_screen_step(state, ev, boxes.data());

    REQUIRE(state.selected == static_cast<int>(DifficultyMenuItem::Medio));
    REQUIRE(result.sfx == DifficultySfxKind::Hover);
    REQUIRE(result.reload);
}

// ---------------------------------------------------------------- ESC na lista

TEST_CASE("difficulty_screen_step: ESC fora do splash devolve exit=Cancelled",
          "[difficulty_screen_step][f4-1b]") {
    DifficultyMenuState state;
    difficulty_menu_open(state);

    const auto boxes = no_boxes();
    const SDL_Event ev = key_down_event(SDLK_ESCAPE);
    const DifficultyStepResult result = difficulty_screen_step(state, ev, boxes.data());

    REQUIRE(result.exit.has_value());
    REQUIRE(*result.exit == DifficultyLoopExit::Cancelled);
    REQUIRE_FALSE(result.reload);  // Cancelled nao passa por reload - o
                                   // CHAMADOR retorna na hora (route_action antigo)
}

// ---------------------------------------------------------------- clique DENTRO vs FORA

TEST_CASE("difficulty_screen_step: clique DENTRO da box de um item "
          "SELECIONAVEL (Facil) toca Click, abre o splash (reload=true, "
          "confirming vira true) - clique FORA de qualquer box e NO-OP TOTAL",
          "[difficulty_screen_step][f4-1b]") {
    DifficultyMenuState state;
    difficulty_menu_open(state);

    std::array<glintfx::ElementBox, kDifficultyItemCount> boxes{};
    boxes[static_cast<std::size_t>(DifficultyMenuItem::Facil)] =
        make_box(/*x=*/10.0f, /*y=*/10.0f, /*w=*/100.0f, /*h=*/40.0f);
    // demais boxes ficam found=false (nao resolvidas neste frame sintetico).

    SECTION("clique DENTRO da box") {
        const SDL_Event ev = mouse_button_down_event(30.0f, 25.0f);  // dentro
        const DifficultyStepResult result = difficulty_screen_step(state, ev, boxes.data());

        REQUIRE(result.sfx == DifficultySfxKind::Click);
        REQUIRE(result.reload);
        REQUIRE_FALSE(result.exit.has_value());
        REQUIRE(state.selected == static_cast<int>(DifficultyMenuItem::Facil));
        REQUIRE(state.confirming);  // abriu o Aviso #2
    }

    SECTION("clique FORA de qualquer box (canto vazio, nenhuma box cobre)") {
        const DifficultyMenuState before = state;
        const SDL_Event ev = mouse_button_down_event(500.0f, 500.0f);  // fora
        const DifficultyStepResult result = difficulty_screen_step(state, ev, boxes.data());

        REQUIRE(result.sfx == DifficultySfxKind::None);
        REQUIRE_FALSE(result.reload);
        REQUIRE_FALSE(result.exit.has_value());
        REQUIRE(state_eq(state, before));  // no-op TOTAL - nem o foco muda
    }
}

TEST_CASE("difficulty_screen_step: clique DENTRO do card Hardcore BLOQUEADO "
          "toca o SFX de bloqueio e permanece NO-OP DE ESTADO (nao abre o "
          "splash)",
          "[difficulty_screen_step][f4-1b]") {
    DifficultyMenuState state;
    difficulty_menu_open(state);  // hardcore_unlocked=false
    const DifficultyMenuState before = state;

    std::array<glintfx::ElementBox, kDifficultyItemCount> boxes{};
    boxes[static_cast<std::size_t>(DifficultyMenuItem::Hardcore)] =
        make_box(10.0f, 200.0f, 100.0f, 40.0f);

    const SDL_Event ev = mouse_button_down_event(50.0f, 220.0f);  // dentro do card
    const DifficultyStepResult result = difficulty_screen_step(state, ev, boxes.data());

    REQUIRE(result.sfx == DifficultySfxKind::Blocked);
    REQUIRE(result.reload);
    REQUIRE_FALSE(result.exit.has_value());
    REQUIRE(state_eq(state, before));  // ESTADO IMUTAVEL - MESMA nuance do teclado
}

TEST_CASE("difficulty_screen_step: dentro do splash, clique na pill 0 "
          "(Confirmar) devolve exit=Chosen; clique na pill 1 (Cancelar) fecha "
          "o splash (reload=true, confirming volta a false)",
          "[difficulty_screen_step][f4-1b]") {
    DifficultyMenuState state;
    difficulty_menu_open(state);
    state.selected = static_cast<int>(DifficultyMenuItem::Medio);
    state.confirming = true;
    state.confirm_selected = 1;

    std::array<glintfx::ElementBox, kDifficultyItemCount> boxes{};
    boxes[0] = make_box(0.0f, 0.0f, 50.0f, 50.0f);    // pill "Confirmar"
    boxes[1] = make_box(100.0f, 0.0f, 50.0f, 50.0f);  // pill "Cancelar"

    SECTION("Confirmar") {
        const SDL_Event ev = mouse_button_down_event(10.0f, 10.0f);
        const DifficultyStepResult result = difficulty_screen_step(state, ev, boxes.data());
        REQUIRE(result.sfx == DifficultySfxKind::Click);
        REQUIRE(result.exit.has_value());
        REQUIRE(*result.exit == DifficultyLoopExit::Chosen);
    }

    SECTION("Cancelar") {
        const SDL_Event ev = mouse_button_down_event(110.0f, 10.0f);
        const DifficultyStepResult result = difficulty_screen_step(state, ev, boxes.data());
        REQUIRE(result.sfx == DifficultySfxKind::Click);
        REQUIRE(result.reload);
        REQUIRE_FALSE(result.exit.has_value());
        REQUIRE_FALSE(state.confirming);
    }
}

// ---------------------------------------------------------------- MOUSE_MOTION

TEST_CASE("difficulty_screen_step: MOUSE_MOTION vira mouse_move=true com "
          "(x,y) repassados - o CHAMADOR encaminha pro glintfx::UiEvent::"
          "MouseMove (hover NATIVO), sem tocar estado/sfx/reload/exit aqui",
          "[difficulty_screen_step][f4-1b]") {
    DifficultyMenuState state;
    difficulty_menu_open(state);
    const DifficultyMenuState before = state;

    SDL_Event ev{};
    ev.type = SDL_EVENT_MOUSE_MOTION;
    ev.motion.x = 42.0f;
    ev.motion.y = 84.0f;
    const auto boxes = no_boxes();
    const DifficultyStepResult result = difficulty_screen_step(state, ev, boxes.data());

    REQUIRE(result.mouse_move);
    REQUIRE(result.mouse_x == 42.0f);
    REQUIRE(result.mouse_y == 84.0f);
    REQUIRE_FALSE(result.reload);
    REQUIRE(result.sfx == DifficultySfxKind::None);
    REQUIRE_FALSE(result.exit.has_value());
    REQUIRE(state_eq(state, before));
}

// ---------------------------------------------------------------- evento nao roteado

TEST_CASE("difficulty_screen_step: tipo de evento que esta tela nao roteia "
          "(ex.: TEXT_INPUT) e NO-OP TOTAL (resultado default)",
          "[difficulty_screen_step][f4-1b]") {
    DifficultyMenuState state;
    difficulty_menu_open(state);
    const DifficultyMenuState before = state;

    SDL_Event ev{};
    ev.type = SDL_EVENT_TEXT_INPUT;
    const auto boxes = no_boxes();
    const DifficultyStepResult result = difficulty_screen_step(state, ev, boxes.data());

    REQUIRE_FALSE(result.window_closed);
    REQUIRE_FALSE(result.resize);
    REQUIRE_FALSE(result.mouse_move);
    REQUIRE_FALSE(result.reload);
    REQUIRE(result.sfx == DifficultySfxKind::None);
    REQUIRE_FALSE(result.exit.has_value());
    REQUIRE(state_eq(state, before));
}

// ---------------------------------------------------------------- repeat de tecla

TEST_CASE("difficulty_screen_step: SDL_EVENT_KEY_DOWN com key.repeat!=0 "
          "(auto-repeat do SO) e NO-OP TOTAL - so a 1a borda (repeat=0) e "
          "roteada, MESMO guard do while(true) antigo",
          "[difficulty_screen_step][f4-1b]") {
    DifficultyMenuState state;
    difficulty_menu_open(state);
    const DifficultyMenuState before = state;

    SDL_Event ev{};
    ev.type = SDL_EVENT_KEY_DOWN;
    ev.key.key = SDLK_DOWN;
    ev.key.repeat = 1;  // auto-repeat - NAO deve navegar
    const auto boxes = no_boxes();
    const DifficultyStepResult result = difficulty_screen_step(state, ev, boxes.data());

    REQUIRE_FALSE(result.reload);
    REQUIRE(result.sfx == DifficultySfxKind::None);
    REQUIRE_FALSE(result.exit.has_value());
    REQUIRE(state_eq(state, before));
}

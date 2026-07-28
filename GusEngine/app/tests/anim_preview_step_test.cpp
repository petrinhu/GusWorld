// SPDX-License-Identifier: GPL-3.0-or-later
// GusEngine/app/tests/anim_preview_step_test.cpp
//
// Catch2 (headless, SEM SDL_Init/janela/IRenderer/AnimClock real) de
// gus::app::screens::anim_preview_step (F4-1b.6, onda F4 "casca SDL -> App
// mode do glintfx", fatia 1b.6 - ferramenta INTERNA de dev --anim-preview, a
// MAIS SIMPLES da onda). A funcao e PURA o suficiente pra ser exercitada com
// SDL_Event construidos a mao (MESMA tecnica de difficulty_screen_step_test.cpp)
// - ela NUNCA carrega frame nem toca AnimClock, so calcula a DECISAO a partir
// de (anim_idx, catalog_size). O CONSUMIDOR real (AnimPreviewScreen, GPU-heavy)
// continua sem teste direto - ver o topo de
// gus/app/src/screens/anim_preview.cpp.

#include <catch2/catch_test_macros.hpp>

#include <SDL3/SDL.h>

#include "gus/app/screens/anim_preview.hpp"

using gus::app::screens::anim_preview_step;
using gus::app::screens::AnimPreviewState;
using gus::app::screens::AnimPreviewStepResult;

namespace {

SDL_Event key_down_event(SDL_Keycode key) {
    SDL_Event ev{};
    ev.type = SDL_EVENT_KEY_DOWN;
    ev.key.key = key;
    return ev;
}

}  // namespace

// ---------------------------------------------------------------- QUIT

TEST_CASE("anim_preview_step: SDL_EVENT_QUIT vira window_closed=true, sem "
          "tocar finished/switch_anim/fps_delta",
          "[anim_preview_step][f4-1b6]") {
    const AnimPreviewState state{/*anim_idx=*/2, /*catalog_size=*/5};
    SDL_Event ev{};
    ev.type = SDL_EVENT_QUIT;

    const AnimPreviewStepResult result = anim_preview_step(state, ev);

    REQUIRE(result.window_closed);
    REQUIRE_FALSE(result.finished);
    REQUIRE_FALSE(result.switch_anim);
    REQUIRE(result.fps_delta == 0.0f);
}

// ---------------------------------------------------------------- Esc

TEST_CASE("anim_preview_step: Esc vira finished=true, sem window_closed/"
          "switch_anim/fps_delta",
          "[anim_preview_step][f4-1b6]") {
    const AnimPreviewState state{/*anim_idx=*/0, /*catalog_size=*/3};
    const SDL_Event ev = key_down_event(SDLK_ESCAPE);

    const AnimPreviewStepResult result = anim_preview_step(state, ev);

    REQUIRE(result.finished);
    REQUIRE_FALSE(result.window_closed);
    REQUIRE_FALSE(result.switch_anim);
    REQUIRE(result.fps_delta == 0.0f);
}

// ---------------------------------------------------------------- WRAP: Tab/Direita no ULTIMO indice -> 0

TEST_CASE("anim_preview_step: Tab no ULTIMO indice do catalogo faz WRAP pra 0 "
          "(mutante classico: quebrar o modulo do wrap)",
          "[anim_preview_step][f4-1b6]") {
    const AnimPreviewState state{/*anim_idx=*/4, /*catalog_size=*/5};  // ultimo (4) de 5
    const SDL_Event ev = key_down_event(SDLK_TAB);

    const AnimPreviewStepResult result = anim_preview_step(state, ev);

    REQUIRE(result.switch_anim);
    REQUIRE(result.new_anim_idx == 0);
    REQUIRE_FALSE(result.window_closed);
    REQUIRE_FALSE(result.finished);
}

TEST_CASE("anim_preview_step: seta-direita tem o MESMO comportamento de Tab "
          "(proxima anim, com wrap)",
          "[anim_preview_step][f4-1b6]") {
    const AnimPreviewState state{/*anim_idx=*/4, /*catalog_size=*/5};
    const SDL_Event ev = key_down_event(SDLK_RIGHT);

    const AnimPreviewStepResult result = anim_preview_step(state, ev);

    REQUIRE(result.switch_anim);
    REQUIRE(result.new_anim_idx == 0);
}

TEST_CASE("anim_preview_step: Tab no MEIO do catalogo so avanca 1 (sem wrap "
          "espurio)",
          "[anim_preview_step][f4-1b6]") {
    const AnimPreviewState state{/*anim_idx=*/1, /*catalog_size=*/5};
    const SDL_Event ev = key_down_event(SDLK_TAB);

    const AnimPreviewStepResult result = anim_preview_step(state, ev);

    REQUIRE(result.switch_anim);
    REQUIRE(result.new_anim_idx == 2);
}

// ---------------------------------------------------------------- WRAP: seta-esquerda no PRIMEIRO indice -> ultimo

TEST_CASE("anim_preview_step: seta-esquerda no PRIMEIRO indice (0) do "
          "catalogo faz WRAP pro ULTIMO (mutante classico: quebrar o modulo "
          "do wrap no sentido negativo)",
          "[anim_preview_step][f4-1b6]") {
    const AnimPreviewState state{/*anim_idx=*/0, /*catalog_size=*/5};
    const SDL_Event ev = key_down_event(SDLK_LEFT);

    const AnimPreviewStepResult result = anim_preview_step(state, ev);

    REQUIRE(result.switch_anim);
    REQUIRE(result.new_anim_idx == 4);
    REQUIRE_FALSE(result.window_closed);
    REQUIRE_FALSE(result.finished);
}

TEST_CASE("anim_preview_step: seta-esquerda no MEIO do catalogo so retrocede "
          "1 (sem wrap espurio)",
          "[anim_preview_step][f4-1b6]") {
    const AnimPreviewState state{/*anim_idx=*/3, /*catalog_size=*/5};
    const SDL_Event ev = key_down_event(SDLK_LEFT);

    const AnimPreviewStepResult result = anim_preview_step(state, ev);

    REQUIRE(result.switch_anim);
    REQUIRE(result.new_anim_idx == 2);
}

// ---------------------------------------------------------------- nudge_fps

TEST_CASE("anim_preview_step: seta-cima devolve fps_delta=+1.0f, sem "
          "switch_anim/finished/window_closed",
          "[anim_preview_step][f4-1b6]") {
    const AnimPreviewState state{/*anim_idx=*/1, /*catalog_size=*/3};
    const SDL_Event ev = key_down_event(SDLK_UP);

    const AnimPreviewStepResult result = anim_preview_step(state, ev);

    REQUIRE(result.fps_delta == 1.0f);
    REQUIRE_FALSE(result.switch_anim);
    REQUIRE_FALSE(result.finished);
    REQUIRE_FALSE(result.window_closed);
}

TEST_CASE("anim_preview_step: seta-baixo devolve fps_delta=-1.0f",
          "[anim_preview_step][f4-1b6]") {
    const AnimPreviewState state{/*anim_idx=*/1, /*catalog_size=*/3};
    const SDL_Event ev = key_down_event(SDLK_DOWN);

    const AnimPreviewStepResult result = anim_preview_step(state, ev);

    REQUIRE(result.fps_delta == -1.0f);
    REQUIRE_FALSE(result.switch_anim);
}

// ---------------------------------------------------------------- evento nao roteado

TEST_CASE("anim_preview_step: tipo de evento que este viewer nao roteia (ex.: "
          "MOUSE_MOTION) e NO-OP TOTAL (resultado default)",
          "[anim_preview_step][f4-1b6]") {
    const AnimPreviewState state{/*anim_idx=*/2, /*catalog_size=*/5};
    SDL_Event ev{};
    ev.type = SDL_EVENT_MOUSE_MOTION;

    const AnimPreviewStepResult result = anim_preview_step(state, ev);

    REQUIRE_FALSE(result.window_closed);
    REQUIRE_FALSE(result.finished);
    REQUIRE_FALSE(result.switch_anim);
    REQUIRE(result.fps_delta == 0.0f);
}

TEST_CASE("anim_preview_step: tecla nao roteada (ex.: SPACE) dentro de "
          "KEY_DOWN tambem e NO-OP TOTAL",
          "[anim_preview_step][f4-1b6]") {
    const AnimPreviewState state{/*anim_idx=*/2, /*catalog_size=*/5};
    const SDL_Event ev = key_down_event(SDLK_SPACE);

    const AnimPreviewStepResult result = anim_preview_step(state, ev);

    REQUIRE_FALSE(result.window_closed);
    REQUIRE_FALSE(result.finished);
    REQUIRE_FALSE(result.switch_anim);
    REQUIRE(result.fps_delta == 0.0f);
}

// GusEngine/app/tests/battle_screen_step_test.cpp
//
// Catch2 HEADLESS (SEM SDL_Init/janela/glintfx::UiLayer/BattleScene real) da FSM DE FASE
// PURA do host da batalha (F4-1b.5, onda F4 "casca SDL -> App mode do glintfx", a
// conversao MAIS ARRISCADA - battle_preview.cpp, gus::app::screens::BattleScreen):
//   battle_phase_initial/battle_phase_next - as transicoes FadeIn -> Main -> FadeOut ->
//     Done, incluindo "quit pula o resto" de QUALQUER fase corrente (a mesma semantica
//     do FIX BUG-3 do while(true) antigo: fechar a janela e um sinal DISTINTO de
//     qualquer CombatOutcome, e pula tanto o loop principal quanto o fade de saida).
//   battle_screen_should_close_on_event - a decisao PURA de fechamento de janela,
//     reusada pelas 3 fases (FadeIn/FadeOut so ouvem isto; Main ouve tudo, mas trata
//     QUIT da MESMA forma) - prova "SDL_EVENT_QUIT -> window_closed" (obrigatorio) e
//     "durante fade, evento nao-QUIT e ignorado" (a funcao so reage a QUIT, nenhum
//     outro tipo de evento tem efeito nela).
//
// ROTEAMENTO DE TECLA DA FASE MAIN: a fase Main NAO reimplementa o roteamento de
// teclado - ela DELEGA pra battle_key_down (extraido em battle_input.cpp/AC-E11 A1, ja
// testado exaustivamente em battle_key_routing_test.cpp - Esc desempilha 1 nivel de
// modal por vez: mira -> preview de ator -> lista -> pilha vazia sai do viewer). Este
// arquivo REUSA a MESMA tecnica (BattleScene real + battle_key_down direto, SEM
// SDL_Init/janela) pra provar que o roteamento que BattleScreen::handle_event_main_ usa
// e o MESMO ja provado la - decisao DELIBERADA de nao duplicar a suite inteira aqui (a
// fase Main mistura efeitos reais - audio/glintfx - que so o harness GL interativo,
// battle_preview_interaction_test.cpp, consegue exercitar ponta-a-ponta).
//
// Cross-ref: gus/app/screens/battle_preview.hpp (BattlePhase/battle_phase_initial/
//            battle_phase_next/battle_screen_should_close_on_event, declaradas la pro
//            teste incluir - mesmo espirito de difficulty_screen_step/title_screen_step);
//            battle_key_routing_test.cpp (a suite completa do roteamento de teclado, NAO
//            duplicada aqui); battle_preview_interaction_test.cpp (harness GL real, QUIT
//            ponta-a-ponta via run_battle_preview_embedded_gl_current).

#include <catch2/catch_test_macros.hpp>

#include <SDL3/SDL.h>

#include "gus/app/screens/battle_preview.hpp"
#include "gus/app/screens/battle_scene.hpp"

using gus::app::screens::battle_key_down;
using gus::app::screens::battle_phase_initial;
using gus::app::screens::battle_phase_next;
using gus::app::screens::battle_screen_should_close_on_event;
using gus::app::screens::BattlePhase;
using gus::app::screens::BattleScene;

// ---------------------------------------------------------------- battle_phase_initial

TEST_CASE("battle_phase_initial: fade_in pedido (Maestro) comeca em FadeIn",
          "[battle_screen_step]") {
    REQUIRE(battle_phase_initial(/*fade_in_enabled=*/true) == BattlePhase::FadeIn);
}

TEST_CASE("battle_phase_initial: SEM fade_in (--battle standalone + todo self-test/"
          "captura, que pedem fade_in_seconds<=0) comeca DIRETO em Main",
          "[battle_screen_step]") {
    REQUIRE(battle_phase_initial(/*fade_in_enabled=*/false) == BattlePhase::Main);
}

// ---------------------------------------------------------------- battle_phase_next: fluxo feliz

TEST_CASE("battle_phase_next: FadeIn -> Main quando o fade termina (elapsed>=duration)",
          "[battle_screen_step]") {
    REQUIRE(battle_phase_next(BattlePhase::FadeIn, /*quit=*/false, /*phase_done=*/true,
                              /*fade_out_enabled=*/true) == BattlePhase::Main);
}

TEST_CASE("battle_phase_next: FadeIn permanece FadeIn enquanto o fade NAO terminou",
          "[battle_screen_step]") {
    REQUIRE(battle_phase_next(BattlePhase::FadeIn, /*quit=*/false, /*phase_done=*/false,
                              /*fade_out_enabled=*/true) == BattlePhase::FadeIn);
}

TEST_CASE("battle_phase_next: Main -> FadeOut quando o combate termina E fade_out foi "
          "pedido (Maestro)",
          "[battle_screen_step]") {
    REQUIRE(battle_phase_next(BattlePhase::Main, /*quit=*/false, /*phase_done=*/true,
                              /*fade_out_enabled=*/true) == BattlePhase::FadeOut);
}

TEST_CASE("battle_phase_next: Main -> Done DIRETO quando o combate termina e fade_out "
          "NAO foi pedido (--battle standalone/self-tests, fade_out_seconds<=0)",
          "[battle_screen_step]") {
    REQUIRE(battle_phase_next(BattlePhase::Main, /*quit=*/false, /*phase_done=*/true,
                              /*fade_out_enabled=*/false) == BattlePhase::Done);
}

TEST_CASE("battle_phase_next: Main permanece Main enquanto o combate NAO terminou "
          "(running interno ainda true)",
          "[battle_screen_step]") {
    REQUIRE(battle_phase_next(BattlePhase::Main, /*quit=*/false, /*phase_done=*/false,
                              /*fade_out_enabled=*/true) == BattlePhase::Main);
}

TEST_CASE("battle_phase_next: FadeOut -> Done quando o fade de saida termina",
          "[battle_screen_step]") {
    REQUIRE(battle_phase_next(BattlePhase::FadeOut, /*quit=*/false, /*phase_done=*/true,
                              /*fade_out_enabled=*/true) == BattlePhase::Done);
}

TEST_CASE("battle_phase_next: FadeOut permanece FadeOut enquanto o fade NAO terminou",
          "[battle_screen_step]") {
    REQUIRE(battle_phase_next(BattlePhase::FadeOut, /*quit=*/false, /*phase_done=*/false,
                              /*fade_out_enabled=*/true) == BattlePhase::FadeOut);
}

TEST_CASE("battle_phase_next: Done e terminal (nunca sai de Done)", "[battle_screen_step]") {
    REQUIRE(battle_phase_next(BattlePhase::Done, /*quit=*/false, /*phase_done=*/true,
                              /*fade_out_enabled=*/true) == BattlePhase::Done);
    REQUIRE(battle_phase_next(BattlePhase::Done, /*quit=*/false, /*phase_done=*/false,
                              /*fade_out_enabled=*/true) == BattlePhase::Done);
}

// ---------------------------------------------------------------- battle_phase_next: quit pula o resto

TEST_CASE("battle_phase_next: QUIT durante FadeIn vai DIRETO pra Done - pula Main E "
          "FadeOut inteiros (FIX BUG-3: fechar a janela e IMEDIATO, nunca segura o "
          "jogador pro loop principal nem pro fade de saida que ele nao pediu)",
          "[battle_screen_step]") {
    // phase_done=false (o fade NAO tinha terminado sozinho) - quit=true MESMO ASSIM
    // forca Done na hora, independente do fade_out ter sido pedido ou nao.
    REQUIRE(battle_phase_next(BattlePhase::FadeIn, /*quit=*/true, /*phase_done=*/false,
                              /*fade_out_enabled=*/true) == BattlePhase::Done);
    REQUIRE(battle_phase_next(BattlePhase::FadeIn, /*quit=*/true, /*phase_done=*/false,
                              /*fade_out_enabled=*/false) == BattlePhase::Done);
}

TEST_CASE("battle_phase_next: QUIT durante Main vai DIRETO pra Done - pula o FadeOut "
          "(o fade de saida antigo era PULADO quando o motivo da saida era fechar a "
          "janela, `!quit_requested` no while(true) antigo)",
          "[battle_screen_step]") {
    REQUIRE(battle_phase_next(BattlePhase::Main, /*quit=*/true, /*phase_done=*/false,
                              /*fade_out_enabled=*/true) == BattlePhase::Done);
    // Mesmo se o combate TAMBEM tivesse terminado nesse exato tick (phase_done=true),
    // quit ainda manda - Done direto, nunca FadeOut.
    REQUIRE(battle_phase_next(BattlePhase::Main, /*quit=*/true, /*phase_done=*/true,
                              /*fade_out_enabled=*/true) == BattlePhase::Done);
}

TEST_CASE("battle_phase_next: QUIT durante FadeOut so encerra o proprio fade MAIS "
          "CEDO (ja e a ultima fase - Done de qualquer jeito, mas SEM esperar o fade "
          "terminar sozinho)",
          "[battle_screen_step]") {
    REQUIRE(battle_phase_next(BattlePhase::FadeOut, /*quit=*/true, /*phase_done=*/false,
                              /*fade_out_enabled=*/true) == BattlePhase::Done);
}

// ---------------------------------------------------------------- battle_screen_should_close_on_event

TEST_CASE("battle_screen_should_close_on_event: SDL_EVENT_QUIT -> window_closed "
          "(obrigatorio, em QUALQUER fase - a mesma funcao e reusada por FadeIn/Main/"
          "FadeOut)",
          "[battle_screen_step]") {
    SDL_Event ev{};
    ev.type = SDL_EVENT_QUIT;
    REQUIRE(battle_screen_should_close_on_event(ev));
}

TEST_CASE("battle_screen_should_close_on_event: durante fade, evento NAO-QUIT e "
          "ignorado (teclado/mouse mudos enquanto a tela clareia/escurece - a funcao "
          "so reage a SDL_EVENT_QUIT, nenhum outro tipo fecha a janela)",
          "[battle_screen_step]") {
    SDL_Event key_ev{};
    key_ev.type = SDL_EVENT_KEY_DOWN;
    key_ev.key.key = SDLK_ESCAPE;
    REQUIRE_FALSE(battle_screen_should_close_on_event(key_ev));

    SDL_Event motion_ev{};
    motion_ev.type = SDL_EVENT_MOUSE_MOTION;
    REQUIRE_FALSE(battle_screen_should_close_on_event(motion_ev));

    SDL_Event click_ev{};
    click_ev.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
    click_ev.button.button = SDL_BUTTON_LEFT;
    REQUIRE_FALSE(battle_screen_should_close_on_event(click_ev));

    SDL_Event resize_ev{};
    resize_ev.type = SDL_EVENT_WINDOW_RESIZED;
    REQUIRE_FALSE(battle_screen_should_close_on_event(resize_ev));
}

// ---------------------------------------------------------------- roteamento de tecla da fase Main
// (reusa o padrao de battle_key_routing_test.cpp - a fase Main de BattleScreen DELEGA
// pra battle_key_down, nao reimplementa o roteamento; ver o comentario grande no topo)

TEST_CASE("fase Main: Esc na pilha VAZIA sai do viewer - MESMO battle_key_down que "
          "BattleScreen::handle_event_main_ chama (out_effect implicito nullptr, "
          "REVERT BATTLE-ESC-PAUSE-ACTOR-LIST)",
          "[battle_screen_step]") {
    BattleScene scene;
    bool running = true;
    battle_key_down(scene, SDLK_ESCAPE, running);
    // running vira false: e o MESMO sinal que tick_main_ le pra decidir phase_done=true
    // (running_ interno da BattleScreen, ver battle_phase_next acima).
    REQUIRE_FALSE(running);
}

TEST_CASE("fase Main: SDL_EVENT_QUIT (via battle_screen_should_close_on_event) e o "
          "MESMO sinal, INDEPENDENTE do estado do roteamento de tecla - nao precisa "
          "estar na pilha vazia pra fechar a janela",
          "[battle_screen_step]") {
    // Fora de qualquer modal (pilha vazia) OU dentro de um (mira/preview/lista): o
    // QUIT fecha igual, sem passar por battle_key_down - BattleScreen::
    // handle_event_main_ checa QUIT ANTES do ramo SDL_EVENT_KEY_DOWN.
    SDL_Event quit_ev{};
    quit_ev.type = SDL_EVENT_QUIT;
    REQUIRE(battle_screen_should_close_on_event(quit_ev));

    BattleScene scene;  // scene nem precisa avancar - a decisao de fechar independe dela
    (void)scene;
}

// SPDX-License-Identifier: GPL-3.0-or-later
// GusEngine/app/tests/battle_preview_interaction_test.cpp
//
// Catch2 HEADLESS (GL REAL - Xvfb :99, MESMA receita de difficulty_menu_loop_
// interaction_test.cpp/save_load_menu_interaction_test.cpp) do HOST REAL da batalha
// (F4-1b.5, gus::app::screens::run_battle_preview_embedded_gl_current) - o FIO
// INTEIRO ate a funcao de producao que a Maestro chama, provando o wiring de saida
// (SDL_EVENT_QUIT real fecha a janela, o MESMO sinal que battle_screen_step_test.cpp
// prova puro em battle_screen_should_close_on_event).
//
// SETUP PESADO (avisado no brief da fatia): o enter() da BattleScreen monta a cena de
// demo inteira (party+inimigos), carrega retratos/sprites/icones/traducao e liga o
// glintfx::UiLayer do cockpit - MUITO mais custoso que o das telas de menu (Difficulty/
// SaveLoad/Title). Este teste prova o MINIMO que garante o wiring de saida ponta-a-
// ponta (QUIT real -> *out_quit_requested==true) SEM depender de nenhum asset externo
// (fade_in/fade_out=0.0f, ver os defaults do header - o teste nao exercita as fases de
// fade, ja cobertas puras em battle_screen_step_test.cpp) - degrada seguro (0
// assercoes) se GL/display estiver indisponivel, MESMO espirito dos harnesses irmaos.
//
// Cross-ref: gus/app/screens/battle_preview.hpp (a funcao sob teste);
//            battle_screen_step_test.cpp (a FSM de fase PURA, sem GL/janela);
//            difficulty_menu_loop_interaction_test.cpp (o harness GlTestEnv/try_boot_gl
//            do qual este e uma variante, copiado AUTO-CONTIDO por convencao da suite).

#include <catch2/catch_test_macros.hpp>

#include <SDL3/SDL.h>

#include "gus/app/screens/battle_preview.hpp"
#include "gus/domain/combat/combat_enums.hpp"
#include "gus/platform/rmlui/gl3_loader.hpp"

namespace {

// MESMA receita de bootstrap GL de difficulty_menu_loop_interaction_test.cpp (GlTestEnv/
// try_boot_gl) - copiada aqui (arquivo AUTO-CONTIDO, MESMO nao-acoplamento entre
// harnesses ja estabelecido nas telas de producao). run_battle_preview_embedded_gl_
// current EXIGE que o glad ja esteja carregado (gl3_load_functions) - MESMA pre-
// condicao que run_battle_preview_embedded (o wrapper que cria o contexto de verdade)
// satisfaz antes de chamar o nucleo.
struct GlTestEnv {
    SDL_Window* window = nullptr;
    SDL_GLContext gl = nullptr;
    bool ok = false;

    ~GlTestEnv() {
        if (gl != nullptr) SDL_GL_DestroyContext(gl);
        if (window != nullptr) SDL_DestroyWindow(window);
    }
};

GlTestEnv try_boot_gl() {
    GlTestEnv env;
    if (!SDL_WasInit(SDL_INIT_VIDEO) && !SDL_InitSubSystem(SDL_INIT_VIDEO)) return env;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    env.window = SDL_CreateWindow("battle_preview_interaction_test", 960, 540,
                                   SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
    if (env.window == nullptr) return env;

    env.gl = SDL_GL_CreateContext(env.window);
    if (env.gl == nullptr) return env;
    SDL_GL_MakeCurrent(env.window, env.gl);

    if (!gus::platform::rmlui::gl3_load_functions(
            reinterpret_cast<void* (*)(const char*)>(SDL_GL_GetProcAddress))) {
        return env;
    }
    env.ok = true;
    return env;
}

}  // namespace

// ---------------------------------------------------------------- QUIT real (F4-1b.5)

TEST_CASE("battle_preview (harness headless): SDL_EVENT_QUIT real (SDL_PushEvent) "
          "fecha a janela DURANTE a batalha - run_battle_preview_embedded_gl_current "
          "grava *out_quit_requested=true (FIX BUG-3, o MESMO sinal que a Maestro usa "
          "pra NAO voltar a cidade)",
          "[battle_preview_interaction][gl]") {
    GlTestEnv env = try_boot_gl();
    if (!env.ok) {
        INFO("GL/display indisponivel neste ambiente (sem Xvfb) - harness pulado "
             "(degradacao segura, 0 assercoes). Rode com Xvfb :99 (export DISPLAY=:99) "
             "pra exercitar de fato.");
        return;
    }

    // QUIT ja na fila ANTES do 1o pump: o enter() da BattleScreen roda um setup pesado
    // (cena de demo + assets), mas nenhum self-test/fade esta ativo aqui
    // (fade_in_seconds/fade_out_seconds default 0.0f) - o 1o handle_event ja deve
    // consumir o QUIT e encerrar sem desenhar frame nenhum (MESMO guard `if (!running)
    // break;`/window_closed() do host).
    SDL_Event quit_ev{};
    quit_ev.type = SDL_EVENT_QUIT;
    REQUIRE(SDL_PushEvent(&quit_ev));

    gus::domain::combat::CombatOutcome outcome = gus::domain::combat::CombatOutcome::Ongoing;
    bool quit_requested = false;
    gus::app::screens::run_battle_preview_embedded_gl_current(
        env.window, &outcome, &quit_requested, /*external_audio=*/nullptr,
        /*fade_in_seconds=*/0.0f, /*fade_out_seconds=*/0.0f);

    // ANTES desta fatia (se a propagacao SDL_EVENT_QUIT -> BattleScreen::window_closed_
    // -> *out_quit_requested tivesse quebrado no meio da conversao pra ScreenState): o
    // teste TRAVARIA esperando o PROXIMO evento pra sempre (fila vazia apos o QUIT) em
    // vez de falhar rapido - MESMO padrao de deteccao ja documentado em
    // difficulty_menu_loop_interaction_test.cpp/title_menu_loop_interaction_test.cpp.
    REQUIRE(quit_requested);
    // Fechar a janela NAO e nenhum CombatOutcome (nao e vitoria/derrota/fuga) - a FSM
    // de combate nunca chegou a CombatEnd (a Maestro trata isto como "abortou", ver o
    // comentario grande em battle_preview.hpp).
    REQUIRE(outcome == gus::domain::combat::CombatOutcome::Ongoing);
}

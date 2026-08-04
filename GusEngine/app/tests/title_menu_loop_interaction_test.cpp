// SPDX-License-Identifier: Apache-2.0
// GusEngine/app/tests/title_menu_loop_interaction_test.cpp
//
// Catch2 HEADLESS (GL REAL - Xvfb :99, MESMA receita de difficulty_menu_loop_
// interaction_test.cpp/save_load_menu_loop_interaction_test.cpp) da tela de
// TITULO (F4-1b.3, onda F4 "casca SDL -> App mode do glintfx", fatia 1b.3 -
// PRIMEIRA tela PAI da onda, introduz o padrao do MINI-DRIVER, ver o
// comentario grande no topo de title_menu_loop.cpp).
//
// DEGRADACAO SEGURA: sem GL/display (sem Xvfb), cada TEST_CASE registra um
// INFO e retorna sem assercoes (0 assertions, Catch2 conta "passou") - MESMO
// espirito dos harnesses analogos.
//
// Cross-ref: gus/app/screens/title_menu_loop.cpp (o codigo sob teste,
//            TitleScreen + o mini-driver run_title_menu_loop_gl_current);
//            title_screen_step_test.cpp (as funcoes PURAS title_screen_step/
//            title_flow_next, testadas headless SEM GL); difficulty_menu_
//            loop_interaction_test.cpp (harness GL/SDL_PushEvent do qual este
//            e uma variante).

#include <catch2/catch_test_macros.hpp>

#include <filesystem>

#include <SDL3/SDL.h>

#include "gus/app/i18n/translator.hpp"
#include "gus/app/screens/title_menu_loop.hpp"
#include "gus/domain/save/save_data.hpp"
#include "gus/platform/audio/audio_engine.hpp"
#include "gus/platform/fs/save_file_store.hpp"
#include "gus/platform/rmlui/gl3_loader.hpp"
#include "tmp_dir_test_support.hpp"

using namespace gus::app::screens;
using gus::platform::audio::AudioEngine;

namespace {

constexpr int kWinW = 960;
constexpr int kWinH = 540;

// MESMA receita de bootstrap GL de difficulty_menu_loop_interaction_test.cpp/
// save_load_menu_loop_interaction_test.cpp (GlTestEnv/try_boot_gl) - copiada
// aqui (arquivo AUTO-CONTIDO, MESMO nao-acoplamento entre harnesses ja
// estabelecido nas telas de producao).
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

    env.window = SDL_CreateWindow("title_menu_loop_interaction_test", kWinW, kWinH,
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

// Traducao MINIMA cobrindo AMBAS as telas envolvidas no fluxo aninhado (titulo
// + dificuldade, MESMAS chaves de title_menu_rml_test.cpp/difficulty_menu_
// loop_interaction_test.cpp) - o mini-driver desta fatia pode abrir as duas.
gus::app::i18n::Translator make_translator() {
    gus::app::i18n::Translator tr;
    tr.load_from_content(
        "## TITLE_LOGO_PREFIX\nGus\n\n"
        "## TITLE_LOGO_SUFFIX\nWorld\n\n"
        "## TITLE_SUBTITLE\nvertical slice\n\n"
        "## TITLE_FOOTER_HINT\nCima/Baixo navega - Enter seleciona\n\n"
        "## TITLE_NEW_GAME_CONFIRM\nComecar novo jogo?\n\n"
        "## TITLE_NEW_GAME_CONFIRM_YES\nSim, comecar\n\n"
        "## TITLE_NEW_GAME_CONFIRM_NO\nCancelar\n\n"
        "## MENU_CONTINUE\nContinuar\n\n"
        "## MENU_NEW_GAME\nNovo Jogo\n\n"
        "## MENU_QUIT\nSair\n\n"
        "## SAVE_DIFFICULTY_TITLE\nEscolha a dificuldade\n\n"
        "## SAVE_DIFFICULTY_HINT\nEssa escolha e definitiva\n\n"
        "## SAVE_DIFFICULTY_FACIL_LABEL\nFacil\n\n"
        "## SAVE_DIFFICULTY_FACIL_DESC\nVolta pro ultimo save\n\n"
        "## SAVE_DIFFICULTY_MEDIO_LABEL\nMedio\n\n"
        "## SAVE_DIFFICULTY_MEDIO_BADGE\nRecomendado\n\n"
        "## SAVE_DIFFICULTY_MEDIO_DESC\nAcorda no Hospital\n\n"
        "## SAVE_DIFFICULTY_DIFICIL_LABEL\nDificil\n\n"
        "## SAVE_DIFFICULTY_DIFICIL_DESC\nAcorda longe e fraco\n\n"
        "## SAVE_DIFFICULTY_HARDCORE_LABEL\nHardcore\n\n"
        "## SAVE_DIFFICULTY_HARDCORE_DESC_LOCKED\nAlgo sombrio aguarda\n\n"
        "## SAVE_DIFFICULTY_HARDCORE_DESC_UNLOCKED\nSo para os valorosos\n\n"
        "## SAVE_DIFFICULTY_CONFIRM_TITLE_FACIL\nJogar no Facil?\n\n"
        "## SAVE_DIFFICULTY_CONFIRM_TITLE_MEDIO\nJogar no Medio?\n\n"
        "## SAVE_DIFFICULTY_CONFIRM_TITLE_DIFICIL\nJogar no Dificil?\n\n"
        "## SAVE_DIFFICULTY_CONFIRM_BODY\nNao da pra trocar depois\n\n"
        "## SAVE_DIFFICULTY_CONFIRM_YES_FACIL\nSim, jogar no Facil\n\n"
        "## SAVE_DIFFICULTY_CONFIRM_YES_MEDIO\nSim, jogar no Medio\n\n"
        "## SAVE_DIFFICULTY_CONFIRM_YES_DIFICIL\nSim, jogar no Dificil\n\n"
        "## SAVE_DIFFICULTY_CONFIRM_NO\nCancelar\n\n"
        "## SAVE_DIFFICULTY_FOOTER_HINT\nCima/Baixo navega - Enter seleciona\n\n");
    return tr;
}

}  // namespace

// ---------------------------------------------------------------- QUIT real (F4-1b.3)

TEST_CASE("title_menu_loop (harness headless): SDL_EVENT_QUIT real "
          "(SDL_PushEvent) fecha a janela - run_title_menu_loop_gl_current "
          "devolve QuitApp (mutante analogo ao QA adversarial F4-1b.1/F4-1b.2)",
          "[title_menu_loop_interaction][gl]") {
    GlTestEnv env = try_boot_gl();
    if (!env.ok) {
        INFO("GL/display indisponivel neste ambiente (sem Xvfb) - harness pulado "
             "(degradacao segura, 0 assercoes). Rode com Xvfb :99 (export "
             "DISPLAY=:99) pra exercitar de fato.");
        return;
    }

    const gus::app::i18n::Translator translator = make_translator();

    const gus::test_support::ScopedTempDir saves_dir("gusworld_title_loop_interaction_quit_saves");

    SDL_Event quit_ev{};
    quit_ev.type = SDL_EVENT_QUIT;
    REQUIRE(SDL_PushEvent(&quit_ev));

    AudioEngine audio(/*device_active=*/false);  // sem hardware no CI
    TitleLoopExit exit = TitleLoopExit::ContinueGame;
    gus::domain::save::SaveData loaded{};
    run_title_menu_loop_gl_current(env.window, audio, translator, saves_dir.string(), &exit,
                                   &loaded);

    // ANTES do fix desta fatia (se a propagacao tivesse regredido): o loop
    // ficaria esperando o PROXIMO evento pra sempre (fila vazia apos o QUIT) -
    // o teste TRAVARIA em vez de falhar rapido (MESMO padrao de deteccao ja
    // documentado nos testes analogos).
    REQUIRE(exit == TitleLoopExit::QuitApp);
}

// ---------------------------------------------------------------- RE-ENTRADA (titulo -> dificuldade -> cancela -> volta)

// F4-1b.3 (regressao do mutante "recriar o estado no enter()" - o MINI-DRIVER
// desta fatia REUSA o MESMO objeto TitleScreen ao voltar de um Cancelled na
// tela de dificuldade; scan_saves()/title_menu_open() rodam UMA VEZ no
// CONSTRUTOR, NUNCA em enter(), ver o comentario grande em title_menu_loop.cpp):
// grava um save real (any_save_exists=true -> foco DEFAULT = Continuar,
// SELECIONAVEL), navega ate Novo Jogo, abre+confirma o mini-dialogo (Sim),
// cancela na tela de dificuldade (ESC na lista) e, JA DE VOLTA na tela de
// titulo, aperta Enter SEM navegar de novo. Se o estado tivesse sido
// RECRIADO no re-enter() (o mutante), o foco teria voltado pro DEFAULT
// (Continuar) e este Enter devolveria ContinueGame - o teste PROVA que isso
// NAO acontece (fecha via QUIT, nunca ContinueGame).
TEST_CASE("title_menu_loop (harness headless): fluxo titulo -> Novo Jogo -> "
          "dificuldade Cancelada -> volta ao titulo PRESERVANDO o foco/estado "
          "(mata o mutante 'recriar TitleMenuState no enter()')",
          "[title_menu_loop_interaction][gl]") {
    GlTestEnv env = try_boot_gl();
    if (!env.ok) {
        INFO("GL/display indisponivel neste ambiente (sem Xvfb) - harness pulado "
             "(degradacao segura, 0 assercoes). Rode com Xvfb :99 (export "
             "DISPLAY=:99) pra exercitar de fato.");
        return;
    }

    const gus::app::i18n::Translator translator = make_translator();

    const gus::test_support::ScopedTempDir saves_dir(
        "gusworld_title_loop_interaction_reentry_saves");

    // Grava um save REAL no slot Auto (0) - any_save_exists=true, Continuar
    // vira SELECIONAVEL e o foco DEFAULT (title_menu_open) recai nele - o
    // divergente exato que o mutante produziria se reaparecesse.
    REQUIRE(gus::platform::fs::save_game(gus::domain::save::SaveData{}, /*slot=*/0,
                                          saves_dir.string()));
    REQUIRE(gus::platform::fs::has_save(0, saves_dir.string()));

    // 1) DOWN: Continuar(0) -> Novo Jogo(1).
    SDL_Event down_ev{};
    down_ev.type = SDL_EVENT_KEY_DOWN;
    down_ev.key.key = SDLK_DOWN;
    down_ev.key.repeat = 0;
    REQUIRE(SDL_PushEvent(&down_ev));

    // 2) ENTER: abre o mini-dialogo "comecar novo jogo?" (any_save_exists=true).
    SDL_Event enter_open_ev{};
    enter_open_ev.type = SDL_EVENT_KEY_DOWN;
    enter_open_ev.key.key = SDLK_RETURN;
    enter_open_ev.key.repeat = 0;
    REQUIRE(SDL_PushEvent(&enter_open_ev));

    // 3) LEFT: move confirm_selected de 1 (Nao, default) pra 0 (Sim).
    SDL_Event left_ev{};
    left_ev.type = SDL_EVENT_KEY_DOWN;
    left_ev.key.key = SDLK_LEFT;
    left_ev.key.repeat = 0;
    REQUIRE(SDL_PushEvent(&left_ev));

    // 4) ENTER: confirma "Sim" - TitleScreen termina com NewGameRequested; o
    // mini-driver abre a tela de dificuldade ANINHADA em seguida.
    SDL_Event enter_confirm_ev = enter_open_ev;
    REQUIRE(SDL_PushEvent(&enter_confirm_ev));

    // 5) ESC (dentro da tela de dificuldade, na LISTA - fora do splash) ->
    // Cancelled (ver difficulty_screen_step_test.cpp) - o mini-driver RE-ENTRA
    // no MESMO TitleScreen.
    SDL_Event esc_ev{};
    esc_ev.type = SDL_EVENT_KEY_DOWN;
    esc_ev.key.key = SDLK_ESCAPE;
    esc_ev.key.repeat = 0;
    REQUIRE(SDL_PushEvent(&esc_ev));

    // 6) JA DE VOLTA no titulo: ENTER de novo SEM navegar - se o foco foi
    // PRESERVADO (selected ainda em NewGame), isto reabre/reconfirma o fluxo
    // de Novo Jogo (NUNCA ContinueGame). Fechamos com QUIT logo depois pra
    // terminar deterministicamente qualquer que seja o caminho (reabrir o
    // splash de novo, ou confirmar direto, dependendo de como
    // confirming_new_game ficou apos o passo 4).
    SDL_Event enter_after_return_ev = enter_open_ev;
    REQUIRE(SDL_PushEvent(&enter_after_return_ev));

    SDL_Event quit_ev{};
    quit_ev.type = SDL_EVENT_QUIT;
    REQUIRE(SDL_PushEvent(&quit_ev));

    AudioEngine audio(/*device_active=*/false);
    TitleLoopExit exit = TitleLoopExit::ContinueGame;  // sentinela: se sobrar
                                                        // assim, o teste falha
                                                        // (nunca deveria devolver
                                                        // isso neste roteiro).
    gus::domain::save::SaveData loaded{};
    gus::domain::save::DifficultyLevel chosen_difficulty{};
    run_title_menu_loop_gl_current(env.window, audio, translator, saves_dir.string(), &exit,
                                    &loaded, &chosen_difficulty);

    // ANTES do fix (se TitleScreen recriasse state_/scan_ no re-enter()): o
    // foco voltaria pro default (Continuar, SELECIONAVEL neste fixture) e o
    // Enter do passo 6 devolveria ContinueGame - este REQUIRE teria FALHADO.
    REQUIRE(exit != TitleLoopExit::ContinueGame);
    REQUIRE(exit == TitleLoopExit::QuitApp);
}

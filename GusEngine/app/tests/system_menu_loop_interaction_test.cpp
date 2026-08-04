// SPDX-License-Identifier: Apache-2.0
// GusEngine/app/tests/system_menu_loop_interaction_test.cpp
//
// Catch2 HEADLESS (GL REAL - Xvfb :99, MESMA receita de title_menu_loop_
// interaction_test.cpp/difficulty_menu_loop_interaction_test.cpp/save_load_
// menu_loop_interaction_test.cpp) do MENU DE SISTEMA (F4-1b.4, onda F4 "casca
// SDL -> App mode do glintfx", fatia 1b.4 - SEGUNDA tela PAI da onda, apos
// F4-1b.3/title_menu_loop.cpp - reusa o padrao do MINI-DRIVER, ver o
// comentario grande no topo de system_menu_loop.cpp).
//
// DEGRADACAO SEGURA: sem GL/display (sem Xvfb), cada TEST_CASE registra um
// INFO e retorna sem assercoes (0 assertions, Catch2 conta "passou") - MESMO
// espirito dos harnesses analogos.
//
// Cross-ref: gus/app/screens/system_menu_loop.cpp (o codigo sob teste,
//            SystemMenuLoopScreen + o mini-driver run_system_menu_loop_gl_
//            current); system_screen_step_test.cpp (as funcoes PURAS
//            system_screen_step/pause_flow_next, testadas headless SEM GL);
//            title_menu_loop_interaction_test.cpp (harness GL/SDL_PushEvent
//            do qual este e uma variante).

#include <catch2/catch_test_macros.hpp>

#include <filesystem>

#include <SDL3/SDL.h>

#include "gus/app/i18n/translator.hpp"
#include "gus/app/screens/system_menu_loop.hpp"
#include "gus/platform/audio/audio_engine.hpp"
#include "gus/platform/rmlui/gl3_loader.hpp"
#include "tmp_dir_test_support.hpp"

using namespace gus::app::screens;
using gus::platform::audio::AudioEngine;

namespace {

constexpr int kWinW = 960;
constexpr int kWinH = 540;

// MESMA receita de bootstrap GL de title_menu_loop_interaction_test.cpp/
// difficulty_menu_loop_interaction_test.cpp (GlTestEnv/try_boot_gl) - copiada
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

    env.window = SDL_CreateWindow("system_menu_loop_interaction_test", kWinW, kWinH,
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

// Traducao MINIMA cobrindo AMBAS as telas envolvidas no fluxo aninhado (Pause/
// arvore de config + save/load, MESMAS chaves de system_menu_rml_test.cpp/
// save_load_menu_loop_interaction_test.cpp) - o mini-driver desta fatia pode
// abrir as duas.
gus::app::i18n::Translator make_translator() {
    gus::app::i18n::Translator tr;
    tr.load_from_content(
        "## MENU_CONTINUE\nContinuar\n\n"
        "## MENU_SAVE_GAME\nSalvar\n\n"
        "## MENU_LOAD_GAME\nCarregar\n\n"
        "## SETTINGS_TITLE\nConfiguracoes\n\n"
        "## MENU_QUIT\nSair\n\n"
        "## SETTINGS_BACK\nVoltar\n\n"
        "## SETTINGS_AUDIO\nAudio\n\n"
        "## SETTINGS_VIDEO\nVideo\n\n"
        "## SETTINGS_LANGUAGE\nIdioma\n\n"
        "## MENU_TO_TITLE\nMenu Inicial\n\n"
        "## MENU_TO_TITLE_CONFIRM_TITLE\nVoltar ao menu inicial?\n\n"
        "## MENU_TO_TITLE_CONFIRM_YES\nSim\n\n"
        "## MENU_TO_TITLE_CONFIRM_NO\nCancelar\n\n"
        "## MENU_SYSTEM_KICKER\nSistema\n\n"
        "## MENU_PAUSE_TITLE\nPausado\n\n"
        "## MENU_PAUSE_HINT\n{0} confirma, {1} volta ao jogo\n\n"
        "## MENU_PLACEHOLDER_TEXT\nEm breve.\n\n"
        "## SETTINGS_MUSIC_VOLUME\nVolume da Musica\n\n"
        "## SETTINGS_SFX_VOLUME\nVolume dos Efeitos (SFX)\n\n"
        "## SETTINGS_CONTROLS\nControles\n\n"
        "## SETTINGS_RESET_DEFAULTS\nRestaurar padroes\n\n"
        "## SETTINGS_APPLY\nAplicar\n\n"
        "## CONTROLS_HINT\nSelecione uma acao\n\n"
        "## CONTROLS_CAPTURE_PROMPT\nPressione uma tecla...\n\n"
        "## CONTROLS_COL_ACTION\nAcao\n\n"
        "## CONTROLS_COL_KEYBOARD\nTeclado\n\n"
        "## CONTROLS_COL_GAMEPAD\nControle\n\n"
        "## CONTROLS_NAV_HINT\nnavega\n\n"
        "## CONTROLS_GROUP_MOVEMENT\nMovimento\n\n"
        "## CONTROLS_GROUP_WORLD\nMundo\n\n"
        "## CONTROLS_GROUP_COMBAT\nCombate\n\n"
        "## CONTROLS_GROUP_MENU_DIALOGUE\nMenu e Dialogo\n\n"
        "## CONTROLS_SWAP_NOTICE\n(!) trocou com: {0}\n\n"
        "## CONTROLS_RESTORE_CONFIRM_TITLE\nTem certeza?\n\n"
        "## CONTROLS_RESTORE_CONFIRM_YES\nSim restaurar\n\n"
        "## CONTROLS_RESTORE_CONFIRM_NO\nCancelar\n\n"
        "## CONTROLS_DISCARD_CONFIRM_TITLE\nDescartar alteracoes?\n\n"
        "## CONTROLS_DISCARD_CONFIRM_YES\nSim descartar\n\n"
        "## CONTROLS_DISCARD_CONFIRM_NO\nCancelar\n\n"
        "## CONTROLS_NO_BINDING\nsem tecla\n\n"
        "## ACTION_MOVE_FORWARD\nAndar para frente\n\n"
        "## ACTION_MOVE_BACKWARD\nAndar para tras\n\n"
        // Tela de save/load (aberta pelo mini-driver ao confirmar "Salvar").
        "## SAVE_SCREEN_TITLE_SAVE\nSalvar\n\n"
        "## SAVE_SCREEN_TITLE_LOAD\nCarregar\n\n"
        "## SAVE_SCREEN_SUBTITLE_SAVE\n{0}\n\n"
        "## SAVE_SCREEN_SUBTITLE_LOAD\n{0}\n\n"
        "## SAVE_SCREEN_FOOTER_SAVE\nx\n\n"
        "## SAVE_SCREEN_FOOTER_LOAD\nx\n\n"
        "## SAVE_SLOT_EMPTY\nVazio {0}\n\n"
        "## SAVE_SLOT_LABEL\nEspaco {0}\n\n"
        "## SAVE_SLOT_AUTO_NAME\nAuto\n\n"
        "## SAVE_SLOT_READONLY_TAG\n(so-leitura)\n\n"
        "## SAVE_XP_LABEL\nXP {0}\n\n"
        "## SAVE_CHAPTER_LABEL\nCap. {0}\n\n"
        "## SAVE_CONFIRM_OVERWRITE\nSobrescrever este slot?\n\n"
        "## SAVE_OVERWRITE_CONFIRM_YES\nSim, sobrescrever\n\n"
        "## SAVE_OVERWRITE_CONFIRM_NO\nCancelar\n\n"
        "## SAVE_CONFIRM_EMPTY\nDeseja salvar no Espaco {0} (vazio)?\n\n"
        "## SAVE_EMPTY_CONFIRM_YES\nSalvar\n\n"
        "## SAVE_EMPTY_CONFIRM_NO\nCancelar\n\n"
        "## SAVE_DELETE_BUTTON_LABEL\nApagar\n\n"
        "## SAVE_LOAD_WARN_DAMAGED\nEste save esta danificado.\n\n"
        "## SAVE_LOAD_WARN_VERSION\nEste save e de uma versao mais nova.\n\n"
        "## SAVE_LOAD_RECOVER_TRY\nTentar recuperar\n\n"
        "## SAVE_LOAD_RECOVER_FAILED\nNao foi possivel recuperar.\n\n"
        "## SAVE_LOAD_SLOT_DAMAGED_LABEL\n! Danificado\n\n"
        "## SAVE_LOAD_SLOT_VERSION_LABEL\n! Versao incompativel\n\n"
        "## SAVE_LOAD_WARN_CANCEL\nCancelar\n\n"
        "## LOCATION_PRACA_COMPILACAO\nx\n\n"
        "## LOCATION_UNKNOWN\nx\n\n");
    return tr;
}

SDL_Event key_down_event(SDL_Keycode key) {
    SDL_Event ev{};
    ev.type = SDL_EVENT_KEY_DOWN;
    ev.key.key = key;
    ev.key.repeat = 0;
    return ev;
}

}  // namespace

// ---------------------------------------------------------------- QUIT real (F4-1b.4)

TEST_CASE("system_menu_loop (harness headless): SDL_EVENT_QUIT real "
          "(SDL_PushEvent) fecha a janela - run_system_menu_loop_gl_current "
          "devolve quit_app=true (mutante analogo ao QA adversarial das "
          "fatias anteriores da onda F4)",
          "[system_menu_loop_interaction][gl]") {
    GlTestEnv env = try_boot_gl();
    if (!env.ok) {
        INFO("GL/display indisponivel neste ambiente (sem Xvfb) - harness pulado "
             "(degradacao segura, 0 assercoes). Rode com Xvfb :99 (export "
             "DISPLAY=:99) pra exercitar de fato.");
        return;
    }

    const gus::app::i18n::Translator translator = make_translator();

    const gus::test_support::ScopedTempDir saves_dir(
        "gusworld_sysmenu_loop_interaction_quit_saves");
    const gus::test_support::ScopedTempDir settings_dir(
        "gusworld_sysmenu_loop_interaction_quit_settings");

    SDL_Event quit_ev{};
    quit_ev.type = SDL_EVENT_QUIT;
    REQUIRE(SDL_PushEvent(&quit_ev));

    AudioEngine audio(/*device_active=*/false);  // sem hardware no CI
    const SystemMenuLoopOutcome outcome = run_system_menu_loop_gl_current(
        env.window, audio, translator, settings_dir.string(), saves_dir.string());

    // ANTES do fix desta fatia (se a propagacao tivesse regredido): o loop
    // ficaria esperando o PROXIMO evento pra sempre (fila vazia apos o QUIT) -
    // o teste TRAVARIA em vez de falhar rapido (MESMO padrao de deteccao ja
    // documentado nos testes analogos).
    REQUIRE(outcome.quit_app);
    REQUIRE_FALSE(outcome.to_title);
}

// ---------------------------------------------------------------- RE-ENTRADA (pausa -> save/load -> BackToPause -> volta)

// F4-1b.4 (regressao do mutante "recriar o estado no enter()" - o MINI-DRIVER
// desta fatia REUSA o MESMO objeto SystemMenuLoopScreen ao voltar de um
// BackToPause na tela de save/load; system_menu_open()/seed de volume/load de
// controls.json rodam UMA VEZ no CONSTRUTOR, NUNCA em enter(), ver o
// comentario grande em system_menu_loop.cpp): navega ate "Salvar" (item 1,
// NAO o default "Continuar", item 0), confirma (abre a tela de save/load em
// modo Save), volta (Esc/Voltar -> BackToPause -> o driver re-entra o MESMO
// SystemMenuLoopScreen) e, JA DE VOLTA no Pause, aperta Enter SEM navegar de
// novo. Se o estado tivesse sido RECRIADO no re-enter() (o mutante), o foco
// teria voltado pro DEFAULT (Continuar, item 0) e este 2o Enter fecharia o
// menu direto (outcome.quit_app=false, SEM abrir save/load de novo) - o teste
// PROVA que isso NAO acontece (abre save/load DE NOVO, fechado por um 2o
// BackToPause, e so ENTAO por QUIT).
TEST_CASE("system_menu_loop (harness headless): fluxo Pause -> Salvar -> "
          "BackToPause -> volta ao Pause PRESERVANDO a selecao (mata o "
          "mutante 'recriar SystemMenuState no enter()')",
          "[system_menu_loop_interaction][gl]") {
    GlTestEnv env = try_boot_gl();
    if (!env.ok) {
        INFO("GL/display indisponivel neste ambiente (sem Xvfb) - harness pulado "
             "(degradacao segura, 0 assercoes). Rode com Xvfb :99 (export "
             "DISPLAY=:99) pra exercitar de fato.");
        return;
    }

    const gus::app::i18n::Translator translator = make_translator();

    const gus::test_support::ScopedTempDir saves_dir(
        "gusworld_sysmenu_loop_interaction_reentry_saves");
    const gus::test_support::ScopedTempDir settings_dir(
        "gusworld_sysmenu_loop_interaction_reentry_settings");

    // 1) DOWN: Continuar(0) -> Salvar(1).
    SDL_Event down_ev = key_down_event(SDLK_DOWN);
    REQUIRE(SDL_PushEvent(&down_ev));

    // 2) ENTER: confirma "Salvar" - o Pause termina com exit=OpenSaveLoadSave;
    // o mini-driver abre a tela de save/load ANINHADA em seguida (modo Save).
    SDL_Event enter_ev = key_down_event(SDLK_RETURN);
    REQUIRE(SDL_PushEvent(&enter_ev));

    // 3) ESC (dentro da tela de save/load, na LISTA normal, fora de qualquer
    // mini-dialogo) -> Back -> BackToPause (ver save_load_screen_step_test.cpp)
    // - o mini-driver RE-ENTRA no MESMO SystemMenuLoopScreen.
    SDL_Event esc_ev = key_down_event(SDLK_ESCAPE);
    REQUIRE(SDL_PushEvent(&esc_ev));

    // 4) JA DE VOLTA no Pause: ENTER de novo SEM navegar - se a selecao foi
    // PRESERVADA (pause_selected ainda em Save), isto reabre a tela de save/
    // load (NUNCA fecha o menu direto via Continuar).
    SDL_Event enter_after_return_ev = key_down_event(SDLK_RETURN);
    REQUIRE(SDL_PushEvent(&enter_after_return_ev));

    // 5) ESC de novo (2a tela de save/load aninhada) -> BackToPause de novo.
    SDL_Event esc_ev2 = key_down_event(SDLK_ESCAPE);
    REQUIRE(SDL_PushEvent(&esc_ev2));

    // 6) QUIT - termina deterministicamente qualquer que seja o caminho.
    SDL_Event quit_ev{};
    quit_ev.type = SDL_EVENT_QUIT;
    REQUIRE(SDL_PushEvent(&quit_ev));

    AudioEngine audio(/*device_active=*/false);
    const SystemMenuLoopOutcome outcome = run_system_menu_loop_gl_current(
        env.window, audio, translator, settings_dir.string(), saves_dir.string());

    // ANTES do fix (se SystemMenuLoopScreen recriasse state_ no re-enter()): o
    // foco voltaria pro default (Continuar) e o Enter do passo 4 fecharia o
    // menu (outcome.quit_app=false) MUITO ANTES do QUIT do passo 6 ser
    // processado - este REQUIRE teria FALHADO (outcome nao chegaria a
    // quit_app=true).
    REQUIRE(outcome.quit_app);
}

// SPDX-License-Identifier: Apache-2.0
// GusEngine/app/tests/save_load_menu_loop_interaction_test.cpp
//
// Catch2 HEADLESS (GL REAL - Xvfb :99, MESMA receita de
// difficulty_menu_loop_interaction_test.cpp/save_load_menu_interaction_test.cpp)
// da tela de save/load: F4-1b.2 (onda F4 "casca SDL -> App mode do glintfx",
// fatia 1b.2 - SEGUNDA tela convertida ao contrato ScreenState, MESMO template
// de F4-1b.1/difficulty_menu_loop.cpp) QA-FOLLOWUP - a propagacao
// SDL_EVENT_QUIT -> SaveLoadScreen::window_closed_ (handle_event(),
// save_load_menu_loop.cpp) nao tinha NENHUM teste no NIVEL DA CLASSE antes
// desta fatia (so o comportamento PURO, save_load_screen_step_test.cpp,
// exercitava `result.window_closed`). Este arquivo prova o FIO INTEIRO ate o
// retorno REAL de run_save_load_menu_loop_gl_current - se alguem quebrar a
// propagacao step.window_closed -> window_closed_ (ou o guard window_closed()
// no wrapper), a janela nunca fecharia pelo X/Alt+F4 nesta tela e a suite
// passaria verde sem isto.
//
// DEGRADACAO SEGURA: sem GL/display (sem Xvfb), o TEST_CASE registra um INFO e
// retorna sem assercoes (0 assertions, Catch2 conta "passou") - MESMO espirito
// de save_load_menu_interaction_test.cpp/difficulty_menu_loop_interaction_test.cpp.
//
// Cross-ref: gus/app/screens/save_load_menu_loop.cpp (o codigo sob teste,
//            SaveLoadScreen); difficulty_menu_loop_interaction_test.cpp
//            (harness GL/SDL_PushEvent do qual este e uma variante);
//            save_load_menu_interaction_test.cpp (harness de
//            POSICIONAMENTO/INTERACAO mais amplo, ja existente, cobre
//            clique/hover/wheel/dialogos via SDL_PushEvent contra o mesmo
//            wrapper).

#include <catch2/catch_test_macros.hpp>


#include <SDL3/SDL.h>

#include "gus/app/i18n/translator.hpp"
#include "gus/app/screens/save_load_menu.hpp"
#include "gus/app/screens/save_load_menu_loop.hpp"
#include "gus/platform/audio/audio_engine.hpp"
#include "gus/platform/rmlui/gl3_loader.hpp"
#include "tmp_dir_test_support.hpp"

using namespace gus::app::screens;
using gus::platform::audio::AudioEngine;

namespace {

constexpr int kWinW = 960;
constexpr int kWinH = 540;

// MESMA receita de bootstrap GL de difficulty_menu_loop_interaction_test.cpp/
// save_load_menu_interaction_test.cpp (GlTestEnv/try_boot_gl) - copiada aqui
// (arquivo AUTO-CONTIDO, MESMO nao-acoplamento entre harnesses ja estabelecido
// nas telas de producao).
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

    env.window = SDL_CreateWindow("save_load_menu_loop_interaction_test", kWinW, kWinH,
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

gus::app::i18n::Translator make_translator() {
    gus::app::i18n::Translator tr;
    tr.load_from_content(
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
        "## SETTINGS_BACK\nVoltar\n\n"
        "## LOCATION_PRACA_COMPILACAO\nx\n\n"
        "## LOCATION_UNKNOWN\nx\n\n");
    return tr;
}

}  // namespace

// ---------------------------------------------------------------- QUIT real (F4-1b.2 QA-FOLLOWUP)

TEST_CASE("save_load_menu_loop (harness headless): SDL_EVENT_QUIT real "
          "(SDL_PushEvent) fecha a janela - run_save_load_menu_loop_gl_current "
          "devolve QuitApp (mutante analogo ao QA adversarial F4-1b.1)",
          "[save_load_menu_loop_interaction][gl]") {
    GlTestEnv env = try_boot_gl();
    if (!env.ok) {
        INFO("GL/display indisponivel neste ambiente (sem Xvfb) - harness pulado "
             "(degradacao segura, 0 assercoes). Rode com Xvfb :99 (export "
             "DISPLAY=:99) pra exercitar de fato.");
        return;
    }

    const gus::app::i18n::Translator translator = make_translator();

    const gus::test_support::ScopedTempDir saves_dir("gusworld_save_load_loop_interaction_quit_saves");

    SDL_Event quit_ev{};
    quit_ev.type = SDL_EVENT_QUIT;
    REQUIRE(SDL_PushEvent(&quit_ev));

    AudioEngine audio(/*device_active=*/false);  // sem hardware no CI
    const SaveLoadLoopExit exit = run_save_load_menu_loop_gl_current(
        env.window, audio, translator, SaveLoadMode::Load, saves_dir.string(),
        /*build_current_save_data=*/{}, /*apply_loaded_save_data=*/{});

    // ANTES do fix desta fatia (se a propagacao tivesse regredido):
    // window_closed_ nunca viraria true, o loop ficaria esperando o PROXIMO
    // evento pra sempre (fila vazia apos o QUIT) - o teste TRAVARIA em vez de
    // falhar rapido (MESMO padrao de deteccao ja documentado nos testes
    // analogos de difficulty_menu_loop_interaction_test.cpp).
    REQUIRE(exit == SaveLoadLoopExit::QuitApp);
    // QUIT e verificado ANTES de qualquer hit-test/SFX (ver save_load_screen_step)
    // - nenhum som deveria ter tocado no caminho ate fechar a janela.
    REQUIRE(audio.sfx_play_count() == 0);

    // saves_dir e um gus::test_support::ScopedTempDir - RAII remove no fim do escopo.
}

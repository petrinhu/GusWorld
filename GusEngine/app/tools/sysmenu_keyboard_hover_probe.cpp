// PROBE EFEMERO (NAO COMMITADO - app/tools/ fica de fora do git, MESMO padrao de
// sysmenu_controls_diag_probe.cpp neste mesmo diretorio) - dispara
// run_system_menu_loop_gl_current direto (sem Maestro/cidade) com
// GUSWORLD_SYSMENU_KEYBOARD_HOVER_SELFTEST=1 pra provar (headless, Xvfb :99, NUNCA
// :0) que a navegacao por TECLADO toca o mesmo hover_sfx do mouse (paridade pedida
// pelo lider). Descartar apos a prova.

#include <cstdlib>
#include <iostream>
#include <string>

#include <SDL3/SDL.h>

#include "gus/app/i18n/translator.hpp"
#include "gus/app/screens/system_menu_loop.hpp"
#include "gus/platform/audio/audio_engine.hpp"
#include "gus/platform/fs/settings_file_store.hpp"
#include "gus/platform/rmlui/gl3_loader.hpp"

int main(int argc, char** argv) {
    // Regressao rapida: argv[1] escolhe QUAL selftest env-gated ligar (default =
    // o novo, teclado). Usado so nesta prova ao vivo (nao muda producao).
    const char* selftest_env_name =
        (argc > 1) ? argv[1] : "GUSWORLD_SYSMENU_KEYBOARD_HOVER_SELFTEST";
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init falhou: " << SDL_GetError() << "\n";
        return 1;
    }

    SDL_Window* window =
        SDL_CreateWindow("sysmenu_keyboard_hover_probe", 1280, 720, SDL_WINDOW_OPENGL);
    if (window == nullptr) {
        std::cerr << "SDL_CreateWindow falhou: " << SDL_GetError() << "\n";
        return 1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GLContext gl = SDL_GL_CreateContext(window);
    if (gl == nullptr) {
        std::cerr << "SDL_GL_CreateContext falhou: " << SDL_GetError() << "\n";
        return 1;
    }
    SDL_GL_MakeCurrent(window, gl);
    if (!gus::platform::rmlui::gl3_load_functions(
            reinterpret_cast<void* (*)(const char*)>(SDL_GL_GetProcAddress))) {
        std::cerr << "gl3_load_functions falhou\n";
        return 1;
    }

    gus::app::i18n::Translator translator;
    const std::string tr_path = gus::app::i18n::resolve_translations_path();
    if (!translator.load_from_file(tr_path)) {
        std::cerr << "Translator::load_from_file falhou: " << tr_path << "\n";
        return 1;
    }

    // device_active=false: SEM hardware de audio, MESMO padrao do self-test
    // headless (sfx_play_count() e o hook de prova, nao o som em si).
    gus::platform::audio::AudioEngine audio(/*device_active=*/false);
    const std::string settings_dir = gus::platform::fs::resolve_settings_dir();

    setenv(selftest_env_name, "1", 1);
    const auto outcome = gus::app::screens::run_system_menu_loop_gl_current(
        window, audio, translator, settings_dir);
    std::cout << "outcome.quit_app=" << outcome.quit_app << "\n";
    std::cout << "audio.sfx_play_count() total=" << audio.sfx_play_count() << "\n";

    SDL_GL_DestroyContext(gl);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

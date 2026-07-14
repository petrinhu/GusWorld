// PROBE EFEMERO (NAO COMMITADO - app/tools/ fica de fora do git, MESMO padrao de
// frozen_bg_probe.cpp neste mesmo diretorio) - dispara run_system_menu_loop_gl_current
// direto (sem Maestro/cidade) com GUSWORLD_SYSMENU_CONTROLS_DIAG=1 pra investigar os 3
// bugs ao vivo da tela Controles (M2). Descartar apos a investigacao.

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <SDL3/SDL.h>

#include "gus/app/i18n/translator.hpp"
#include "gus/app/screens/system_menu_loop.hpp"
#include "gus/platform/audio/audio_engine.hpp"
#include "gus/platform/fs/settings_file_store.hpp"
#include "gus/platform/rmlui/gl3_loader.hpp"

int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init falhou: " << SDL_GetError() << "\n";
        return 1;
    }

    int win_w = 1280;
    int win_h = 720;
    if (const char* w = std::getenv("GUSWORLD_PROBE_WIN_W")) win_w = std::atoi(w);
    if (const char* h = std::getenv("GUSWORLD_PROBE_WIN_H")) win_h = std::atoi(h);
    SDL_Window* window =
        SDL_CreateWindow("sysmenu_controls_diag_probe", win_w, win_h, SDL_WINDOW_OPENGL);
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

    gus::platform::audio::AudioEngine audio(/*device_active=*/false);
    const std::string settings_dir = gus::platform::fs::resolve_settings_dir();
    std::cout << "settings_dir=" << settings_dir << "\n";

    if (std::getenv("GUSWORLD_PROBE_PUSH_EVENTS") != nullptr) {
        // MODO 2: prova o CAMINHO REAL de SDL_Event (SDL_PushEvent, so nesta
        // investigacao efemera - producao NUNCA usa isto) atraves do while(true)
        // de producao (nao do atalho GUSWORLD_SYSMENU_CONTROLS_DIAG).
        auto push_key = [](SDL_Keycode kc) {
            SDL_Event ev{};
            ev.type = SDL_EVENT_KEY_DOWN;
            ev.key.type = SDL_EVENT_KEY_DOWN;
            ev.key.key = kc;
            ev.key.repeat = false;
            SDL_PushEvent(&ev);
        };
        push_key(SDLK_DOWN);    // Continue->Save
        push_key(SDLK_DOWN);    // Save->Settings
        push_key(SDLK_RETURN);  // entra ConfigCategories
        push_key(SDLK_DOWN);    // Audio->Video
        push_key(SDLK_DOWN);    // Video->Controls
        push_key(SDLK_RETURN);  // entra Controls
        // CLIQUE REAL de mouse na linha 1 ("Andar para tras", coords medidas via
        // GUSWORLD_SYSMENU_CONTROLS_DIAG a 1280x720: box(controls-item-1) x=265
        // y=301.867 w=744 h=43.6 -> centro (637, 324)) - prova o CAMINHO REAL de
        // SDL_EVENT_MOUSE_BUTTON_DOWN + hit_test do loop de producao (nao
        // click_controls_option chamado direto).
        auto push_mouse_down = [](float x, float y) {
            SDL_Event ev{};
            ev.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
            ev.button.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
            ev.button.button = SDL_BUTTON_LEFT;
            ev.button.x = x;
            ev.button.y = y;
            SDL_PushEvent(&ev);
        };
        push_mouse_down(637.0f, 324.0f);
        push_key(SDLK_ESCAPE);  // cancela o modo captura aberto pelo clique
        push_key(SDLK_DOWN);
        push_key(SDLK_DOWN);  // prova que a navegacao por teclado CONTINUA
                               // funcionando depois do clique
        SDL_Event quit_ev{};
        quit_ev.type = SDL_EVENT_QUIT;
        SDL_PushEvent(&quit_ev);

        const auto outcome = gus::app::screens::run_system_menu_loop_gl_current(
            window, audio, translator, settings_dir);
        std::cout << "outcome.quit_app=" << outcome.quit_app << "\n";

        // O ULTIMO RML escrito por write_system_menu_rml_file reflete o estado
        // FINAL (reload() a cada handle_action) - inspeciona qual controls-item-N
        // tem a classe 'sel'.
        const std::string stage =
            (std::filesystem::temp_directory_path() / "gusworld_glintfx_sysmenu").string();
        std::ifstream rml_in(stage + "/system_menu.rml");
        std::ostringstream ss;
        ss << rml_in.rdbuf();
        const std::string txt = ss.str();
        for (int i = 0; i < 32; ++i) {
            const std::string needle = "controls-item-" + std::to_string(i) + "\">";
            auto pos = txt.find(needle);
            if (pos == std::string::npos) continue;
            // A classe fica ANTES do id no nosso RML ("<div class=\"...\" id=\"...\">").
            const auto div_start = txt.rfind("<div class=\"", pos);
            const std::string frag = txt.substr(div_start, pos - div_start);
            if (frag.find(" sel") != std::string::npos ||
                frag.find(" capturing") != std::string::npos) {
                std::cout << "[push-events] item " << i << " esta selecionado/capturando: "
                          << frag << "\n";
            }
        }
        SDL_GL_DestroyContext(gl);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 0;
    }

    setenv("GUSWORLD_SYSMENU_CONTROLS_SELFTEST", "1", 1);
    const auto outcome = gus::app::screens::run_system_menu_loop_gl_current(
        window, audio, translator, settings_dir);
    std::cout << "outcome.quit_app=" << outcome.quit_app << "\n";

    SDL_GL_DestroyContext(gl);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

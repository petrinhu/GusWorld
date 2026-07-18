// PROBE EFEMERO (NAO COMMITADO - app/tools/ fica de fora do git ate hoje, MESMO
// padrao de repro_bertoldo.cpp neste mesmo diretorio) - prova headless sob Xvfb
// do botao "Continuar" (hover/clique/SFX) do dialogo do NPC. MESMO espirito do
// probe citado no commit 6e9f977 ("probe empirico sob Xvfb... nao commitado").
// Descartar apos a prova (nao faz parte do jogo).

#include <cstdlib>
#include <iostream>
#include <map>
#include <string>

#include <SDL3/SDL.h>

#include "gus/app/i18n/translator.hpp"
#include "gus/app/sdl_window.hpp"
#include "gus/app/screens/npc_dialogue_loop_gl.hpp"
#include "gus/domain/dialogue/dialogue_graph.hpp"
#include "gus/domain/dialogue/dialogue_runtime.hpp"
#include "gus/platform/audio/audio_engine.hpp"
#include "gus/platform/rmlui/gl3_loader.hpp"  // FLASH-CTX: gl3_load_functions

int main() {
    setenv("GUSWORLD_NPCDLG_HOVER_SELFTEST", "1", 1);

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init falhou: " << SDL_GetError() << "\n";
        return 1;
    }
    SDL_Window* window =
        SDL_CreateWindow("npcdlg_hover_probe", 960, 540, SDL_WINDOW_OPENGL);
    if (window == nullptr) {
        std::cerr << "SDL_CreateWindow falhou: " << SDL_GetError() << "\n";
        return 1;
    }

    // FLASH-CTX (A1): SdlWindow::init_attached() agora ASSUME um contexto GL ja
    // corrente (MESMA receita de Maestro::init() - ver maestro.cpp) - este probe
    // precisa criar/fazer-current o PROPRIO contexto ANTES de chamar init_attached.
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
    SDL_GL_SetSwapInterval(0);
    if (!gus::platform::rmlui::gl3_load_functions(
            reinterpret_cast<void* (*)(const char*)>(SDL_GL_GetProcAddress))) {
        std::cerr << "gl3_load_functions falhou (glad)\n";
        return 1;
    }

    {
        // FLASH-CTX: escopo PROPRIO pra `city` - o dtor da SdlWindow destroi um
        // Render2dGl3 que NUNCA e destruido por release_renderer (so pausado, ver
        // sdl_window.hpp) - PRECISA rodar enquanto `gl` ainda esta vivo/corrente,
        // ANTES do SDL_DestroyWindow/SDL_Quit no fim do main() (ordem importa: GL
        // apos contexto/janela destruidos e UB).
        gus::app::SdlWindow city;
        if (!city.init_attached(window)) {
            std::cerr << "city.init_attached falhou\n";
            return 1;
        }
        city.release_renderer();

        gus::app::i18n::Translator tr;
        tr.load_from_content(
            "## ACTOR_BERTOLDO\nSeu Bertoldo\n\n"
            "## DIALOGUE_CONTINUE\n(continuar)\n\n"
            "## DIALOGUE_NPC_INTRO_N0_GREET\nCedo pra rua, moco.\n\n");

        gus::domain::dialogue::DialogueGraph graph;
        graph.dialogue_id = "probe";
        graph.entry_node_id = "n0";
        gus::domain::dialogue::DialogueNode n0;
        n0.id = "n0";
        n0.speaker_id = "bertoldo";
        n0.text_key = "DIALOGUE_NPC_INTRO_N0_GREET";
        n0.next_node_id = std::string(gus::domain::dialogue::kExitNodeId);
        graph.nodes["n0"] = n0;
        graph.validate();

        std::map<std::string, bool> flags;
        gus::domain::dialogue::DialogueRuntime runtime(graph, flags);
        runtime.enter();

        gus::platform::audio::AudioEngine audio(/*device_active=*/false);

        const bool quit = gus::app::screens::run_npc_dialogue_loop_gl(window, city,
                                                                       runtime, tr, audio);
        std::cout << "npcdlg_hover_probe: run_npc_dialogue_loop_gl retornou quit="
                  << quit << "\n";

        city.reacquire_renderer();
    }
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

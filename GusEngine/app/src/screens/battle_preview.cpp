// gus/app/src/screens/battle_preview.cpp
//
// Ver header. Casca SDL do viewer da BattleScene (esqueleto M5). Reusa Render2dSdl
// (atras de IRenderer) e o mesmo padrao de loop do anim_preview (poll -> render). A
// cena LE o motor de combate; aqui so abrimos janela, carregamos os retratos 48px e
// desenhamos o esqueleto a cada frame. Esc/fechar encerra.

#include "gus/app/screens/battle_preview.hpp"

#include <cstdlib>  // std::getenv
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include <SDL3/SDL.h>

#include "gus/app/screens/battle_scene.hpp"
#include "gus/platform/render2d/render2d_sdl.hpp"

// Raiz resources/ do repo, embutida pelo CMake (mesma macro do resolver de sprites).
#ifndef GUSWORLD_ASSETS_DIR
#define GUSWORLD_ASSETS_DIR ""
#endif

namespace gus::app::screens {

namespace {

constexpr int kWindowW = 1280;  // 640x360 * 2 (escala inteira x2, D1)
constexpr int kWindowH = 720;

std::string join(const std::string& a, const std::string& b) {
    if (a.empty()) {
        return b;
    }
    if (a.back() == '/') {
        return a + b;
    }
    return a + "/" + b;
}

// Mapeia o id de ator de DEMO -> arquivo de retrato 48px. Os inimigos (inimigoN)
// compartilham retrato_inimigo. Ponto unico; quando os retratos forem por-personagem
// reais, troca-se aqui (ou vira data-driven).
std::string retrato_file_for(const std::string& actor_id) {
    if (actor_id == "gus") return "retrato_gus.png";
    if (actor_id == "caua") return "retrato_caua.png";
    if (actor_id == "jaci") return "retrato_jaci.png";
    // inimigo1..4 e qualquer outro -> retrato generico de inimigo.
    return "retrato_inimigo.png";
}

}  // namespace

std::string resolve_retratos_dir() {
    if (const char* env = std::getenv("GUSWORLD_ASSETS")) {
        if (env[0] != '\0') {
            return join(env, "sprites/icons-m5/retratos");
        }
    }
    const std::string compiled = GUSWORLD_ASSETS_DIR;
    if (!compiled.empty()) {
        return join(compiled, "sprites/icons-m5/retratos");
    }
    return "resources/sprites/icons-m5/retratos";
}

int run_battle_preview() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "BattlePreview: SDL_Init falhou: " << SDL_GetError() << "\n";
        return 1;
    }
    SDL_Window* window = nullptr;
    SDL_Renderer* sdl_renderer = nullptr;
    if (!SDL_CreateWindowAndRenderer("GusWorld BattlePreview (M5 esqueleto)",
                                     kWindowW, kWindowH, SDL_WINDOW_RESIZABLE,
                                     &window, &sdl_renderer)) {
        std::cerr << "BattlePreview: SDL_CreateWindowAndRenderer falhou: "
                  << SDL_GetError() << "\n";
        SDL_Quit();
        return 1;
    }
    SDL_SetRenderVSync(sdl_renderer, 1);

    {
        gus::platform::render2d::Render2dSdl renderer(sdl_renderer);

        // A cena monta o encontro de demo e ja le a fila do motor.
        BattleScene scene;

        // Carrega os retratos 48px da fila CTB (handles resolvidos pelo renderer) e os
        // entrega a cena. Cada id de ator -> seu retrato; ausencia degrada pro retangulo.
        const std::string dir = resolve_retratos_dir();
        BattlePortraitSet portraits;
        for (const auto* actor : scene.machine().queue().order()) {
            if (actor == nullptr) {
                continue;
            }
            const std::string path = join(dir, retrato_file_for(actor->id()));
            const gus::platform::render2d::TextureId tex =
                renderer.load_texture(path.c_str());
            portraits.by_id.emplace_back(actor->id(), tex);
        }
        scene.set_portraits(std::move(portraits));

        std::cout << "BattlePreview: party=" << scene.party_count()
                  << " inimigos=" << scene.enemy_count()
                  << " fila=" << scene.queue_len() << " retratos em " << dir
                  << " (Esc sai)\n";

        bool running = true;
        while (running) {
            SDL_Event ev;
            while (SDL_PollEvent(&ev)) {
                if (ev.type == SDL_EVENT_QUIT) {
                    running = false;
                } else if (ev.type == SDL_EVENT_KEY_DOWN &&
                           ev.key.key == SDLK_ESCAPE) {
                    running = false;
                }
            }
            if (!running) {
                break;
            }

            int pw = kWindowW, ph = kWindowH;
            SDL_GetCurrentRenderOutputSize(sdl_renderer, &pw, &ph);
            scene.render(renderer, static_cast<float>(pw),
                         static_cast<float>(ph));
        }
    }  // Render2dSdl destruido antes do renderer SDL

    SDL_DestroyRenderer(sdl_renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

}  // namespace gus::app::screens

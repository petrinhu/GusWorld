// SPDX-License-Identifier: Apache-2.0
// PROBE EFEMERO (mesmo padrao de idle_facing_probe.cpp neste diretorio - janela SDL
// HIDDEN + renderer "software", SEM Xvfb/compositor, SEM tocar a sessao viva): valida
// visualmente a migracao do Cauã pra sprites/caua_volt_cyan_v2/ (ASSETS-VERSIONAR-
// SPRITES, 2026-08-06) - anda 1 tick em cada direcao cardinal (South, North, East,
// West), para (dx=dy=0, entra em idle = fallback pro walk f0, ja que
// caua_volt_cyan_v2/ nao tem anims/breathing_idle/ ainda), e despeja 1 PNG por
// direcao. Nao faz parte do jogo; descartavel apos a verificacao.

#include <SDL3/SDL.h>

#include <iostream>
#include <string>

#include "gus/app/screens/city_loader.hpp"
#include "gus/app/screens/overworld_sim.hpp"
#include "gus/app/screens/player_sprites_loader.hpp"
#include "gus/core/asset_paths.hpp"
#include "gus/platform/render2d/render2d_sdl.hpp"

#include "stb_image_write.h"

namespace {

bool render_and_dump(gus::app::screens::OverworldSim& sim,
                     gus::platform::render2d::Render2dSdl& r2d, SDL_Renderer* renderer,
                     int pw, int ph, const std::string& out_path) {
    sim.render(r2d, static_cast<float>(pw), static_cast<float>(ph), 0.0f);
    SDL_Surface* surf = SDL_RenderReadPixels(renderer, nullptr);
    if (surf == nullptr) {
        std::cerr << "SDL_RenderReadPixels falhou: " << SDL_GetError() << "\n";
        return false;
    }
    SDL_Surface* conv = SDL_ConvertSurface(surf, SDL_PIXELFORMAT_RGBA32);
    bool ok = false;
    if (conv != nullptr) {
        ok = stbi_write_png(out_path.c_str(), conv->w, conv->h, 4, conv->pixels,
                            conv->pitch) != 0;
        std::cout << "PNG salvo (" << ok << "): " << out_path << "\n";
        SDL_DestroySurface(conv);
    }
    SDL_DestroySurface(surf);
    return ok;
}

}  // namespace

int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init falhou: " << SDL_GetError() << "\n";
        return 1;
    }
    SDL_Window* window =
        SDL_CreateWindow("caua_idle_facing_probe", 640, 480, SDL_WINDOW_HIDDEN);
    if (window == nullptr) {
        std::cerr << "SDL_CreateWindow falhou: " << SDL_GetError() << "\n";
        return 1;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    if (renderer == nullptr) {
        std::cerr << "SDL_CreateRenderer falhou: " << SDL_GetError() << "\n";
        return 1;
    }
    std::cout << "renderer driver = " << SDL_GetRendererName(renderer) << "\n";

    gus::platform::render2d::Render2dSdl r2d(renderer);

    gus::app::screens::CityLoadOutcome city = gus::app::screens::load_city_or_fallback();
    gus::app::screens::OverworldSim sim(std::move(city.sim));

    const std::string caua_dir = gus::app::screens::resolve_caua_sprites_dir();
    std::cout << "caua_dir = " << caua_dir << "\n";
    sim.set_player_sprites(gus::app::screens::load_caua_sprites(r2d, caua_dir));

    const float dt = 1.0f / 60.0f;
    const int pw = 640;
    const int ph = 480;

    struct Case {
        const char* label;
        int dx, dy;
    };
    // Ordem: 1 tick de movimento na direcao (registra o facing, MESMO se colidir - o
    // facing e derivado da INTENCAO dx/dy, nao do deslocamento real), depois N ticks
    // parado (dx=dy=0) pra estabilizar no idle (walk f0 congelado, ja que o Cauã ainda
    // nao tem anims/breathing_idle/) daquela direcao.
    const Case cases[] = {
        {"south", 0, 1},
        {"north", 0, -1},
        {"east", 1, 0},
        {"west", -1, 0},
    };

    bool all_ok = true;
    for (const Case& c : cases) {
        sim.step_fixed(c.dx, c.dy, /*run=*/false, dt);
        for (int i = 0; i < 30; ++i) {
            sim.step_fixed(0, 0, /*run=*/false, dt);
        }
        const std::string out_path =
            "/var/tmp/gusworld_caua_idle_facing_" + std::string(c.label) + ".png";
        if (!render_and_dump(sim, r2d, renderer, pw, ph, out_path)) {
            all_ok = false;
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return all_ok ? 0 : 1;
}

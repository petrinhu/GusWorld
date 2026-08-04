// SPDX-License-Identifier: Apache-2.0
// PROBE EFEMERO (app/tools/ e scratch/untracked - MESMO padrao de frozen_bg_probe.cpp
// e npcdlg_screenshot_probe.cpp neste mesmo diretorio) - VERIFICACAO VISUAL da fatia D
// do DEMO-CIDADE-VESTIDA: captura frames REAIS da cidade vestida (SdlWindow::
// init_attached -> dress_city/populate_city -> capture_frame_to_png, a MESMA cadeia da
// producao) para um QA independente olhar contra o checklist.
//
// A sequencia de atores (marcador do inimigo + ronda + marcador do Bertoldo) e
// TRANSCRITA de gus/app/src/maestro.cpp::init() - o probe NAO inventa posicao, so
// repete o que o jogo faz, porque so o que o jogo faz e o que precisa ser auditado.
//
// Controle por env var (nada hardcoded, um binario serve todas as capturas):
//   AUDIT_W / AUDIT_H          tamanho da janela em px (default 960x540 = producao)
//   AUDIT_CELL_X / AUDIT_CELL_Y  teleporta o jogador para esta celula antes de capturar
//   AUDIT_STEPS                passos de loop antes da 1a captura (default 8)
//   AUDIT_STEPS2               passos ADICIONAIS antes da 2a captura (0 = nao captura)
//   AUDIT_OUT                  caminho do PNG de saida (a 2a vira <out>_t2.png)
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>

#include <SDL3/SDL.h>

#include "gus/app/maestro_logic.hpp"
#include "gus/app/screens/city_actors.hpp"
#include "gus/app/screens/city_patrol.hpp"
#include "gus/app/sdl_window.hpp"
#include "gus/platform/rmlui/gl3_loader.hpp"

namespace {

int env_int(const char* name, int fallback) {
    const char* v = std::getenv(name);
    if (v == nullptr || *v == '\0') return fallback;
    return std::atoi(v);
}

std::string env_str(const char* name, const std::string& fallback) {
    const char* v = std::getenv(name);
    if (v == nullptr || *v == '\0') return fallback;
    return std::string(v);
}

}  // namespace

int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init falhou: " << SDL_GetError() << "\n";
        return 1;
    }

    const int kW = env_int("AUDIT_W", 960);
    const int kH = env_int("AUDIT_H", 540);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    SDL_Window* window = SDL_CreateWindow("city_audit_probe", kW, kH, SDL_WINDOW_OPENGL);
    if (window == nullptr) {
        std::cerr << "SDL_CreateWindow falhou: " << SDL_GetError() << "\n";
        return 1;
    }
    SDL_GLContext gl = SDL_GL_CreateContext(window);
    if (gl == nullptr) {
        std::cerr << "SDL_GL_CreateContext falhou: " << SDL_GetError() << "\n";
        return 1;
    }
    SDL_GL_MakeCurrent(window, gl);
    SDL_GL_SetSwapInterval(0);
    if (!gus::platform::rmlui::gl3_load_functions(
            reinterpret_cast<void* (*)(const char*)>(SDL_GL_GetProcAddress))) {
        std::cerr << "gl3_load_functions falhou\n";
        return 1;
    }

    int rc = 0;
    {
        gus::app::SdlWindow city;
        if (!city.init_attached(window)) {
            std::cerr << "city.init_attached falhou\n";
            return 1;
        }

        int dw = 0, dh = 0;
        SDL_GetWindowSizeInPixels(window, &dw, &dh);
        std::cout << "[probe] janela pedida " << kW << "x" << kH << ", real " << dw
                  << "x" << dh << "\n";
        std::cout << "[probe] mapa " << city.grid().width() << "x" << city.grid().height()
                  << ", tile_size " << city.grid().tile_size()
                  << ", zoom " << city.tuning().camera_zoom_px_per_tile << " px/tile\n";

        // --- TRANSCRICAO de Maestro::init() (androide sentinela + ronda) ---------
        const gus::core::spatial::Aabb enemy_anchor =
            gus::app::pick_actor_position_at_cell(
                city.grid(), city.player_aabb(),
                gus::app::screens::kSentinelArenaCellX,
                gus::app::screens::kSentinelArenaCellY);
        const gus::core::spatial::Aabb enemy_aabb = gus::app::enemy_sprite_footprint_aabb(
            enemy_anchor, city.tuning().player_sprite_height_tiles,
            city.grid().tile_size());
        city.set_enemy_marker(enemy_aabb);

        const int enemy_cell_x =
            city.grid().world_to_cell(enemy_anchor.x + enemy_anchor.w * 0.5f);
        const int enemy_cell_y =
            city.grid().world_to_cell(enemy_anchor.y + enemy_anchor.h * 0.5f);
        const gus::domain::world::PatrolRoute enemy_route =
            gus::app::screens::build_horizontal_patrol_route(
                city.grid(), enemy_cell_x, enemy_cell_y,
                gus::app::screens::kDemoPatrolReachTiles,
                gus::app::screens::kDemoPatrolSpeedTilesPerSec,
                gus::app::screens::kDemoPatrolPauseSeconds);
        city.set_enemy_patrol_route(enemy_route);
        std::cout << "[probe] sentinela em (" << enemy_aabb.x << ", " << enemy_aabb.y
                  << "), celula (" << enemy_cell_x << "," << enemy_cell_y
                  << "), rota valida=" << enemy_route.valid() << "\n";

        // --- TRANSCRICAO de Maestro::init() (Bertoldo) ---------------------------
        const gus::core::spatial::Aabb npc_anchor = gus::app::pick_actor_position_at_cell(
            city.grid(), city.player_aabb(), gus::app::screens::kBertoldoBenchCellX,
            gus::app::screens::kBertoldoBenchCellY);
        const gus::core::spatial::Aabb npc_aabb = gus::app::enemy_sprite_footprint_aabb(
            npc_anchor, city.tuning().npc_bertoldo_sprite_height_tiles,
            city.grid().tile_size());
        city.set_npc_bertoldo_marker(npc_aabb);
        std::cout << "[probe] bertoldo em (" << npc_aabb.x << ", " << npc_aabb.y << ")\n";

        // --- TELEPORTE opcional do jogador --------------------------------------
        const int cell_x = env_int("AUDIT_CELL_X", -1);
        const int cell_y = env_int("AUDIT_CELL_Y", -1);
        if (cell_x >= 0 && cell_y >= 0) {
            gus::core::spatial::Aabb pos = city.player_aabb();
            const gus::core::spatial::Aabb target = gus::app::pick_actor_position_at_cell(
                city.grid(), city.player_aabb(), cell_x, cell_y);
            pos.x = target.x;
            pos.y = target.y;
            city.set_player_position(pos);
        }
        std::cout << "[probe] aabb do jogador w=" << city.player_aabb().w
                  << " h=" << city.player_aabb().h
                  << " base_y=" << (city.player_aabb().y + city.player_aabb().h) << "\n";
        std::cout << "[probe] jogador em (" << city.player_aabb().x << ", "
                  << city.player_aabb().y << ") celula ("
                  << city.grid().world_to_cell(city.player_aabb().x +
                                               city.player_aabb().w * 0.5f)
                  << ","
                  << city.grid().world_to_cell(city.player_aabb().y +
                                               city.player_aabb().h * 0.5f)
                  << ")\n";

        const int steps = env_int("AUDIT_STEPS", 8);
        for (int i = 0; i < steps; ++i) {
            if (!city.step()) break;
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }

        const std::string out = env_str("AUDIT_OUT", "/tmp/city_audit.png");
        if (!city.capture_frame_to_png(out)) {
            std::cerr << "capture_frame_to_png falhou: " << out << "\n";
            rc = 1;
        } else {
            std::cout << "[probe] PNG 1 => " << out << "\n";
        }
        if (const auto m = city.enemy_marker_aabb(); m.has_value()) {
            std::cout << "[probe] marcador do sentinela T1 x=" << m->x << " y=" << m->y
                      << "\n";
        }

        const int steps2 = env_int("AUDIT_STEPS2", 0);
        if (steps2 > 0) {
            for (int i = 0; i < steps2; ++i) {
                if (!city.step()) break;
                std::this_thread::sleep_for(std::chrono::milliseconds(16));
            }
            const std::string out2 = out.substr(0, out.size() - 4) + "_t2.png";
            if (!city.capture_frame_to_png(out2)) {
                std::cerr << "capture_frame_to_png falhou: " << out2 << "\n";
                rc = 1;
            } else {
                std::cout << "[probe] PNG 2 => " << out2 << "\n";
            }
            if (const auto m = city.enemy_marker_aabb(); m.has_value()) {
                std::cout << "[probe] marcador do sentinela T2 x=" << m->x
                          << " y=" << m->y << "\n";
            }
        }
    }

    SDL_GL_DestroyContext(gl);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return rc;
}

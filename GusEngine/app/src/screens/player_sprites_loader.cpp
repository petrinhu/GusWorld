// gus/app/src/screens/player_sprites_loader.cpp
//
// Ver header. Monta os caminhos dos PNGs do Caua e os carrega via IRenderer.
// O macro GUSWORLD_ASSETS_DIR (caminho absoluto do repo) e injetado pelo CMake.

#include "gus/app/screens/player_sprites_loader.hpp"

#include <array>
#include <cstdlib>  // std::getenv

#ifndef GUSWORLD_ASSETS_DIR
#define GUSWORLD_ASSETS_DIR ""
#endif

namespace gus::app::screens {

namespace {

// Nomes dos sprites neutro/idle, na ORDEM do enum Direction (Sul,Norte,Leste,Oeste).
constexpr std::array<const char*, kDirectionCount> kIdleFiles = {
    "south.png", "north.png", "east.png", "west.png"};

// Subpasta de walk por direcao, na MESMA ordem do enum Direction.
constexpr std::array<const char*, kDirectionCount> kWalkDirs = {
    "south", "north", "east", "west"};

std::string join(const std::string& a, const std::string& b) {
    if (a.empty()) {
        return b;
    }
    if (a.back() == '/') {
        return a + b;
    }
    return a + "/" + b;
}

}  // namespace

PlayerSpriteSet load_caua_sprites(gus::platform::render2d::IRenderer& renderer,
                                  const std::string& base_dir) {
    PlayerSpriteSet set;
    for (int d = 0; d < kDirectionCount; ++d) {
        // Neutro/idle: <base>/<dir>.png.
        const std::string idle_path = join(base_dir, kIdleFiles[d]);
        set.idle[d] = renderer.load_texture(idle_path.c_str());

        // Walk: <base>/walk/<dir>/<f>.png  (f = 0..kWalkFrameCount-1).
        for (int f = 0; f < kWalkFrameCount; ++f) {
            const std::string walk_path =
                join(join(join(base_dir, "walk"), kWalkDirs[d]),
                     std::to_string(f) + ".png");
            set.walk[d][f] = renderer.load_texture(walk_path.c_str());
        }
    }
    return set;
}

std::string resolve_caua_sprites_dir() {
    // 1) Override por ambiente (o lider aponta pra qualquer pasta).
    if (const char* env = std::getenv("GUSWORLD_ASSETS")) {
        if (env[0] != '\0') {
            return join(env, "sprites/caua_volt");
        }
    }
    // 2) Caminho do repo embutido em compilacao (raiz do repo).
    const std::string compiled = GUSWORLD_ASSETS_DIR;
    if (!compiled.empty()) {
        return join(compiled, "sprites/caua_volt");
    }
    // 3) Relativo ao CWD (rodando da raiz do repo).
    return "resources/sprites/caua_volt";
}

}  // namespace gus::app::screens

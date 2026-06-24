// gus/app/src/screens/city_scene.cpp
//
// Ver header. Montagem PURA da cena da cidade a partir de um TileMap + resolucao do
// CAMINHO do .gmap (env GUSWORLD_MAPS > macro embutido > relativo ao CWD, igual ao
// resolver de sprites do Caua/Gus). O I/O de BYTES do .gmap (ler o arquivo, load_map)
// fica na fronteira de quem chama (city_loader/main): aqui so montamos a STRING do
// caminho; nao abrimos arquivo nem tocamos SDL.

#include "gus/app/screens/city_scene.hpp"

#include <cstdlib>  // std::getenv
#include <utility>  // std::move

// Caminho ABSOLUTO da pasta de mapas compilados do repo, embutido pelo CMake (=
// GusEngine/assets/maps/compiled). Permite rodar do build dir sem CWD na raiz.
#ifndef GUSWORLD_MAPS_DIR
#define GUSWORLD_MAPS_DIR ""
#endif

namespace gus::app::screens {

namespace {

std::string join(const std::string& a, const std::string& b) {
    if (a.empty()) {
        return b;
    }
    if (a.back() == '/') {
        return a + b;
    }
    return a + "/" + b;
}

// Nome do arquivo do 1o mapa (Distritos Inferiores). Fonte: assets/maps/source.
constexpr const char* kDistritosInferioresGmap = "distritos_inferiores.gmap";

}  // namespace

gus::core::spatial::Aabb spawn_player_aabb(const gus::domain::map::TileMap& map) {
    const float ts = map.tile_size();
    const float side = kPlayerHitboxTileFraction * ts;
    const gus::domain::map::Cell s = map.spawn();
    // Centro da celula de spawn em mundo: (cx + 0.5)*ts. Canto sup-esq da hitbox =
    // centro - meia-hitbox, mantendo o jogador centrado na celula (nao colado a borda).
    const float center_x = (static_cast<float>(s.x) + 0.5f) * ts;
    const float center_y = (static_cast<float>(s.y) + 0.5f) * ts;
    return gus::core::spatial::Aabb{center_x - side * 0.5f, center_y - side * 0.5f,
                                    side, side};
}

OverworldSim make_city_scene(gus::domain::map::TileMap map,
                             const OverworldTuning& tuning) {
    const gus::core::spatial::Aabb start = spawn_player_aabb(map);
    // O ctor do TileMap deriva a colisao (to_tile_grid) e guarda o mapa pro render.
    return OverworldSim(std::move(map), start, tuning);
}

OverworldTuning make_city_tuning() {
    // Default do OverworldTuning (velocidade em TILES/s; o tile_size real do .gmap
    // entra via o grid, ajustando a velocidade em mundo). Ponto unico de feel.
    return OverworldTuning{};
}

std::string resolve_distritos_inferiores_gmap() {
    // 1) Override por ambiente (o lider aponta pra qualquer pasta de mapas).
    if (const char* env = std::getenv("GUSWORLD_MAPS")) {
        if (env[0] != '\0') {
            return join(env, kDistritosInferioresGmap);
        }
    }
    // 2) Caminho do repo embutido em compilacao (assets/maps/compiled do GusEngine).
    const std::string compiled = GUSWORLD_MAPS_DIR;
    if (!compiled.empty()) {
        return join(compiled, kDistritosInferioresGmap);
    }
    // 3) Relativo ao CWD (rodando da raiz do GusEngine).
    return join("assets/maps/compiled", kDistritosInferioresGmap);
}

}  // namespace gus::app::screens

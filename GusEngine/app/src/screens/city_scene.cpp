// gus/app/src/screens/city_scene.cpp
//
// Ver header. Montagem PURA da cena da cidade a partir de um TileMap + resolucao do
// CAMINHO do .gmap. O I/O de BYTES do .gmap (ler o arquivo, load_map) fica na fronteira
// de quem chama (city_loader/main): aqui so montamos a STRING do caminho; nao abrimos
// arquivo nem tocamos SDL.

#include "gus/app/screens/city_scene.hpp"

#include <utility>  // std::move

#include "gus/core/asset_paths.hpp"  // caminhos de asset centralizados (mapa .gmap)
#include "gus/platform/assets/asset_source.hpp"  // ASSETS-VFS-F1c (ADR-013): porteiro

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
    // ASSETS-VFS-F1c (ADR-013): a cadeia `env GUSWORLD_MAPS (pasta, junta so ao NOME do
    // arquivo) > macro GUSWORLD_MAPS_DIR > CWD (kMapsCompiledDir)` foi CONSOLIDADA em
    // FilesystemAssetSource::resolve_path (familia MAPS, dispatch pelo prefixo
    // "assets/maps/compiled/" do id, mesmo padrao das familias SFX/MUSICA). Assinatura/
    // contrato INTOCADOS - paridade provada em platform/tests/asset_source_test.cpp e
    // reforcada pelo smoke da cidade (city_loader.cpp carrega o .gmap real via o path
    // resolvido).
    const std::string id =
        join(std::string(gus::core::assets::kMapsCompiledDir),
             std::string(gus::core::assets::kDistritosInferioresGmapFile));
    return gus::platform::assets::FilesystemAssetSource().resolve_path(id);
}

}  // namespace gus::app::screens

// gus/app/screens/test_overworld.hpp
//
// Cena de teste HARDCODED do overworld do M1 (header-only): o mapa, a posicao/
// tamanho inicial do jogador e o TUNING (movimento + corner-assist + camera +
// cores). Fonte UNICA usada tanto pela janela real (GameWindow) quanto pelo smoke
// offscreen (main --smoke), pra que os dois exercitem exatamente a mesma cena.
// TUDO placeholder. Quando houver carregamento de mapa de verdade, isto sai.
//
// AJUSTE DE FEEL: o lider mexe em make_test_tuning() abaixo (e em OverworldTuning
// em overworld_tuning.hpp) - velocidade, corrida, corner-assist, normalize_diagonal,
// zoom, cores. Ponto unico.

#ifndef GUS_APP_SCREENS_TEST_OVERWORLD_HPP
#define GUS_APP_SCREENS_TEST_OVERWORLD_HPP

#include "gus/app/screens/overworld_tuning.hpp"
#include "gus/core/spatial/grid_collision.hpp"  // Aabb
#include "gus/core/spatial/tile_grid.hpp"

namespace gus::app::screens {

// Tamanho de uma celula em unidades de mundo (placeholder).
inline constexpr float kTestTileSize = 32.0f;

// Velocidade base de caminhada em TILES por segundo (placeholder; ~4.5).
inline constexpr float kTestWalkTilesPerSec = 4.5f;

// AABB inicial do jogador (canto sup-esq + w/h em mundo): ~0.6 tile, numa area
// livre. Placeholder no limite da hitbox (sem sprite).
inline constexpr gus::core::spatial::Aabb kTestPlayerStart{64.0f, 40.0f, 20.0f, 20.0f};

// Tuning da cena de teste. Default do OverworldTuning + a velocidade da cena.
// corner-assist JA LIGADO (default), normalize_diagonal DESLIGADO (default),
// camera_zoom 1.0 (default). O lider ajusta aqui sem reescrever nada.
inline OverworldTuning make_test_tuning() {
    OverworldTuning t;  // pega os defaults documentados em overworld_tuning.hpp
    t.walk_speed_tiles_per_sec = kTestWalkTilesPerSec;
    // corner-assist ligado com perdao ~0.35 do tile (default). Ajuste a gosto:
    //   t.corner.max_assist_fraction = 0.5f;   // perdoa mais
    //   t.corner.enabled = false;              // desliga o assist
    //   t.normalize_diagonal = true;           // diagonal com vel. das cardinais
    //   t.camera_zoom = 1.5f;                  // (gancho; zoom ainda nao ligado)
    return t;
}

// Mapa de teste: '#' = parede, '.' = chao. Origem (0,0) no canto superior-esquerdo,
// +X direita, +Y baixo. Tem blocos internos pro lider SENTIR o slide ao raspar.
inline gus::core::spatial::TileGrid make_test_map() {
    return gus::core::spatial::TileGrid::from_rows(
        {
            "###############",
            "#.............#",
            "#...#####.....#",
            "#.....#.......#",
            "#.....#...##..#",
            "#.........##..#",
            "#...####......#",
            "#.............#",
            "###############",
        },
        kTestTileSize);
}

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_TEST_OVERWORLD_HPP

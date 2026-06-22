// gus/app/screens/test_overworld.hpp
//
// Cena de teste HARDCODED do overworld do M1 (header-only): o mapa, a posicao/
// tamanho inicial do jogador e a velocidade. Fonte UNICA usada tanto pela janela
// real (GameWindow) quanto pelo smoke offscreen (main --smoke), pra que os dois
// exercitem exatamente a mesma cena. TUDO placeholder (o lider ajusta vendo: mapa,
// velocidade, cores). Quando houver carregamento de mapa de verdade, isto sai.

#ifndef GUS_APP_SCREENS_TEST_OVERWORLD_HPP
#define GUS_APP_SCREENS_TEST_OVERWORLD_HPP

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

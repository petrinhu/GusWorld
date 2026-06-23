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

    // ANCORAGEM PELOS PES = AUTOMATICA (M1-BUG.SUL, lider 2026-06-22): "colar o pe
    // na parede sozinho". O jogo MEDE a sobra transparente embaixo de cada sprite
    // (alpha-bbox no load) e desce o desenho ate o pe encostar na base da hitbox -
    // por personagem/direcao, SEM numero magico. (Antes era um 0.45 tile calculado a
    // mao do south.png 68x68; agora o automatico cobre todas as direcoes.)
    //
    // sprite_foot_offset_tiles fica so como AJUSTE FINO opcional, SOMADO por cima do
    // automatico (default 0 = so o automatico). O lider mexe aqui se quiser afundar/
    // levantar um tiquinho a gosto; nao precisa pra colar o pe.
    t.sprite_foot_offset_tiles = 0.0f;

    // Ajustes a gosto:
    //   t.corner.max_assist_fraction = 0.5f;   // perdoa mais na quina
    //   t.corner.enabled = false;              // desliga o corner-assist
    //   t.normalize_diagonal = true;           // diagonal com vel. das cardinais
    //   t.camera_zoom = 1.5f;                  // (gancho; zoom ainda nao ligado)
    //   t.diagonal_facing = DiagonalFacing::VerticalWins;  // N/S sempre na diagonal
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

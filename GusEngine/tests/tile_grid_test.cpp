// GusEngine/tests/tile_grid_test.cpp
//
// Spec executavel (Catch2 v3) do TileGrid: o modelo de mapa de grade do M4
// (logica pura, headless, sem Qt). TEST-FIRST: estes casos DEFINEM o contrato
// (M4 e design novo, nao ha C# pra portar).
//
// CONTRATO exercitado aqui:
//   - grade WxH de celulas, cada celula livre (false) ou bloqueada (true);
//   - is_blocked(cellX, cellY): celula fora dos limites = bloqueada (true);
//   - tile_size em unidades de mundo (float), origem em (0,0);
//   - mapeamento mundo->celula: floor(coord / tile_size).
//
// Cross-ref: docs/tech/pivot/engine-design.md secao 2 (core/ POCO puro).

#include <catch2/catch_test_macros.hpp>

#include "gus/core/spatial/tile_grid.hpp"

using gus::core::spatial::TileGrid;

TEST_CASE("TileGrid expoe dimensoes e tile_size", "[core][spatial][grid]") {
    TileGrid g(4, 3, 16.0f);
    REQUIRE(g.width() == 4);
    REQUIRE(g.height() == 3);
    REQUIRE(g.tile_size() == 16.0f);
}

TEST_CASE("TileGrid nasce todo livre", "[core][spatial][grid]") {
    TileGrid g(3, 3, 16.0f);
    for (int y = 0; y < 3; ++y) {
        for (int x = 0; x < 3; ++x) {
            REQUIRE_FALSE(g.is_blocked(x, y));
        }
    }
}

TEST_CASE("TileGrid set_blocked marca/desmarca celula", "[core][spatial][grid]") {
    TileGrid g(3, 3, 16.0f);
    g.set_blocked(1, 1, true);
    REQUIRE(g.is_blocked(1, 1));
    REQUIRE_FALSE(g.is_blocked(0, 0));

    g.set_blocked(1, 1, false);
    REQUIRE_FALSE(g.is_blocked(1, 1));
}

TEST_CASE("TileGrid trata fora-dos-limites como bloqueado", "[core][spatial][grid]") {
    TileGrid g(3, 3, 16.0f);
    SECTION("x negativo") { REQUIRE(g.is_blocked(-1, 0)); }
    SECTION("y negativo") { REQUIRE(g.is_blocked(0, -1)); }
    SECTION("x alem da largura") { REQUIRE(g.is_blocked(3, 0)); }
    SECTION("y alem da altura") { REQUIRE(g.is_blocked(0, 3)); }
    SECTION("canto distante") { REQUIRE(g.is_blocked(99, 99)); }
}

TEST_CASE("TileGrid construido a partir de linhas ASCII", "[core][spatial][grid]") {
    // '#' = bloqueado, '.' = livre. Conveniencia de fixture para os testes de
    // colisao; cada string e uma linha (eixo Y cresce para baixo).
    TileGrid g = TileGrid::from_rows({
        "...",
        ".#.",
        "...",
    }, 16.0f);
    REQUIRE(g.width() == 3);
    REQUIRE(g.height() == 3);
    REQUIRE(g.is_blocked(1, 1));
    REQUIRE_FALSE(g.is_blocked(0, 0));
    REQUIRE_FALSE(g.is_blocked(2, 2));
}

TEST_CASE("TileGrid mapeia coordenada de mundo para celula (floor)",
          "[core][spatial][grid]") {
    TileGrid g(4, 4, 16.0f);
    REQUIRE(g.world_to_cell(0.0f) == 0);
    REQUIRE(g.world_to_cell(15.9f) == 0);
    REQUIRE(g.world_to_cell(16.0f) == 1);
    REQUIRE(g.world_to_cell(31.0f) == 1);
    REQUIRE(g.world_to_cell(32.0f) == 2);
    SECTION("negativo arredonda para baixo (fora do mapa)") {
        REQUIRE(g.world_to_cell(-0.1f) == -1);
        REQUIRE(g.world_to_cell(-16.0f) == -1);
        REQUIRE(g.world_to_cell(-16.1f) == -2);
    }
}

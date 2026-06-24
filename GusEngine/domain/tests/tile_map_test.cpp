// tile_map_test.cpp
//
// Spec executavel (Catch2 v3) do TileMap (POCO do mapa de tiles do overworld) e da
// conversao TileMap -> core::spatial::TileGrid que a colisao do overworld consome.
//
// Oraculo:
//   (a) construtor/invariantes: dims, tile_size, matriz coerente, fail-fast;
//   (b) bordas/acesso: at/set, in_bounds, out_of_range fora dos limites;
//   (c) colisao: is_tile_blocking (so Parede bloqueia) + to_tile_grid fiel;
//   (d) metadados: spawn + portais validados.
//
// Subsistema: domain/map. POCO puro, ZERO Qt/SDL, headless.

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <stdexcept>

#include "gus/core/spatial/tile_grid.hpp"
#include "gus/domain/map/tile_map.hpp"

using gus::domain::map::Cell;
using gus::domain::map::is_tile_blocking;
using gus::domain::map::Portal;
using gus::domain::map::TileKind;
using gus::domain::map::TileMap;

namespace {
std::uint16_t k(TileKind t) { return static_cast<std::uint16_t>(t); }
}  // namespace

TEST_CASE("TileMap: construtor cria matriz coerente de Chao", "[map][tile_map]") {
    TileMap m(4, 3, 2.0f);
    REQUIRE(m.width() == 4);
    REQUIRE(m.height() == 3);
    REQUIRE(m.tile_size() == 2.0f);
    REQUIRE(m.tiles().size() == 12u);
    for (std::int32_t y = 0; y < 3; ++y)
        for (std::int32_t x = 0; x < 4; ++x)
            REQUIRE(m.at(x, y) == k(TileKind::Chao));
}

TEST_CASE("TileMap: construtor rejeita dims/tile_size invalidos", "[map][tile_map]") {
    REQUIRE_THROWS_AS(TileMap(0, 3, 1.0f), std::invalid_argument);
    REQUIRE_THROWS_AS(TileMap(3, 0, 1.0f), std::invalid_argument);
    REQUIRE_THROWS_AS(TileMap(-1, 3, 1.0f), std::invalid_argument);
    REQUIRE_THROWS_AS(TileMap(3, 3, 0.0f), std::invalid_argument);
    REQUIRE_THROWS_AS(TileMap(3, 3, -1.0f), std::invalid_argument);
}

TEST_CASE("TileMap: at/set respeitam bordas (out_of_range fora)", "[map][tile_map]") {
    TileMap m(3, 2, 1.0f);
    m.set(2, 1, k(TileKind::Parede));
    REQUIRE(m.at(2, 1) == k(TileKind::Parede));
    REQUIRE(m.in_bounds(0, 0));
    REQUIRE(m.in_bounds(2, 1));
    REQUIRE_FALSE(m.in_bounds(3, 0));
    REQUIRE_FALSE(m.in_bounds(0, 2));
    REQUIRE_FALSE(m.in_bounds(-1, 0));
    REQUIRE_THROWS_AS(m.at(3, 0), std::out_of_range);
    REQUIRE_THROWS_AS(m.at(0, 2), std::out_of_range);
    REQUIRE_THROWS_AS(m.set(-1, 0, 0), std::out_of_range);
}

TEST_CASE("TileMap: index row-major (y*width+x)", "[map][tile_map]") {
    TileMap m(3, 3, 1.0f);
    // marca uma diagonal e confere o layout linear esperado.
    m.set(0, 0, 1);
    m.set(1, 1, 2);
    m.set(2, 2, 3);
    const auto& t = m.tiles();
    REQUIRE(t[0] == 1);  // (0,0)
    REQUIRE(t[4] == 2);  // (1,1) = 1*3+1
    REQUIRE(t[8] == 3);  // (2,2) = 2*3+2
}

TEST_CASE("is_tile_blocking: so Parede bloqueia", "[map][tile_map][colisao]") {
    REQUIRE_FALSE(is_tile_blocking(k(TileKind::Chao)));
    REQUIRE(is_tile_blocking(k(TileKind::Parede)));
    REQUIRE_FALSE(is_tile_blocking(k(TileKind::Marco)));
    REQUIRE_FALSE(is_tile_blocking(k(TileKind::Entrada)));
    REQUIRE_FALSE(is_tile_blocking(k(TileKind::Saida)));
    // id reservado ao futuro = andavel (nao bloqueia) ate ganhar semantica.
    REQUIRE_FALSE(is_tile_blocking(999));
}

TEST_CASE("TileMap::to_tile_grid replica dims + bloqueio fiel", "[map][tile_map][colisao]") {
    TileMap m(3, 2, 4.0f);
    m.set(1, 0, k(TileKind::Parede));
    m.set(2, 1, k(TileKind::Parede));
    m.set(0, 1, k(TileKind::Marco));  // marco NAO bloqueia

    const auto grid = m.to_tile_grid();
    REQUIRE(grid.width() == 3);
    REQUIRE(grid.height() == 2);
    REQUIRE(grid.tile_size() == 4.0f);

    REQUIRE_FALSE(grid.is_blocked(0, 0));
    REQUIRE(grid.is_blocked(1, 0));
    REQUIRE_FALSE(grid.is_blocked(2, 0));
    REQUIRE_FALSE(grid.is_blocked(0, 1));  // marco livre
    REQUIRE_FALSE(grid.is_blocked(1, 1));
    REQUIRE(grid.is_blocked(2, 1));
    // borda implicita do TileGrid = parede.
    REQUIRE(grid.is_blocked(-1, 0));
    REQUIRE(grid.is_blocked(3, 0));
}

TEST_CASE("TileMap: spawn + portais validados", "[map][tile_map][meta]") {
    TileMap m(5, 5, 1.0f);
    m.set_spawn(Cell{2, 3});
    m.add_portal(Portal{"saida_sul", Cell{2, 4}});
    REQUIRE(m.spawn() == Cell{2, 3});
    REQUIRE(m.portals().size() == 1u);
    REQUIRE(m.portals()[0].id == "saida_sul");
    REQUIRE_NOTHROW(m.validate());
}

TEST_CASE("TileMap::validate rejeita spawn/portal fora dos limites", "[map][tile_map][meta]") {
    {
        TileMap m(3, 3, 1.0f);
        m.set_spawn(Cell{3, 0});  // fora
        REQUIRE_THROWS_AS(m.validate(), std::invalid_argument);
    }
    {
        TileMap m(3, 3, 1.0f);
        m.add_portal(Portal{"p", Cell{0, 9}});  // fora
        REQUIRE_THROWS_AS(m.validate(), std::invalid_argument);
    }
    {
        TileMap m(3, 3, 1.0f);
        m.add_portal(Portal{"", Cell{0, 0}});  // id vazio
        REQUIRE_THROWS_AS(m.validate(), std::invalid_argument);
    }
}

TEST_CASE("TileMap: igualdade compara matriz + metadados", "[map][tile_map]") {
    TileMap a(2, 2, 1.0f);
    TileMap b(2, 2, 1.0f);
    REQUIRE(a == b);
    b.set(0, 0, 1);
    REQUIRE_FALSE(a == b);
    a.set(0, 0, 1);
    REQUIRE(a == b);
    a.set_spawn(Cell{1, 1});
    REQUIRE_FALSE(a == b);
}

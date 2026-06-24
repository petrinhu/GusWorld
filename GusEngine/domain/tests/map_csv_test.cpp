// map_csv_test.cpp
//
// Spec executavel (Catch2 v3) do parser POCO de CSV -> TileMap (compilador de mapa,
// lado puro). O CSV e uma grade de numeros separados por virgula: cada LINHA do
// texto = uma LINHA do mapa (eixo Y para baixo), cada numero = um tile-id.
//
// Metadados opcionais por linhas-diretiva com prefixo '#':
//   #tile_size <float>     lado da celula em unidades de mundo (default 1.0)
//   #spawn <x> <y>         celula de spawn do player
//   #portal <id> <x> <y>   portal/saida nomeada
// Linhas em branco e comentarios '//' sao ignorados.
//
// Oraculo:
//   (a) parse feliz: grade NxM coerente -> TileMap com matriz/spawn/portais certos;
//   (b) malformado: linha de tamanho errado -> erro tipado com numero da linha;
//   (c) malformado: numero invalido / negativo / fora de uint16 -> erro tipado;
//   (d) diretivas: tile_size/spawn/portal aplicados; sem grade -> erro.
//
// Subsistema: domain/map. POCO puro, ZERO Qt/SDL, ZERO I/O (opera sobre string).

#include <catch2/catch_test_macros.hpp>

#include <string>

#include "gus/domain/map/map_csv.hpp"
#include "gus/domain/map/tile_map.hpp"

using gus::domain::map::Cell;
using gus::domain::map::MapCsvError;
using gus::domain::map::parse_csv_to_tilemap;
using gus::domain::map::TileKind;
using gus::domain::map::TileMap;

namespace {
std::uint16_t k(TileKind t) { return static_cast<std::uint16_t>(t); }
}  // namespace

TEST_CASE("map_csv: parse feliz de grade 3x2", "[map][csv]") {
    const std::string csv =
        "1,1,1\n"
        "1,0,4\n";
    const TileMap m = parse_csv_to_tilemap(csv);
    REQUIRE(m.width() == 3);
    REQUIRE(m.height() == 2);
    REQUIRE(m.at(0, 0) == 1);
    REQUIRE(m.at(2, 0) == 1);
    REQUIRE(m.at(1, 1) == 0);
    REQUIRE(m.at(2, 1) == k(TileKind::Saida));
}

TEST_CASE("map_csv: diretivas tile_size/spawn/portal", "[map][csv][meta]") {
    const std::string csv =
        "#tile_size 2.0\n"
        "#spawn 1 1\n"
        "#portal saida_sul 2 1\n"
        "0,0,0\n"
        "0,0,4\n";
    const TileMap m = parse_csv_to_tilemap(csv);
    REQUIRE(m.tile_size() == 2.0f);
    REQUIRE(m.spawn() == Cell{1, 1});
    REQUIRE(m.portals().size() == 1u);
    REQUIRE(m.portals()[0].id == "saida_sul");
    REQUIRE(m.portals()[0].cell == Cell{2, 1});
}

TEST_CASE("map_csv: ignora linhas em branco e comentarios //", "[map][csv]") {
    const std::string csv =
        "// mapa de teste\n"
        "\n"
        "1,1\n"
        "\n"
        "0,0\n";
    const TileMap m = parse_csv_to_tilemap(csv);
    REQUIRE(m.width() == 2);
    REQUIRE(m.height() == 2);
}

TEST_CASE("map_csv: linha de largura errada -> erro com numero da linha",
          "[map][csv][malformado]") {
    const std::string csv =
        "0,0,0\n"
        "0,0\n";  // 2 colunas, esperado 3
    REQUIRE_THROWS_AS(parse_csv_to_tilemap(csv), MapCsvError);
}

TEST_CASE("map_csv: numero invalido -> erro", "[map][csv][malformado]") {
    REQUIRE_THROWS_AS(parse_csv_to_tilemap("0,abc,0\n0,0,0\n"), MapCsvError);
    REQUIRE_THROWS_AS(parse_csv_to_tilemap("0,,0\n0,0,0\n"), MapCsvError);  // celula vazia
    REQUIRE_THROWS_AS(parse_csv_to_tilemap("0,-1,0\n0,0,0\n"), MapCsvError);  // negativo
    REQUIRE_THROWS_AS(parse_csv_to_tilemap("0,99999999,0\n"), MapCsvError);  // > uint16
}

TEST_CASE("map_csv: sem nenhuma linha de grade -> erro", "[map][csv][malformado]") {
    REQUIRE_THROWS_AS(parse_csv_to_tilemap("#tile_size 2.0\n"), MapCsvError);
    REQUIRE_THROWS_AS(parse_csv_to_tilemap(""), MapCsvError);
}

TEST_CASE("map_csv: spawn fora dos limites -> erro (via validate)", "[map][csv][malformado]") {
    const std::string csv =
        "#spawn 5 0\n"
        "0,0\n"
        "0,0\n";
    REQUIRE_THROWS_AS(parse_csv_to_tilemap(csv), std::invalid_argument);
}

TEST_CASE("map_csv: roundtrip CSV -> TileMap -> grid coerente", "[map][csv][colisao]") {
    const std::string csv =
        "1,1,1\n"
        "1,0,1\n"
        "1,1,1\n";
    const TileMap m = parse_csv_to_tilemap(csv);
    const auto grid = m.to_tile_grid();
    REQUIRE(grid.is_blocked(0, 0));
    REQUIRE_FALSE(grid.is_blocked(1, 1));  // unica celula livre (Chao)
    REQUIRE(grid.is_blocked(2, 2));
}

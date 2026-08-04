// SPDX-License-Identifier: Apache-2.0
// GusEngine/app/tests/city_actors_test.cpp
//
// Catch2 de QUEM POVOA A CIDADE (DEMO-CIDADE-VESTIDA fatia C, a costura).
//
// A fatia B deu ao overworld uma LISTA de atores no lugar dos dois slots fixos, e
// a ronda em rota fixa. O que faltava era o dado: quem está no mapa, onde, e
// andando por onde. Esta spec protege exatamente esse dado.
//
// O defeito que ela existe para pegar é o mesmo das peças de cenário, e é
// silencioso: o mapa muda, a célula do ator vira parede, o ator some, e o jogo
// não reclama - a praça só fica vazia. Por isso a seção final carrega o .gmap
// REAL e exige o elenco inteiro de pé, com as rondas que a tabela declara.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

#include "gus/app/screens/city_actors.hpp"
#include "gus/app/screens/city_loader.hpp"
#include "gus/app/screens/city_props.hpp"
#include "gus/core/spatial/tile_grid.hpp"
#include "gus/domain/map/tile_map.hpp"
#include "gus/platform/assets/asset_source.hpp"

using Catch::Matchers::WithinAbs;
using gus::app::screens::CityActorRow;
using gus::app::screens::kDistritosInferioresCast;
using gus::app::screens::kDistritosInferioresCastCount;
using gus::app::screens::kDistritosInferioresDressing;
using gus::app::screens::kDistritosInferioresDressingCount;
using gus::app::screens::resolve_city_actors;
using gus::app::screens::WorldActorRole;
using gus::core::spatial::TileGrid;

namespace {

constexpr double kEps = 1e-3;
constexpr float kTile = 2.0f;  // mesma escala do .gmap real da cidade

TileGrid open_grid() { return TileGrid(20, 20, kTile); }

// Linha de teste: papel e arte não importam para a resolução geométrica, então
// ficam num único helper para as tabelas de teste dizerem só o que interessa.
CityActorRow row_at(int cx, int cy, int reach) {
    return CityActorRow{WorldActorRole::Npc, "sprites/x", "south.png", cx, cy, reach};
}

}  // namespace

// ===========================================================================
//  Resolução da tabela de elenco contra a grade
// ===========================================================================

TEST_CASE("elenco: ator em célula andável entra com a célula que a tabela pediu",
          "[city-actors]") {
    const TileGrid g = open_grid();
    const CityActorRow rows[] = {row_at(7, 9, 0)};
    const auto out = resolve_city_actors(g, rows, 1);

    REQUIRE(out.size() == 1);
    REQUIRE(out[0].cell_x == 7);
    REQUIRE(out[0].cell_y == 9);
    REQUIRE(out[0].role == WorldActorRole::Npc);
}

TEST_CASE("elenco: ator dentro de parede é descartado", "[city-actors]") {
    // Personagem dentro do prédio é pior que peça dentro do prédio: ele mexe, e o
    // jogador vê meio corpo saindo da fachada.
    TileGrid g = open_grid();
    g.set_blocked(7, 9, true);
    const CityActorRow rows[] = {row_at(7, 9, 0), row_at(8, 9, 0)};
    const auto out = resolve_city_actors(g, rows, 2);
    REQUIRE(out.size() == 1);
    REQUIRE(out[0].cell_x == 8);
}

TEST_CASE("elenco: ator fora do mapa é descartado", "[city-actors]") {
    const TileGrid g = open_grid();
    const CityActorRow rows[] = {row_at(-1, 5, 0), row_at(20, 5, 0), row_at(5, 20, 0),
                                 row_at(5, 5, 0)};
    const auto out = resolve_city_actors(g, rows, 4);
    REQUIRE(out.size() == 1);
    REQUIRE(out[0].cell_x == 5);
}

TEST_CASE("elenco: tabela vazia devolve lista vazia sem estourar", "[city-actors]") {
    const TileGrid g = open_grid();
    REQUIRE(resolve_city_actors(g, nullptr, 0).empty());
    const CityActorRow rows[] = {row_at(5, 5, 0)};
    REQUIRE(resolve_city_actors(g, rows, 0).empty());
}

// ===========================================================================
//  A ronda de cada ator, construída contra o mapa
// ===========================================================================

TEST_CASE("elenco: alcance zero deixa o ator parado, não some com ele",
          "[city-actors]") {
    const TileGrid g = open_grid();
    const CityActorRow rows[] = {row_at(7, 9, 0)};
    const auto out = resolve_city_actors(g, rows, 1);
    REQUIRE(out.size() == 1);
    REQUIRE_FALSE(out[0].route.valid());  // parado é um estado legítimo
}

TEST_CASE("elenco: alcance positivo em rua aberta vira ronda de ida e volta",
          "[city-actors]") {
    const TileGrid g = open_grid();
    const CityActorRow rows[] = {row_at(10, 9, 3)};
    const auto out = resolve_city_actors(g, rows, 1);
    REQUIRE(out.size() == 1);
    REQUIRE(out[0].route.valid());
    REQUIRE(out[0].route.count == 2);
    REQUIRE_THAT(out[0].route.waypoints[0].x, WithinAbs(7.0, kEps));
    REQUIRE_THAT(out[0].route.waypoints[1].x, WithinAbs(13.0, kEps));
    REQUIRE_THAT(out[0].route.waypoints[0].y, WithinAbs(9.0, kEps));
}

TEST_CASE("elenco: a ronda encurta na parede em vez de atravessá-la",
          "[city-actors]") {
    TileGrid g = open_grid();
    g.set_blocked(12, 9, true);
    const CityActorRow rows[] = {row_at(10, 9, 5)};
    const auto out = resolve_city_actors(g, rows, 1);
    REQUIRE(out.size() == 1);
    REQUIRE(out[0].route.valid());
    REQUIRE_THAT(out[0].route.waypoints[1].x, WithinAbs(11.0, kEps));
}

TEST_CASE("elenco: sem espaço para os dois lados o ator fica parado, e fica",
          "[city-actors]") {
    // Degradação que importa: rota impossível não pode custar o personagem. Ele
    // monta guarda no lugar, que é o comportamento de antes desta fatia.
    TileGrid g = open_grid();
    g.set_blocked(9, 9, true);
    g.set_blocked(11, 9, true);
    const CityActorRow rows[] = {row_at(10, 9, 4)};
    const auto out = resolve_city_actors(g, rows, 1);
    REQUIRE(out.size() == 1);
    REQUIRE_FALSE(out[0].route.valid());
}

// ===========================================================================
//  A tabela real, e o mapa real
// ===========================================================================

TEST_CASE("elenco: a tabela não põe dois atores na mesma célula", "[city-actors]") {
    for (int i = 0; i < kDistritosInferioresCastCount; ++i) {
        for (int j = i + 1; j < kDistritosInferioresCastCount; ++j) {
            const bool mesmo =
                kDistritosInferioresCast[i].cell_x ==
                    kDistritosInferioresCast[j].cell_x &&
                kDistritosInferioresCast[i].cell_y ==
                    kDistritosInferioresCast[j].cell_y;
            REQUIRE_FALSE(mesmo);
        }
    }
}

TEST_CASE("elenco: nenhum ator nasce em cima de uma peça de cenário",
          "[city-actors]") {
    // Ator e casa na mesma célula é arte sobreposta no melhor caso, e personagem
    // enterrado no pior - e some justo aquele com quem o jogador precisa esbarrar.
    for (int i = 0; i < kDistritosInferioresCastCount; ++i) {
        for (int j = 0; j < kDistritosInferioresDressingCount; ++j) {
            const bool mesmo = kDistritosInferioresCast[i].cell_x ==
                                   kDistritosInferioresDressing[j].cell_x &&
                               kDistritosInferioresCast[i].cell_y ==
                                   kDistritosInferioresDressing[j].cell_y;
            REQUIRE_FALSE(mesmo);
        }
    }
}

TEST_CASE("elenco: toda linha da tabela aponta para uma arte QUE EXISTE",
          "[city-actors]") {
    // Caminho de asset nunca literal solto: cada linha traz pasta + arquivo, os
    // dois vindos de core/asset_paths.hpp. E o par tem que resolver para um
    // arquivo de verdade - sem isto, a única sinalização de figurante sem arte é
    // um aviso no terminal que ninguém lê, e a cidade fica com menos gente.
    REQUIRE(kDistritosInferioresCastCount > 0);
    for (int i = 0; i < kDistritosInferioresCastCount; ++i) {
        const CityActorRow& row = kDistritosInferioresCast[i];
        REQUIRE_FALSE(row.sprite_dir.empty());
        REQUIRE_FALSE(row.sprite_file.empty());
        const std::string id =
            std::string(row.sprite_dir) + "/" + std::string(row.sprite_file);
        const std::string path =
            gus::platform::assets::FilesystemAssetSource().resolve_path(id);
        INFO("figurante " << i << " -> " << path);
        REQUIRE(std::filesystem::exists(path));
    }
}

namespace {

gus::app::screens::CityLoadOutcome load_real_city() {
    gus::app::screens::CityLoadOutcome city =
        gus::app::screens::load_city_or_fallback();
    REQUIRE(city.status == gus::app::screens::CityLoadStatus::CityOk);
    return city;
}

}  // namespace

TEST_CASE("cidade real: o elenco inteiro cabe no mapa carregado",
          "[city-actors][mapa-real]") {
    const gus::app::screens::CityLoadOutcome city = load_real_city();
    const auto out = resolve_city_actors(city.sim.grid(), kDistritosInferioresCast,
                                         kDistritosInferioresCastCount);
    REQUIRE(static_cast<int>(out.size()) == kDistritosInferioresCastCount);
}

TEST_CASE("cidade real: toda ronda declarada nasce válida",
          "[city-actors][mapa-real]") {
    // Alcance escrito na tabela e rota inválida no mapa significa que o espaço de
    // patrulha não existe onde o level design disse que existia. É um erro de
    // dado, e sem este teste ele aparece como "o inimigo não anda".
    const gus::app::screens::CityLoadOutcome city = load_real_city();
    const auto out = resolve_city_actors(city.sim.grid(), kDistritosInferioresCast,
                                         kDistritosInferioresCastCount);
    REQUIRE(out.size() == static_cast<std::size_t>(kDistritosInferioresCastCount));
    for (int i = 0; i < kDistritosInferioresCastCount; ++i) {
        if (kDistritosInferioresCast[i].patrol_reach_tiles > 0) {
            REQUIRE(out[static_cast<std::size_t>(i)].route.valid());
        }
    }
}

TEST_CASE("cidade real: as células de ator com papel na história são andáveis",
          "[city-actors][mapa-real]") {
    // O Bertoldo e o androide sentinela não vêm da tabela de figurantes: eles têm
    // diálogo e batalha, e quem os posiciona é a Maestro. As células deles moram
    // aqui do mesmo jeito, e precisam valer contra o mapa real igual às outras.
    const gus::app::screens::CityLoadOutcome city = load_real_city();
    const gus::core::spatial::TileGrid& g = city.sim.grid();
    REQUIRE_FALSE(g.is_blocked(gus::app::screens::kBertoldoBenchCellX,
                               gus::app::screens::kBertoldoBenchCellY));
    REQUIRE_FALSE(g.is_blocked(gus::app::screens::kSentinelArenaCellX,
                               gus::app::screens::kSentinelArenaCellY));
}

TEST_CASE("cidade real: nenhum ator nasce em cima do jogador",
          "[city-actors][mapa-real]") {
    const gus::app::screens::CityLoadOutcome city = load_real_city();
    const gus::domain::map::Cell spawn = city.sim.tile_map()->spawn();
    for (int i = 0; i < kDistritosInferioresCastCount; ++i) {
        const bool em_cima = kDistritosInferioresCast[i].cell_x == spawn.x &&
                             kDistritosInferioresCast[i].cell_y == spawn.y;
        REQUIRE_FALSE(em_cima);
    }
    const bool bertoldo_no_spawn =
        gus::app::screens::kBertoldoBenchCellX == spawn.x &&
        gus::app::screens::kBertoldoBenchCellY == spawn.y;
    const bool sentinela_no_spawn =
        gus::app::screens::kSentinelArenaCellX == spawn.x &&
        gus::app::screens::kSentinelArenaCellY == spawn.y;
    REQUIRE_FALSE(bertoldo_no_spawn);
    REQUIRE_FALSE(sentinela_no_spawn);
}

// SPDX-License-Identifier: Apache-2.0
// GusEngine/app/tests/city_props_test.cpp
//
// Catch2 do VESTIR DA CIDADE (DEMO-CIDADE-VESTIDA fatia B): a tradução entre o
// DADO do mundo (gus::domain::world) e as instâncias que o OverworldSim desenha e
// colide, mais a construção da rota de ronda contra o mapa real. TEST-FIRST,
// headless (o único toque em plataforma é um IRenderer falso).
//
// O que esta spec protege: a tabela de vestimenta é escrita à mão por quem faz
// level design, e mapa muda (o mapa desta cidade já foi redesenhado uma vez, de
// 30x20 para 90x60). Célula inválida na tabela não pode virar casa dentro de
// parede, prop fora do mapa nem crash - tem que virar peça descartada, com aviso.
//
// FATIA C (a costura): a tabela deixou de ser relativa ao spawn e passou a ser
// ABSOLUTA, derivada das 34 âncoras do doc de blockout. Por isso a seção final
// desta spec carrega o .gmap REAL e exige que a cidade inteira caiba nele - é o
// teste que falha no dia em que alguém mexer no mapa e esquecer da vestimenta.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <cstdint>
#include <vector>

#include "gus/app/screens/city_loader.hpp"
#include "gus/domain/map/tile_map.hpp"
#include "gus/app/screens/city_patrol.hpp"
#include "gus/app/screens/city_props.hpp"
#include "gus/core/spatial/tile_grid.hpp"
#include "gus/platform/render2d/i_renderer.hpp"

using Catch::Matchers::WithinAbs;
using gus::app::screens::build_horizontal_patrol_route;
using gus::app::screens::build_scene_prop_instances;
using gus::app::screens::CityPropRow;
using gus::app::screens::kDistritosInferioresDressing;
using gus::app::screens::kDistritosInferioresDressingCount;
using gus::app::screens::load_scene_prop_textures;
using gus::app::screens::PropCellRule;
using gus::app::screens::resolve_city_props;
using gus::app::screens::ScenePropTextures;
using gus::core::spatial::Aabb;
using gus::core::spatial::Rect;
using gus::core::spatial::TileGrid;
using gus::domain::world::kScenePropKindCount;
using gus::domain::world::ScenePropKind;
using gus::domain::world::ScenePropPlacement;
using gus::platform::render2d::DrawColor;
using gus::platform::render2d::IRenderer;
using gus::platform::render2d::kInvalidTexture;
using gus::platform::render2d::TextureId;
using gus::platform::render2d::UvRect;

namespace {

constexpr double kEps = 1e-3;
constexpr float kTile = 2.0f;  // mesma escala do .gmap real da cidade

// IRenderer falso que só serve para carregar textura: devolve um id novo a cada
// caminho pedido, ou inválido se instruído a "não achar" nada.
class StubRenderer : public IRenderer {
public:
    explicit StubRenderer(bool assets_present) : present_(assets_present) {}

    void begin_frame(const Rect&, int, int) override {}
    void draw_filled_rect(const Rect&, const DrawColor&) override {}
    void draw_rect_outline(const Rect&, const DrawColor&, float) override {}
    TextureId load_texture(const char* path) override {
        paths.emplace_back(path);
        return present_ ? ++next_texture : kInvalidTexture;
    }
    void draw_textured_rect(const Rect&, TextureId, const UvRect&,
                            const DrawColor&) override {}
    gus::platform::render2d::ContentBbox texture_content_bbox(
        TextureId) const override {
        return gus::platform::render2d::ContentBbox{};
    }
    void draw_text(const char*, float, float, float, const DrawColor&, bool) override {}
    void end_frame() override {}

    bool present_;
    TextureId next_texture = kInvalidTexture;
    std::vector<std::string> paths;
};

// Grade 20x20 aberta, tile 2.0 - o mesmo tile_size do mapa real.
TileGrid open_grid() { return TileGrid(20, 20, kTile); }

}  // namespace

// ===========================================================================
//  Resolução da tabela de vestimenta
// ===========================================================================

TEST_CASE("vestir: linha absoluta vira placement na MESMA célula", "[city-props]") {
    const TileGrid g = open_grid();
    const CityPropRow rows[] = {
        {ScenePropKind::CasaCibergoticaA, 8, 3, PropCellRule::WalkableCell},
        {ScenePropKind::PosteNeonCiano, 4, 9, PropCellRule::WalkableCell},
    };
    const auto out = resolve_city_props(g, rows, 2);

    REQUIRE(out.size() == 2);
    REQUIRE(out[0].kind == ScenePropKind::CasaCibergoticaA);
    REQUIRE(out[0].cell_x == 8);
    REQUIRE(out[0].cell_y == 3);
    REQUIRE(out[1].cell_x == 4);
    REQUIRE(out[1].cell_y == 9);
}

TEST_CASE("vestir: peça que cai fora do mapa é descartada", "[city-props]") {
    const TileGrid g = open_grid();
    const CityPropRow rows[] = {
        {ScenePropKind::CasaCibergoticaA, 105, 5, PropCellRule::WalkableCell},
        {ScenePropKind::PosteNeonCiano, 5, -94, PropCellRule::WalkableCell},
        {ScenePropKind::FonteLatao, 6, 6, PropCellRule::WalkableCell},  // válida
    };
    const auto out = resolve_city_props(g, rows, 3);
    REQUIRE(out.size() == 1);
    REQUIRE(out[0].kind == ScenePropKind::FonteLatao);
}

TEST_CASE("vestir: peça de RUA que cai dentro de parede é descartada",
          "[city-props]") {
    // Casa dentro de parede é o erro clássico de tabela escrita à mão, e o mais
    // difícil de ver no playtest: a arte fica meio enterrada e ninguém percebe
    // que aquele tile era chão.
    TileGrid g = open_grid();
    g.set_blocked(8, 5, true);
    const CityPropRow rows[] = {
        {ScenePropKind::CasaCibergoticaA, 8, 5, PropCellRule::WalkableCell},
        {ScenePropKind::PosteNeonCiano, 7, 5, PropCellRule::WalkableCell},
    };
    const auto out = resolve_city_props(g, rows, 2);
    REQUIRE(out.size() == 1);
    REQUIRE(out[0].kind == ScenePropKind::PosteNeonCiano);
}

TEST_CASE("vestir: peça que É a parede exige célula de parede", "[city-props]") {
    // A caixa de cobertura não fica ao LADO de um obstáculo: ela É o obstáculo -
    // uma célula de Parede solta desenhada como caixa (convenção do blockout, §2).
    // Validá-la com a régua da peça de rua a descartaria sempre, e a arena
    // ficaria sem cobertura nenhuma sem ninguém notar.
    TileGrid g = open_grid();
    g.set_blocked(9, 9, true);
    const CityPropRow rows[] = {
        {ScenePropKind::CoverBox, 9, 9, PropCellRule::WallCell},  // certa
        {ScenePropKind::CoverBox, 3, 3, PropCellRule::WallCell},  // chão: descartada
    };
    const auto out = resolve_city_props(g, rows, 2);
    REQUIRE(out.size() == 1);
    REQUIRE(out[0].cell_x == 9);
    REQUIRE(out[0].cell_y == 9);
}

TEST_CASE("vestir: peça de parede FORA do mapa também é descartada",
          "[city-props]") {
    // Fora dos limites o TileGrid responde "bloqueado" (a borda é parede
    // implícita). Sem um teste de limites próprio, a régua da peça-parede
    // aceitaria qualquer célula do lado de fora do mapa.
    const TileGrid g = open_grid();
    const CityPropRow rows[] = {
        {ScenePropKind::CoverBox, -1, 5, PropCellRule::WallCell},
        {ScenePropKind::CoverBox, 20, 5, PropCellRule::WallCell},
        {ScenePropKind::CoverBox, 5, -1, PropCellRule::WallCell},
        {ScenePropKind::CoverBox, 5, 20, PropCellRule::WallCell},
    };
    REQUIRE(resolve_city_props(g, rows, 4).empty());
}

TEST_CASE("vestir: tabela vazia devolve lista vazia sem estourar", "[city-props]") {
    const TileGrid g = open_grid();
    REQUIRE(resolve_city_props(g, nullptr, 0).empty());
    const CityPropRow rows[] = {
        {ScenePropKind::CasaCibergoticaA, 1, 1, PropCellRule::WalkableCell}};
    REQUIRE(resolve_city_props(g, rows, 0).empty());
}

TEST_CASE("vestir: a tabela da cidade é toda de peças conhecidas", "[city-props]") {
    REQUIRE(kDistritosInferioresDressingCount > 0);
    for (int i = 0; i < kDistritosInferioresDressingCount; ++i) {
        const int k = static_cast<int>(kDistritosInferioresDressing[i].kind);
        REQUIRE(k >= 0);
        REQUIRE(k < kScenePropKindCount);
    }
}

TEST_CASE("vestir: a tabela da cidade não põe duas peças na mesma célula",
          "[city-props]") {
    // Duas peças no mesmo lugar é erro de autoria que só aparece no playtest
    // como arte sobreposta (ou pior, como duas caixas de colisão empilhadas).
    for (int i = 0; i < kDistritosInferioresDressingCount; ++i) {
        for (int j = i + 1; j < kDistritosInferioresDressingCount; ++j) {
            const bool mesmo =
                kDistritosInferioresDressing[i].cell_x ==
                    kDistritosInferioresDressing[j].cell_x &&
                kDistritosInferioresDressing[i].cell_y ==
                    kDistritosInferioresDressing[j].cell_y;
            REQUIRE_FALSE(mesmo);
        }
    }
}

// ===========================================================================
//  Construção das instâncias (dado + textura -> o que o sim desenha)
// ===========================================================================

TEST_CASE("vestir: instância recebe a geometria que o catálogo mandou",
          "[city-props]") {
    StubRenderer r(/*assets_present=*/true);
    const ScenePropTextures tex = load_scene_prop_textures(r);
    const ScenePropPlacement places[] = {{ScenePropKind::CasaCibergoticaA, 6, 4}};
    const auto out = build_scene_prop_instances(places, 1, kTile, /*scale=*/1.0f, tex);

    REQUIRE(out.size() == 1);
    // Casa de 3x3 tiles: base centrada na célula (6,4).
    REQUIRE_THAT(out[0].footprint.w, WithinAbs(3.0 * kTile, kEps));
    REQUIRE_THAT(out[0].footprint.h, WithinAbs(3.0 * kTile, kEps));
    REQUIRE_THAT(out[0].footprint.x + out[0].footprint.w * 0.5f,
                 WithinAbs((6.0 + 0.5) * kTile, kEps));
    REQUIRE_THAT(out[0].footprint.y + out[0].footprint.h,
                 WithinAbs((4.0 + 1.0) * kTile, kEps));
    REQUIRE(out[0].blocks());
}

TEST_CASE("vestir: a escala global chega na instância", "[city-props]") {
    StubRenderer r(true);
    const ScenePropTextures tex = load_scene_prop_textures(r);
    const ScenePropPlacement places[] = {{ScenePropKind::CasaCibergoticaA, 6, 4}};
    const auto um = build_scene_prop_instances(places, 1, kTile, 1.0f, tex);
    const auto dois = build_scene_prop_instances(places, 1, kTile, 2.0f, tex);
    REQUIRE_THAT(dois[0].footprint.h, WithinAbs(um[0].footprint.h * 2.0, kEps));
    // A base não sai do chão ao escalar.
    REQUIRE_THAT(dois[0].footprint.y + dois[0].footprint.h,
                 WithinAbs(um[0].footprint.y + um[0].footprint.h, kEps));
}

TEST_CASE("vestir: peça de chão sai marcada como chão", "[city-props]") {
    StubRenderer r(true);
    const ScenePropTextures tex = load_scene_prop_textures(r);
    const ScenePropPlacement places[] = {{ScenePropKind::BoardPuzzle, 6, 4}};
    const auto out = build_scene_prop_instances(places, 1, kTile, 1.0f, tex);
    REQUIRE(out.size() == 1);
    REQUIRE(out[0].ground);
    REQUIRE_FALSE(out[0].blocks());
}

TEST_CASE("vestir: peça sem arte no disco é omitida em vez de virar fantasma",
          "[city-props]") {
    // Sem isto a peça entraria no mundo invisível e (se sólida) barraria o
    // jogador contra o nada. Some inteira, e o carregador avisa no terminal.
    StubRenderer r(/*assets_present=*/false);
    const ScenePropTextures tex = load_scene_prop_textures(r);
    const ScenePropPlacement places[] = {{ScenePropKind::CasaCibergoticaA, 6, 4}};
    const auto out = build_scene_prop_instances(places, 1, kTile, 1.0f, tex);
    REQUIRE(out.empty());
}

TEST_CASE("vestir: o carregador pede uma textura por peça do catálogo",
          "[city-props]") {
    StubRenderer r(true);
    const ScenePropTextures tex = load_scene_prop_textures(r);
    REQUIRE(static_cast<int>(r.paths.size()) == kScenePropKindCount);
    for (int i = 0; i < kScenePropKindCount; ++i) {
        REQUIRE(tex.by_kind[i] != kInvalidTexture);
    }
}

TEST_CASE("vestir: o caminho de cada arte sai do header central de assets",
          "[city-props]") {
    // Caminho de asset nunca hardcoded: o carregador junta a pasta da área com o
    // arquivo da linha do catálogo, os dois vindos de core/asset_paths.hpp.
    StubRenderer r(true);
    load_scene_prop_textures(r);
    for (const std::string& p : r.paths) {
        REQUIRE(p.find("sprites/world/distritos_inferiores/") != std::string::npos);
        REQUIRE(p.find(".png") != std::string::npos);
    }
}

// ===========================================================================
//  Rota de ronda construída contra o mapa real
// ===========================================================================

TEST_CASE("ronda: rota horizontal nasce centrada na célula do inimigo",
          "[city-patrol]") {
    const TileGrid g = open_grid();
    const auto r = build_horizontal_patrol_route(g, /*cell_x=*/10, /*cell_y=*/8,
                                                 /*reach_tiles=*/3,
                                                 /*speed_tiles_per_sec=*/1.5f,
                                                 /*pause_seconds=*/1.0f);
    REQUIRE(r.valid());
    REQUIRE(r.count == 2);
    REQUIRE_THAT(r.waypoints[0].x, WithinAbs(7.0, kEps));
    REQUIRE_THAT(r.waypoints[1].x, WithinAbs(13.0, kEps));
    REQUIRE_THAT(r.waypoints[0].y, WithinAbs(8.0, kEps));
    REQUIRE_THAT(r.speed_tiles_per_sec, WithinAbs(1.5, kEps));
}

TEST_CASE("ronda: parede encurta a rota em vez de fazer o inimigo atravessar",
          "[city-patrol]") {
    // A ronda é um trilho: ninguém verifica colisão a cada passo dela. Então a
    // validação tem que acontecer AQUI, ao construir - senão o inimigo entra na
    // parede e o jogador vê um androide dentro de um prédio.
    TileGrid g = open_grid();
    g.set_blocked(12, 8, true);
    const auto r = build_horizontal_patrol_route(g, 10, 8, 5, 1.5f, 0.0f);
    REQUIRE(r.valid());
    REQUIRE_THAT(r.waypoints[1].x, WithinAbs(11.0, kEps));  // parou antes da parede
    REQUIRE_THAT(r.waypoints[0].x, WithinAbs(5.0, kEps));   // do outro lado, livre
}

TEST_CASE("ronda: parede dos dois lados devolve rota inválida - inimigo parado",
          "[city-patrol]") {
    TileGrid g = open_grid();
    g.set_blocked(9, 8, true);
    g.set_blocked(11, 8, true);
    const auto r = build_horizontal_patrol_route(g, 10, 8, 4, 1.5f, 0.0f);
    REQUIRE_FALSE(r.valid());
}

TEST_CASE("ronda: borda do mapa também encurta a rota", "[city-patrol]") {
    const TileGrid g = open_grid();  // 20x20: célula 19 é a última
    const auto r = build_horizontal_patrol_route(g, 18, 8, 5, 1.5f, 0.0f);
    REQUIRE(r.valid());
    REQUIRE_THAT(r.waypoints[1].x, WithinAbs(19.0, kEps));
}

TEST_CASE("ronda: alcance zero ou velocidade zero devolve rota inválida",
          "[city-patrol]") {
    const TileGrid g = open_grid();
    REQUIRE_FALSE(build_horizontal_patrol_route(g, 10, 8, 0, 1.5f, 0.0f).valid());
    REQUIRE_FALSE(build_horizontal_patrol_route(g, 10, 8, 3, 0.0f, 0.0f).valid());
}

TEST_CASE("ronda: célula de origem fora do mapa devolve rota inválida",
          "[city-patrol]") {
    const TileGrid g = open_grid();
    REQUIRE_FALSE(build_horizontal_patrol_route(g, -5, 8, 3, 1.5f, 0.0f).valid());
    REQUIRE_FALSE(build_horizontal_patrol_route(g, 10, 999, 3, 1.5f, 0.0f).valid());
}

// ===========================================================================
//  A COSTURA (fatia C): a tabela contra o .gmap REAL da cidade.
//
//  Os testes acima provam a REGRA com grades sintéticas; estes provam o DADO
//  contra o mapa que o jogo de fato carrega. É a única verificação que pega o
//  erro que interessa: alguém redesenha o mapa, a vestimenta continua apontando
//  para onde a rua estava, e o jogo não reclama - só fica com menos cidade.
// ===========================================================================

namespace {

// Carrega a cidade REAL (o mesmo caminho da janela e do smoke). Falha explícita
// se cair no fallback: um teste que degrada em silêncio aqui passaria a medir a
// cena de teste do M1 em vez do mapa de verdade.
gus::app::screens::CityLoadOutcome load_real_city() {
    gus::app::screens::CityLoadOutcome city =
        gus::app::screens::load_city_or_fallback();
    REQUIRE(city.status == gus::app::screens::CityLoadStatus::CityOk);
    return city;
}

}  // namespace

TEST_CASE("cidade real: a vestimenta inteira cabe no mapa carregado",
          "[city-props][mapa-real]") {
    const gus::app::screens::CityLoadOutcome city = load_real_city();
    const auto out = resolve_city_props(city.sim.grid(), kDistritosInferioresDressing,
                                        kDistritosInferioresDressingCount);
    // TODAS as peças, não "quase todas": peça descartada é peça que o líder não
    // vê no playtest e ninguém procura.
    REQUIRE(static_cast<int>(out.size()) == kDistritosInferioresDressingCount);
}

TEST_CASE("cidade real: o mapa carregado é o traçado de 90x60",
          "[city-props][mapa-real]") {
    // Guarda contra o defeito que abriu esta fatia: o CSV foi redesenhado e o
    // .gmap compilado continuou o antigo, de 30x20. Nada quebrava - o jogo só
    // carregava outra cidade.
    const gus::app::screens::CityLoadOutcome city = load_real_city();
    REQUIRE(city.sim.tile_map().has_value());
    REQUIRE(city.sim.tile_map()->width() == 90);
    REQUIRE(city.sim.tile_map()->height() == 60);
}

TEST_CASE("cidade real: toda peça de rua pisa numa âncora do level design",
          "[city-props][mapa-real]") {
    // A convenção do blockout (§2) é que a célula da peça é um tile Marco. Este
    // teste é o que impede a tabela de voltar a ser escrita "no olho": posição
    // inventada cai em Chão comum e reprova aqui, mesmo estando livre.
    const gus::app::screens::CityLoadOutcome city = load_real_city();
    const gus::domain::map::TileMap& map = *city.sim.tile_map();
    for (int i = 0; i < kDistritosInferioresDressingCount; ++i) {
        const CityPropRow& row = kDistritosInferioresDressing[i];
        REQUIRE(map.in_bounds(row.cell_x, row.cell_y));
        const std::uint16_t tile = map.at(row.cell_x, row.cell_y);
        if (row.cell_rule == PropCellRule::WalkableCell) {
            REQUIRE(tile == static_cast<std::uint16_t>(gus::domain::map::TileKind::Marco));
        } else {
            // Peça que É a parede (caixa de cobertura): célula sólida, por definição.
            REQUIRE(tile == static_cast<std::uint16_t>(gus::domain::map::TileKind::Parede));
        }
    }
}

TEST_CASE("cidade real: nenhuma peça nasce em cima do jogador",
          "[city-props][mapa-real]") {
    // Peça na célula de spawn enterraria o Gus dentro da casa no primeiro quadro.
    const gus::app::screens::CityLoadOutcome city = load_real_city();
    const gus::domain::map::Cell spawn = city.sim.tile_map()->spawn();
    for (int i = 0; i < kDistritosInferioresDressingCount; ++i) {
        const bool em_cima = kDistritosInferioresDressing[i].cell_x == spawn.x &&
                             kDistritosInferioresDressing[i].cell_y == spawn.y;
        REQUIRE_FALSE(em_cima);
    }
}

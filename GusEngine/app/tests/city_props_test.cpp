// SPDX-License-Identifier: Apache-2.0
// GusEngine/app/tests/city_props_test.cpp
//
// Catch2 do VESTIR DA CIDADE (DEMO-CIDADE-VESTIDA fatia B): a tradução entre o
// DADO do mundo (gus::domain::world) e as instâncias que o OverworldSim desenha e
// colide, mais a construção da rota de ronda contra o mapa real. TEST-FIRST,
// headless (o único toque em plataforma é um IRenderer falso).
//
// O que esta spec protege: a tabela de vestimenta é escrita à mão por quem faz
// level design, e mapa muda (o próprio mapa desta cidade está sendo redesenhado
// de 30x20 para 90x60 enquanto esta fatia é escrita). Célula inválida na tabela
// não pode virar casa dentro de parede, prop fora do mapa nem crash - tem que
// virar peça descartada, com aviso.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <vector>

#include "gus/app/screens/city_patrol.hpp"
#include "gus/app/screens/city_props.hpp"
#include "gus/core/spatial/tile_grid.hpp"
#include "gus/platform/render2d/i_renderer.hpp"

using Catch::Matchers::WithinAbs;
using gus::app::screens::build_horizontal_patrol_route;
using gus::app::screens::build_scene_prop_instances;
using gus::app::screens::kDistritosInferioresDressing;
using gus::app::screens::kDistritosInferioresDressingCount;
using gus::app::screens::load_scene_prop_textures;
using gus::app::screens::resolve_spawn_relative_props;
using gus::app::screens::ScenePropTextures;
using gus::app::screens::SpawnRelativePropRow;
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

// Jogador nascido no centro da célula (5,5).
Aabb spawn_at(int cx, int cy) {
    const float side = 0.6f * kTile;
    return Aabb{(static_cast<float>(cx) + 0.5f) * kTile - side * 0.5f,
                (static_cast<float>(cy) + 0.5f) * kTile - side * 0.5f, side, side};
}

}  // namespace

// ===========================================================================
//  Resolução da tabela de vestimenta
// ===========================================================================

TEST_CASE("vestir: linha relativa vira célula absoluta somada ao spawn",
          "[city-props]") {
    const TileGrid g = open_grid();
    const SpawnRelativePropRow rows[] = {
        {ScenePropKind::CasaCibergoticaA, 3, -2},
        {ScenePropKind::PosteNeonCiano, -1, 4},
    };
    const auto out = resolve_spawn_relative_props(g, spawn_at(5, 5), rows, 2);

    REQUIRE(out.size() == 2);
    REQUIRE(out[0].kind == ScenePropKind::CasaCibergoticaA);
    REQUIRE(out[0].cell_x == 8);
    REQUIRE(out[0].cell_y == 3);
    REQUIRE(out[1].cell_x == 4);
    REQUIRE(out[1].cell_y == 9);
}

TEST_CASE("vestir: peça que cai fora do mapa é descartada", "[city-props]") {
    const TileGrid g = open_grid();
    const SpawnRelativePropRow rows[] = {
        {ScenePropKind::CasaCibergoticaA, 100, 0},
        {ScenePropKind::PosteNeonCiano, 0, -99},
        {ScenePropKind::FonteLatao, 1, 1},  // esta é válida
    };
    const auto out = resolve_spawn_relative_props(g, spawn_at(5, 5), rows, 3);
    REQUIRE(out.size() == 1);
    REQUIRE(out[0].kind == ScenePropKind::FonteLatao);
}

TEST_CASE("vestir: peça que cai dentro de parede é descartada", "[city-props]") {
    // Casa dentro de parede é o erro clássico de tabela escrita à mão, e o mais
    // difícil de ver no playtest: a arte fica meio enterrada e ninguém percebe
    // que aquele tile era chão.
    TileGrid g = open_grid();
    g.set_blocked(8, 5, true);
    const SpawnRelativePropRow rows[] = {
        {ScenePropKind::CasaCibergoticaA, 3, 0},   // cai em (8,5) - parede
        {ScenePropKind::PosteNeonCiano, 2, 0},     // cai em (7,5) - livre
    };
    const auto out = resolve_spawn_relative_props(g, spawn_at(5, 5), rows, 2);
    REQUIRE(out.size() == 1);
    REQUIRE(out[0].kind == ScenePropKind::PosteNeonCiano);
}

TEST_CASE("vestir: tabela vazia devolve lista vazia sem estourar", "[city-props]") {
    const TileGrid g = open_grid();
    REQUIRE(resolve_spawn_relative_props(g, spawn_at(5, 5), nullptr, 0).empty());
    const SpawnRelativePropRow rows[] = {{ScenePropKind::CasaCibergoticaA, 1, 1}};
    REQUIRE(resolve_spawn_relative_props(g, spawn_at(5, 5), rows, 0).empty());
}

TEST_CASE("vestir: a tabela provisória da cidade é toda de peças conhecidas",
          "[city-props]") {
    REQUIRE(kDistritosInferioresDressingCount > 0);
    for (int i = 0; i < kDistritosInferioresDressingCount; ++i) {
        const int k = static_cast<int>(kDistritosInferioresDressing[i].kind);
        REQUIRE(k >= 0);
        REQUIRE(k < kScenePropKindCount);
    }
}

TEST_CASE("vestir: a tabela provisória não põe duas peças na mesma célula",
          "[city-props]") {
    // Duas peças no mesmo lugar é erro de autoria que só aparece no playtest
    // como arte sobreposta (ou pior, como duas caixas de colisão empilhadas).
    for (int i = 0; i < kDistritosInferioresDressingCount; ++i) {
        for (int j = i + 1; j < kDistritosInferioresDressingCount; ++j) {
            const bool mesmo =
                kDistritosInferioresDressing[i].offset_tiles_x ==
                    kDistritosInferioresDressing[j].offset_tiles_x &&
                kDistritosInferioresDressing[i].offset_tiles_y ==
                    kDistritosInferioresDressing[j].offset_tiles_y;
            REQUIRE_FALSE(mesmo);
        }
    }
}

TEST_CASE("vestir: nenhuma peça da tabela nasce em cima do jogador",
          "[city-props]") {
    // Offset (0,0) enterraria o Gus dentro da casa no primeiro quadro do jogo.
    for (int i = 0; i < kDistritosInferioresDressingCount; ++i) {
        const bool em_cima = kDistritosInferioresDressing[i].offset_tiles_x == 0 &&
                             kDistritosInferioresDressing[i].offset_tiles_y == 0;
        REQUIRE_FALSE(em_cima);
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

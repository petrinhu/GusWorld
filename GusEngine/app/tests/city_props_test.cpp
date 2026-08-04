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
#include "gus/app/screens/city_scene.hpp"  // spawn_player_aabb (invariante do 1o quadro)
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

TEST_CASE("vestir: a CÉLULA que a peça pisa chega na instância", "[city-props]") {
    // Sem isto o conserto do achado A3 funciona no teste e NÃO funciona no jogo:
    // o render decide não pintar a marcação de graybox por baixo de uma peça
    // olhando a célula que a instância carrega (OverworldSim::is_cell_dressed).
    // Se a montagem esquecer de copiar a célula, todo teste de supressão continua
    // verde (eles montam a instância à mão) e a cidade volta a mostrar a parede
    // atrás de cada caixa de cobertura, sem nada reclamar. Provado por mutante:
    // apagar as duas linhas da cópia deixava 20 casos e 1097 asserções passando.
    StubRenderer r(/*assets_present=*/true);
    const ScenePropTextures tex = load_scene_prop_textures(r);
    const ScenePropPlacement places[] = {{ScenePropKind::CoverBox, 7, 9},
                                         {ScenePropKind::PosteNeonCiano, 31, 2}};
    const auto out = build_scene_prop_instances(places, 2, kTile, /*scale=*/1.0f, tex);

    REQUIRE(out.size() == 2);
    REQUIRE(out[0].cell_x == 7);
    REQUIRE(out[0].cell_y == 9);
    REQUIRE(out[1].cell_x == 31);
    REQUIRE(out[1].cell_y == 2);
}

TEST_CASE("vestir: peça descartada não desloca a célula das outras",
          "[city-props]") {
    // A lista de saída é MENOR que a de entrada quando falta arte, então copiar a
    // célula por índice paralelo (out[i] <- places[i]) daria a célula errada para
    // todas as peças depois do buraco - e o defeito seria "a parede some no lugar
    // errado", que ninguém liga à peça que faltou.
    StubRenderer r(/*assets_present=*/false);  // NENHUMA arte carrega
    ScenePropTextures tex = load_scene_prop_textures(r);
    StubRenderer r2(/*assets_present=*/true);
    const ScenePropTextures tex_ok = load_scene_prop_textures(r2);
    // Só a segunda peça tem arte.
    tex.by_kind[static_cast<int>(ScenePropKind::PosteNeonCiano)] =
        tex_ok.at(ScenePropKind::PosteNeonCiano);

    const ScenePropPlacement places[] = {{ScenePropKind::CoverBox, 7, 9},
                                         {ScenePropKind::PosteNeonCiano, 31, 2}};
    const auto out = build_scene_prop_instances(places, 2, kTile, /*scale=*/1.0f, tex);

    REQUIRE(out.size() == 1);
    REQUIRE(out[0].cell_x == 31);
    REQUIRE(out[0].cell_y == 2);
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

TEST_CASE("cidade real: no primeiro quadro o jogador cabe INTEIRO na tela",
          "[city-props][mapa-real]") {
    // ACHADO A5 do laudo visual da fatia D: o Gus nascia em (44,1), a câmera não
    // rola acima de y=0, e 27% do sprite (a cabeça) ficava fora do quadro - no
    // PRIMEIRO frame do jogo. Medido pelo laudo: bbox de 58x77 px no spawn contra
    // 61x105 px em qualquer outro lugar.
    //
    // A invariante é geométrica e não depende de câmera nenhuma: o desenho do
    // jogador é um quadrado de player_sprite_height_tiles ancorado na BASE da
    // hitbox e crescendo para cima, e a borda de cima do mundo é y=0. Se o topo
    // desse quadrado é negativo, ele está fora do mapa e nenhuma câmera consegue
    // mostrá-lo.
    //
    // Medido com o QUADRO inteiro (não com o conteúdo do PNG): a margem
    // transparente do sprite é dado de arte que pode mudar sem aviso, e um teste
    // que dependesse dela passaria a aprovar corte de cabelo por meio pixel.
    const gus::app::screens::CityLoadOutcome city = load_real_city();
    const gus::core::spatial::Aabb hitbox =
        gus::app::screens::spawn_player_aabb(*city.sim.tile_map());
    const float tile = city.sim.grid().tile_size();
    const float altura_do_desenho =
        city.sim.tuning().player_sprite_height_tiles * tile;
    const float base_dos_pes = hitbox.y + hitbox.h;
    const float topo_do_desenho = base_dos_pes - altura_do_desenho;

    INFO("base dos pes = " << base_dos_pes << " unidades, desenho = "
                           << altura_do_desenho << " unidades, topo = "
                           << topo_do_desenho);
    REQUIRE(topo_do_desenho >= 0.0f);
}

// ===========================================================================
//  ESCALA A2 (fatia G): as PEÇAS crescem, o personagem fica intacto.
//
//  O líder viu a montagem comparativa da fatia F e escolheu A2: subir
//  `scene_prop_scale` sem mexer no sprite do Gus nem no zoom da câmera. O que
//  esta seção trava NÃO é o número escolhido - é a RAZÃO que ele produz na tela,
//  que é o que o líder de fato olhou. Testar o número seria copiar a constante
//  para um segundo lugar e chamar isso de verificação.
//
//  As faixas abaixo são de INTENÇÃO DE LEITURA, não de tolerância numérica:
//  "criança de 11 anos diante de uma cidade que a domina". Elas reprovam tanto o
//  estado antigo (peça mais BAIXA que a criança) quanto um exagero que
//  transformasse a rua em desfiladeiro.
//
//  Razão de QUADRO (altura de canvas em tiles), que é o que o motor desenha. A
//  razão VISÍVEL medida em pixel é maior, porque o conteúdo do PNG do Gus ocupa
//  88,3% do canvas dele e o das peças ocupa mais (poste 90,0%, casa A 90,6%,
//  casa B 93,0% - alpha-bbox medido). Na escala vigente isso dá, em pixel,
//  poste 1,70x / casa A 2,05x / casa B 2,80x a criança - os números que o QA
//  mediu na montagem e que o líder aprovou.
// ===========================================================================

namespace {

// Altura DESENHADA de uma peça, em tiles, já com a escala global do líder.
float altura_desenhada_tiles(gus::domain::world::ScenePropKind kind, float escala) {
    return gus::domain::world::scene_prop_def(kind).height_tiles() * escala;
}

}  // namespace

TEST_CASE("escala A2: a cidade domina a criança na altura de tela",
          "[city-props][escala]") {
    const gus::app::screens::OverworldTuning tuning = gus::app::screens::make_city_tuning();
    const float escala = tuning.scene_prop_scale;
    const float crianca = tuning.player_sprite_height_tiles;
    REQUIRE(crianca > 0.0f);

    const float poste =
        altura_desenhada_tiles(ScenePropKind::PosteNeonCiano, escala) / crianca;
    const float casa_a =
        altura_desenhada_tiles(ScenePropKind::CasaCibergoticaA, escala) / crianca;
    const float casa_b =
        altura_desenhada_tiles(ScenePropKind::CasaCibergoticaB, escala) / crianca;

    INFO("razoes de QUADRO peca/Gus: poste " << poste << ", casa A " << casa_a
                                             << ", casa B " << casa_b
                                             << " (scene_prop_scale " << escala << ")");

    // Poste de rua: acima da cabeça da criança com folga, e ainda um poste - não
    // uma torre. Reprova o estado anterior à fatia G, em que o poste era 0,91x o
    // Gus (uma luminária mais baixa que uma criança de 11 anos).
    REQUIRE(poste > 1.55f);
    REQUIRE(poste < 1.80f);

    // Casa térrea: o dobro da criança. É a leitura de "porta que ela mal alcança".
    REQUIRE(casa_a > 1.90f);
    REQUIRE(casa_a < 2.15f);

    // O prédio alto do bairro leste: quase o triplo. É o que dá verticalidade ao
    // skyline ciber-gótico sem sair da escala de rua.
    REQUIRE(casa_b > 2.50f);
    REQUIRE(casa_b < 2.85f);

    // Ordem de massa: poste < casa térrea < prédio. Invariante de leitura de
    // cidade, independente de qual escala esteja em vigor.
    REQUIRE(poste < casa_a);
    REQUIRE(casa_a < casa_b);
}

TEST_CASE("escala A2: escalar peça NÃO mexe no corpo do jogador",
          "[city-props][escala][mapa-real]") {
    // O par da decisão A2: a via descartada era encolher o Gus. Este teste é o que
    // reprova alguém "compensando" a escala das peças no personagem depois. A
    // hitbox do jogador vem de kPlayerHitboxTileFraction e do tile do mapa - a
    // escala de cenário não entra na conta, e não pode passar a entrar.
    const gus::app::screens::CityLoadOutcome city = load_real_city();
    const Aabb hitbox = gus::app::screens::spawn_player_aabb(*city.sim.tile_map());
    const float tile = city.sim.grid().tile_size();
    const float lado_esperado = gus::app::screens::kPlayerHitboxTileFraction * tile;

    INFO("hitbox " << hitbox.w << "x" << hitbox.h << ", esperado lado "
                   << lado_esperado << ", scene_prop_scale "
                   << city.sim.tuning().scene_prop_scale);
    REQUIRE_THAT(static_cast<double>(hitbox.w),
                 WithinAbs(static_cast<double>(lado_esperado), kEps));
    REQUIRE_THAT(static_cast<double>(hitbox.h),
                 WithinAbs(static_cast<double>(lado_esperado), kEps));
}

// ===========================================================================
//  CONECTIVIDADE DA CIDADE VESTIDA (fatia G)
//
//  A MESMA escala que aumenta o desenho multiplica a caixa que BLOQUEIA
//  (domain/src/world/scene_prop.cpp::scene_prop_solid). Subir a escala cobra rua:
//  medido na fatia F, 62 células andáveis eram tocadas por peça em 1,00x e 155 na
//  escala vigente. O líder aceitou esse custo sabendo o número - o que NINGUÉM
//  tinha verificado é se alguma dessas células é um gargalo que parte a cidade em
//  duas.
//
//  Este teste é a verificação. Ele NÃO conta células seladas (número que muda a
//  cada peça nova e viraria teste de baixa fidelidade): ele responde a única
//  pergunta que interessa ao jogador - dá para ir do spawn a todo canto do mapa
//  com a cidade vestida?
//
//  A régua é a do MOTOR, não a da célula: o jogo colide em AABB contínua, então a
//  busca varre uma sub-grade fina e aceita uma posição quando a hitbox REAL do
//  jogador cabe ali sem encostar em parede nem em caixa de peça. Uma busca por
//  célula aprovaria fresta de meio tile por onde o Gus não passa.
// ===========================================================================

namespace {

// Sub-amostras por tile na busca. 4 = passo de 0,25 tile contra uma hitbox de
// 0,6 tile: fino o bastante para não inventar passagem, grosso o bastante para a
// varredura inteira (90x60x16 posições) rodar em fração de segundo.
constexpr int kSubAmostras = 4;

// Mapa de alcance do jogador sobre a cidade vestida.
struct AlcanceDaCidade {
    int largura_celulas = 0;
    int altura_celulas = 0;
    std::vector<char> livre;      // sub-grade: a hitbox cabe aqui
    std::vector<char> visitado;   // sub-grade: alcançável a pé desde o spawn

    [[nodiscard]] bool celula_tem_espaco(int cx, int cy) const {
        for (int sy = cy * kSubAmostras; sy < (cy + 1) * kSubAmostras; ++sy) {
            for (int sx = cx * kSubAmostras; sx < (cx + 1) * kSubAmostras; ++sx) {
                if (livre[static_cast<std::size_t>(sy) * largura_celulas * kSubAmostras
                          + sx] != 0) {
                    return true;
                }
            }
        }
        return false;
    }

    [[nodiscard]] bool celula_alcancada(int cx, int cy) const {
        for (int sy = cy * kSubAmostras; sy < (cy + 1) * kSubAmostras; ++sy) {
            for (int sx = cx * kSubAmostras; sx < (cx + 1) * kSubAmostras; ++sx) {
                if (visitado[static_cast<std::size_t>(sy) * largura_celulas
                                 * kSubAmostras + sx] != 0) {
                    return true;
                }
            }
        }
        return false;
    }
};

AlcanceDaCidade varrer_cidade_vestida(const gus::app::screens::CityLoadOutcome& city) {
    const TileGrid& grid = city.sim.grid();
    const float ts = grid.tile_size();
    const float escala = city.sim.tuning().scene_prop_scale;

    // As caixas que BLOQUEIAM, exatamente como o jogo as monta.
    std::vector<Aabb> solidos;
    for (const ScenePropPlacement& p :
         resolve_city_props(grid, kDistritosInferioresDressing,
                            kDistritosInferioresDressingCount)) {
        const gus::domain::world::ScenePropDef& def =
            gus::domain::world::scene_prop_def(p.kind);
        if (!def.solid()) continue;
        const Aabb fp = gus::domain::world::scene_prop_footprint(p, def, ts, escala);
        solidos.push_back(gus::domain::world::scene_prop_solid(fp, def, ts, escala));
    }

    AlcanceDaCidade out;
    out.largura_celulas = grid.width();
    out.altura_celulas = grid.height();
    const int W = out.largura_celulas * kSubAmostras;
    const int H = out.altura_celulas * kSubAmostras;
    out.livre.assign(static_cast<std::size_t>(W) * H, 0);
    out.visitado.assign(static_cast<std::size_t>(W) * H, 0);

    const float meio = gus::app::screens::kPlayerHitboxTileFraction * ts * 0.5f;
    const float passo = ts / static_cast<float>(kSubAmostras);
    const float largura_mundo = static_cast<float>(out.largura_celulas) * ts;
    const float altura_mundo = static_cast<float>(out.altura_celulas) * ts;

    for (int sy = 0; sy < H; ++sy) {
        for (int sx = 0; sx < W; ++sx) {
            const float cx = (static_cast<float>(sx) + 0.5f) * passo;
            const float cy = (static_cast<float>(sy) + 0.5f) * passo;
            const float x0 = cx - meio, y0 = cy - meio;
            const float x1 = cx + meio, y1 = cy + meio;
            if (x0 < 0.0f || y0 < 0.0f || x1 > largura_mundo || y1 > altura_mundo) {
                continue;
            }
            bool livre = true;
            for (int gy = grid.world_to_cell(y0); gy <= grid.world_to_cell(y1 - 1e-4f)
                 && livre; ++gy) {
                for (int gx = grid.world_to_cell(x0);
                     gx <= grid.world_to_cell(x1 - 1e-4f); ++gx) {
                    if (grid.is_blocked(gx, gy)) {
                        livre = false;
                        break;
                    }
                }
            }
            if (livre) {
                for (const Aabb& s : solidos) {
                    if (x0 < s.x + s.w && x1 > s.x && y0 < s.y + s.h && y1 > s.y) {
                        livre = false;
                        break;
                    }
                }
            }
            out.livre[static_cast<std::size_t>(sy) * W + sx] = livre ? 1 : 0;
        }
    }

    // Busca em largura a partir do spawn REAL do mapa.
    const gus::domain::map::Cell spawn = city.sim.tile_map()->spawn();
    int inicio_x = spawn.x * kSubAmostras + kSubAmostras / 2;
    int inicio_y = spawn.y * kSubAmostras + kSubAmostras / 2;
    REQUIRE(out.livre[static_cast<std::size_t>(inicio_y) * W + inicio_x] != 0);

    std::vector<int> fila;
    fila.reserve(static_cast<std::size_t>(W) * H);
    fila.push_back(inicio_y * W + inicio_x);
    out.visitado[static_cast<std::size_t>(inicio_y) * W + inicio_x] = 1;
    for (std::size_t i = 0; i < fila.size(); ++i) {
        const int no = fila[i];
        const int x = no % W, y = no / W;
        const int viz[4][2] = {{x + 1, y}, {x - 1, y}, {x, y + 1}, {x, y - 1}};
        for (const auto& v : viz) {
            if (v[0] < 0 || v[0] >= W || v[1] < 0 || v[1] >= H) continue;
            const std::size_t k = static_cast<std::size_t>(v[1]) * W + v[0];
            if (out.livre[k] != 0 && out.visitado[k] == 0) {
                out.visitado[k] = 1;
                fila.push_back(static_cast<int>(k));
            }
        }
    }
    return out;
}

}  // namespace

TEST_CASE("cidade vestida: nenhuma célula andável fica ilhada",
          "[city-props][mapa-real][conectividade]") {
    const gus::app::screens::CityLoadOutcome city = load_real_city();
    const AlcanceDaCidade alcance = varrer_cidade_vestida(city);

    std::vector<std::pair<int, int>> ilhadas;
    for (int cy = 0; cy < alcance.altura_celulas; ++cy) {
        for (int cx = 0; cx < alcance.largura_celulas; ++cx) {
            if (alcance.celula_tem_espaco(cx, cy) && !alcance.celula_alcancada(cx, cy)) {
                ilhadas.emplace_back(cx, cy);
            }
        }
    }
    std::string amostra;
    for (std::size_t i = 0; i < ilhadas.size() && i < 12; ++i) {
        amostra += "(" + std::to_string(ilhadas[i].first) + "," +
                   std::to_string(ilhadas[i].second) + ") ";
    }
    INFO("celulas com espaco util mas inalcancaveis a pe: " << ilhadas.size() << " "
                                                            << amostra);
    REQUIRE(ilhadas.empty());
}

TEST_CASE("cidade vestida: a saída sul continua alcançável",
          "[city-props][mapa-real][conectividade]") {
    // O jogador tem que conseguir SAIR da área. Peça que sele o vestíbulo tranca o
    // jogo com o mapa inteiro parecendo normal.
    const gus::app::screens::CityLoadOutcome city = load_real_city();
    const AlcanceDaCidade alcance = varrer_cidade_vestida(city);

    bool achou_saida = false;
    for (const gus::domain::map::Portal& portal : city.sim.tile_map()->portals()) {
        INFO("portal " << portal.id << " em (" << portal.cell.x << ","
                       << portal.cell.y << ")");
        REQUIRE(alcance.celula_alcancada(portal.cell.x, portal.cell.y));
        if (portal.id == "saida_sul") achou_saida = true;
    }
    REQUIRE(achou_saida);
}

TEST_CASE("cidade vestida: os chokes do blockout continuam passáveis",
          "[city-props][mapa-real][conectividade]") {
    // Os três estrangulamentos declarados no doc de blockout. São os pontos onde a
    // rua já é estreita de propósito, e por isso os primeiros a fechar quando a
    // caixa das peças cresce: o degrau da escadaria (R13, 4 células em y32) e as
    // duas únicas aberturas laterais da praça (R9).
    const gus::app::screens::CityLoadOutcome city = load_real_city();
    const AlcanceDaCidade alcance = varrer_cidade_vestida(city);
    const TileGrid& grid = city.sim.grid();

    struct Choke {
        const char* nome;
        int x0, y0, x1, y1;  // faixa inclusiva de células
    };
    const Choke chokes[] = {
        {"R13 degrau da escadaria (y32)", 43, 32, 46, 32},
        {"R9 abertura oeste do hub (x25)", 25, 20, 25, 22},
        {"R9 abertura leste do hub (x64)", 64, 16, 64, 18},
    };

    for (const Choke& c : chokes) {
        int passaveis = 0;
        int livres_no_tracado = 0;
        for (int cy = c.y0; cy <= c.y1; ++cy) {
            for (int cx = c.x0; cx <= c.x1; ++cx) {
                if (grid.is_blocked(cx, cy)) continue;
                ++livres_no_tracado;
                if (alcance.celula_alcancada(cx, cy)) ++passaveis;
            }
        }
        INFO(c.nome << ": " << livres_no_tracado << " celulas livres no tracado, "
                    << passaveis << " alcancaveis com a cidade vestida");
        // Toda célula que o traçado deixou livre tem que continuar servindo: um
        // choke que perde metade da largura ainda "passa", mas vira funil de
        // colisão que o jogador sente como travamento.
        REQUIRE(livres_no_tracado > 0);
        REQUIRE(passaveis == livres_no_tracado);
    }
}

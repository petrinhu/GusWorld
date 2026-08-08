// SPDX-License-Identifier: Apache-2.0
// GusEngine/app/tests/overworld_world_entities_test.cpp
//
// Catch2 das PEÇAS DE CENÁRIO e das LISTAS DE ATORES do overworld
// (DEMO-CIDADE-VESTIDA fatia B1/B2/B3). TEST-FIRST, headless: IRenderer falso que
// só registra o que seria desenhado.
//
// O que esta spec trava, e que não existia antes desta fatia:
//   B1. peça de cenário no mundo, vinda de DADO (o mundo desenhava só retângulos
//       de cor por tipo de tile, mais dois slots fixos de personagem);
//   B2. VÁRIOS NPCs e VÁRIOS inimigos - os dois slots únicos viraram lista, e o
//       Y-sort continua correto com N entidades, não só com 3;
//   B3. ronda em rota fixa (ida e volta, sem reagir ao jogador).
//
// A fachada legada (set_enemy_marker/set_npc_bertoldo_marker) continua no ar e
// tem spec própria em overworld_sim_test.cpp - aqui só se prova que ela passou a
// ser UM item da lista, não um slot paralelo.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <vector>

#include "gus/app/screens/city_patrol.hpp"  // build_horizontal_patrol_route
#include "gus/app/screens/overworld_sim.hpp"
#include "gus/app/screens/overworld_tuning.hpp"
#include "gus/app/screens/world_entities.hpp"
#include "gus/core/spatial/tile_grid.hpp"
#include "gus/domain/world/patrol_route.hpp"
#include "gus/domain/world/scene_prop.hpp"
#include "gus/platform/render2d/i_renderer.hpp"

using Catch::Matchers::WithinAbs;
using gus::app::screens::OverworldSim;
using gus::app::screens::OverworldTuning;
using gus::app::screens::PlayerSpriteSet;
using gus::app::screens::ScenePropInstance;
using gus::app::screens::WorldActorRole;
using gus::app::screens::WorldActorSpec;
using gus::core::spatial::Aabb;
using gus::core::spatial::Rect;
using gus::core::spatial::TileGrid;
using gus::domain::world::PatrolRoute;
using gus::platform::render2d::DrawColor;
using gus::platform::render2d::IRenderer;
using gus::platform::render2d::kInvalidTexture;
using gus::platform::render2d::TextureId;
using gus::platform::render2d::UvRect;

namespace {

constexpr double kEps = 1e-3;
constexpr float kTile = 16.0f;

// IRenderer falso: registra na ORDEM em que foi chamado (é a ordem de desenho, e
// portanto a prova do Y-sort), sem tocar GPU.
class RecordingRenderer : public IRenderer {
public:
    struct SpriteCmd {
        Rect rect;
        TextureId texture;
    };

    void begin_frame(const Rect& camera_world, int, int) override {
        last_camera = camera_world;
        sprites.clear();
        fills = 0;
    }
    void draw_filled_rect(const Rect&, const DrawColor&) override { ++fills; }
    void draw_rect_outline(const Rect& r, const DrawColor&, float) override {
        // Fallback de jogador sem sprite: registrado como "sprite" de textura 0
        // para o teste poder afirmar a POSIÇÃO dele na sequência de desenho.
        sprites.push_back({r, 0});
    }
    TextureId load_texture(const char*) override { return ++next_texture; }
    void draw_textured_rect(const Rect& r, TextureId t, const UvRect&,
                            const DrawColor&) override {
        sprites.push_back({r, t});
    }
    gus::platform::render2d::ContentBbox texture_content_bbox(
        TextureId) const override {
        return gus::platform::render2d::ContentBbox{};
    }
    void draw_text(const char*, float, float, float, const DrawColor&, bool) override {}
    void end_frame() override {}

    [[nodiscard]] int index_of(TextureId t) const {
        for (std::size_t i = 0; i < sprites.size(); ++i) {
            if (sprites[i].texture == t) return static_cast<int>(i);
        }
        return -1;
    }

    int fills = 0;
    TextureId next_texture = kInvalidTexture;
    Rect last_camera{};
    std::vector<SpriteCmd> sprites;
};

// Grade aberta: nenhuma parede além da borda, para que a ÚNICA coisa capaz de
// barrar o jogador seja o que o teste colocar no mundo.
TileGrid open_grid(int w = 20, int h = 20) { return TileGrid(w, h, kTile); }

ScenePropInstance make_prop(float x, float y, float w, float h, TextureId tex,
                            bool solid, bool ground = false) {
    ScenePropInstance p;
    p.footprint = Aabb{x, y, w, h};
    p.tex = tex;
    p.ground = ground;
    if (solid) {
        // Base do desenho, largura cheia (a mesma convenção do domínio).
        p.solid = Aabb{x, y + h - kTile, w, kTile};
    }
    return p;
}

WorldActorSpec make_actor(WorldActorRole role, float x, float y, TextureId tex) {
    WorldActorSpec s;
    s.role = role;
    s.anchor = Aabb{x, y, 8.0f, 8.0f};
    s.tex = tex;
    s.sprite_height_tiles = 1.0f;  // quad de 1 tile: matemática fácil no teste
    return s;
}

// Ronda horizontal de `len` tiles a partir da célula (cx,cy).
PatrolRoute line_route(float cx, float cy, float len, float speed) {
    PatrolRoute r;
    r.waypoints[0] = {cx, cy};
    r.waypoints[1] = {cx + len, cy};
    r.count = 2;
    r.speed_tiles_per_sec = speed;
    return r;
}

}  // namespace

// ===========================================================================
//  B1 - peças de cenário vindas de dado
// ===========================================================================

TEST_CASE("mundo: cidade nasce sem nenhuma peça de cenário", "[overworld][props]") {
    OverworldSim sim(open_grid(), Aabb{40.0f, 40.0f, 8.0f, 8.0f}, OverworldTuning{});
    REQUIRE(sim.scene_props().empty());
}

TEST_CASE("mundo: peça de cenário adicionada é desenhada onde o dado mandou",
          "[overworld][props]") {
    OverworldSim sim(open_grid(), Aabb{40.0f, 40.0f, 8.0f, 8.0f}, OverworldTuning{});
    sim.add_scene_prop(make_prop(100.0f, 60.0f, 48.0f, 48.0f, /*tex=*/70, false));

    RecordingRenderer r;
    sim.render(r, 2000.0f, 2000.0f, 0.0f);

    const int i = r.index_of(70);
    REQUIRE(i >= 0);
    REQUIRE_THAT(r.sprites[static_cast<std::size_t>(i)].rect.x, WithinAbs(100.0, kEps));
    REQUIRE_THAT(r.sprites[static_cast<std::size_t>(i)].rect.y, WithinAbs(60.0, kEps));
    REQUIRE_THAT(r.sprites[static_cast<std::size_t>(i)].rect.w, WithinAbs(48.0, kEps));
    REQUIRE_THAT(r.sprites[static_cast<std::size_t>(i)].rect.h, WithinAbs(48.0, kEps));
}

TEST_CASE("mundo: N peças de cenário convivem e todas são desenhadas",
          "[overworld][props]") {
    OverworldSim sim(open_grid(), Aabb{40.0f, 40.0f, 8.0f, 8.0f}, OverworldTuning{});
    for (int i = 0; i < 12; ++i) {
        sim.add_scene_prop(make_prop(32.0f + static_cast<float>(i) * 16.0f, 60.0f,
                                     16.0f, 32.0f, /*tex=*/200 + i, false));
    }
    REQUIRE(sim.scene_props().size() == 12);

    RecordingRenderer r;
    sim.render(r, 4000.0f, 4000.0f, 0.0f);
    for (int i = 0; i < 12; ++i) {
        REQUIRE(r.index_of(200 + i) >= 0);
    }
}

TEST_CASE("mundo: peça sem textura não é desenhada", "[overworld][props]") {
    OverworldSim sim(open_grid(), Aabb{40.0f, 40.0f, 8.0f, 8.0f}, OverworldTuning{});
    sim.add_scene_prop(make_prop(100.0f, 60.0f, 48.0f, 48.0f, kInvalidTexture, false));

    RecordingRenderer r;
    sim.render(r, 2000.0f, 2000.0f, 0.0f);
    // Só o fallback de contorno do jogador (sem sprites carregados).
    REQUIRE(r.sprites.size() == 1);
    REQUIRE(r.sprites[0].texture == 0);
}

TEST_CASE("mundo: peça fora do enquadramento da câmera é descartada no desenho",
          "[overworld][props]") {
    OverworldSim sim(open_grid(40, 40), Aabb{40.0f, 40.0f, 8.0f, 8.0f},
                     OverworldTuning{});
    sim.add_scene_prop(make_prop(560.0f, 560.0f, 32.0f, 32.0f, /*tex=*/71, false));

    RecordingRenderer r;
    sim.render(r, 200.0f, 200.0f, 0.0f);  // enquadramento pequeno perto do jogador
    REQUIRE(r.index_of(71) < 0);
}

TEST_CASE("mundo: peça sólida barra o jogador como parede", "[overworld][props]") {
    OverworldTuning t;
    t.corner.enabled = false;  // resultado 100% analítico
    OverworldSim sim(open_grid(), Aabb{20.0f, 100.0f, 8.0f, 8.0f}, t);
    // Casa com base em y:[96,112) e x:[96,144) - o jogador anda na faixa y=100.
    sim.add_scene_prop(make_prop(96.0f, 64.0f, 48.0f, 48.0f, /*tex=*/70, true));

    for (int i = 0; i < 200; ++i) {
        sim.step_fixed(1, 0, false, 1.0f / 60.0f);
    }
    REQUIRE(sim.player().x + sim.player().w <= 96.0f + kEps);
    REQUIRE_THAT(sim.player().x, WithinAbs(96.0 - sim.player().w, 0.05));
}

TEST_CASE("mundo: peça atravessável não barra o jogador", "[overworld][props]") {
    OverworldTuning t;
    t.corner.enabled = false;
    OverworldSim sim(open_grid(), Aabb{20.0f, 100.0f, 8.0f, 8.0f}, t);
    sim.add_scene_prop(make_prop(96.0f, 64.0f, 48.0f, 48.0f, /*tex=*/70, false));

    const float x0 = sim.player().x;
    for (int i = 0; i < 200; ++i) {
        sim.step_fixed(1, 0, false, 1.0f / 60.0f);
    }
    REQUIRE(sim.player().x > x0 + 100.0f);
}

TEST_CASE("mundo: peça sem textura também não barra - nada de parede invisível",
          "[overworld][props]") {
    // Degradação por arte ausente NUNCA pode virar colisão fantasma: o jogador
    // travaria contra algo que ele não vê, e o bug seria invisível no playtest.
    OverworldTuning t;
    t.corner.enabled = false;
    OverworldSim sim(open_grid(), Aabb{20.0f, 100.0f, 8.0f, 8.0f}, t);
    sim.add_scene_prop(make_prop(96.0f, 64.0f, 48.0f, 48.0f, kInvalidTexture, true));

    const float x0 = sim.player().x;
    for (int i = 0; i < 200; ++i) {
        sim.step_fixed(1, 0, false, 1.0f / 60.0f);
    }
    REQUIRE(sim.player().x > x0 + 100.0f);
}

TEST_CASE("mundo: clear_scene_props tira as peças e libera a passagem",
          "[overworld][props]") {
    OverworldTuning t;
    t.corner.enabled = false;
    OverworldSim sim(open_grid(), Aabb{20.0f, 100.0f, 8.0f, 8.0f}, t);
    sim.add_scene_prop(make_prop(96.0f, 64.0f, 48.0f, 48.0f, /*tex=*/70, true));
    sim.clear_scene_props();
    REQUIRE(sim.scene_props().empty());

    const float x0 = sim.player().x;
    for (int i = 0; i < 200; ++i) {
        sim.step_fixed(1, 0, false, 1.0f / 60.0f);
    }
    REQUIRE(sim.player().x > x0 + 100.0f);
}

TEST_CASE("mundo: peça de PÉ entra no Y-sort - jogador acima dela fica atrás",
          "[overworld][props][depth-sort]") {
    OverworldSim sim(open_grid(), Aabb{40.0f, 20.0f, 8.0f, 8.0f}, OverworldTuning{});
    // Base da casa em y=112; base do jogador em y=28 (mais acima na tela).
    sim.add_scene_prop(make_prop(96.0f, 64.0f, 48.0f, 48.0f, /*tex=*/70, false));

    RecordingRenderer r;
    sim.render(r, 4000.0f, 4000.0f, 0.0f);
    const int player_i = r.index_of(0);   // fallback de contorno
    const int prop_i = r.index_of(70);
    REQUIRE(player_i >= 0);
    REQUIRE(prop_i >= 0);
    REQUIRE(player_i < prop_i);  // jogador desenha primeiro = fica ATRÁS da casa
}

TEST_CASE("mundo: peça de PÉ entra no Y-sort - jogador abaixo dela fica na frente",
          "[overworld][props][depth-sort]") {
    OverworldSim sim(open_grid(), Aabb{40.0f, 200.0f, 8.0f, 8.0f}, OverworldTuning{});
    sim.add_scene_prop(make_prop(96.0f, 64.0f, 48.0f, 48.0f, /*tex=*/70, false));

    RecordingRenderer r;
    sim.render(r, 4000.0f, 4000.0f, 0.0f);
    REQUIRE(r.index_of(70) < r.index_of(0));  // casa primeiro, jogador por cima
}

TEST_CASE("mundo: peça de CHÃO é pintada antes de todo mundo e nunca esconde ninguém",
          "[overworld][props][depth-sort]") {
    // O tabuleiro de puzzle é chão: o jogador PISA nele. Se entrasse no Y-sort,
    // apagaria o jogador toda vez que ele andasse pela metade de cima.
    OverworldSim sim(open_grid(), Aabb{40.0f, 20.0f, 8.0f, 8.0f}, OverworldTuning{});
    sim.add_scene_prop(make_prop(96.0f, 160.0f, 48.0f, 48.0f, /*tex=*/80, false,
                                 /*ground=*/true));

    RecordingRenderer r;
    sim.render(r, 4000.0f, 4000.0f, 0.0f);
    // Base do chão (208) é MUITO maior que a do jogador (28): num Y-sort ingênuo
    // ele viria por último. Como é chão, vem primeiro de todos.
    REQUIRE(r.index_of(80) == 0);
    REQUIRE(r.index_of(0) > 0);
}

// ===========================================================================
//  REGRESSÃO DO CASO RELATADO PELO LÍDER (DEMO-CIDADE-VESTIDA fatia I).
//
//  Ele olhou uma captura da cidade e viu o PRÉDIO DESENHADO POR CIMA DO GUS, com
//  a parede cortando o rosto e o ombro, apesar de o menino estar nivelado com a
//  base da casa. A aritmética do defeito, medida na cena real:
//
//    - a casa cibergótica A tem 3 células na arte e a escala global de peças está
//      em 1,83, então o DESENHO ocupa 5,49 células: ancorado na célula 8, cobre da
//      5,75 até a 11,25 - e o jogador estava na célula 10, DENTRO dessa faixa,
//      a duas células de distância da casa;
//    - a base do desenho da casa cai na base da célula (y = 44,0), enquanto a
//      âncora do jogador é uma caixa CENTRADA na célula, cuja base fica 0,4
//      unidade acima (y = 43,6).
//
//  Resultado com a chave escalar de antes: 44,0 > 43,6, a casa desenhava por
//  último e engolia o menino. E a diferença de 0,4 é ESTRUTURAL, não acidental -
//  vale para TODA peça contra TODO personagem na mesma linha, em qualquer ponto do
//  mapa. Nenhuma escolha de número conserta isso: só um modelo que compare a fatia
//  de chão de cada um, que é o que a ordenação por oclusão faz.
//
//  As coordenadas abaixo são as da cena real (grade 90x60 com tile 2,0 do mapa dos
//  Distritos Inferiores), e a geometria da peça vem do CATÁLOGO de verdade, não
//  copiada à mão: se a arte, a escala ou a fórmula de ancoragem mudarem, este
//  teste acompanha em vez de mentir.
// ===========================================================================
TEST_CASE("mundo: REGRESSÃO do líder - jogador nivelado com a base da casa fica NA FRENTE",
          "[overworld][props][depth-sort][regressao-lider]") {
    constexpr float kTileCidade = 2.0f;
    constexpr int kCasaCelulaX = 8;
    constexpr int kCasaCelulaY = 21;
    constexpr int kJogadorCelulaX = 10;

    OverworldTuning t;  // scene_prop_scale = 1,83, o botão do líder
    TileGrid grid(90, 60, kTileCidade);

    // Âncora do jogador na célula pedida: caixa de kPlayerHitboxTileFraction do
    // tile, CENTRADA na célula (a mesma regra de pick_actor_position_at_cell).
    const auto ancora_na_celula = [&](int cx, int cy) {
        const float lado = kTileCidade * 0.6f;
        return Aabb{(static_cast<float>(cx) + 0.5f) * kTileCidade - lado * 0.5f,
                    (static_cast<float>(cy) + 0.5f) * kTileCidade - lado * 0.5f, lado,
                    lado};
    };

    const gus::domain::world::ScenePropPlacement onde{
        gus::domain::world::ScenePropKind::CasaCibergoticaA, kCasaCelulaX,
        kCasaCelulaY};
    const gus::domain::world::ScenePropDef& def =
        gus::domain::world::scene_prop_def(onde.kind);
    ScenePropInstance casa;
    casa.footprint = gus::domain::world::scene_prop_footprint(onde, def, kTileCidade,
                                                              t.scene_prop_scale);
    casa.solid = gus::domain::world::scene_prop_solid(casa.footprint, def, kTileCidade,
                                                      t.scene_prop_scale);
    casa.tex = 700;
    casa.cell_x = kCasaCelulaX;
    casa.cell_y = kCasaCelulaY;

    SECTION("na MESMA linha da base: o menino aparece na frente") {
        const Aabb jogador = ancora_na_celula(kJogadorCelulaX, kCasaCelulaY);
        // Trava a premissa do defeito: a base do jogador é MENOR que a da casa (era
        // exatamente isso que a chave escalar lia), e o desenho da casa cobre ele.
        REQUIRE(jogador.y + jogador.h < casa.footprint.y + casa.footprint.h);
        REQUIRE(jogador.x > casa.footprint.x);
        REQUIRE(jogador.x + jogador.w < casa.footprint.x + casa.footprint.w);

        OverworldSim sim(grid, jogador, t);
        sim.add_scene_prop(casa);

        RecordingRenderer r;
        sim.render(r, 4000.0f, 4000.0f, 0.0f);
        REQUIRE(r.index_of(700) >= 0);
        REQUIRE(r.index_of(0) >= 0);
        REQUIRE(r.index_of(700) < r.index_of(0));  // casa antes = jogador NA FRENTE
    }

    SECTION("duas linhas ao NORTE da base: o menino continua escondido atrás") {
        // O contrapeso. Consertar o caso de cima não pode fazer o jogador flutuar na
        // frente de uma casa que ele está claramente contornando por trás - isso
        // seria trocar um defeito por outro pior, porque o jogador ANDA ali.
        const Aabb jogador = ancora_na_celula(kJogadorCelulaX, kCasaCelulaY - 2);
        OverworldSim sim(grid, jogador, t);
        sim.add_scene_prop(casa);

        RecordingRenderer r;
        sim.render(r, 4000.0f, 4000.0f, 0.0f);
        REQUIRE(r.index_of(0) < r.index_of(700));  // jogador antes = ATRÁS da casa
    }

    SECTION("uma linha ao SUL da base: o menino continua na frente") {
        const Aabb jogador = ancora_na_celula(kJogadorCelulaX, kCasaCelulaY + 1);
        OverworldSim sim(grid, jogador, t);
        sim.add_scene_prop(casa);

        RecordingRenderer r;
        sim.render(r, 4000.0f, 4000.0f, 0.0f);
        REQUIRE(r.index_of(700) < r.index_of(0));
    }

    SECTION("A TRANSIÇÃO: com o corpo a cavalo na borda da célula, ainda fica atrás") {
        // ONDE exatamente o menino deixa de estar atrás e passa a estar na frente.
        // Enquanto a caixa dos pés dele estiver PARTIDA entre a linha da peça e a
        // linha de cima, ele ainda não está nivelado com a base: continua atrás. A
        // troca acontece quando o corpo INTEIRO entra na linha da peça. Fixar isto
        // por teste é o que impede a virada de escorregar sem ninguém ver, já que a
        // largura dessa faixa é só 0,6 de uma célula.
        const Aabb dentro = ancora_na_celula(kJogadorCelulaX, kCasaCelulaY);
        const float topo_da_celula =
            static_cast<float>(kCasaCelulaY) * kTileCidade;  // 42,0
        Aabb a_cavalo = dentro;
        a_cavalo.y = topo_da_celula - dentro.h * 0.5f;  // metade dentro, metade fora
        REQUIRE(a_cavalo.y < topo_da_celula);
        REQUIRE(a_cavalo.y + a_cavalo.h > topo_da_celula);

        OverworldSim sim(grid, a_cavalo, t);
        sim.add_scene_prop(casa);
        RecordingRenderer r;
        sim.render(r, 4000.0f, 4000.0f, 0.0f);
        REQUIRE(r.index_of(0) < r.index_of(700));  // ainda atrás

        // Um fio de cabelo adiante, com o corpo inteiro dentro da linha: vira.
        Aabb inteiro_dentro = dentro;
        inteiro_dentro.y = topo_da_celula;
        OverworldSim sim2(grid, inteiro_dentro, t);
        sim2.add_scene_prop(casa);
        RecordingRenderer r2;
        sim2.render(r2, 4000.0f, 4000.0f, 0.0f);
        REQUIRE(r2.index_of(700) < r2.index_of(0));  // agora na frente
    }

    SECTION("a faixa lateral do jogador é a do SPRITE, não a da hitbox") {
        // O sprite do Gus é bem mais largo que a caixa dos pés (2,75 células contra
        // 0,6). Se a faixa lateral viesse da hitbox, um menino parado logo ao lado
        // da casa teria a caixa FORA do desenho dela e o sprite DENTRO: nenhuma
        // aresta seria criada, a comparação cairia na borda de chão, e a parede
        // voltaria a cortar o ombro dele - o defeito original, agora só na margem.
        PlayerSpriteSet sprites;
        for (int d = 0; d < gus::app::screens::kDirectionCount; ++d) {
            sprites.idle[d] = 900 + d;
            sprites.idle_frames[d][0] = sprites.idle[d];
            sprites.idle_count[d] = 1;
        }

        // Uma célula à direita da borda direita do desenho da casa: a hitbox fica
        // fora do retângulo pintado, o sprite não.
        const float borda_direita = casa.footprint.x + casa.footprint.w;
        Aabb jogador = ancora_na_celula(kJogadorCelulaX, kCasaCelulaY);
        jogador.x = borda_direita + 0.1f;
        REQUIRE(jogador.x > borda_direita);
        const float meio_sprite = t.player_sprite_height_tiles * kTileCidade * 0.5f;
        REQUIRE(jogador.x + jogador.w * 0.5f - meio_sprite < borda_direita);

        OverworldSim sim(grid, jogador, t);
        sim.set_player_sprites(sprites);
        sim.add_scene_prop(casa);

        RecordingRenderer r;
        sim.render(r, 4000.0f, 4000.0f, 0.0f);
        const int i_jogador = r.index_of(sprites.idle[static_cast<int>(
            gus::app::screens::Direction::South)]);
        REQUIRE(i_jogador >= 0);
        REQUIRE(r.index_of(700) < i_jogador);  // casa antes = ombro preservado
    }

    SECTION("a mesma transição vale para NPC/inimigo, não só para o jogador") {
        // Os atores passam pela mesma regra, mas por OUTRA linha de código - e essa
        // linha tem de ser exercitada por si. Um Bertoldo a cavalo na borda da
        // célula da casa continua atrás; com o corpo inteiro dentro, vem à frente.
        // Vale mais aqui do que para o jogador: o posicionamento de ator não olha a
        // caixa que bloqueia (só as paredes do mapa), então ator NIVELADO com a
        // base de uma peça é situação comum, não canto raro.
        const float topo_da_celula = static_cast<float>(kCasaCelulaY) * kTileCidade;
        const Aabb longe = ancora_na_celula(kJogadorCelulaX, kCasaCelulaY - 8);

        WorldActorSpec a_cavalo;
        a_cavalo.role = WorldActorRole::Npc;
        a_cavalo.anchor = ancora_na_celula(kJogadorCelulaX, kCasaCelulaY);
        a_cavalo.anchor.y = topo_da_celula - a_cavalo.anchor.h * 0.5f;
        a_cavalo.tex = 800;
        a_cavalo.sprite_height_tiles = 1.0f;

        OverworldSim sim(grid, longe, t);
        sim.add_scene_prop(casa);
        REQUIRE(sim.add_actor(a_cavalo) >= 0);
        RecordingRenderer r;
        sim.render(r, 4000.0f, 4000.0f, 0.0f);
        REQUIRE(r.index_of(800) >= 0);
        REQUIRE(r.index_of(800) < r.index_of(700));  // ainda atrás da casa

        WorldActorSpec dentro = a_cavalo;
        dentro.anchor.y = topo_da_celula;
        OverworldSim sim2(grid, longe, t);
        sim2.add_scene_prop(casa);
        REQUIRE(sim2.add_actor(dentro) >= 0);
        RecordingRenderer r2;
        sim2.render(r2, 4000.0f, 4000.0f, 0.0f);
        REQUIRE(r2.index_of(700) < r2.index_of(800));  // agora na frente
    }

    SECTION("peça ATRAVESSÁVEL (sem caixa que bloqueia) vale pela CÉLULA que pisa") {
        // O holograma do Sterling é o único de pé que não bloqueia nada: a caixa
        // sólida dele é degenerada. Sem um piso, a fatia de chão viraria uma LINHA
        // na base da célula, e o jogador nivelado com ele voltaria a sumir atrás de
        // uma PROJEÇÃO - o pior lugar possível para o defeito reaparecer, porque
        // luz não esconde ninguém. Quem segura é a célula em que a peça foi
        // plantada, que é a própria definição de "onde a peça pisa".
        const gus::domain::world::ScenePropPlacement onde_holo{
            gus::domain::world::ScenePropKind::HoloSterling, kCasaCelulaX,
            kCasaCelulaY};
        const gus::domain::world::ScenePropDef& holo_def =
            gus::domain::world::scene_prop_def(onde_holo.kind);
        ScenePropInstance holo;
        holo.footprint = gus::domain::world::scene_prop_footprint(
            onde_holo, holo_def, kTileCidade, t.scene_prop_scale);
        holo.solid = gus::domain::world::scene_prop_solid(
            holo.footprint, holo_def, kTileCidade, t.scene_prop_scale);
        holo.tex = 701;
        holo.cell_x = kCasaCelulaX;
        holo.cell_y = kCasaCelulaY;
        // Trava a premissa: esta peça REALMENTE não tem caixa sólida.
        REQUIRE(holo.solid.h == 0.0f);

        const Aabb jogador = ancora_na_celula(kCasaCelulaX + 1, kCasaCelulaY);
        OverworldSim sim(grid, jogador, t);
        sim.add_scene_prop(holo);

        RecordingRenderer r;
        sim.render(r, 4000.0f, 4000.0f, 0.0f);
        REQUIRE(r.index_of(701) >= 0);
        REQUIRE(r.index_of(701) < r.index_of(0));  // holograma atrás do menino
    }
}

// ===========================================================================
//  B2 - vários NPCs e vários inimigos (os dois slots únicos viraram lista)
// ===========================================================================

TEST_CASE("mundo: cidade nasce sem nenhum ator", "[overworld][actors]") {
    OverworldSim sim(open_grid(), Aabb{40.0f, 40.0f, 8.0f, 8.0f}, OverworldTuning{});
    REQUIRE(sim.actors().empty());
}

TEST_CASE("mundo: vários NPCs e vários inimigos convivem na mesma lista",
          "[overworld][actors]") {
    OverworldSim sim(open_grid(), Aabb{40.0f, 40.0f, 8.0f, 8.0f}, OverworldTuning{});
    const int a = sim.add_actor(make_actor(WorldActorRole::Npc, 100.0f, 60.0f, 10));
    const int b = sim.add_actor(make_actor(WorldActorRole::Enemy, 140.0f, 80.0f, 11));
    const int c = sim.add_actor(make_actor(WorldActorRole::Enemy, 180.0f, 100.0f, 12));
    const int d = sim.add_actor(make_actor(WorldActorRole::Npc, 220.0f, 120.0f, 13));

    REQUIRE(a != b);
    REQUIRE(b != c);
    REQUIRE(c != d);
    REQUIRE(sim.actors().size() == 4);

    RecordingRenderer r;
    sim.render(r, 4000.0f, 4000.0f, 0.0f);
    REQUIRE(r.index_of(10) >= 0);
    REQUIRE(r.index_of(11) >= 0);
    REQUIRE(r.index_of(12) >= 0);
    REQUIRE(r.index_of(13) >= 0);
}

TEST_CASE("mundo: Y-sort correto com N entidades e não só com três",
          "[overworld][actors][depth-sort]") {
    // O bug que esta spec impede: a ordenação antiga era um array de 3 posições.
    // Aqui são 6 atores mais o jogador, deliberadamente adicionados FORA de ordem
    // de profundidade - a sequência de desenho tem que sair crescente em Y.
    OverworldSim sim(open_grid(40, 40), Aabb{40.0f, 300.0f, 8.0f, 8.0f},
                     OverworldTuning{});
    const float ys[] = {200.0f, 40.0f, 360.0f, 120.0f, 280.0f, 80.0f};
    const TextureId texs[] = {21, 22, 23, 24, 25, 26};
    for (int i = 0; i < 6; ++i) {
        sim.add_actor(make_actor(WorldActorRole::Enemy, 100.0f + 20.0f * i, ys[i],
                                 texs[i]));
    }

    RecordingRenderer r;
    sim.render(r, 8000.0f, 8000.0f, 0.0f);
    REQUIRE(r.sprites.size() == 7);  // 6 atores + fallback do jogador

    // Ordem esperada por base (y+8): 22(48) 26(88) 24(128) 21(208) 25(288)
    // jogador(308) 23(368).
    REQUIRE(r.index_of(22) < r.index_of(26));
    REQUIRE(r.index_of(26) < r.index_of(24));
    REQUIRE(r.index_of(24) < r.index_of(21));
    REQUIRE(r.index_of(21) < r.index_of(25));
    REQUIRE(r.index_of(25) < r.index_of(0));   // jogador
    REQUIRE(r.index_of(0) < r.index_of(23));
}

TEST_CASE("mundo: cada ator da lista bloqueia o jogador com corpo próprio",
          "[overworld][actors][solid-collision]") {
    OverworldTuning t;
    t.corner.enabled = false;
    // A faixa Y do jogador tem que SOBREPOR a do corpo sólido: a âncora do ator
    // tem base em 58 e o corpo sólido de 1 tile fica em y:[42,58). Sobreposição é
    // meio-aberta, então um jogador em y=58 passaria raspando sem tocar.
    OverworldSim sim(open_grid(), Aabb{20.0f, 46.0f, 8.0f, 8.0f}, t);
    // Dois atores na mesma faixa Y; o primeiro barra antes do segundo.
    sim.add_actor(make_actor(WorldActorRole::Enemy, 100.0f, 50.0f, 30));
    sim.add_actor(make_actor(WorldActorRole::Npc, 200.0f, 50.0f, 31));

    for (int i = 0; i < 200; ++i) {
        sim.step_fixed(1, 0, false, 1.0f / 60.0f);
    }
    // Corpo sólido = npc_solid_box_tiles (1 tile) centrado em X sobre a âncora
    // (104) com base na base dela (58): x em [96,112).
    REQUIRE(sim.player().x + sim.player().w <= 96.0f + kEps);
}

TEST_CASE("mundo: remove_actor tira o ator do desenho e da colisão",
          "[overworld][actors]") {
    OverworldTuning t;
    t.corner.enabled = false;
    // Mesma faixa Y do corpo sólido (ver a nota do teste de bloqueio acima): sem
    // isso o jogador passaria livre de qualquer jeito e o teste aprovaria sozinho.
    OverworldSim sim(open_grid(), Aabb{20.0f, 46.0f, 8.0f, 8.0f}, t);
    const int h = sim.add_actor(make_actor(WorldActorRole::Enemy, 100.0f, 50.0f, 30));
    sim.remove_actor(h);

    RecordingRenderer r;
    sim.render(r, 4000.0f, 4000.0f, 0.0f);
    REQUIRE(r.index_of(30) < 0);

    const float x0 = sim.player().x;
    for (int i = 0; i < 200; ++i) {
        sim.step_fixed(1, 0, false, 1.0f / 60.0f);
    }
    REQUIRE(sim.player().x > x0 + 100.0f);
}

TEST_CASE("mundo: handle de ator continua válido depois de remover outro",
          "[overworld][actors]") {
    // Remoção não pode embaralhar índices: quem guardou um handle (a Maestro, o
    // save) tem que continuar apontando para o MESMO ator.
    OverworldSim sim(open_grid(), Aabb{20.0f, 58.0f, 8.0f, 8.0f}, OverworldTuning{});
    const int a = sim.add_actor(make_actor(WorldActorRole::Enemy, 100.0f, 50.0f, 30));
    const int b = sim.add_actor(make_actor(WorldActorRole::Enemy, 140.0f, 50.0f, 31));
    sim.remove_actor(a);

    const auto pos_b = sim.actor_anchor(b);
    REQUIRE(pos_b.has_value());
    REQUIRE_THAT(pos_b->x, WithinAbs(140.0, kEps));
    REQUIRE_FALSE(sim.actor_anchor(a).has_value());
}

TEST_CASE("mundo: handle inválido em qualquer API de ator é no-op seguro",
          "[overworld][actors]") {
    OverworldSim sim(open_grid(), Aabb{20.0f, 58.0f, 8.0f, 8.0f}, OverworldTuning{});
    REQUIRE_FALSE(sim.actor_anchor(-1).has_value());
    REQUIRE_FALSE(sim.actor_anchor(99).has_value());
    REQUIRE_NOTHROW(sim.remove_actor(-1));
    REQUIRE_NOTHROW(sim.remove_actor(99));
    REQUIRE_NOTHROW(sim.set_actor_patrol(99, line_route(0.0f, 0.0f, 3.0f, 1.0f)));
}

TEST_CASE("mundo: a fachada legada do inimigo é um ator da lista",
          "[overworld][actors][legacy]") {
    // A fatia trocou os slots únicos por listas SEM quebrar quem já chamava a
    // API antiga (Maestro, SdlWindow, ferramentas): o marcador virou um item.
    OverworldSim sim(open_grid(), Aabb{40.0f, 40.0f, 8.0f, 8.0f}, OverworldTuning{});
    REQUIRE(sim.actors().empty());
    sim.set_enemy_marker(Aabb{100.0f, 50.0f, 20.0f, 20.0f}, /*tex=*/7);
    REQUIRE(sim.actors().size() == 1);
    REQUIRE(sim.has_enemy_marker());

    sim.set_npc_bertoldo_marker(Aabb{140.0f, 50.0f, 20.0f, 20.0f}, /*tex=*/9);
    REQUIRE(sim.actors().size() == 2);

    // Reposicionar NÃO cria ator novo (o slot é reservado, não empilha).
    sim.set_enemy_marker(Aabb{180.0f, 50.0f, 20.0f, 20.0f}, /*tex=*/7);
    REQUIRE(sim.actors().size() == 2);

    const auto pos = sim.enemy_marker_aabb();
    REQUIRE(pos.has_value());
    REQUIRE_THAT(pos->x, WithinAbs(180.0, kEps));
}

TEST_CASE("mundo: sem marcador de inimigo a posição corrente é vazia",
          "[overworld][actors][legacy]") {
    OverworldSim sim(open_grid(), Aabb{40.0f, 40.0f, 8.0f, 8.0f}, OverworldTuning{});
    REQUIRE_FALSE(sim.enemy_marker_aabb().has_value());
    sim.set_enemy_marker(Aabb{100.0f, 50.0f, 20.0f, 20.0f}, /*tex=*/7);
    sim.clear_enemy_marker();
    REQUIRE_FALSE(sim.enemy_marker_aabb().has_value());
}

// ===========================================================================
//  B3 - ronda em rota fixa
// ===========================================================================

TEST_CASE("mundo: ator sem rota nunca sai do lugar", "[overworld][patrol]") {
    OverworldSim sim(open_grid(), Aabb{40.0f, 40.0f, 8.0f, 8.0f}, OverworldTuning{});
    const int h = sim.add_actor(make_actor(WorldActorRole::Enemy, 100.0f, 50.0f, 30));
    for (int i = 0; i < 300; ++i) {
        sim.step_fixed(0, 0, false, 1.0f / 60.0f);
    }
    REQUIRE_THAT(sim.actor_anchor(h)->x, WithinAbs(100.0, kEps));
    REQUIRE_THAT(sim.actor_anchor(h)->y, WithinAbs(50.0, kEps));
}

TEST_CASE("mundo: inimigo com rota anda pela rota", "[overworld][patrol]") {
    OverworldSim sim(open_grid(), Aabb{40.0f, 40.0f, 8.0f, 8.0f}, OverworldTuning{});
    WorldActorSpec spec = make_actor(WorldActorRole::Enemy, 100.0f, 50.0f, 30);
    // Ronda de 4 tiles a 2 tiles/s, começando na célula onde o ator já está.
    spec.route = line_route(/*cx=*/6.0f, /*cy=*/3.0f, /*len=*/4.0f, /*speed=*/2.0f);
    const int h = sim.add_actor(spec);

    // 1 segundo = 2 tiles = 32 unidades de mundo (tile 16).
    for (int i = 0; i < 60; ++i) {
        sim.step_fixed(0, 0, false, 1.0f / 60.0f);
    }
    REQUIRE_THAT(sim.actor_anchor(h)->x, WithinAbs(132.0, 0.5));
    REQUIRE_THAT(sim.actor_anchor(h)->y, WithinAbs(50.0, kEps));
}

TEST_CASE("mundo: a ronda é ida e VOLTA - o inimigo retorna ao ponto de partida",
          "[overworld][patrol]") {
    OverworldSim sim(open_grid(), Aabb{40.0f, 40.0f, 8.0f, 8.0f}, OverworldTuning{});
    WorldActorSpec spec = make_actor(WorldActorRole::Enemy, 100.0f, 50.0f, 30);
    spec.route = line_route(6.0f, 3.0f, 4.0f, 2.0f);  // 4 tiles a 2 tiles/s = 2 s
    const int h = sim.add_actor(spec);

    // 2 s de ida (chega na ponta) + 2 s de volta = de novo no ponto de partida.
    for (int i = 0; i < 240; ++i) {
        sim.step_fixed(0, 0, false, 1.0f / 60.0f);
    }
    REQUIRE_THAT(sim.actor_anchor(h)->x, WithinAbs(100.0, 0.5));
}

TEST_CASE("mundo: a ronda avança mesmo com o jogador parado", "[overworld][patrol]") {
    // Guarda de regressão do caminho de saída antecipada do passo fixo: quando o
    // jogador não tecla nada, o passo retornava cedo. Se a ronda ficasse depois
    // daquele ponto, o inimigo só andaria enquanto o jogador andasse - e ninguém
    // notaria, porque no playtest o jogador está quase sempre andando.
    OverworldSim sim(open_grid(), Aabb{40.0f, 40.0f, 8.0f, 8.0f}, OverworldTuning{});
    WorldActorSpec spec = make_actor(WorldActorRole::Enemy, 100.0f, 50.0f, 30);
    spec.route = line_route(6.0f, 3.0f, 4.0f, 2.0f);
    const int h = sim.add_actor(spec);

    const float x0 = sim.actor_anchor(h)->x;
    for (int i = 0; i < 30; ++i) {
        sim.step_fixed(0, 0, false, 1.0f / 60.0f);  // jogador PARADO o tempo todo
    }
    REQUIRE(sim.actor_anchor(h)->x > x0 + 8.0f);
}

TEST_CASE("mundo: o corpo sólido acompanha o inimigo em ronda",
          "[overworld][patrol][solid-collision]") {
    // Sem isto, o inimigo andaria e deixaria a colisão para trás - o jogador
    // trombaria no ar e atravessaria o sprite.
    OverworldTuning t;
    t.corner.enabled = false;
    OverworldSim sim(open_grid(), Aabb{20.0f, 46.0f, 8.0f, 8.0f}, t);
    WorldActorSpec spec = make_actor(WorldActorRole::Enemy, 200.0f, 50.0f, 30);
    spec.route = line_route(/*cx=*/12.0f, 3.0f, /*len=*/-6.0f, /*speed=*/2.0f);
    const int h = sim.add_actor(spec);

    // O inimigo caminha para a ESQUERDA enquanto o jogador caminha para a
    // DIREITA: eles se encontram no meio, e o jogador tem que ser barrado pelo
    // corpo do inimigo na posição NOVA dele.
    for (int i = 0; i < 120; ++i) {
        sim.step_fixed(1, 0, false, 1.0f / 60.0f);
    }
    const auto anchor = sim.actor_anchor(h);
    REQUIRE(anchor.has_value());
    const float solid_left = anchor->x + anchor->w * 0.5f - kTile * 0.5f;
    REQUIRE(sim.player().x + sim.player().w <= solid_left + 0.2f);
}

TEST_CASE("mundo: ronda com pausa segura o inimigo na ponta", "[overworld][patrol]") {
    OverworldSim sim(open_grid(), Aabb{40.0f, 40.0f, 8.0f, 8.0f}, OverworldTuning{});
    WorldActorSpec spec = make_actor(WorldActorRole::Enemy, 100.0f, 50.0f, 30);
    spec.route = line_route(6.0f, 3.0f, 2.0f, 2.0f);  // 2 tiles a 2 tiles/s = 1 s
    spec.route.pause_seconds = 3.0f;
    const int h = sim.add_actor(spec);

    for (int i = 0; i < 90; ++i) {  // 1 s de marcha + 0.5 s da pausa
        sim.step_fixed(0, 0, false, 1.0f / 60.0f);
    }
    // Chegou na ponta (2 tiles = 32 unidades) e está parado lá cumprindo a pausa.
    REQUIRE_THAT(sim.actor_anchor(h)->x, WithinAbs(132.0, 0.5));
    for (int i = 0; i < 60; ++i) {
        sim.step_fixed(0, 0, false, 1.0f / 60.0f);
    }
    REQUIRE_THAT(sim.actor_anchor(h)->x, WithinAbs(132.0, 0.5));
}

TEST_CASE("mundo: set_actor_patrol arma a ronda depois do ator já existir",
          "[overworld][patrol]") {
    OverworldSim sim(open_grid(), Aabb{40.0f, 40.0f, 8.0f, 8.0f}, OverworldTuning{});
    const int h = sim.add_actor(make_actor(WorldActorRole::Enemy, 100.0f, 50.0f, 30));
    sim.set_actor_patrol(h, line_route(6.0f, 3.0f, 4.0f, 2.0f));

    for (int i = 0; i < 60; ++i) {
        sim.step_fixed(0, 0, false, 1.0f / 60.0f);
    }
    REQUIRE(sim.actor_anchor(h)->x > 120.0f);
}

TEST_CASE("mundo: a ronda parte de onde o ator está e não do ponto zero da rota",
          "[overworld][patrol]") {
    // A rota é escrita em CÉLULAS pelo level designer; o ator é posicionado em
    // unidades de mundo pela Maestro. O que amarra os dois é o DESLOCAMENTO em
    // relação ao primeiro ponto - assim uma rota nunca teleporta o ator ao ser
    // armada, mesmo que quem a escreveu tenha errado a célula de origem.
    OverworldSim sim(open_grid(), Aabb{40.0f, 40.0f, 8.0f, 8.0f}, OverworldTuning{});
    const int h = sim.add_actor(make_actor(WorldActorRole::Enemy, 100.0f, 50.0f, 30));
    sim.set_actor_patrol(h, line_route(/*cx=*/0.0f, /*cy=*/0.0f, 4.0f, 2.0f));

    // Armar a rota não moveu ninguém.
    REQUIRE_THAT(sim.actor_anchor(h)->x, WithinAbs(100.0, kEps));
    REQUIRE_THAT(sim.actor_anchor(h)->y, WithinAbs(50.0, kEps));
}

TEST_CASE("mundo: reposicionar pela fachada legada rearma a ronda no lugar novo",
          "[overworld][patrol][legacy]") {
    // Carregar um save reposiciona o inimigo: a ronda tem que recomeçar dali, e
    // não continuar medindo a partir de onde ele estava antes.
    OverworldSim sim(open_grid(40, 40), Aabb{40.0f, 40.0f, 8.0f, 8.0f},
                     OverworldTuning{});
    sim.set_enemy_marker(Aabb{100.0f, 50.0f, 20.0f, 20.0f}, /*tex=*/7);
    sim.set_actor_patrol(sim.enemy_marker_handle(), line_route(6.0f, 3.0f, 4.0f, 2.0f));
    for (int i = 0; i < 30; ++i) {
        sim.step_fixed(0, 0, false, 1.0f / 60.0f);
    }
    REQUIRE(sim.enemy_marker_aabb()->x > 100.0f);

    sim.set_enemy_marker(Aabb{300.0f, 50.0f, 20.0f, 20.0f}, /*tex=*/7);
    REQUIRE_THAT(sim.enemy_marker_aabb()->x, WithinAbs(300.0, kEps));
    for (int i = 0; i < 30; ++i) {
        sim.step_fixed(0, 0, false, 1.0f / 60.0f);
    }
    // Andou a partir da posição NOVA (não voltou para perto da antiga).
    REQUIRE(sim.enemy_marker_aabb()->x > 300.0f);
    REQUIRE(sim.enemy_marker_aabb()->x < 340.0f);
}

TEST_CASE("mundo: o desenho do inimigo em ronda é interpolado entre os passos",
          "[overworld][patrol]") {
    // Mesmo tratamento que o jogador já recebe: sem interpolar, a 60 quadros por
    // segundo o inimigo anda aos trancos enquanto o jogador desliza liso.
    OverworldSim sim(open_grid(), Aabb{40.0f, 300.0f, 8.0f, 8.0f}, OverworldTuning{});
    WorldActorSpec spec = make_actor(WorldActorRole::Enemy, 100.0f, 50.0f, 30);
    spec.route = line_route(6.0f, 3.0f, 4.0f, 2.0f);
    sim.add_actor(spec);
    sim.step_fixed(0, 0, false, 1.0f / 6.0f);  // passo grosso: 1/6 s = 5.33 unidades

    RecordingRenderer r0;
    sim.render(r0, 4000.0f, 4000.0f, 0.0f);
    RecordingRenderer r1;
    sim.render(r1, 4000.0f, 4000.0f, 1.0f);
    const float x_alpha0 = r0.sprites[static_cast<std::size_t>(r0.index_of(30))].rect.x;
    const float x_alpha1 = r1.sprites[static_cast<std::size_t>(r1.index_of(30))].rect.x;
    REQUIRE(x_alpha1 > x_alpha0);

    RecordingRenderer rm;
    sim.render(rm, 4000.0f, 4000.0f, 0.5f);
    const float x_meio = rm.sprites[static_cast<std::size_t>(rm.index_of(30))].rect.x;
    REQUIRE_THAT(x_meio, WithinAbs((x_alpha0 + x_alpha1) * 0.5, 0.05));
}

TEST_CASE("mundo: ator removido no meio da ronda para de andar", "[overworld][patrol]") {
    OverworldSim sim(open_grid(), Aabb{40.0f, 40.0f, 8.0f, 8.0f}, OverworldTuning{});
    WorldActorSpec spec = make_actor(WorldActorRole::Enemy, 100.0f, 50.0f, 30);
    spec.route = line_route(6.0f, 3.0f, 4.0f, 2.0f);
    const int h = sim.add_actor(spec);
    for (int i = 0; i < 30; ++i) {
        sim.step_fixed(0, 0, false, 1.0f / 60.0f);
    }
    sim.remove_actor(h);
    for (int i = 0; i < 60; ++i) {
        sim.step_fixed(0, 0, false, 1.0f / 60.0f);
    }
    REQUIRE_FALSE(sim.actor_anchor(h).has_value());
}

// ===========================================================================
//  CLIPPING-ATOR-RONDA-SEM-COLISAO - regressão do playtest ao vivo do líder
//  (Gus Dragon, 2026-08-07).
//
//  O QUE ELE VIU: parado, encostado num prop sólido, um NPC em ronda andou POR
//  CIMA dele; o menino ficou "preso" na interseção NPC+prop e só saiu apertando
//  Sul. Segundo achado do mesmo playtest: o androide "entra e sai" de objetos
//  livremente, com ou sem o jogador no caminho.
//
//  A CAUSA: até esta fatia, advance_actor_patrols posicionava o ator DIRETO pela
//  rota (advance_patrol) e atualizava a.solid, sem NUNCA passar por
//  resolve_move/resolve_move_with_corner_assist. A resolução contra o
//  ObstacleSpan (props E atores) só rodava do lado do JOGADOR, e só quando ELE se
//  movia - então um jogador PARADO nunca resolvia sobreposição nenhuma, e a
//  interseção ficava de pé até ele apertar uma direção livre.
//
//  O INVARIANTE que estas specs travam, e que vale sob QUALQUER política de
//  atraso/recuperação da rota: o corpo sólido de um ator em ronda NUNCA sobrepõe
//  o corpo do jogador nem uma peça sólida, em NENHUM quadro da simulação. É
//  medido a cada passo (não só no fim), porque a travessia é transitória: uma
//  aferição só no estado final passaria com o ator já do outro lado.
// ===========================================================================

namespace {

// true se os dois retângulos de mundo se sobrepõem (meio-aberto, a MESMA
// convenção de overlaps_aabb do resolve_move).
bool aabb_overlaps(const Aabb& a, const Aabb& b) noexcept {
    return a.x < b.x + b.w && a.x + a.w > b.x && a.y < b.y + b.h && a.y + a.h > b.y;
}

}  // namespace

TEST_CASE("mundo: ator em ronda NÃO atravessa o jogador parado encostado num prop",
          "[overworld][patrol][solid-collision][regressao-lider]") {
    // A CENA EXATA do playtest. Jogador parado com a borda direita COLADA na peça
    // (96 == 96, encosta sem entrar) e um ator em ronda vindo do oeste pela mesma
    // faixa de Y. O jogador não tecla NADA o tempo todo - é justamente essa a
    // condição em que o defeito aparece.
    OverworldTuning t;
    t.corner.enabled = false;  // resultado 100% analítico
    const Aabb jogador{88.0f, 100.0f, 8.0f, 8.0f};  // caixa [88,96) x [100,108)
    OverworldSim sim(open_grid(), jogador, t);

    // Caixa/engradado: base sólida em x:[96,144), y:[96,112).
    sim.add_scene_prop(make_prop(96.0f, 64.0f, 48.0f, 48.0f, /*tex=*/70, true));
    const Aabb prop_solid = sim.scene_props()[0].solid;

    // Ator em ronda: âncora em (20,100) -> corpo sólido [16,32) x [92,108), que
    // SOBREPÕE a faixa Y do jogador. A rota leva ele 6 tiles para leste (96
    // unidades), passando por cima do jogador e entrando na peça.
    WorldActorSpec spec = make_actor(WorldActorRole::Enemy, 20.0f, 100.0f, 30);
    spec.route = line_route(/*cx=*/1.0f, /*cy=*/6.0f, /*len=*/6.0f, /*speed=*/2.0f);
    const int h = sim.add_actor(spec);

    // Premissas da cena (se alguma quebrar, o teste vira tautologia silenciosa).
    REQUIRE_THAT(jogador.x + jogador.w, WithinAbs(prop_solid.x, kEps));  // encostado
    REQUIRE(aabb_overlaps(Aabb{prop_solid.x - 1.0f, 92.0f, 2.0f, 16.0f}, prop_solid));

    // 5 s de simulação: 3 s de ida (6 tiles a 2 tiles/s) + 2 s de volta.
    for (int i = 0; i < 300; ++i) {
        sim.step_fixed(0, 0, false, 1.0f / 60.0f);  // jogador PARADO
        const Aabb corpo = sim.actors()[static_cast<std::size_t>(h)].solid;
        // (1) nunca ocupa a mesma posição do jogador...
        REQUIRE_FALSE(aabb_overlaps(corpo, sim.player()));
        // (2) ...nem entra na peça sólida.
        REQUIRE_FALSE(aabb_overlaps(corpo, prop_solid));
    }

    // E o jogador continua exatamente onde estava: ninguém o empurrou para dentro
    // da peça (o oposto do defeito, que era ele acabar DENTRO da interseção).
    REQUIRE_THAT(sim.player().x, WithinAbs(jogador.x, kEps));
    REQUIRE_THAT(sim.player().y, WithinAbs(jogador.y, kEps));
}

TEST_CASE("mundo: ator em ronda NÃO entra e sai de peça sólida",
          "[overworld][patrol][solid-collision][regressao-lider]") {
    // O segundo achado do mesmo playtest, isolado do jogador: o androide
    // atravessava objeto do cenário mesmo sem ninguém no caminho. A rota nasce
    // conferida contra as PAREDES do mapa (city_patrol.hpp), nunca contra as
    // peças - então este caso não é canto raro, é o dia a dia da cidade vestida.
    OverworldTuning t;
    t.corner.enabled = false;
    OverworldSim sim(open_grid(), Aabb{20.0f, 20.0f, 8.0f, 8.0f}, t);  // longe

    sim.add_scene_prop(make_prop(200.0f, 64.0f, 48.0f, 48.0f, /*tex=*/70, true));
    const Aabb prop_solid = sim.scene_props()[0].solid;  // [200,248) x [96,112)

    WorldActorSpec spec = make_actor(WorldActorRole::Enemy, 100.0f, 100.0f, 30);
    spec.route = line_route(/*cx=*/6.0f, /*cy=*/6.0f, /*len=*/8.0f, /*speed=*/2.0f);
    const int h = sim.add_actor(spec);

    for (int i = 0; i < 480; ++i) {  // 4 s de ida + 4 s de volta
        sim.step_fixed(0, 0, false, 1.0f / 60.0f);
        REQUIRE_FALSE(
            aabb_overlaps(sim.actors()[static_cast<std::size_t>(h)].solid, prop_solid));
    }
}

TEST_CASE("mundo: ator em ronda NÃO atravessa parede da grade",
          "[overworld][patrol][solid-collision][regressao-lider]") {
    // Rede de segurança do mesmo mecanismo. Hoje a rota do demo é construída
    // encurtando nas paredes (build_horizontal_patrol_route), então a grade não
    // costuma ser exercitada - mas rota é DADO escrito à mão, e dado escrito à
    // mão erra. Uma rota que atravessa prédio tem de virar "ele para na parede",
    // nunca "ele anda dentro do prédio".
    OverworldTuning t;
    t.corner.enabled = false;
    TileGrid grid = open_grid();
    grid.set_blocked(10, 6, true);  // parede na célula (10,6) = x:[160,176) y:[96,112)
    OverworldSim sim(grid, Aabb{20.0f, 20.0f, 8.0f, 8.0f}, t);

    WorldActorSpec spec = make_actor(WorldActorRole::Enemy, 100.0f, 100.0f, 30);
    spec.route = line_route(/*cx=*/6.0f, /*cy=*/6.0f, /*len=*/6.0f, /*speed=*/2.0f);
    const int h = sim.add_actor(spec);

    for (int i = 0; i < 360; ++i) {
        sim.step_fixed(0, 0, false, 1.0f / 60.0f);
        const Aabb corpo = sim.actors()[static_cast<std::size_t>(h)].solid;
        REQUIRE_FALSE(aabb_overlaps(corpo, Aabb{160.0f, 96.0f, 16.0f, 16.0f}));
    }
}

// ---------------------------------------------------------------------------
//  D4 - a ronda andava FORA do trecho que foi conferido contra as paredes.
//
//  Defeito medido em 2026-08-07 contra o .gmap real, e SEPARADO do clipping
//  acima: build_horizontal_patrol_route escreve waypoints[0] = celula - alcance,
//  e rearm_patrol ancorava o deslocamento da rota nesse waypoint[0] estando o
//  ator na celula do MEIO. Resultado: o trecho conferido era [c-alcance,
//  c+alcance] e o percorrido era [c, c+2*alcance], deslocado de um alcance
//  inteiro. No demo isso pos a Vanda em [30..38] (conferido [26..34]) e o
//  androide da FIR em [74..86] (conferido [68..80]) - e a celula (86,50) do
//  percurso real dele E PAREDE, ou seja, ele terminava a ida dentro de um
//  predio. E a explicacao medida do "o androide entra e sai de objetos
//  livremente" do playtest.
// ---------------------------------------------------------------------------
TEST_CASE("mundo: a ronda anda no trecho que foi CONFERIDO contra as paredes",
          "[overworld][patrol][regressao-lider]") {
    OverworldTuning t;
    t.corner.enabled = false;
    TileGrid grid = open_grid();
    grid.set_blocked(14, 3, true);  // corta o alcance do lado leste

    // Ator no CENTRO da celula (10,3): a mesma convencao de
    // pick_actor_position_at_cell (caixa centrada na celula).
    OverworldSim sim(grid, Aabb{20.0f, 250.0f, 8.0f, 8.0f}, t);  // jogador longe
    WorldActorSpec spec = make_actor(WorldActorRole::Enemy, 164.0f, 52.0f, 30);
    spec.route = gus::app::screens::build_horizontal_patrol_route(
        grid, /*cell_x=*/10, /*cell_y=*/3, /*reach_tiles=*/3, /*speed=*/2.0f,
        /*pause=*/0.0f);
    const int h = sim.add_actor(spec);

    // A rota nasceu encurtada na parede: celulas 7 a 13 (a 14 esta bloqueada).
    REQUIRE(spec.route.valid());
    REQUIRE_THAT(spec.route.waypoints[0].x, WithinAbs(7.0, kEps));
    REQUIRE_THAT(spec.route.waypoints[1].x, WithinAbs(13.0, kEps));

    // O trecho CONFERIDO, em unidades de mundo, para uma ancora de 8 de largura
    // centrada na celula: da celula 7 a celula 13.
    const float limite_oeste = 164.0f - 3.0f * kTile;          // 116
    const float limite_leste = 164.0f + 3.0f * kTile + 8.0f;   // 220

    for (int i = 0; i < 600; ++i) {  // 10 s: varias idas e voltas
        sim.step_fixed(0, 0, false, 1.0f / 60.0f);
        const auto anc = sim.actor_anchor(h);
        REQUIRE(anc.has_value());
        REQUIRE(anc->x >= limite_oeste - 0.05f);
        REQUIRE(anc->x + anc->w <= limite_leste + 0.05f);
    }
}

// ---------------------------------------------------------------------------
//  D1 (decisao do lider) - "continua de onde parou, sem pressa".
//
//  O relogio da rota so avanca enquanto o ator efetivamente anda. Ele nunca
//  acelera para recuperar o atraso e nunca salta para o ponto onde a rota diria
//  que ele deveria estar. A rota e um trilho com o ator EM CIMA dele, nao um
//  alvo que o ator persegue.
// ---------------------------------------------------------------------------
TEST_CASE("mundo: ator barrado continua de onde parou e NUNCA acelera",
          "[overworld][patrol][solid-collision][regressao-lider]") {
    OverworldTuning t;
    t.corner.enabled = false;
    // ISOLA O D1 DO D2: sem isto a meia-volta dispararia no meio da medicao e o
    // teste passaria a medir DOIS comportamentos de uma vez. A meia-volta tem
    // spec propria logo abaixo.
    t.actor_blocked_turnaround_seconds = 1000.0f;
    // Jogador parado no caminho da ronda, na MESMA faixa Y do corpo do ator.
    OverworldSim sim(open_grid(), Aabb{160.0f, 100.0f, 8.0f, 8.0f}, t);
    WorldActorSpec spec = make_actor(WorldActorRole::Enemy, 20.0f, 100.0f, 30);
    spec.route = line_route(/*cx=*/1.0f, /*cy=*/6.0f, /*len=*/15.0f, /*speed=*/2.0f);
    const int h = sim.add_actor(spec);

    // Passo maximo LEGITIMO da ronda: velocidade * tile * dt.
    const float passo_da_ronda = 2.0f * kTile / 60.0f;
    float maior_passo = 0.0f;
    float x_antes = sim.actor_anchor(h)->x;

    // Fase 1: ele avanca ate esbarrar no jogador e fica preso 4 s.
    for (int i = 0; i < 300; ++i) {
        sim.step_fixed(0, 0, false, 1.0f / 60.0f);
        const float x = sim.actor_anchor(h)->x;
        const float d = x - x_antes < 0.0f ? x_antes - x : x - x_antes;
        if (d > maior_passo) maior_passo = d;
        x_antes = x;
    }
    // Barrado pelo corpo do jogador. Quem encosta e o CORPO SOLIDO do ator (1
    // tile = 16, centrado na ancora de 8, logo solid.x = ancora.x - 4), nao a
    // ancora: solid.x + 16 == 160 => ancora.x == 148.
    const float x_travado = sim.actor_anchor(h)->x;
    REQUIRE_THAT(x_travado, WithinAbs(148.0, 0.05));

    // Fase 2: o caminho abre (teleporte legitimo do jogador, como um load faria).
    sim.set_player_position(Aabb{160.0f, 280.0f, 8.0f, 8.0f});
    for (int i = 0; i < 120; ++i) {
        sim.step_fixed(0, 0, false, 1.0f / 60.0f);
        const float x = sim.actor_anchor(h)->x;
        const float d = x - x_antes < 0.0f ? x_antes - x : x - x_antes;
        if (d > maior_passo) maior_passo = d;
        x_antes = x;
    }

    // A TRAVA DO D1: em NENHUM quadro ele andou mais que o passo da ronda. Com a
    // politica rejeitada ("corre pra recuperar"), o passo de recuperacao iria ao
    // teto anti-tunneling (0.95 tile = 15.2), quase 30x isto.
    REQUIRE(maior_passo <= passo_da_ronda * 1.02f);
    // E ele nao ficou travado depois que o caminho abriu.
    REQUIRE(sim.actor_anchor(h)->x > x_travado + 8.0f);
}

// ---------------------------------------------------------------------------
//  D2 (decisao do lider) - bloqueio prolongado vira MEIA-VOLTA.
//
//  Sem isto, "a rota espera pelo ator" tem um custo escondido: um ator barrado
//  por algo que nunca sai (uma peca no meio da rota, outro ator vindo de frente)
//  espera para SEMPRE, e a ronda dele morre em pe. Meia-volta e a saida que se
//  cura sozinha e ainda le bem em cena ("viu o obstaculo e voltou").
// ---------------------------------------------------------------------------
TEST_CASE("mundo: ator bloqueado por muito tempo da meia-volta na ronda",
          "[overworld][patrol][solid-collision][regressao-lider]") {
    OverworldTuning t;
    t.corner.enabled = false;
    OverworldSim sim(open_grid(), Aabb{20.0f, 250.0f, 8.0f, 8.0f}, t);  // longe
    // Peca solida atravessada na rota: [96,144) x [96,112).
    sim.add_scene_prop(make_prop(96.0f, 64.0f, 48.0f, 48.0f, /*tex=*/70, true));

    WorldActorSpec spec = make_actor(WorldActorRole::Enemy, 20.0f, 100.0f, 30);
    spec.route = line_route(1.0f, 6.0f, 15.0f, 2.0f);
    const int h = sim.add_actor(spec);

    // 2.25 s: ele sai de 20, anda 64 unidades a 32 u/s e trava aos 2 s. Quem
    // encosta e o CORPO SOLIDO (16 de largura, solid.x = ancora.x - 4), entao
    // solid.x + 16 == 96 => ancora.x == 84. Aos 2.25 s ele esta parado ali ha
    // 0.25 s - ainda DENTRO da janela de 0.5 s da meia-volta.
    for (int i = 0; i < 135; ++i) {
        sim.step_fixed(0, 0, false, 1.0f / 60.0f);
    }
    const float x_travado = sim.actor_anchor(h)->x;
    REQUIRE_THAT(x_travado, WithinAbs(84.0, 0.05));
    REQUIRE(sim.actors()[static_cast<std::size_t>(h)].blocked_seconds > 0.0f);

    // Mais 1 s: a meia-volta dispara aos 2.5 s e ele passa a voltar para oeste.
    for (int i = 0; i < 60; ++i) {
        sim.step_fixed(0, 0, false, 1.0f / 60.0f);
    }
    REQUIRE(sim.actor_anchor(h)->x < x_travado - kTile);
    // E a contagem de bloqueio zerou: ele nao esta mais barrado, esta andando.
    REQUIRE_THAT(sim.actors()[static_cast<std::size_t>(h)].blocked_seconds,
                 WithinAbs(0.0, kEps));
}

// ===========================================================================
//  DEPENETRACAO DO JOGADOR - blindagem contra a classe de exploit de
//  sobreposicao deliberada (pedido do lider, 2026-08-07).
//
//  Contexto: quem achou o clipping estuda glitch hunting, e "corpo empurra o
//  jogador para dentro da parede, jogador atravessa" e um exploit documentado
//  em jogos comerciais. A defesa padrao nao e tapar o caso particular: e uma
//  passada que, a CADA quadro e INDEPENDENTE de input, tira qualquer corpo que
//  esteja sobreposto a um bloqueador, empurrando pela MENOR distancia de saida.
//
//  A trava e "sem apertar tecla nenhuma": o defeito original do playtest era
//  exatamente uma sobreposicao que so se resolvia quando o jogador se movia.
// ===========================================================================

TEST_CASE("mundo: jogador sobreposto a uma peca solida e expulso sem apertar tecla",
          "[overworld][depenetracao][regressao-lider]") {
    OverworldTuning t;
    t.corner.enabled = false;
    OverworldSim sim(open_grid(), Aabb{20.0f, 20.0f, 8.0f, 8.0f}, t);
    sim.add_scene_prop(make_prop(96.0f, 64.0f, 48.0f, 48.0f, /*tex=*/70, true));
    const Aabb prop_solid = sim.scene_props()[0].solid;  // [96,144) x [96,112)

    // O estado que um exploit produziria: corpo DENTRO do solido, sem ter
    // passado por resolve_move nenhum.
    sim.set_player_position(Aabb{92.0f, 100.0f, 8.0f, 8.0f});  // [92,100) x [100,108)
    REQUIRE(aabb_overlaps(sim.player(), prop_solid));

    sim.step_fixed(0, 0, false, 1.0f / 60.0f);  // NENHUMA tecla

    REQUIRE_FALSE(aabb_overlaps(sim.player(), prop_solid));
    // Saiu pela MENOR distancia (4 unidades para oeste), nao por uma direcao
    // qualquer: pelo norte/sul seriam 12 e pelo leste 52.
    REQUIRE_THAT(sim.player().x, WithinAbs(88.0, 0.01));
    REQUIRE_THAT(sim.player().y, WithinAbs(100.0, 0.01));
}

TEST_CASE("mundo: jogador sobreposto a uma PAREDE da grade e expulso sem apertar tecla",
          "[overworld][depenetracao][regressao-lider]") {
    OverworldTuning t;
    t.corner.enabled = false;
    TileGrid grid = open_grid();
    grid.set_blocked(10, 6, true);  // x:[160,176) y:[96,112)
    OverworldSim sim(grid, Aabb{20.0f, 20.0f, 8.0f, 8.0f}, t);

    sim.set_player_position(Aabb{156.0f, 100.0f, 8.0f, 8.0f});  // entra 4 na parede
    sim.step_fixed(0, 0, false, 1.0f / 60.0f);

    REQUIRE_FALSE(aabb_overlaps(sim.player(), Aabb{160.0f, 96.0f, 16.0f, 16.0f}));
    REQUIRE_THAT(sim.player().x, WithinAbs(152.0, 0.01));
}

TEST_CASE("mundo: jogador ENCOSTADO (sem sobrepor) nao e empurrado por nada",
          "[overworld][depenetracao]") {
    // O contrapeso da blindagem, e o que a impede de virar um defeito novo: o
    // resolve_move deixa o jogador com a borda COINCIDINDO com a do obstaculo o
    // tempo todo, e sobreposicao e meio-aberta. Se a depenetracao lesse encosto
    // como sobreposicao, o jogador seria empurrado para longe de toda parede em
    // que encostasse - visivel em todo quadro do jogo.
    OverworldTuning t;
    t.corner.enabled = false;
    OverworldSim sim(open_grid(), Aabb{20.0f, 20.0f, 8.0f, 8.0f}, t);
    sim.add_scene_prop(make_prop(96.0f, 64.0f, 48.0f, 48.0f, /*tex=*/70, true));

    sim.set_player_position(Aabb{88.0f, 100.0f, 8.0f, 8.0f});  // borda em 96 == 96
    for (int i = 0; i < 60; ++i) {
        sim.step_fixed(0, 0, false, 1.0f / 60.0f);
    }
    REQUIRE_THAT(sim.player().x, WithinAbs(88.0, kEps));
    REQUIRE_THAT(sim.player().y, WithinAbs(100.0, kEps));
}

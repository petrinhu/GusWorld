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

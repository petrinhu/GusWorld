// GusEngine/app/tests/overworld_sim_test.cpp
//
// Catch2 da simulacao do overworld (app/screens): junta input cardinal +
// resolve_move (colisao slide) + clamp_camera + interpolacao de render. TEST-FIRST.
// Headless: usa um IRenderer FALSO que so registra o que seria desenhado, sem GPU.
//
// O OverworldSim e a "regra de jogo POCO fora da casca" (engine-design.md sec 2/4):
// recebe dx/dy CRUS (int, ja resolvidos pelo input) + dt fixo. A casca SDL
// (SdlWindow/main) so o alimenta e desenha. Isso o torna testavel sem janela - e
// prova o feel (desliza nas paredes, camera presa ao mapa) deterministicamente.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <cmath>
#include <vector>

#include "gus/app/screens/overworld_sim.hpp"
#include "gus/app/screens/overworld_tuning.hpp"
#include "gus/core/spatial/tile_grid.hpp"
#include "gus/platform/render2d/i_renderer.hpp"

using Catch::Matchers::WithinAbs;
using gus::app::screens::Direction;
using gus::app::screens::kWalkFrameCount;
using gus::app::screens::OverworldSim;
using gus::app::screens::OverworldTuning;
using gus::app::screens::PlayerSpriteSet;
using gus::core::spatial::Aabb;
using gus::core::spatial::CameraView;
using gus::core::spatial::Rect;
using gus::core::spatial::TileGrid;
using gus::platform::render2d::DrawColor;
using gus::platform::render2d::IRenderer;
using gus::platform::render2d::kInvalidTexture;
using gus::platform::render2d::TextureId;
using gus::platform::render2d::UvRect;

namespace {

// IRenderer falso: registra cada chamada (cor + retangulo de mundo) pra inspecao,
// sem tocar GPU.
class FakeRenderer : public IRenderer {
public:
    struct Cmd {
        Rect rect;
        DrawColor color;
        bool filled;
    };
    struct SpriteCmd {
        Rect rect;
        TextureId texture;
        UvRect uv;
    };

    void begin_frame(const Rect& camera_world, int pixel_w, int pixel_h) override {
        ++begins;
        last_camera = camera_world;
        last_px = pixel_w;
        last_py = pixel_h;
        cmds.clear();
        sprites.clear();
    }
    void draw_filled_rect(const Rect& world_rect, const DrawColor& c) override {
        cmds.push_back({world_rect, c, true});
    }
    void draw_rect_outline(const Rect& world_rect, const DrawColor& c,
                           float /*thickness_world*/) override {
        cmds.push_back({world_rect, c, false});
    }
    TextureId load_texture(const char* /*path*/) override { return ++next_texture; }
    void draw_textured_rect(const Rect& world_rect, TextureId texture,
                            const UvRect& uv, const DrawColor& /*tint*/) override {
        sprites.push_back({world_rect, texture, uv});
    }
    // Headless: sem decode de PNG, devolve sempre um bbox invalido (anchor legado).
    gus::platform::render2d::ContentBbox texture_content_bbox(
        TextureId /*texture*/) const override {
        return gus::platform::render2d::ContentBbox{};
    }
    void draw_text(const char*, float, float, float,
                   const gus::platform::render2d::DrawColor&, bool) override {}
    void end_frame() override { ++ends; }

    int begins = 0;
    int ends = 0;
    int last_px = 0;
    int last_py = 0;
    TextureId next_texture = kInvalidTexture;
    Rect last_camera;
    std::vector<Cmd> cmds;
    std::vector<SpriteCmd> sprites;

    int filled_count() const {
        int n = 0;
        for (const auto& c : cmds) {
            if (c.filled) ++n;
        }
        return n;
    }
    const Cmd* player_cmd() const {
        for (auto it = cmds.rbegin(); it != cmds.rend(); ++it) {
            if (!it->filled) return &*it;
        }
        return nullptr;
    }
};

TileGrid make_map() {
    return TileGrid::from_rows(
        {
            "#####",
            "#...#",
            "#.#.#",
            "#...#",
            "#####",
        },
        16.0f);
}

PlayerSpriteSet make_fake_sprites() {
    PlayerSpriteSet s;
    TextureId next = 1;
    for (int d = 0; d < gus::app::screens::kDirectionCount; ++d) {
        s.idle[d] = next++;
        s.idle_frames[d][0] = s.idle[d];  // idle congelado (1 quadro) = compat Caua
        s.idle_count[d] = 1;
        for (int f = 0; f < kWalkFrameCount; ++f) {
            s.walk[d][f] = next++;
        }
        s.walk_count[d] = kWalkFrameCount;
    }
    return s;
}

// Set "estilo Gus": 7 quadros de walk + 5 quadros de breathing idle por direcao,
// todos com handles distintos (pra assertar QUAL textura saiu). NAO toca disco.
PlayerSpriteSet make_gus_like_sprites(int walk_n = 7, int idle_n = 5) {
    PlayerSpriteSet s;
    TextureId next = 1;
    for (int d = 0; d < gus::app::screens::kDirectionCount; ++d) {
        s.idle_count[d] = idle_n;
        for (int f = 0; f < idle_n; ++f) {
            s.idle_frames[d][f] = next++;
        }
        s.idle[d] = s.idle_frames[d][0];  // representativo = quadro 0
        s.walk_count[d] = walk_n;
        for (int f = 0; f < walk_n; ++f) {
            s.walk[d][f] = next++;
        }
    }
    return s;
}

constexpr double kEps = 1e-3;
}  // namespace

TEST_CASE("overworld: anda para a direita em campo livre", "[overworld]") {
    TileGrid g = make_map();
    OverworldSim sim(g, Aabb{16.0f, 16.0f, 8.0f, 8.0f}, 4.0f);
    float x0 = sim.player().x;
    sim.step_fixed(1, 0, false, 1.0f / 60.0f);
    REQUIRE(sim.player().x > x0);
    REQUIRE_THAT(sim.player().y, WithinAbs(16.0, kEps));
}

TEST_CASE("overworld: parede bloqueia um eixo mas desliza no outro", "[overworld]") {
    TileGrid g = make_map();
    OverworldSim sim(g, Aabb{24.0f, 34.0f, 8.0f, 8.0f}, 6.0f);
    sim.step_fixed(1, 1, true, 1.0f / 60.0f);
    REQUIRE(sim.player().x <= 24.0f + kEps);
    REQUIRE(sim.player().y > 34.0f);
}

TEST_CASE("overworld: nao atravessa a borda do mapa", "[overworld]") {
    TileGrid g = make_map();
    OverworldSim sim(g, Aabb{16.0f, 16.0f, 8.0f, 8.0f}, 8.0f);
    for (int i = 0; i < 10; ++i) {
        sim.step_fixed(-1, 0, false, 1.0f / 60.0f);
    }
    REQUIRE(sim.player().x >= 16.0f - kEps);
}

// Tuning que neutraliza o ZOOM (1 px == 1 unidade de mundo) para os testes de CLAMP
// e CULLING que raciocinam em unidades de mundo. Com tile_size 16 do make_map(),
// camera_zoom_px_per_tile = 16 => px-por-unidade = 16/16 = 1.0. Assim camera_view e
// render recebem pixels que valem 1:1 em mundo (semantica testada antes do zoom).
OverworldTuning tuning_no_zoom_for_tile16() {
    OverworldTuning t;
    t.camera_zoom_px_per_tile = 16.0f;  // ppu = 16 / tile_size(16) = 1.0
    return t;
}

TEST_CASE("overworld: camera segue o jogador presa ao mapa", "[overworld]") {
    TileGrid g = make_map();
    OverworldSim sim(g, Aabb{36.0f, 36.0f, 8.0f, 8.0f}, tuning_no_zoom_for_tile16());
    CameraView v = sim.camera_view(40.0f, 40.0f);  // zoom 1:1 => 40 px == 40 unidades
    REQUIRE_THAT(v.center.x, WithinAbs(40.0, kEps));
    REQUIRE_THAT(v.center.y, WithinAbs(40.0, kEps));
    REQUIRE_THAT(v.rect.x, WithinAbs(20.0, kEps));
}

TEST_CASE("overworld: camera clampa na borda", "[overworld]") {
    TileGrid g = make_map();
    OverworldSim sim(g, Aabb{16.0f, 16.0f, 8.0f, 8.0f}, tuning_no_zoom_for_tile16());
    CameraView v = sim.camera_view(40.0f, 40.0f);
    REQUIRE(v.rect.x >= -kEps);
    REQUIRE(v.rect.y >= -kEps);
}

// ZOOM REAL (M4-BUG.CAMERA): com zoom > 1:1 a visao mostra MENOS unidades de mundo do
// que os pixels da janela (AMPLIA). camera_zoom_px_per_tile 48 no tile_size 16 da
// px-por-unidade = 3.0; logo camera_view(120 px) = 40 unidades de mundo de visao (e
// NAO 120). Era exatamente isto que faltava: a visao deixa de "caber o mapa inteiro".
TEST_CASE("overworld: zoom amplia (pixels da janela viram menos mundo)", "[overworld][zoom]") {
    TileGrid g = make_map();  // tile_size 16, mapa 80x80 unidades
    OverworldTuning t;
    t.camera_zoom_px_per_tile = 48.0f;  // ppu = 48/16 = 3.0
    OverworldSim sim(g, Aabb{36.0f, 36.0f, 8.0f, 8.0f}, t);
    REQUIRE_THAT(sim.px_per_world_unit(), WithinAbs(3.0, kEps));
    // 120 px de janela / 3.0 = 40 unidades de mundo visiveis (centradas no Gus 40,40).
    CameraView v = sim.camera_view(120.0f, 120.0f);
    REQUIRE_THAT(v.rect.w, WithinAbs(40.0, kEps));
    REQUIRE_THAT(v.rect.h, WithinAbs(40.0, kEps));
    REQUIRE_THAT(v.center.x, WithinAbs(40.0, kEps));  // centra no Gus, dentro do mapa
    REQUIRE_THAT(v.rect.x, WithinAbs(20.0, kEps));     // 40 - 40/2
}

TEST_CASE("overworld: render emite paredes e jogador", "[overworld]") {
    TileGrid g = make_map();
    OverworldSim sim(g, Aabb{36.0f, 36.0f, 8.0f, 8.0f}, tuning_no_zoom_for_tile16());
    FakeRenderer r;
    sim.render(r, 80.0f, 80.0f, 0.0f);  // zoom 1:1 => visao 80x80 = mapa inteiro
    REQUIRE(r.begins == 1);
    REQUIRE(r.ends == 1);
    REQUIRE(r.filled_count() == 17);  // 16 borda + 1 bloco interno
    REQUIRE(r.player_cmd() != nullptr);
}

// DUNGEON-SCALING (PI8, fix do letterbox ao maximizar): sem os 2 parametros novos
// (screen_px_w/h), begin_frame recebe o MESMO tamanho passado como viewport - o
// comportamento LEGADO, byte-identico ao caso acima (sentinela -1.0f).
TEST_CASE("overworld: render sem screen_px usa viewport_px pro begin_frame (legado)",
         "[overworld][dungeon-scaling]") {
    TileGrid g = make_map();
    OverworldSim sim(g, Aabb{36.0f, 36.0f, 8.0f, 8.0f}, tuning_no_zoom_for_tile16());
    FakeRenderer r;
    sim.render(r, 80.0f, 80.0f, 0.0f);
    REQUIRE(r.last_px == 80);
    REQUIRE(r.last_py == 80);
}

// DUNGEON-SCALING: com screen_px_w/h informados (a JANELA REAL, maximizada), o
// ENQUADRAMENTO da camera continua vindo de viewport_px_w/h (o zoom LOGICO, fixo -
// nao muda so porque a janela cresceu), mas begin_frame recebe o tamanho de TELA
// REAL - e o retangulo de camera (mundo) e ESTICADO pra ele, nunca sobra mundo alem
// do mapa (o "letterbox" reportado pelo lider: antes, screen==viewport pedia MAIS
// MUNDO ao maximizar, esbarrava no limite do mapa 80x80 e a area alem do mapa ficava
// sem tile - a cor de fundo aparecendo como margem preta).
TEST_CASE("overworld: render com screen_px maior estica o MESMO enquadramento (nao revela mais mundo)",
         "[overworld][dungeon-scaling]") {
    TileGrid g = make_map();  // mapa 80x80 unidades (tile_size 16, 5x5 celulas)
    OverworldSim sim(g, Aabb{36.0f, 36.0f, 8.0f, 8.0f}, tuning_no_zoom_for_tile16());

    FakeRenderer window_size;
    sim.render(window_size, 80.0f, 80.0f, 0.0f);  // viewport == screen (default janela)
    const Rect logical_camera = window_size.last_camera;

    FakeRenderer maximized;
    // JANELA MAXIMIZADA pra 3x o tamanho (240x240 px reais) - viewport LOGICO
    // (80x80, o mesmo de sempre) continua ditando o ZOOM/enquadramento.
    sim.render(maximized, 80.0f, 80.0f, 0.0f, /*screen_px_w=*/240.0f,
              /*screen_px_h=*/240.0f);

    // begin_frame recebeu o tamanho de TELA REAL (esticar), nao o viewport logico.
    REQUIRE(maximized.last_px == 240);
    REQUIRE(maximized.last_py == 240);
    // O retangulo de MUNDO da camera e IDENTICO ao da janela default - a camera NAO
    // revelou mais mundo so porque a tela cresceu (o bug antigo).
    REQUIRE_THAT(maximized.last_camera.x, WithinAbs(logical_camera.x, kEps));
    REQUIRE_THAT(maximized.last_camera.y, WithinAbs(logical_camera.y, kEps));
    REQUIRE_THAT(maximized.last_camera.w, WithinAbs(logical_camera.w, kEps));
    REQUIRE_THAT(maximized.last_camera.h, WithinAbs(logical_camera.h, kEps));
}

// MARCADOR DE INIMIGO FIXO (M7-COSTURA Inc 2): o placeholder do androide (a MESMA
// textura da tela de batalha) desenhado por cima do mapa, na MESMA escala/ancoragem do
// Gus (player_sprite_height_tiles) - o lider precisa VER o inimigo pra esbarrar nele de
// proposito. Ver overworld_sim.hpp/set_enemy_marker.
TEST_CASE("overworld: marcador de inimigo desenha na posicao/escala certas", "[overworld][enemy-marker]") {
    TileGrid g = make_map();
    OverworldSim sim(g, Aabb{16.0f, 16.0f, 8.0f, 8.0f}, tuning_no_zoom_for_tile16());
    const Aabb enemy{48.0f, 16.0f, 8.0f, 8.0f};
    sim.set_enemy_marker(enemy, /*tex=*/7);
    REQUIRE(sim.has_enemy_marker());

    FakeRenderer r;
    sim.render(r, 80.0f, 80.0f, 0.0f);
    // So o marcador (o jogador, sem set_player_sprites, cai no contorno - draw_rect_outline,
    // nao draw_textured_rect).
    REQUIRE(r.sprites.size() == 1);
    REQUIRE(r.sprites[0].texture == 7);
    // Tamanho = MESMA escala do sprite do Gus (player_sprite_height_tiles * tile_size).
    const float expected_h = OverworldTuning{}.player_sprite_height_tiles * 16.0f;
    REQUIRE_THAT(r.sprites[0].rect.w, WithinAbs(expected_h, kEps));
    REQUIRE_THAT(r.sprites[0].rect.h, WithinAbs(expected_h, kEps));
    // Centrado em X sobre a AABB do inimigo.
    REQUIRE_THAT(r.sprites[0].rect.x + r.sprites[0].rect.w * 0.5f,
                WithinAbs(enemy.x + enemy.w * 0.5f, kEps));
    // Base do quad == base da AABB (bottom_fraction 0, sem foot-inset: e um busto/icone).
    REQUIRE_THAT(r.sprites[0].rect.y + r.sprites[0].rect.h,
                WithinAbs(enemy.y + enemy.h, kEps));
}

TEST_CASE("overworld: sem marcador (default), textura invalida ou apos clear -> nada desenhado",
         "[overworld][enemy-marker]") {
    TileGrid g = make_map();
    OverworldSim sim(g, Aabb{16.0f, 16.0f, 8.0f, 8.0f}, tuning_no_zoom_for_tile16());
    REQUIRE_FALSE(sim.has_enemy_marker());  // default: nenhum marcador

    sim.set_enemy_marker(Aabb{48.0f, 16.0f, 8.0f, 8.0f}, kInvalidTexture);
    REQUIRE_FALSE(sim.has_enemy_marker());  // textura invalida nao conta como "ativo"
    FakeRenderer r1;
    sim.render(r1, 80.0f, 80.0f, 0.0f);
    REQUIRE(r1.sprites.empty());

    sim.set_enemy_marker(Aabb{48.0f, 16.0f, 8.0f, 8.0f}, /*tex=*/9);
    REQUIRE(sim.has_enemy_marker());
    sim.clear_enemy_marker();
    REQUIRE_FALSE(sim.has_enemy_marker());  // Victory (item 4 do escopo): some do mapa
    FakeRenderer r2;
    sim.render(r2, 80.0f, 80.0f, 0.0f);
    REQUIRE(r2.sprites.empty());
}

// MARCADOR DO NPC BERTOLDO (M7-DIALOGO, NPC-MVP - integracao do sprite): o sprite
// ESTATICO do Seu Bertoldo Caim (south.png) desenhado por cima do mapa, na MESMA
// ancoragem "busto simples" do marcador de inimigo (quad quadrado, centrado em X,
// base = base da AABB), so que com ESCALA PROPRIA (slot PROPRIO, distinto do
// enemy_marker_* - nao compartilha textura/posicao/escala). Ver overworld_sim.hpp/
// set_npc_bertoldo_marker.
TEST_CASE("overworld: marcador do NPC Bertoldo desenha na posicao/escala certas",
         "[overworld][npc-bertoldo-marker]") {
    TileGrid g = make_map();
    OverworldSim sim(g, Aabb{16.0f, 16.0f, 8.0f, 8.0f}, tuning_no_zoom_for_tile16());
    const Aabb bertoldo{48.0f, 16.0f, 8.0f, 8.0f};
    sim.set_npc_bertoldo_marker(bertoldo, /*tex=*/7);
    REQUIRE(sim.has_npc_bertoldo_marker());

    FakeRenderer r;
    sim.render(r, 80.0f, 80.0f, 0.0f);
    // So o marcador (o jogador, sem set_player_sprites, cai no contorno).
    REQUIRE(r.sprites.size() == 1);
    REQUIRE(r.sprites[0].texture == 7);
    // Tamanho = escala PROPRIA do Bertoldo (npc_bertoldo_sprite_height_tiles *
    // tile_size) - FIX BUG do lider "Bertoldo menor que o Gus" (M7-DIALOGO):
    // NAO reusa mais player_sprite_height_tiles (ver overworld_tuning.hpp pro
    // porque: o retrato do Bertoldo tem margem transparente maior que o do Gus).
    const float expected_h =
        OverworldTuning{}.npc_bertoldo_sprite_height_tiles * 16.0f;
    REQUIRE_THAT(r.sprites[0].rect.w, WithinAbs(expected_h, kEps));
    REQUIRE_THAT(r.sprites[0].rect.h, WithinAbs(expected_h, kEps));
    // A escala do Bertoldo e DIFERENTE (maior) da escala do jogador - prova que os
    // dois NAO estao mais acoplados no mesmo numero (regressao do bug original).
    REQUIRE(OverworldTuning{}.npc_bertoldo_sprite_height_tiles >
            OverworldTuning{}.player_sprite_height_tiles);
    // Centrado em X sobre a AABB do Bertoldo.
    REQUIRE_THAT(r.sprites[0].rect.x + r.sprites[0].rect.w * 0.5f,
                WithinAbs(bertoldo.x + bertoldo.w * 0.5f, kEps));
    // Base do quad == base da AABB (bottom_fraction 0, sem foot-inset: NPC parado).
    REQUIRE_THAT(r.sprites[0].rect.y + r.sprites[0].rect.h,
                WithinAbs(bertoldo.y + bertoldo.h, kEps));
}

TEST_CASE("overworld: sem marcador do Bertoldo (default), textura invalida ou apos "
         "clear -> nada desenhado",
         "[overworld][npc-bertoldo-marker]") {
    TileGrid g = make_map();
    OverworldSim sim(g, Aabb{16.0f, 16.0f, 8.0f, 8.0f}, tuning_no_zoom_for_tile16());
    REQUIRE_FALSE(sim.has_npc_bertoldo_marker());  // default: nenhum marcador

    sim.set_npc_bertoldo_marker(Aabb{48.0f, 16.0f, 8.0f, 8.0f}, kInvalidTexture);
    REQUIRE_FALSE(sim.has_npc_bertoldo_marker());  // textura invalida nao conta
    FakeRenderer r1;
    sim.render(r1, 80.0f, 80.0f, 0.0f);
    REQUIRE(r1.sprites.empty());

    sim.set_npc_bertoldo_marker(Aabb{48.0f, 16.0f, 8.0f, 8.0f}, /*tex=*/9);
    REQUIRE(sim.has_npc_bertoldo_marker());
    sim.clear_npc_bertoldo_marker();
    REQUIRE_FALSE(sim.has_npc_bertoldo_marker());
    FakeRenderer r2;
    sim.render(r2, 80.0f, 80.0f, 0.0f);
    REQUIRE(r2.sprites.empty());
}

// Os DOIS marcadores (inimigo + Bertoldo) sao slots INDEPENDENTES: ambos ativos ao
// mesmo tempo desenham 2 sprites distintos, sem um pisar no estado do outro.
TEST_CASE("overworld: marcador de inimigo e do Bertoldo coexistem sem se pisarem",
         "[overworld][npc-bertoldo-marker][enemy-marker]") {
    TileGrid g = make_map();
    OverworldSim sim(g, Aabb{16.0f, 16.0f, 8.0f, 8.0f}, tuning_no_zoom_for_tile16());
    sim.set_enemy_marker(Aabb{40.0f, 16.0f, 8.0f, 8.0f}, /*tex=*/3);
    sim.set_npc_bertoldo_marker(Aabb{48.0f, 40.0f, 8.0f, 8.0f}, /*tex=*/7);
    REQUIRE(sim.has_enemy_marker());
    REQUIRE(sim.has_npc_bertoldo_marker());

    FakeRenderer r;
    sim.render(r, 80.0f, 80.0f, 0.0f);
    REQUIRE(r.sprites.size() == 2);
    REQUIRE(r.sprites[0].texture == 3);  // inimigo desenhado primeiro
    REQUIRE(r.sprites[1].texture == 7);  // Bertoldo em seguida

    // clear_enemy_marker nao afeta o marcador do Bertoldo.
    sim.clear_enemy_marker();
    REQUIRE_FALSE(sim.has_enemy_marker());
    REQUIRE(sim.has_npc_bertoldo_marker());
    FakeRenderer r2;
    sim.render(r2, 80.0f, 80.0f, 0.0f);
    REQUIRE(r2.sprites.size() == 1);
    REQUIRE(r2.sprites[0].texture == 7);
}

// COLISAO SOLIDA (M7-COSTURA/M7-DIALOGO, playtest ao vivo do lider: "o Gus anda POR
// CIMA/ATRAVES do Bertoldo, o NPC fica escondido debaixo do sprite ao aproximar pelo
// norte"). set_enemy_marker/set_npc_bertoldo_marker agora TAMBEM derivam uma caixa de
// bloqueio FISICO (~1 tile, ver overworld_tuning.hpp::npc_solid_box_tiles) a partir do
// footprint recebido; step_fixed passa essa caixa como ObstacleSpan extra pro
// resolve_move_with_corner_assist - o jogador NUNCA mais ocupa a mesma posicao do
// marcador (mas contorna livre pelos tiles adjacentes, SEM travar o corredor inteiro).
TEST_CASE("overworld: colisao solida bloqueia o jogador na posicao do marcador do inimigo",
          "[overworld][enemy-marker][solid-collision]") {
    // Grade ABERTA (10x10, tile 16) SEM nenhuma parede da grade nas proximidades - a
    // UNICA coisa que pode bloquear o jogador aqui e o obstaculo pontual do marcador.
    TileGrid g(10, 10, 16.0f);
    OverworldTuning t;
    t.corner.enabled = false;  // desliga corner-assist p/ o resultado ser 100% analitico
    // Jogador comeca a esquerda, na MESMA faixa Y do obstaculo (overlap total em Y).
    OverworldSim sim(g, Aabb{20.0f, 58.0f, 8.0f, 8.0f}, t);

    // Footprint VISUAL do inimigo (o quad grande desenhado) - arbitrario, so serve de
    // entrada pra derivar a caixa solida (~1 tile = 16 unidades neste tile_size).
    sim.set_enemy_marker(Aabb{100.0f, 50.0f, 20.0f, 20.0f}, /*tex=*/7);
    // Caixa solida esperada (MESMA formula de solid_obstacle_from_footprint): centro
    // em X do footprint (110), base = base do footprint (70), tamanho 16.
    //   solid.x = 110 - 8 = 102; solid.y = 70 - 16 = 54 (x:[102,118) y:[54,70)).
    constexpr float kSolidX = 102.0f;
    constexpr float kSolidW = 16.0f;

    // Anda pra DIREITA repetidamente com passo PEQUENO por frame (evita tunneling,
    // documentado como limite conhecido do resolve_move para deltas grandes demais).
    for (int i = 0; i < 200; ++i) {
        sim.step_fixed(1, 0, false, 1.0f / 60.0f);
    }

    // O jogador NUNCA deve ultrapassar a borda esquerda do obstaculo (encosta e para,
    // exatamente como uma parede) - a borda direita da hitbox <= kSolidX.
    REQUIRE(sim.player().x + sim.player().w <= kSolidX + kEps);
    // Prova que REALMENTE tentou entrar no obstaculo (nao parou longe por acidente):
    // chegou pertinho da borda de encosto esperada.
    REQUIRE_THAT(sim.player().x, WithinAbs(kSolidX - sim.player().w, 0.05));
    (void)kSolidW;
}

TEST_CASE("overworld: jogador contorna o marcador SEM travar o corredor inteiro",
          "[overworld][enemy-marker][solid-collision]") {
    // MESMO obstaculo do teste acima, mas o jogador anda numa faixa Y que NAO se
    // sobrepoe ao obstaculo (contornando por baixo) - deve atravessar livremente por
    // cima de onde o obstaculo fica em X, sem NENHUM bloqueio (prova que a colisao e
    // PONTUAL, nao uma parede que cobre o corredor inteiro).
    TileGrid g(10, 10, 16.0f);
    OverworldTuning t;
    t.corner.enabled = false;
    // Jogador comeca BEM abaixo da faixa Y do obstaculo (solido em y:[54,70)).
    OverworldSim sim(g, Aabb{20.0f, 120.0f, 8.0f, 8.0f}, t);
    sim.set_enemy_marker(Aabb{100.0f, 50.0f, 20.0f, 20.0f}, /*tex=*/7);

    const float x0 = sim.player().x;
    for (int i = 0; i < 200; ++i) {
        sim.step_fixed(1, 0, false, 1.0f / 60.0f);
    }
    // Andou livre, bem alem de onde o obstaculo fica em X (nao foi barrado).
    REQUIRE(sim.player().x > x0 + 100.0f);
}

TEST_CASE("overworld: colisao solida do Bertoldo (NPC) tambem bloqueia o jogador",
          "[overworld][npc-bertoldo-marker][solid-collision]") {
    // MESMA garantia do inimigo acima, agora pro slot PROPRIO do Bertoldo (obstaculos
    // independentes - ver step_fixed, array de ate 2).
    TileGrid g(10, 10, 16.0f);
    OverworldTuning t;
    t.corner.enabled = false;
    OverworldSim sim(g, Aabb{20.0f, 58.0f, 8.0f, 8.0f}, t);
    sim.set_npc_bertoldo_marker(Aabb{100.0f, 50.0f, 20.0f, 20.0f}, /*tex=*/9);
    constexpr float kSolidX = 102.0f;

    for (int i = 0; i < 200; ++i) {
        sim.step_fixed(1, 0, false, 1.0f / 60.0f);
    }
    REQUIRE(sim.player().x + sim.player().w <= kSolidX + kEps);
    REQUIRE_THAT(sim.player().x, WithinAbs(kSolidX - sim.player().w, 0.05));
}

TEST_CASE("overworld: inimigo derrotado (clear_enemy_marker) libera o bloqueio fisico",
          "[overworld][enemy-marker][solid-collision]") {
    // Victory (item 4 do escopo M7): o inimigo some do mapa E para de bloquear -
    // clear_enemy_marker reseta enemy_solid_aabb_ (ver header).
    TileGrid g(10, 10, 16.0f);
    OverworldTuning t;
    t.corner.enabled = false;
    OverworldSim sim(g, Aabb{20.0f, 58.0f, 8.0f, 8.0f}, t);
    sim.set_enemy_marker(Aabb{100.0f, 50.0f, 20.0f, 20.0f}, /*tex=*/7);
    sim.clear_enemy_marker();

    const float x0 = sim.player().x;
    for (int i = 0; i < 200; ++i) {
        sim.step_fixed(1, 0, false, 1.0f / 60.0f);
    }
    // Sem o marcador, o jogador atravessa livremente por onde o obstaculo estaria.
    REQUIRE(sim.player().x > x0 + 100.0f);
}

// Y-SORT no render REAL (M7-COSTURA, ver depth_sort_test.cpp para a prova da funcao
// pura de ordenacao). Aqui a integracao: OverworldSim::render() consome sort_by_depth
// pra decidir a SEQUENCIA de desenho - jogador, inimigo e NPC entram na MESMA lista
// ordenavel por profundidade (base/pe = aabb.y+aabb.h). set_player_sprites faz o
// jogador desenhar via draw_textured_rect (MESMO vetor `sprites` dos marcadores),
// entao a ORDEM em r.sprites reflete diretamente a sequencia de desenho real.
TEST_CASE("overworld: Y-sort real - jogador ACIMA dos marcadores desenha primeiro (atras)",
          "[overworld][depth-sort]") {
    TileGrid g(10, 10, 16.0f);
    OverworldSim sim(g, Aabb{20.0f, 10.0f, 8.0f, 8.0f}, OverworldTuning{});
    PlayerSpriteSet sprites = make_fake_sprites();
    sim.set_player_sprites(sprites);
    sim.set_enemy_marker(Aabb{40.0f, 60.0f, 8.0f, 8.0f}, /*tex=*/100);       // base=68
    sim.set_npc_bertoldo_marker(Aabb{60.0f, 70.0f, 8.0f, 8.0f}, /*tex=*/101);  // base=78

    FakeRenderer r;
    sim.render(r, 800.0f, 800.0f, 0.0f);  // viewport GRANDE: garante que nada e cullado
    REQUIRE(r.sprites.size() == 3);

    const TextureId player_tex = sprites.idle[static_cast<int>(Direction::South)];
    // Jogador (base = 10+8 = 18, o MENOR dos 3) desenha PRIMEIRO (indice 0).
    REQUIRE(r.sprites[0].texture == player_tex);
    REQUIRE(r.sprites[1].texture == 100);  // inimigo (base 68) em seguida
    REQUIRE(r.sprites[2].texture == 101);  // Bertoldo (base 78) por ultimo (na frente)
}

TEST_CASE("overworld: Y-sort real - jogador ABAIXO dos marcadores desenha por ultimo (frente)",
          "[overworld][depth-sort]") {
    TileGrid g(10, 10, 16.0f);
    OverworldSim sim(g, Aabb{20.0f, 150.0f, 8.0f, 8.0f}, OverworldTuning{});
    PlayerSpriteSet sprites = make_fake_sprites();
    sim.set_player_sprites(sprites);
    sim.set_enemy_marker(Aabb{40.0f, 60.0f, 8.0f, 8.0f}, /*tex=*/100);       // base=68
    sim.set_npc_bertoldo_marker(Aabb{60.0f, 70.0f, 8.0f, 8.0f}, /*tex=*/101);  // base=78

    FakeRenderer r;
    sim.render(r, 800.0f, 800.0f, 0.0f);
    REQUIRE(r.sprites.size() == 3);

    const TextureId player_tex = sprites.idle[static_cast<int>(Direction::South)];
    REQUIRE(r.sprites[0].texture == 100);  // inimigo (base 68) primeiro
    REQUIRE(r.sprites[1].texture == 101);  // Bertoldo (base 78) em seguida
    // Jogador (base = 150+8 = 158, o MAIOR dos 3) desenha por ULTIMO (fica na frente)
    // - o INVERSO da ordem fixa legada (jogador SEMPRE por cima, independente do Y).
    REQUIRE(r.sprites[2].texture == player_tex);
}

TEST_CASE("overworld: render interpola posicao do jogador", "[overworld]") {
    TileGrid g = make_map();
    OverworldSim sim(g, Aabb{16.0f, 16.0f, 8.0f, 8.0f}, 60.0f);
    float xprev = sim.player().x;
    sim.step_fixed(1, 0, false, 1.0f / 60.0f);
    float xcurr = sim.player().x;
    REQUIRE(xcurr > xprev);

    FakeRenderer r0;
    sim.render(r0, 80.0f, 80.0f, 0.0f);
    REQUIRE_THAT(r0.player_cmd()->rect.x, WithinAbs(xprev, kEps));

    FakeRenderer r1;
    sim.render(r1, 80.0f, 80.0f, 1.0f);
    REQUIRE_THAT(r1.player_cmd()->rect.x, WithinAbs(xcurr, kEps));

    FakeRenderer rh;
    sim.render(rh, 80.0f, 80.0f, 0.5f);
    float mid = (xprev + xcurr) * 0.5f;
    REQUIRE_THAT(rh.player_cmd()->rect.x, WithinAbs(mid, kEps));
}

TEST_CASE("overworld: passo parado nao move", "[overworld]") {
    TileGrid g = make_map();
    OverworldSim sim(g, Aabb{36.0f, 36.0f, 8.0f, 8.0f}, 4.0f);
    float x0 = sim.player().x;
    float y0 = sim.player().y;
    sim.step_fixed(0, 0, false, 1.0f / 60.0f);
    REQUIRE_THAT(sim.player().x, WithinAbs(x0, kEps));
    REQUIRE_THAT(sim.player().y, WithinAbs(y0, kEps));
}

TEST_CASE("overworld: run move mais que andar", "[overworld]") {
    TileGrid g = make_map();
    OverworldSim walk(g, Aabb{16.0f, 16.0f, 8.0f, 8.0f}, 4.0f);
    OverworldSim run(g, Aabb{16.0f, 16.0f, 8.0f, 8.0f}, 4.0f);
    walk.step_fixed(1, 0, false, 1.0f / 60.0f);
    run.step_fixed(1, 0, true, 1.0f / 60.0f);
    REQUIRE(run.player().x > walk.player().x);
}

TEST_CASE("overworld: ctor com tuning define velocidade", "[overworld]") {
    TileGrid g = make_map();
    OverworldTuning t;
    t.walk_speed_tiles_per_sec = 4.0f;
    OverworldSim sim(g, Aabb{16.0f, 16.0f, 8.0f, 8.0f}, t);
    float x0 = sim.player().x;
    sim.step_fixed(1, 0, false, 1.0f / 60.0f);
    REQUIRE(sim.player().x > x0);
}

TEST_CASE("overworld: corner-assist ligado contorna a quina", "[overworld]") {
    TileGrid g = TileGrid::from_rows({".#.", "..."}, 16.0f);
    OverworldTuning on;  // corner.enabled = true (default)
    on.walk_speed_tiles_per_sec = 8.0f;
    OverworldSim sim_on(g, Aabb{8.0f, 12.0f, 8.0f, 8.0f}, on);
    sim_on.step_fixed(1, 0, false, 1.0f / 60.0f);
    REQUIRE(sim_on.player().y > 12.0f);  // empurrado para a abertura (baixo)

    OverworldTuning off;
    off.walk_speed_tiles_per_sec = 8.0f;
    off.corner.enabled = false;
    OverworldSim sim_off(g, Aabb{8.0f, 12.0f, 8.0f, 8.0f}, off);
    sim_off.step_fixed(1, 0, false, 1.0f / 60.0f);
    REQUIRE_THAT(sim_off.player().y, WithinAbs(12.0, kEps));  // sem assist
}

TEST_CASE("overworld: normalize_diagonal iguala velocidade a cardinal",
          "[overworld]") {
    TileGrid g(9, 9, 16.0f);
    OverworldTuning norm;
    norm.normalize_diagonal = true;
    norm.walk_speed_tiles_per_sec = 4.0f;
    OverworldSim card(g, Aabb{64.0f, 64.0f, 8.0f, 8.0f}, norm);
    OverworldSim diag(g, Aabb{64.0f, 64.0f, 8.0f, 8.0f}, norm);
    card.step_fixed(1, 0, false, 1.0f / 60.0f);
    diag.step_fixed(1, 1, false, 1.0f / 60.0f);
    const float card_dx = card.player().x - 64.0f;
    const float diag_dx = diag.player().x - 64.0f;
    const float diag_dy = diag.player().y - 64.0f;
    const float diag_len = std::sqrt(diag_dx * diag_dx + diag_dy * diag_dy);
    REQUIRE_THAT(diag_len, WithinAbs(card_dx, 1e-3));
}

TEST_CASE("overworld: diagonal crua por padrao anda mais que cardinal",
          "[overworld]") {
    TileGrid g(9, 9, 16.0f);
    OverworldTuning raw;  // normalize_diagonal = false (default)
    raw.walk_speed_tiles_per_sec = 4.0f;
    OverworldSim card(g, Aabb{64.0f, 64.0f, 8.0f, 8.0f}, raw);
    OverworldSim diag(g, Aabb{64.0f, 64.0f, 8.0f, 8.0f}, raw);
    card.step_fixed(1, 0, false, 1.0f / 60.0f);
    diag.step_fixed(1, 1, false, 1.0f / 60.0f);
    const float card_dx = card.player().x - 64.0f;
    const float diag_dx = diag.player().x - 64.0f;
    REQUIRE_THAT(diag_dx, WithinAbs(card_dx, kEps));
}

// --- integracao do SPRITE no render ------------------------------------------

TEST_CASE("overworld: sem sprites desenha contorno fallback", "[overworld]") {
    TileGrid g = make_map();
    OverworldSim sim(g, Aabb{36.0f, 36.0f, 8.0f, 8.0f}, 4.0f);
    FakeRenderer r;
    sim.render(r, 80.0f, 80.0f, 0.0f);
    REQUIRE(r.player_cmd() != nullptr);
    REQUIRE(r.sprites.empty());
}

TEST_CASE("overworld: com sprites desenha textura e nao contorno", "[overworld]") {
    TileGrid g = make_map();
    OverworldSim sim(g, Aabb{36.0f, 36.0f, 8.0f, 8.0f}, 4.0f);
    sim.set_player_sprites(make_fake_sprites());
    FakeRenderer r;
    sim.render(r, 80.0f, 80.0f, 0.0f);
    REQUIRE(static_cast<int>(r.sprites.size()) == 1);
    REQUIRE(r.player_cmd() == nullptr);
}

TEST_CASE("overworld: parado usa o sprite neutro da direcao", "[overworld]") {
    TileGrid g = make_map();
    OverworldSim sim(g, Aabb{36.0f, 36.0f, 8.0f, 8.0f}, 4.0f);
    PlayerSpriteSet s = make_fake_sprites();
    sim.set_player_sprites(s);
    sim.step_fixed(0, 0, false, 1.0f / 60.0f);  // parado: direcao default Sul
    FakeRenderer r;
    sim.render(r, 80.0f, 80.0f, 1.0f);
    REQUIRE(static_cast<int>(r.sprites.size()) == 1);
    REQUIRE(r.sprites[0].texture == s.idle[static_cast<int>(Direction::South)]);
}

TEST_CASE("overworld: direcao muda o sprite escolhido", "[overworld]") {
    TileGrid g = make_map();
    OverworldSim sim(g, Aabb{36.0f, 36.0f, 8.0f, 8.0f}, 4.0f);
    PlayerSpriteSet s = make_fake_sprites();
    sim.set_player_sprites(s);
    sim.step_fixed(-1, 0, false, 1.0f / 60.0f);
    REQUIRE(sim.facing() == Direction::West);
    FakeRenderer r;
    sim.render(r, 80.0f, 80.0f, 1.0f);
    const TextureId t = r.sprites[0].texture;
    bool is_west_walk = false;
    for (int f = 0; f < kWalkFrameCount; ++f) {
        if (t == s.walk[static_cast<int>(Direction::West)][f]) is_west_walk = true;
    }
    REQUIRE(is_west_walk);
}

TEST_CASE("overworld: sprite ancorado nos pes e maior que a aabb", "[overworld]") {
    TileGrid g = make_map();  // tile 16
    OverworldSim sim(g, Aabb{36.0f, 36.0f, 8.0f, 8.0f}, 4.0f);
    sim.set_player_sprites(make_fake_sprites());
    FakeRenderer r;
    sim.render(r, 80.0f, 80.0f, 1.0f);
    const Rect sr = r.sprites[0].rect;
    const Rect aabb{36.0f, 36.0f, 8.0f, 8.0f};
    REQUIRE(sr.h > aabb.h);
    REQUIRE_THAT((sr.y + sr.h) - (aabb.y + aabb.h), WithinAbs(0.0, kEps));
    REQUIRE(sr.y < aabb.y);
    const float sr_cx = sr.x + sr.w * 0.5f;
    const float aabb_cx = aabb.x + aabb.w * 0.5f;
    REQUIRE_THAT(sr_cx - aabb_cx, WithinAbs(0.0, kEps));
}

TEST_CASE("overworld: ancoragem AUTOMATICA desce o desenho pela margem medida",
          "[overworld]") {
    // M1-BUG.SUL: a margem inferior transparente MEDIDA do sprite (FootInset, vinda
    // do alpha-bbox no loader) desce o CANVAS pra o PE REAL encostar na base da AABB.
    // Aqui injetamos a fracao direto (sem renderer/PNG): o render deve baixar a base
    // do canvas por fracao*sprite_h, e o pe (canvas - margem) cair sobre a base AABB.
    TileGrid g = make_map();  // tile 16
    const Aabb start{36.0f, 36.0f, 8.0f, 8.0f};
    OverworldTuning base;  // sprite_foot_offset_tiles default 0 (so o automatico)

    // Sem margem (referencia): base do canvas == base da AABB.
    OverworldSim s0(g, start, base);
    s0.set_player_sprites(make_fake_sprites());
    FakeRenderer r0;
    s0.render(r0, 80.0f, 80.0f, 1.0f);
    const Rect ref = r0.sprites[0].rect;
    REQUIRE_THAT((ref.y + ref.h) - (start.y + start.h), WithinAbs(0.0, kEps));

    // Com margem medida (ex.: 0.16 do canvas no Sul). Parado => direcao Sul (default).
    PlayerSpriteSet s = make_fake_sprites();
    const float frac = 11.0f / 68.0f;  // caso medido do Caua south
    s.foot.bottom_fraction[static_cast<int>(Direction::South)] = frac;
    OverworldSim s1(g, start, base);
    s1.set_player_sprites(s);
    FakeRenderer r1;
    s1.render(r1, 80.0f, 80.0f, 1.0f);
    const Rect sr = r1.sprites[0].rect;
    const float foot_world = frac * sr.h;  // margem convertida pra mundo
    // O canvas desceu: o PE REAL (canvas_bottom - margem) cai EXATO na base da AABB.
    REQUIRE_THAT((sr.y + sr.h) - foot_world - (start.y + start.h),
                 WithinAbs(0.0, kEps));
    // E a base do canvas avancou pra DENTRO da parede (canvas_bottom > base AABB).
    REQUIRE((sr.y + sr.h) > (start.y + start.h));
    // Mesma altura do sprite (so a posicao vertical mudou).
    REQUIRE_THAT(sr.h, WithinAbs(ref.h, kEps));
}

TEST_CASE("overworld: ajuste manual SOMA por cima do automatico", "[overworld]") {
    // O automatico (FootInset medido) e o padrao; sprite_foot_offset_tiles e refino
    // OPCIONAL somado por cima - nao um mecanismo concorrente. Aqui: mesma margem
    // automatica, com e sem o ajuste manual; a diferenca = exatamente o ajuste.
    TileGrid g = make_map();  // tile 16
    const Aabb start{36.0f, 36.0f, 8.0f, 8.0f};
    PlayerSpriteSet s = make_fake_sprites();
    s.foot.bottom_fraction[static_cast<int>(Direction::South)] = 11.0f / 68.0f;

    OverworldTuning autom;  // so automatico (offset 0)
    OverworldSim sa(g, start, autom);
    sa.set_player_sprites(s);
    FakeRenderer ra;
    sa.render(ra, 80.0f, 80.0f, 1.0f);
    const float auto_bottom = ra.sprites[0].rect.y + ra.sprites[0].rect.h;

    OverworldTuning manual = autom;
    manual.sprite_foot_offset_tiles = 0.5f;  // +0.5 tile (8 u no tile 16) por cima
    OverworldSim sm(g, start, manual);
    sm.set_player_sprites(s);
    FakeRenderer rm;
    sm.render(rm, 80.0f, 80.0f, 1.0f);
    const float manual_bottom = rm.sprites[0].rect.y + rm.sprites[0].rect.h;

    // O manual desce exatamente 8 u ALEM do automatico (soma, nao substitui).
    REQUIRE_THAT(manual_bottom - auto_bottom, WithinAbs(8.0, kEps));
}

TEST_CASE("overworld: sprite_foot_offset afunda/levanta a base do sprite",
          "[overworld]") {
    // BUG-FIX bug 1 (lider 2026-06-22): a base do sprite cai sobre a base da AABB
    // por padrao (offset 0). O lider ajusta sprite_foot_offset_tiles pra alinhar os
    // PES VISUAIS do desenho a hitbox sem mexer em codigo. offset > 0 desce a base
    // (afunda os pes/encosta mais perto da parede de baixo); < 0 levanta.
    TileGrid g = make_map();  // tile 16
    const Aabb start{36.0f, 36.0f, 8.0f, 8.0f};

    OverworldTuning base;  // sprite_foot_offset_tiles default 0
    OverworldSim s0(g, start, base);
    s0.set_player_sprites(make_fake_sprites());
    FakeRenderer r0;
    s0.render(r0, 80.0f, 80.0f, 1.0f);
    const float base_bottom = r0.sprites[0].rect.y + r0.sprites[0].rect.h;
    // Offset 0: base do sprite == base da AABB (comportamento preservado).
    REQUIRE_THAT(base_bottom - (start.y + start.h), WithinAbs(0.0, kEps));

    OverworldTuning off = base;
    off.sprite_foot_offset_tiles = 0.5f;  // desce meia tile (8 u no tile 16)
    OverworldSim s1(g, start, off);
    s1.set_player_sprites(make_fake_sprites());
    FakeRenderer r1;
    s1.render(r1, 80.0f, 80.0f, 1.0f);
    const float off_bottom = r1.sprites[0].rect.y + r1.sprites[0].rect.h;
    // A base desceu exatamente 0.5 tile (8 u) em relacao ao default.
    REQUIRE_THAT(off_bottom - base_bottom, WithinAbs(8.0, kEps));
    // Largura/altura do sprite nao mudam (so a posicao vertical).
    REQUIRE_THAT(r1.sprites[0].rect.h, WithinAbs(r0.sprites[0].rect.h, kEps));
}

TEST_CASE("overworld: diagonal sustentada nao OSCILA a direcao (anti-flicker)",
          "[overworld]") {
    // BUG do flicker (M1): segurar uma diagonal (D+W) fazia o facing alternar a cada
    // tick (East->North->East...) porque a direcao era derivada do facing ANTERIOR.
    // Com a memoria de INPUT (dx_prev,dy_prev) no sim, segurar a MESMA diagonal por
    // N ticks tem que manter a MESMA direcao apos o primeiro tick que a aciona.
    TileGrid g(20, 20, 16.0f);
    OverworldTuning t;
    t.walk_speed_tiles_per_sec = 4.0f;
    t.diagonal_facing = gus::app::screens::DiagonalFacing::LastAxisWins;
    OverworldSim sim(g, Aabb{64.0f, 64.0f, 8.0f, 8.0f}, t);
    // 1) anda so pro lado um tick (estabelece dx_prev=1, dy_prev=0, facing=East).
    sim.step_fixed(1, 0, false, 1.0f / 60.0f);
    REQUIRE(sim.facing() == Direction::East);
    // 2) adiciona W: o eixo vertical e o recem-acionado -> vira Norte.
    sim.step_fixed(1, -1, false, 1.0f / 60.0f);
    const Direction held = sim.facing();
    REQUIRE(held == Direction::North);
    // 3) segura a diagonal por varios ticks: NAO pode mais oscilar.
    for (int i = 0; i < 20; ++i) {
        sim.step_fixed(1, -1, false, 1.0f / 60.0f);
        REQUIRE(sim.facing() == held);  // estavel, sem piscar
    }
}

TEST_CASE("overworld: cima-depois-lado vira para o lado e fica estavel", "[overworld]") {
    TileGrid g(20, 20, 16.0f);
    OverworldTuning t;
    t.walk_speed_tiles_per_sec = 4.0f;
    t.diagonal_facing = gus::app::screens::DiagonalFacing::LastAxisWins;
    OverworldSim sim(g, Aabb{64.0f, 64.0f, 8.0f, 8.0f}, t);
    sim.step_fixed(0, -1, false, 1.0f / 60.0f);  // so pra cima
    REQUIRE(sim.facing() == Direction::North);
    sim.step_fixed(1, -1, false, 1.0f / 60.0f);  // adiciona D (horizontal recem)
    REQUIRE(sim.facing() == Direction::East);
    for (int i = 0; i < 20; ++i) {
        sim.step_fixed(1, -1, false, 1.0f / 60.0f);
        REQUIRE(sim.facing() == Direction::East);  // estavel no Leste
    }
}

TEST_CASE("overworld: quadro de walk avanca com a distancia", "[overworld]") {
    TileGrid g(20, 20, 16.0f);
    OverworldSim sim(g, Aabb{64.0f, 64.0f, 8.0f, 8.0f}, 8.0f);
    sim.set_player_sprites(make_fake_sprites());
    for (int i = 0; i < 30; ++i) {
        sim.step_fixed(1, 0, false, 1.0f / 60.0f);
    }
    REQUIRE(sim.walk_cycle().is_moving());
    const int f = sim.walk_cycle().current_frame();
    REQUIRE(f >= 0);
    REQUIRE(f < kWalkFrameCount);
}

// REGRESSAO do bug "Gus desliza sem dar passos" (irmao do bug do zoom): a cadencia do
// walk era ABSOLUTA (~8 px), calibrada para o tile 16 do M1; no .gmap real (tile_size
// 2.0) o quadro quase nunca trocava. A cadencia agora e por FRACAO DE TILE, escalada
// pelo tile_size, entao a MESMA velocidade (em tiles/s) por o MESMO tempo tem que dar
// o MESMO numero de quadros de walk EM QUALQUER escala. Dois mapas, tile_size 2.0 e
// 16.0, andando os mesmos ticks na mesma velocidade: a contagem de trocas de quadro
// tem que bater (imune a escala).
TEST_CASE("overworld: cadencia do walk e invariante ao tile_size (anti-desliza)",
          "[overworld]") {
    auto count_frame_changes = [](float tile_size) {
        // Mapa grande o bastante pra nao bater na borda nos ticks andados (qualquer
        // escala). Velocidade IGUAL em tiles/s; a fracao de tile/quadro do walk e a
        // default do tuning. O Gus (7 quadros) anda pra Leste em campo livre.
        TileGrid g(200, 200, tile_size);
        OverworldTuning t;
        t.walk_speed_tiles_per_sec = 4.5f;
        OverworldSim sim(g, Aabb{50.0f * tile_size, 50.0f * tile_size, 0.5f * tile_size,
                                 0.5f * tile_size},
                         t);
        sim.set_player_sprites(make_gus_like_sprites(7, 5));
        int changes = 0;
        int last = sim.walk_cycle().current_frame();
        for (int i = 0; i < 120; ++i) {
            sim.step_fixed(1, 0, false, 1.0f / 60.0f);
            const int f = sim.walk_cycle().current_frame();
            if (f != last) ++changes;
            last = f;
        }
        return changes;
    };
    const int small_scale = count_frame_changes(2.0f);   // .gmap real (era o bug)
    const int large_scale = count_frame_changes(16.0f);  // cena de teste do M1
    // Mesma cadencia nas duas escalas: prova que a troca de quadro acompanha o tile_size.
    REQUIRE(small_scale == large_scale);
    // E houve passos de verdade em 2 s a 4.5 tiles/s (nao "deslizou parado").
    REQUIRE(small_scale > 0);
}

// --- HISTERESE/COAST do walk: SPAM de direcao nao desliza (lider 2026-06-23) --

TEST_CASE("overworld: spammar a direcao MANTEM a anim de walk (nao desliza)",
          "[overworld][coast]") {
    // BUG do lider: tap-tap-tap rapido (ex.: 1 frame movendo, 2 parados) fazia o Gus
    // DESLIZAR sem animar - cada gap caia no ramo parado e RESETAVA o walk pro neutro.
    // Com a histerese (coast), o estado "andando" segura por um buffer curto entre os
    // taps: a anim fica em movimento o tempo todo, nunca cai pro neutro no meio do spam.
    TileGrid g(40, 40, 16.0f);
    OverworldSim sim(g, Aabb{200.0f, 200.0f, 8.0f, 8.0f}, 8.0f);
    sim.set_player_sprites(make_gus_like_sprites(7, 5));
    // Estabelece movimento com um tick andando.
    sim.step_fixed(0, 1, false, 1.0f / 60.0f);  // Sul
    REQUIRE(sim.walk_cycle().is_moving());
    int neutral_hits = 0;
    for (int i = 0; i < 60; ++i) {
        const bool tap = (i % 3 == 0);  // spam: 1 tap a cada 3 frames
        sim.step_fixed(0, tap ? 1 : 0, false, 1.0f / 60.0f);
        if (!sim.walk_cycle().is_moving()) ++neutral_hits;
    }
    REQUIRE(neutral_hits == 0);  // nunca caiu pro idle durante o spam (deslize curado)
}

TEST_CASE("overworld: soltar de verdade (alem do buffer) volta ao idle",
          "[overworld][coast]") {
    // Parar mesmo (soltar e ficar parado alem do coast) tem que voltar ao idle normal.
    TileGrid g(40, 40, 16.0f);
    OverworldSim sim(g, Aabb{200.0f, 200.0f, 8.0f, 8.0f}, 8.0f);
    sim.set_player_sprites(make_gus_like_sprites(7, 5));
    sim.step_fixed(0, 1, false, 1.0f / 60.0f);  // anda Sul
    REQUIRE(sim.walk_cycle().is_moving());
    // Solta por bem mais que o coast (~0.12 s ~ 8 frames): 30 frames parado.
    for (int i = 0; i < 30; ++i) {
        sim.step_fixed(0, 0, false, 1.0f / 60.0f);
    }
    REQUIRE_FALSE(sim.walk_cycle().is_moving());  // idle de volta
}

TEST_CASE("overworld: parado de verdade nao MARCHA no lugar durante o buffer",
          "[overworld][coast]") {
    // Durante o coast SEM movimento real, o quadro do walk e SEGURADO (nao progride):
    // o Gus nao "anda no lugar". Anda um tick, depois fica parado dentro do buffer; o
    // quadro mostrado nao pode avancar enquanto nao ha deslocamento real.
    TileGrid g(40, 40, 16.0f);
    OverworldSim sim(g, Aabb{200.0f, 200.0f, 8.0f, 8.0f}, 8.0f);
    sim.set_player_sprites(make_gus_like_sprites(7, 5));
    sim.step_fixed(1, 0, false, 1.0f / 60.0f);  // Leste, fixa um quadro
    const int held = sim.walk_cycle().current_frame();
    for (int i = 0; i < 5; ++i) {  // parado dentro do coast
        sim.step_fixed(0, 0, false, 1.0f / 60.0f);
        REQUIRE(sim.walk_cycle().current_frame() == held);  // segura: nao marcha
    }
}

TEST_CASE("feel: coast do walk no default (curto, anti-deslize ao spammar)",
          "[overworld][feel][coast]") {
    const OverworldTuning t{};
    // Curto o bastante pra nao "andar no lugar" perceptivel, longo pra cobrir os
    // micro-gaps do spam humano a 60fps. Faixa util ~0.08..0.16 (lider afina no display).
    REQUIRE(t.anim_walk_coast_seconds >= 0.08f);
    REQUIRE(t.anim_walk_coast_seconds <= 0.16f);
}

// --- GUS: walk de 7 quadros + breathing idle animado ------------------------

TEST_CASE("overworld: set_player_sprites configura o ciclo pelo walk_count (Gus 7)",
          "[overworld]") {
    // O WalkCycle tem que ciclar pelos 7 quadros do Gus (nao no 4 do default Caua).
    TileGrid g(40, 40, 16.0f);
    OverworldSim sim(g, Aabb{200.0f, 200.0f, 8.0f, 8.0f}, 8.0f);
    sim.set_player_sprites(make_gus_like_sprites(/*walk*/ 7, /*idle*/ 5));
    REQUIRE(sim.walk_cycle().frame_count() == 7);
    REQUIRE(sim.idle_clock().frame_count() == 5);
}

TEST_CASE("overworld: andando mostra um quadro de walk do Gus da direcao certa",
          "[overworld]") {
    TileGrid g(40, 40, 16.0f);
    OverworldSim sim(g, Aabb{200.0f, 200.0f, 8.0f, 8.0f}, 8.0f);
    PlayerSpriteSet s = make_gus_like_sprites(7, 5);
    sim.set_player_sprites(s);
    for (int i = 0; i < 20; ++i) {
        sim.step_fixed(1, 0, false, 1.0f / 60.0f);  // Leste
    }
    REQUIRE(sim.facing() == Direction::East);
    FakeRenderer r;
    sim.render(r, 80.0f, 80.0f, 1.0f);
    REQUIRE(static_cast<int>(r.sprites.size()) == 1);
    const TextureId t = r.sprites[0].texture;
    bool is_east_walk = false;
    for (int f = 0; f < s.walk_count[static_cast<int>(Direction::East)]; ++f) {
        if (t == s.walk[static_cast<int>(Direction::East)][f]) is_east_walk = true;
    }
    REQUIRE(is_east_walk);
}

TEST_CASE("overworld: parado mostra um quadro do breathing idle (animado)",
          "[overworld]") {
    // O lider pediu RESPIRACAO no idle, nao frame congelado. Parado, o sprite mostrado
    // tem que ser um dos quadros do breathing daquela direcao.
    TileGrid g(40, 40, 16.0f);
    OverworldSim sim(g, Aabb{200.0f, 200.0f, 8.0f, 8.0f}, 8.0f);
    PlayerSpriteSet s = make_gus_like_sprites(7, 5);
    sim.set_player_sprites(s);
    sim.step_fixed(0, 0, false, 1.0f / 60.0f);  // parado, direcao default Sul
    FakeRenderer r;
    sim.render(r, 80.0f, 80.0f, 1.0f);
    REQUIRE(static_cast<int>(r.sprites.size()) == 1);
    const TextureId t = r.sprites[0].texture;
    bool is_south_idle = false;
    for (int f = 0; f < s.idle_count[static_cast<int>(Direction::South)]; ++f) {
        if (t == s.idle_frames[static_cast<int>(Direction::South)][f]) {
            is_south_idle = true;
        }
    }
    REQUIRE(is_south_idle);
}

TEST_CASE(
    "overworld: idle CALMO (descansado) NAO troca quadro - mostra o neutro f0",
    "[overworld]") {
    // IDLE EM DOIS MODOS por STAMINA (lider 2026-06-23). DESCANSADO (stamina cheia >=
    // limiar) = respiracao CALMA PROCEDURAL: o render usa o quadro NEUTRO (frame 0) e
    // aplica uma senoide de bob/escala (testada a parte no breath_oscillator). O sprite
    // mostrado NAO troca de quadro (fim do staccato): segue sempre o idle_frames[di][0].
    TileGrid g(40, 40, 16.0f);
    OverworldSim sim(g, Aabb{200.0f, 200.0f, 8.0f, 8.0f}, 8.0f);
    PlayerSpriteSet s = make_gus_like_sprites(7, 5);
    sim.set_player_sprites(s);

    auto idle_tex = [&]() {
        FakeRenderer r;
        sim.render(r, 80.0f, 80.0f, 1.0f);
        return r.sprites.at(0).texture;
    };

    sim.step_fixed(0, 0, false, 1.0f / 60.0f);  // parado, sem correr -> descansado
    REQUIRE_FALSE(sim.is_tired());
    const TextureId t0 = idle_tex();

    // Varios segundos parado e descansado: o quadro mostrado segue o NEUTRO (f0).
    for (int i = 0; i < 240; ++i) {
        sim.step_fixed(0, 0, false, 1.0f / 60.0f);
    }
    REQUIRE_FALSE(sim.is_tired());
    const TextureId t1 = idle_tex();
    const int di = static_cast<int>(sim.facing());

    REQUIRE(t0 == t1);                                  // nao trocou de quadro (calmo)
    REQUIRE(t1 == s.idle_frames[di][0]);                // e e o quadro NEUTRO
    // E a senoide da respiracao calma avancou (continua, sem staccato).
    REQUIRE(sim.breath().phase() > 0.0f);
}

TEST_CASE(
    "overworld: idle CALMO aplica bob/escala procedural no RECT (sem trocar quadro)",
    "[overworld]") {
    // Prova que a respiracao calma e PROCEDURAL: com o MESMO quadro neutro, o RETANGULO
    // de desenho (altura e/ou y) MUDA ao longo do tempo pela senoide do breath, e o PE
    // (base = y + altura) fica plantado na ancoragem. Sem staccato (textura fixa).
    TileGrid g(40, 40, 16.0f);
    OverworldTuning t = OverworldTuning{};
    t.walk_speed_tiles_per_sec = 8.0f;
    t.idle_calm_scale_amplitude = 0.03f;  // amplitude visivel pro teste
    t.idle_calm_bob_tiles = 0.1f;
    OverworldSim sim(g, Aabb{200.0f, 200.0f, 8.0f, 8.0f}, t);
    sim.set_player_sprites(make_gus_like_sprites(7, 5));

    auto idle_rect_tex = [&]() {
        FakeRenderer r;
        sim.render(r, 80.0f, 80.0f, 1.0f);
        return r.sprites.at(0);
    };

    // Captura o desenho em duas fases distintas da senoide (parado, descansado).
    sim.step_fixed(0, 0, false, 1.0f / 60.0f);
    REQUIRE_FALSE(sim.is_tired());
    const auto a = idle_rect_tex();
    // Avanca ~1/4 do ciclo (~0.94 s a 16/min) -> perto do pico inspirado (osc ~ +1).
    for (int i = 0; i < 56; ++i) {
        sim.step_fixed(0, 0, false, 1.0f / 60.0f);
    }
    const auto b = idle_rect_tex();

    REQUIRE(a.texture == b.texture);  // MESMO quadro neutro (procedural, nao troca)
    // O desenho respirou: altura e y mudaram entre as duas fases.
    const bool rect_changed =
        std::fabs(a.rect.h - b.rect.h) > 1e-3f || std::fabs(a.rect.y - b.rect.y) > 1e-3f;
    REQUIRE(rect_changed);
}

TEST_CASE(
    "overworld: idle OFEGANTE (cansado) troca os quadros do breathing rapido",
    "[overworld]") {
    // CANSADO (Carga < limiar 34) = respiracao OFEGANTE: aI sim toca os 5 quadros do
    // breathing num ritmo RAPIDO (AnimClock). Drenamos a Carga correndo de verdade ate
    // cruzar a ofegancia, depois paramos e provamos que o quadro mostrado MUDA no tempo.
    TileGrid g(40, 40, 16.0f);
    OverworldSim sim(g, Aabb{200.0f, 200.0f, 8.0f, 8.0f}, 8.0f);
    sim.set_player_sprites(make_gus_like_sprites(7, 5));

    auto idle_tex = [&]() {
        FakeRenderer r;
        sim.render(r, 80.0f, 80.0f, 1.0f);
        return r.sprites.at(0).texture;
    };

    // CORRE de verdade (Shift + movimento) por 10 s (600 ticks). NUMEROS CANONICOS:
    // drain 8/s, max 89, limiar 34 -> cruzar a ofegancia leva ~6.9 s (89-34=55, /8);
    // 10 s drenam BEM abaixo do limiar, pra a curta recuperacao do trecho parado nao
    // mascarar o estado cansado. (Paredes nao importam: drena por INTENCAO de input.)
    for (int i = 0; i < 600; ++i) {
        sim.step_fixed(/*dx=*/1, 0, /*run=*/true, 1.0f / 60.0f);
    }
    REQUIRE(sim.is_tired());

    // Agora PARADO e cansado: o idle ofegante deve girar os quadros do breathing.
    sim.step_fixed(0, 0, false, 1.0f / 60.0f);
    const int f0 = sim.idle_clock().frame();
    const TextureId t0 = idle_tex();
    // Ainda cansado logo apos parar: a Carga foi a ~0 e o regen PARADO e 13/s, logo
    // ~0.25 s (15 ticks) sobem so ~3.25 (bem abaixo de 34), mas ja cruzam varias trocas
    // no ritmo ofegante (~6 fps), sem fechar o ciclo de 5 quadros.
    for (int i = 0; i < 15; ++i) {
        sim.step_fixed(0, 0, false, 1.0f / 60.0f);
    }
    REQUIRE(sim.is_tired());
    const int f1 = sim.idle_clock().frame();
    const TextureId t1 = idle_tex();

    REQUIRE(f1 != f0);  // o relogio do breathing ofegante girou
    REQUIRE(t1 != t0);  // e o sprite mostrado acompanhou (5 quadros rapidos)
}

// --- BUG 1 (lider 2026-06-23): parar/colidir NAO gira o boneco -------------

TEST_CASE("overworld: parar PRESERVA a ultima direcao (todas as 4)", "[overworld][facing]") {
    // O bug reportado era o Gus virar pra SUL ao soltar os botoes. A logica do sim deve
    // MANTER o facing anterior quando parado, em qualquer das 4 direcoes.
    TileGrid g(40, 40, 16.0f);
    struct Caso { int dx; int dy; Direction esperado; };
    const Caso casos[] = {
        {1, 0, Direction::East},
        {-1, 0, Direction::West},
        {0, -1, Direction::North},
        {0, 1, Direction::South},
    };
    for (const auto& c : casos) {
        OverworldSim sim(g, Aabb{300.0f, 300.0f, 8.0f, 8.0f}, 4.0f);
        sim.step_fixed(c.dx, c.dy, false, 1.0f / 60.0f);  // anda na direcao
        REQUIRE(sim.facing() == c.esperado);
        // Solta os botoes por varios ticks: o facing NAO pode reverter pra Sul.
        for (int i = 0; i < 30; ++i) {
            sim.step_fixed(0, 0, false, 1.0f / 60.0f);
            REQUIRE(sim.facing() == c.esperado);
        }
    }
}

TEST_CASE("overworld: colidir num eixo PRESERVA o facing do INPUT (nao gira)",
          "[overworld][facing]") {
    // Encostar numa parede num eixo NAO deve girar o boneco: o facing vem do INPUT cru
    // (dx,dy), nao do movimento resolvido (que zera no eixo bloqueado).
    // Mapa: parede a Oeste; jogador colado nela empurrando pra Oeste -> X bloqueia, mas
    // o facing tem que ficar Oeste (input), nao virar pra outra direcao.
    TileGrid g = TileGrid::from_rows({"#..", "#..", "#.."}, 16.0f);
    OverworldSim sim(g, Aabb{16.0f, 20.0f, 8.0f, 8.0f}, 8.0f);
    for (int i = 0; i < 10; ++i) {
        sim.step_fixed(-1, 0, false, 1.0f / 60.0f);  // empurra contra a parede oeste
    }
    REQUIRE(sim.player().x <= 16.0f + kEps);          // ficou colado (X bloqueado)
    REQUIRE(sim.facing() == Direction::West);          // mas continua olhando Oeste

    // Colisao na vertical (parede ao Norte): empurra pra Norte preso -> facing Norte.
    TileGrid gv = TileGrid::from_rows({"###", "...", "..."}, 16.0f);
    OverworldSim simv(gv, Aabb{20.0f, 16.0f, 8.0f, 8.0f}, 8.0f);
    for (int i = 0; i < 10; ++i) {
        simv.step_fixed(0, -1, false, 1.0f / 60.0f);
    }
    REQUIRE(simv.player().y <= 16.0f + kEps);
    REQUIRE(simv.facing() == Direction::North);
}

TEST_CASE("overworld: parado mostra o sprite IDLE da DIRECAO corrente (nao Sul)",
          "[overworld][facing]") {
    // RAIZ VISUAL do bug: parado tem que indexar idle_frames[facing], nao sempre o Sul.
    // Com sprites DIRECIONAIS distintos por direcao, parar olhando Norte mostra um
    // quadro de idle do NORTE, nunca do Sul.
    TileGrid g(40, 40, 16.0f);
    OverworldSim sim(g, Aabb{300.0f, 300.0f, 8.0f, 8.0f}, 8.0f);
    PlayerSpriteSet s = make_gus_like_sprites(7, 5);  // handles distintos por direcao
    sim.set_player_sprites(s);

    sim.step_fixed(0, -1, false, 1.0f / 60.0f);   // anda pro Norte
    REQUIRE(sim.facing() == Direction::North);
    // Solta ALEM do coast (~0.10 s ~ 6 frames): a histerese segura o walk por um
    // instante apos o ultimo passo, entao o IDLE so aparece depois do buffer expirar.
    // 12 frames parado garante o estado idle (sem isso, o boneco ainda mostraria walk).
    for (int i = 0; i < 12; ++i) {
        sim.step_fixed(0, 0, false, 1.0f / 60.0f);  // para
    }
    REQUIRE(sim.facing() == Direction::North);     // facing preservado
    FakeRenderer r;
    sim.render(r, 80.0f, 80.0f, 1.0f);
    const TextureId t = r.sprites.at(0).texture;
    const int north = static_cast<int>(Direction::North);
    const int south = static_cast<int>(Direction::South);
    // O quadro mostrado e um idle do NORTE...
    bool is_north_idle = false;
    for (int f = 0; f < s.idle_count[north]; ++f) {
        if (t == s.idle_frames[north][f]) is_north_idle = true;
    }
    REQUIRE(is_north_idle);
    // ...e NAO um idle do Sul (o bug original).
    for (int f = 0; f < s.idle_count[south]; ++f) {
        REQUIRE(t != s.idle_frames[south][f]);
    }
}

// --- TIMER DE FOLEGO (corpo) vs CARGA (aparato) - lider 2026-06-23 -----------

TEST_CASE(
    "overworld: ao parar de correr o Gus ofega por >= 5 s mesmo com a Carga recarregada",
    "[overworld][winded]") {
    // BUG do lider: a ofegancia (antes so atada a Carga) durava ~2-3 s porque a Carga
    // regenera rapido (~13/s) ao parar. Com o TIMER DE FOLEGO separado, parar apos uma
    // corrida longa mantem o idle ofegante por >= 5 s INDEPENDENTE da Carga.
    TileGrid g(60, 60, 16.0f);
    OverworldSim sim(g, Aabb{300.0f, 300.0f, 8.0f, 8.0f}, 8.0f);
    sim.set_player_sprites(make_gus_like_sprites(7, 5));

    // Corre o bastante pra encher o folego (>= run_for_max 8 s). 9 s = 540 ticks.
    for (int i = 0; i < 540; ++i) {
        sim.step_fixed(1, 0, /*run=*/true, 1.0f / 60.0f);
    }
    // Para: dispara o folego. A Carga ja vai recuperar rapido daqui pra frente.
    sim.step_fixed(0, 0, false, 1.0f / 60.0f);
    REQUIRE(sim.is_winded());
    REQUIRE(sim.show_winded_idle());

    // Parado 4.5 s (270 ticks): a Carga ja recarregou MUITO (13/s -> bem acima do
    // limiar 34), mas o folego do corpo ainda nao zerou -> SEGUE ofegante.
    for (int i = 0; i < 270; ++i) {
        sim.step_fixed(0, 0, false, 1.0f / 60.0f);
    }
    REQUIRE_FALSE(sim.is_tired());      // a CARGA ja descansou (prova a independencia)
    REQUIRE(sim.is_winded());           // mas o folego do CORPO ainda esta ativo
    REQUIRE(sim.show_winded_idle());    // -> idle ofegante forcado pelo folego
}

TEST_CASE("overworld: a ofegancia ESCALA com o tempo de corrida (mais corre, mais ofega)",
          "[overworld][winded]") {
    TileGrid g(60, 60, 16.0f);
    OverworldSim curto(g, Aabb{300.0f, 300.0f, 8.0f, 8.0f}, 8.0f);
    OverworldSim longo(g, Aabb{300.0f, 300.0f, 8.0f, 8.0f}, 8.0f);
    // Corrida curta (3 s) vs longa (8 s), ambas acima do limiar de 2 s.
    for (int i = 0; i < 180; ++i) curto.step_fixed(1, 0, true, 1.0f / 60.0f);
    for (int i = 0; i < 480; ++i) longo.step_fixed(1, 0, true, 1.0f / 60.0f);
    curto.step_fixed(0, 0, false, 1.0f / 60.0f);
    longo.step_fixed(0, 0, false, 1.0f / 60.0f);
    REQUIRE(longo.winded().remaining() > curto.winded().remaining());
    REQUIRE(curto.winded().remaining() >= 5.0f - 1e-3f);  // piso
    REQUIRE(longo.winded().remaining() <= 8.0f + 1e-3f);  // teto
}

TEST_CASE("overworld: andar (sem correr) e parar NAO dispara folego", "[overworld][winded]") {
    // So CORRER (sprint real) conta. Andar muito tempo e parar nao deve ofegar.
    TileGrid g(60, 60, 16.0f);
    OverworldSim sim(g, Aabb{300.0f, 300.0f, 8.0f, 8.0f}, 8.0f);
    for (int i = 0; i < 600; ++i) {
        sim.step_fixed(1, 0, /*run=*/false, 1.0f / 60.0f);  // anda 10 s, sem Shift
    }
    sim.step_fixed(0, 0, false, 1.0f / 60.0f);  // para
    REQUIRE_FALSE(sim.is_winded());
}

TEST_CASE("overworld: idle congelado (1 quadro) nao quebra - compat Caua",
          "[overworld]") {
    // make_fake_sprites = idle de 1 quadro (Caua). Parado tem que mostrar idle[di] e
    // o clock fica com frame_count 1 (congelado), sem animar.
    TileGrid g = make_map();
    OverworldSim sim(g, Aabb{36.0f, 36.0f, 8.0f, 8.0f}, 4.0f);
    PlayerSpriteSet s = make_fake_sprites();
    sim.set_player_sprites(s);
    REQUIRE(sim.idle_clock().frame_count() == 1);
    sim.step_fixed(0, 0, false, 1.0f / 60.0f);
    FakeRenderer r;
    sim.render(r, 80.0f, 80.0f, 1.0f);
    REQUIRE(r.sprites[0].texture == s.idle[static_cast<int>(Direction::South)]);
}

// --- CALIBRACAO DE FEEL (lider 2026-06-23, no display) ----------------------
// Trava as 3 decisoes de feel do lider nos DEFAULTS do OverworldTuning pra um edit
// descuidado nao desfazer o ajuste em silencio. Sao invariantes de DADO PURO (sem
// SDL/GPU/IO), nao testam render. O lider ainda afina os numeros exatos no display;
// estes testes so guardam a FAIXA/DIRECAO da calibracao, nao o valor cravado.
TEST_CASE("feel: zoom default reduzido ~10% (lider no display)", "[overworld][feel]") {
    using Catch::Matchers::WithinAbs;
    const OverworldTuning t{};
    // Era 48; o lider pediu -10% (~43). Faixa util do header ~24..64.
    REQUIRE_THAT(t.camera_zoom_px_per_tile, WithinAbs(43.0f, 1e-4f));
    REQUIRE(t.camera_zoom_px_per_tile < 48.0f);          // ficou MENOR (menos zoom)
    REQUIRE(t.camera_zoom_px_per_tile >= 24.0f);         // dentro da faixa util
    REQUIRE(t.camera_zoom_px_per_tile <= 64.0f);
}

TEST_CASE("feel: cadencia do walk NATURAL (meio-termo, lider round 2)",
          "[overworld][feel]") {
    const OverworldTuning t{};
    // ROUND 2 (lider no display): 0.30 = arrastado, 0.16 = frenetico ("passos muito
    // rapidos"). Buscado o MEIO-TERMO em ciclos/s a 4.5 tiles/s (ciclo de 7 quadros):
    // ciclos/s = 4.5 / (7*cadencia). Janela pedida 0.22..0.24 (0.23 = 2.80 ciclos/s,
    // passo natural nem arrastado nem frenetico). Trava a janela + a direcao.
    REQUIRE(t.anim_walk_tiles_per_frame >= 0.18f);       // nao frenetico (mais lento que 0.16)
    REQUIRE(t.anim_walk_tiles_per_frame <= 0.24f);       // nao arrastado (mais rapido que 0.30)
    // Correr troca MENOS quadros por tile (passada mais LONGA, "pe colado" na corrida) e
    // mantem ~1.4x a cadencia do walk.
    REQUIRE(t.anim_run_tiles_per_frame > t.anim_walk_tiles_per_frame);
    REQUIRE(t.anim_run_tiles_per_frame >= t.anim_walk_tiles_per_frame * 1.3f);
    REQUIRE(t.anim_run_tiles_per_frame <= t.anim_walk_tiles_per_frame * 1.5f);
}

TEST_CASE("feel: respiracao calma so BOB (escala zerada, sem esticar/achatar, lider round 2)",
          "[overworld][feel]") {
    const OverworldTuning t{};
    // ROUND 2 (lider no display): "ainda estica/achata muito" mesmo com escala em 0.008.
    // CAUSA REAL = a escala procedural e ANCORADA NA BASE (squash/stretch nao-uniforme):
    // a cabeca balanca com o pe cravado, e em pixel-art rigido o olho le isso como
    // deformacao elastica mesmo a ~1px. CORRECAO: ZERAR a escala; a respiracao calma fica
    // SO no bob (sobe-desce UNIFORME, sem distorcer). NAO era o idle ofegante (winded), que
    // so dispara apos corrida real, nunca apos caminhada.
    REQUIRE(t.idle_calm_scale_amplitude == 0.0f);        // escala DESLIGADA (sem squash/stretch)
    REQUIRE(t.idle_calm_bob_tiles > 0.0f);               // ainda respira, so pelo bob
    REQUIRE(t.idle_calm_bob_tiles <= 0.06f);             // deslize vertical contido (sutil)
}

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
        for (int f = 0; f < kWalkFrameCount; ++f) {
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

TEST_CASE("overworld: camera segue o jogador presa ao mapa", "[overworld]") {
    TileGrid g = make_map();
    OverworldSim sim(g, Aabb{36.0f, 36.0f, 8.0f, 8.0f}, 4.0f);
    CameraView v = sim.camera_view(40.0f, 40.0f);
    REQUIRE_THAT(v.center.x, WithinAbs(40.0, kEps));
    REQUIRE_THAT(v.center.y, WithinAbs(40.0, kEps));
    REQUIRE_THAT(v.rect.x, WithinAbs(20.0, kEps));
}

TEST_CASE("overworld: camera clampa na borda", "[overworld]") {
    TileGrid g = make_map();
    OverworldSim sim(g, Aabb{16.0f, 16.0f, 8.0f, 8.0f}, 4.0f);
    CameraView v = sim.camera_view(40.0f, 40.0f);
    REQUIRE(v.rect.x >= -kEps);
    REQUIRE(v.rect.y >= -kEps);
}

TEST_CASE("overworld: render emite paredes e jogador", "[overworld]") {
    TileGrid g = make_map();
    OverworldSim sim(g, Aabb{36.0f, 36.0f, 8.0f, 8.0f}, 4.0f);
    FakeRenderer r;
    sim.render(r, 80.0f, 80.0f, 0.0f);
    REQUIRE(r.begins == 1);
    REQUIRE(r.ends == 1);
    REQUIRE(r.filled_count() == 17);  // 16 borda + 1 bloco interno
    REQUIRE(r.player_cmd() != nullptr);
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

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

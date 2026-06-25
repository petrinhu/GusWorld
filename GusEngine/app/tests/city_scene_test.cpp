// GusEngine/app/tests/city_scene_test.cpp
//
// Catch2 (headless) da montagem da CENA DA CIDADE a partir de um TileMap (M4-visual).
// Prova, sem janela:
//   - spawn_player_aabb: hitbox ~0.6 tile CENTRADA na celula de spawn();
//   - make_city_scene: colisao = to_tile_grid (so Parede bloqueia), spawn certo,
//     TileMap guardado pro render por TileKind, e o Gus colide com Parede;
//   - color_for_tile: cada TileKind -> sua cor da paleta (graybox);
//   - o render por TileMap pinta TODAS as celulas (nao so as bloqueadas).
// I/O de arquivo (.gmap do disco) NAO entra aqui: e exercitado pelo smoke do main.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <cstdint>

#include "gus/app/screens/city_scene.hpp"
#include "gus/app/screens/overworld_sim.hpp"
#include "gus/app/screens/tile_palette.hpp"
#include "gus/core/spatial/grid_collision.hpp"
#include "gus/domain/map/tile_map.hpp"
#include "gus/platform/render2d/i_renderer.hpp"

using Catch::Matchers::WithinAbs;
using gus::app::screens::color_for_tile;
using gus::app::screens::make_city_scene;
using gus::app::screens::make_city_tuning;
using gus::app::screens::OverworldSim;
using gus::app::screens::spawn_player_aabb;
using gus::app::screens::TilePalette;
using gus::core::spatial::Aabb;
using gus::core::spatial::Rect;
using gus::domain::map::Cell;
using gus::domain::map::TileKind;
using gus::domain::map::TileMap;
using gus::platform::render2d::DrawColor;
using gus::platform::render2d::IRenderer;
using gus::platform::render2d::TextureId;
using gus::platform::render2d::UvRect;

namespace {

std::uint16_t k(TileKind t) { return static_cast<std::uint16_t>(t); }

// Mapa pequeno e coerente: bordas de Parede, um Marco, spawn no meio. tile_size 2.0
// (igual ao .gmap real) pra apanhar bugs de escala.
TileMap small_walled_map() {
    TileMap m(5, 4, 2.0f);
    for (std::int32_t x = 0; x < 5; ++x) {
        m.set(x, 0, k(TileKind::Parede));
        m.set(x, 3, k(TileKind::Parede));
    }
    for (std::int32_t y = 0; y < 4; ++y) {
        m.set(0, y, k(TileKind::Parede));
        m.set(4, y, k(TileKind::Parede));
    }
    m.set(2, 1, k(TileKind::Marco));
    m.set(2, 2, k(TileKind::Chao));
    m.set_spawn(Cell{2, 2});
    m.validate();
    return m;
}

// IRenderer falso minimo: conta os filled-rects e guarda cor/retangulo.
class CountingRenderer : public IRenderer {
public:
    struct Fill {
        Rect rect;
        DrawColor color;
    };
    void begin_frame(const Rect&, int, int) override { fills.clear(); }
    void draw_filled_rect(const Rect& r, const DrawColor& c) override {
        fills.push_back({r, c});
    }
    void draw_rect_outline(const Rect&, const DrawColor&, float) override {}
    TextureId load_texture(const char*) override { return 0; }
    void draw_textured_rect(const Rect&, TextureId, const UvRect&,
                            const DrawColor&) override {}
    gus::platform::render2d::ContentBbox texture_content_bbox(
        TextureId) const override {
        return {};
    }
    void draw_text(const char*, float, float, float, const DrawColor&,
                   bool) override {}
    void end_frame() override {}

    std::vector<Fill> fills;
};

}  // namespace

TEST_CASE("spawn_player_aabb centra a hitbox na celula de spawn", "[city_scene]") {
    const TileMap m = small_walled_map();
    const Aabb a = spawn_player_aabb(m);

    // tile_size 2.0, hitbox = 0.6*2.0 = 1.2. Centro da celula (2,2) = (2.5, 2.5)*2.0
    // = (5.0, 5.0). Canto sup-esq = centro - meia-hitbox = 5.0 - 0.6 = 4.4.
    REQUIRE_THAT(a.w, WithinAbs(1.2f, 1e-4f));
    REQUIRE_THAT(a.h, WithinAbs(1.2f, 1e-4f));
    REQUIRE_THAT(a.x, WithinAbs(4.4f, 1e-4f));
    REQUIRE_THAT(a.y, WithinAbs(4.4f, 1e-4f));
}

TEST_CASE("make_city_scene nasce no spawn e guarda o TileMap", "[city_scene]") {
    const TileMap m = small_walled_map();
    OverworldSim sim = make_city_scene(m, make_city_tuning());

    // Nasce centrado no spawn.
    const Aabb p = sim.player();
    REQUIRE_THAT(p.x, WithinAbs(4.4f, 1e-4f));
    REQUIRE_THAT(p.y, WithinAbs(4.4f, 1e-4f));

    // Guardou o TileMap (pro render por TileKind) e a colisao casa o tile_size real.
    REQUIRE(sim.tile_map().has_value());
    REQUIRE(sim.grid().tile_size() == 2.0f);
    REQUIRE(sim.grid().width() == 5);
    REQUIRE(sim.grid().height() == 4);
    // Parede da borda bloqueia; o Marco (2,1) e andavel; o spawn (2,2) e andavel.
    REQUIRE(sim.grid().is_blocked(0, 0));
    REQUIRE_FALSE(sim.grid().is_blocked(2, 1));
    REQUIRE_FALSE(sim.grid().is_blocked(2, 2));
}

TEST_CASE("o Gus colide com a Parede do mapa real", "[city_scene]") {
    const TileMap m = small_walled_map();
    OverworldSim sim = make_city_scene(m, make_city_tuning());

    // Empurra forte pra ESQUERDA por varios ticks: a borda (coluna 0 = Parede) trava.
    const float dt = 1.0f / 60.0f;
    for (int i = 0; i < 600; ++i) {
        sim.step_fixed(/*dx=*/-1, /*dy=*/0, /*run=*/true, dt);
    }
    // Nao atravessa a coluna 0 (Parede): x do canto fica >= a borda interna da
    // coluna 1 (1*tile_size = 2.0). Com folga numerica.
    REQUIRE(sim.player().x >= 2.0f - 1e-3f);
}

TEST_CASE("color_for_tile mapeia cada TileKind pra sua cor", "[tile_palette]") {
    TilePalette pal;  // defaults aprovados
    REQUIRE(color_for_tile(pal, k(TileKind::Chao)).g == pal.chao.g);
    REQUIRE(color_for_tile(pal, k(TileKind::Parede)).b == pal.parede.b);
    REQUIRE(color_for_tile(pal, k(TileKind::Marco)).r == pal.marco.r);
    REQUIRE(color_for_tile(pal, k(TileKind::Entrada)).g == pal.entrada.g);
    REQUIRE(color_for_tile(pal, k(TileKind::Saida)).b == pal.saida.b);
    // id reservado ao futuro (sem TileKind) cai no Chao (andavel).
    REQUIRE(color_for_tile(pal, 9999).r == pal.chao.r);
}

TEST_CASE("render do mapa real pinta TODAS as celulas por TileKind", "[city_scene]") {
    const TileMap m = small_walled_map();  // 5x4 = 20 celulas
    OverworldSim sim = make_city_scene(m, make_city_tuning());

    CountingRenderer r;
    // Viewport gigante em mundo: a camera clampa, mas todas as 20 celulas cabem na
    // janela (mapa 10x8 em mundo). Espera 20 fills (uma por celula), nao so paredes.
    sim.render(r, /*viewport_w=*/1000.0f, /*viewport_h=*/1000.0f, /*alpha=*/0.0f);
    REQUIRE(r.fills.size() == 20);

    // A celula de Marco (2,1) pintou na cor de Marco da paleta.
    const TilePalette pal = sim.tile_palette();
    bool found_marco = false;
    for (const auto& f : r.fills) {
        // Centro da celula (2,1): x=(2.5)*2=5.0, y=(1.5)*2=3.0.
        if (f.rect.x <= 5.0f && f.rect.x + f.rect.w > 5.0f && f.rect.y <= 3.0f &&
            f.rect.y + f.rect.h > 3.0f) {
            REQUIRE(f.color.r == pal.marco.r);
            REQUIRE(f.color.g == pal.marco.g);
            found_marco = true;
        }
    }
    REQUIRE(found_marco);
}

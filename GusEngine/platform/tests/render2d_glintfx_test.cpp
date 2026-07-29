// SPDX-License-Identifier: GPL-3.0-or-later
// GusEngine/platform/tests/render2d_glintfx_test.cpp
//
// Catch2 do Render2dGlintfx (platform/render2d), a TERCEIRA implementacao de IRenderer
// (D2D-2-SENTINELA), que delega ao glintfx::Draw2d (pin v0.23.0) pras 5 primitivas de
// mundo (draw_text fica no caminho FreeType/FontAtlas atual - ver render2d_glintfx.hpp).
// TEST-FIRST.
//
// DOIS BLOCOS:
//   1. EQUIVALENCIA DE CAMERA (a parte delicada da fatia): prova, com matematica PURA
//      (sem GPU/contexto), que a conversao Rect-de-mundo -> Camera2d do glintfx
//      (camera_from_world_rect) projeta os MESMOS pixels que o world_to_screen do
//      Render2dGl3 (viewport_transform.hpp), para uma camera de aspecto uniforme (o
//      invariante que clamp_camera()+world_span_from_pixels() garantem em todo caminho
//      de render do jogo). Chama a funcao REAL glintfx::world_to_screen (pura, D12,
//      compila sem GLINTFX_MODULE_APP nem contexto GL) - nao reimplementa a formula.
//   2. HEADLESS: mesmo contrato de seguranca do Render2dGl3 (gl_active=false -> nenhuma
//      chamada GL/Draw2D, tudo no-op contabilizado), mesmo padrao de teste.
//
// O irredutivel de GPU real (Draw2d::init() com contexto, upload de textura, o quad de
// texto na pipeline GL propria) fica pro smoke visual do app, igual ao Render2dGl3.

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <glintfx/draw2d.hpp>

#include "gus/core/spatial/camera_clamp.hpp"
#include "gus/platform/render2d/render2d_glintfx.hpp"
#include "gus/platform/render2d/viewport_transform.hpp"

using Catch::Approx;
using gus::core::spatial::Rect;
using gus::platform::render2d::DrawColor;
using gus::platform::render2d::kInvalidTexture;
using gus::platform::render2d::Render2dGlintfx;
using gus::platform::render2d::ScreenPoint;
using gus::platform::render2d::UvRect;
using gus::platform::render2d::world_to_screen;

namespace {

// Compara a projecao de um ponto de mundo pelas DUAS matematicas: a nossa
// (viewport_transform::world_to_screen, usada pelo Render2dGl3) e a do glintfx
// (glintfx::world_to_screen com a Camera2d construida por camera_from_world_rect) - a
// MESMA funcao pura que Render2dGlintfx::begin_frame usaria internamente.
void require_same_projection(float world_x, float world_y, const Rect& cam_rect,
                             int pixel_w, int pixel_h) {
    const ScreenPoint ours =
        world_to_screen(world_x, world_y, cam_rect, pixel_w, pixel_h);

    const glintfx::RectF world_rect{cam_rect.x, cam_rect.y, cam_rect.w, cam_rect.h};
    const glintfx::Camera2d cam =
        glintfx::camera_from_world_rect(world_rect, pixel_w, pixel_h);
    const glintfx::Vec2F theirs =
        glintfx::world_to_screen(cam, pixel_w, pixel_h, glintfx::Vec2F{world_x, world_y});

    INFO("world=(" << world_x << "," << world_y << ") cam={" << cam_rect.x << ","
                   << cam_rect.y << "," << cam_rect.w << "," << cam_rect.h << "} viewport="
                   << pixel_w << "x" << pixel_h);
    REQUIRE(theirs.x == Approx(ours.x).margin(0.01));
    REQUIRE(theirs.y == Approx(ours.y).margin(0.01));
}

}  // namespace

TEST_CASE("camera_from_world_rect equivale ao world_to_screen do Render2dGl3 (aspecto "
          "uniforme, zoom=1)",
          "[render2d_glintfx][camera]") {
    // cam == viewport em unidades (ppu=1), o caso mais comum no repo (ex.:
    // npc_dialogue_loop_gl.cpp/anim_preview.cpp: Rect{0,0,pw,ph}).
    const Rect cam{0.0f, 0.0f, 960.0f, 540.0f};
    require_same_projection(0.0f, 0.0f, cam, 960, 540);        // canto superior-esquerdo
    require_same_projection(960.0f, 540.0f, cam, 960, 540);    // canto inferior-direito
    require_same_projection(480.0f, 270.0f, cam, 960, 540);    // centro
    require_same_projection(100.0f, 50.0f, cam, 960, 540);     // ponto arbitrario
}

TEST_CASE("camera_from_world_rect equivale ao world_to_screen do Render2dGl3 (aspecto "
          "uniforme, zoom=20, camera deslocada)",
          "[render2d_glintfx][camera]") {
    // Mapa 60x40 unidades, viewport 1200x800 px (zoom=20 nos dois eixos, camera clampada
    // fora da origem) - o caso real do OverworldSim (clamp_camera + world_span_from_pixels
    // com o MESMO ppu nos dois eixos).
    const Rect cam{10.0f, 20.0f, 60.0f, 40.0f};
    require_same_projection(10.0f, 20.0f, cam, 1200, 800);   // canto superior-esquerdo
    require_same_projection(70.0f, 60.0f, cam, 1200, 800);   // canto inferior-direito
    require_same_projection(40.0f, 40.0f, cam, 1200, 800);   // centro
    require_same_projection(15.5f, 37.25f, cam, 1200, 800);  // ponto arbitrario/subpixel
}

TEST_CASE("Render2dGlintfx headless (sem contexto GL) nao crasha e conta draws",
          "[render2d_glintfx]") {
    Render2dGlintfx r(/*gl_active=*/false);  // headless: tudo no-op contabilizado
    const Rect cam{0.0f, 0.0f, 64.0f, 64.0f};
    r.begin_frame(cam, 64, 64);
    r.draw_filled_rect(Rect{8.0f, 8.0f, 16.0f, 16.0f},
                       DrawColor{0.2f, 0.2f, 0.3f, 1.0f});
    r.draw_rect_outline(Rect{0.0f, 0.0f, 64.0f, 64.0f},
                        DrawColor{0.0f, 1.0f, 1.0f, 1.0f}, 1.0f);
    r.end_frame();
    REQUIRE(r.last_draw_count() == 2);
}

TEST_CASE("Render2dGlintfx headless: load_texture degrada para invalido",
          "[render2d_glintfx]") {
    Render2dGlintfx r(/*gl_active=*/false);
    REQUIRE(r.load_texture("nao/existe.png") == kInvalidTexture);
}

TEST_CASE("Render2dGlintfx headless: sprite invalido nao desenha nada",
          "[render2d_glintfx]") {
    Render2dGlintfx r(/*gl_active=*/false);
    const Rect cam{0.0f, 0.0f, 64.0f, 64.0f};
    r.begin_frame(cam, 64, 64);
    r.draw_textured_rect(Rect{0.0f, 0.0f, 16.0f, 16.0f}, kInvalidTexture, UvRect{},
                         DrawColor{1.0f, 1.0f, 1.0f, 1.0f});
    r.end_frame();
    REQUIRE(r.last_draw_count() == 0);  // textura invalida = no-op (fallback e do app)
}

TEST_CASE("Render2dGlintfx headless: texture_content_bbox degrada para invalido",
          "[render2d_glintfx]") {
    Render2dGlintfx r(/*gl_active=*/false);
    const auto bbox = r.texture_content_bbox(kInvalidTexture);
    REQUIRE_FALSE(bbox.valid());
}

TEST_CASE("Render2dGlintfx frame vazio e valido (zero draws)", "[render2d_glintfx]") {
    Render2dGlintfx r(/*gl_active=*/false);
    const Rect cam{0.0f, 0.0f, 32.0f, 32.0f};
    r.begin_frame(cam, 32, 32);
    r.end_frame();
    REQUIRE(r.last_draw_count() == 0);
}

TEST_CASE("Render2dGlintfx headless: draw_text nao crasha (sem fonte/contexto)",
          "[render2d_glintfx]") {
    Render2dGlintfx r(/*gl_active=*/false);
    const Rect cam{0.0f, 0.0f, 640.0f, 360.0f};
    r.begin_frame(cam, 640, 360);
    r.draw_text("Atacar", 10.0f, 10.0f, 8.0f,
                DrawColor{1.0f, 1.0f, 1.0f, 1.0f}, /*bold=*/false);
    r.end_frame();
    REQUIRE(r.last_draw_count() == 0);  // headless: sem fonte, nada emitido
}

TEST_CASE("Render2dGlintfx present diferido (simetria de API) headless e seguro",
          "[render2d_glintfx]") {
    Render2dGlintfx r(/*gl_active=*/false);
    REQUIRE_FALSE(r.defer_present());
    r.set_defer_present(true);
    REQUIRE(r.defer_present());
    const Rect cam{0.0f, 0.0f, 32.0f, 32.0f};
    r.begin_frame(cam, 32, 32);
    r.draw_filled_rect(Rect{0.0f, 0.0f, 8.0f, 8.0f}, DrawColor{1, 0, 0, 1});
    r.end_frame();
    REQUIRE(r.last_draw_count() == 1);
    r.present();  // headless: no-op seguro
    SUCCEED("present diferido headless ok");
}

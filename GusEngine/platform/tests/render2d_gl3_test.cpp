// GusEngine/platform/tests/render2d_gl3_test.cpp
//
// Catch2 do Render2dGl3 (platform/render2d), o backend de IRenderer sobre OpenGL 3.3
// (ADR-009 adendo GL3). TEST-FIRST.
//
// O irredutivel de GPU/contexto GL (criar programa/VAO, emitir glDrawElements, swap) e
// coberto pelo SMOKE VISUAL do app (--battle com contexto GL real). Aqui exercitamos a
// LOGICA do backend que da pra travar sem display: o ciclo begin/draw/end conta os
// primitivos certos e o MODO HEADLESS (gl_active == false) nao chama NENHUMA funcao GL
// (nao ha contexto), so contabiliza - espelha o Render2dSdl(nullptr). A matematica de
// projecao mundo->pixel ja e testada em viewport_transform_test (compartilhada); aqui o
// foco e o contrato do IRenderer e a seguranca headless.

#include <catch2/catch_test_macros.hpp>

#include "gus/core/spatial/camera_clamp.hpp"
#include "gus/platform/render2d/render2d_gl3.hpp"
#include "gus/platform/rmlui/gl3_loader.hpp"

using gus::core::spatial::Rect;
using gus::platform::render2d::DrawColor;
using gus::platform::render2d::kInvalidTexture;
using gus::platform::render2d::Render2dGl3;
using gus::platform::render2d::UvRect;

TEST_CASE("Render2dGl3 headless (sem contexto GL) nao crasha e conta draws",
          "[render2d_gl3]") {
    Render2dGl3 r(/*gl_active=*/false);  // headless: tudo no-op contabilizado, zero GL
    const Rect cam{0.0f, 0.0f, 64.0f, 64.0f};
    r.begin_frame(cam, 64, 64);
    r.draw_filled_rect(Rect{8.0f, 8.0f, 16.0f, 16.0f},
                       DrawColor{0.2f, 0.2f, 0.3f, 1.0f});
    r.draw_rect_outline(Rect{0.0f, 0.0f, 64.0f, 64.0f},
                        DrawColor{0.0f, 1.0f, 1.0f, 1.0f}, 1.0f);
    r.end_frame();
    REQUIRE(r.last_draw_count() == 2);
}

TEST_CASE("Render2dGl3 headless: load_texture degrada para invalido", "[render2d_gl3]") {
    Render2dGl3 r(/*gl_active=*/false);
    // Sem contexto GL nao da pra criar textura: qualquer caminho devolve invalido.
    REQUIRE(r.load_texture("nao/existe.png") == kInvalidTexture);
}

TEST_CASE("Render2dGl3 headless: sprite invalido nao desenha nada", "[render2d_gl3]") {
    Render2dGl3 r(/*gl_active=*/false);
    const Rect cam{0.0f, 0.0f, 64.0f, 64.0f};
    r.begin_frame(cam, 64, 64);
    r.draw_textured_rect(Rect{0.0f, 0.0f, 16.0f, 16.0f}, kInvalidTexture, UvRect{},
                         DrawColor{1.0f, 1.0f, 1.0f, 1.0f});
    r.end_frame();
    REQUIRE(r.last_draw_count() == 0);  // textura invalida = no-op (cabe ao app o fallback)
}

TEST_CASE("Render2dGl3 headless: texture_content_bbox degrada para invalido",
          "[render2d_gl3]") {
    Render2dGl3 r(/*gl_active=*/false);
    const auto bbox = r.texture_content_bbox(kInvalidTexture);
    REQUIRE_FALSE(bbox.valid());
}

TEST_CASE("Render2dGl3 frame vazio e valido (zero draws)", "[render2d_gl3]") {
    Render2dGl3 r(/*gl_active=*/false);
    const Rect cam{0.0f, 0.0f, 32.0f, 32.0f};
    r.begin_frame(cam, 32, 32);
    r.end_frame();
    REQUIRE(r.last_draw_count() == 0);
}

TEST_CASE("Render2dGl3 headless: draw_text nao crasha (sem fonte/contexto)",
          "[render2d_gl3]") {
    Render2dGl3 r(/*gl_active=*/false);
    const Rect cam{0.0f, 0.0f, 640.0f, 360.0f};
    r.begin_frame(cam, 640, 360);
    r.draw_text("Atacar", 10.0f, 10.0f, 8.0f,
                DrawColor{1.0f, 1.0f, 1.0f, 1.0f}, /*bold=*/false);
    r.end_frame();
    REQUIRE(r.last_draw_count() == 0);  // headless: sem fonte, nada emitido
}

TEST_CASE("gl3_load_functions(nullptr) degrada para false (sem loader)",
          "[render2d_gl3][gl3_loader]") {
    // Loader nulo = sem como resolver proc-address: nao carrega (false), nao crasha. O
    // caminho com loader real (SDL_GL_GetProcAddress) e exercitado no smoke visual --battle.
    REQUIRE_FALSE(gus::platform::rmlui::gl3_load_functions(nullptr));
}

TEST_CASE("Render2dGl3 present diferido (ADR-009) headless e seguro", "[render2d_gl3]") {
    Render2dGl3 r(/*gl_active=*/false);
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

// GusEngine/platform/tests/render2d_sdl_test.cpp
//
// Catch2 do Render2dSdl (platform/render2d), o backend de IRenderer sobre
// SDL_Renderer (pos repivot ADR-008). TEST-FIRST.
//
// O irredutivel de GPU/janela (criar device, apresentar na tela) e coberto pelo
// smoke do app. Aqui exercitamos a LOGICA do backend que da pra travar sem
// display: o ciclo begin/draw/end conta os primitivos certos, e o MODO HEADLESS
// (renderer == nullptr) nao crasha e degrada texturas para kInvalidTexture - o
// mesmo caminho do smoke SDL dummy. A matematica de projecao mundo->pixel em si ja
// e testada em viewport_transform_test; aqui o foco e o contrato do IRenderer.

#include <catch2/catch_test_macros.hpp>

#include "gus/core/spatial/camera_clamp.hpp"
#include "gus/platform/render2d/render2d_sdl.hpp"

using gus::core::spatial::Rect;
using gus::platform::render2d::ContentBbox;
using gus::platform::render2d::DrawColor;
using gus::platform::render2d::kInvalidTexture;
using gus::platform::render2d::Render2dSdl;
using gus::platform::render2d::UvRect;

TEST_CASE("Render2dSdl headless (renderer nulo) nao crasha", "[render2d_sdl]") {
    Render2dSdl r(nullptr);  // modo headless: tudo no-op contabilizado
    const Rect cam{0.0f, 0.0f, 64.0f, 64.0f};
    r.begin_frame(cam, 64, 64);
    r.draw_filled_rect(Rect{8.0f, 8.0f, 16.0f, 16.0f},
                       DrawColor{0.2f, 0.2f, 0.3f, 1.0f});
    r.draw_rect_outline(Rect{30.0f, 30.0f, 10.0f, 10.0f},
                        DrawColor{0.2f, 0.9f, 0.9f, 1.0f}, 2.0f);
    r.end_frame();
    // 1 quad preenchido + 1 contorno = 2 primitivos emitidos.
    REQUIRE(r.last_draw_count() == 2);
}

TEST_CASE("Render2dSdl headless: load_texture degrada para invalido",
          "[render2d_sdl]") {
    Render2dSdl r(nullptr);
    // Sem renderer, nao ha como criar SDL_Texture: devolve o sentinela invalido.
    REQUIRE(r.load_texture("qualquer/caminho.png") == kInvalidTexture);
}

TEST_CASE("Render2dSdl headless: sprite invalido nao desenha nada",
          "[render2d_sdl]") {
    Render2dSdl r(nullptr);
    const Rect cam{0.0f, 0.0f, 32.0f, 32.0f};
    r.begin_frame(cam, 32, 32);
    // draw_textured_rect com textura invalida e no-op (cabe ao chamador o fallback).
    r.draw_textured_rect(Rect{0.0f, 0.0f, 8.0f, 8.0f}, kInvalidTexture,
                         UvRect{0.0f, 0.0f, 1.0f, 1.0f},
                         DrawColor{1.0f, 1.0f, 1.0f, 1.0f});
    r.end_frame();
    REQUIRE(r.last_draw_count() == 0);
}

TEST_CASE("Render2dSdl headless: texture_content_bbox degrada para invalido",
          "[render2d_sdl]") {
    Render2dSdl r(nullptr);
    // Sem renderer, nenhuma textura e criada e nenhum PNG e decodificado: o bbox e
    // invalido (valid()==false, bottom_margin()==0) e o anchor cai no comportamento
    // legado. Tambem para o sentinela invalido e handles fora de faixa.
    const ContentBbox b0 = r.texture_content_bbox(kInvalidTexture);
    REQUIRE_FALSE(b0.valid());
    REQUIRE(b0.bottom_margin() == 0);
    const ContentBbox b1 = r.texture_content_bbox(r.load_texture("x.png"));
    REQUIRE_FALSE(b1.valid());  // load_texture devolveu invalido (headless)
    const ContentBbox b2 = r.texture_content_bbox(9999);  // fora de faixa
    REQUIRE_FALSE(b2.valid());
}

TEST_CASE("Render2dSdl frame vazio e valido (zero draws)", "[render2d_sdl]") {
    Render2dSdl r(nullptr);
    const Rect cam{0.0f, 0.0f, 32.0f, 32.0f};
    r.begin_frame(cam, 32, 32);
    r.end_frame();
    REQUIRE(r.last_draw_count() == 0);
}

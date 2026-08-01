// SPDX-License-Identifier: Apache-2.0
// GusEngine/platform/tests/font_atlas_test.cpp
//
// Catch2 (headless) do FONT ATLAS (M5, incremento 3.5). O bake (stb_truetype -> bitmap
// de glifos + metricas) NAO precisa de SDL/janela: roda na CPU. Prova:
//   - resolve_font_path monta o caminho (env > macro embutido > relativo);
//   - bake_font_atlas carrega a Pixel Operator Mono embarcada e produz um atlas valido
//     (bitmap nao-vazio, celula quadrada, cobre o ASCII printable, glifo da letra tem
//     pixels acesos);
//   - glyph_uv devolve a sub-regiao [0,1] de um caractere;
//   - degradacao: arquivo ausente => atlas invalido (valid()==false), sem crash.
//
// Se a fonte embarcada sumir do repo, o teste de "carrega a fonte real" acende: e o
// guarda de que o asset CC0 continua versionado.

#include <catch2/catch_test_macros.hpp>

#include <string>

#include "gus/platform/render2d/font_atlas.hpp"

using gus::platform::render2d::bake_cell_px_for_draw;
using gus::platform::render2d::bake_font_atlas;
using gus::platform::render2d::FontAtlas;
using gus::platform::render2d::quantize_cell_px;
using gus::platform::render2d::resolve_font_path;

TEST_CASE("resolve_font_path monta um caminho terminando no .ttf pedido",
          "[font_atlas]") {
    const std::string p = resolve_font_path("PixelOperatorMono.ttf");
    REQUIRE(p.size() >= 4);
    REQUIRE(p.substr(p.size() - 4) == ".ttf");
    REQUIRE(p.find("PixelOperatorMono.ttf") != std::string::npos);
}

TEST_CASE("bake_font_atlas carrega a Pixel Operator Mono embarcada (CC0)",
          "[font_atlas]") {
    const std::string p = resolve_font_path("PixelOperatorMono.ttf");
    const FontAtlas atlas = bake_font_atlas(p, /*cell_px=*/16);
    REQUIRE(atlas.valid());
    // Bitmap nao-vazio e celula coerente.
    REQUIRE(atlas.atlas_w > 0);
    REQUIRE(atlas.atlas_h > 0);
    REQUIRE(atlas.cell_px == 16);
    REQUIRE(static_cast<int>(atlas.pixels.size()) == atlas.atlas_w * atlas.atlas_h);
    // Cobre o ASCII printable (32..126): o primeiro e o ultimo glifo existem.
    REQUIRE(atlas.has_glyph(' '));
    REQUIRE(atlas.has_glyph('A'));
    REQUIRE(atlas.has_glyph('~'));
    // A letra 'A' tem pixels acesos (foi rasterizada, nao e celula vazia).
    REQUIRE(atlas.glyph_has_ink('A'));
    // O espaco NAO tem tinta (so avanco).
    REQUIRE_FALSE(atlas.glyph_has_ink(' '));
}

TEST_CASE("BUG A: o atlas cobre os acentos do pt-br (Latin-1) com tinta",
          "[font_atlas]") {
    const FontAtlas atlas =
        bake_font_atlas(resolve_font_path("PixelOperatorMono.ttf"), 16);
    REQUIRE(atlas.valid());
    // Code points essenciais do pt-br (Latin-1 Supplement): a-til, c-cedilha, e-agudo,
    // a-agudo, o-til, e maiusculas. Todos devem existir E ter tinta (rasterizados), nao
    // virar buraco. (Antes do incremento 6, o atlas so tinha ASCII -> acento = buraco.)
    const int cps[] = {0x00E3 /*ã*/, 0x00E7 /*ç*/, 0x00E9 /*é*/, 0x00E1 /*á*/,
                       0x00F3 /*ó*/, 0x00ED /*í*/, 0x00FA /*ú*/, 0x00C7 /*Ç*/,
                       0x00C1 /*Á*/};
    for (int cp : cps) {
        REQUIRE(atlas.has_glyph(cp));
        REQUIRE(atlas.glyph_has_ink(cp));
        const auto uv = atlas.glyph_uv(cp);
        REQUIRE(uv.w > 0.0f);
        REQUIRE(uv.u + uv.w <= 1.0f + 1e-4f);
        REQUIRE(uv.v + uv.h <= 1.0f + 1e-4f);
    }
    // Code point fora das faixas (ex.: U+0100 Latin Extended) cai em UV vazio sem crash.
    REQUIRE_FALSE(atlas.has_glyph(0x0100));
    REQUIRE(atlas.glyph_uv(0x0100).w == 0.0f);
}

TEST_CASE("glyph_uv devolve sub-regiao normalizada [0,1] do caractere",
          "[font_atlas]") {
    const FontAtlas atlas =
        bake_font_atlas(resolve_font_path("PixelOperatorMono.ttf"), 16);
    REQUIRE(atlas.valid());
    const auto uv = atlas.glyph_uv('A');
    REQUIRE(uv.u >= 0.0f);
    REQUIRE(uv.v >= 0.0f);
    REQUIRE(uv.u + uv.w <= 1.0f + 1e-4f);
    REQUIRE(uv.v + uv.h <= 1.0f + 1e-4f);
    REQUIRE(uv.w > 0.0f);
    REQUIRE(uv.h > 0.0f);
    // Caractere fora do range printable cai num UV vazio (w==0) sem crash.
    const auto uv_bad = atlas.glyph_uv('\x01');
    REQUIRE(uv_bad.w == 0.0f);
}

TEST_CASE("quantize_cell_px arredonda o px_size fracionario pro inteiro mais proximo",
          "[font_atlas]") {
    // D2D-2-SENTINELA: o bake escolhe a RESOLUCAO pelo inteiro mais proximo do px_size
    // logico (float) - so a resolucao do bake muda, o layout (text_metrics) continua float.
    REQUIRE(quantize_cell_px(8.0f) == 8);
    REQUIRE(quantize_cell_px(8.4f) == 8);   // arredonda pra baixo
    REQUIRE(quantize_cell_px(8.6f) == 9);   // arredonda pra cima
    REQUIRE(quantize_cell_px(20.16f) == 20);  // exemplo real: cam.rect.h*0.028f (~sdl_window)
}

TEST_CASE("quantize_cell_px degrada pro minimo (1) sem crash em entradas invalidas",
          "[font_atlas]") {
    REQUIRE(quantize_cell_px(0.0f) == 1);
    REQUIRE(quantize_cell_px(-5.0f) == 1);
    REQUIRE(quantize_cell_px(0.4f) == 1);  // arredondaria pra 0; clampado em 1
    // NaN: comparacao "> 0.0f" com NaN e sempre false -> cai no ramo de invalido.
    REQUIRE(quantize_cell_px(std::numeric_limits<float>::quiet_NaN()) == 1);
}

TEST_CASE("bake_cell_px_for_draw escala px_size pelo zoom da camera (nao usa px_size cru)",
          "[font_atlas]") {
    // REGRESSAO (achada em 2026-07, pos-review do team-lead): o HUD do cockpit desenha
    // com camera ~identidade (Rect{0,0,960,540}, pixel_w/h tipicamente ~960x540) -
    // px_size (mundo) ja aproxima o tamanho final em tela, entao a escala e ~1.0.
    REQUIRE(bake_cell_px_for_draw(8.0f, /*cam_axis_world=*/540.0f,
                                  /*viewport_axis_px=*/540) == 8);
    REQUIRE(bake_cell_px_for_draw(8.0f, 540.0f, 1080) == 16);  // janela 2x: escala 2.0

    // CASO REAL que expos o bug do design anterior (quantizar px_size CRU): o overlay de
    // dialogo (sdl_window.cpp:render_dialogue_overlay_frame) usa a camera do OVERWORLD,
    // que tem ZOOM de verdade (px_per_world_unit=21.5 medido via probe standalone contra
    // a cidade real). px_size ali e ~0.9377 (unidades de MUNDO) - quantize_cell_px cru
    // dava cell_px=1 (atlas ILEGIVEL), mesmo o glifo aparecendo com ~20px REAIS na tela.
    // bake_cell_px_for_draw com o eixo da camera (cam.rect.h=33.4884, viewport_px_h=720)
    // corrige pro tamanho fisico correto.
    const float px_size_dialogo = 0.937674f;  // cam.rect.h(33.4884) * 0.028, medido
    const float cam_h_overworld = 33.4884f;   // cam.rect.h medido (cidade real, probe)
    const int cell_px = bake_cell_px_for_draw(px_size_dialogo, cam_h_overworld, 720);
    REQUIRE(cell_px == 20);  // 0.9377 * (720/33.4884) = 0.9377*21.5 = 20.16 -> 20
    REQUIRE(cell_px > 1);    // trava a regressao: NUNCA mais cai pro bake ilegivel de nda

    // Camera/viewport degenerados: cai pra escala 1.0 (trata px_size cru como se ja
    // fosse tela), nunca divide por zero, nunca crash.
    REQUIRE(bake_cell_px_for_draw(8.0f, /*cam_axis_world=*/0.0f, 720) == 8);
    REQUIRE(bake_cell_px_for_draw(8.0f, -5.0f, 720) == 8);
    REQUIRE(bake_cell_px_for_draw(8.0f, 540.0f, /*viewport_axis_px=*/0) == 8);
    REQUIRE(bake_cell_px_for_draw(8.0f, 540.0f, -100) == 8);
}

TEST_CASE("bake_font_atlas produz atlases distintos e coerentes em varios cell_px",
          "[font_atlas]") {
    // Prova a base do cache por (face, cell_px): tamanhos diferentes bakeiam atlases
    // com dimensoes proporcionais (cols=rows=14 fixos - kFontGlyphCount=191), sem
    // interferir uns nos outros (bake_font_atlas e uma funcao pura por chamada).
    for (const int cell_px : {8, 9, 10, 11, 12, 13, 14, 15, 20, 24, 30}) {
        const FontAtlas atlas =
            bake_font_atlas(resolve_font_path("PixelOperatorMono.ttf"), cell_px);
        REQUIRE(atlas.valid());
        REQUIRE(atlas.cell_px == cell_px);
        REQUIRE(atlas.atlas_w == atlas.cols * cell_px);
        REQUIRE(atlas.atlas_h == atlas.rows * cell_px);
        REQUIRE(atlas.glyph_has_ink('A'));
    }
}

TEST_CASE("bake degrada sem crash quando o arquivo nao existe", "[font_atlas]") {
    const FontAtlas atlas = bake_font_atlas("/nao/existe/fonte.ttf", 16);
    REQUIRE_FALSE(atlas.valid());
    REQUIRE(atlas.pixels.empty());
    // Consultas num atlas invalido sao seguras (UV vazio, sem ink).
    REQUIRE_FALSE(atlas.has_glyph('A'));
    REQUIRE(atlas.glyph_uv('A').w == 0.0f);
}

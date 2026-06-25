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

using gus::platform::render2d::bake_font_atlas;
using gus::platform::render2d::FontAtlas;
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

TEST_CASE("bake degrada sem crash quando o arquivo nao existe", "[font_atlas]") {
    const FontAtlas atlas = bake_font_atlas("/nao/existe/fonte.ttf", 16);
    REQUIRE_FALSE(atlas.valid());
    REQUIRE(atlas.pixels.empty());
    // Consultas num atlas invalido sao seguras (UV vazio, sem ink).
    REQUIRE_FALSE(atlas.has_glyph('A'));
    REQUIRE(atlas.glyph_uv('A').w == 0.0f);
}

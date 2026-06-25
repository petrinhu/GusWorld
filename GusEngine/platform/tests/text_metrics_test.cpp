// GusEngine/platform/tests/text_metrics_test.cpp
//
// Catch2 (headless) das METRICAS DE TEXTO puras (M5, incremento 3.5). Prova, SEM SDL
// nem fonte carregada: contagem de caracteres, largura monospace (= chars * ratio *
// px), altura de linha, avanco de glifo. E a peca que a LOGICA de layout (centrar
// verbo, caber o log) usa headless, separada do raster.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "gus/platform/render2d/text_metrics.hpp"

using Catch::Matchers::WithinAbs;
using gus::platform::render2d::decode_utf8;
using gus::platform::render2d::glyph_advance;
using gus::platform::render2d::kMonoAdvanceRatio;
using gus::platform::render2d::text_char_count;
using gus::platform::render2d::text_height;
using gus::platform::render2d::text_width;

TEST_CASE("text_char_count conta CODE POINTS (nao bytes), inclui acentos UTF-8",
          "[text_metrics]") {
    REQUIRE(text_char_count("") == 0);
    REQUIRE(text_char_count("Atacar") == 6);
    REQUIRE(text_char_count("HP 45/58") == 8);
    // BUG A: "Aceleracao" com acentos UTF-8 ("Aceleração" = A-c-e-l-e-r-a-c-a-o, 10
    // code points, mas o c-cedilha e o a-til sao 2 bytes cada => 12 bytes). Conta 10.
    REQUIRE(text_char_count("Acelera\xC3\xA7\xC3\xA3o") == 10);  // Aceleração
    REQUIRE(text_char_count("An\xC3\xA1lise") == 7);              // Análise
}

TEST_CASE("decode_utf8 itera CODE POINTS de uma string UTF-8", "[text_metrics]") {
    // "cão" = c (0x63) + a-til (U+00E3 = 0xC3 0xA3) + o (0x6F) = 3 code points.
    const auto cps = decode_utf8("c\xC3\xA3o");
    REQUIRE(cps.size() == 3);
    REQUIRE(cps[0] == 0x63);    // 'c'
    REQUIRE(cps[1] == 0x00E3);  // 'ã' (U+00E3)
    REQUIRE(cps[2] == 0x6F);    // 'o'
    // ASCII puro: 1 code point por byte.
    const auto ascii = decode_utf8("AB");
    REQUIRE(ascii.size() == 2);
    REQUIRE(ascii[0] == 'A');
    // c-cedilha minusculo (U+00E7) e maiusculo (U+00C7).
    const auto ce = decode_utf8("\xC3\xA7\xC3\x87");  // ç Ç
    REQUIRE(ce.size() == 2);
    REQUIRE(ce[0] == 0x00E7);
    REQUIRE(ce[1] == 0x00C7);
}

TEST_CASE("text_width = chars * ratio * px (monospace)", "[text_metrics]") {
    const float px = 8.0f;
    REQUIRE_THAT(text_width("", px), WithinAbs(0.0f, 1e-5f));
    REQUIRE_THAT(text_width("AB", px),
                 WithinAbs(2.0f * kMonoAdvanceRatio * px, 1e-5f));
    REQUIRE_THAT(text_width("Atacar", px),
                 WithinAbs(6.0f * kMonoAdvanceRatio * px, 1e-5f));
    // px <= 0 => 0 (sem largura negativa).
    REQUIRE_THAT(text_width("X", 0.0f), WithinAbs(0.0f, 1e-5f));
    REQUIRE_THAT(text_width("X", -4.0f), WithinAbs(0.0f, 1e-5f));
}

TEST_CASE("text_width cresce com o numero de caracteres", "[text_metrics]") {
    REQUIRE(text_width("Defender", 8.0f) > text_width("Scan", 8.0f));
}

TEST_CASE("text_height = px_size (1 linha), clamp >= 0", "[text_metrics]") {
    REQUIRE_THAT(text_height(8.0f), WithinAbs(8.0f, 1e-5f));
    REQUIRE_THAT(text_height(16.0f), WithinAbs(16.0f, 1e-5f));
    REQUIRE_THAT(text_height(-1.0f), WithinAbs(0.0f, 1e-5f));
}

TEST_CASE("glyph_advance = ratio * px", "[text_metrics]") {
    REQUIRE_THAT(glyph_advance(8.0f), WithinAbs(kMonoAdvanceRatio * 8.0f, 1e-5f));
}

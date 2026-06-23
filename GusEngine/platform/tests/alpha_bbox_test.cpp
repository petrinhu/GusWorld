// GusEngine/platform/tests/alpha_bbox_test.cpp
//
// Catch2 da VARREDURA do alpha-bbox (scan_alpha_content_bbox), POCO puro: mede o
// bounding-box do conteudo nao-transparente de um buffer RGBA8. TEST-FIRST. E o que
// faz o anchor do sprite COLAR o pe na base da AABB sem numero magico (M1-BUG.SUL):
// a margem inferior transparente sai daqui, medida de cada PNG. Sem SDL/GPU/I-O.

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <vector>

#include "gus/platform/render2d/alpha_bbox.hpp"

using gus::platform::render2d::ContentBbox;
using gus::platform::render2d::scan_alpha_content_bbox;

namespace {

// Monta um buffer RGBA8 w*h todo transparente (alpha 0).
std::vector<std::uint8_t> make_transparent(int w, int h) {
    return std::vector<std::uint8_t>(static_cast<std::size_t>(w) * h * 4, 0);
}

// Pinta o pixel (x,y) opaco (alpha 255) no buffer.
void set_opaque(std::vector<std::uint8_t>& buf, int w, int x, int y) {
    const std::size_t i = (static_cast<std::size_t>(y) * w + x) * 4;
    buf[i + 0] = 255;
    buf[i + 1] = 255;
    buf[i + 2] = 255;
    buf[i + 3] = 255;
}

}  // namespace

TEST_CASE("alpha_bbox: bloco opaco centrado mede left/top/width/height",
          "[alpha_bbox]") {
    // Canvas 10x10; conteudo opaco no retangulo [2..7]x[3..6].
    const int w = 10, h = 10;
    auto buf = make_transparent(w, h);
    for (int y = 3; y <= 6; ++y) {
        for (int x = 2; x <= 7; ++x) {
            set_opaque(buf, w, x, y);
        }
    }
    const ContentBbox b = scan_alpha_content_bbox(buf.data(), w, h);
    REQUIRE(b.valid());
    REQUIRE(b.canvas_w == 10);
    REQUIRE(b.canvas_h == 10);
    REQUIRE(b.left == 2);
    REQUIRE(b.top == 3);
    REQUIRE(b.width == 6);   // 2..7 inclusive
    REQUIRE(b.height == 4);  // 3..6 inclusive
}

TEST_CASE("alpha_bbox: a MARGEM INFERIOR transparente e canvas_h - (top+height)",
          "[alpha_bbox]") {
    // Conteudo nas linhas [0..56] de um canvas de 68 (caso medido do Caua south):
    // sobra inferior = 68 - 57 = 11 px.
    const int w = 4, h = 68;
    auto buf = make_transparent(w, h);
    for (int y = 0; y <= 56; ++y) {
        set_opaque(buf, w, 0, y);
    }
    const ContentBbox b = scan_alpha_content_bbox(buf.data(), w, h);
    REQUIRE(b.valid());
    REQUIRE(b.height == 57);          // 0..56
    REQUIRE(b.bottom_margin() == 11);  // 68 - 57
}

TEST_CASE("alpha_bbox: imagem toda transparente e invalida (margem 0)",
          "[alpha_bbox]") {
    const int w = 8, h = 8;
    auto buf = make_transparent(w, h);
    const ContentBbox b = scan_alpha_content_bbox(buf.data(), w, h);
    REQUIRE_FALSE(b.valid());
    REQUIRE(b.bottom_margin() == 0);  // sem medicao confiavel => anchor legado
}

TEST_CASE("alpha_bbox: entradas degeneradas nao crasham e ficam invalidas",
          "[alpha_bbox]") {
    REQUIRE_FALSE(scan_alpha_content_bbox(nullptr, 4, 4).valid());
    std::uint8_t dummy[4] = {255, 255, 255, 255};
    REQUIRE_FALSE(scan_alpha_content_bbox(dummy, 0, 4).valid());
    REQUIRE_FALSE(scan_alpha_content_bbox(dummy, 4, 0).valid());
}

TEST_CASE("alpha_bbox: conteudo encostado na base nao tem margem inferior",
          "[alpha_bbox]") {
    const int w = 4, h = 20;
    auto buf = make_transparent(w, h);
    for (int y = 5; y <= 19; ++y) {  // ate a ultima linha
        set_opaque(buf, w, 1, y);
    }
    const ContentBbox b = scan_alpha_content_bbox(buf.data(), w, h);
    REQUIRE(b.valid());
    REQUIRE(b.bottom_margin() == 0);  // pe ja na base do canvas
}

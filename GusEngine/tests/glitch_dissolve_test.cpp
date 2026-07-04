// GusEngine/tests/glitch_dissolve_test.cpp
//
// Catch2 do POCO core::anim::glitch_* (M7-COSTURA Inc 2b, "GLITCH DIGITAL/
// PROCESSANDO" - substitui o fade preto liso, pedido ao vivo do Gus Dragon + veredito
// do lider). Matematica pura (hash determinista + limiar por celula), headless -
// sem SDL/GL/janela. TEST-FIRST.
//
// NOMES DE TEST_CASE CURTOS DE PROPOSITO (<=76 chars): nomes longos fazem o
// reporter --list-tests do Catch2 QUEBRAR LINHA, e o parser do catch_discover_
// tests (CMake) nao lida com nome multilinha - funde TODOS os casos seguintes
// num unico teste ctest (achado empirico desta sessao; ver comentario git).

#include <cmath>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "gus/core/anim/glitch_dissolve.hpp"

using Catch::Matchers::WithinAbs;
using namespace gus::core::anim;

TEST_CASE("glitch_cell_threshold: determinista (mesma celula sempre)",
          "[glitch_dissolve]") {
    const float a = glitch_cell_threshold(3, 5);
    const float b = glitch_cell_threshold(3, 5);
    REQUIRE_THAT(a, WithinAbs(b, 1e-9f));
}

TEST_CASE("glitch_cell_threshold: sempre entre 0 e 1 (exclusive)",
          "[glitch_dissolve]") {
    for (int row = 0; row < kGlitchGridRows; ++row) {
        for (int col = 0; col < kGlitchGridCols; ++col) {
            const float t = glitch_cell_threshold(col, row);
            REQUIRE(t >= 0.0f);
            REQUIRE(t < 1.0f);
        }
    }
}

TEST_CASE("glitch_cell_threshold: celulas diferentes divergem",
          "[glitch_dissolve]") {
    // Sanity: nem TODAS as celulas da grade tem o mesmo limiar (senao a "dissolve"
    // degeneraria de volta a um fade liso uniforme, o que o efeito existe pra evitar).
    int distinct = 0;
    float first = glitch_cell_threshold(0, 0);
    for (int row = 0; row < kGlitchGridRows; ++row) {
        for (int col = 0; col < kGlitchGridCols; ++col) {
            if (glitch_cell_threshold(col, row) != first) {
                ++distinct;
            }
        }
    }
    REQUIRE(distinct > 0);
}

TEST_CASE("glitch_block_alpha: alpha<=0 -> 0 em qualquer celula",
          "[glitch_dissolve]") {
    REQUIRE_THAT(glitch_block_alpha(0, 0, 0.0f), WithinAbs(0.0f, 1e-6f));
    REQUIRE_THAT(glitch_block_alpha(15, 8, 0.0f), WithinAbs(0.0f, 1e-6f));
    REQUIRE_THAT(glitch_block_alpha(7, 3, -1.0f), WithinAbs(0.0f, 1e-6f));
}

TEST_CASE("glitch_block_alpha: alpha>=1 -> 1 em qualquer celula",
          "[glitch_dissolve]") {
    // Invariante de seguranca: no pico, a grade INTEIRA fica opaca (a troca de
    // renderer da Maestro fica escondida atras de preto solido, sem excecao).
    for (int row = 0; row < kGlitchGridRows; ++row) {
        for (int col = 0; col < kGlitchGridCols; ++col) {
            REQUIRE_THAT(glitch_block_alpha(col, row, 1.0f), WithinAbs(1.0f, 1e-6f));
            REQUIRE_THAT(glitch_block_alpha(col, row, 2.0f), WithinAbs(1.0f, 1e-6f));
        }
    }
}

TEST_CASE("glitch_block_alpha: acende ao cruzar o limiar da celula",
          "[glitch_dissolve]") {
    constexpr int kCol = 4;
    constexpr int kRow = 2;
    const float threshold = glitch_cell_threshold(kCol, kRow);

    if (threshold > 0.001f) {
        REQUIRE_THAT(glitch_block_alpha(kCol, kRow, threshold * 0.5f),
                     WithinAbs(0.0f, 1e-6f));
    }
    REQUIRE_THAT(glitch_block_alpha(kCol, kRow, threshold), WithinAbs(1.0f, 1e-6f));
    if (threshold < 0.999f) {
        const float beyond = threshold + (1.0f - threshold) * 0.5f;
        REQUIRE_THAT(glitch_block_alpha(kCol, kRow, beyond), WithinAbs(1.0f, 1e-6f));
    }
}

TEST_CASE("glitch_cell_is_wavefront: false nos extremos do envelope",
          "[glitch_dissolve]") {
    // Nunca ha franja colorida/deslocada quando a tela deveria estar limpa (alpha
    // 0) ou 100% solida (alpha 1) - o invariante de seguranca da troca de renderer.
    for (int row = 0; row < kGlitchGridRows; ++row) {
        for (int col = 0; col < kGlitchGridCols; ++col) {
            REQUIRE_FALSE(glitch_cell_is_wavefront(col, row, 0.0f, 0.1f));
            REQUIRE_FALSE(glitch_cell_is_wavefront(col, row, 1.0f, 0.1f));
        }
    }
}

TEST_CASE("glitch_cell_is_wavefront: true logo apos acender",
          "[glitch_dissolve]") {
    constexpr int kCol = 9;
    constexpr int kRow = 6;
    constexpr float kBand = 0.05f;
    const float threshold = glitch_cell_threshold(kCol, kRow);

    if (threshold < 0.999f) {
        // Bem no limiar (acabou de acender): franja ativa.
        REQUIRE(glitch_cell_is_wavefront(kCol, kRow, threshold, kBand));
    }
    if (threshold < 1.0f - kBand - 0.001f) {
        // Um pouco alem do limiar mas AINDA dentro da banda.
        REQUIRE(glitch_cell_is_wavefront(kCol, kRow, threshold + kBand * 0.5f, kBand));
    }
}

TEST_CASE("glitch_cell_is_wavefront: false bem alem da banda",
          "[glitch_dissolve]") {
    // A celula ja "assentada" (preta ha varios frames) nao deve mais piscar.
    constexpr int kCol = 1;
    constexpr int kRow = 1;
    constexpr float kBand = 0.05f;
    const float threshold = glitch_cell_threshold(kCol, kRow);

    // Um valor bem alem do limiar + banda (mas ainda < 1, senao cai no extremo).
    const float far_beyond = threshold + kBand + (1.0f - threshold - kBand) * 0.9f;
    if (far_beyond > threshold && far_beyond < 1.0f) {
        REQUIRE_FALSE(glitch_cell_is_wavefront(kCol, kRow, far_beyond, kBand));
    }
}

TEST_CASE("glitch_cell_is_wavefront: false com band<=0", "[glitch_dissolve]") {
    REQUIRE_FALSE(glitch_cell_is_wavefront(0, 0, 0.5f, 0.0f));
    REQUIRE_FALSE(glitch_cell_is_wavefront(0, 0, 0.5f, -1.0f));
}

TEST_CASE("glitch_cell_shift_fraction: determinista e em [-1,1]",
          "[glitch_dissolve]") {
    for (int row = 0; row < kGlitchGridRows; ++row) {
        for (int col = 0; col < kGlitchGridCols; ++col) {
            const float s1 = glitch_cell_shift_fraction(col, row);
            const float s2 = glitch_cell_shift_fraction(col, row);
            REQUIRE_THAT(s1, WithinAbs(s2, 1e-9f));
            REQUIRE(s1 >= -1.0f);
            REQUIRE(s1 <= 1.0f);
        }
    }
}

TEST_CASE("glitch_cell_shift_fraction: independente do limiar",
          "[glitch_dissolve]") {
    // Sanity fraca: em pelo menos ALGUMAS celulas, threshold e |shift| divergem o
    // bastante pra nao serem a mesma sequencia disfarcada (hash independente).
    int divergent = 0;
    for (int row = 0; row < kGlitchGridRows; ++row) {
        for (int col = 0; col < kGlitchGridCols; ++col) {
            const float threshold = glitch_cell_threshold(col, row);
            const float shift_mapped_to_01 =
                (glitch_cell_shift_fraction(col, row) + 1.0f) * 0.5f;
            if (std::abs(threshold - shift_mapped_to_01) > 0.05f) {
                ++divergent;
            }
        }
    }
    REQUIRE(divergent > 0);
}

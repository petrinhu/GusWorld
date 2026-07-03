// GusEngine/tests/fade_transition_test.cpp
//
// Catch2 do POCO core::anim::fade_overlay_alpha (M7-COSTURA Inc 2, ADR-012 decisao 5:
// "fade preto curto com crossfade de musica"). Matematica pura (interpolacao linear
// clampada), headless - sem SDL/GL/janela. TEST-FIRST.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "gus/core/anim/fade_transition.hpp"

using Catch::Matchers::WithinAbs;
using gus::core::anim::FadeDirection;
using gus::core::anim::fade_overlay_alpha;

TEST_CASE("fade_overlay_alpha kOut: comeca em 0 (tela visivel)", "[fade_transition]") {
    REQUIRE_THAT(fade_overlay_alpha(FadeDirection::kOut, 0.0f, 0.4f),
                 WithinAbs(0.0f, 1e-5f));
}

TEST_CASE("fade_overlay_alpha kOut: termina em 1 (tela 100% preta)", "[fade_transition]") {
    REQUIRE_THAT(fade_overlay_alpha(FadeDirection::kOut, 0.4f, 0.4f),
                 WithinAbs(1.0f, 1e-5f));
}

TEST_CASE("fade_overlay_alpha kOut: meio do caminho = 0.5", "[fade_transition]") {
    REQUIRE_THAT(fade_overlay_alpha(FadeDirection::kOut, 0.2f, 0.4f),
                 WithinAbs(0.5f, 1e-5f));
}

TEST_CASE("fade_overlay_alpha kIn: comeca em 1 (tela 100% preta)", "[fade_transition]") {
    REQUIRE_THAT(fade_overlay_alpha(FadeDirection::kIn, 0.0f, 0.4f),
                 WithinAbs(1.0f, 1e-5f));
}

TEST_CASE("fade_overlay_alpha kIn: termina em 0 (tela totalmente visivel)",
          "[fade_transition]") {
    REQUIRE_THAT(fade_overlay_alpha(FadeDirection::kIn, 0.4f, 0.4f),
                 WithinAbs(0.0f, 1e-5f));
}

TEST_CASE("fade_overlay_alpha kIn: meio do caminho = 0.5", "[fade_transition]") {
    REQUIRE_THAT(fade_overlay_alpha(FadeDirection::kIn, 0.2f, 0.4f),
                 WithinAbs(0.5f, 1e-5f));
}

TEST_CASE("fade_overlay_alpha clampa elapsed NEGATIVO no inicio da fase",
          "[fade_transition]") {
    REQUIRE_THAT(fade_overlay_alpha(FadeDirection::kOut, -1.0f, 0.4f),
                 WithinAbs(0.0f, 1e-5f));
    REQUIRE_THAT(fade_overlay_alpha(FadeDirection::kIn, -1.0f, 0.4f),
                 WithinAbs(1.0f, 1e-5f));
}

TEST_CASE("fade_overlay_alpha clampa elapsed ALEM da duracao no fim da fase",
          "[fade_transition]") {
    REQUIRE_THAT(fade_overlay_alpha(FadeDirection::kOut, 999.0f, 0.4f),
                 WithinAbs(1.0f, 1e-5f));
    REQUIRE_THAT(fade_overlay_alpha(FadeDirection::kIn, 999.0f, 0.4f),
                 WithinAbs(0.0f, 1e-5f));
}

TEST_CASE("fade_overlay_alpha duration_seconds<=0 devolve o estado FINAL direto "
          "(sem dividir por zero)",
          "[fade_transition]") {
    REQUIRE_THAT(fade_overlay_alpha(FadeDirection::kOut, 0.0f, 0.0f),
                 WithinAbs(1.0f, 1e-5f));
    REQUIRE_THAT(fade_overlay_alpha(FadeDirection::kIn, 0.0f, 0.0f),
                 WithinAbs(0.0f, 1e-5f));
    REQUIRE_THAT(fade_overlay_alpha(FadeDirection::kOut, 0.0f, -1.0f),
                 WithinAbs(1.0f, 1e-5f));
}

TEST_CASE("fade_overlay_alpha: kOut e kIn sao espelhados (soma 1) no mesmo instante",
          "[fade_transition]") {
    for (float t = 0.0f; t <= 0.4f; t += 0.05f) {
        const float out = fade_overlay_alpha(FadeDirection::kOut, t, 0.4f);
        const float in = fade_overlay_alpha(FadeDirection::kIn, t, 0.4f);
        REQUIRE_THAT(out + in, WithinAbs(1.0f, 1e-4f));
    }
}

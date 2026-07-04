// GusEngine/tests/boot_pixel_sequence_test.cpp
//
// Catch2 do POCO core::anim::boot_pixel_* (M7-COSTURA Inc 2c: sequencia de frames
// pre-renderizada substitui o glitch procedural vetado pelo lider - "pareceu bug").
// Matematica pura (progresso -> indice de frame, por LEG da transicao), headless -
// sem SDL/GL/janela. TEST-FIRST.
//
// A transicao INTEIRA cidade<->batalha tem 2 "pernas" (legs) cada sentido: o lado que
// ESCURECE (Darkening) e o lado que REVELA (Revealing) - ver o header pra o mapeamento
// completo. Os testes abaixo provam que as 4 pernas se ENCAIXAM numa unica sequencia
// CONTINUA (0->19 indo pra batalha; 19->0 voltando), sem "pulo pra tras" na costura.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "gus/core/anim/boot_pixel_sequence.hpp"

using Catch::Matchers::WithinAbs;
using gus::core::anim::boot_pixel_frame_index;
using gus::core::anim::boot_pixel_safety_alpha;
using gus::core::anim::BootPixelLeg;
using gus::core::anim::kBootPixelFrameCount;

// --- boot_pixel_frame_index: extremos de cada perna ---

TEST_CASE("boot_pixel_frame_index: kToBattleDarkening comeca em 0",
          "[boot_pixel_sequence]") {
    REQUIRE(boot_pixel_frame_index(BootPixelLeg::kToBattleDarkening, 0.0f,
                                    kBootPixelFrameCount) == 0);
}

TEST_CASE("boot_pixel_frame_index: kToBattleRevealing termina no ULTIMO frame "
          "('SYSTEM READY.' exatamente quando a arena aparece)",
          "[boot_pixel_sequence]") {
    REQUIRE(boot_pixel_frame_index(BootPixelLeg::kToBattleRevealing, 1.0f,
                                    kBootPixelFrameCount) ==
            kBootPixelFrameCount - 1);
}

TEST_CASE("boot_pixel_frame_index: kFromBattleDarkening comeca no ULTIMO frame",
          "[boot_pixel_sequence]") {
    REQUIRE(boot_pixel_frame_index(BootPixelLeg::kFromBattleDarkening, 0.0f,
                                    kBootPixelFrameCount) ==
            kBootPixelFrameCount - 1);
}

TEST_CASE("boot_pixel_frame_index: kFromBattleRevealing termina em 0 (tela escura/"
          "cursor exatamente quando a cidade reaparece)",
          "[boot_pixel_sequence]") {
    REQUIRE(boot_pixel_frame_index(BootPixelLeg::kFromBattleRevealing, 1.0f,
                                    kBootPixelFrameCount) == 0);
}

// --- A COSTURA: a perna Darkening("indo") termina exatamente onde a perna
// Revealing("indo") comeca (sem pulo/reset pra frame 0) - e o espelho pra volta. ---

TEST_CASE("boot_pixel_frame_index: costura CONTINUA indo pra batalha (Darkening "
          "termina onde Revealing comeca)",
          "[boot_pixel_sequence]") {
    const int end_of_darkening = boot_pixel_frame_index(
        BootPixelLeg::kToBattleDarkening, 1.0f, kBootPixelFrameCount);
    const int start_of_revealing = boot_pixel_frame_index(
        BootPixelLeg::kToBattleRevealing, 0.0f, kBootPixelFrameCount);
    // Passo de no MAXIMO 1 frame na costura (continuidade, nao um reset pra 0).
    REQUIRE(start_of_revealing - end_of_darkening <= 1);
    REQUIRE(start_of_revealing - end_of_darkening >= 0);
}

TEST_CASE("boot_pixel_frame_index: costura CONTINUA voltando pra cidade "
          "(FromBattleDarkening termina onde FromBattleRevealing comeca)",
          "[boot_pixel_sequence]") {
    const int end_of_darkening = boot_pixel_frame_index(
        BootPixelLeg::kFromBattleDarkening, 1.0f, kBootPixelFrameCount);
    const int start_of_revealing = boot_pixel_frame_index(
        BootPixelLeg::kFromBattleRevealing, 0.0f, kBootPixelFrameCount);
    REQUIRE(end_of_darkening - start_of_revealing <= 1);
    REQUIRE(end_of_darkening - start_of_revealing >= 0);
}

// --- Monotonia dentro de cada perna (nunca "pula pra tras" dentro da MESMA perna) ---

TEST_CASE("boot_pixel_frame_index: kToBattleDarkening e kToBattleRevealing sao "
          "NAO-DECRESCENTES (o boot so avanca, nunca recua, indo pra batalha)",
          "[boot_pixel_sequence]") {
    for (BootPixelLeg leg :
         {BootPixelLeg::kToBattleDarkening, BootPixelLeg::kToBattleRevealing}) {
        int prev = boot_pixel_frame_index(leg, 0.0f, kBootPixelFrameCount);
        for (float t = 0.0f; t <= 1.0f; t += 0.05f) {
            const int idx = boot_pixel_frame_index(leg, t, kBootPixelFrameCount);
            REQUIRE(idx >= prev);
            prev = idx;
        }
    }
}

TEST_CASE("boot_pixel_frame_index: kFromBattleDarkening e kFromBattleRevealing sao "
          "NAO-CRESCENTES (o 'desligar' so recua, nunca avanca, voltando pra "
          "cidade)",
          "[boot_pixel_sequence]") {
    for (BootPixelLeg leg :
         {BootPixelLeg::kFromBattleDarkening, BootPixelLeg::kFromBattleRevealing}) {
        int prev = boot_pixel_frame_index(leg, 0.0f, kBootPixelFrameCount);
        for (float t = 0.0f; t <= 1.0f; t += 0.05f) {
            const int idx = boot_pixel_frame_index(leg, t, kBootPixelFrameCount);
            REQUIRE(idx <= prev);
            prev = idx;
        }
    }
}

// --- Clamps defensivos ---

TEST_CASE("boot_pixel_frame_index: clampa t fora de [0,1]", "[boot_pixel_sequence]") {
    REQUIRE(boot_pixel_frame_index(BootPixelLeg::kToBattleDarkening, -1.0f,
                                    kBootPixelFrameCount) == 0);
    REQUIRE(boot_pixel_frame_index(BootPixelLeg::kToBattleRevealing, 2.0f,
                                    kBootPixelFrameCount) ==
            kBootPixelFrameCount - 1);
}

TEST_CASE("boot_pixel_frame_index: frame_count<=0 devolve 0 (sem UB/divisao por "
          "zero)",
          "[boot_pixel_sequence]") {
    REQUIRE(boot_pixel_frame_index(BootPixelLeg::kToBattleDarkening, 0.5f, 0) == 0);
    REQUIRE(boot_pixel_frame_index(BootPixelLeg::kFromBattleRevealing, 0.5f, -3) ==
            0);
}

TEST_CASE("boot_pixel_frame_index: indice SEMPRE dentro de [0, frame_count-1]",
          "[boot_pixel_sequence]") {
    for (BootPixelLeg leg : {BootPixelLeg::kToBattleDarkening,
                              BootPixelLeg::kToBattleRevealing,
                              BootPixelLeg::kFromBattleDarkening,
                              BootPixelLeg::kFromBattleRevealing}) {
        for (float t = 0.0f; t <= 1.0f; t += 0.1f) {
            const int idx = boot_pixel_frame_index(leg, t, kBootPixelFrameCount);
            REQUIRE(idx >= 0);
            REQUIRE(idx < kBootPixelFrameCount);
        }
    }
}

// --- boot_pixel_safety_alpha: a camada solida de seguranca escala com t igual o
// fade liso original escalava com o alpha de fade_overlay_alpha (Darkening=t
// crescente, Revealing=1-t decrescente) ---

TEST_CASE("boot_pixel_safety_alpha: Darkening cresce 0->1 com t (escurecendo)",
          "[boot_pixel_sequence]") {
    REQUIRE_THAT(boot_pixel_safety_alpha(BootPixelLeg::kToBattleDarkening, 0.0f),
                 WithinAbs(0.0f, 1e-5f));
    REQUIRE_THAT(boot_pixel_safety_alpha(BootPixelLeg::kToBattleDarkening, 1.0f),
                 WithinAbs(1.0f, 1e-5f));
    REQUIRE_THAT(boot_pixel_safety_alpha(BootPixelLeg::kFromBattleDarkening, 0.5f),
                 WithinAbs(0.5f, 1e-5f));
}

TEST_CASE("boot_pixel_safety_alpha: Revealing decresce 1->0 com t (revelando)",
          "[boot_pixel_sequence]") {
    REQUIRE_THAT(boot_pixel_safety_alpha(BootPixelLeg::kToBattleRevealing, 0.0f),
                 WithinAbs(1.0f, 1e-5f));
    REQUIRE_THAT(boot_pixel_safety_alpha(BootPixelLeg::kToBattleRevealing, 1.0f),
                 WithinAbs(0.0f, 1e-5f));
    REQUIRE_THAT(boot_pixel_safety_alpha(BootPixelLeg::kFromBattleRevealing, 0.5f),
                 WithinAbs(0.5f, 1e-5f));
}

TEST_CASE("boot_pixel_safety_alpha: clampa t fora de [0,1]", "[boot_pixel_sequence]") {
    REQUIRE_THAT(boot_pixel_safety_alpha(BootPixelLeg::kToBattleDarkening, -1.0f),
                 WithinAbs(0.0f, 1e-5f));
    REQUIRE_THAT(boot_pixel_safety_alpha(BootPixelLeg::kToBattleDarkening, 2.0f),
                 WithinAbs(1.0f, 1e-5f));
    REQUIRE_THAT(boot_pixel_safety_alpha(BootPixelLeg::kToBattleRevealing, -1.0f),
                 WithinAbs(1.0f, 1e-5f));
    REQUIRE_THAT(boot_pixel_safety_alpha(BootPixelLeg::kToBattleRevealing, 2.0f),
                 WithinAbs(0.0f, 1e-5f));
}

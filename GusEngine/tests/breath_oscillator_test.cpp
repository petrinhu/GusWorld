// GusEngine/tests/breath_oscillator_test.cpp
//
// Catch2 do POCO core::anim::BreathOscillator: respiracao CALMA procedural - uma
// senoide continua e suave no tempo (sem trocar quadro), dada a cadencia em
// ciclos/min. Devolve um offset normalizado em [-1, 1] que o render escala num
// pequeno bob/scale do sprite parado. TEST-FIRST. Matematica pura (sin), headless.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <cmath>

#include "gus/core/anim/breath_oscillator.hpp"

using Catch::Matchers::WithinAbs;
using gus::core::anim::BreathOscillator;

TEST_CASE("Comeca no fundo do ciclo (offset 0, subindo)", "[breath]") {
    // Fase escolhida pra que t=0 fique no piso da respiracao (expirado): value()==0
    // e o proximo passo SOBE (inspira). Evita um "pulo" ao entrar no idle.
    BreathOscillator b(/*cycles_per_min=*/16.0f);
    REQUIRE_THAT(b.value(), WithinAbs(0.0f, 1e-4f));
}

TEST_CASE("value fica sempre em [-1, 1]", "[breath]") {
    BreathOscillator b(16.0f);
    for (int i = 0; i < 500; ++i) {
        b.advance(0.05f);
        REQUIRE(b.value() >= -1.0f - 1e-4f);
        REQUIRE(b.value() <= 1.0f + 1e-4f);
    }
}

TEST_CASE("Um ciclo inteiro leva 60/cpm segundos e volta ao inicio", "[breath]") {
    const float cpm = 16.0f;
    BreathOscillator b(cpm);
    const float period = 60.0f / cpm;  // ~3.75 s
    // Quarto de ciclo -> pico (inspirado).
    b.advance(period * 0.25f);
    REQUIRE_THAT(b.value(), WithinAbs(1.0f, 1e-3f));
    // Meio ciclo a partir do pico -> volta ao piso (expirado).
    b.advance(period * 0.5f);
    REQUIRE_THAT(b.value(), WithinAbs(-1.0f, 1e-3f));
    // Fecha o ciclo -> de volta ao inicio (0).
    b.advance(period * 0.25f);
    REQUIRE_THAT(b.value(), WithinAbs(0.0f, 1e-3f));
}

TEST_CASE("dt nao-positivo nao avanca a fase", "[breath]") {
    BreathOscillator b(16.0f);
    b.advance(0.5f);
    const float v = b.value();
    b.advance(0.0f);
    b.advance(-1.0f);
    REQUIRE_THAT(b.value(), WithinAbs(v, 1e-5f));
}

TEST_CASE("cycles_per_min <= 0 e saneado (nao trava em NaN/parado)", "[breath]") {
    BreathOscillator b(/*cycles_per_min=*/0.0f);
    b.advance(1.0f);
    REQUIRE(std::isfinite(b.value()));
}

TEST_CASE("set_cycles_per_min muda a cadencia sem saltar a fase", "[breath]") {
    BreathOscillator b(16.0f);
    b.advance(1.0f);
    const float before = b.value();
    b.set_cycles_per_min(32.0f);  // dobra a cadencia
    // Trocar a cadencia nao deve teletransportar o valor corrente.
    REQUIRE_THAT(b.value(), WithinAbs(before, 1e-5f));
}

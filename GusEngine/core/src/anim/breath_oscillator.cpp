// gus/core/src/anim/breath_oscillator.cpp
//
// Implementacao do POCO BreathOscillator. Ver header. Travado por
// GusEngine/tests/breath_oscillator_test.cpp (TEST-FIRST).

#include "gus/core/anim/breath_oscillator.hpp"

#include <cmath>  // std::sin, std::fmod

namespace gus::core::anim {

namespace {

constexpr float kTwoPi = 6.2831853071795864769f;

float sane_cpm(float cpm) noexcept {
    return cpm > BreathOscillator::kMinCyclesPerMin
               ? cpm
               : BreathOscillator::kMinCyclesPerMin;
}

}  // namespace

BreathOscillator::BreathOscillator(float cycles_per_min) noexcept
    : cycles_per_min_(sane_cpm(cycles_per_min)) {}

void BreathOscillator::advance(float dt) noexcept {
    if (dt <= 0.0f) {
        return;
    }
    // Periodo de um ciclo (s) = 60 / cpm. Avanca a fase pela fracao do ciclo.
    const float period = 60.0f / cycles_per_min_;
    phase_ += dt / period;
    // Wrap em [0, 1) sem acumular erro pra tempos longos.
    phase_ -= std::floor(phase_);
}

float BreathOscillator::value() const noexcept {
    return std::sin(phase_ * kTwoPi);
}

void BreathOscillator::set_cycles_per_min(float cycles_per_min) noexcept {
    // So muda a cadencia; a fase (e portanto value()) fica intacta.
    cycles_per_min_ = sane_cpm(cycles_per_min);
}

}  // namespace gus::core::anim

// gus/core/src/anim/anim_clock.cpp
//
// Ver header. Implementacao POCO do relogio de animacao por tempo.

#include "gus/core/anim/anim_clock.hpp"

namespace gus::core::anim {

namespace {

float clamp_fps(float fps) noexcept {
    if (fps < AnimClock::kMinFps) {
        return AnimClock::kMinFps;
    }
    if (fps > AnimClock::kMaxFps) {
        return AnimClock::kMaxFps;
    }
    return fps;
}

int sane_count(int n) noexcept { return n >= 1 ? n : 1; }

}  // namespace

AnimClock::AnimClock(int frame_count, float fps) noexcept
    : frame_count_(sane_count(frame_count)), fps_(clamp_fps(fps)) {}

void AnimClock::advance(float dt) noexcept {
    if (dt <= 0.0f) {
        return;
    }
    accum_ += dt;
    const float step = 1.0f / fps_;  // fps_ >= kMinFps garante step finito
    // Consome quadros enquanto sobrar tempo (suporta dt grande sem engasgar).
    while (accum_ >= step) {
        accum_ -= step;
        ++frame_;
        if (frame_ >= frame_count_) {
            frame_ %= frame_count_;
        }
    }
}

void AnimClock::set_fps(float fps) noexcept { fps_ = clamp_fps(fps); }

void AnimClock::set_frame_count(int frame_count) noexcept {
    frame_count_ = sane_count(frame_count);
    frame_ = 0;
    accum_ = 0.0f;
}

void AnimClock::reset() noexcept {
    frame_ = 0;
    accum_ = 0.0f;
}

}  // namespace gus::core::anim

// gus/core/src/player/winded_timer.cpp
//
// Implementacao do POCO WindedTimer (folego do corpo). Ver header. Travado por
// GusEngine/tests/winded_timer_test.cpp (TEST-FIRST). POCO puro: ZERO SDL/IO/GPU.

#include "gus/core/player/winded_timer.hpp"

namespace gus::core::player {

namespace {

float non_negative(float v) noexcept { return v > 0.0f ? v : 0.0f; }

float clampf(float v, float lo, float hi) noexcept {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

}  // namespace

WindedTimer::WindedTimer(WindedConfig cfg) noexcept {
    // Piso >= 0; teto nunca abaixo do piso (config degenerada vira piso == teto, sem
    // escala). run_for_max > 0 (evita divisao por ~0 ao escalar). Limiar >= 0.
    cfg_.min_winded_seconds = non_negative(cfg.min_winded_seconds);
    cfg_.max_winded_seconds = cfg.max_winded_seconds > cfg_.min_winded_seconds
                                  ? cfg.max_winded_seconds
                                  : cfg_.min_winded_seconds;
    cfg_.run_for_max_winded =
        cfg.run_for_max_winded > 0.0f ? cfg.run_for_max_winded : 1.0f;
    cfg_.run_threshold_seconds = non_negative(cfg.run_threshold_seconds);
}

void WindedTimer::tick_running(float dt) noexcept {
    if (dt <= 0.0f) {
        return;
    }
    // Voltou a correr: limpa o folego ativo (o esforco recomeca; correndo nao ofega).
    if (!was_running_) {
        remaining_ = 0.0f;
        run_acc_ = 0.0f;
    }
    run_acc_ += dt;
    was_running_ = true;
}

void WindedTimer::tick_stopped(float dt) noexcept {
    // TRANSICAO corrida -> parado: avalia o gatilho UMA vez (mesmo com dt == 0, pra a
    // borda nao depender do tamanho do primeiro passo parado).
    if (was_running_) {
        was_running_ = false;
        if (run_acc_ >= cfg_.run_threshold_seconds && run_acc_ > 0.0f) {
            // Escala LINEAR do tempo de corrida -> duracao do folego, saturada no teto.
            const float frac = clampf(run_acc_ / cfg_.run_for_max_winded, 0.0f, 1.0f);
            remaining_ = cfg_.min_winded_seconds +
                         (cfg_.max_winded_seconds - cfg_.min_winded_seconds) * frac;
        } else {
            // Corrida curta (abaixo do limiar): nao ofega.
            remaining_ = 0.0f;
        }
        run_acc_ = 0.0f;  // consumido na avaliacao
    }

    if (dt <= 0.0f) {
        return;
    }
    // Parado com folego ativo: decai ate zerar (clamp em 0).
    if (remaining_ > 0.0f) {
        remaining_ -= dt;
        if (remaining_ < 0.0f) {
            remaining_ = 0.0f;
        }
    }
}

void WindedTimer::reset() noexcept {
    run_acc_ = 0.0f;
    remaining_ = 0.0f;
    was_running_ = false;
}

}  // namespace gus::core::player

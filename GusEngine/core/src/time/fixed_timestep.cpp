// gus/core/src/time/fixed_timestep.cpp
//
// Implementacao do loop de tempo fixo (M1). Ver header pro contrato. Travado por
// tests/fixed_timestep_test.cpp (TEST-FIRST).

#include "gus/core/time/fixed_timestep.hpp"

namespace gus::core::time {

namespace {
// Default coerente quando o construtor recebe um fixed_dt invalido (<= 0).
constexpr double kDefaultDt = 1.0 / 60.0;
}  // namespace

FixedTimestep::FixedTimestep(double fixed_dt, int max_ticks_per_frame) noexcept
    : fixed_dt_(fixed_dt > 0.0 ? fixed_dt : kDefaultDt),
      max_ticks_(max_ticks_per_frame > 0 ? max_ticks_per_frame : 1) {}

FrameSteps FixedTimestep::advance(double frame_dt) noexcept {
    // dt <= 0 (relogio recuado, pausa, primeiro frame degenerado): nao avanca
    // nada. O acumulador fica intacto; o alpha continua valido.
    if (frame_dt > 0.0) {
        accumulator_ += frame_dt;
    }

    FrameSteps out;
    out.ticks = 0;

    // Consome passos fixos enquanto couberem, ate o teto anti spiral-of-death.
    while (accumulator_ >= fixed_dt_ && out.ticks < max_ticks_) {
        accumulator_ -= fixed_dt_;
        ++out.ticks;
    }

    // Clamp: se ainda sobra um passo inteiro, estouramos o teto. DESCARTA o
    // excedente zerando o acumulador, em vez de carregar a divida pro proximo
    // frame (senao a cascata so adia). O jogo desacelera neste instante; o
    // proximo frame normal volta a 1 tick limpo, sem heranca.
    if (accumulator_ >= fixed_dt_) {
        accumulator_ = 0.0;
    }

    // Defesa numerica: arredondamento pode deixar o acumulador minimamente
    // negativo; mantem o invariante accumulator >= 0 (alpha nunca negativo).
    if (accumulator_ < 0.0) {
        accumulator_ = 0.0;
    }

    out.alpha = accumulator_ / fixed_dt_;  // em [0,1) por construcao
    return out;
}

}  // namespace gus::core::time

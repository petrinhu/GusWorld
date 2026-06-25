// gus/app/src/screens/battle_pacing.cpp
//
// Implementacao do diretor de pacing (ver header). Timer + maquina de estados pura.

#include "gus/app/screens/battle_pacing.hpp"

namespace gus::app::screens {

bool PacingDirector::ready_to_step() const noexcept {
    // Esperando o jogador: o tempo nunca libera passo (so player_acted retoma).
    if (state_ == PacingState::WaitingPlayerInput) {
        return false;
    }
    // Intro/delay: liberado quando o timer zerou.
    return timer_ <= 0.0f;
}

void PacingDirector::tick(float dt_seconds) noexcept {
    if (dt_seconds <= 0.0f) {
        return;
    }
    if (state_ == PacingState::WaitingPlayerInput) {
        return;  // a vez do jogador nao avanca por tempo
    }
    timer_ -= dt_seconds;
    if (timer_ < 0.0f) {
        timer_ = 0.0f;
    }
}

void PacingDirector::skip() noexcept {
    // Acelera intro/delay; NAO pula o turno do jogador.
    if (state_ == PacingState::WaitingPlayerInput) {
        return;
    }
    timer_ = 0.0f;
}

void PacingDirector::begin_enemy_step() noexcept {
    state_ = PacingState::WaitingDelay;
    timer_ = kPacingStepDelaySeconds;
}

void PacingDirector::begin_player_turn() noexcept {
    state_ = PacingState::WaitingPlayerInput;
    timer_ = 0.0f;  // sem timer: espera o menu
}

void PacingDirector::player_acted() noexcept {
    state_ = PacingState::WaitingDelay;
    timer_ = kPacingStepDelaySeconds;
}

}  // namespace gus::app::screens

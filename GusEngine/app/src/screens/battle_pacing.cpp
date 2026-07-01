// gus/app/src/screens/battle_pacing.cpp
//
// Implementacao do diretor de pacing (ver header). Timer + maquina de estados pura.

#include "gus/app/screens/battle_pacing.hpp"

namespace gus::app::screens {

bool PacingDirector::ready_to_step() const noexcept {
    // Estados que esperam INPUT (nao tempo) nunca liberam passo sozinhos:
    //   Intro              -> espera ENCARAR (begin_combat);
    //   WaitingPlayerInput -> espera o menu (player_acted).
    if (state_ == PacingState::Intro ||
        state_ == PacingState::WaitingPlayerInput) {
        return false;
    }
    // Estados timed (anuncio/delay): liberado quando o timer zerou.
    return timer_ <= 0.0f;
}

void PacingDirector::tick(float dt_seconds) noexcept {
    if (dt_seconds <= 0.0f) {
        return;
    }
    // Intro e vez-do-jogador esperam INPUT: o tempo nao avanca a abertura nem a vez.
    if (state_ == PacingState::Intro ||
        state_ == PacingState::WaitingPlayerInput) {
        return;
    }
    timer_ -= dt_seconds;
    if (timer_ < 0.0f) {
        timer_ = 0.0f;
    }
}

void PacingDirector::skip() noexcept {
    // Acelera APENAS a pausa de LEITURA pos-resolucao (WaitingDelay). NAO afeta:
    //   - Intro (espera Encarar) e WaitingPlayerInput (espera o menu): saem por input;
    //   - AnnouncingEnemy (BEAT 1): o anuncio "Vez de <nome>" SEMPRE toca seu tempo
    //     proprio. BUG (lider no display): apertar a tecla durante o anuncio colapsava o
    //     anuncio em 1 frame -> o ataque do inimigo seguinte saia "colado" (impressao de
    //     ataque duplo). O anuncio e o beat onde a animacao de ataque vai morar; ele NAO
    //     pode ser pulado, senao o golpe perde o windup. So a pausa de leitura acelera.
    if (state_ != PacingState::WaitingDelay) {
        return;
    }
    timer_ = 0.0f;
}

void PacingDirector::begin_combat() noexcept {
    // ENCARAR: sai da abertura PARADA. Vai pra WaitingDelay com o MESMO delay dos demais
    // turnos (kPacingStepDelaySeconds) - um RESPIRO INICIAL antes do 1o anuncio.
    //
    // FIX W1 (lider: "o 1o ataque resolve rapido demais / ja comecei apanhando"): antes o
    // timer saia 0 (liberado JA) -> o 1o turno de inimigo ANUNCIAVA no frame seguinte e o
    // golpe conectava ~0.7s (so o anuncio) apos Encarar. TODO turno subsequente, porem, tem
    // ANTES do anuncio a pausa pos-resolucao do turno anterior (kPacingStepDelaySeconds) =>
    // ~1.5s de respiro antes do golpe. O 1o turno era o UNICO sem esse respiro de entrada.
    // Dando a ele o mesmo delay, o 1o ataque inimigo passa pelo MESMO ritmo visivel
    // (respiro -> anuncio -> resolucao) dos demais. skip() zera este respiro (impaciente).
    // Valor: reusa kPacingStepDelaySeconds (ja usado entre turnos); nao inventa constante.
    if (state_ != PacingState::Intro) {
        return;
    }
    state_ = PacingState::WaitingDelay;
    timer_ = kPacingStepDelaySeconds;
}

void PacingDirector::begin_enemy_announce() noexcept {
    // BEAT 1: anuncia o turno de inimigo. Estado timed (como Intro/WaitingDelay):
    // ready_to_step/tick/skip ja o tratam (so WaitingPlayerInput e especial).
    state_ = PacingState::AnnouncingEnemy;
    timer_ = kPacingAnnounceSeconds;
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

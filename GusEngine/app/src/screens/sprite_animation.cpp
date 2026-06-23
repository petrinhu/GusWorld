// gus/app/src/screens/sprite_animation.cpp
//
// Implementacao da logica de animacao de locomocao (M1+). Ver header. Travada por
// app/tests/sprite_animation_test.cpp (TEST-FIRST). POCO puro: sem Qt/GPU/I/O.

#include "gus/app/screens/sprite_animation.hpp"

namespace gus::app::screens {

Direction direction_from_move(int dx, int dy, Direction prev) noexcept {
    if (dx == 0 && dy == 0) {
        return prev;  // parado: idle nao gira o boneco, mantem a ultima direcao
    }
    // DIAGONAL: o horizontal vence (criterio documentado no header). Como dx aqui
    // ja basta para isso, basta checar dx primeiro: qualquer dx != 0 manda.
    if (dx > 0) {
        return Direction::East;
    }
    if (dx < 0) {
        return Direction::West;
    }
    // dx == 0 aqui: movimento puramente vertical.
    return (dy > 0) ? Direction::South : Direction::North;
}

void WalkCycle::reset() noexcept {
    accum_ = 0.0f;
    frame_ = 0;
    moving_ = false;
}

void WalkCycle::advance(float distance, bool running) noexcept {
    if (distance <= 0.0f) {
        reset();  // parado: volta ao neutro (idle)
        return;
    }

    moving_ = true;
    accum_ += distance;

    // Passo de troca: mais LONGO correndo (passada comprida, nao fps maior).
    const float step = running ? cfg_.run_px_per_frame : cfg_.walk_px_per_frame;
    if (step <= 0.0f) {
        return;  // config degenerada: nao divide por zero, fica no quadro atual
    }

    // Consome o acumulado em passos de "step", avancando o quadro ciclico a cada
    // troca. Loop (em vez de modulo direto) para suportar dist > step num unico
    // advance sem pular a contagem de quadros.
    while (accum_ >= step) {
        accum_ -= step;
        frame_ = (frame_ + 1) % kWalkFrameCount;
    }
}

}  // namespace gus::app::screens

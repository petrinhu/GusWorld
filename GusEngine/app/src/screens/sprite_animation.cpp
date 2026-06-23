// gus/app/src/screens/sprite_animation.cpp
//
// Implementacao da logica de animacao de locomocao (M1+). Ver header. Travada por
// app/tests/sprite_animation_test.cpp (TEST-FIRST). POCO puro: sem Qt/GPU/I/O.

#include "gus/app/screens/sprite_animation.hpp"

namespace gus::app::screens {

namespace {

// Direcao do componente horizontal (assume dx != 0).
Direction horizontal_dir(int dx) noexcept {
    return (dx > 0) ? Direction::East : Direction::West;
}
// Direcao do componente vertical (assume dy != 0; +Y = baixo = Sul).
Direction vertical_dir(int dy) noexcept {
    return (dy > 0) ? Direction::South : Direction::North;
}

bool is_horizontal(Direction d) noexcept {
    return d == Direction::East || d == Direction::West;
}

bool is_vertical(Direction d) noexcept {
    return d == Direction::North || d == Direction::South;
}

}  // namespace

Direction direction_from_move(int dx, int dy, Direction prev) noexcept {
    // Sobrecarga legada: horizontal vence na diagonal (comportamento do M1).
    return direction_from_move(dx, dy, prev, DiagonalFacing::HorizontalWins);
}

Direction direction_from_move(int dx, int dy, Direction prev,
                              DiagonalFacing policy) noexcept {
    // Sobrecarga SEM memoria de input: assume "input do tick anterior == 0", o que
    // mantem o comportamento antigo de VerticalWins/HorizontalWins (que nao olham a
    // memoria) e a aproximacao via facing do LastAxisWins (que OSCILA na diagonal
    // sustentada). Pra anti-flicker, use a sobrecarga de 6 args.
    if (policy == DiagonalFacing::LastAxisWins && dx != 0 && dy != 0) {
        // Preserva a regra legada desta sobrecarga: eixo novo derivado de prev.
        return is_horizontal(prev) ? vertical_dir(dy) : horizontal_dir(dx);
    }
    return direction_from_move(dx, dy, /*dx_prev*/ 0, /*dy_prev*/ 0, prev, policy);
}

Direction direction_from_move(int dx, int dy, int dx_prev, int dy_prev, Direction prev,
                              DiagonalFacing policy) noexcept {
    if (dx == 0 && dy == 0) {
        return prev;  // parado: idle nao gira o boneco, mantem a ultima direcao
    }
    // Cardinal puro: a propria direcao, independente da politica.
    if (dy == 0) {
        return horizontal_dir(dx);
    }
    if (dx == 0) {
        return vertical_dir(dy);
    }

    // DIAGONAL (dx != 0 E dy != 0): resolve pela politica.
    switch (policy) {
        case DiagonalFacing::VerticalWins:
            return vertical_dir(dy);

        case DiagonalFacing::LastAxisWins: {
            // ANTI-FLICKER: o eixo recem-acionado e decidido pela MEMORIA DO INPUT
            // (dx_prev,dy_prev), nao pelo facing anterior. Como o sim realimenta prev
            // com o proprio resultado a cada tick, derivar do facing fazia o boneco
            // oscilar (East->North->East...) numa diagonal sustentada.
            const bool vertical_new = (dy != 0 && dy_prev == 0);
            const bool horizontal_new = (dx != 0 && dx_prev == 0);
            if (vertical_new && !horizontal_new) {
                return vertical_dir(dy);  // adicionou W/S sobre um movimento lateral
            }
            if (horizontal_new && !vertical_new) {
                return horizontal_dir(dx);  // adicionou A/D sobre um movimento vertical
            }
            // Ambos sustentados (nenhum recem) OU ambos recem no mesmo tick: fica
            // ESTAVEL no prev se ele ja for um dos eixos desta diagonal; senao cai no
            // vertical da diagonal (fallback deterministico).
            if ((is_horizontal(prev) && horizontal_dir(dx) == prev) ||
                (is_vertical(prev) && vertical_dir(dy) == prev)) {
                return prev;
            }
            return vertical_dir(dy);
        }

        case DiagonalFacing::HorizontalWins:
        default:
            return horizontal_dir(dx);
    }
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
        frame_ = (frame_ + 1) % frame_count_;
    }
}

}  // namespace gus::app::screens

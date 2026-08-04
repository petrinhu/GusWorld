// SPDX-License-Identifier: Apache-2.0
// gus/domain/world/patrol_route.cpp
//
// Implementação da ronda em rota fixa (ida e volta). Ver o header para a decisão
// do líder e o contrato. Travado por domain/tests/patrol_route_test.cpp
// (TEST-FIRST).

#include "gus/domain/world/patrol_route.hpp"

#include <cmath>  // std::sqrt

namespace gus::domain::world {

namespace {

// Teto de pernas consumidas numa única chamada. Existe por causa de dois casos
// reais: dt gigante (a janela ficou minimizada e o relógio acumulou) e perna de
// comprimento zero (dois pontos iguais na tabela, erro de autoria plausível). Sem
// o teto, o segundo caso é laço infinito dentro do passo fixo, ou seja, jogo
// travado. Com o teto, o pior caso é a ronda ficar num ponto qualquer DENTRO da
// própria rota - degradação visível e inofensiva.
constexpr int kMaxLegsPerAdvance = 64;

constexpr int clamp_index(int i, int count) noexcept {
    if (i < 0) return 0;
    if (i >= count) return count - 1;
    return i;
}

constexpr float clamp01(float t) noexcept {
    if (t < 0.0f) return 0.0f;
    if (t > 1.0f) return 1.0f;
    return t;
}

float leg_length(const PatrolRoute& route, int from, int to) noexcept {
    const float dx = route.waypoints[to].x - route.waypoints[from].x;
    const float dy = route.waypoints[to].y - route.waypoints[from].y;
    return std::sqrt(dx * dx + dy * dy);
}

// Reancora índices corrompidos (dado de save antigo, cast inválido, estado
// zerado à mão) DENTRO da rota, e garante from != to - senão a perna teria
// comprimento zero por motivo errado e a ronda pararia sem explicação.
void sanitize(const PatrolRoute& route, PatrolState& state) noexcept {
    state.from_index = clamp_index(state.from_index, route.count);
    state.to_index = clamp_index(state.to_index, route.count);
    if (state.from_index == state.to_index) {
        state.to_index = state.forward
                             ? clamp_index(state.from_index + 1, route.count)
                             : clamp_index(state.from_index - 1, route.count);
        if (state.from_index == state.to_index) {
            // Rota de 1 ponto útil: nada a fazer além de não andar.
            state.traveled_tiles = 0.0f;
        }
    }
    if (state.traveled_tiles < 0.0f) {
        state.traveled_tiles = 0.0f;
    }
    if (state.pause_left < 0.0f) {
        state.pause_left = 0.0f;
    }
}

// IDA E VOLTA: chegou na ponta, inverte o sentido; no meio, segue para o próximo.
// Nunca teleporta de volta ao início (isso seria rota cíclica).
void step_to_next_leg(const PatrolRoute& route, PatrolState& state) noexcept {
    const int last = route.count - 1;
    if (state.forward) {
        if (state.to_index >= last) {
            state.forward = false;
            state.from_index = last;
            state.to_index = last - 1;
        } else {
            state.from_index = state.to_index;
            state.to_index = state.to_index + 1;
        }
    } else {
        if (state.to_index <= 0) {
            state.forward = true;
            state.from_index = 0;
            state.to_index = 1;
        } else {
            state.from_index = state.to_index;
            state.to_index = state.to_index - 1;
        }
    }
}

PatrolSample sample_unchecked(const PatrolRoute& route,
                              const PatrolState& state) noexcept {
    const PatrolWaypoint& a = route.waypoints[state.from_index];
    const PatrolWaypoint& b = route.waypoints[state.to_index];
    const float len = leg_length(route, state.from_index, state.to_index);
    const float t = len > 0.0f ? clamp01(state.traveled_tiles / len) : 0.0f;
    PatrolSample out;
    out.x = a.x + (b.x - a.x) * t;
    out.y = a.y + (b.y - a.y) * t;
    out.paused = state.pause_left > 0.0f;
    return out;
}

// Ator sem ronda: fica no primeiro ponto declarado (ou na origem, se nem isso).
PatrolSample stationary(const PatrolRoute& route) noexcept {
    PatrolSample out;
    if (route.count >= 1) {
        out.x = route.waypoints[0].x;
        out.y = route.waypoints[0].y;
    }
    out.paused = true;
    return out;
}

}  // namespace

PatrolSample advance_patrol(const PatrolRoute& route, PatrolState& state,
                            float dt) noexcept {
    if (!route.valid()) {
        return stationary(route);
    }
    sanitize(route, state);
    if (dt <= 0.0f) {
        return sample_unchecked(route, state);
    }

    float remaining = dt;
    for (int i = 0; i < kMaxLegsPerAdvance && remaining > 0.0f; ++i) {
        // A pausa consome TEMPO, não distância: chegar num ponto com sobra de dt
        // gasta essa sobra parado, e só o que sobrar dela vira marcha.
        if (state.pause_left > 0.0f) {
            const float spent =
                state.pause_left < remaining ? state.pause_left : remaining;
            state.pause_left -= spent;
            remaining -= spent;
            if (state.pause_left > 0.0f) {
                break;  // a pausa engoliu o passo inteiro
            }
            continue;
        }

        const float len = leg_length(route, state.from_index, state.to_index);
        const float missing_tiles = len - state.traveled_tiles;
        const float seconds_to_finish =
            missing_tiles > 0.0f ? missing_tiles / route.speed_tiles_per_sec : 0.0f;

        if (remaining < seconds_to_finish) {
            state.traveled_tiles += remaining * route.speed_tiles_per_sec;
            remaining = 0.0f;
            break;
        }

        // Chegou no ponto: zera a perna, vira para a próxima e arma a pausa.
        remaining -= seconds_to_finish;
        state.traveled_tiles = 0.0f;
        step_to_next_leg(route, state);
        state.pause_left = route.pause_seconds;
    }

    return sample_unchecked(route, state);
}

PatrolSample sample_patrol(const PatrolRoute& route,
                           const PatrolState& state) noexcept {
    if (!route.valid()) {
        return stationary(route);
    }
    PatrolState copy = state;  // sanitize sem tocar no estado do chamador
    sanitize(route, copy);
    return sample_unchecked(route, copy);
}

}  // namespace gus::domain::world

// gus/core/src/player/stamina.cpp
//
// Implementacao do POCO Stamina. Ver header. Travado por
// GusEngine/tests/stamina_test.cpp (TEST-FIRST).

#include "gus/core/player/stamina.hpp"

namespace gus::core::player {

namespace {

// Saneia uma taxa >= 0 (negativa vira 0). A direcao (drenar/recuperar) vem do flag,
// nunca do sinal da taxa.
float non_negative(float v) noexcept { return v > 0.0f ? v : 0.0f; }

// Restringe v ao intervalo [lo, hi].
float clampf(float v, float lo, float hi) noexcept {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

}  // namespace

Stamina::Stamina(StaminaConfig cfg) noexcept {
    cfg_.max = cfg.max > 0.0f ? cfg.max : 1.0f;
    cfg_.drain_per_sec = non_negative(cfg.drain_per_sec);
    cfg_.recover_walk_per_sec = non_negative(cfg.recover_walk_per_sec);
    cfg_.recover_idle_per_sec = non_negative(cfg.recover_idle_per_sec);
    cfg_.tired_value = clampf(cfg.tired_value, 0.0f, cfg_.max);
    value_ = cfg_.max;  // comeca cheio (descansado).
}

void Stamina::tick(MoveState state, float dt) noexcept {
    if (dt <= 0.0f) {
        return;
    }
    // Seam de 3 estados: a taxa efetiva vem do MoveState (correr drena; andar regenera
    // devagar; parar regenera rapido). Ver docs/design/mecanicas/stamina.md.
    float delta = 0.0f;
    switch (state) {
        case MoveState::Running:
            delta = -cfg_.drain_per_sec * dt;
            break;
        case MoveState::Walking:
            delta = cfg_.recover_walk_per_sec * dt;
            break;
        case MoveState::Idle:
            delta = cfg_.recover_idle_per_sec * dt;
            break;
    }
    value_ = clampf(value_ + delta, 0.0f, cfg_.max);
}

}  // namespace gus::core::player

// gus/app/src/screens/battle_anim.cpp
//
// Implementacao do BattleAnimDirector (ver header). Curvas:
//   - Approach/Return: smoothstep (ease-in-out) entre from e to - acelera e freia,
//     vendendo "corrida ate o alvo" sem biblioteca de easing;
//   - HitReact: seno 0->1->0 (pico no meio) - tranco que recua e volta sozinho;
//   - CastWindup: bob vertical sutil em seno (conjura NO LUGAR, par.2.1).
// Toda fase TERMINA cravando o offset final EXATO (to / zero): sem drift acumulado
// (checklist par.6: "volta a posicao de repouso, sem ficar deslocado").

#include "gus/app/screens/battle_anim.hpp"

#include <cmath>
#include <utility>

namespace gus::app::screens {

namespace {

using gus::core::spatial::Vec2;

constexpr float kPi = 3.14159265358979323846f;

// Progresso 0..1 clampado da fase.
float progress(float elapsed, float duration) noexcept {
    if (duration <= 0.0f) {
        return 1.0f;
    }
    const float t = elapsed / duration;
    return t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t);
}

// smoothstep classico (ease-in-out): 3t^2 - 2t^3.
float smoothstep01(float t) noexcept { return t * t * (3.0f - 2.0f * t); }

}  // namespace

Vec2 AnimProjectile::position() const noexcept {
    const float t = progress(elapsed, duration);
    return Vec2{x0 + (x1 - x0) * t, y0 + (y1 - y0) * t};
}

void BattleAnimDirector::update(float dt_seconds) {
    if (dt_seconds <= 0.0f) {
        return;
    }
    // Anims por ator: avanca; fase terminada TRANSICIONA (Approach -> Hold) ou
    // TERMINA (Return/HitReact/CastWindup -> remove = repouso exato).
    for (auto it = anims_.begin(); it != anims_.end();) {
        ActorAnim& a = it->second;
        a.elapsed += dt_seconds;
        bool erase = false;
        if (a.elapsed >= a.duration) {
            switch (a.kind) {
                case ActorAnimKind::MeleeApproach:
                    // Chegou: crava o offset no alvo e segura (Hold) ate o consumidor
                    // resolver o contato e pedir o Return.
                    a.kind = ActorAnimKind::MeleeHold;
                    a.from = a.to;
                    a.elapsed = 0.0f;
                    a.duration = 0.0f;
                    break;
                case ActorAnimKind::MeleeHold:
                    break;  // segura indefinido (duration 0; espera begin_melee_return)
                case ActorAnimKind::MeleeReturn:
                case ActorAnimKind::CastWindup:
                case ActorAnimKind::HitReact:
                case ActorAnimKind::None:
                    erase = true;  // terminou: repouso EXATO (offset volta a zero)
                    break;
            }
        }
        it = erase ? anims_.erase(it) : std::next(it);
    }

    // Projeteis: avanca; os que chegam reportam o impacto e saem do voo.
    for (auto it = projectiles_.begin(); it != projectiles_.end();) {
        it->elapsed += dt_seconds;
        if (it->elapsed >= it->duration) {
            impacts_.push_back(it->target_id);
            it = projectiles_.erase(it);
        } else {
            ++it;
        }
    }
}

void BattleAnimDirector::start_melee_approach(const std::string& id,
                                              Vec2 displacement, float seconds) {
    ActorAnim a;
    a.kind = ActorAnimKind::MeleeApproach;
    a.elapsed = 0.0f;
    a.duration = seconds;
    a.from = offset_for(id);  // parte de onde esta (robusto a anim substituida)
    a.to = displacement;
    anims_[id] = a;
}

bool BattleAnimDirector::melee_arrived(const std::string& id) const {
    return kind_for(id) == ActorAnimKind::MeleeHold;
}

bool BattleAnimDirector::melee_in_flight(const std::string& id) const {
    const ActorAnimKind k = kind_for(id);
    return k == ActorAnimKind::MeleeApproach || k == ActorAnimKind::MeleeHold;
}

void BattleAnimDirector::begin_melee_return(const std::string& id, float seconds) {
    if (!melee_in_flight(id)) {
        return;  // NO-OP fora do caminho de melee (nao interrompe hit-react/cast)
    }
    ActorAnim a;
    a.kind = ActorAnimKind::MeleeReturn;
    a.elapsed = 0.0f;
    a.duration = seconds;
    a.from = offset_for(id);  // volta de ONDE esta (mesmo se o approach nao terminou)
    a.to = Vec2{0.0f, 0.0f};
    anims_[id] = a;
}

void BattleAnimDirector::start_hit_react(const std::string& id, float dir_x,
                                         float seconds, float knock_px) {
    ActorAnim a;
    a.kind = ActorAnimKind::HitReact;
    a.elapsed = 0.0f;
    a.duration = seconds;
    a.knock_dir_x = dir_x;
    a.knock_px = knock_px;
    anims_[id] = a;
}

void BattleAnimDirector::start_cast(const std::string& id, float seconds) {
    ActorAnim a;
    a.kind = ActorAnimKind::CastWindup;
    a.elapsed = 0.0f;
    a.duration = seconds;
    anims_[id] = a;
}

bool BattleAnimDirector::is_casting(const std::string& id) const {
    return kind_for(id) == ActorAnimKind::CastWindup;
}

void BattleAnimDirector::spawn_projectile(const std::string& target_id, float x0,
                                          float y0, float x1, float y1,
                                          float seconds) {
    AnimProjectile p;
    p.target_id = target_id;
    p.x0 = x0;
    p.y0 = y0;
    p.x1 = x1;
    p.y1 = y1;
    p.elapsed = 0.0f;
    p.duration = seconds;
    projectiles_.push_back(std::move(p));
}

std::vector<std::string> BattleAnimDirector::take_impacts() {
    std::vector<std::string> out;
    out.swap(impacts_);
    return out;
}

Vec2 BattleAnimDirector::offset_for(const std::string& id) const {
    const auto it = anims_.find(id);
    if (it == anims_.end()) {
        return Vec2{0.0f, 0.0f};
    }
    const ActorAnim& a = it->second;
    const float t = progress(a.elapsed, a.duration);
    switch (a.kind) {
        case ActorAnimKind::MeleeApproach:
        case ActorAnimKind::MeleeReturn: {
            const float s = smoothstep01(t);
            return Vec2{a.from.x + (a.to.x - a.from.x) * s,
                        a.from.y + (a.to.y - a.from.y) * s};
        }
        case ActorAnimKind::MeleeHold:
            return a.from;  // cravado no alvo (from == to pos-approach)
        case ActorAnimKind::HitReact: {
            const float k = std::sin(kPi * t);  // 0 -> 1 -> 0
            return Vec2{a.knock_dir_x * a.knock_px * k, 0.0f};
        }
        case ActorAnimKind::CastWindup: {
            // Bob sutil pra CIMA (eixo +Y pra baixo => negativo), sem deslocar em x.
            const float k = std::sin(kPi * t);
            return Vec2{0.0f, -kCastBobPx * k};
        }
        case ActorAnimKind::None:
            break;
    }
    return Vec2{0.0f, 0.0f};
}

ActorAnimKind BattleAnimDirector::kind_for(const std::string& id) const {
    const auto it = anims_.find(id);
    return it == anims_.end() ? ActorAnimKind::None : it->second.kind;
}

float BattleAnimDirector::phase_remaining_seconds(const std::string& id) const {
    const auto it = anims_.find(id);
    if (it == anims_.end()) {
        return 0.0f;  // repouso/desconhecido
    }
    const float rem = it->second.duration - it->second.elapsed;
    return rem > 0.0f ? rem : 0.0f;  // Hold (duration 0) e fase vencida clampam em 0
}

}  // namespace gus::app::screens

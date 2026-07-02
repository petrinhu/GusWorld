// gus/app/src/screens/battle_sprite_anim.cpp
//
// Implementacao do POCO de sprite animado da arena (ver header). Tabela de cadencia
// (fps) por clip - valores escolhidos PROPORCIONAIS aos beats existentes (approach
// 0.7s / return 0.4s / hit-react 0.3s / cast 0.5s), documentados aqui e ajustaveis
// em playtest:
//
//   clip           frames  fps   loop  racional
//   battle_idle       7      8   sim   respiracao calma (~0.9s/ciclo)
//   run               7     14   sim   cadencia de corrida (~0.5s/ciclo; a ida de
//                                      0.42s mostra ~1 ciclo, a volta de 0.4s idem)
//   attack_melee      7     25   nao   7/25 = 0.28s = kMeleeSwingSeconds (o swing
//                                      comeca na cauda do Approach e crava no contato)
//   cast              7     14   nao   7/14 = 0.5s = kCastWindupSeconds (dormante)
//   defend            5     12   nao   dormante
//   hurt_physical     5     16   nao   5/16 = 0.31s ~ kHitReactSeconds (0.3s)
//   hurt_magic        5     16   nao   dormante (espelha o physical)
//   ko                7     10   nao   dormante (a cena so desenha vivos hoje)
//   revive            7     10   nao   dormante
//   victory           7     10   sim   dormante (tela de resultado)
//   dragon_victory    9     10   sim   dormante (mecanica epica)
//   breathing_idle    5      6   sim   dormante (idle alternativo)
//   run_east          9     18   sim   corrida de PERFIL (ida; ~0.5s/ciclo, mesma
//                                      sensacao do run front 7@14; a ida de 0.42s
//                                      mostra ~0.85 ciclo)
//   run_west          9     18   sim   corrida de perfil-esquerda (volta de 0.4s)
//   attack_melee_east 6*    25   nao   murro de perfil; *frame cap 6 (f6-f8 do disco
//                                      DERIVAM e nao carregam). 6/25 = 0.24s <=
//                                      kMeleeSwingSeconds (0.28s): o pico f5 crava
//                                      ~0.04s ANTES do contato e segura (decisao
//                                      "estica-e-crava", ver header)

#include "gus/app/screens/battle_sprite_anim.hpp"

namespace gus::app::screens {

std::string_view clip_dir_name(BattleClipId id) noexcept {
    switch (id) {
        case BattleClipId::Idle:           return "battle_idle";
        case BattleClipId::Run:            return "run";
        case BattleClipId::AttackMelee:    return "attack_melee";
        case BattleClipId::Cast:           return "cast";
        case BattleClipId::Defend:         return "defend";
        case BattleClipId::HurtPhysical:   return "hurt_physical";
        case BattleClipId::HurtMagic:      return "hurt_magic";
        case BattleClipId::KO:             return "ko";
        case BattleClipId::Revive:         return "revive";
        case BattleClipId::Victory:        return "victory";
        case BattleClipId::DragonVictory:  return "dragon_victory";
        case BattleClipId::BreathingIdle:  return "breathing_idle";
        case BattleClipId::RunEast:        return "run_east";
        case BattleClipId::RunWest:        return "run_west";
        case BattleClipId::AttackMeleeEast: return "attack_melee_east";
        case BattleClipId::Count:          break;
    }
    return "battle_idle";  // fallback seguro (nunca vazio)
}

float default_clip_fps(BattleClipId id) noexcept {
    switch (id) {
        case BattleClipId::Idle:           return 8.0f;
        case BattleClipId::Run:            return 14.0f;
        case BattleClipId::AttackMelee:    return 25.0f;
        case BattleClipId::Cast:           return 14.0f;
        case BattleClipId::Defend:         return 12.0f;
        case BattleClipId::HurtPhysical:   return 16.0f;
        case BattleClipId::HurtMagic:      return 16.0f;
        case BattleClipId::KO:             return 10.0f;
        case BattleClipId::Revive:         return 10.0f;
        case BattleClipId::Victory:        return 10.0f;
        case BattleClipId::DragonVictory:  return 10.0f;
        case BattleClipId::BreathingIdle:  return 6.0f;
        case BattleClipId::RunEast:        return 18.0f;
        case BattleClipId::RunWest:        return 18.0f;
        case BattleClipId::AttackMeleeEast: return 25.0f;
        case BattleClipId::Count:          break;
    }
    return 10.0f;
}

bool default_clip_loop(BattleClipId id) noexcept {
    switch (id) {
        case BattleClipId::Idle:
        case BattleClipId::Run:
        case BattleClipId::RunEast:
        case BattleClipId::RunWest:
        case BattleClipId::Victory:
        case BattleClipId::DragonVictory:
        case BattleClipId::BreathingIdle:
            return true;  // ciclos continuos
        case BattleClipId::AttackMelee:
        case BattleClipId::AttackMeleeEast:
        case BattleClipId::Cast:
        case BattleClipId::Defend:
        case BattleClipId::HurtPhysical:
        case BattleClipId::HurtMagic:
        case BattleClipId::KO:
        case BattleClipId::Revive:
        case BattleClipId::Count:
            break;
    }
    return false;  // one-shot: trava no ultimo frame
}

int clip_frame_cap(BattleClipId id) noexcept {
    // So o murro de perfil capa: f6-f8 do disco DERIVAM (o Gus gira pra camera) e
    // NAO carregam - o pico util e f5 (soco esticado). Ver header (timing).
    return id == BattleClipId::AttackMeleeEast ? 6 : 0;
}

BattleClipId clip_fallback(BattleClipId id) noexcept {
    switch (id) {
        case BattleClipId::RunEast:
        case BattleClipId::RunWest:
            return BattleClipId::Run;  // perfil ausente: corrida front-facing
        case BattleClipId::AttackMeleeEast:
            return BattleClipId::AttackMelee;  // murro front-facing
        default:
            return BattleClipId::Idle;  // ultimo degrau (melhor idle vivo que buraco)
    }
}

const SpriteClip* ActorSpriteSet::find(BattleClipId id) const noexcept {
    const int i = static_cast<int>(id);
    if (i < 0 || i >= kBattleClipCount) {
        return nullptr;
    }
    const SpriteClip& c = clips[static_cast<std::size_t>(i)];
    return c.valid() ? &c : nullptr;
}

bool ActorSpriteSet::any() const noexcept {
    for (const SpriteClip& c : clips) {
        if (c.valid()) {
            return true;
        }
    }
    return false;
}

int clip_frame_index(int frame_count, float fps, bool loop,
                     float elapsed_seconds) noexcept {
    if (frame_count <= 1 || fps <= 0.0f || elapsed_seconds <= 0.0f) {
        return 0;  // degenerado/estatico: frame 0 (seguro)
    }
    const int steps = static_cast<int>(elapsed_seconds * fps);
    if (loop) {
        return steps % frame_count;  // da a volta
    }
    return steps >= frame_count ? frame_count - 1 : steps;  // trava no ultimo
}

BattleClipId clip_for_kind(ActorAnimKind kind, float remaining_seconds,
                           float swing_seconds) noexcept {
    switch (kind) {
        case ActorAnimKind::MeleeApproach:
            // Correndo de PERFIL ate o alvo (run_east: encarando o inimigo); na
            // CAUDA (restam <= swing) o murro de perfil comeca, pra CRAVAR no pico
            // f5 pouco antes do contato (decisao "estica-e-crava", ver header).
            return remaining_seconds <= swing_seconds
                       ? BattleClipId::AttackMeleeEast
                       : BattleClipId::RunEast;
        case ActorAnimKind::MeleeHold:
            return BattleClipId::AttackMeleeEast;  // cravado no pico (clampa em f5)
        case ActorAnimKind::MeleeReturn:
            return BattleClipId::RunWest;  // volta de perfil-esquerda (loop)
        case ActorAnimKind::HitReact:
            return BattleClipId::HurtPhysical;  // tranco de golpe fisico
        case ActorAnimKind::CastWindup:
            return BattleClipId::Cast;  // conjura no lugar (dormante hoje)
        case ActorAnimKind::None:
            break;
    }
    return BattleClipId::Idle;  // repouso
}

void BattleSpriteAnimator::tick(const std::string& id, BattleClipId clip,
                                float dt_seconds) {
    auto [it, inserted] = state_.try_emplace(id);
    Playback& pb = it->second;
    if (inserted || pb.clip != clip) {
        // Troca de clip: reseta o relogio (o novo clip mostra o frame 0 inteiro;
        // o dt do frame da troca nao conta).
        pb.clip = clip;
        pb.elapsed = 0.0f;
        return;
    }
    if (dt_seconds > 0.0f) {
        pb.elapsed += dt_seconds;
    }
}

BattleSpriteAnimator::Playback BattleSpriteAnimator::playback_for(
    const std::string& id) const {
    const auto it = state_.find(id);
    return it == state_.end() ? Playback{} : it->second;
}

}  // namespace gus::app::screens

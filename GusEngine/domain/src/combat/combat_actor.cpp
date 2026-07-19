// gus/domain/combat/combat_actor.cpp
//
// Implementacao do CombatActor (combatente mutavel). Portado de
// engine/foundation/turn_combat/CombatActor.cs, paridade de comportamento 1:1.
// POCO puro, ZERO Qt, ZERO I/O.
//
// Cross-ref: engine/foundation/turn_combat/CombatActor.cs;
//            docs/design/mecanicas/combat.md secao 5/6/9/16/17; ADR-006.

#include "gus/domain/combat/combat_actor.hpp"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <utility>

#include "gus/domain/combat/combat_constants.hpp"

namespace gus::domain::combat {

namespace {

// Classificacao buff (benefico) x nao-buff pro dispel do Decrypt (secao 9). RATIFICADO
// (criador 2026-06-03): Decrypt dispela QUALQUER status benefico, incluindo futuros. O
// enum StatusId e plano (sem flag de benefico), entao invertemos a logica: enumeramos
// explicitamente os NAO-BUFFS (debuffs + neutros + o proprio Decrypt) e tudo o que
// sobra e tratado como buff. Assim, um buff novo cai automaticamente em is_buff sem
// editar esta lista; so debuffs novos precisam entrar aqui.
// Debuffs: Stun, Poison, Corrode, Disrupt, Silence, Knockback, Break, Expose, Slow,
// SobrecargaTermica (DoT+Slow eletrico, cartas-technomagik.md secao 5.1 - achado ADR-016
// Balde B/Faraday 2026-07-15: faltava aqui, o que classificaria SobrecargaTermica como
// BUFF por exclusao e furaria o portao de imunidade EM-Shield/F-1, que so trava debuff).
// Neutro/auto: Decrypt (nao dispela a si mesmo).
[[nodiscard]] bool is_non_buff(StatusId id) {
    switch (id) {
        case StatusId::Stun:
        case StatusId::Poison:
        case StatusId::Corrode:
        case StatusId::Disrupt:
        case StatusId::Silence:
        case StatusId::Knockback:
        case StatusId::Break:
        case StatusId::Expose:
        case StatusId::Slow:
        case StatusId::Decrypt:
        case StatusId::SobrecargaTermica:
            return true;
        default:
            return false;
    }
}

// Util: e so espaco em branco (espelha string.IsNullOrWhiteSpace do C#).
[[nodiscard]] bool is_blank(const std::string& s) {
    return std::all_of(s.begin(), s.end(),
                       [](unsigned char c) { return std::isspace(c) != 0; });
}

}  // namespace

CombatActor::CombatActor(std::string id,
                         std::string display_name,
                         int max_hp,
                         int atk,
                         int def,
                         int spd,
                         CardFamily family,
                         bool is_player_side,
                         bool is_boss,
                         int knowledge_kills,
                         bool is_universal_compiler)
    : id_(std::move(id)),
      display_name_(std::move(display_name)),
      max_hp_(max_hp),
      atk_(atk),
      family_(family),
      is_player_side_(is_player_side),
      is_boss_(is_boss),
      knowledge_kills_(knowledge_kills),
      is_universal_compiler_(is_universal_compiler),
      hp_(max_hp),
      def_(def),
      spd_(spd) {
    if (id_.empty() || is_blank(id_))
        throw std::invalid_argument("CombatActor.id nao pode ser vazio.");
    if (max_hp <= 0)
        throw std::out_of_range("CombatActor.max_hp deve ser > 0.");
    if (atk < 0)
        throw std::out_of_range("CombatActor.atk deve ser >= 0.");
    if (def < 0)
        throw std::out_of_range("CombatActor.def deve ser >= 0.");
    if (spd < 0)
        throw std::out_of_range("CombatActor.spd deve ser >= 0.");
    if (knowledge_kills < 0)
        throw std::out_of_range("CombatActor.knowledge_kills deve ser >= 0.");
}

bool CombatActor::is_buff(StatusId id) { return !is_non_buff(id); }

void CombatActor::take_damage(int amount) {
    if (amount < 0)
        throw std::out_of_range("Dano deve ser >= 0.");
    const int remaining = absorb_with_shield(amount);
    hp_ = std::max(0, hp_ - remaining);
}

int CombatActor::absorb_with_shield(int amount) {
    if (amount <= 0) return amount;

    const auto it = std::find_if(status_effects_.begin(), status_effects_.end(),
                                 [](const StatusEffect& s) { return s.id == StatusId::Shield; });
    if (it == status_effects_.end()) return amount;

    const StatusEffect shield = *it;
    const int absorbed = std::min(amount, shield.magnitude);
    if (absorbed <= 0) return amount;  // pool ja vazio (defensivo); deixa expirar normal

    const int new_magnitude = shield.magnitude - absorbed;
    if (new_magnitude <= 0) {
        status_effects_.erase(it);
        status_changes_.push_back(StatusEffectChange{
            id_, StatusId::Shield, StatusChangeKind::Absorbed, absorbed, shield.duration});
        status_changes_.push_back(StatusEffectChange{
            id_, StatusId::Shield, StatusChangeKind::Expired, 0, 0});
    } else {
        StatusEffect updated = shield;
        updated.magnitude = new_magnitude;
        *it = updated;
        status_changes_.push_back(StatusEffectChange{
            id_, StatusId::Shield, StatusChangeKind::Absorbed, new_magnitude, shield.duration});
    }

    return amount - absorbed;
}

void CombatActor::heal(int amount) {
    if (amount < 0)
        throw std::out_of_range("Cura deve ser >= 0.");
    hp_ = std::min(max_hp_, hp_ + amount);
}

void CombatActor::reduce_def(int amount) {
    if (amount < 0)
        throw std::out_of_range("Reducao de Def deve ser >= 0.");
    def_ = std::max(0, def_ - amount);
}

bool CombatActor::apply_stat_delta(StatusId id, int def_delta, int spd_delta) {
    const auto it = std::find_if(applied_stat_deltas_.begin(), applied_stat_deltas_.end(),
                                 [id](const auto& kv) { return kv.first == id; });
    if (it != applied_stat_deltas_.end())
        return false;  // ja aplicado: tick subsequente nao reaplica

    if (def_delta != 0)
        def_ = std::max(0, def_ + def_delta);
    if (spd_delta != 0)
        spd_ = std::max(0, spd_ + spd_delta);

    // Break so mexe em Def; Haste/Slow so em Spd. Guarda o delta do eixo que cada um usa.
    applied_stat_deltas_.emplace_back(id, def_delta != 0 ? def_delta : spd_delta);
    return true;
}

bool CombatActor::revert_stat_delta(StatusId id) {
    const auto it = std::find_if(applied_stat_deltas_.begin(), applied_stat_deltas_.end(),
                                 [id](const auto& kv) { return kv.first == id; });
    if (it == applied_stat_deltas_.end())
        return false;

    const int delta = it->second;
    applied_stat_deltas_.erase(it);

    switch (id) {
        case StatusId::Break:
            // delta era negativo (reducao de Def). Inverte somando |delta|.
            def_ = std::max(0, def_ - delta);
            return false;
        case StatusId::Haste:
        case StatusId::Slow:
            spd_ = std::max(0, spd_ - delta);
            return true;
        default:
            return false;
    }
}

void CombatActor::refresh_resources_for_turn(int round_index) {
    ap_ = max_ap_;
    max_mana_ = std::min(combat_constants::kManaCap,
                         combat_constants::kBaseMana + std::max(0, round_index));
    mana_ = max_mana_;  // recarrega ao maximo, sem banking
    // Overclock (CARTAS-COMUNS-ENGINE): trava 1x/turno reseta no MESMO TurnStart que zera
    // AP/mana (secao 5) - o Tavus-Overclock volta a funcionar todo turno novo.
    overclock_used_ = false;
    // MemoryJammed (CARDS-HW-2 fatia 1, virus ZipBomb): "resto do turno" = ate o proximo
    // TurnStart PROPRIO deste ator, MESMO racional/sitio do overclock_used_ acima.
    memory_jammed_ = false;
}

void CombatActor::spend_ap(int cost) {
    if (cost < 0)
        throw std::out_of_range("Custo AP deve ser >= 0.");
    if (cost > ap_)
        throw std::logic_error("AP insuficiente para '" + id_ + "'.");
    ap_ -= cost;
}

void CombatActor::spend_mana(int cost) {
    if (cost < 0)
        throw std::out_of_range("Custo de mana deve ser >= 0.");
    if (cost > mana_)
        throw std::logic_error("Mana insuficiente para '" + id_ + "'.");
    mana_ -= cost;
}

void CombatActor::add_status(const StatusEffect& status) { try_add_status(status); }

bool CombatActor::blocked_by_em_shield(const StatusEffect& status) const {
    if (is_buff(status.id) || status.family_origin != CardFamily::Eletrico)
        return false;  // portao so trava DEBUFF de familia Eletrico (F-1).
    return std::any_of(status_effects_.begin(), status_effects_.end(),
                       [](const StatusEffect& s) { return s.id == StatusId::BlindagemEM; });
}

StatusApplyResult CombatActor::try_add_status(const StatusEffect& status) {
    // Portao de imunidade EM-Shield (Faraday, ADR-016 Balde B): bloqueia ANTES de
    // inserir/registrar Applied - o bloqueio nao pode deixar rastro de "aplicado" no
    // buffer de mudancas (secao 16), senao o log/UI mentiria.
    if (blocked_by_em_shield(status))
        return StatusApplyResult::BlockedByImmunity;

    insert_or_stack_status(status);

    // F-2 "previne + limpa": o proprio BlindagemEM entrando limpa os debuffs eletricos
    // JA presentes no alvo (nao so previne os futuros).
    if (status.id == StatusId::BlindagemEM)
        clear_electric_debuffs();

    return StatusApplyResult::Applied;
}

void CombatActor::insert_or_stack_status(const StatusEffect& status) {
    const auto it = std::find_if(status_effects_.begin(), status_effects_.end(),
                                 [&](const StatusEffect& s) { return s.id == status.id; });
    if (it == status_effects_.end()) {
        status_effects_.push_back(status);
        record_applied(status);
        return;
    }

    StatusEffect existing = *it;
    switch (status.stack_rule) {
        case StackRule::Replace:
            *it = status;
            break;
        case StackRule::Refresh:
            existing.duration = std::max(existing.duration, status.duration);
            *it = existing;
            break;
        case StackRule::StackMagnitude:
            existing.magnitude = existing.magnitude + status.magnitude;
            *it = existing;
            break;
        case StackRule::StackDuration:
            existing.duration = existing.duration + status.duration;
            *it = existing;
            break;
        default:
            throw std::out_of_range("StackRule desconhecida.");
    }

    // Stack/Replace/Refresh: a UI ainda quer refletir o estado resultante. Registra
    // Applied com os valores efetivos pos-stack (secao 16).
    record_applied(*it);
}

void CombatActor::clear_electric_debuffs() {
    std::vector<StatusId> to_remove;
    for (const StatusEffect& s : status_effects_) {
        if (s.id != StatusId::BlindagemEM && !is_buff(s.id) && s.family_origin == CardFamily::Eletrico)
            to_remove.push_back(s.id);
    }
    // Coleta os ids ANTES de remover (remove_status muta status_effects_ - iterar e mutar
    // o mesmo vetor ao mesmo tempo invalidaria o iterador do for-range acima).
    for (StatusId id : to_remove)
        remove_status(id);
}

void CombatActor::record_applied(const StatusEffect& status) {
    status_changes_.push_back(StatusEffectChange{
        id_, status.id, StatusChangeKind::Applied, status.magnitude, status.duration});
}

bool CombatActor::remove_status(StatusId id) {
    revert_stat_delta(id);  // restaura Def/Spd antes de tirar da lista (no-op se sem delta)
    const auto before = status_effects_.size();
    status_effects_.erase(
        std::remove_if(status_effects_.begin(), status_effects_.end(),
                       [id](const StatusEffect& s) { return s.id == id; }),
        status_effects_.end());
    return status_effects_.size() < before;
}

int CombatActor::index_of_status(StatusId id) const {
    for (std::size_t i = 0; i < status_effects_.size(); ++i) {
        if (status_effects_[i].id == id)
            return static_cast<int>(i);
    }
    return -1;
}

void CombatActor::replace_status_at(int index, const StatusEffect& updated) {
    status_effects_[static_cast<std::size_t>(index)] = updated;
}

bool CombatActor::expire_elapsed_statuses() {
    bool spd_changed = false;
    for (int i = static_cast<int>(status_effects_.size()) - 1; i >= 0; --i) {
        const StatusEffect status = status_effects_[static_cast<std::size_t>(i)];
        if (status.duration > 0) continue;

        // Restaura Def/Spd antes de remover (no-op se nao aplicou delta de stat).
        if (revert_stat_delta(status.id))
            spd_changed = true;

        status_effects_.erase(status_effects_.begin() + i);
        status_changes_.push_back(StatusEffectChange{
            id_, status.id, StatusChangeKind::Expired, 0, 0});
    }
    return spd_changed;
}

std::vector<StatusEffectChange> CombatActor::drain_status_changes() {
    std::vector<StatusEffectChange> snapshot = std::move(status_changes_);
    status_changes_.clear();
    return snapshot;
}

void CombatActor::restore_mana(int amount) {
    if (amount < 0)
        throw std::out_of_range("Restauracao de mana deve ser >= 0.");
    mana_ = std::min(max_mana_, mana_ + amount);
}

void CombatActor::grant_bonus_ap(int amount) {
    if (amount < 0)
        throw std::out_of_range("Bonus de AP deve ser >= 0.");
    ap_ += amount;
}

}  // namespace gus::domain::combat

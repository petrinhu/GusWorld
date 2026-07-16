// action_clock.cpp
//
// Implementacao do ActionClock isolado (ADR-017). Ver action_clock.hpp para o contrato
// e o racional. POCO puro, ZERO Qt/SDL, ZERO RNG (deterministico por construcao).

#include "gus/domain/combat/action_clock.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace gus::domain::combat {

using action_clock_constants::kBaseClock;
using action_clock_constants::kCardSpeedBasico;
using action_clock_constants::kCardSpeedCompilada;
using action_clock_constants::kCardSpeedInterpretada;
using action_clock_constants::kStyleTimeAgil1st;
using action_clock_constants::kStyleTimeAgil2nd;
using action_clock_constants::kStyleTimeAgil3rdPlus;
using action_clock_constants::kStyleTimeForte;
using action_clock_constants::kStyleTimeNormal;

namespace {

// styleMult (fator de TEMPO) para o estilo + a posicao na cadeia consecutiva de Agil
// (1-based; so relevante quando style == Agil). ADR-017 D3.
float style_time_mult(CastStyle style, int agile_streak_after_use) {
    switch (style) {
        case CastStyle::Normal:
            return kStyleTimeNormal;
        case CastStyle::Forte:
            return kStyleTimeForte;
        case CastStyle::Agil:
            if (agile_streak_after_use <= 1) return kStyleTimeAgil1st;
            if (agile_streak_after_use == 2) return kStyleTimeAgil2nd;
            return kStyleTimeAgil3rdPlus;
    }
    return kStyleTimeNormal;  // inalcancavel (switch exaustivo); silencia warning.
}

}  // namespace

float ActionClock::card_speed_mult(CardSpeedClass card_speed) noexcept {
    switch (card_speed) {
        case CardSpeedClass::Compilada:
            return kCardSpeedCompilada;
        case CardSpeedClass::Interpretada:
            return kCardSpeedInterpretada;
        case CardSpeedClass::Basico:
            return kCardSpeedBasico;
    }
    return kCardSpeedBasico;  // inalcancavel (switch exaustivo).
}

void ActionClock::add_actor(ActorId id, int initial_next_action_at, int spd,
                             int preference_order, bool is_protected,
                             int initial_last_acted_at) {
    ActionClockEntry entry;
    entry.actor_id = id;
    entry.next_action_at = initial_next_action_at;
    entry.last_acted_at = initial_last_acted_at;
    entry.spd = spd;
    entry.preference_order = preference_order;
    entry.is_protected = is_protected;
    entry.agile_streak = 0;
    entries_.push_back(entry);
}

const ActionClockEntry& ActionClock::entry_of(ActorId id) const {
    for (const auto& entry : entries_) {
        if (entry.actor_id == id) return entry;
    }
    throw std::invalid_argument("ActionClock::entry_of: actor_id nao registrado");
}

ActionClockEntry& ActionClock::mutable_entry_of(ActorId id) {
    for (auto& entry : entries_) {
        if (entry.actor_id == id) return entry;
    }
    throw std::invalid_argument("ActionClock::mutable_entry_of: actor_id nao registrado");
}

void ActionClock::apply_starvation_guard() {
    // D1 hibrido: nenhum ator protegido pode ter next_action_at agendado alem de
    // last_acted_at + max_starve_ticks_. Roda a CADA advance(), entao a invariante
    // nunca fica violada por mais de uma decisao de turno.
    for (auto& entry : entries_) {
        if (!entry.is_protected) continue;
        int cap = entry.last_acted_at + max_starve_ticks_;
        if (entry.next_action_at > cap) {
            // Catch-up bump: nunca deixa o cap regredir o clock global (tempo so anda
            // pra frente) - se o cap ja ficou no passado, forca pro "agora".
            entry.next_action_at = std::max(cap, global_tick_);
        }
    }
}

bool ActionClock::acts_before(const ActionClockEntry& a, const ActionClockEntry& b) noexcept {
    if (a.next_action_at != b.next_action_at) return a.next_action_at < b.next_action_at;
    // Desempate D3: quem nao agiu ha mais tempo (last_acted_at menor) vai primeiro.
    if (a.last_acted_at != b.last_acted_at) return a.last_acted_at < b.last_acted_at;
    // Depois, SPD mais alto.
    if (a.spd != b.spd) return a.spd > b.spd;
    // Por fim, preferencia do jogador (menor preference_order vence).
    return a.preference_order < b.preference_order;
}

ActorId ActionClock::advance() {
    if (entries_.empty()) {
        throw std::logic_error("ActionClock::advance: nenhum ator registrado");
    }
    apply_starvation_guard();

    std::size_t winner = 0;
    for (std::size_t i = 1; i < entries_.size(); ++i) {
        if (acts_before(entries_[i], entries_[winner])) winner = i;
    }
    global_tick_ = entries_[winner].next_action_at;
    return entries_[winner].actor_id;
}

void ActionClock::reset_after_action(ActorId id, int spd, CardSpeedClass card_speed,
                                      CastStyle style) {
    reset_after_action(id, spd, card_speed_mult(card_speed), style);
}

void ActionClock::reset_after_action(ActorId id, int spd, float card_speed_mult_value,
                                      CastStyle style) {
    ActionClockEntry& entry = mutable_entry_of(id);

    // Decaimento suave do Agil consecutivo (D3): incrementa em uso Agil, reseta em
    // Normal/Forte ("o contador por-ator reseta ao usar Normal/Forte").
    if (style == CastStyle::Agil) {
        entry.agile_streak += 1;
    } else {
        entry.agile_streak = 0;
    }

    float style_mult = style_time_mult(style, entry.agile_streak);
    double raw_delta = (static_cast<double>(kBaseClock) / static_cast<double>(spd)) *
                        static_cast<double>(card_speed_mult_value) *
                        static_cast<double>(style_mult);
    int delta = static_cast<int>(std::lround(raw_delta));

    entry.last_acted_at = global_tick_;
    entry.next_action_at = global_tick_ + delta;
}

int ActionClock::schedule_pending(ActorId target, int resolve_at) {
    PendingResolution pending;
    pending.id = next_pending_id_++;
    pending.target = target;
    pending.resolve_at = resolve_at;
    pending_.push_back(pending);
    return pending.id;
}

void ActionClock::cancel_pending(ActorId target) {
    for (auto& pending : pending_) {
        if (pending.target == target && !pending.fired && !pending.cancelled) {
            pending.cancelled = true;
        }
    }
}

std::vector<int> ActionClock::collect_due_pending() {
    std::vector<int> due_ids;
    for (auto& pending : pending_) {
        if (pending.cancelled || pending.fired) continue;
        if (pending.resolve_at <= global_tick_) {
            pending.fired = true;
            due_ids.push_back(pending.id);
        }
    }
    return due_ids;
}

bool ActionClock::is_pending_cancelled(int pending_id) const {
    for (const auto& pending : pending_) {
        if (pending.id == pending_id) return pending.cancelled;
    }
    throw std::invalid_argument("ActionClock::is_pending_cancelled: pending_id ausente");
}

bool ActionClock::is_pending_fired(int pending_id) const {
    for (const auto& pending : pending_) {
        if (pending.id == pending_id) return pending.fired;
    }
    throw std::invalid_argument("ActionClock::is_pending_fired: pending_id ausente");
}

}  // namespace gus::domain::combat

// gus/domain/combat/environment_clock.cpp
//
// Implementacao da roda temporal de periodo (secao 18.3), portada de
// engine/foundation/turn_combat/EnvironmentClock.cs. Deterministica, sem RNG. POCO puro,
// ZERO Qt, ZERO IO. As duracoes vem do catalogo (period_duration: 5/2/5/2).
//
// Cross-ref: engine/foundation/turn_combat/EnvironmentClock.cs;
//            docs/design/mecanicas/combat.md secao 18.1/18.3/18.6; ADR-006.

#include "gus/domain/combat/environment_clock.hpp"

#include <algorithm>
#include <array>
#include <stdexcept>

#include "gus/domain/combat/environment_catalog.hpp"

namespace gus::domain::combat {
namespace {

// Ordem canonica da roda (secao 18.3). Durações vem do catalogo.
constexpr std::array<EnvironmentId, 4> kCycle = {
    EnvironmentId::Dia,
    EnvironmentId::Crepusculo,
    EnvironmentId::Noite,
    EnvironmentId::Aurora,
};

// Indice de start_phase no ciclo, ou -1 se nao e fase de periodo (espelha Array.IndexOf).
int index_of(EnvironmentId phase) noexcept {
    for (std::size_t i = 0; i < kCycle.size(); ++i)
        if (kCycle[i] == phase)
            return static_cast<int>(i);
    return -1;
}

}  // namespace

EnvironmentClock::EnvironmentClock(EnvironmentId start_phase, int telegraph_turns)
    : telegraph_turns_(telegraph_turns), phase_index_(index_of(start_phase)) {
    if (telegraph_turns < 0)
        throw std::out_of_range("telegraph_turns deve ser >= 0.");
    if (phase_index_ < 0)
        throw std::invalid_argument("fase de inicio nao e uma fase de periodo (secao 18.3).");
}

EnvironmentId EnvironmentClock::current_phase() const noexcept {
    return kCycle[static_cast<std::size_t>(phase_index_)];
}

const EnvironmentModifier& EnvironmentClock::current() const {
    return EnvironmentCatalog::get(current_phase());
}

EnvironmentId EnvironmentClock::next_phase() const noexcept {
    return kCycle[(static_cast<std::size_t>(phase_index_) + 1) % kCycle.size()];
}

int EnvironmentClock::phase_duration() const {
    return EnvironmentCatalog::get(current_phase()).period_duration;
}

int EnvironmentClock::turns_remaining() const {
    return std::max(0, phase_duration() - turns_into_phase_);
}

bool EnvironmentClock::transition_telegraphed() const {
    return turns_remaining() <= telegraph_turns_;
}

bool EnvironmentClock::advance() {
    ++turns_into_phase_;
    if (turns_into_phase_ >= phase_duration()) {
        phase_index_ = static_cast<int>((static_cast<std::size_t>(phase_index_) + 1) % kCycle.size());
        turns_into_phase_ = 0;
        return true;
    }
    return false;
}

std::vector<EnvironmentId> EnvironmentClock::project(int turns) const {
    if (turns < 0)
        throw std::out_of_range("turns deve ser >= 0.");

    std::vector<EnvironmentId> result;
    result.reserve(static_cast<std::size_t>(turns));
    auto idx = static_cast<std::size_t>(phase_index_);
    int into = turns_into_phase_;
    for (int i = 0; i < turns; ++i) {
        result.push_back(kCycle[idx]);
        ++into;
        if (into >= EnvironmentCatalog::get(kCycle[idx]).period_duration) {
            idx = (idx + 1) % kCycle.size();
            into = 0;
        }
    }
    return result;
}

bool EnvironmentClock::is_chaotic_vector(EnvironmentId id) noexcept {
    return id == EnvironmentId::AnomaliaPerlin;
}

bool EnvironmentClock::any_chaotic(std::initializer_list<EnvironmentId> active) noexcept {
    for (const auto id : active)
        if (is_chaotic_vector(id))
            return true;
    return false;
}

bool EnvironmentClock::any_chaotic(const std::vector<EnvironmentId>& active) noexcept {
    for (const auto id : active)
        if (is_chaotic_vector(id))
            return true;
    return false;
}

}  // namespace gus::domain::combat

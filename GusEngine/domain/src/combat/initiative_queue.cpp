// gus/domain/combat/initiative_queue.cpp
//
// Implementacao da fila de iniciativa. Portado de
// engine/foundation/turn_combat/InitiativeQueue.cs, paridade de comportamento 1:1.
// POCO puro, ZERO Qt, ZERO I/O.
//
// Cross-ref: engine/foundation/turn_combat/InitiativeQueue.cs;
//            docs/design/mecanicas/combat.md secao 3/4; ADR-006.

#include "gus/domain/combat/initiative_queue.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace gus::domain::combat {

namespace {

// Clamp inteiro (espelha Math.Clamp do C#).
[[nodiscard]] int clamp_int(int value, int lo, int hi) {
    return std::max(lo, std::min(value, hi));
}

}  // namespace

InitiativeQueue::InitiativeQueue(std::vector<CombatActor*> actors) : order_(std::move(actors)) {
    if (order_.empty())
        throw std::invalid_argument("Fila de iniciativa precisa de pelo menos 1 ator.");
    // OrderByDescending(a => a.Spd) do LINQ e estavel; std::stable_sort preserva a ordem
    // de entrada em empates de SPD.
    std::stable_sort(order_.begin(), order_.end(),
                     [](const CombatActor* a, const CombatActor* b) { return a->spd() > b->spd(); });
    cursor_ = 0;
}

int InitiativeQueue::index_of(CombatActor* actor) const {
    const auto it = std::find(order_.begin(), order_.end(), actor);
    if (it == order_.end()) return -1;
    return static_cast<int>(it - order_.begin());
}

void InitiativeQueue::reorder_actor(CombatActor* actor, int delta_pos) {
    const int from = index_of(actor);
    if (from < 0)
        throw std::invalid_argument("Ator '" + (actor ? actor->id() : std::string{}) +
                                    "' nao esta na fila.");
    if (delta_pos == 0) return;

    const int to = clamp_int(from + delta_pos, 0, static_cast<int>(order_.size()) - 1);
    if (to == from) return;

    order_.erase(order_.begin() + from);
    order_.insert(order_.begin() + to, actor);

    // Reordenacao nao muda de quem e o turno corrente; mantem o cursor valido.
    cursor_ = clamp_int(cursor_, 0, static_cast<int>(order_.size()) - 1);
}

void InitiativeQueue::recompute_by_speed() {
    CombatActor* current_actor = current();
    std::stable_sort(order_.begin(), order_.end(),
                     [](const CombatActor* a, const CombatActor* b) { return a->spd() > b->spd(); });
    cursor_ = std::max(0, index_of(current_actor));
}

void InitiativeQueue::sync_cursor_to(CombatActor* actor) {
    const int idx = index_of(actor);
    if (idx >= 0)
        cursor_ = idx;
}

void InitiativeQueue::bring_to_current(CombatActor* actor) {
    const int from = index_of(actor);
    // No-op se ausente (from < 0), ja e o current (from == cursor_), ou esta ATRAS do
    // cursor (from < cursor_: ja passou nesta rodada; puxa-lo pra frente pularia o current).
    // So puxamos PRA FRENTE (from > cursor_): permutacao segura que preserva "cada ator age
    // uma vez por rodada". round_index e o INDICE do cursor ficam inalterados.
    if (from <= cursor_) return;

    // erase em `from` (> cursor_) nao desloca os slots [0, cursor_]; insert em cursor_ poe
    // o ator no slot corrente e empurra os intermediarios [cursor_, from-1] uma casa a
    // frente (todos continuam >= cursor_, ou seja, seguem pendentes nesta rodada).
    order_.erase(order_.begin() + from);
    order_.insert(order_.begin() + cursor_, actor);
    // cursor_ inalterado de proposito: order_[cursor_] agora e `actor` => current() == actor.
}

void InitiativeQueue::advance() {
    ++cursor_;
    if (cursor_ >= static_cast<int>(order_.size())) {
        cursor_ = 0;
        ++round_index_;
    }
}

void InitiativeQueue::remove(CombatActor* actor) {
    const int idx = index_of(actor);
    if (idx < 0) return;

    order_.erase(order_.begin() + idx);

    if (order_.empty()) {
        cursor_ = 0;
        return;
    }

    // Se removemos antes do cursor, o cursor desliza pra esquerda.
    if (idx < cursor_)
        --cursor_;
    // Se removemos o proprio ator no cursor, o cursor agora aponta pro que era o
    // seguinte; se estava no fim, faz wrap pro topo.
    if (cursor_ >= static_cast<int>(order_.size()))
        cursor_ = 0;
}

bool InitiativeQueue::contains(CombatActor* actor) const { return index_of(actor) >= 0; }

}  // namespace gus::domain::combat

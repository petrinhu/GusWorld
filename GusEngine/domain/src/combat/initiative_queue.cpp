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

int InitiativeQueue::reorder_pending(CombatActor* actor, int delta_pos) {
    const int from = index_of(actor);
    if (from < 0)
        throw std::invalid_argument("Ator '" + (actor ? actor->id() : std::string{}) +
                                    "' nao esta na fila.");

    // Alvo e o current() (from == cursor_) ou ja agiu nesta rodada (from < cursor_): nao ha
    // "acao futura" pra reordenar. No-op, retorna 0 (dissipacao - o caller loga/decide).
    if (from <= cursor_) return 0;

    const int to =
        clamp_int(from + delta_pos, cursor_ + 1, static_cast<int>(order_.size()) - 1);
    if (to == from) return 0;

    order_.erase(order_.begin() + from);
    order_.insert(order_.begin() + to, actor);
    // cursor_ intocado por construcao: from > cursor_ e to >= cursor_+1, entao a regiao
    // [0, cursor_] NUNCA e reescrita. current() e todo ator ja-agido preservam identidade -
    // a fila jamais cruza o cursor (raiz do bug GambitReorder-duplo, achado QA 2026-07-15).
    return to - from;
}

bool InitiativeQueue::delay_current(int n) {
    const int to = clamp_int(cursor_ + n, cursor_, static_cast<int>(order_.size()) - 1);
    if (to == cursor_) return false;  // ja no ultimo slot: nada a adiar.

    CombatActor* delayed = order_[static_cast<std::size_t>(cursor_)];
    order_.erase(order_.begin() + cursor_);
    order_.insert(order_.begin() + to, delayed);
    // cursor_ intocado: order_[cursor_] agora e quem estava logo apos `delayed`, que vira o
    // novo current(). round_index_ nao muda (nao e uma volta de fila, so um adiamento
    // intra-rodada - decisao do lider 2026-07-15, A2).
    return true;
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

void InitiativeQueue::regroup_stable(
    const std::function<bool(const CombatActor*)>& first_group) {
    // stable_partition move os que satisfazem `first_group` para a frente PRESERVANDO a ordem
    // relativa de AMBOS os grupos (ao contrario de std::partition, que nao garante ordem). E o
    // que torna o regroup Gambito-safe: um empurrao intra-rodada (reorder_actor) fica gravado
    // na ordem relativa e sobrevive ao agrupamento. NAO recomputa por SPD.
    std::stable_partition(order_.begin(), order_.end(), first_group);

    // Inicio de rodada: o cursor aponta pro primeiro ator do primeiro grupo (slot 0). Na
    // fronteira o cursor ja e 0 (wrap de advance); zeramos explicitamente pra nao depender
    // disso (contrato do metodo) e pra deixar current() == primeiro do lado que abre.
    // round_index_ NAO muda: regroup nao e uma volta de fila.
    cursor_ = 0;
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

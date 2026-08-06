// SPDX-License-Identifier: Apache-2.0
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
    // PRESERVA A PARTICAO (COMBATE-FILA-CURSOR-FIX, decisao do lider 2026-07-27/28): sorts
    // SEPARADOS pra [0, cursor_) - ja-agidos - e (cursor_, fim] - pendentes. O slot cursor_
    // em si (order_[cursor_], o current()) fica de FORA dos dois ranges - nao participa de
    // nenhum sort e portanto nunca se move.
    //
    // Por que o current NAO entra no sort do bloco ja-agido (mesmo sendo tecnicamente
    // "[0, cursor_]" inclusive, como a decisao descreve em prosa): se ele entrasse e saisse
    // mais rapido que algum ja-agido, o cursor teria que RECUAR pra acompanhar a nova
    // posicao do current dentro do bloco (cursor_ = index_of(current_actor)) - e isso
    // REABRE como "pendentes" os indices que ele ultrapassou, dando aos atores que la
    // estavam um SEGUNDO turno na MESMA rodada. E a mesma classe de bug que este metodo
    // existe pra fechar, so espelhada (ja-agido ganha turno extra em vez de pendente
    // perder o seu). Fixar o current no proprio slot - cursor_ NUNCA recalculado por busca
    // - e a unica construcao que preserva os dois invariantes ao mesmo tempo: identidade
    // de current() E "cada ator age exatamente 1x por rodada", pra qualquer posicao do
    // cursor, inclusive o caso-limite cursor_ no ultimo indice (onde o bloco "ja-agido"
    // e a fila INTEIRA e o bloco pendente e vazio).
    std::stable_sort(order_.begin(), order_.begin() + cursor_,
                     [](const CombatActor* a, const CombatActor* b) { return a->spd() > b->spd(); });
    std::stable_sort(order_.begin() + cursor_ + 1, order_.end(),
                     [](const CombatActor* a, const CombatActor* b) { return a->spd() > b->spd(); });
    // cursor_ intocado por construcao: nenhum dos dois sorts acima cobre o indice cursor_,
    // entao order_[cursor_] (current()) e sempre o MESMO ator, no MESMO slot.
}

void InitiativeQueue::sync_cursor_to(CombatActor* actor) {
    // FILA-SYNC-CURSOR-GUARDA (2026-08-06, QUARTO portao da mesma raiz de reorder_actor
    // 2026-07-15, recompute_by_speed 2026-08-01 e remove 2026-08-06). Ate esta data o corpo
    // era um SALTO CRU do cursor:
    //
    //     const int idx = index_of(actor);
    //     if (idx >= 0) cursor_ = idx;
    //
    // O cursor e a FRONTEIRA da particao "indice <= cursor_ ja agiu / indice > cursor_ esta
    // pendente". Um salto a atravessa, e cada sentido reproduz um dos dois bugs historicos:
    // PRA TRAS reabre a rodada (quem ja agiu volta a ser pendente e joga de novo com AP/mana
    // recarregados - o bug do 3o portao); PRA FRENTE marca como "ja agiu" quem nunca agiu, e
    // esse ator e PULADO na rodada (o bug do Knockback de 2026-07-15, que motivou criar
    // delay_current). Nao existe direcao segura: "so avanca" fecharia metade do buraco e
    // deixaria a outra metade com cara de guarda.
    //
    // Guarda: o cursor deixa de se mover. A intencao legitima da funcao - "current() volta a
    // ser `actor` depois que a ordem mudou" - ja era realizada com seguranca por
    // bring_to_current, que move o ATOR ate o slot do cursor por PERMUTACAO, com o indice do
    // cursor e round_index_ FIXOS. Delegar apaga o `cursor_ = idx` do arquivo: apos este
    // conserto, `cursor_` so e escrito no construtor (0), em regroup_stable (0, fronteira de
    // rodada), em advance() (o unico ++round_index_) e em remove() (deslize/recuo, contratos
    // contados) - nenhuma escrita por busca de indice sobrou.
    //
    // Efeito por caso: alvo PENDENTE -> vira current() por permutacao (os intermediarios
    // continuam > cursor_, seguem pendentes); alvo JA-AGIDO -> no-op (nao se re-sincroniza
    // pra quem ja jogou sem lhe dar 2o turno); alvo AUSENTE ou ja e o current -> no-op
    // (contrato antigo preservado). Ver initiative_queue_sync_cursor_guard_test.cpp.
    bring_to_current(actor);
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

    // Se removemos antes do cursor, o cursor desliza pra esquerda (o current continua sendo
    // o MESMO ator, agora uma casa antes). Nunca sai do intervalo valido.
    if (idx < cursor_)
        --cursor_;

    // Se removemos o proprio ator no cursor, o cursor (intocado) passa a apontar pro que era
    // o seguinte - a menos que o removido fosse o ULTIMO slot: ai o cursor cai fora da fila
    // encurtada e precisa voltar pra dentro.
    //
    // FILA-REMOVE-ROUND-WRAP (auditoria independente 2026-08-06): aqui ficava
    // `cursor_ = 0`, ou seja, a fila DAVA A VOLTA sem contar a rodada - e o advance()
    // seguinte (advance_to_next_actor) levava o cursor de 0 pra 1 com a rodada ainda aberta:
    // todo ator do indice 1 em diante ganhava um SEGUNDO turno nela, com AP e mana
    // recarregados por refresh_resources_for_turn, e o ator do indice 0 era PULADO na rodada
    // seguinte. Terceiro portao da mesma raiz de reorder_actor (2026-07-15) e
    // recompute_by_speed (2026-08-01).
    //
    // O conserto NAO conta rodada aqui: advance() segue sendo o UNICO dono do
    // ++round_index_ (e o unico ponto onde a FSM detecta a fronteira - advance_to_next_actor
    // compara round_index antes/depois pra disparar process_round_end_hooks/
    // regroup_round_by_side/advance_period_clock; contar a rodada DENTRO do prune tornaria
    // essa fronteira invisivel pra ela). O cursor recua pro NOVO ultimo slot, deixando a
    // volta PENDENTE: o proximo advance() a executa, conta a rodada e abre a nova no topo.
    //
    // Bonus de invariante: o slot pro qual o cursor recua e de um ator que JA AGIU nesta
    // rodada, entao a particao "todo indice <= cursor_ ja agiu / todo indice > cursor_ esta
    // pendente" continua valendo - a mesma particao que recompute_by_speed existe pra
    // preservar. Saltar pro topo a quebrava (indices > 0 ficavam cheios de ja-agidos).
    if (cursor_ >= static_cast<int>(order_.size()))
        cursor_ = static_cast<int>(order_.size()) - 1;
}

bool InitiativeQueue::contains(CombatActor* actor) const { return index_of(actor) >= 0; }

}  // namespace gus::domain::combat

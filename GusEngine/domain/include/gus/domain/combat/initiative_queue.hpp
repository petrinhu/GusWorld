// gus/domain/combat/initiative_queue.hpp
//
// Fila de iniciativa por-ator (CTB-style), SEMPRE VISIVEL ao jogador (secao 4). E
// mecanica central: o Gambito opera sobre esta fila. Ordenada por SPD desc na
// construcao; reordenacoes manuais (reorder_pending / delay_current / Gambito / Knockback)
// persistem ate a proxima recomputacao natural por SPD. Portado de
// engine/foundation/turn_combat/InitiativeQueue.cs. POCO puro, ZERO Qt (invariante de
// domain/, engine-design.md secao 2).
//
// PORTE DE REFERENCIA: o C# guarda referencias a CombatActor (class). No C++ a fila
// guarda ponteiros NAO-DONOS (CombatActor*): os atores vivem no escopo do dono
// (CombatStateMachine / teste) e a fila apenas ordena/aponta. Nenhuma propriedade de
// ownership; nao deleta atores.
//
// Invariante protegido (herdado do C#):
//   "A ordem da fila reflete a prioridade de turno corrente. O ponteiro current() sempre
//    aponta para um ator enquanto houver atores. round_index() conta rodadas completas
//    de fila, governando o ramp de mana de forma consistente entre todos os atores."
//
// MAPEAMENTO de excecoes C# -> C++:
//   ArgumentException -> std::invalid_argument
//
// Cross-ref: engine/foundation/turn_combat/InitiativeQueue.cs;
//            docs/design/mecanicas/combat.md secao 3/4; ADR-006.

#ifndef GUS_DOMAIN_COMBAT_INITIATIVE_QUEUE_HPP
#define GUS_DOMAIN_COMBAT_INITIATIVE_QUEUE_HPP

#include <functional>
#include <vector>

#include "gus/domain/combat/combat_actor.hpp"

namespace gus::domain::tests {
struct InitiativeQueueRawReorderTestAccess;
}  // namespace gus::domain::tests

namespace gus::domain::combat {

// Fila de iniciativa visivel e manipulavel. POCO testavel. secao 4.
class InitiativeQueue {
public:
    // Constroi a fila ordenando por SPD descendente. Empate de SPD mantem a ordem de
    // entrada (std::stable_sort, espelha OrderByDescending estavel do LINQ). Lanca
    // std::invalid_argument se a colecao vier vazia.
    explicit InitiativeQueue(std::vector<CombatActor*> actors);

    // Snapshot read-only da ordem atual (UI mostra proximos N). secao 4.
    [[nodiscard]] const std::vector<CombatActor*>& order() const noexcept { return order_; }

    // Ator cujo turno e o corrente.
    [[nodiscard]] CombatActor* current() const noexcept { return order_[cursor_]; }

    // Indice do slot do turno corrente (0-based). Simetrico a round_index(); usado pela
    // Janela de Comando da Party (§4.1) para saber quem ainda nao agiu nesta rodada
    // (slots >= cursor). Leitura pura.
    [[nodiscard]] int cursor() const noexcept { return cursor_; }

    // Total de atores na fila.
    [[nodiscard]] int count() const noexcept { return static_cast<int>(order_.size()); }

    // Indice da rodada completa de fila (0-based). Governa o ramp de mana (secao 3/5).
    [[nodiscard]] int round_index() const noexcept { return round_index_; }

    // Reordena um ator PENDENTE (ainda nao agiu nesta rodada) delta_pos casas, SEM jamais
    // cruzar o cursor: clamp em [cursor()+1, count()-1]. Primitiva SEGURA de Gambito-
    // Reordenar/Knockback-em-outro-ator/Einstein (decisao do lider 2026-07-15, A1/A3) - ao
    // contrario da antiga reorder_actor (privatizada no M9, 2026-07-22 - ver secao private
    // abaixo), a regiao [0, cursor()] NUNCA e tocada, entao current() e
    // todo ator com indice <= cursor() preservam identidade por construcao. Alvo ausente:
    // lanca std::invalid_argument. Alvo e o current() OU ja agiu (indice <= cursor()):
    // no-op, retorna 0 (dissipacao - a carta ja gastou o custo, so o efeito nao se aplica;
    // o CALLER decide o log). Retorna o delta REALMENTE aplicado (0 = no-op/dissipado; pode
    // divergir de delta_pos se o clamp absorveu parte do pedido).
    [[nodiscard]] int reorder_pending(CombatActor* actor, int delta_pos);

    // Adia o turno do ator CORRENTE em ate n slots (clamp no fim da fila), SEM mexer no
    // indice do cursor: o vizinho que estava logo apos passa a ser current(), e o ator
    // adiado age em seguida (decisao do lider 2026-07-15, A2 - Knockback "adia o turno" em
    // vez de saltar o vizinho). round_index_ NAO muda. Retorna false (no-op) se o corrente
    // ja esta no ultimo slot da fila (nada a adiar - o caller consome o status e o ator age
    // agora mesmo).
    bool delay_current(int n);

    // Recomputa a ordem por SPD (entrada de novos atores ou mudanca de SPD via
    // Haste/Slow). secao 4. Mantem o cursor apontando pro ator que estava em turno.
    void recompute_by_speed();

    // Re-sincroniza o cursor pra continuar apontando para actor (FSM quando uma
    // reordenacao no proprio tick muda a ordem mas o ator NAO perde o turno). secao 4.
    // No-op se o ator nao esta na fila.
    void sync_cursor_to(CombatActor* actor);

    // Traz `actor` para o SLOT DO CURSOR (passa a ser current()) SEM mexer no indice do
    // cursor nem em round_index. Realiza a escolha do jogador dentro do bloco da party
    // (Janela de Comando da Party, §4.1): reordena o ator ate o slot corrente, deslocando
    // os demais para frente. E PERMUTACAO, nao salto de cursor, logo preserva "cada ator
    // age uma vez por rodada". No-op se o ator nao esta na fila, ja e o current(), ou esta
    // ATRAS do cursor (ja passou nesta rodada: nao pode ser puxado sem pular o current).
    void bring_to_current(CombatActor* actor);

    // Reagrupa a fila na FRONTEIRA da rodada, movendo para a FRENTE os atores que satisfazem
    // `first_group` (na sua ordem relativa CORRENTE) e deixando os demais atras (idem), via
    // std::stable_partition. E o primitivo do regroup-por-lado da Janela de Comando da Party
    // (§4.1): a rodada vira "um lado age todo, depois o outro". Como e stable_partition (NAO
    // sort), a ordem relativa DENTRO de cada grupo e preservada => um empurrao de Gambito/
    // knockback aplicado na rodada anterior SOBREVIVE (Gambito-safe: um SORT por SPD o
    // desfaria). A fila nao conhece "lado": quem abre e o predicado sao decididos pelo caller
    // (CombatStateMachine).
    //
    // CONTRATO DE CURSOR: operacao de INICIO de rodada. O cursor volta a 0 (o primeiro ator do
    // primeiro grupo passa a ser current()) e round_index NAO muda. Chamar SOMENTE na fronteira
    // da rodada (onde o cursor ja e 0); no meio da rodada quebraria "cada ator age uma vez por
    // rodada". No-op de ordenacao quando a fila ja esta agrupada.
    void regroup_stable(const std::function<bool(const CombatActor*)>& first_group);

    // Avanca o ponteiro pro proximo ator. Ao dar a volta (wrap), incrementa round_index.
    void advance();

    // Remove um ator da fila (morte/incapacitacao). Ajusta o cursor pra continuar
    // apontando pro "proximo a jogar" coerente. No-op se o ator nao esta na fila.
    void remove(CombatActor* actor);

    // true se o ator esta na fila.
    [[nodiscard]] bool contains(CombatActor* actor) const;

    // Indice de `actor` na ordem corrente, ou -1 se ausente. Leitura pura (read-only):
    // publico pro CALLER decidir guards de cursor (ex.: "alvo ja agiu" em
    // resolve_gambit_reorder/handle_delay_action) sem duplicar a busca linear.
    [[nodiscard]] int index_of(CombatActor* actor) const;

private:
    // Acesso de teste-only pro reorder_actor privado logo abaixo (privatizacao M9,
    // COMBATE-FILA-CURSOR-FIX, decisao do lider 2026-07-15/fechamento 2026-07-22): os testes
    // que exercitam o clamp CRU [0, count()-1] ignorando o cursor (a prova do proprio
    // invariante que motivou privatizar a funcao) nao tem equivalente publico - nenhuma
    // primitiva publica cruza o cursor por design. Friend de TESTE, nunca de producao; ver
    // domain/tests/initiative_queue_test_access.hpp.
    friend struct gus::domain::tests::InitiativeQueueRawReorderTestAccess;

    // Move um ator delta_pos casas (negativo = adiantar, positivo = atrasar). Clamp nos
    // limites da fila [0, count-1] SEM olhar o cursor: pode cruzar current() e reescrever
    // a regiao [0, cursor_], desincronizando current()/identidade de quem ja agiu (raiz do
    // bug GambitReorder-duplo/Knockback-pula-vizinho, achado QA 2026-07-15). PRIVATIZADA no
    // M9 (2026-07-22): os 3 callers de producao (Gambito/Knockback/Einstein) ja tinham
    // migrado pra reorder_pending/delay_current desde 2026-07-15 (decisao do lider, A1/A3);
    // mante-la publica so deixava a porta aberta pra codigo novo reintroduzir a mesma classe
    // de bug sem perceber. Nao chamada por nenhum outro metodo desta classe hoje - sobrevive
    // como primitiva crua documentada, acessivel so a friend de teste (ver acima). Lanca
    // std::invalid_argument se o ator nao esta na fila.
    void reorder_actor(CombatActor* actor, int delta_pos);

    std::vector<CombatActor*> order_;
    int cursor_ = 0;
    int round_index_ = 0;
};

}  // namespace gus::domain::combat

#endif  // GUS_DOMAIN_COMBAT_INITIATIVE_QUEUE_HPP

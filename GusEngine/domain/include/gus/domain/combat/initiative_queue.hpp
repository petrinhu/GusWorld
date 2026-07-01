// gus/domain/combat/initiative_queue.hpp
//
// Fila de iniciativa por-ator (CTB-style), SEMPRE VISIVEL ao jogador (secao 4). E
// mecanica central: o Gambito opera sobre esta fila. Ordenada por SPD desc na
// construcao; reordenacoes manuais (reorder_actor / knockback / Gambito) persistem ate
// a proxima recomputacao natural por SPD. Portado de
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

#include <vector>

#include "gus/domain/combat/combat_actor.hpp"

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

    // Move um ator delta_pos casas (negativo = adiantar, positivo = atrasar). Clamp nos
    // limites da fila. Primitiva de knockback (+1) e Gambito-reordenar. secao 4. Lanca
    // std::invalid_argument se o ator nao esta na fila.
    void reorder_actor(CombatActor* actor, int delta_pos);

    // Recomputa a ordem por SPD (entrada de novos atores ou mudanca de SPD via
    // Haste/Slow). secao 4. Mantem o cursor apontando pro ator que estava em turno.
    void recompute_by_speed();

    // Re-sincroniza o cursor pra continuar apontando para actor (FSM quando um
    // reorder_actor no proprio tick muda a ordem mas o ator NAO perde o turno). secao 4.
    // No-op se o ator nao esta na fila.
    void sync_cursor_to(CombatActor* actor);

    // Traz `actor` para o SLOT DO CURSOR (passa a ser current()) SEM mexer no indice do
    // cursor nem em round_index. Realiza a escolha do jogador dentro do bloco da party
    // (Janela de Comando da Party, §4.1): reordena o ator ate o slot corrente, deslocando
    // os demais para frente. E PERMUTACAO, nao salto de cursor, logo preserva "cada ator
    // age uma vez por rodada". No-op se o ator nao esta na fila, ja e o current(), ou esta
    // ATRAS do cursor (ja passou nesta rodada: nao pode ser puxado sem pular o current).
    void bring_to_current(CombatActor* actor);

    // Avanca o ponteiro pro proximo ator. Ao dar a volta (wrap), incrementa round_index.
    void advance();

    // Remove um ator da fila (morte/incapacitacao). Ajusta o cursor pra continuar
    // apontando pro "proximo a jogar" coerente. No-op se o ator nao esta na fila.
    void remove(CombatActor* actor);

    // true se o ator esta na fila.
    [[nodiscard]] bool contains(CombatActor* actor) const;

private:
    [[nodiscard]] int index_of(CombatActor* actor) const;

    std::vector<CombatActor*> order_;
    int cursor_ = 0;
    int round_index_ = 0;
};

}  // namespace gus::domain::combat

#endif  // GUS_DOMAIN_COMBAT_INITIATIVE_QUEUE_HPP

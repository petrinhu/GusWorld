// initiative_queue_test_access.hpp
//
// Friend de TESTE (nunca de producao) pro reorder_actor privatizado no M9 (fechamento da
// dívida COMBATE-FILA-CURSOR-FIX, 2026-07-22 - decisao do lider original em 2026-07-15,
// A1/A3). reorder_actor cruza o cursor SEM guard por design (a primitiva crua que causou o
// bug turno-duplo do Gambito e o vizinho-pulado do Knockback); os 3 callers de producao ja
// usam as primitivas seguras (reorder_pending/delay_current/bring_to_current) desde
// 2026-07-15, entao a funcao crua nao deve mais ser alcancavel por codigo de jogo.
//
// Os testes que exercitam o clamp cru [0, count()-1] ignorando o cursor continuam cobrindo
// esse comportamento (a prova do proprio invariante que motivou privatizar a funcao) via
// este shim, em vez de reescrever sobre reorder_pending/bring_to_current - essas primitivas
// TEM clamps diferentes (respeitam o cursor por contrato) e nao reproduzem o comportamento
// cru sendo testado.
//
// Incluido SOMENTE por domain/tests/*.cpp. Nao compilado em codigo de producao.
//
// Cross-ref: gus/domain/combat/initiative_queue.hpp (friend declarado na classe).

#ifndef GUS_DOMAIN_TESTS_INITIATIVE_QUEUE_TEST_ACCESS_HPP
#define GUS_DOMAIN_TESTS_INITIATIVE_QUEUE_TEST_ACCESS_HPP

#include "gus/domain/combat/initiative_queue.hpp"

namespace gus::domain::tests {

struct InitiativeQueueRawReorderTestAccess {
    static void reorder_actor(gus::domain::combat::InitiativeQueue& queue,
                              gus::domain::combat::CombatActor* actor, int delta_pos) {
        queue.reorder_actor(actor, delta_pos);
    }
};

}  // namespace gus::domain::tests

#endif  // GUS_DOMAIN_TESTS_INITIATIVE_QUEUE_TEST_ACCESS_HPP

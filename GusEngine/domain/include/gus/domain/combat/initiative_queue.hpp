// SPDX-License-Identifier: Apache-2.0
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
    // Haste/Slow). secao 4. PRESERVA A PARTICAO (COMBATE-FILA-CURSOR-FIX, decisao do
    // lider 2026-07-27/28): ordena por SPD dentro de [0, cursor()) - ja-agidos - e dentro
    // de (cursor(), fim] - pendentes - de forma independente, NUNCA cruzando o cursor. O
    // ator current() fica FIXO no proprio slot (nao entra em nenhum dos dois sorts): se
    // entrasse no sort do bloco ja-agido e saisse mais rapido que algum deles, o cursor
    // teria que RECUAR pra acompanhar sua nova posicao, reabrindo indices ja-agidos como
    // "pendentes" e dando a eles um 2o turno na mesma rodada (a mesma classe de bug do
    // turno-duplo/pulo que este metodo existe pra evitar, so espelhada). current() e
    // cursor() saem IDENTICOS (mesmo ator, mesmo indice); quem ja agiu continua tendo
    // agido, quem falta continua faltando - so a ordem de EXIBICAO dentro de cada lado
    // muda. Substitui o comportamento antigo (full-sort da fila inteira reapontando o
    // cursor por index_of(current) - achado QA 2026-07-28, initiative_queue_recompute_
    // round_invariant_test.cpp).
    void recompute_by_speed();

    // Faz current() voltar a ser `actor` depois que a ordem mudou (reordenacao no proprio
    // tick que NAO tira o turno do ator). secao 4. No-op se o ator nao esta na fila.
    //
    // CONTRATO DE CURSOR (FILA-SYNC-CURSOR-GUARDA, 2026-08-06) - o cursor NAO SE MOVE: esta
    // funcao e hoje um encaminhamento pra bring_to_current, ou seja, move o ATOR ate o slot
    // do cursor por PERMUTACAO, com cursor() e round_index() INVARIANTES. Ate esta data o
    // corpo era um salto cru (`cursor_ = index_of(actor)`) e o nome dizia a verdade sobre o
    // mecanismo - o problema e que o mecanismo era inseguro nos DOIS sentidos: pra TRAS
    // reabria a rodada (ja-agido volta a pendente e joga 2x, com AP/mana recarregados - o bug
    // do 3o portao) e pra FRENTE marcava como ja-agido quem nunca agiu, PULANDO o ator (o bug
    // do Knockback de 2026-07-15, que motivou criar delay_current). Por isso a guarda nao e um
    // clamp direcional ("so avanca" fecharia so metade do buraco).
    //
    // ATENCAO - ESTE DOC MENTIA ate 2026-08-06: dizia que a CombatStateMachine chamava a funcao
    // "apos reorder". A FSM nunca chamou - a varredura da fatia achou ZERO caller de producao
    // (so testes e o fuzz). O comentario gemeo em initiative_queue_property_test.cpp foi
    // corrigido no mesmo commit. Comentario que mente convida codigo novo a usar a primitiva
    // errada; era metade do defeito.
    //
    // Efeito por caso: alvo PENDENTE (indice > cursor()) -> vira current(), os intermediarios
    // sao empurrados uma casa a frente e seguem PENDENTES; alvo JA-AGIDO (indice < cursor())
    // -> no-op (nao se re-sincroniza pra quem ja jogou sem lhe dar um 2o turno); alvo AUSENTE
    // ou ja e o current() -> no-op. bring_to_current e o nome canonico do primitivo; este
    // sobrevive como o nome HISTORICO que o codigo de jogo poderia procurar, agora pousando
    // no caminho seguro. Ver initiative_queue_sync_cursor_guard_test.cpp.
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
    // CONTRATO DE CURSOR (FILA-REGROUP-GUARDA, 2026-08-06) - operacao de INICIO de rodada, e
    // agora IMPOSTA pelo codigo em vez de prometida pelo comentario. Ate esta data o corpo
    // reparticionava a fila INTEIRA e fazia `cursor_ = 0` incondicionalmente: chamada no meio
    // da rodada, ela REABRIA a rodada - todo ator que ja tinha agido voltava a "pendente" e
    // jogava de novo, com AP/mana recarregados por refresh_resources_for_turn. E a assinatura
    // exata do bug de remove() (3o portao), pela quinta porta da mesma raiz.
    //
    // A guarda e a precondicao REAL do metodo, "estamos na fronteira da rodada" == cursor_ == 0:
    //   - cursor_ != 0 -> NO-OP TOTAL (ordem, cursor e round_index intocados), retorna false;
    //   - cursor_ == 0 -> particiona a fila inteira, retorna true. round_index NAO muda.
    // Retorna true tambem quando nao havia o que mover (fila ja agrupada): o retorno responde
    // "a operacao foi ACEITA?", nao "a ordem mudou?".
    //
    // [[nodiscard]] de proposito: com -Werror=unused-result ligado (2026-08-06), um caller
    // futuro NAO COMPILA ignorando a recusa. A guarda nao e um no-op silencioso - ela e
    // sinalizada em tempo de COMPILACAO (o retorno obrigatorio) e em runtime pelo caller
    // (CombatStateMachine::regroup_round_by_side loga a recusa no log de combate).
    //
    // POR QUE NAO "alargar" em vez de guardar: a alternativa de reparticionar so [cursor_, fim)
    // sem tocar o cursor e byte-identica na fronteira e PARECE seguro, mas foi MEDIDA e
    // reprovada - a permutacao tira do slot do cursor o ator que esta agindo e o joga pra tras,
    // onde ele volta a contar como pendente: age 2x e quem tomou seu slot e pulado. Mesma classe
    // de bug, raio de 1 ator em vez de N. (Nao contradiz bring_to_current, que faz permutacao
    // parecida: ela e chamada por begin_turn no COMECO do turno, quando o ator do slot do cursor
    // ainda nao agiu. regroup_stable nao sabe em que ponto do turno esta, entao nao pode assumir
    // esse timing.) Ver initiative_queue_regroup_guard_test.cpp.
    //
    // Os 2 callers de producao entram SEMPRE na fronteira - medido, nao presumido: instrumentando
    // CombatStateMachine::regroup_round_by_side e rodando a suite de domain inteira, cursor == 0
    // em 62.684 de 62.684 chamadas.
    [[nodiscard]] bool regroup_stable(const std::function<bool(const CombatActor*)>& first_group);

    // Avanca o ponteiro pro proximo ator. Ao dar a volta (wrap), incrementa round_index.
    void advance();

    // Remove um ator da fila (morte/incapacitacao). Ajusta o cursor pra continuar apontando
    // pro "proximo a jogar" coerente. No-op se o ator nao esta na fila.
    //
    // CONTRATO DE CURSOR (FILA-REMOVE-ROUND-WRAP, auditoria independente 2026-08-06) -
    // remove NUNCA conta rodada: advance() e o UNICO dono do ++round_index. Os 4 casos:
    //   - removido ANTES do cursor (ja agiu): cursor desliza -1, current() preservado por
    //     IDENTIDADE;
    //   - removido DEPOIS do cursor (pendente): cursor e current() intocados;
    //   - removido E o current, com pendentes atras: cursor intocado, current() passa a ser
    //     o ator SEGUINTE (que assume o turno);
    //   - removido E o current NO ULTIMO SLOT (todos os outros ja agiram): o cursor recua
    //     pro NOVO ultimo slot, deixando a volta de fila PENDENTE - o proximo advance() a
    //     executa, conta a rodada e abre a nova no topo. Ate 2026-08-06 o cursor SALTAVA
    //     pro topo (0) aqui, sem contar rodada: a rodada REABRIA e todo ator do indice 1 pra
    //     frente ganhava um 2o turno nela (com AP/mana recarregados), enquanto o do indice 0
    //     era pulado na seguinte. Ver initiative_queue_remove_round_invariant_test.cpp.
    // Em TODOS os casos vale a particao "todo indice <= cursor() ja agiu nesta rodada / todo
    // indice > cursor() esta pendente" - a mesma que reorder_pending/recompute_by_speed
    // protegem. Fila esvaziada: cursor 0 (e current() deixa de ter contrato - count()==0).
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

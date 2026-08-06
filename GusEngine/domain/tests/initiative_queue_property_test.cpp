// SPDX-License-Identifier: Apache-2.0
// initiative_queue_property_test.cpp
//
// REFORCO DE QA (marco M5) da InitiativeQueue (secao 4) por PROPRIEDADE + FUZZ. A fila e a
// mecanica central sobre a qual o Gambito opera; sua INTEGRIDADE sob sequencias arbitrarias
// de advance/reorder/recompute/remove e critica. Varremos milhares de sequencias aleatorias
// deterministicas e afirmamos os INVARIANTES (precondicao respeitada: nunca esvaziamos a
// fila, pois current() exige >= 1 ator por contrato - "current() sempre aponta para um ator
// enquanto houver atores", initiative_queue.hpp):
//
//   INV-9a o cursor SEMPRE indexa dentro de [0, count); current() devolve um ator presente.
//   INV-9b count() == numero de atores ainda na fila (sem duplicatas, sem fantasmas).
//   INV-9c (revisado COMBATE-FILA-CURSOR-FIX, decisao do lider 2026-07-27/28) recompute_by_
//          speed PRESERVA A PARTICAO: ordena dentro de [0, cursor) e dentro de (cursor, fim]
//          separadamente, NUNCA move um ator de um lado pro outro do cursor - current()
//          fica FIXO no proprio slot (identidade E indice) e todo ator com indice < cursor
//          (ja agiu) continua com indice < cursor apos o recompute; todo ator com indice >
//          cursor (pendente) continua com indice > cursor. Ordem NAO-crescente em SPD DENTRO
//          de cada lado (nao mais global: current pode ficar fora de ordem de SPD em relacao
//          aos dois lados - e o preco de nao dar 2o turno pra quem ja agiu nem pular quem
//          falta). Substitui o INV-9c antigo (ordem global nao-crescente apos recompute), que
//          descrevia o full-sort cruzando o cursor - a RAIZ do bug de turno-duplo/pulo
//          (achado QA 2026-07-28, mesmo ramo do F2-QA.8 abaixo).
//   INV-9d advance() em volta completa incrementa round_index exatamente uma vez por wrap.
//   INV-9e remove de ator ausente / sync de ator ausente sao no-op (cursor intacto).
//   INV-9f reorder_actor preserva o conjunto (mesma multiplicidade de atores) e o current().
//   INV-9h (FILA-REMOVE-ROUND-WRAP, auditoria independente 2026-08-06) remove() NUNCA conta
//          rodada: advance() e o UNICO dono do ++round_index (e o unico ponto onde a FSM ve
//          a fronteira, comparando round_index antes/depois, pra disparar os hooks de fim
//          de rodada). Ate esta data remove() dava a volta na fila (cursor -> 0) sem contar
//          rodada quando o current no ULTIMO slot saia (veneno matando no proprio TurnStart,
//          via prune_dead), e a rodada REABRIA.
//   INV-9i (FILA-REMOVE-ROUND-WRAP) remove() PRESERVA A PARTICAO, como reorder_pending e
//          recompute_by_speed: apos o remove, todo ator com indice < cursor() era ja-agido
//          (indice <= cursor() antes) e todo ator com indice > cursor() era pendente
//          (indice > cursor() antes). Ninguem atravessa o cursor - era exatamente isso que o
//          wrap mudo quebrava (com cursor_ = 0, tudo a partir do indice 1 virava "pendente"
//          de novo e ganhava um 2o turno na mesma rodada, com AP/mana recarregados).
//   INV-9j (FILA-REMOVE-ROUND-WRAP) remover OUTRO ator (que nao o current) preserva
//          current() por IDENTIDADE: quem esta em acao nao perde o turno pro vizinho quando
//          alguem cai da fila. Achado por mutation testing 2026-08-06 (apagar o slide
//          `--cursor_` do ramo "removido ja-agido" sobrevivia a suite inteira).
//   INV-9g (COMBATE-FILA-CURSOR-FIX, decisao do lider 2026-07-15) reorder_pending NUNCA
//          cruza o cursor: current() e todo ator com indice <= cursor(), POR IDENTIDADE,
//          permanecem intactos apos qualquer reorder_pending, para qualquer alvo/delta
//          pendente numa sequencia arbitraria - mata a RAIZ do bug (reorder_actor cru
//          podia reescrever essa regiao e desincronizar quem esta em acao/ja agiu).
//
// property-based "a mao" (property_gen.hpp): LCG semeado por indice de caso. NAO altera
// codigo de producao. Um caso que viole um invariante e BUG -> reportar ao lider com a seed.
//
// Subsistema: domain/combat (InitiativeQueue). POCO puro, ZERO Qt, headless.

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include "gus/domain/combat/combat_actor.hpp"
#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/combat/initiative_queue.hpp"
#include "initiative_queue_test_access.hpp"
#include "property_gen.hpp"

using namespace gus::domain::combat;
using gus::domain::tests::InitiativeQueueRawReorderTestAccess;
using gus::domain::tests::Lcg;

namespace {

constexpr int kCasesPerProperty = 2000;
constexpr int kStepsPerCase = 60;

// Pool de atores donos (estaveis): a fila guarda ponteiros NAO-DONOS.
std::vector<std::unique_ptr<CombatActor>> make_pool(Lcg& g, int n) {
    std::vector<std::unique_ptr<CombatActor>> pool;
    pool.reserve(static_cast<std::size_t>(n));
    for (int i = 0; i < n; ++i) {
        const int spd = g.in_range(0, 30);
        pool.push_back(std::make_unique<CombatActor>(
            "a" + std::to_string(i), "a" + std::to_string(i), 50, 5, 2, spd,
            static_cast<CardFamily>(g.in_range(0, 4)), /*player=*/true));
    }
    return pool;
}

// Verifica integridade estrutural num ponto qualquer da sequencia.
void check_integrity(const InitiativeQueue& q) {
    const auto& order = q.order();
    const int count = q.count();
    // INV-9b: count coerente com a ordem visivel.
    REQUIRE(count == static_cast<int>(order.size()));
    REQUIRE(count >= 1);

    // INV-9a: current() e um ator presente na ordem (cursor dentro de [0,count)).
    CombatActor* cur = q.current();
    REQUIRE(cur != nullptr);
    REQUIRE(std::find(order.begin(), order.end(), cur) != order.end());

    // Sem ponteiros nulos na ordem.
    for (auto* a : order)
        REQUIRE(a != nullptr);

    // round_index nunca negativo.
    REQUIRE(q.round_index() >= 0);
}

}  // namespace

// ===== INV-9a/b/c/d/f: integridade sob sequencia aleatoria de operacoes =====

TEST_CASE("property: a fila mantem integridade sob qualquer sequencia de operacoes",
          "[domain][combat][property][queue]") {
    for (int c = 0; c < kCasesPerProperty; ++c) {
        Lcg g(0x90000000u + static_cast<unsigned>(c));

        const int n = g.in_range(1, 4);  // party+inimigos 1..4 por lado (secao 2)
        auto pool = make_pool(g, n);
        std::vector<CombatActor*> ptrs;
        for (auto& up : pool) ptrs.push_back(up.get());

        InitiativeQueue q(ptrs);
        check_integrity(q);

        for (int step = 0; step < kStepsPerCase; ++step) {
            const int op = g.in_range(0, 4);
            const CombatActor* before_cur = q.current();
            const int round_before = q.round_index();

            switch (op) {
                case 0: {  // advance
                    const int cnt = q.count();
                    q.advance();
                    // INV-9d: nunca pula rodada; round so sobe (no maximo) 1 por advance.
                    REQUIRE(q.round_index() >= round_before);
                    REQUIRE(q.round_index() - round_before <= 1);
                    (void)cnt;
                    break;
                }
                case 1: {  // reorder_actor (ator presente)
                    CombatActor* who = q.order()[static_cast<std::size_t>(
                        g.in_range(0, q.count() - 1))];
                    const int before_count = q.count();
                    InitiativeQueueRawReorderTestAccess::reorder_actor(q, who, g.in_range(-5, 5));
                    // INV-9f: reorder preserva o CONJUNTO (mesma cardinalidade, ator ainda
                    // presente) e mantem o cursor num indice VALIDO. NOTA: reorder NAO
                    // re-aponta current() por identidade de ator - ele preserva o INDICE do
                    // cursor (contrato initiative_queue.hpp/.cpp); a FSM chama sync_cursor_to
                    // apos reorder pra re-apontar pro ator em turno (ver initiative_queue_test
                    // linhas 209-211). Logo NAO afirmamos current()==before_cur aqui.
                    REQUIRE(q.count() == before_count);
                    REQUIRE(q.contains(who));
                    (void)before_cur;
                    break;
                }
                case 2: {  // mudar SPD de verdade (Haste/Slow) + recompute_by_speed
                    // F2-QA.8 (achado QA 2026-07-28): este ramo se declarava "mudar SPD + "
                    // recompute" mas NUNCA aplicava delta nenhum (fuzz morto, INV-9c passava
                    // por vacuidade). Agora aplica de verdade via apply_stat_delta (mesma
                    // primitiva que Haste/Slow usam em producao) e afirma a propriedade que
                    // a fila existe pra proteger: "cada ator age 1x por rodada" - por
                    // PRESERVACAO DA PARTICAO (COMBATE-FILA-CURSOR-FIX, decisao do lider
                    // 2026-07-27/28), nao mais por ordem global (ver INV-9c revisado acima).
                    const int cursor_before = q.cursor();
                    std::vector<CombatActor*> acted_before(
                        q.order().begin(), q.order().begin() + cursor_before);
                    std::vector<CombatActor*> pending_before(
                        q.order().begin() + cursor_before + 1, q.order().end());

                    // Muda SPD de um ator QUALQUER da fila - inclusive o proprio current, ou
                    // quem ja agiu, ou quem falta: o recompute precisa aguentar todos os
                    // casos, nao so o de producao (onde e sempre o current mudando a propria
                    // SPD). apply_stat_delta so aplica 1x por StatusId; se ja aplicado antes
                    // neste ator, e no-op seguro (delta zero) - a chamada de recompute abaixo
                    // continua valendo como fuzz de "recompute sem mudanca nenhuma".
                    CombatActor* who = q.order()[static_cast<std::size_t>(
                        g.in_range(0, q.count() - 1))];
                    const bool haste = g.in_range(0, 1) == 0;
                    const int magnitude = g.in_range(1, 40);
                    who->apply_stat_delta(haste ? StatusId::Haste : StatusId::Slow, 0,
                                          haste ? +magnitude : -magnitude);

                    q.recompute_by_speed();

                    // current() preservado por IDENTIDADE E por INDICE - o slot do cursor
                    // nunca muda, e e isso que impede um ja-agido de ganhar 2o turno ou um
                    // pendente de ser pulado, pra qualquer SPD sorteada.
                    REQUIRE(q.current() == before_cur);
                    REQUIRE(q.cursor() == cursor_before);

                    // As duas particoes preservam o CONJUNTO (mesmos atores por identidade;
                    // so a ordem interna pode mudar) - ninguem cruza o cursor pro outro lado.
                    std::vector<CombatActor*> acted_after(
                        q.order().begin(), q.order().begin() + cursor_before);
                    std::vector<CombatActor*> pending_after(
                        q.order().begin() + cursor_before + 1, q.order().end());
                    std::sort(acted_before.begin(), acted_before.end());
                    std::sort(acted_after.begin(), acted_after.end());
                    REQUIRE(acted_after == acted_before);
                    std::sort(pending_before.begin(), pending_before.end());
                    std::sort(pending_after.begin(), pending_after.end());
                    REQUIRE(pending_after == pending_before);

                    // Ordem nao-crescente em SPD DENTRO de cada lado (nao mais GLOBAL: o
                    // current pode ficar fora de ordem de SPD em relacao aos dois lados -
                    // e o preco de nao dar 2o turno pra quem ja agiu nem pular quem falta).
                    const auto& order = q.order();
                    for (std::size_t i = 1; i < acted_after.size(); ++i)
                        REQUIRE(order[i - 1]->spd() >= order[i]->spd());
                    for (std::size_t i = static_cast<std::size_t>(cursor_before) + 2;
                        i < order.size(); ++i)
                        REQUIRE(order[i - 1]->spd() >= order[i]->spd());
                    break;
                }
                case 3: {  // remove (so se sobrar >= 1; preserva precondicao de current())
                    // FILA-REMOVE-ROUND-WRAP (achado de auditoria 2026-08-06): este ramo so
                    // afirmava CARDINALIDADE e AUSENCIA - nunca a rodada nem a particao -,
                    // e por isso o fuzz atravessou milhoes de assercoes sem ver que remove()
                    // dava a volta na fila (cursor -> 0) SEM contar rodada quando o current
                    // no ULTIMO slot saia, reabrindo a rodada pra quem ja tinha agido. As
                    // duas propriedades que faltavam entram aqui (INV-9h/INV-9i).
                    if (q.count() > 1) {
                        CombatActor* who = q.order()[static_cast<std::size_t>(
                            g.in_range(0, q.count() - 1))];
                        const int before_count = q.count();
                        const int cursor_before = q.cursor();

                        // Particao ANTES: quem ja agiu ou esta agindo ([0, cursor]) x quem
                        // ainda falta ((cursor, fim]) - menos o proprio removido.
                        std::vector<CombatActor*> acted_before(
                            q.order().begin(), q.order().begin() + cursor_before + 1);
                        std::vector<CombatActor*> pending_before(
                            q.order().begin() + cursor_before + 1, q.order().end());
                        const auto erase_who = [who](std::vector<CombatActor*>& v) {
                            v.erase(std::remove(v.begin(), v.end(), who), v.end());
                        };
                        erase_who(acted_before);
                        erase_who(pending_before);

                        q.remove(who);

                        REQUIRE(q.count() == before_count - 1);
                        REQUIRE_FALSE(q.contains(who));

                        // INV-9h: remove NUNCA conta rodada - advance() e o unico dono do
                        // ++round_index (e o unico ponto onde a FSM ve a fronteira pra
                        // disparar os hooks de fim de rodada).
                        REQUIRE(q.round_index() == round_before);

                        // INV-9j: remover OUTRO ator (nao o current) preserva current() por
                        // IDENTIDADE - quem esta em acao nao perde o turno pro vizinho. E o
                        // que o slide `--cursor_` (removido ANTES do cursor) e o "cursor
                        // intocado" (removido DEPOIS) garantem juntos. Achado por mutation
                        // testing 2026-08-06: apagar o slide sobrevivia a suite inteira,
                        // porque o unico caso unitario de "removido ja-agido" tinha o cursor
                        // no ULTIMO slot - onde a guarda do wrap corrige por acidente.
                        if (who != before_cur)
                            REQUIRE(q.current() == before_cur);

                        // INV-9i: remove PRESERVA A PARTICAO - ninguem atravessa o cursor.
                        // Todo ator ANTES do novo cursor tem de ser um ja-agido; todo ator
                        // DEPOIS tem de ser um pendente. E exatamente o que o wrap mudo
                        // quebrava: com cursor_ = 0, a fila inteira a partir do indice 1
                        // ficava cheia de ja-agidos "reabertos" como pendentes.
                        const auto& after = q.order();
                        const int cursor_after = q.cursor();
                        const auto in = [](const std::vector<CombatActor*>& v,
                                           const CombatActor* a) {
                            return std::find(v.begin(), v.end(), a) != v.end();
                        };
                        for (int i = 0; i < cursor_after; ++i)
                            REQUIRE(in(acted_before, after[static_cast<std::size_t>(i)]));
                        for (int i = cursor_after + 1; i < q.count(); ++i)
                            REQUIRE(in(pending_before, after[static_cast<std::size_t>(i)]));
                    }
                    break;
                }
                default: {  // sync_cursor_to (ator presente)
                    CombatActor* who = q.order()[static_cast<std::size_t>(
                        g.in_range(0, q.count() - 1))];
                    q.sync_cursor_to(who);
                    REQUIRE(q.current() == who);
                    break;
                }
            }

            check_integrity(q);
        }
    }
}

// ===== INV-9d: advance da exatamente uma volta = 1 round, current() volta ao topo =====

TEST_CASE("property: advance em volta completa incrementa round_index exatamente uma vez",
          "[domain][combat][property][queue]") {
    for (int c = 0; c < 500; ++c) {
        Lcg g(0x91000000u + static_cast<unsigned>(c));
        const int n = g.in_range(1, 4);
        auto pool = make_pool(g, n);
        std::vector<CombatActor*> ptrs;
        for (auto& up : pool) ptrs.push_back(up.get());
        InitiativeQueue q(ptrs);

        CombatActor* top = q.current();
        const int round0 = q.round_index();
        // Uma volta = count() advances.
        for (int i = 0; i < q.count(); ++i)
            q.advance();
        REQUIRE(q.round_index() == round0 + 1);
        REQUIRE(q.current() == top);  // de volta ao topo da fila
    }
}

// ===== INV-9e: operacoes sobre ator AUSENTE sao no-op (cursor/conjunto intactos) =====

TEST_CASE("property: remove/sync de ator ausente sao no-op; reorder ausente lanca",
          "[domain][combat][property][queue]") {
    for (int c = 0; c < 500; ++c) {
        Lcg g(0x92000000u + static_cast<unsigned>(c));
        const int n = g.in_range(1, 4);
        auto pool = make_pool(g, n);
        std::vector<CombatActor*> ptrs;
        for (auto& up : pool) ptrs.push_back(up.get());
        InitiativeQueue q(ptrs);

        // Ator fora da fila.
        CombatActor outsider("outsider", "outsider", 50, 5, 2, g.in_range(0, 30),
                             CardFamily::Eletrico, /*player=*/true);

        const int count_before = q.count();
        CombatActor* cur_before = q.current();

        q.remove(&outsider);  // no-op
        REQUIRE(q.count() == count_before);
        REQUIRE(q.current() == cur_before);

        q.sync_cursor_to(&outsider);  // no-op
        REQUIRE(q.current() == cur_before);

        REQUIRE_FALSE(q.contains(&outsider));
        REQUIRE_THROWS_AS(InitiativeQueueRawReorderTestAccess::reorder_actor(q, &outsider, 1),
                         std::invalid_argument);
    }
}

// ===== INV-9g: reorder_pending NUNCA cruza o cursor - identidade de current()/ja-agidos =====
// =====         preservada sob sequencia aleatoria de advance()/reorder_pending() ============
// =====         (COMBATE-FILA-CURSOR-FIX, decisao do lider 2026-07-15) =======================

TEST_CASE("property: reorder_pending preserva por identidade current() e todo ator "
          "ja-agido, sob qualquer sequencia de advance/reorder_pending",
          "[domain][combat][property][queue][pending]") {
    for (int c = 0; c < kCasesPerProperty; ++c) {
        Lcg g(0x93000000u + static_cast<unsigned>(c));

        const int n = g.in_range(2, 6);  // precisa de >=2 pra ter regiao "ja-agida" + pendente
        auto pool = make_pool(g, n);
        std::vector<CombatActor*> ptrs;
        for (auto& up : pool) ptrs.push_back(up.get());

        InitiativeQueue q(ptrs);
        check_integrity(q);

        for (int step = 0; step < kStepsPerCase; ++step) {
            const int op = g.in_range(0, 2);

            if (op == 0) {
                // advance: nunca invalida a regiao [0, cursor], so a estende.
                q.advance();
            } else {
                // Snapshot da regiao "current() + ja-agidos" (indices [0, cursor()]) ANTES.
                const int cursor_before = q.cursor();
                std::vector<CombatActor*> region_before(
                    q.order().begin(), q.order().begin() + cursor_before + 1);

                CombatActor* who = q.order()[static_cast<std::size_t>(
                    g.in_range(0, q.count() - 1))];
                const int delta = g.in_range(-99, 99);

                if (op == 1) {
                    (void)q.reorder_pending(who, delta);  // [[nodiscard]]: so o efeito importa.
                } else {
                    // Metade das iteracoes tambem exercita alvo == current() (guard direto,
                    // sem depender do sorteio bater exatamente no indice do cursor).
                    (void)q.reorder_pending(q.current(), delta);
                }

                // INV-9g: cursor()/round_index intocados e a regiao [0, cursor()] e
                // IDENTICA (mesmos ponteiros, mesma ordem) - reorder_pending nunca escreve
                // ali, para NENHUM alvo/delta.
                REQUIRE(q.cursor() == cursor_before);
                std::vector<CombatActor*> region_after(
                    q.order().begin(), q.order().begin() + cursor_before + 1);
                REQUIRE(region_after == region_before);
            }

            check_integrity(q);
        }
    }
}

// SPDX-License-Identifier: Apache-2.0
// initiative_queue_sync_cursor_guard_test.cpp
//
// FILA-SYNC-CURSOR-GUARDA (2026-08-06). QUARTO portao da MESMA raiz ja fechada tres vezes:
// "cada ator age EXATAMENTE UMA VEZ por rodada da fila".
//   1o portao  reorder_actor       - fechado 2026-07-15 (privatizado no M9, sem caller)
//   2o portao  recompute_by_speed  - fechado 2026-08-01 (initiative_queue_recompute_round_
//                                    invariant_test.cpp)
//   3o portao  remove()            - fechado 2026-08-06 (initiative_queue_remove_round_
//                                    invariant_test.cpp)
//   4o portao  sync_cursor_to()    - ESTE arquivo.
//
// ---------------------------------------------------------------------------------
// O BUG
// ---------------------------------------------------------------------------------
// InitiativeQueue::sync_cursor_to() era um SALTO CRU do cursor, sem guarda nenhuma:
//
//     void InitiativeQueue::sync_cursor_to(CombatActor* actor) {
//         const int idx = index_of(actor);
//         if (idx >= 0)
//             cursor_ = idx;
//     }
//
// O cursor e a FRONTEIRA da particao que a fila inteira existe pra proteger: "todo indice
// <= cursor() ja agiu nesta rodada / todo indice > cursor() esta pendente". Mover o cursor
// por busca de indice atravessa essa fronteira nos DOIS sentidos, e cada sentido reproduz
// um dos dois bugs historicos:
//
//   PRA TRAS (idx < cursor_) - TURNO DUPLO. Todo ator entre o novo e o velho cursor volta a
//     ser "pendente" sem nunca ter perdido o turno: age de novo NA MESMA RODADA, com AP e
//     mana recarregados por refresh_resources_for_turn. E o bug do 3o portao (remove com
//     wrap mudo) e do 2o (recompute cruzando o cursor), pela terceira porta.
//
//   PRA FRENTE (idx > cursor_) - VIZINHO PULADO. Todo ator entre o velho e o novo cursor
//     passa a contar como "ja agiu" sem NUNCA ter agido: e simplesmente pulado na rodada.
//     E, literalmente, o bug do Knockback de 2026-07-15 (reorder_actor(+1) cruzando o
//     cursor), que motivou criar delay_current.
//
// Ou seja: NAO existe direcao segura pra um salto de cursor. "So avanca" fecharia metade do
// buraco e deixaria a outra metade aberta com cara de guarda - por isso o conserto NAO e um
// clamp direcional.
//
// ---------------------------------------------------------------------------------
// ESTADO NO MOMENTO DO ACHADO
// ---------------------------------------------------------------------------------
// INERTE: zero caller de PRODUCAO (varredura de 2026-08-06 - so estes testes e o fuzz
// chamavam). Mas era PUBLICA, e DOIS COMENTARIOS AFIRMAVAM que a maquina de estados a
// chamava ("FSM quando uma reordenacao no proprio tick..." no header; "a FSM chama
// sync_cursor_to apos reorder" no property test) - a FSM NAO chamava, nem chama. Comentario
// que mente convida codigo novo a usar a primitiva errada; os dois foram corrigidos junto
// com este conserto.
//
// ---------------------------------------------------------------------------------
// A SEMANTICA IMPLEMENTADA (o conserto)
// ---------------------------------------------------------------------------------
// sync_cursor_to deixa de MOVER O CURSOR e passa a MOVER O ATOR: delega a
// bring_to_current(), o primitivo que ja realizava com seguranca a mesma intencao
// ("continuar apontando pro ator X") por PERMUTACAO, com o INDICE do cursor fixo. Com isso:
//   - pra tras (ator ja agiu): NO-OP - nao se re-sincroniza pra quem ja jogou sem lhe dar
//     um 2o turno;
//   - pra frente (ator pendente): o ator vem pro slot do cursor e os intermediarios sao
//     empurrados uma casa a frente - todos continuam > cursor, ou seja, seguem PENDENTES;
//   - ator ausente / ator ja e o current: NO-OP (contrato antigo preservado).
// cursor() e round_index() ficam INVARIANTES em toda chamada - a guarda e "o cursor nao se
// move", nao "o cursor so avanca".
//
// Subsistema: domain/combat (InitiativeQueue). POCO puro, headless.

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <functional>
#include <string>
#include <vector>

#include "gus/domain/combat/combat_actor.hpp"
#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/combat/initiative_queue.hpp"
#include "initiative_queue_test_access.hpp"

using namespace gus::domain::combat;
using gus::domain::tests::InitiativeQueueRawReorderTestAccess;

namespace {

CombatActor unit(const std::string& id, int spd) {
    return CombatActor(id, id, /*max_hp=*/50, /*atk=*/5, /*def=*/2, spd, CardFamily::Eletrico,
                       /*player=*/true);
}

std::vector<std::string> order_ids(const InitiativeQueue& q) {
    std::vector<std::string> ids;
    for (const CombatActor* a : q.order())
        ids.push_back(a->id());
    return ids;
}

// Percorre uma rodada INTEIRA da fila (ate round_index subir) gravando quem foi current em
// cada turno. `hook` roda depois de observar o current e antes do advance - e onde os testes
// injetam a chamada sob suspeita. Teto de seguranca: se a rodada nao fechar em `max_turns`,
// para (com o bug, ela nao fecha nunca; o teste falha na contagem, nao em loop infinito).
std::vector<std::string> walk_one_round(InitiativeQueue& q,
                                        const std::function<void(InitiativeQueue&, int)>& hook,
                                        int max_turns = 20) {
    std::vector<std::string> seq;
    const int round0 = q.round_index();
    for (int t = 0; t < max_turns && q.round_index() == round0; ++t) {
        seq.push_back(q.current()->id());
        hook(q, t);
        q.advance();
    }
    return seq;
}

int count_of(const std::vector<std::string>& v, const std::string& id) {
    return static_cast<int>(std::count(v.begin(), v.end(), id));
}

}  // namespace

// ============================================================================
// RED 1 - PRA TRAS: o salto de cursor REABRE a rodada (turno duplo)
// ============================================================================

TEST_CASE("initiative_queue: sync_cursor_to nao move o cursor PRA TRAS (nao reabre a rodada)",
          "[domain][combat][queue][sync-guard]") {
    CombatActor a = unit("a", 30), b = unit("b", 20), c = unit("c", 10);
    InitiativeQueue q({&a, &b, &c});  // [a, b, c], cursor 0
    q.advance();
    q.advance();  // cursor 2 (ultimo slot), current = c; a e b JA AGIRAM nesta rodada
    REQUIRE(q.cursor() == 2);
    REQUIRE(q.round_index() == 0);

    // Re-sincronizar pra `a` (que ja agiu) e exatamente o movimento proibido: com o corpo
    // antigo (`cursor_ = index_of(actor)`) o cursor caia de 2 pra 0 e a rodada REABRIA -
    // a, b e c voltavam a ser "pendentes" e agiam de novo, com AP e mana recarregados.
    q.sync_cursor_to(&a);

    REQUIRE(q.cursor() == 2);                 // <- VERMELHO antes da guarda (era 0)
    REQUIRE(q.current()->id() == "c");        // <- VERMELHO antes da guarda (era "a")
    REQUIRE(q.round_index() == 0);            // remove/sync nunca contam rodada
    REQUIRE(order_ids(q) == std::vector<std::string>{"a", "b", "c"});  // ordem intacta
}

TEST_CASE("initiative_queue: sync_cursor_to pra tras nao da 2o turno a ninguem na rodada",
          "[domain][combat][queue][sync-guard]") {
    // A mesma coisa do caso acima, medida pela CONSEQUENCIA (o que a FSM enxergaria):
    // percorre a rodada 0 inteira chamando sync_cursor_to pro ator do TOPO em todo turno.
    // Com o salto cru, o cursor voltava a 0 a cada chamada e a rodada NUNCA fechava.
    CombatActor a = unit("a", 30), b = unit("b", 20), c = unit("c", 10);
    InitiativeQueue q({&a, &b, &c});
    CombatActor* top = q.order()[0];

    const std::vector<std::string> seq =
        walk_one_round(q, [top](InitiativeQueue& queue, int) { queue.sync_cursor_to(top); });

    // Cada ator abre EXATAMENTE 1 turno na rodada - a invariante-mae.
    REQUIRE(seq.size() == 3u);  // <- VERMELHO antes da guarda (batia no teto de 20)
    REQUIRE(count_of(seq, "a") == 1);
    REQUIRE(count_of(seq, "b") == 1);
    REQUIRE(count_of(seq, "c") == 1);
    REQUIRE(q.round_index() == 1);  // a rodada FECHOU pelo advance(), como tem de ser
}

// ============================================================================
// RED 2 - PRA FRENTE: o salto de cursor PULA o vizinho (bug do Knockback, 2026-07-15)
// ============================================================================

TEST_CASE("initiative_queue: sync_cursor_to nao move o cursor PRA FRENTE (nao pula vizinho)",
          "[domain][combat][queue][sync-guard]") {
    CombatActor a = unit("a", 30), b = unit("b", 20), c = unit("c", 10);
    InitiativeQueue q({&a, &b, &c});  // [a, b, c], cursor 0, current = a

    // O cenario que o doc do header descrevia como caso de uso: `a` leva knockback no
    // PROPRIO turno, a ordem muda, e `a` NAO deve perder o turno.
    InitiativeQueueRawReorderTestAccess::reorder_actor(q, &a, +1);  // [b, a, c], cursor 0
    REQUIRE(order_ids(q) == std::vector<std::string>{"b", "a", "c"});

    q.sync_cursor_to(&a);

    // Com o salto cru, cursor ia pra 1: `b`, parado no indice 0, passava a contar como
    // "ja agiu" sem NUNCA ter agido - PULADO na rodada. O conserto traz o ATOR ate o slot
    // do cursor (permutacao), deixando `b` uma casa a frente e portanto PENDENTE.
    REQUIRE(q.current()->id() == "a");  // a intencao original preservada
    REQUIRE(q.cursor() == 0);           // <- VERMELHO antes da guarda (era 1)
    REQUIRE(q.index_of(&b) > q.cursor());  // <- VERMELHO antes da guarda (b em 0, "ja agiu")
    REQUIRE(order_ids(q) == std::vector<std::string>{"a", "b", "c"});
    REQUIRE(q.round_index() == 0);
}

TEST_CASE("initiative_queue: sync_cursor_to pra frente nao pula ninguem na rodada",
          "[domain][combat][queue][sync-guard]") {
    // A consequencia medida: apos o reorder + sync no 1o turno, a rodada 0 tem de conter
    // os TRES atores. Com o salto cru, `b` sumia da rodada (pulado).
    CombatActor a = unit("a", 30), b = unit("b", 20), c = unit("c", 10);
    InitiativeQueue q({&a, &b, &c});
    CombatActor* victim = &a;

    const std::vector<std::string> seq =
        walk_one_round(q, [victim](InitiativeQueue& queue, int turn) {
            if (turn != 0) return;
            InitiativeQueueRawReorderTestAccess::reorder_actor(queue, victim, +1);
            queue.sync_cursor_to(victim);
        });

    REQUIRE(seq.size() == 3u);
    REQUIRE(count_of(seq, "a") == 1);
    REQUIRE(count_of(seq, "b") == 1);  // <- VERMELHO antes da guarda (era 0: b era pulado)
    REQUIRE(count_of(seq, "c") == 1);
    REQUIRE(q.round_index() == 1);
}

// ============================================================================
// CONTROLES NEGATIVOS - o movimento LEGITIMO continua funcionando
// ============================================================================
// Travam um conserto largo demais: uma guarda que simplesmente transformasse a funcao em
// no-op total passaria nos RED acima e falharia aqui.

TEST_CASE("initiative_queue: sync_cursor_to ainda re-aponta pro ator PENDENTE (caso de uso)",
          "[domain][combat][queue][sync-guard]") {
    CombatActor a = unit("a", 30), b = unit("b", 20), c = unit("c", 10), d = unit("d", 5);
    InitiativeQueue q({&a, &b, &c, &d});
    q.advance();  // cursor 1, current = b; `a` ja agiu

    q.sync_cursor_to(&d);  // `d` esta PENDENTE (indice 3 > cursor 1)

    REQUIRE(q.current()->id() == "d");  // a intencao "current passa a ser d" e cumprida
    REQUIRE(q.cursor() == 1);           // por permutacao, nao por salto
    REQUIRE(order_ids(q) == std::vector<std::string>{"a", "d", "b", "c"});
    // A particao aguenta: `a` (ja agiu) segue antes do cursor; b e c seguem pendentes.
    REQUIRE(q.index_of(&a) < q.cursor());
    REQUIRE(q.index_of(&b) > q.cursor());
    REQUIRE(q.index_of(&c) > q.cursor());
    REQUIRE(q.round_index() == 0);
}

TEST_CASE("initiative_queue: sync_cursor_to pro proprio current e no-op total",
          "[domain][combat][queue][sync-guard]") {
    CombatActor a = unit("a", 30), b = unit("b", 20), c = unit("c", 10);
    InitiativeQueue q({&a, &b, &c});
    q.advance();  // cursor 1, current = b

    q.sync_cursor_to(&b);

    REQUIRE(q.cursor() == 1);
    REQUIRE(q.current()->id() == "b");
    REQUIRE(order_ids(q) == std::vector<std::string>{"a", "b", "c"});
    REQUIRE(q.round_index() == 0);
}

TEST_CASE("initiative_queue: sync_cursor_to de ator AUSENTE segue no-op",
          "[domain][combat][queue][sync-guard]") {
    CombatActor a = unit("a", 30), b = unit("b", 20);
    CombatActor outsider = unit("outsider", 99);
    InitiativeQueue q({&a, &b});
    q.advance();  // cursor 1, current = b

    q.sync_cursor_to(&outsider);

    REQUIRE(q.cursor() == 1);
    REQUIRE(q.current()->id() == "b");
    REQUIRE(q.count() == 2);
    REQUIRE(order_ids(q) == std::vector<std::string>{"a", "b"});
    REQUIRE(q.round_index() == 0);
}

// ============================================================================
// INVARIANTE GERAL - o cursor NUNCA se move num sync, para NENHUM alvo
// ============================================================================

TEST_CASE("initiative_queue: sync_cursor_to preserva cursor e round para QUALQUER alvo e "
          "QUALQUER posicao do cursor",
          "[domain][combat][queue][sync-guard]") {
    // Varredura fechada e pequena: 4 atores x 4 posicoes de cursor x 4 alvos = 48 casos
    // (posicao 0..3 x alvo 0..3, menos nada). Enumerar o espaco todo em vez de sortear
    // dentro dele - o espaco cabe.
    for (int cursor_pos = 0; cursor_pos < 4; ++cursor_pos) {
        for (int target = 0; target < 4; ++target) {
            CombatActor a = unit("a", 40), b = unit("b", 30), c = unit("c", 20),
                        d = unit("d", 10);
            InitiativeQueue q({&a, &b, &c, &d});
            for (int i = 0; i < cursor_pos; ++i)
                q.advance();

            const int cursor_before = q.cursor();
            const int round_before = q.round_index();
            const std::vector<CombatActor*> acted_before(
                q.order().begin(), q.order().begin() + cursor_before);
            CombatActor* who = q.order()[static_cast<std::size_t>(target)];
            const bool who_was_pending = target > cursor_before;

            q.sync_cursor_to(who);

            INFO("cursor_pos=" << cursor_pos << " target=" << target);
            // A guarda: o cursor NAO se move (nem pra tras nem pra frente) e a rodada nao
            // e tocada, para qualquer combinacao.
            REQUIRE(q.cursor() == cursor_before);
            REQUIRE(q.round_index() == round_before);
            // O bloco ja-agido [0, cursor) e IDENTICO (mesmos ponteiros, mesma ordem):
            // ninguem entra nele nem sai dele.
            const std::vector<CombatActor*> acted_after(
                q.order().begin(), q.order().begin() + cursor_before);
            REQUIRE(acted_after == acted_before);
            // Semantica util preservada: alvo pendente vira o current; alvo ja-agido (ou o
            // proprio current, ou ausente) e no-op.
            if (who_was_pending)
                REQUIRE(q.current() == who);
        }
    }
}

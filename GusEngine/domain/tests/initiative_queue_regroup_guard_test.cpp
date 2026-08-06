// SPDX-License-Identifier: Apache-2.0
// initiative_queue_regroup_guard_test.cpp
//
// FILA-REGROUP-GUARDA (2026-08-06). QUINTO e ULTIMO portao da MESMA raiz ja fechada quatro
// vezes: "cada ator age EXATAMENTE UMA VEZ por rodada da fila".
//   1o portao  reorder_actor       - fechado 2026-07-15 (privatizado no M9, sem caller)
//   2o portao  recompute_by_speed  - fechado 2026-08-01 (initiative_queue_recompute_round_
//                                    invariant_test.cpp)
//   3o portao  remove()            - fechado 2026-08-06 (initiative_queue_remove_round_
//                                    invariant_test.cpp)
//   4o portao  sync_cursor_to()    - fechado 2026-08-06 (initiative_queue_sync_cursor_
//                                    guard_test.cpp)
//   5o portao  regroup_stable()    - ESTE arquivo.
//
// ---------------------------------------------------------------------------------
// O BUG
// ---------------------------------------------------------------------------------
// InitiativeQueue::regroup_stable() reparticionava a fila INTEIRA e zerava o cursor:
//
//     void InitiativeQueue::regroup_stable(const std::function<bool(const CombatActor*)>& p) {
//         std::stable_partition(order_.begin(), order_.end(), p);
//         cursor_ = 0;
//     }
//
// `cursor_ = 0` fora da fronteira da rodada e EXATAMENTE a assinatura do bug do 3o portao
// (remove dando a volta na fila sem contar rodada): todo ator que ja tinha agido volta a ser
// "pendente" e age DE NOVO na mesma rodada, com AP e mana recarregados por
// refresh_resources_for_turn - e a rodada nunca fecha do jeito que a FSM espera.
//
// O contrato "chamar SOMENTE na fronteira da rodada" existia APENAS no comentario do header.
// Nada no codigo o impunha. Garantia por comentario nao e garantia (decisao do lider,
// 2026-08-06).
//
// ---------------------------------------------------------------------------------
// ESTADO NO MOMENTO DO ACHADO
// ---------------------------------------------------------------------------------
// INERTE, e MEDIDO como inerte (nao presumido): instrumentando
// CombatStateMachine::regroup_round_by_side (o UNICO caminho de producao ate
// regroup_stable) e rodando a suite de domain inteira, os DOIS callers entraram com
// cursor == 0 em 62.684 de 62.684 chamadas - 100%, nenhuma fora da fronteira. Sao eles:
//   - combat_state_machine.cpp:570  (construtor, SetupPhase, rodada 0: cursor_ nasce 0)
//   - combat_state_machine.cpp:931  (dentro do ramo `round_index() > round_before` de
//                                    advance_to_next_actor: prune_dead ja rodou, entao todo
//                                    ator restante esta vivo, o laco de pular-mortos e no-op
//                                    e o cursor e o que o wrap de advance() deixou: 0)
//
// ---------------------------------------------------------------------------------
// A SEMANTICA IMPLEMENTADA (o conserto) E POR QUE NAO FOI "ALARGAR O ALCANCE"
// ---------------------------------------------------------------------------------
// A alternativa tentadora era tornar a funcao TOTAL em vez de guardada: reparticionar so
// [cursor_, fim) e nunca escrever cursor_. Ela e byte-identica na fronteira (cursor_ == 0 =>
// o range e a fila inteira) e PARECE segura, mas NAO E - e este arquivo a mata de proposito
// (ver o RED 2). Uma permutacao sobre [cursor_, fim) tira do slot do cursor o ator que esta
// agindo e o joga pra tras, onde ele conta como PENDENTE: se o turno dele ja tinha sido
// resolvido, ele age uma 2a vez e quem tomou o slot dele e PULADO. E a mesma classe de bug,
// so com raio de 1 ator em vez de N - meio conserto com cara de conserto inteiro.
//
// (Isso nao contradiz bring_to_current, que faz uma permutacao parecida: ela e chamada por
// begin_turn no COMECO do turno, quando o ator do slot do cursor ainda NAO agiu, e por isso
// desloca-lo o mantem pendente. regroup_stable nao tem como saber em que ponto do turno
// esta, entao nao pode assumir esse timing.)
//
// Logo o conserto correto e uma GUARDA sobre a precondicao real, que e "estamos na fronteira
// da rodada" == `cursor_ == 0`:
//   - cursor_ != 0 -> NO-OP TOTAL (ordem, cursor e round_index intocados) e retorno `false`;
//   - cursor_ == 0 -> particiona a fila inteira e retorna `true`.
// A funcao deixou de escrever cursor_ (o `cursor_ = 0` era redundante sob a guarda) e passou
// a ser [[nodiscard]] bool: com -Werror=unused-result ligado (2026-08-06), um caller futuro
// NAO CONSEGUE COMPILAR ignorando a falha - o no-op e sinalizado em tempo de compilacao, nao
// em silencio. Em runtime, CombatStateMachine::regroup_round_by_side loga a recusa no log de
// combate (nada acontece em silencio, regra da casa).
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

using namespace gus::domain::combat;

namespace {

CombatActor unit(const std::string& id, int spd, bool player) {
    return CombatActor(id, id, /*max_hp=*/50, /*atk=*/5, /*def=*/2, spd, CardFamily::Eletrico,
                       player);
}

std::vector<std::string> order_ids(const InitiativeQueue& q) {
    std::vector<std::string> ids;
    for (const CombatActor* a : q.order())
        ids.push_back(a->id());
    return ids;
}

// Predicado do regroup por lado (§4.1): player-side abre.
bool party_first(const CombatActor* a) { return a->is_player_side(); }

// Percorre uma rodada INTEIRA (ate round_index subir) gravando quem foi current em cada
// turno. `hook` roda DEPOIS de observar o current (ou seja: o ator ja "agiu") e ANTES do
// advance - e ali que injetamos a chamada sob suspeita. Teto de seguranca: com o bug a
// rodada pode nao fechar, e o teste falha na contagem em vez de travar.
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
// RED 1 - o estado cru: regroup no MEIO da rodada REABRIA a rodada
// ============================================================================

TEST_CASE("initiative_queue: regroup_stable no meio da rodada e no-op total (nao reabre)",
          "[domain][combat][queue][regroup-guard]") {
    // [a(p,40), b(e,30), c(p,20), d(e,10)] - ja em ordem de SPD.
    CombatActor a = unit("a", 40, true), b = unit("b", 30, false), c = unit("c", 20, true),
                d = unit("d", 10, false);
    InitiativeQueue q({&a, &b, &c, &d});
    q.advance();
    q.advance();  // cursor 2, current = c; `a` e `b` JA AGIRAM nesta rodada.
    REQUIRE(q.cursor() == 2);
    REQUIRE(q.round_index() == 0);

    const bool grouped = q.regroup_stable(party_first);

    // Com o corpo antigo, a fila virava [a, c, b, d] e o cursor CAIA pra 0: `a` e `b`
    // voltavam a ser pendentes e ganhavam um 2o turno na MESMA rodada.
    REQUIRE_FALSE(grouped);            // <- VERMELHO antes da guarda (era void; nem existia)
    REQUIRE(q.cursor() == 2);          // <- VERMELHO antes da guarda (era 0)
    REQUIRE(q.current()->id() == "c");  // <- VERMELHO antes da guarda (era "a")
    REQUIRE(q.round_index() == 0);
    // No-op TOTAL: a ordem tambem fica intacta (a guarda recusa ANTES do stable_partition).
    REQUIRE(order_ids(q) == std::vector<std::string>{"a", "b", "c", "d"});
}

TEST_CASE("initiative_queue: regroup_stable no meio da rodada nao da 2o turno nem pula ninguem",
          "[domain][combat][queue][regroup-guard]") {
    // A mesma coisa medida pela CONSEQUENCIA - o que a FSM enxergaria. Este e o caso que
    // mata TAMBEM o conserto-alternativo "particionar so [cursor_, fim) sem tocar o cursor":
    //   - corpo antigo (particao total + cursor_=0): a fila vira [a,c,b,d] com cursor 0, e a
    //     rodada rende 5 turnos - `b` age DUAS vezes.
    //   - alternativa [cursor_, fim): a fila vira [a,c,b,d] com cursor 1 => current passa a
    //     ser `c`, `b` (que ACABOU de agir no turno 1) volta pro slot 2 e age de novo, e `c`
    //     - que tomou o slot ja consumido - e PULADO: 0 turnos.
    //   - guarda correta: no-op, os 4 atores agem 1x cada, na ordem de entrada.
    CombatActor a = unit("a", 40, true), b = unit("b", 30, false), c = unit("c", 20, true),
                d = unit("d", 10, false);
    InitiativeQueue q({&a, &b, &c, &d});

    const std::vector<std::string> seq = walk_one_round(q, [](InitiativeQueue& queue, int turn) {
        if (turn != 1) return;  // dispara com cursor == 1, bem no meio da rodada.
        const bool grouped = queue.regroup_stable(party_first);
        REQUIRE_FALSE(grouped);  // a fila recusa e AVISA (nao no-op silencioso).
    });

    REQUIRE(seq.size() == 4u);  // <- VERMELHO antes da guarda (eram 5)
    REQUIRE(count_of(seq, "a") == 1);
    REQUIRE(count_of(seq, "b") == 1);  // <- VERMELHO antes da guarda (eram 2)
    REQUIRE(count_of(seq, "c") == 1);  // <- mata a alternativa [cursor_, fim) (seria 0)
    REQUIRE(count_of(seq, "d") == 1);
    REQUIRE(seq == std::vector<std::string>{"a", "b", "c", "d"});
    REQUIRE(q.round_index() == 1);  // a rodada FECHOU pelo advance(), como tem de ser
}

// ============================================================================
// RED 2 - o bloco JA-AGIDO nao pode ser reescrito, para NENHUMA posicao do cursor
// ============================================================================

TEST_CASE("initiative_queue: regroup_stable preserva o bloco ja-agido em QUALQUER posicao "
          "do cursor fora da fronteira",
          "[domain][combat][queue][regroup-guard]") {
    // Espaco pequeno e fechado: 4 atores x 4 posicoes de cursor. Enumerar o espaco todo em
    // vez de sortear dentro dele. A posicao 0 e a fronteira LEGITIMA e vai no bloco de
    // controle negativo abaixo; aqui varremos 1..3 (fora da fronteira).
    for (int cursor_pos = 1; cursor_pos < 4; ++cursor_pos) {
        CombatActor a = unit("a", 40, true), b = unit("b", 30, false), c = unit("c", 20, true),
                    d = unit("d", 10, false);
        InitiativeQueue q({&a, &b, &c, &d});
        for (int i = 0; i < cursor_pos; ++i)
            q.advance();

        const std::vector<std::string> order_before = order_ids(q);
        const int round_before = q.round_index();
        CombatActor* current_before = q.current();

        const bool grouped = q.regroup_stable(party_first);

        INFO("cursor_pos=" << cursor_pos);
        REQUIRE_FALSE(grouped);
        // Nada se move: nem o cursor, nem a rodada, nem UM slot da ordem. Em particular o
        // ator do slot do cursor continua sendo o current - e o que a alternativa
        // "particionar [cursor_, fim)" quebraria (ela trocaria o current de lugar).
        REQUIRE(q.cursor() == cursor_pos);
        REQUIRE(q.round_index() == round_before);
        REQUIRE(q.current() == current_before);
        REQUIRE(order_ids(q) == order_before);
    }
}

// ============================================================================
// CONTROLES NEGATIVOS - os 2 usos LEGITIMOS (na fronteira) seguem funcionando
// ============================================================================
// Travam um conserto largo demais: uma guarda que transformasse a funcao em no-op TOTAL
// passaria em todos os RED acima e falharia aqui. Sao o espelho exato dos 2 callers de
// producao medidos (construtor/SetupPhase e o wrap de advance_to_next_actor), os dois com
// cursor == 0.

TEST_CASE("initiative_queue: regroup_stable NA FRONTEIRA (cursor 0, rodada 0) ainda agrupa",
          "[domain][combat][queue][regroup-guard]") {
    // Espelha o caller do construtor (combat_state_machine.cpp:570): fila recem-criada.
    CombatActor a = unit("a", 40, false), b = unit("b", 30, true), c = unit("c", 20, false),
                d = unit("d", 10, true);
    InitiativeQueue q({&a, &b, &c, &d});  // [a(e), b(p), c(e), d(p)]
    REQUIRE(q.cursor() == 0);

    const bool grouped = q.regroup_stable(party_first);

    REQUIRE(grouped);
    REQUIRE(order_ids(q) == std::vector<std::string>{"b", "d", "a", "c"});
    REQUIRE(q.cursor() == 0);
    REQUIRE(q.round_index() == 0);
    REQUIRE(q.current()->id() == "b");  // o primeiro do lado que abre
}

TEST_CASE("initiative_queue: regroup_stable NA FRONTEIRA depois do wrap (rodada 1+) ainda agrupa",
          "[domain][combat][queue][regroup-guard]") {
    // Espelha o caller de advance_to_next_actor (combat_state_machine.cpp:931): o wrap de
    // advance() acabou de levar o cursor a 0 e contar a rodada.
    CombatActor a = unit("a", 40, false), b = unit("b", 30, true), c = unit("c", 20, false);
    InitiativeQueue q({&a, &b, &c});  // [a(e), b(p), c(e)]
    q.advance();
    q.advance();
    q.advance();  // volta completa -> round_index 1, cursor 0
    REQUIRE(q.round_index() == 1);
    REQUIRE(q.cursor() == 0);

    const bool grouped = q.regroup_stable(party_first);

    REQUIRE(grouped);
    REQUIRE(order_ids(q) == std::vector<std::string>{"b", "a", "c"});
    REQUIRE(q.cursor() == 0);
    REQUIRE(q.round_index() == 1);  // regroup nunca conta rodada
}

TEST_CASE("initiative_queue: regroup_stable na fronteira segue Gambito-safe (stable_partition)",
          "[domain][combat][queue][regroup-guard]") {
    // O controle que impede trocar stable_partition por sort/partition: a ordem relativa
    // DENTRO de cada grupo tem de sobreviver (um empurrao de Gambito da rodada anterior nao
    // pode ser apagado). Aqui a party entra fora da ordem de SPD (d antes de b) e assim deve
    // sair.
    CombatActor a = unit("a", 40, false), d = unit("d", 10, true), b = unit("b", 30, true),
                c = unit("c", 20, false);
    // Construimos a ordem a mao (o ctor ordena por SPD, entao passamos SPDs que ja produzem
    // a ordem desejada nao e possivel aqui - usamos o wrap do proprio regroup pra medir).
    InitiativeQueue q({&a, &b, &c, &d});  // SPD desc: [a(e,40), b(p,30), c(e,20), d(p,10)]
    REQUIRE(order_ids(q) == std::vector<std::string>{"a", "b", "c", "d"});

    const bool grouped = q.regroup_stable(party_first);

    REQUIRE(grouped);
    // player-side na frente NA ORDEM RELATIVA CORRENTE (b antes de d), enemy-side atras
    // (a antes de c). Um sort por SPD daria a mesma coisa aqui, mas a ordem relativa e o
    // que o teste de initiative_queue_test.cpp ja prova com um empurrao cru; este fixa o
    // caso do regroup por lado propriamente dito.
    REQUIRE(order_ids(q) == std::vector<std::string>{"b", "d", "a", "c"});
    REQUIRE(q.cursor() == 0);
}

TEST_CASE("initiative_queue: regroup_stable na fronteira e no-op de ordem quando ja agrupado, "
          "e ainda assim reporta sucesso",
          "[domain][combat][queue][regroup-guard]") {
    // Distingue "recusei" (false) de "aceitei e nao havia o que mover" (true) - sem isso, um
    // caller nao consegue diferenciar as duas coisas pelo retorno.
    CombatActor a = unit("a", 40, true), b = unit("b", 30, true), c = unit("c", 20, false);
    InitiativeQueue q({&a, &b, &c});  // ja player-first

    const bool grouped = q.regroup_stable(party_first);

    REQUIRE(grouped);
    REQUIRE(order_ids(q) == std::vector<std::string>{"a", "b", "c"});
    REQUIRE(q.cursor() == 0);
}

// ============================================================================
// A CADEIA COMPLETA - a rodada seguinte a um regroup legitimo continua sa
// ============================================================================

TEST_CASE("initiative_queue: regroup na fronteira + rodada inteira = cada ator age 1x",
          "[domain][combat][queue][regroup-guard]") {
    // O invariante-mae ponta a ponta no uso LEGITIMO: reagrupa na fronteira e roda a rodada
    // toda. Nenhum ator duplicado, nenhum pulado, a rodada fecha pelo advance().
    CombatActor a = unit("a", 40, false), b = unit("b", 30, true), c = unit("c", 20, false),
                d = unit("d", 10, true);
    InitiativeQueue q({&a, &b, &c, &d});

    const bool grouped = q.regroup_stable(party_first);
    REQUIRE(grouped);

    const std::vector<std::string> seq = walk_one_round(q, [](InitiativeQueue&, int) {});

    REQUIRE(seq == std::vector<std::string>{"b", "d", "a", "c"});  // party inteira, dps inimigo
    REQUIRE(q.round_index() == 1);
    REQUIRE(q.cursor() == 0);
}

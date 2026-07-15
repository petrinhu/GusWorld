// initiative_queue_pending_test.cpp
//
// Spec executavel (Catch2 v3) das primitivas SEGURAS de reordenacao intra-rodada
// (COMBATE-FILA-CURSOR-FIX, decisao do lider 2026-07-15, A1/A2/A3): reorder_pending e
// delay_current. Cobrem a RAIZ dos 2 bugs QA-confirmados (GambitReorder duplicando o
// ator + Knockback pulando o vizinho): InitiativeQueue::reorder_actor fazia erase+insert
// SEM guard de cursor, podendo cruzar current() e reescrever a regiao [0, cursor()],
// desincronizando identidade de quem esta em acao ou ja agiu.
//
// reorder_pending: clamp em [cursor()+1, count()-1] - NUNCA cruza o cursor. Alvo == current()
// ou ja agiu (indice <= cursor()) e no-op (retorna 0, dissipacao - o caller loga).
// delay_current: adia o TURNO do ator no slot do cursor (o vizinho pendente vira current()),
// SEM alterar o indice do cursor nem round_index.
//
// reorder_actor (a primitiva CRUA antiga) continua coberta por initiative_queue_test.cpp;
// NAO foi alterada neste PR (A3: privatizacao adiada pro M9, 6+ testes dependem do
// comportamento cru sem guard de cursor).
//
// Cross-ref: gus/domain/combat/initiative_queue.hpp/.cpp;
//            docs/design/mecanicas/combat.md secao 4/8/9;
//            domain/src/combat/combat_state_machine.cpp (resolve_gambit_reorder/begin_turn);
//            domain/src/combat/techmagic.cpp (handle_delay_action).

#include <catch2/catch_test_macros.hpp>

#include <stdexcept>
#include <string>
#include <vector>

#include "gus/domain/combat/combat_actor.hpp"
#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/combat/initiative_queue.hpp"

using namespace gus::domain::combat;

namespace {

CombatActor actor(const std::string& id, int spd, bool player = true) {
    return CombatActor(id, id, /*max_hp=*/20, /*atk=*/5, /*def=*/2, spd,
                       CardFamily::Eletrico, player);
}

std::vector<std::string> order_ids(const InitiativeQueue& q) {
    std::vector<std::string> ids;
    for (const CombatActor* a : q.order())
        ids.push_back(a->id());
    return ids;
}

}  // namespace

// ---- reorder_pending: adianta/atrasa dentro do range pendente ---------------------

TEST_CASE("initiative_queue: reorder_pending atrasa um alvo pendente",
          "[domain][combat][queue][pending]") {
    CombatActor a = actor("a", 30), b = actor("b", 20), c = actor("c", 10);
    InitiativeQueue q({&a, &b, &c});  // [a, b, c], cursor 0
    const int applied = q.reorder_pending(&b, +1);
    REQUIRE(applied == 1);
    REQUIRE(order_ids(q) == std::vector<std::string>{"a", "c", "b"});
    REQUIRE(q.current()->id() == "a");  // cursor intocado.
}

TEST_CASE("initiative_queue: reorder_pending adianta um alvo pendente (sem cruzar o cursor)",
          "[domain][combat][queue][pending]") {
    CombatActor a = actor("a", 30), b = actor("b", 20), c = actor("c", 10), d = actor("d", 5);
    InitiativeQueue q({&a, &b, &c, &d});  // [a, b, c, d], cursor 0
    q.advance();  // cursor 1 (current = b; a "ja agiu")
    const int applied = q.reorder_pending(&d, -2);
    // d pede -2 (indice 3 -> 1), mas o clamp inferior cursor()+1=2 barra em 2: applied =
    // to(2) - from(3) = -1, NAO o -2 pedido.
    REQUIRE(applied == -1);
    REQUIRE(order_ids(q) == std::vector<std::string>{"a", "b", "d", "c"});
    REQUIRE(q.current()->id() == "b");  // cursor/identidade intocados.
}

// ---- reorder_pending: clamp NUNCA cruza o cursor -----------------------------------

TEST_CASE("initiative_queue: reorder_pending clamp inferior para exatamente em cursor()+1 "
          "(delta grande demais para tras)",
          "[domain][combat][queue][pending]") {
    CombatActor a = actor("a", 30), b = actor("b", 20), c = actor("c", 10), d = actor("d", 5);
    InitiativeQueue q({&a, &b, &c, &d});
    q.advance();  // cursor 1 (current = b)
    const int applied = q.reorder_pending(&d, -99);  // pede adiantar 99 -> clamp em cursor+1=2
    REQUIRE(order_ids(q) == std::vector<std::string>{"a", "b", "d", "c"});
    REQUIRE(applied == 2 - 3);  // to(2) - from(3) = -1, NAO -99 (efetivamente aplicado).
    REQUIRE(q.current()->id() == "b");
    REQUIRE(order_ids(q)[0] == "a");  // slot 0 (< cursor, "ja agiu") jamais e tocado.
}

TEST_CASE("initiative_queue: reorder_pending clamp superior no ultimo slot (delta grande "
          "demais pra frente)",
          "[domain][combat][queue][pending]") {
    CombatActor a = actor("a", 30), b = actor("b", 20), c = actor("c", 10);
    InitiativeQueue q({&a, &b, &c});  // cursor 0
    const int applied = q.reorder_pending(&b, +99);
    REQUIRE(order_ids(q) == std::vector<std::string>{"a", "c", "b"});
    REQUIRE(applied == 1);  // b sai do indice 1 pro ultimo (indice 2): +1, nao +99.
}

// ---- reorder_pending: alvo == current() OU ja agiu -> no-op (0), SEM lancar -------

TEST_CASE("initiative_queue: reorder_pending no alvo == current() e no-op (0)",
          "[domain][combat][queue][pending]") {
    CombatActor a = actor("a", 30), b = actor("b", 20), c = actor("c", 10);
    InitiativeQueue q({&a, &b, &c});
    const std::vector<CombatActor*> before = q.order();
    const int applied = q.reorder_pending(&a, +1);  // a e o current().
    REQUIRE(applied == 0);
    REQUIRE(q.order() == before);
}

TEST_CASE("initiative_queue: reorder_pending num alvo que ja agiu (indice < cursor) e "
          "no-op (0)",
          "[domain][combat][queue][pending]") {
    CombatActor a = actor("a", 30), b = actor("b", 20), c = actor("c", 10);
    InitiativeQueue q({&a, &b, &c});
    q.advance();  // cursor 1 (current = b); a "ja agiu"
    const std::vector<CombatActor*> before = q.order();
    const int applied = q.reorder_pending(&a, +1);
    REQUIRE(applied == 0);
    REQUIRE(q.order() == before);
    REQUIRE(q.current()->id() == "b");
}

// ---- reorder_pending: ator ausente lanca ------------------------------------------

TEST_CASE("initiative_queue: reorder_pending de ator ausente lanca invalid_argument",
          "[domain][combat][queue][pending]") {
    CombatActor a = actor("a", 30);
    InitiativeQueue q({&a});
    CombatActor estranho = actor("z", 99);
    REQUIRE_THROWS_AS(q.reorder_pending(&estranho, +1), std::invalid_argument);
}

// ---- INVARIANTE DE IDENTIDADE: current() e todo ator ja-agido (indice <= cursor()) ====
// ==== permanecem, POR IDENTIDADE, EXATAMENTE OS MESMOS antes/depois - mata a RAIZ do ====
// ==== bug (reorder_actor cruzava o cursor e reescrevia essa regiao) ====================

TEST_CASE("initiative_queue: reorder_pending PRESERVA por identidade quem e current() e "
          "quem ja agiu, para qualquer alvo/delta pendente",
          "[domain][combat][queue][pending]") {
    CombatActor a = actor("a", 40), b = actor("b", 30), c = actor("c", 20), d = actor("d", 10),
               e = actor("e", 5);
    InitiativeQueue q({&a, &b, &c, &d, &e});  // [a, b, c, d, e]
    q.advance();
    q.advance();  // cursor 2 (current = c); a, b "ja agiram"
    const std::vector<CombatActor*> already_acted_and_current = {q.order()[0], q.order()[1],
                                                                  q.order()[2]};

    for (int delta : {-99, -3, -1, 0, 1, 3, 99}) {
        (void)q.reorder_pending(&e, delta);  // [[nodiscard]]: so o efeito colateral importa aqui.
        REQUIRE(q.order()[0] == already_acted_and_current[0]);
        REQUIRE(q.order()[1] == already_acted_and_current[1]);
        REQUIRE(q.order()[2] == already_acted_and_current[2]);
        REQUIRE(q.current() == already_acted_and_current[2]);
        REQUIRE(q.cursor() == 2);
    }
}

// ---- delay_current: basico + edge do ultimo slot -----------------------------------

TEST_CASE("initiative_queue: delay_current(1) adia o corrente - o vizinho pendente vira "
          "current(), cursor/round_index intocados",
          "[domain][combat][queue][pending]") {
    CombatActor a = actor("a", 30), b = actor("b", 20), c = actor("c", 10);
    InitiativeQueue q({&a, &b, &c});  // [a, b, c], cursor 0
    const bool delayed = q.delay_current(1);
    REQUIRE(delayed);
    REQUIRE(order_ids(q) == std::vector<std::string>{"b", "a", "c"});
    REQUIRE(q.current()->id() == "b");  // vizinho vira current().
    REQUIRE(q.cursor() == 0);           // indice do cursor intocado.
    REQUIRE(q.round_index() == 0);      // nao e uma volta de fila.
}

TEST_CASE("initiative_queue: delay_current no ULTIMO slot da fila e no-op (false), nada a "
          "adiar",
          "[domain][combat][queue][pending]") {
    CombatActor a = actor("a", 30), b = actor("b", 20), c = actor("c", 10);
    InitiativeQueue q({&a, &b, &c});
    q.advance();
    q.advance();  // cursor 2 (current = c, ULTIMO slot)
    const std::vector<CombatActor*> before = q.order();
    const bool delayed = q.delay_current(1);
    REQUIRE_FALSE(delayed);
    REQUIRE(q.order() == before);
    REQUIRE(q.current()->id() == "c");  // permanece current(): "age agora mesmo".
    REQUIRE(q.cursor() == 2);
}

TEST_CASE("initiative_queue: delay_current(1) chamado 2x seguidas no slot do cursor "
          "devolve a fila a ordem original (o par current()/vizinho troca de lugar 2x)",
          "[domain][combat][queue][pending]") {
    // Nota: isto NAO simula 2 KNOCKBACKS distintos (o caller real - begin_turn - so chama
    // delay_current de novo se o NOVO current() tambem tiver Knockback, um ator DIFERENTE;
    // a cadeia real de 2 atores distintos com Knockback e coberta em
    // combat_status_effects_test.cpp/combat_inc3_test.cpp, nivel FSM). Aqui e so a
    // primitiva crua chamada 2x seguidas SEM remover status - documenta que, com n=1, o
    // par current()/vizinho imediato faz "swap-e-volta".
    CombatActor a = actor("a", 40), b = actor("b", 30), c = actor("c", 20), d = actor("d", 10);
    InitiativeQueue q({&a, &b, &c, &d});  // [a, b, c, d], cursor 0
    REQUIRE(q.delay_current(1));
    REQUIRE(order_ids(q) == std::vector<std::string>{"b", "a", "c", "d"});
    REQUIRE(q.current()->id() == "b");
    REQUIRE(q.delay_current(1));  // adia b (agora no cursor) de volta pra tras de a.
    REQUIRE(order_ids(q) == std::vector<std::string>{"a", "b", "c", "d"});  // ordem original.
    REQUIRE(q.current()->id() == "a");
    REQUIRE(q.cursor() == 0);
}

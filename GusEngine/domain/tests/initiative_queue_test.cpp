// initiative_queue_test.cpp
//
// Spec executavel (Catch2 v3) da fila de iniciativa visivel (CTB-style, secao 4),
// portada de engine/foundation/turn_combat/InitiativeQueue.cs (origem canonica) e dos
// casos de engine/tests/turn_combat/InitiativeQueueTests.cs (xUnit = SPEC). POCO puro,
// ZERO Qt, headless. Marco M5 (chunk 2). Paridade 1:1 com o C#.
//
// PORTE DE REFERENCIA: o C# guarda referencias a CombatActor (class). No C++ a fila
// guarda ponteiros nao-donos (CombatActor*). Os atores vivem no escopo do teste/FSM e
// a fila apenas os ordena/aponta. Ordenacao estavel por SPD desc (std::stable_sort
// espelha OrderByDescending estavel do LINQ).
//
// Cross-ref: engine/foundation/turn_combat/InitiativeQueue.cs;
//            engine/tests/turn_combat/InitiativeQueueTests.cs;
//            docs/design/mecanicas/combat.md secao 3/4.

#include <catch2/catch_test_macros.hpp>

#include <stdexcept>
#include <string>
#include <vector>

#include "gus/domain/combat/combat_actor.hpp"
#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/combat/initiative_queue.hpp"

using namespace gus::domain::combat;

namespace {

// Espelha o helper Actor de InitiativeQueueTests.cs.
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

// ---- Ordenacao por SPD desc (Ordena_por_spd_descendente) --------------------------

TEST_CASE("initiative_queue: ordena por spd descendente",
          "[domain][combat][queue]") {
    CombatActor slow = actor("slow", 5);
    CombatActor fast = actor("fast", 20);
    CombatActor mid = actor("mid", 12);

    InitiativeQueue q({&slow, &fast, &mid});

    REQUIRE(order_ids(q) == std::vector<std::string>{"fast", "mid", "slow"});
}

TEST_CASE("initiative_queue: empate de spd e estavel na ordem de entrada",
          "[domain][combat][queue]") {
    CombatActor a = actor("a", 10);
    CombatActor b = actor("b", 10);
    CombatActor c = actor("c", 10);

    InitiativeQueue q({&a, &b, &c});

    REQUIRE(order_ids(q) == std::vector<std::string>{"a", "b", "c"});
}

TEST_CASE("initiative_queue: order e snapshot visivel com count",
          "[domain][combat][queue]") {
    CombatActor a = actor("a", 10);
    CombatActor b = actor("b", 5);
    InitiativeQueue q({&a, &b});
    REQUIRE(q.order().size() == 2);
    REQUIRE(q.count() == 2);
}

TEST_CASE("initiative_queue: fila vazia e rejeitada",
          "[domain][combat][queue]") {
    REQUIRE_THROWS_AS(InitiativeQueue(std::vector<CombatActor*>{}), std::invalid_argument);
}

// ---- ReorderActor (clamp, delta) --------------------------------------------------

TEST_CASE("initiative_queue: reorder_actor adianta com delta negativo",
          "[domain][combat][queue]") {
    CombatActor a = actor("a", 30), b = actor("b", 20), c = actor("c", 10);
    InitiativeQueue q({&a, &b, &c});
    // ordem inicial: a, b, c. Adianta c em 2 (-2) -> c, a, b
    q.reorder_actor(q.order()[2], -2);
    REQUIRE(order_ids(q) == std::vector<std::string>{"c", "a", "b"});
}

TEST_CASE("initiative_queue: reorder_actor atrasa com delta positivo",
          "[domain][combat][queue]") {
    CombatActor a = actor("a", 30), b = actor("b", 20), c = actor("c", 10);
    InitiativeQueue q({&a, &b, &c});
    // atrasa a em 1 (+1) -> b, a, c (knockback tipico)
    q.reorder_actor(q.order()[0], +1);
    REQUIRE(order_ids(q) == std::vector<std::string>{"b", "a", "c"});
}

TEST_CASE("initiative_queue: reorder_actor clamp no inicio",
          "[domain][combat][queue]") {
    CombatActor a = actor("a", 30), b = actor("b", 20), c = actor("c", 10);
    InitiativeQueue q({&a, &b, &c});
    // tenta adiantar b em -5 (alem do indice 0) -> clamp pro topo: b, a, c
    q.reorder_actor(q.order()[1], -5);
    REQUIRE(order_ids(q) == std::vector<std::string>{"b", "a", "c"});
}

TEST_CASE("initiative_queue: reorder_actor clamp no fim",
          "[domain][combat][queue]") {
    CombatActor a = actor("a", 30), b = actor("b", 20), c = actor("c", 10);
    InitiativeQueue q({&a, &b, &c});
    // tenta atrasar a em +99 (alem do ultimo) -> clamp pro fim: b, c, a
    q.reorder_actor(q.order()[0], +99);
    REQUIRE(order_ids(q) == std::vector<std::string>{"b", "c", "a"});
}

TEST_CASE("initiative_queue: reorder_actor delta zero e noop",
          "[domain][combat][queue]") {
    CombatActor a = actor("a", 30), b = actor("b", 20);
    InitiativeQueue q({&a, &b});
    q.reorder_actor(q.order()[0], 0);
    REQUIRE(order_ids(q) == std::vector<std::string>{"a", "b"});
}

TEST_CASE("initiative_queue: reorder_actor ator inexistente lanca",
          "[domain][combat][queue]") {
    CombatActor a = actor("a", 30);
    InitiativeQueue q({&a});
    CombatActor estranho = actor("z", 99);
    REQUIRE_THROWS_AS(q.reorder_actor(&estranho, +1), std::invalid_argument);
}

// ---- Current / Advance (ciclico + RoundIndex) -------------------------------------

TEST_CASE("initiative_queue: current aponta pro primeiro e advance avanca ciclico",
          "[domain][combat][queue]") {
    CombatActor a = actor("a", 30), b = actor("b", 20), c = actor("c", 10);
    InitiativeQueue q({&a, &b, &c});
    REQUIRE(q.current()->id() == "a");
    q.advance();
    REQUIRE(q.current()->id() == "b");
    q.advance();
    REQUIRE(q.current()->id() == "c");
    q.advance();  // volta ao topo (nova rodada)
    REQUIRE(q.current()->id() == "a");
}

TEST_CASE("initiative_queue: advance incrementa round_index a cada rodada completa",
          "[domain][combat][queue]") {
    CombatActor a = actor("a", 30), b = actor("b", 20);
    InitiativeQueue q({&a, &b});
    REQUIRE(q.round_index() == 0);
    q.advance();  // -> b, ainda rodada 0
    REQUIRE(q.round_index() == 0);
    q.advance();  // wrap -> a, rodada 1
    REQUIRE(q.round_index() == 1);
}

// ---- Remove (ajusta o ponteiro) ---------------------------------------------------

TEST_CASE("initiative_queue: remove tira ator da fila e ajusta ponteiro",
          "[domain][combat][queue]") {
    CombatActor a = actor("a", 30), b = actor("b", 20), c = actor("c", 10);
    InitiativeQueue q({&a, &b, &c});
    q.advance();   // current = b
    q.remove(&b);  // remove o atual; current deve apontar pro proximo (c)
    REQUIRE(q.count() == 2);
    REQUIRE(q.current()->id() == "c");
}

TEST_CASE("initiative_queue: contains e remove de ausente e seguro",
          "[domain][combat][queue]") {
    CombatActor a = actor("a", 30);
    CombatActor ausente = actor("z", 1);
    InitiativeQueue q({&a});
    REQUIRE(q.contains(&a));
    REQUIRE_FALSE(q.contains(&ausente));
    q.remove(&ausente);  // no-op
    REQUIRE(q.count() == 1);
}

// ---- RecomputeBySpeed (Haste/Slow, secao 4) ---------------------------------------

TEST_CASE("initiative_queue: recompute_by_speed reordena mantendo o ator corrente",
          "[domain][combat][queue]") {
    CombatActor a = actor("a", 30), b = actor("b", 20), c = actor("c", 10);
    InitiativeQueue q({&a, &b, &c});  // ordem: a, b, c
    q.advance();                      // current = b
    // c fica mais rapido que todos (Haste): nova ordem por SPD = c, a, b
    c.apply_stat_delta(StatusId::Haste, 0, +50);  // spd 60
    q.recompute_by_speed();
    REQUIRE(order_ids(q) == std::vector<std::string>{"c", "a", "b"});
    // o cursor continua apontando pro ator que estava em turno (b).
    REQUIRE(q.current()->id() == "b");
}

// ---- SyncCursorTo (Knockback no proprio tick, secao 4) ----------------------------

TEST_CASE("initiative_queue: sync_cursor_to reaponta pro ator dado",
          "[domain][combat][queue]") {
    CombatActor a = actor("a", 30), b = actor("b", 20), c = actor("c", 10);
    InitiativeQueue q({&a, &b, &c});  // current = a
    // a leva knockback (+1) no proprio turno -> ordem b, a, c, mas a NAO perde o turno.
    q.reorder_actor(&a, +1);
    q.sync_cursor_to(&a);
    REQUIRE(q.current()->id() == "a");
}

// ---- cursor() + bring_to_current (Janela de Comando da Party 1B, secao 4.1) --------
// bring_to_current traz um ator para o SLOT DO CURSOR (vira current()) SEM alterar o
// indice do cursor nem round_index. E o primitivo que realiza a escolha do jogador dentro
// do bloco da party (comando livre): permutacao, nao salto de cursor, entao cada ator
// segue agindo uma vez por rodada.

TEST_CASE("initiative_queue: cursor expoe o indice do turno corrente",
          "[domain][combat][queue]") {
    CombatActor a = actor("a", 30), b = actor("b", 20);
    InitiativeQueue q({&a, &b});
    REQUIRE(q.cursor() == 0);
    q.advance();
    REQUIRE(q.cursor() == 1);
    q.advance();  // wrap
    REQUIRE(q.cursor() == 0);
}

TEST_CASE("initiative_queue: bring_to_current puxa um ator de tras pro slot do cursor",
          "[domain][combat][queue]") {
    CombatActor a = actor("a", 30), b = actor("b", 20), c = actor("c", 10);
    InitiativeQueue q({&a, &b, &c});  // [a, b, c], cursor 0
    q.bring_to_current(&c);           // puxa c (idx2) pro cursor 0
    REQUIRE(order_ids(q) == std::vector<std::string>{"c", "a", "b"});
    REQUIRE(q.current()->id() == "c");
    REQUIRE(q.cursor() == 0);
    REQUIRE(q.round_index() == 0);
}

TEST_CASE("initiative_queue: bring_to_current respeita o cursor avancado",
          "[domain][combat][queue]") {
    CombatActor a = actor("a", 30), b = actor("b", 20), c = actor("c", 10);
    InitiativeQueue q({&a, &b, &c});
    q.advance();             // cursor 1 (current = b)
    q.bring_to_current(&c);  // c (idx2) -> cursor 1; a (atras do cursor) fica intocado
    REQUIRE(order_ids(q) == std::vector<std::string>{"a", "c", "b"});
    REQUIRE(q.current()->id() == "c");
    REQUIRE(q.cursor() == 1);
}

TEST_CASE("initiative_queue: bring_to_current e no-op se ja e o current ou esta atras",
          "[domain][combat][queue]") {
    CombatActor a = actor("a", 30), b = actor("b", 20), c = actor("c", 10);
    InitiativeQueue q({&a, &b, &c});
    q.advance();             // cursor 1 (current = b)
    q.bring_to_current(&b);  // ja e o current -> no-op
    REQUIRE(order_ids(q) == std::vector<std::string>{"a", "b", "c"});
    q.bring_to_current(&a);  // a (idx0) esta ATRAS do cursor -> no-op (nao pula o current)
    REQUIRE(order_ids(q) == std::vector<std::string>{"a", "b", "c"});
    REQUIRE(q.current()->id() == "b");
}

TEST_CASE("initiative_queue: bring_to_current de ausente e no-op seguro (nao lanca)",
          "[domain][combat][queue]") {
    CombatActor a = actor("a", 30), b = actor("b", 20);
    CombatActor estranho = actor("z", 99);
    InitiativeQueue q({&a, &b});
    q.bring_to_current(&estranho);  // ausente -> no-op
    REQUIRE(order_ids(q) == std::vector<std::string>{"a", "b"});
    REQUIRE(q.cursor() == 0);
}

// ---- regroup_stable (regroup por lado na fronteira da rodada, secao 4.1) -----------
// regroup_stable(pred) move os atores que satisfazem `pred` para a FRENTE, preservando a
// ordem relativa corrente de AMBOS os grupos (std::stable_partition). E o primitivo do
// "um lado age todo, depois o outro" (§4.1). A prova de Gambito-safe: um empurrao intra-
// rodada (reorder_actor) fora da ordem de SPD SOBREVIVE ao regroup (stable_partition NAO
// re-sorteia; um SORT restauraria a ordem de SPD e apagaria o empurrao).

TEST_CASE("initiative_queue: regroup_stable agrupa por predicado preservando ordem relativa",
          "[domain][combat][queue][regroup]") {
    // 2 player-side + 2 enemy-side. Empurramos pA1 (rapido) PARA TRAS de pA2 (lento) DENTRO
    // do grupo player => a ordem relativa do grupo fica FORA da ordem de SPD. regroup_stable
    // agrupa player-side na frente MAS mantem pA2 antes de pA1 (prova de stable_partition;
    // um SORT por SPD daria [pA1, pA2] e apagaria o empurrao).
    CombatActor pA1 = actor("pA1", 30, /*player=*/true);
    CombatActor pA2 = actor("pA2", 20, /*player=*/true);
    CombatActor eB1 = actor("eB1", 25, /*player=*/false);
    CombatActor eB2 = actor("eB2", 10, /*player=*/false);
    InitiativeQueue q({&pA1, &pA2, &eB1, &eB2});  // sort SPD: [pA1, eB1, pA2, eB2]
    REQUIRE(order_ids(q) == std::vector<std::string>{"pA1", "eB1", "pA2", "eB2"});

    q.reorder_actor(&pA1, +2);  // empurra pA1 pra tras -> [eB1, pA2, pA1, eB2]
    REQUIRE(order_ids(q) == std::vector<std::string>{"eB1", "pA2", "pA1", "eB2"});

    q.regroup_stable([](const CombatActor* a) { return a->is_player_side(); });
    // player-side na frente NA ORDEM RELATIVA CORRENTE (pA2 antes de pA1), depois enemy-side.
    REQUIRE(order_ids(q) == std::vector<std::string>{"pA2", "pA1", "eB1", "eB2"});
}

TEST_CASE("initiative_queue: regroup_stable zera o cursor e nao toca round_index",
          "[domain][combat][queue][regroup]") {
    CombatActor a = actor("a", 30, /*player=*/false);  // enemy
    CombatActor b = actor("b", 20, /*player=*/true);   // player
    CombatActor c = actor("c", 10, /*player=*/false);  // enemy
    InitiativeQueue q({&a, &b, &c});   // [a(e), b(p), c(e)]
    q.advance(); q.advance(); q.advance();  // da a volta -> round_index 1, cursor 0
    REQUIRE(q.round_index() == 1);

    // enemy-first (predicado = NAO player-side): [a, c] enemigos, [b] party.
    q.regroup_stable([](const CombatActor* x) { return !x->is_player_side(); });
    REQUIRE(order_ids(q) == std::vector<std::string>{"a", "c", "b"});
    REQUIRE(q.cursor() == 0);        // inicio de rodada: primeiro do lado que abre
    REQUIRE(q.round_index() == 1);   // regroup nao conta rodada
    REQUIRE(q.current()->id() == "a");
}

TEST_CASE("initiative_queue: regroup_stable e no-op de ordem quando ja esta agrupado",
          "[domain][combat][queue][regroup]") {
    CombatActor a = actor("a", 30, /*player=*/true);
    CombatActor b = actor("b", 20, /*player=*/true);
    CombatActor c = actor("c", 10, /*player=*/false);
    InitiativeQueue q({&a, &b, &c});  // [a, b, c], ja player-first
    q.regroup_stable([](const CombatActor* x) { return x->is_player_side(); });
    REQUIRE(order_ids(q) == std::vector<std::string>{"a", "b", "c"});
    REQUIRE(q.cursor() == 0);
}

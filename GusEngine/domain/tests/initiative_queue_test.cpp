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

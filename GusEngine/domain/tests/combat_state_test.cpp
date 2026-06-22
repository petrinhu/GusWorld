// combat_state_test.cpp
//
// Spec executavel (Catch2 v3) do CombatState (view read-only do combate em ActionSelect,
// secao 3/13), portado de engine/foundation/turn_combat/CombatState.cs (origem
// canonica). POCO puro, ZERO Qt, headless. Marco M5 (chunk 2). Paridade 1:1 com o C#.
//
// CombatState.cs nao tem arquivo de teste xUnit dedicado; e exercitado indiretamente
// pela FSM. Aqui validamos o contrato puro da view: ActiveActor / RoundIndex / Order /
// AlivePlayers / AliveEnemies / FindById, e o CardRegistry vazio por default.
//
// PORTE DE REFERENCIA: o C# guarda referencia a InitiativeQueue e CombatActor. No C++ a
// view guarda ponteiros NAO-DONOS (const InitiativeQueue* / CombatActor*). CardRegistry
// (IReadOnlyDictionary<string,Card>) vira const std::unordered_map<string,Card>*
// (nullptr => registry vazio).
//
// Cross-ref: engine/foundation/turn_combat/CombatState.cs;
//            docs/design/mecanicas/combat.md secao 3/13.

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <unordered_map>
#include <vector>

#include "gus/domain/combat/combat_actor.hpp"
#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/combat/combat_records.hpp"
#include "gus/domain/combat/combat_state.hpp"
#include "gus/domain/combat/initiative_queue.hpp"

using namespace gus::domain::combat;

namespace {

CombatActor actor(const std::string& id, int spd, bool player) {
    return CombatActor(id, id, 30, 8, 2, spd, CardFamily::Eletrico, player);
}

}  // namespace

TEST_CASE("combat_state: expoe ator ativo, round e ordem da fila",
          "[domain][combat][state]") {
    CombatActor gus = actor("gus", 20, true);
    CombatActor foe = actor("e", 10, false);
    InitiativeQueue q({&gus, &foe});

    const CombatState s(q, q.current(), q.round_index());

    REQUIRE(s.active_actor()->id() == "gus");
    REQUIRE(s.round_index() == 0);
    REQUIRE(s.order().size() == 2);
    REQUIRE(s.order()[0]->id() == "gus");
}

TEST_CASE("combat_state: card_registry vazio por default",
          "[domain][combat][state]") {
    CombatActor gus = actor("gus", 20, true);
    InitiativeQueue q({&gus});
    const CombatState s(q, q.current(), q.round_index());
    REQUIRE(s.card_registry().empty());
}

TEST_CASE("combat_state: card_registry resolve cartas quando fornecido",
          "[domain][combat][state]") {
    CombatActor gus = actor("gus", 20, true);
    InitiativeQueue q({&gus});

    std::unordered_map<std::string, Card> registry;
    Card pulso{};
    pulso.id = "pulso";
    pulso.family = CardFamily::Eletrico;
    registry.emplace("pulso", pulso);

    const CombatState s(q, q.current(), q.round_index(), &registry);
    REQUIRE(s.card_registry().size() == 1);
    REQUIRE(s.card_registry().at("pulso").id == "pulso");
}

TEST_CASE("combat_state: alive_players e alive_enemies filtram por lado e vida",
          "[domain][combat][state]") {
    CombatActor gus = actor("gus", 30, true);
    CombatActor iara = actor("iara", 20, true);
    CombatActor foe1 = actor("e1", 15, false);
    CombatActor foe2 = actor("e2", 10, false);
    InitiativeQueue q({&gus, &iara, &foe1, &foe2});

    foe2.take_damage(1000);  // morto

    const CombatState s(q, q.current(), q.round_index());

    const auto players = s.alive_players();
    REQUIRE(players.size() == 2);

    const auto enemies = s.alive_enemies();
    REQUIRE(enemies.size() == 1);  // foe2 morto fica de fora
    REQUIRE(enemies[0]->id() == "e1");
}

TEST_CASE("combat_state: find_by_id localiza ator ou retorna nullptr",
          "[domain][combat][state]") {
    CombatActor gus = actor("gus", 20, true);
    CombatActor foe = actor("e", 10, false);
    InitiativeQueue q({&gus, &foe});

    const CombatState s(q, q.current(), q.round_index());
    REQUIRE(s.find_by_id("gus") != nullptr);
    REQUIRE(s.find_by_id("gus")->id() == "gus");
    REQUIRE(s.find_by_id("inexistente") == nullptr);
}

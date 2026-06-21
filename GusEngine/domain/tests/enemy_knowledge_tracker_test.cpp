// enemy_knowledge_tracker_test.cpp
//
// Spec executavel (Catch2 v3) da logica PURA de progressao de Knowledge por
// TIPO de inimigo, portada de engine/tests/knowledge/EnemyKnowledgeTrackerTests.cs
// (xUnit). O xUnit e a SPEC do comportamento canonico (F2-E.9); preservado 1:1.
//
// Subsistema portado (engine-design.md secao 2, marco M3): domain/progression
// (o CMakeLists do dominio reserva 'progression/ = EnemyKnowledgeTracker +
// XpDifferential'). POCO puro, ZERO Qt, headless.
//
// Semantica canonica (knowledge-progression.md secao 1/3/7): knowledge_kills e
// "quantos inimigos DAQUELE TIPO o player derrotou", o conhecimento do PLAYER
// sobre o bestiario, NAO um stat de membro da party. O store e
// map<enemy_type_id,int> (chave = id do EnemyTemplate; valor = kills acumulados).
//
// Regras de incremento (secao 3):
//   - Derrotar o inimigo em combate (Victory + HP 0) -> +1 para aquele tipo;
//   - Fugir (Flee)                                   -> 0 (ciclo nao observado);
//   - Party perdeu (Defeat)                          -> 0;
//   - Scan sem derrota                               -> 0 (kill = ciclo completo).
//
// FRONTEIRA (decisao de porte, ver header): o C# resolve DefeatedEnemyTypes a
// partir de CombatActor (entidade mutavel de combate, ainda NAO portada em C++,
// domain/combat e .gitkeep). A logica so le 3 campos do ator (id, is_player_side,
// is_alive); portamos uma VIEW minima DefeatedActor em vez de acoplar progression
// a um tipo combat inexistente. Quando combat/CombatActor for portado, um
// adaptador trivial preenche a view.
//
// Cross-ref: engine/foundation/knowledge/EnemyKnowledgeTracker.cs (origem);
//            docs/design/mecanicas/knowledge-progression.md secao 1/3/7, combat.md secao 11.

#include <catch2/catch_test_macros.hpp>

#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include "gus/domain/progression/enemy_knowledge_tracker.hpp"

using gus::domain::progression::apply_victory;
using gus::domain::progression::CombatOutcome;
using gus::domain::progression::DefeatedActor;
using gus::domain::progression::defeated_enemy_types;
using gus::domain::progression::knowledge_for;
using gus::domain::progression::KnowledgeStore;

namespace {

// Atores de combate minimos para defeated_enemy_types (espelha os helpers
// NonPlayer/Player do xUnit, que usam CombatActor; aqui basta a view POCO).
DefeatedActor non_player(const std::string& id, bool alive) {
    return DefeatedActor{id, /*is_player_side=*/false, /*is_alive=*/alive};
}

DefeatedActor player(const std::string& id, bool alive) {
    return DefeatedActor{id, /*is_player_side=*/true, /*is_alive=*/alive};
}

}  // namespace

// ---- knowledge_for: leitura defensiva (chave ausente = 0 = 1o encontro) ----

TEST_CASE("knowledge: tipo ausente le zero", "[domain][progression][knowledge]") {
    const KnowledgeStore store;
    REQUIRE(knowledge_for(store, "sentinela_bit") == 0);
}

TEST_CASE("knowledge: tipo presente devolve valor armazenado",
          "[domain][progression][knowledge]") {
    const KnowledgeStore store{{"sentinela_bit", 8}};
    REQUIRE(knowledge_for(store, "sentinela_bit") == 8);
}

TEST_CASE("knowledge: type_id vazio lanca", "[domain][progression][knowledge]") {
    const KnowledgeStore store;
    REQUIRE_THROWS_AS(knowledge_for(store, "  "), std::invalid_argument);
    REQUIRE_THROWS_AS(knowledge_for(store, ""), std::invalid_argument);
}

// ---- apply_victory: incremento PURO (devolve novo mapa, nao muta o input) ---

TEST_CASE("knowledge: primeira kill leva o tipo a um",
          "[domain][progression][knowledge]") {
    const KnowledgeStore before;
    const auto after = apply_victory(before, {"sentinela_bit"});
    REQUIRE(after.at("sentinela_bit") == 1);
}

TEST_CASE("knowledge: tipo existente incrementa em um",
          "[domain][progression][knowledge]") {
    const KnowledgeStore before{{"sentinela_bit", 7}};
    const auto after = apply_victory(before, {"sentinela_bit"});
    REQUIRE(after.at("sentinela_bit") == 8);
}

TEST_CASE("knowledge: apply_victory nao muta o input (funcao pura)",
          "[domain][progression][knowledge]") {
    const KnowledgeStore before{{"sentinela_bit", 7}};
    const auto discarded = apply_victory(before, {"sentinela_bit"});
    (void)discarded;  // so interessa o efeito (ou ausencia dele) sobre o input.
    REQUIRE(before.at("sentinela_bit") == 7);  // input intacto
}

TEST_CASE("knowledge: multiplos tipos distintos cada um incrementado",
          "[domain][progression][knowledge]") {
    const KnowledgeStore before{{"sentinela_bit", 2}};
    const auto after = apply_victory(before, {"sentinela_bit", "daemon_guard"});
    REQUIRE(after.at("sentinela_bit") == 3);
    REQUIRE(after.at("daemon_guard") == 1);
}

TEST_CASE("knowledge: mesmo tipo duas vezes no lote incrementa pela contagem",
          "[domain][progression][knowledge]") {
    // Dois Sentinela-Bit derrotados no MESMO combate = +2 (cada corpo e um ciclo).
    const KnowledgeStore before;
    const auto after = apply_victory(before, {"sentinela_bit", "sentinela_bit"});
    REQUIRE(after.at("sentinela_bit") == 2);
}

TEST_CASE("knowledge: lista vazia de derrotados devolve store equivalente",
          "[domain][progression][knowledge]") {
    const KnowledgeStore before{{"sentinela_bit", 5}};
    const auto after = apply_victory(before, {});
    REQUIRE(after.at("sentinela_bit") == 5);
}

TEST_CASE("knowledge: id vazio no lote lanca", "[domain][progression][knowledge]") {
    const KnowledgeStore before;
    REQUIRE_THROWS_AS(apply_victory(before, {"sentinela_bit", " "}),
                      std::invalid_argument);
}

// ---- defeated_enemy_types: so Victory conta; Flee/Defeat nao ---------------

TEST_CASE("knowledge: Victory devolve os tipos de inimigo derrotados",
          "[domain][progression][knowledge]") {
    // Outcome Victory + atores derrotados (inimigos, is_player_side=false).
    // Companions caidos NAO contam (nao sao bestiario).
    const std::vector<DefeatedActor> enemies{
        non_player("sentinela_bit", /*alive=*/false),
        non_player("daemon_guard", /*alive=*/false),
    };

    const auto types = defeated_enemy_types(CombatOutcome::Victory, enemies);

    const std::vector<std::string> expected{"sentinela_bit", "daemon_guard"};
    REQUIRE(types == expected);
}

TEST_CASE("knowledge: Flee devolve vazio", "[domain][progression][knowledge]") {
    // Flee = 0 kills: nao ha aprendizado sem o ciclo completo.
    const std::vector<DefeatedActor> enemies{non_player("sentinela_bit", false)};
    const auto types = defeated_enemy_types(CombatOutcome::Fled, enemies);
    REQUIRE(types.empty());
}

TEST_CASE("knowledge: Defeat devolve vazio", "[domain][progression][knowledge]") {
    // Party perdeu: nenhum aprendizado de bestiario concedido.
    const std::vector<DefeatedActor> enemies{non_player("sentinela_bit", false)};
    const auto types = defeated_enemy_types(CombatOutcome::Defeat, enemies);
    REQUIRE(types.empty());
}

TEST_CASE("knowledge: Victory so conta inimigos mortos",
          "[domain][progression][knowledge]") {
    // Inimigo ainda vivo (fim por outro criterio) nao conta como derrotado.
    const std::vector<DefeatedActor> enemies{
        non_player("sentinela_bit", /*alive=*/false),
        non_player("daemon_guard", /*alive=*/true),
    };

    const auto types = defeated_enemy_types(CombatOutcome::Victory, enemies);

    const std::vector<std::string> expected{"sentinela_bit"};
    REQUIRE(types == expected);
}

TEST_CASE("knowledge: Victory exclui o lado do player",
          "[domain][progression][knowledge]") {
    // Um companion caido (is_player_side=true, HP 0) nao e bestiario; nao entra.
    const std::vector<DefeatedActor> actors{
        player("gus", /*alive=*/false),
        non_player("sentinela_bit", /*alive=*/false),
    };

    const auto types = defeated_enemy_types(CombatOutcome::Victory, actors);

    const std::vector<std::string> expected{"sentinela_bit"};
    REQUIRE(types == expected);
}

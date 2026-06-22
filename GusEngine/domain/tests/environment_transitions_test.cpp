// environment_transitions_test.cpp
//
// Spec executavel (Catch2 v3) da tabela FECHADA e DETERMINISTICA de mutabilidade de
// ambiente (secao 18.6), portada de
// engine/foundation/turn_combat/EnvironmentTransitions.cs (origem canonica) e dos casos
// de engine/tests/turn_combat/environments/EnvironmentTransitionsTests.cs (xUnit = SPEC).
// Sem RNG; transicoes nunca pulam estados; resolucao nunca ambigua. POCO puro, ZERO Qt,
// headless. Marco M5 (chunk 3).
//
// Cross-ref: engine/foundation/turn_combat/EnvironmentTransitions.cs;
//            engine/tests/turn_combat/environments/EnvironmentTransitionsTests.cs;
//            docs/design/mecanicas/combat.md secao 18.6/18.4 (T4 lapida em T1).

#include <catch2/catch_test_macros.hpp>

#include <set>
#include <utility>

#include "gus/domain/combat/environment_enums.hpp"
#include "gus/domain/combat/environment_transitions.hpp"

using namespace gus::domain::combat;

// ---- Cada linha canonica resolve para o resultado esperado (secao 18.6) ------------

TEST_CASE("environment_transitions: transicao canonica resolve para o resultado esperado",
          "[domain][combat][environment]") {
    struct Case {
        EnvironmentTrigger trigger;
        EnvironmentId target;
        EnvironmentId expected;
    };
    const Case cases[] = {
        {EnvironmentTrigger::EletricoOuCalor, EnvironmentId::Lamacento, EnvironmentId::Seco},
        {EnvironmentTrigger::EletricoOuCalor, EnvironmentId::Vinhas, EnvironmentId::Seco},
        {EnvironmentTrigger::EletricoOuCalor, EnvironmentId::Gelo, EnvironmentId::AguaAlagado},
        {EnvironmentTrigger::SonicoForte, EnvironmentId::Neblina, EnvironmentId::None},
        {EnvironmentTrigger::SonicoForte, EnvironmentId::Fumaca, EnvironmentId::None},
        {EnvironmentTrigger::SonicoForte, EnvironmentId::Estatica, EnvironmentId::None},
        {EnvironmentTrigger::Vento, EnvironmentId::Neblina, EnvironmentId::None},
        {EnvironmentTrigger::Vento, EnvironmentId::Fumaca, EnvironmentId::None},
        {EnvironmentTrigger::Chuva, EnvironmentId::Calor, EnvironmentId::Neblina},  // Vapor
        {EnvironmentTrigger::Chuva, EnvironmentId::TempestadeEletrica, EnvironmentId::TempestadeEletrica},
        {EnvironmentTrigger::Agua, EnvironmentId::MetalCondutor, EnvironmentId::MetalCondutor},
        {EnvironmentTrigger::AcaceiroSaudavel, EnvironmentId::AnomaliaPerlin, EnvironmentId::AnomaliaPerlin},
        {EnvironmentTrigger::AshlarVencidoEntreEncontros, EnvironmentId::AshlarBruto, EnvironmentId::PavimentoTesselado},
    };
    for (const auto& c : cases) {
        const auto result = EnvironmentTransitions::resolve(c.trigger, c.target);
        REQUIRE(result.has_value());
        REQUIRE(result.value() == c.expected);
    }
}

TEST_CASE("environment_transitions: gatilho sem linha correspondente nao transiciona",
          "[domain][combat][environment]") {
    // Sonico forte nao muda Lamacento (so dissipa neblina/fumaca/estatica).
    const auto result = EnvironmentTransitions::resolve(EnvironmentTrigger::SonicoForte,
                                                        EnvironmentId::Lamacento);
    REQUIRE_FALSE(result.has_value());
}

TEST_CASE("environment_transitions: Ashlar Bruto lapida em Pavimento Tesselado (progressao)",
          "[domain][combat][environment]") {
    // secao 18.4 + 18.6: bruto -> polido (oficio maconico canon).
    const auto result = EnvironmentTransitions::resolve(
        EnvironmentTrigger::AshlarVencidoEntreEncontros, EnvironmentId::AshlarBruto);
    REQUIRE(result.has_value());
    REQUIRE(result.value() == EnvironmentId::PavimentoTesselado);
}

TEST_CASE("environment_transitions: tabela e deterministica no maximo uma linha por par",
          "[domain][combat][environment]") {
    // Garante que (gatilho, alvo) nunca casa 2 linhas (sem ambiguidade, secao 18.6).
    std::set<std::pair<EnvironmentTrigger, EnvironmentId>> seen;
    for (const auto& t : EnvironmentTransitions::all()) {
        const auto key = std::make_pair(t.trigger, t.target);
        REQUIRE(seen.find(key) == seen.end());
        seen.insert(key);
    }
}

TEST_CASE("environment_transitions: a tabela tem as 16 linhas canonicas",
          "[domain][combat][environment]") {
    // 16 transicoes na ordem da tabela do doc (EnvironmentTransitions.cs).
    REQUIRE(EnvironmentTransitions::all().size() == 16u);
}

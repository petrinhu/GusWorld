// environment_enums_test.cpp
//
// Spec executavel (Catch2 v3) dos enums de ambiente de combate, portados de
// engine/foundation/turn_combat/EnvironmentEnums.cs (origem canonica). POCO puro,
// ZERO Qt, headless. Marco M5 (chunk 1: fundacoes do combate).
//
// Tres camadas SIMULTANEAS (terreno x clima x periodo), secao 18. O EnvironmentId
// e o contrato de identidade estavel (nao localizado): travar os ordinais protege o
// catalogo/serializer futuro contra reordenacao acidental.
//
// Cross-ref: engine/foundation/turn_combat/EnvironmentEnums.cs;
//            docs/design/mecanicas/combat.md secao 18/11/9; ADR-006.

#include <catch2/catch_test_macros.hpp>

#include <cstdint>

#include "gus/domain/combat/environment_enums.hpp"

using namespace gus::domain::combat;

namespace {

template <typename E>
constexpr std::uint32_t ord(E e) {
    return static_cast<std::uint32_t>(e);
}

}  // namespace

// ---- EnvironmentLayer (secao 18.1) ------------------------------------------------

TEST_CASE("environment_enums: EnvironmentLayer ordinais",
          "[domain][combat][environment]") {
    REQUIRE(ord(EnvironmentLayer::Terreno) == 0u);
    REQUIRE(ord(EnvironmentLayer::Clima) == 1u);
    REQUIRE(ord(EnvironmentLayer::Periodo) == 2u);
}

// ---- EnvironmentTier (secao 18.4/18.5) --------------------------------------------

TEST_CASE("environment_enums: EnvironmentTier ordinais",
          "[domain][combat][environment]") {
    REQUIRE(ord(EnvironmentTier::Visivel) == 0u);
    REQUIRE(ord(EnvironmentTier::Codex) == 1u);
}

// ---- EnvironmentId: catalogo completo do vertical slice (secao 18) ----------------
// None=0; depois CLIMA(8), PERIODO(4), TERRENO Visivel(12), TERRENO Codex(3).
// Total: 1 + 8 + 4 + 12 + 3 = 28 valores, ordinais 0..27.

TEST_CASE("environment_enums: EnvironmentId.None e o ordinal 0",
          "[domain][combat][environment]") {
    REQUIRE(ord(EnvironmentId::None) == 0u);
}

TEST_CASE("environment_enums: EnvironmentId clima (transitorio) ordinais 1..8",
          "[domain][combat][environment]") {
    REQUIRE(ord(EnvironmentId::Neblina) == 1u);
    REQUIRE(ord(EnvironmentId::Chuva) == 2u);
    REQUIRE(ord(EnvironmentId::Calor) == 3u);
    REQUIRE(ord(EnvironmentId::TempestadeEletrica) == 4u);
    REQUIRE(ord(EnvironmentId::Vento) == 5u);
    REQUIRE(ord(EnvironmentId::Estatica) == 6u);
    REQUIRE(ord(EnvironmentId::Fumaca) == 7u);
    REQUIRE(ord(EnvironmentId::EscuridaoTotal) == 8u);
}

TEST_CASE("environment_enums: EnvironmentId periodo (roda de 4) ordinais 9..12",
          "[domain][combat][environment]") {
    REQUIRE(ord(EnvironmentId::Dia) == 9u);
    REQUIRE(ord(EnvironmentId::Crepusculo) == 10u);
    REQUIRE(ord(EnvironmentId::Noite) == 11u);
    REQUIRE(ord(EnvironmentId::Aurora) == 12u);
}

TEST_CASE("environment_enums: EnvironmentId terreno visivel ordinais 13..24",
          "[domain][combat][environment]") {
    REQUIRE(ord(EnvironmentId::Lamacento) == 13u);
    REQUIRE(ord(EnvironmentId::Seco) == 14u);
    REQUIRE(ord(EnvironmentId::Vinhas) == 15u);
    REQUIRE(ord(EnvironmentId::Gelo) == 16u);
    REQUIRE(ord(EnvironmentId::AguaAlagado) == 17u);
    REQUIRE(ord(EnvironmentId::MetalCondutor) == 18u);
    REQUIRE(ord(EnvironmentId::Bioluminescencia) == 19u);
    REQUIRE(ord(EnvironmentId::PavimentoTesselado) == 20u);
    REQUIRE(ord(EnvironmentId::TaludeInstavel) == 21u);
    REQUIRE(ord(EnvironmentId::AshlarBruto) == 22u);
    REQUIRE(ord(EnvironmentId::SoloFertilRecursivo) == 23u);
    REQUIRE(ord(EnvironmentId::AnomaliaPerlin) == 24u);
}

TEST_CASE("environment_enums: EnvironmentId terreno codex ordinais 25..27",
          "[domain][combat][environment]") {
    REQUIRE(ord(EnvironmentId::EspelhoRessonante) == 25u);
    REQUIRE(ord(EnvironmentId::DutoCondutorPressurizado) == 26u);
    REQUIRE(ord(EnvironmentId::ElevacaoDominante) == 27u);
}

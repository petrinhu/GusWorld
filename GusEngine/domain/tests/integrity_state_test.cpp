// integrity_state_test.cpp
//
// Spec executavel (Catch2 v3) da PECA de integridade/virus de uma carta possuida
// (ATOM-1, decomposicao atomica de CardPhysicalState em pecas componiveis,
// gus/domain/infection/integrity_state.hpp): IntegrityState, VirusKind e
// IntegrityState::validate() (invariante secao 6 inv.1: virus_kind/is_infected/
// is_diagnosed). Cobertura FOCADA na peca isolada; a spec exaustiva do AGREGADO
// CardPhysicalState (que tambem cobre is_burned_out, fora desta peca) continua em
// card_hardware_test.cpp (zero diff, criterio de aceite do ATOM-1).
//
// Cross-ref: docs/design/mecanicas/cartas-spec-dados.md secao 5/6;
//            gus/domain/deck/card_hardware.hpp (fachada agregada, herda desta peca);
//            card_hardware_test.cpp (oraculo do agregado).

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <stdexcept>

#include "gus/domain/infection/integrity_state.hpp"

using gus::domain::infection::IntegrityState;
using gus::domain::infection::kVirusKindCount;
using gus::domain::infection::kWormPropagationChancePercent;
using gus::domain::infection::VirusKind;

TEST_CASE("integrity_state: default e sem infeccao (estado seguro)",
          "[domain][infection][integrity_state]") {
    IntegrityState s;
    REQUIRE_FALSE(s.is_infected);
    REQUIRE_FALSE(s.is_diagnosed);
    REQUIRE(s.virus_kind == VirusKind::None);
    REQUIRE_NOTHROW(s.validate());
}

TEST_CASE("integrity_state: operator== compara por valor",
          "[domain][infection][integrity_state]") {
    IntegrityState a;
    IntegrityState b;
    REQUIRE(a == b);
    b.is_infected = true;
    REQUIRE_FALSE(a == b);
}

TEST_CASE("integrity_state: kVirusKindCount bate o dominio canonico (8 valores, "
          "inclusive IndustrialWeapon)",
          "[domain][infection][integrity_state]") {
    REQUIRE(kVirusKindCount == 8);
    REQUIRE(static_cast<std::uint32_t>(VirusKind::IndustrialWeapon) == 7);
}

TEST_CASE("integrity_state: validate() rejeita virus_kind != None sem is_infected",
          "[domain][infection][integrity_state][validate]") {
    IntegrityState s;
    s.virus_kind = VirusKind::LogicBomb;
    s.is_infected = false;
    REQUIRE_THROWS_AS(s.validate(), std::invalid_argument);
}

TEST_CASE("integrity_state: validate() rejeita is_diagnosed sem is_infected",
          "[domain][infection][integrity_state][validate]") {
    IntegrityState s;
    s.is_diagnosed = true;
    s.is_infected = false;
    REQUIRE_THROWS_AS(s.validate(), std::invalid_argument);
}

TEST_CASE("integrity_state: validate() aceita is_infected + virus_kind + "
          "is_diagnosed coerentes",
          "[domain][infection][integrity_state][validate]") {
    IntegrityState s;
    s.is_infected = true;
    s.virus_kind = VirusKind::Worm;
    s.is_diagnosed = true;
    REQUIRE_NOTHROW(s.validate());
}

TEST_CASE("integrity_state: validate() rejeita virus_kind com ordinal fora do "
          "dominio (defesa em profundidade)",
          "[domain][infection][integrity_state][validate]") {
    IntegrityState s;
    s.is_infected = true;
    s.virus_kind = static_cast<VirusKind>(255);
    REQUIRE_THROWS_AS(s.validate(), std::invalid_argument);
}

TEST_CASE("integrity_state: kWormPropagationChancePercent bate o valor fechado (13%)",
          "[domain][infection][integrity_state]") {
    REQUIRE(kWormPropagationChancePercent == 13);
}

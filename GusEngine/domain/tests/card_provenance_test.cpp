// card_provenance_test.cpp
//
// Spec executavel (Catch2 v3) da PECA de proveniencia fisica de uma carta possuida
// (ATOM-1, decomposicao atomica de CardPhysicalState em pecas componiveis,
// gus/domain/hardware/card_provenance.hpp): CardOrigin, RsbConnector, connector_of()
// e CardProvenance::validate(). Cobertura FOCADA na peca isolada; a spec exaustiva do
// AGREGADO CardPhysicalState (composicao das 3 pecas + is_burned_out) continua em
// card_hardware_test.cpp (zero diff, criterio de aceite do ATOM-1).
//
// Cross-ref: docs/design/mecanicas/cartas-spec-dados.md secao 5/6;
//            gus/domain/deck/card_hardware.hpp (fachada agregada, herda desta peca);
//            card_hardware_test.cpp (oraculo do agregado).

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <stdexcept>

#include "gus/domain/hardware/card_provenance.hpp"

using gus::domain::hardware::CardOrigin;
using gus::domain::hardware::CardProvenance;
using gus::domain::hardware::connector_of;
using gus::domain::hardware::kCardOriginCount;
using gus::domain::hardware::RsbConnector;

TEST_CASE("card_provenance: default e OriginalRom (estado seguro)",
          "[domain][hardware][card_provenance]") {
    CardProvenance p;
    REQUIRE(p.origin == CardOrigin::OriginalRom);
    REQUIRE_NOTHROW(p.validate());
}

TEST_CASE("card_provenance: operator== compara por valor",
          "[domain][hardware][card_provenance]") {
    CardProvenance a;
    CardProvenance b;
    REQUIRE(a == b);
    b.origin = CardOrigin::PirateClone;
    REQUIRE_FALSE(a == b);
}

TEST_CASE("card_provenance: kCardOriginCount bate o dominio canonico (3 valores)",
          "[domain][hardware][card_provenance]") {
    REQUIRE(kCardOriginCount == 3);
}

TEST_CASE("card_provenance: validate() rejeita ordinal fora do dominio (defesa em "
          "profundidade)",
          "[domain][hardware][card_provenance][validate]") {
    CardProvenance p;
    p.origin = static_cast<CardOrigin>(99);
    REQUIRE_THROWS_AS(p.validate(), std::invalid_argument);
}

TEST_CASE("card_provenance: connector_of deriva 1:1 de CardOrigin",
          "[domain][hardware][card_provenance][connector]") {
    REQUIRE(connector_of(CardOrigin::OriginalRom) == RsbConnector::None);
    REQUIRE(connector_of(CardOrigin::HomebrewEprom) == RsbConnector::ExternalVisible);
    REQUIRE(connector_of(CardOrigin::PirateClone) == RsbConnector::InternalHidden);
}

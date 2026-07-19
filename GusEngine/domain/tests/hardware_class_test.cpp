// hardware_class_test.cpp
//
// Spec executavel (Catch2 v3) da classificacao derivada de hardware de uma carta
// possuida (ATOM-1, decomposicao atomica ao nivel de modulo,
// gus/domain/hardware/hardware_class.hpp): HardwareClass, hardware_class_of() e
// contamination_percent_for(). HardwareClass NAO e um campo armazenado - e a CHAVE
// derivada de (CardTier do catalogo, CardOrigin, mimics_special), por isso mora ao
// lado do vocabulario de carta (cards::CardTier) e nao dentro de nenhuma das 3 pecas
// de estado (CardProvenance/BatteryState/IntegrityState). Cobertura FOCADA nesta
// classificacao; battery_capacity_for(HardwareClass, DifficultyLevel) continua em
// gus/domain/deck/card_hardware_constants.hpp (cruza com save::DifficultyLevel,
// umbrella deck/) e sua spec continua em card_hardware_test.cpp (zero diff).
//
// Cross-ref: docs/design/mecanicas/cartas-spec-dados.md secao 5.3 (hardware_class_of);
//            docs/design/mecanicas/cartas-numeros-proposta.md secao 3 (contaminacao);
//            gus/domain/deck/card_hardware.hpp (fachada, re-exporta pra deck::);
//            card_hardware_test.cpp (oraculo do agregado + tabela de bateria).

#include <catch2/catch_test_macros.hpp>

#include "gus/domain/cards/card_enums.hpp"
#include "gus/domain/hardware/card_provenance.hpp"
#include "gus/domain/hardware/hardware_class.hpp"

using gus::domain::cards::CardTier;
using gus::domain::hardware::CardOrigin;
using gus::domain::hardware::contamination_percent_for;
using gus::domain::hardware::hardware_class_of;
using gus::domain::hardware::HardwareClass;

TEST_CASE("hardware_class: hardware_class_of resolve as 5 classes",
          "[domain][hardware][hardware_class]") {
    REQUIRE(hardware_class_of(CardTier::Comum, CardOrigin::OriginalRom, false) ==
            HardwareClass::ComumOriginal);
    REQUIRE(hardware_class_of(CardTier::Comum, CardOrigin::HomebrewEprom, false) ==
            HardwareClass::HomebrewEprom);
    REQUIRE(hardware_class_of(CardTier::Comum, CardOrigin::PirateClone, false) ==
            HardwareClass::PirataComum);
    REQUIRE(hardware_class_of(CardTier::Comum, CardOrigin::PirateClone, true) ==
            HardwareClass::PirataEspecialFalso);
    REQUIRE(hardware_class_of(CardTier::Especial, CardOrigin::OriginalRom, false) ==
            HardwareClass::EspecialSelada);
}

TEST_CASE("hardware_class: hardware_class_of(Especial/Super, ...) SEMPRE "
          "EspecialSelada independente de origin/mimics_special (classe PROTEGIDA)",
          "[domain][hardware][hardware_class]") {
    REQUIRE(hardware_class_of(CardTier::Especial, CardOrigin::HomebrewEprom, false) ==
            HardwareClass::EspecialSelada);
    REQUIRE(hardware_class_of(CardTier::Especial, CardOrigin::OriginalRom, true) ==
            HardwareClass::EspecialSelada);
    REQUIRE(hardware_class_of(CardTier::Super, CardOrigin::OriginalRom, false) ==
            HardwareClass::EspecialSelada);
}

TEST_CASE("hardware_class: contamination_percent_for bate a tabela fechada "
          "(0/1/8/21/55%)",
          "[domain][hardware][hardware_class][contamination]") {
    REQUIRE(contamination_percent_for(HardwareClass::EspecialSelada) == 0);
    REQUIRE(contamination_percent_for(HardwareClass::ComumOriginal) == 1);
    REQUIRE(contamination_percent_for(HardwareClass::PirataEspecialFalso) == 8);
    REQUIRE(contamination_percent_for(HardwareClass::PirataComum) == 21);
    REQUIRE(contamination_percent_for(HardwareClass::HomebrewEprom) == 55);
}

// gus/domain/src/deck/card_hardware.cpp
//
// Implementacao de CardPhysicalState::validate() - o AGREGADO. ATOM-1: as funcoes
// puras derivadas (state_of_health_percent, is_battery_dead,
// battery_charge_remaining, connector_of, hardware_class_of) foram extraidas pras
// PECAS (gus/domain/hardware/{card_provenance,battery_state,hardware_class}.cpp,
// gus/domain/infection/integrity_state.cpp) - card_hardware.hpp so RE-EXPORTA (using-
// declaration). validate() do agregado DELEGA as 2 pecas com invariante proprio
// (CardProvenance::validate() + IntegrityState::validate()), preservando tipo
// (std::invalid_argument) e mensagem de cada excecao 1:1 - BatteryState nao tem
// invariante proprio (nenhum lancamento la). Ver card_hardware.hpp.

#include "gus/domain/deck/card_hardware.hpp"

namespace gus::domain::deck {

void CardPhysicalState::validate() const {
    hardware::CardProvenance::validate();
    infection::IntegrityState::validate();
}

}  // namespace gus::domain::deck

// gus/domain/src/hardware/card_provenance.cpp
//
// Implementacao de CardProvenance::validate() + connector_of(). Ver header para o
// contrato. ATOM-1: extraido de gus/domain/src/deck/card_hardware.cpp.

#include "gus/domain/hardware/card_provenance.hpp"

#include <stdexcept>

namespace gus::domain::hardware {

void CardProvenance::validate() const {
    // Defesa em profundidade (mesmo padrao de DifficultyLevel::validate() em
    // save_data.cpp): um payload selado mas schema-divergente (ordinal fora do
    // dominio canonico do enum) nao e aceito silenciosamente.
    if (static_cast<std::uint32_t>(origin) >= kCardOriginCount)
        throw std::invalid_argument(
            "CardPhysicalState: origin fora do dominio (ordinal invalido).");
}

RsbConnector connector_of(CardOrigin origin) noexcept {
    switch (origin) {
        case CardOrigin::OriginalRom:
            return RsbConnector::None;
        case CardOrigin::HomebrewEprom:
            return RsbConnector::ExternalVisible;
        case CardOrigin::PirateClone:
            return RsbConnector::InternalHidden;
    }
    return RsbConnector::None;  // inalcancavel (switch exaustivo).
}

}  // namespace gus::domain::hardware

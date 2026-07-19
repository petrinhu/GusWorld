// gus/domain/src/hardware/hardware_class.cpp
//
// Implementacao de hardware_class_of(). Ver header para o contrato. ATOM-1:
// extraido de gus/domain/src/deck/card_hardware.cpp.

#include "gus/domain/hardware/hardware_class.hpp"

namespace gus::domain::hardware {

using gus::domain::cards::CardTier;

HardwareClass hardware_class_of(CardTier catalog_tier, CardOrigin origin,
                                 bool mimics_special) noexcept {
    // CardTier::Especial/Super = classe PROTEGIDA (mesmo agrupamento de
    // CardCollection::guard_protected_tier, deck-mao-sistema.md secao 7 inv.9) -
    // SEMPRE EspecialSelada, independente de origin/mimics_special.
    if (catalog_tier == CardTier::Especial || catalog_tier == CardTier::Super)
        return HardwareClass::EspecialSelada;
    // Clone-falso (secao 7 do doc-fonte): mimics_special VENCE origin - a carta e
    // CardTier::Comum no catalogo, mas o hardware por baixo e "qualidade
    // intermediaria" (8% de risco, nao 21%).
    if (mimics_special) return HardwareClass::PirataEspecialFalso;
    switch (origin) {
        case CardOrigin::OriginalRom:
            return HardwareClass::ComumOriginal;
        case CardOrigin::HomebrewEprom:
            return HardwareClass::HomebrewEprom;
        case CardOrigin::PirateClone:
            return HardwareClass::PirataComum;
    }
    return HardwareClass::ComumOriginal;  // inalcancavel (switch exaustivo).
}

}  // namespace gus::domain::hardware

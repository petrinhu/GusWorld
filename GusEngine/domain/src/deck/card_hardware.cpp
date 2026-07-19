// gus/domain/src/deck/card_hardware.cpp
//
// Implementacao de CardPhysicalState::validate() + funcoes puras derivadas
// (state_of_health_percent, is_battery_dead, battery_charge_remaining,
// connector_of, hardware_class_of). Ver card_hardware.hpp pros invariantes
// garantidos (cartas-spec-dados.md secao 5/6).

#include "gus/domain/deck/card_hardware.hpp"

#include <algorithm>
#include <stdexcept>

#include "gus/domain/deck/card_hardware_constants.hpp"

namespace gus::domain::deck {

using gus::domain::combat::CardTier;

void CardPhysicalState::validate() const {
    // Defesa em profundidade (mesmo padrao de DifficultyLevel::validate() em
    // save_data.cpp): um payload selado mas schema-divergente (ordinal fora do
    // dominio canonico do enum) nao e aceito silenciosamente.
    if (static_cast<std::uint32_t>(origin) >= kCardOriginCount)
        throw std::invalid_argument(
            "CardPhysicalState: origin fora do dominio (ordinal invalido).");
    if (static_cast<std::uint32_t>(virus_kind) >= kVirusKindCount)
        throw std::invalid_argument(
            "CardPhysicalState: virus_kind fora do dominio (ordinal invalido).");

    // Secao 6 inv.1: nao existe "tipo de virus" sem infeccao.
    if (virus_kind != VirusKind::None && !is_infected)
        throw std::invalid_argument(
            "CardPhysicalState: virus_kind != None exige is_infected == true "
            "(cartas-spec-dados.md secao 6 inv.1).");
    // Secao 6 inv.1: nao se diagnostica infeccao que nao existe.
    if (is_diagnosed && !is_infected)
        throw std::invalid_argument(
            "CardPhysicalState: is_diagnosed == true exige is_infected == true "
            "(cartas-spec-dados.md secao 6 inv.1).");
}

std::uint8_t state_of_health_percent(const CardPhysicalState& state) noexcept {
    // Calculo em int (nao uint8/uint16) para nao estourar/underflowar antes do
    // clamp - cycles pode ser ate 65535 (uint16 max), 13*65535 cabe folgado num
    // int de 32 bits.
    const int raw = 100 - static_cast<int>(kBatteryDegradationPerRechargeCyclePp) *
                              static_cast<int>(state.battery_recharge_cycles);
    const int clamped = std::clamp(raw, 0, 100);
    return static_cast<std::uint8_t>(clamped);
}

bool is_battery_dead(const CardPhysicalState& state) noexcept {
    return state_of_health_percent(state) <= kBatteryDeadSohFloorPercent;
}

std::uint32_t battery_charge_remaining(const CardPhysicalState& state,
                                        std::uint32_t capacity) noexcept {
    // Clamp >= 0 SEM underflow de unsigned: subtrai so quando o deficit cabe
    // dentro da capacidade.
    if (state.battery_charge_deficit >= capacity) return 0;
    return capacity - state.battery_charge_deficit;
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

}  // namespace gus::domain::deck

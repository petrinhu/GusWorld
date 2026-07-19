// gus/domain/src/hardware/battery_state.cpp
//
// Implementacao das funcoes puras derivadas de BatteryState (state_of_health_percent,
// is_battery_dead, battery_charge_remaining). Ver header para o contrato. ATOM-1:
// extraido de gus/domain/src/deck/card_hardware.cpp.

#include "gus/domain/hardware/battery_state.hpp"

#include <algorithm>

namespace gus::domain::hardware {

std::uint8_t state_of_health_percent(const BatteryState& state) noexcept {
    // Calculo em int (nao uint8/uint16) para nao estourar/underflowar antes do
    // clamp - cycles pode ser ate 65535 (uint16 max), 13*65535 cabe folgado num
    // int de 32 bits.
    const int raw = 100 - static_cast<int>(kBatteryDegradationPerRechargeCyclePp) *
                              static_cast<int>(state.battery_recharge_cycles);
    const int clamped = std::clamp(raw, 0, 100);
    return static_cast<std::uint8_t>(clamped);
}

bool is_battery_dead(const BatteryState& state) noexcept {
    return state_of_health_percent(state) <= kBatteryDeadSohFloorPercent;
}

std::uint32_t battery_charge_remaining(const BatteryState& state,
                                        std::uint32_t capacity) noexcept {
    // Clamp >= 0 SEM underflow de unsigned: subtrai so quando o deficit cabe
    // dentro da capacidade.
    if (state.battery_charge_deficit >= capacity) return 0;
    return capacity - state.battery_charge_deficit;
}

}  // namespace gus::domain::hardware

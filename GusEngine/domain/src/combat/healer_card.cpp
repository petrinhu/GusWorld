// SPDX-License-Identifier: Apache-2.0
// gus/domain/combat/healer_card.cpp
//
// Implementacao de resolve_healer_card_use. Ver o header para o contrato completo
// (ordem de consumo de RNG, racional de is_battery_dead, escopo fora desta fatia).

#include "gus/domain/combat/healer_card.hpp"

#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

namespace gus::domain::combat {

namespace {

// Empurra 1 CombatLogEntry pro log, se `log` != nullptr (regra "todo efeito loga no
// terminal"). Mesmo padrao de techmagic.cpp::log_entry.
void log_entry(std::vector<CombatLogEntry>* log, const CombatActor& caster,
               const CombatActor& target, int value, std::string message) {
    if (log == nullptr) return;
    log->push_back(CombatLogEntry{caster.id(), CombatActionType::UseCard,
                                   std::optional<std::string>(target.id()), value,
                                   std::move(message)});
}

}  // namespace

HealerCardUseResult resolve_healer_card_use(hardware::HardwareClass hardware_class,
                                             gus::domain::save::DifficultyLevel difficulty,
                                             hardware::BatteryState& battery,
                                             std::uint32_t battery_capacity,
                                             CombatActor& caster, CombatActor& target,
                                             IRandomSource& rng,
                                             std::vector<CombatLogEntry>* log) {
    const std::uint32_t remaining_before =
        hardware::battery_charge_remaining(battery, battery_capacity);
    if (remaining_before == 0) {
        throw std::logic_error(
            "resolve_healer_card_use: bateria sem carga remanescente - bug de call "
            "site, o chamador deve gatear carta esgotada ANTES de invocar (mesmo "
            "padrao de CombatActor::spend_ap/spend_mana).");
    }

    HealerCardUseResult result;

    // 1) Chance de falha total por degradacao (SoH). SoH==100 nunca rola (0 consumo de
    // RNG, deterministico "nunca falha" por construcao - mesmo padrao de
    // RepeatLastAction/Mandelbrot magnitude==0).
    const std::uint8_t soh = hardware::state_of_health_percent(battery);
    const int fail_percent = 100 - static_cast<int>(soh);
    if (fail_percent > 0) {
        result.roll_fail_percent = fail_percent;
        const int roll = rng.next(100);
        if (roll < fail_percent) {
            result.failed = true;
            log_entry(log, caster, target, 0,
                      "Sylvesse-Religação falhou: bateria degradada (SoH " +
                          std::to_string(soh) + "%, " + std::to_string(fail_percent) +
                          "% de chance de falha, roll " + std::to_string(roll) + ").");
            return result;
        }
    }

    // 2) Cura: fixa quando heal_min==heal_max, senao 1 rng.next() uniforme.
    const HealerCardTuning& tuning = healer_card_tuning_for(hardware_class, difficulty);
    const int heal_amount = (tuning.heal_max > tuning.heal_min)
                                 ? tuning.heal_min +
                                       rng.next(tuning.heal_max - tuning.heal_min + 1)
                                 : tuning.heal_min;

    // 3) Dreno: mesmo padrao fixo-ou-1-rng, aplicado sobre a carga ATUAL remanescente
    // (ANTES desta mutacao - "nao da capacidade maxima", spec do lider). ROUND-HALF-UP
    // (arredonda pro inteiro mais proximo, +50 antes de dividir por 100) - NAO floor:
    // floor deixaria um residuo de 1 unidade de carga PRESO PRA SEMPRE (floor(1*50/100)
    // == 0, a carga nunca desce do ultimo 1 por mais usos que rolem). Round-half-up
    // garante que a carga SEMPRE atinge exatamente 0 apos usos finitos suficientes
    // (ver healer_card_test.cpp, teste de "nunca fica negativa" - trava a sequencia
    // completa ate 0). Nunca super-drena de qualquer forma (arredonda ATE a carga
    // remanescente, nunca alem dela, pois percent <= 100).
    const int drain_percent =
        (tuning.drain_percent_max > tuning.drain_percent_min)
            ? tuning.drain_percent_min +
                  rng.next(tuning.drain_percent_max - tuning.drain_percent_min + 1)
            : tuning.drain_percent_min;
    const std::uint64_t drain_amount =
        (static_cast<std::uint64_t>(remaining_before) *
             static_cast<std::uint64_t>(drain_percent) +
         50) /
        100;

    battery.battery_charge_deficit += static_cast<std::uint32_t>(drain_amount);
    target.heal(heal_amount);

    result.healed_amount = heal_amount;
    result.drain_applied = static_cast<int>(drain_amount);
    log_entry(log, caster, target, heal_amount,
              "Sylvesse-Religação curou " + std::to_string(heal_amount) + " HP (drenou " +
                  std::to_string(drain_amount) + "/" + std::to_string(remaining_before) +
                  " de carga remanescente).");
    return result;
}

}  // namespace gus::domain::combat

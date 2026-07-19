// gus/domain/deck/card_hardware_constants.hpp
//
// UMBRELLA das constantes numericas da camada FISICA de carta que CRUZAM pecas/
// dominios (CARDS-HARDWARE-ENGINE incremento 1, CARDS-HW-1), cada uma marcada
// //PLAYTEST (mesmo padrao de deck_constants.hpp). Afinaveis no playtest N=3;
// nenhum consumidor deve hardcodar estes valores soltos no proprio codigo - sempre
// importar daqui (fonte unica). Header-only (sem .cpp): tabelas constexpr + funcoes
// de lookup inline, mesmo padrao de gus/domain/input/action_registry.hpp::
// all_action_categories().
//
// ATOM-1 (decomposicao atomica): as constantes que pertencem a UMA SO peca
// migraram pro modulo da propria peca (kBatteryDegradationPerRechargeCyclePp/
// kBatteryDeadSohFloorPercent -> hardware/battery_state.hpp;
// kWormPropagationChancePercent -> infection/integrity_state.hpp;
// kContaminationPercentTable/contamination_percent_for -> hardware/hardware_class.hpp,
// indexada SO por HardwareClass). Este umbrella guarda o que sobra: numeros que
// CRUZAM HardwareClass com save::DifficultyLevel (battery_capacity_for - hardware/
// nao pode incluir save/) e numeros de config sem peca dona obvia (EPROM/Turing).
// Todos continuam acessiveis em gus::domain::deck:: (re-exportados via
// card_hardware.hpp, incluido abaixo).
//
// POCO puro, ZERO SDL/glintfx.
//
// Cross-ref: docs/design/mecanicas/cartas-numeros-proposta.md secao 0/1a/1b/2/3/6
//            (fonte dos valores - este header NAO recopia o racional, so nomeia).

#ifndef GUS_DOMAIN_DECK_CARD_HARDWARE_CONSTANTS_HPP
#define GUS_DOMAIN_DECK_CARD_HARDWARE_CONSTANTS_HPP

#include <array>
#include <cstddef>
#include <cstdint>

#include "gus/domain/deck/card_hardware.hpp"  // HardwareClass
#include "gus/domain/save/save_data.hpp"      // DifficultyLevel

namespace gus::domain::deck {

// ---- Bateria CR2032 - capacidade (cartas-numeros-proposta.md secao 1a) ------------
//
// Escada Fibonacci, 5 HardwareClass x 4 DifficultyLevel. AMB-DADOS-01 RESOLVIDO
// 2026-07-19 (economy-designer + lider): PirataEspecialFalso um degrau ACIMA de
// PirataComum (34 no Medio, o degrau que faltava entre 21 e 55), NAO igual a ele.
// Linhas na ORDEM ordinal de HardwareClass (ComumOriginal=0..EspecialSelada=4);
// colunas na ORDEM ordinal de DifficultyLevel (Facil=0..Hardcore=3). //PLAYTEST
inline constexpr std::array<std::array<std::uint32_t, 4>, 5> kBatteryCapacityTable = {{
    {{110, 55, 27, 27}},   // ComumOriginal
    {{16, 8, 4, 4}},        // HomebrewEprom
    {{42, 21, 10, 10}},     // PirataComum
    {{68, 34, 17, 17}},     // PirataEspecialFalso
    {{288, 144, 72, 72}},   // EspecialSelada
}};

// Capacidade de bateria (unidades de carga) para (HardwareClass, DifficultyLevel).
// Table lookup puro - HardwareClass tem exatamente 5 valores canonicos e
// DifficultyLevel exatamente 4 (kDifficultyLevelCount, save_data.hpp); os
// chamadores desta funcao SEMPRE passam valores ja validados (nunca u32 cru de
// payload nao-confiavel - o hardening de ordinal fica em validate(), nao aqui).
[[nodiscard]] inline std::uint32_t battery_capacity_for(
    HardwareClass hardware_class, gus::domain::save::DifficultyLevel difficulty) noexcept {
    return kBatteryCapacityTable[static_cast<std::size_t>(hardware_class)]
                                [static_cast<std::size_t>(difficulty)];
}

// ---- Memoria / EPROM (cartas-numeros-proposta.md secao 2) -------------------------

// Ciclos de regravacao homebrew antes do EPROM queimar de vez (vira sucata).
// //PLAYTEST
inline constexpr std::uint8_t kEpromRewriteCyclesBeforeBurnout = 8;

// ---- Cura do Turing (cartas-numeros-proposta.md secao 6) --------------------------
//
// Binario sucesso/queima (soma 100%, sem 3o resultado neutro - aposta real, sem
// "tentar de novo de graca"). //PLAYTEST
inline constexpr std::uint8_t kTuringCureSuccessPercent = 62;
inline constexpr std::uint8_t kTuringCureBurnoutPercent = 38;

}  // namespace gus::domain::deck

#endif  // GUS_DOMAIN_DECK_CARD_HARDWARE_CONSTANTS_HPP

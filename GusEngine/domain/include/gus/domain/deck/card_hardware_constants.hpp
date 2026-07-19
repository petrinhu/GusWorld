// gus/domain/deck/card_hardware_constants.hpp
//
// TODAS as constantes numericas da camada FISICA de carta (CARDS-HARDWARE-ENGINE
// incremento 1, CARDS-HW-1), num header so, cada uma marcada //PLAYTEST (mesmo
// padrao de deck_constants.hpp). Afinaveis no playtest N=3; nenhum consumidor deve
// hardcodar estes valores soltos no proprio codigo - sempre importar daqui (fonte
// unica). Header-only (sem .cpp): tabelas constexpr + funcoes de lookup inline,
// mesmo padrao de gus/domain/input/action_registry.hpp::all_action_categories().
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

// Degradacao por ciclo de recarga em-lugar (pontos percentuais de SoH). NAO escala
// por dificuldade (decisao do lider - so a CAPACIDADE escala, cartas-numeros-
// proposta.md secao 1b). //PLAYTEST
inline constexpr std::uint16_t kBatteryDegradationPerRechargeCyclePp = 13;

// Piso de SoH considerado bateria morta (nao serve mais pra recarregar em-lugar).
// //PLAYTEST
inline constexpr std::uint8_t kBatteryDeadSohFloorPercent = 21;

// ---- Contaminacao por virus (%) - cartas-numeros-proposta.md secao 3 -------------
//
// FECHADO PELO LIDER 2026-07-18: fixo em TODO modo de dificuldade (nao escala,
// ao contrario da bateria). Ordem ordinal de HardwareClass. //PLAYTEST
inline constexpr std::array<std::uint8_t, 5> kContaminationPercentTable = {
    1,   // ComumOriginal
    55,  // HomebrewEprom
    21,  // PirataComum
    8,   // PirataEspecialFalso
    0,   // EspecialSelada (canon fixo - so gatilho narrativo, nunca RNG)
};

// % de risco de contaminacao na aquisicao (loot/compra/upload homebrew), por
// HardwareClass. Table lookup puro - mesma nota de battery_capacity_for sobre
// dominio ja-validado do chamador.
[[nodiscard]] inline std::uint8_t contamination_percent_for(
    HardwareClass hardware_class) noexcept {
    return kContaminationPercentTable[static_cast<std::size_t>(hardware_class)];
}

// Propagacao secundaria (carta JA infectada, ao ser conjurada) - "worm de deck".
// //PLAYTEST
inline constexpr std::uint8_t kWormPropagationChancePercent = 13;

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

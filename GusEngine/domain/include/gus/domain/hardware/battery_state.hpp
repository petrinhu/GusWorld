// gus/domain/hardware/battery_state.hpp
//
// PECA de energia/bateria de uma carta possuida (ATOM-1, decomposicao atomica de
// CardPhysicalState em pecas componiveis em modulos estreitos, generalizando
// ADR-019 ao nivel de modulo; CARDS-HARDWARE-ENGINE incremento 1, CARDS-HW-1):
// degradacao por recarga-em-lugar (bateria CR2032) + deficit de carga. POCO puro,
// ZERO SDL/GL/RmlUi/IO (invariante de domain/, engine-design.md secao 2).
//
// hardware/ NAO inclui combat/ nem deck/ (gate de camadas).
//
// Cross-ref: docs/design/mecanicas/cartas-hardware-pirataria-energia.md secao 5;
//            docs/design/mecanicas/cartas-numeros-proposta.md secao 1a/1b;
//            gus/domain/deck/card_hardware.hpp (fachada agregada - CardPhysicalState
//            HERDA desta peca); gus/domain/deck/card_hardware_constants.hpp
//            (battery_capacity_for, cruza com save::DifficultyLevel - fora desta
//            peca, umbrella deck/); card_hardware_test.cpp (oraculo do agregado).

#ifndef GUS_DOMAIN_HARDWARE_BATTERY_STATE_HPP
#define GUS_DOMAIN_HARDWARE_BATTERY_STATE_HPP

#include <cstdint>

namespace gus::domain::hardware {

// Degradacao por ciclo de recarga em-lugar (pontos percentuais de SoH). NAO escala
// por dificuldade (decisao do lider - so a CAPACIDADE escala, cartas-numeros-
// proposta.md secao 1b). //PLAYTEST
inline constexpr std::uint16_t kBatteryDegradationPerRechargeCyclePp = 13;

// Piso de SoH considerado bateria morta (nao serve mais pra recarregar em-lugar).
// //PLAYTEST
inline constexpr std::uint8_t kBatteryDeadSohFloorPercent = 21;

// Estado de energia MUTAVEL de uma copia especifica de carta. Default =
// BatteryState{} = "bateria cheia, zero ciclos de recarga" - o estado mais SEGURO/
// generoso, tanto para uma instancia nova quanto para um save V6 migrado (que nunca
// teve este campo) - cartas-spec-dados.md secao 5.2, "zero e seguro".
struct BatteryState {
    // Fonte de verdade da degradacao: cada RECARGA EM-LUGAR (mesma bateria fisica)
    // incrementa 1. SoH = 100 - 13xcycles (clamp 0), derivado - ver
    // state_of_health_percent() abaixo. Trocar de bateria (item novo, onda futura)
    // ZERA este campo; recarga-em-lugar INCREMENTA.
    std::uint16_t battery_recharge_cycles = 0;

    // Deficit de carga em relacao a capacidade ATUAL (nao a carga absoluta). 0 =
    // bateria CHEIA. Carga restante utilizavel = capacidade(hardware_class,
    // dificuldade) - battery_charge_deficit, clamp >= 0 (nunca negativo). Modelado
    // como DEFICIT de proposito: o valor-zero (default de struct, e o valor que um
    // save V6 migrado ganha por nao ter este campo) significa "bateria cheia" - o
    // estado mais SEGURO/generoso pra retrocompatibilidade.
    std::uint32_t battery_charge_deficit = 0;

    [[nodiscard]] bool operator==(const BatteryState&) const = default;
};

// SoH em pontos percentuais, clamp [0,100]. 13pp por recarga
// (kBatteryDegradationPerRechargeCyclePp).
[[nodiscard]] std::uint8_t state_of_health_percent(const BatteryState&) noexcept;

// SoH <= kBatteryDeadSohFloorPercent (21%, piso de descarte) - "nao serve mais pra
// recarregar em-lugar", so vender/reciclar OU trocar de bateria.
[[nodiscard]] bool is_battery_dead(const BatteryState&) noexcept;

// Carga restante = capacidade_atual - battery_charge_deficit, clamp >= 0 (SEM
// underflow de unsigned mesmo com deficit > capacidade). Recebe a capacidade JA
// RESOLVIDA (chamador calcula via battery_capacity_for, card_hardware_constants.hpp)
// - esta funcao NAO conhece dificuldade/classe, so faz a subtracao/clamp.
[[nodiscard]] std::uint32_t battery_charge_remaining(const BatteryState&,
                                                      std::uint32_t capacity) noexcept;

}  // namespace gus::domain::hardware

#endif  // GUS_DOMAIN_HARDWARE_BATTERY_STATE_HPP

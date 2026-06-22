// gus/domain/combat/environment_enums.hpp
//
// Enums POCO do sistema de ambientes de combate (secao 18). Portado de
// engine/foundation/turn_combat/EnvironmentEnums.cs. POCO puro, ZERO Qt (invariante
// de domain/, engine-design.md secao 2). Header-only.
//
// Tres camadas SIMULTANEAS (terreno x clima x periodo) cujos multiplicadores se
// multiplicam na formula secao 11 (mult_ambiente), com cap final [0.44, 2.25].
//
// Ordinais EXPLICITOS = contrato de identidade estavel (catalogo/serializer). O C#
// usa ordinais implicitos na ordem de declaracao; aqui os tornamos explicitos sem
// mudar a ordem (None=0, depois CLIMA, PERIODO, TERRENO Visivel, TERRENO Codex).
//
// Cross-ref: engine/foundation/turn_combat/EnvironmentEnums.cs;
//            docs/design/mecanicas/combat.md secao 18/11/9; ADR-006.

#ifndef GUS_DOMAIN_COMBAT_ENVIRONMENT_ENUMS_HPP
#define GUS_DOMAIN_COMBAT_ENVIRONMENT_ENUMS_HPP

#include <cstdint>

namespace gus::domain::combat {

// Camada de ambiente (secao 18.1). Terreno fixo por encontro; Clima transitorio (muda
// a cada N turnos); Periodo roda automatica de 4 fases.
enum class EnvironmentLayer : std::uint32_t {
    Terreno = 0,
    Clima = 1,
    Periodo = 2,
};

// Tier de leitura de um terreno (secao 18.4/18.5). Visivel = lido a olho nu (teto
// cognitivo ~12); Codex = efeito sutil que so ativa/revela apos Scan-ambiente (1 AP).
// Clima e Periodo sao sempre Visiveis.
enum class EnvironmentTier : std::uint32_t {
    Visivel = 0,
    Codex = 1,
};

// Catalogo COMPLETO de ambientes do vertical slice (secao 18, decisao F2-PROD.4:
// catalogo completo MANTIDO no VS - 12 terrenos + 8 climas + 4 periodos, mais None).
// Identidade estavel (nao-localizada): o nome de exibicao vem do catalogo/tr() na UI.
enum class EnvironmentId : std::uint32_t {
    // Ausencia de ambiente. mult_ambiente = 1.0 (retrocompat secao 11: inalterado).
    None = 0,

    // ---- CLIMA (8, transitorio) - secao 18.2 ----
    Neblina = 1,
    Chuva = 2,
    Calor = 3,
    TempestadeEletrica = 4,
    Vento = 5,
    Estatica = 6,
    Fumaca = 7,
    EscuridaoTotal = 8,

    // ---- PERIODO (roda de 4 fases) - secao 18.3 ----
    Dia = 9,
    Crepusculo = 10,
    Noite = 11,
    Aurora = 12,

    // ---- TERRENO Tier VISIVEL - secao 18.4 ----
    Lamacento = 13,
    Seco = 14,
    Vinhas = 15,
    Gelo = 16,
    AguaAlagado = 17,
    MetalCondutor = 18,
    Bioluminescencia = 19,
    PavimentoTesselado = 20,    // T1
    TaludeInstavel = 21,        // T2
    AshlarBruto = 22,           // T4
    SoloFertilRecursivo = 23,   // T5
    AnomaliaPerlin = 24,        // T6 - vetor anti-padrao exclusivo Patch-Zero (boss)

    // ---- TERRENO Tier CODEX (revela so apos Scan-ambiente 1 AP) - secao 18.5 ----
    EspelhoRessonante = 25,         // T3
    DutoCondutorPressurizado = 26,  // T7
    ElevacaoDominante = 27,         // T8
};

}  // namespace gus::domain::combat

#endif  // GUS_DOMAIN_COMBAT_ENVIRONMENT_ENUMS_HPP

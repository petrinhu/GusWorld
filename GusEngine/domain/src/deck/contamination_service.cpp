// gus/domain/src/deck/contamination_service.cpp
//
// Implementacao da rolagem de contaminacao na aquisicao (CARDS-HW-3B). Ver o header
// para o contrato completo (guards, ordem, tabela de risco/payload, AMB-07).

#include "gus/domain/deck/contamination_service.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <stdexcept>

namespace gus::domain::deck {

namespace {

struct VirusPayloadWeightEntry {
    VirusKind kind;
    int weight;
};

// Tabela de pesos por classe (cartas-numeros-proposta.md secao 3a, AMB-07 RESOLVIDA;
// grade Fibonacci 1/3/5/8/13). Colunas [Backdoor, Worm, LogicBomb, ZipBomb], SEMPRE
// nesta ordem. Linhas na MESMA ordem ordinal de HardwareClass usada em
// kContaminationPercentTable (ComumOriginal/HomebrewEprom/PirataComum/
// PirataEspecialFalso/EspecialSelada - hardware_class.hpp).
//
// EspecialSelada (linha 4) e INALCANCAVEL em jogo normal (guardada upstream por
// roll_contamination_on_acquisition, guard 1) - pesos todos zero de proposito,
// dispara o fallback defensivo de pick_weighted_payload (devolve entry[0].kind,
// Backdoor) se isso acontecer mesmo assim.
constexpr std::array<std::array<VirusPayloadWeightEntry, 4>, 5> kVirusPayloadWeightTable = {{
    // ComumOriginal - 52,0%/32,0%/12,0%/4,0%
    std::array<VirusPayloadWeightEntry, 4>{{{VirusKind::Backdoor, 13},
                                             {VirusKind::Worm, 8},
                                             {VirusKind::LogicBomb, 3},
                                             {VirusKind::ZipBomb, 1}}},
    // HomebrewEprom - espelho EXATO de ComumOriginal - 4,0%/12,0%/32,0%/52,0%
    std::array<VirusPayloadWeightEntry, 4>{{{VirusKind::Backdoor, 1},
                                             {VirusKind::Worm, 3},
                                             {VirusKind::LogicBomb, 8},
                                             {VirusKind::ZipBomb, 13}}},
    // PirataComum - espelho EXATO de PirataEspecialFalso - 12,5%/20,8%/33,3%/33,3%
    std::array<VirusPayloadWeightEntry, 4>{{{VirusKind::Backdoor, 3},
                                             {VirusKind::Worm, 5},
                                             {VirusKind::LogicBomb, 8},
                                             {VirusKind::ZipBomb, 8}}},
    // PirataEspecialFalso - 33,3%/33,3%/20,8%/12,5%
    std::array<VirusPayloadWeightEntry, 4>{{{VirusKind::Backdoor, 8},
                                             {VirusKind::Worm, 8},
                                             {VirusKind::LogicBomb, 5},
                                             {VirusKind::ZipBomb, 3}}},
    // EspecialSelada - INALCANCAVEL (ver comentario acima).
    std::array<VirusPayloadWeightEntry, 4>{{{VirusKind::Backdoor, 0},
                                             {VirusKind::Worm, 0},
                                             {VirusKind::LogicBomb, 0},
                                             {VirusKind::ZipBomb, 0}}},
}};

}  // namespace

VirusKind pick_weighted_payload(HardwareClass hardware_class, combat::IRandomSource& rng) {
    const auto& table = kVirusPayloadWeightTable[static_cast<std::size_t>(hardware_class)];

    int total = 0;
    for (const auto& entry : table) total += entry.weight;
    if (total <= 0)
        return table[0].kind;  // defensivo (linha EspecialSelada, inalcancavel em jogo
                                // normal - guardada upstream).

    const int roll = rng.next(total);
    int cumulative = 0;
    for (const auto& entry : table) {
        cumulative += entry.weight;
        if (roll < cumulative) return entry.kind;
    }
    return table.back().kind;  // fallback defensivo (arredondamento), matematicamente
                                // inalcancavel: roll < total sempre por construcao do rng.
}

ContaminationRollOutcome roll_contamination_on_acquisition(CardPhysicalState& physical,
                                                             cards::CardTier catalog_tier,
                                                             bool mimics_special,
                                                             combat::IRandomSource& rng) {
    // Guard defensivo (mesmo espirito do guard 1 de attempt_cure, turing_service.cpp):
    // classe protegida tem 0% de risco por definicao (contamination_percent_for(
    // EspecialSelada) == 0) - este guard e so defesa em profundidade extra, nao
    // deveria disparar em jogo normal (a excecao Sterling/Faraday e evento
    // narrativo scriptado a parte, fora deste sistema geral - secao 5.1/6).
    if (catalog_tier == cards::CardTier::Especial || catalog_tier == cards::CardTier::Super)
        return ContaminationRollOutcome::SkippedProtectedTier;

    const HardwareClass hardware_class =
        hardware_class_of(catalog_tier, physical.origin, mimics_special);
    const std::uint8_t risk_percent = contamination_percent_for(hardware_class);

    // 1o draw, canonico (secao 11) - roll em [0,100), infecta se < risco%.
    const int roll = rng.next(100);
    if (roll >= static_cast<int>(risk_percent)) return ContaminationRollOutcome::Clean;

    // 2o draw, SO neste ramo (secao 3a/AMB-07): payload ponderado pela mesma classe.
    physical.is_infected = true;
    physical.virus_kind = pick_weighted_payload(hardware_class, rng);
    return ContaminationRollOutcome::Infected;
}

std::string_view translation_key_for(ContaminationRollOutcome outcome) {
    switch (outcome) {
        case ContaminationRollOutcome::Infected:
            // AMBIGUO de proposito - nao confirma infeccao (a certeza fica reservada
            // ao diagnostico do Turing, secao 6). "CHECKSUM" e o mesmo vocabulario
            // diegetico ja usado no doc-fonte pro sinal ambiguo de aquisicao.
            return "CONTAMINATION_ACQUIRE_SUSPICIOUS_CHECKSUM";
        case ContaminationRollOutcome::Clean:
            return "CONTAMINATION_ACQUIRE_CLEAN";
        case ContaminationRollOutcome::SkippedProtectedTier:
            return "CONTAMINATION_ACQUIRE_PROTECTED_TIER";
    }
    throw std::logic_error(
        "contamination_service: ContaminationRollOutcome sem chave i18n mapeada "
        "(CARDS-HW-3B) - um valor novo append-only sem case aqui e bug de "
        "implementacao.");
}

}  // namespace gus::domain::deck

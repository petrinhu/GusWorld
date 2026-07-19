// gus/domain/src/deck/turing_service.cpp
//
// Implementacao do servico de diagnostico/cura do Turing. Ver o header para o
// contrato completo (guards, ordem, split 62/38%, AMB-T1). CARDS-HW-2A.

#include "gus/domain/deck/turing_service.hpp"

#include <stdexcept>

#include "gus/domain/deck/card_hardware_constants.hpp"

namespace gus::domain::deck {

DiagnoseOutcome diagnose(infection::IntegrityState& state) noexcept {
    if (!state.is_infected) return DiagnoseOutcome::RejectedNotInfected;
    state.is_diagnosed = true;
    return DiagnoseOutcome::Diagnosed;
}

CureOutcome attempt_cure(CardPhysicalState& physical, cards::CardTier tier,
                          combat::IRandomSource& rng) {
    // Guard 1 (defensivo): classe protegida nunca deveria chegar aqui infectada
    // (secao 5.1, 0% de risco geral) - checado ANTES do guard de diagnostico
    // (secao 6 pseudocodigo, ordem das duas pre-condicoes).
    if (tier == cards::CardTier::Especial || tier == cards::CardTier::Super)
        return CureOutcome::RejectedProtectedTier;

    // Guard 2: nao cura no escuro.
    if (!physical.is_diagnosed) return CureOutcome::RejectedNotDiagnosed;

    // 1 draw de RNG, canonico (secao 11) - roll em [0,100), sucesso se < 62%.
    const int roll = rng.next(100);
    if (roll < static_cast<int>(kTuringCureSuccessPercent)) {
        physical.is_infected = false;
        physical.virus_kind = VirusKind::None;
        physical.is_diagnosed = false;
        return CureOutcome::Cured;
    }

    // Falha (38%): SUCATA, nao destruicao (AMB-T1 RESOLVIDA pelo lider,
    // 2026-07-19). is_infected/virus_kind ficam intocados de proposito - a carta
    // so nao roda mais (gate de gameplay futuro), mas continua na colecao.
    physical.is_burned_out = true;
    return CureOutcome::Burned;
}

std::string_view translation_key_for(DiagnoseOutcome outcome) {
    switch (outcome) {
        case DiagnoseOutcome::Diagnosed:
            return "TURING_DIAGNOSE_SUCCESS";
        case DiagnoseOutcome::RejectedNotInfected:
            return "TURING_DIAGNOSE_REJECTED_NOT_INFECTED";
    }
    throw std::logic_error(
        "turing_service: DiagnoseOutcome sem chave i18n mapeada (CARDS-HW-2A) - "
        "um valor novo append-only sem case aqui e bug de implementacao.");
}

std::string_view translation_key_for(CureOutcome outcome) {
    switch (outcome) {
        case CureOutcome::Cured:
            return "TURING_CURE_SUCCESS";
        case CureOutcome::Burned:
            return "TURING_CURE_BURNED";
        case CureOutcome::RejectedNotDiagnosed:
            return "TURING_CURE_REJECTED_NOT_DIAGNOSED";
        case CureOutcome::RejectedProtectedTier:
            return "TURING_CURE_REJECTED_PROTECTED_TIER";
    }
    throw std::logic_error(
        "turing_service: CureOutcome sem chave i18n mapeada (CARDS-HW-2A) - um "
        "valor novo append-only sem case aqui e bug de implementacao.");
}

}  // namespace gus::domain::deck

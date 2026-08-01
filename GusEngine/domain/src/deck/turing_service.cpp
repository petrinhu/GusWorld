// SPDX-License-Identifier: Apache-2.0
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

DiagnoseOutcome diagnose_in_deck(CardCollection& collection, std::uint64_t instance_id) {
    // Valor de entrada nunca observado pelo chamador se apply_to_physical lancar
    // (fail-fast propaga antes do return) - so existe pra dar um valor inicial
    // ao outcome capturado por referencia dentro do emprestimo.
    DiagnoseOutcome outcome = DiagnoseOutcome::RejectedNotInfected;
    collection.apply_to_physical(
        instance_id, [&outcome](const std::string&, CardPhysicalState& physical) {
            // diagnose() e puro (opera sobre a PECA IntegrityState, base publica de
            // CardPhysicalState) - o emprestimo garante que esta 'physical' e a
            // copia local revalidada pelo agregado no commit, nao o container real.
            outcome = diagnose(physical);
        });
    return outcome;
}

CureOutcome attempt_cure_in_deck(CardCollection& collection, std::uint64_t instance_id,
                                  const CardCollection::TierLookup& tier_of,
                                  combat::IRandomSource& rng) {
    CureOutcome outcome = CureOutcome::RejectedNotDiagnosed;  // idem diagnose_in_deck
    collection.apply_to_physical(
        instance_id,
        [&outcome, &tier_of, &rng](const std::string& card_id, CardPhysicalState& physical) {
            // card_id vem do PROPRIO emprestimo (copia estavel, card_collection.hpp) -
            // resolve o tier ANTES de chamar o servico puro, mesma convencao de
            // guard_protected_tier/sell()/upload() (TierLookup por card_id).
            const cards::CardTier tier = tier_of(card_id);
            outcome = attempt_cure(physical, tier, rng);
        });
    return outcome;
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

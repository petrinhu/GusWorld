// SPDX-License-Identifier: GPL-3.0-or-later
// turing_service_test.cpp
//
// Spec executavel (Catch2 v3) do servico de diagnostico/cura do Turing (CARDS-HW-2
// fatia A, CARDS-HW-2A, TODO.md; gus/domain/deck/turing_service.hpp;
// docs/design/mecanicas/cartas-spec-logica.md secao 6, AttemptCure).
//
// Cobre: diagnose() (guard is_infected, idempotencia); attempt_cure() nas bordas
// exatas do split 62/38% (roll 61 vs 62 vs 63 - kTuringCureSuccessPercent==62);
// AMB-T1 RESOLVIDA (queima = SUCATA - is_burned_out=true, carta permanece,
// is_infected/virus_kind ficam como estavam); os 2 guards (RejectedNotDiagnosed,
// RejectedProtectedTier - Especial e Super) nao mutam NADA; pos-estado sempre passa
// physical.validate() nos 4 desfechos; determinismo de draws (exatamente 1 draw de
// rng por attempt_cure(), via CountingRandom); retry apos cura/queima.
//
// Cross-ref: gus/domain/deck/turing_service.hpp; gus/domain/infection/
//            integrity_state.hpp; gus/domain/deck/card_hardware.hpp;
//            gus/domain/deck/card_hardware_constants.hpp (kTuringCureSuccessPercent/
//            kTuringCureBurnoutPercent); fixed_random.hpp; counting_random.hpp.

#include <catch2/catch_test_macros.hpp>

#include <stdexcept>

#include <string>

#include "counting_random.hpp"
#include "fixed_random.hpp"
#include "gus/domain/cards/card_enums.hpp"
#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/deck/card_collection.hpp"
#include "gus/domain/deck/card_hardware.hpp"
#include "gus/domain/deck/card_hardware_constants.hpp"
#include "gus/domain/deck/deck_constants.hpp"
#include "gus/domain/deck/turing_service.hpp"
#include "gus/domain/infection/integrity_state.hpp"

using gus::domain::cards::CardTier;
using gus::domain::deck::attempt_cure;
using gus::domain::deck::attempt_cure_in_deck;
using gus::domain::deck::CardCollection;
using gus::domain::deck::CardPhysicalState;
using gus::domain::deck::CureOutcome;
using gus::domain::deck::diagnose;
using gus::domain::deck::diagnose_in_deck;
using gus::domain::deck::DiagnoseOutcome;
using gus::domain::deck::kDeckCapacityTier1;
using gus::domain::deck::kTuringCureBurnoutPercent;
using gus::domain::deck::kTuringCureSuccessPercent;
using gus::domain::deck::translation_key_for;
using gus::domain::deck::VirusKind;
using gus::domain::infection::IntegrityState;
using gus::domain::tests::CountingRandom;
using gus::domain::tests::FixedRandom;

namespace {

// Mesma convencao de card_collection_test.cpp/deck_transactions_test.cpp: sufixo
// "_especial"/"_super" marca a classe protegida (secao 7 inv.9), sem depender do
// registry real de combate.
CardTier fake_tier_of(const std::string& card_id) {
    if (card_id.find("_especial") != std::string::npos) return CardTier::Especial;
    if (card_id.find("_super") != std::string::npos) return CardTier::Super;
    return CardTier::Comum;
}

}  // namespace

namespace {

// Instancia infectada + diagnosticada (o estado de entrada canonico de
// attempt_cure() nos casos que passam os 2 guards).
[[nodiscard]] CardPhysicalState infected_diagnosed() {
    CardPhysicalState p;
    p.is_infected = true;
    p.virus_kind = VirusKind::Worm;
    p.is_diagnosed = true;
    return p;
}

}  // namespace

// ---- Split 62/38% (cartas-numeros-proposta.md secao 6) ----------------------------

TEST_CASE("turing_service: constantes 62/38 somam 100 (binario sem 3o resultado)",
          "[domain][infection][turing_service]") {
    REQUIRE(kTuringCureSuccessPercent == 62);
    REQUIRE(kTuringCureBurnoutPercent == 38);
    REQUIRE(kTuringCureSuccessPercent + kTuringCureBurnoutPercent == 100);
}

// ---- diagnose() ---------------------------------------------------------------

TEST_CASE("turing_service: diagnose() rejeita carta sem infeccao (nada muta)",
          "[domain][infection][turing_service][diagnose]") {
    IntegrityState s;
    REQUIRE(diagnose(s) == DiagnoseOutcome::RejectedNotInfected);
    REQUIRE_FALSE(s.is_diagnosed);
    REQUIRE_NOTHROW(s.validate());
}

TEST_CASE("turing_service: diagnose() diagnostica carta infectada oculta",
          "[domain][infection][turing_service][diagnose]") {
    IntegrityState s;
    s.is_infected = true;
    s.virus_kind = VirusKind::LogicBomb;
    REQUIRE(diagnose(s) == DiagnoseOutcome::Diagnosed);
    REQUIRE(s.is_diagnosed);
    REQUIRE_NOTHROW(s.validate());
}

TEST_CASE("turing_service: diagnose() e idempotente (carta ja diagnosticada)",
          "[domain][infection][turing_service][diagnose]") {
    IntegrityState s;
    s.is_infected = true;
    s.virus_kind = VirusKind::Backdoor;
    s.is_diagnosed = true;
    REQUIRE(diagnose(s) == DiagnoseOutcome::Diagnosed);
    REQUIRE(s.is_diagnosed);
    REQUIRE(s.virus_kind == VirusKind::Backdoor);
    REQUIRE_NOTHROW(s.validate());
}

// ---- attempt_cure() - bordas exatas do split (roll 61/62/63) ----------------------

TEST_CASE("turing_service: attempt_cure() roll=61 (<62) cura - limpa completamente",
          "[domain][infection][turing_service][attempt_cure]") {
    CardPhysicalState p = infected_diagnosed();
    FixedRandom rng(0.5, 61);

    REQUIRE(attempt_cure(p, CardTier::Comum, rng) == CureOutcome::Cured);
    REQUIRE_FALSE(p.is_infected);
    REQUIRE(p.virus_kind == VirusKind::None);
    REQUIRE_FALSE(p.is_diagnosed);
    REQUIRE_FALSE(p.is_burned_out);
    REQUIRE_NOTHROW(p.validate());
}

TEST_CASE("turing_service: attempt_cure() roll=62 (borda, >=62) queima - sucata",
          "[domain][infection][turing_service][attempt_cure]") {
    CardPhysicalState p = infected_diagnosed();
    FixedRandom rng(0.5, 62);

    REQUIRE(attempt_cure(p, CardTier::Comum, rng) == CureOutcome::Burned);
    REQUIRE(p.is_burned_out);
    // AMB-T1 RESOLVIDA: is_infected/virus_kind ficam como estavam - NAO e destruicao.
    REQUIRE(p.is_infected);
    REQUIRE(p.virus_kind == VirusKind::Worm);
    REQUIRE(p.is_diagnosed);
    REQUIRE_NOTHROW(p.validate());
}

TEST_CASE("turing_service: attempt_cure() roll=63 (>62) queima - sucata",
          "[domain][infection][turing_service][attempt_cure]") {
    CardPhysicalState p = infected_diagnosed();
    FixedRandom rng(0.5, 63);

    REQUIRE(attempt_cure(p, CardTier::Comum, rng) == CureOutcome::Burned);
    REQUIRE(p.is_burned_out);
    REQUIRE(p.is_infected);
    REQUIRE_NOTHROW(p.validate());
}

// ---- AMB-T1: queima = SUCATA, carta permanece na colecao, inutilizavel ------------

TEST_CASE("turing_service: queima NAO remove/destroi - carta so vira sucata "
          "(AMB-T1 RESOLVIDA)",
          "[domain][infection][turing_service][attempt_cure]") {
    CardPhysicalState p = infected_diagnosed();
    FixedRandom rng(0.5, 99);  // topo da faixa de queima

    REQUIRE(attempt_cure(p, CardTier::Comum, rng) == CureOutcome::Burned);
    // O objeto CardPhysicalState continua existindo/acessivel - "sucata" e so um
    // flag, nao uma remocao de container (nada aqui apaga a instancia).
    REQUIRE(p.is_burned_out);
    REQUIRE_NOTHROW(p.validate());
}

// ---- Guards nao mutam nada ---------------------------------------------------------

TEST_CASE("turing_service: attempt_cure() rejeita carta nao-diagnosticada "
          "(nao cura no escuro, nada muta)",
          "[domain][infection][turing_service][attempt_cure][guard]") {
    CardPhysicalState p;
    p.is_infected = true;
    p.virus_kind = VirusKind::AdwareSterling;
    p.is_diagnosed = false;
    CardPhysicalState before = p;
    FixedRandom rng(0.5, 0);  // roll que curaria se o guard nao barrasse

    REQUIRE(attempt_cure(p, CardTier::Comum, rng) == CureOutcome::RejectedNotDiagnosed);
    REQUIRE(p == before);
    REQUIRE_NOTHROW(p.validate());
}

TEST_CASE("turing_service: attempt_cure() rejeita tier Especial (defensivo, nada "
          "muta)",
          "[domain][infection][turing_service][attempt_cure][guard]") {
    CardPhysicalState p = infected_diagnosed();
    CardPhysicalState before = p;
    FixedRandom rng(0.5, 0);

    REQUIRE(attempt_cure(p, CardTier::Especial, rng) == CureOutcome::RejectedProtectedTier);
    REQUIRE(p == before);
    REQUIRE_NOTHROW(p.validate());
}

TEST_CASE("turing_service: attempt_cure() rejeita tier Super (defensivo, nada muta)",
          "[domain][infection][turing_service][attempt_cure][guard]") {
    CardPhysicalState p = infected_diagnosed();
    CardPhysicalState before = p;
    FixedRandom rng(0.5, 0);

    REQUIRE(attempt_cure(p, CardTier::Super, rng) == CureOutcome::RejectedProtectedTier);
    REQUIRE(p == before);
    REQUIRE_NOTHROW(p.validate());
}

TEST_CASE("turing_service: guard de tier protegido e checado ANTES do guard de "
          "diagnostico (nao-diagnosticada + Especial ainda rejeita por tier)",
          "[domain][infection][turing_service][attempt_cure][guard]") {
    CardPhysicalState p;  // is_infected=false, is_diagnosed=false (default seguro)
    FixedRandom rng(0.5, 0);

    REQUIRE(attempt_cure(p, CardTier::Especial, rng) == CureOutcome::RejectedProtectedTier);
    REQUIRE_NOTHROW(p.validate());
}

// ---- Determinismo de draws (secao 11: consumo de RNG e contrato canonico) ---------

TEST_CASE("turing_service: attempt_cure() consome EXATAMENTE 1 draw de rng quando "
          "os guards passam",
          "[domain][infection][turing_service][attempt_cure][determinism]") {
    CardPhysicalState p = infected_diagnosed();
    CountingRandom rng;  // next(max) -> 99 fixo (>=62 => Burned)

    REQUIRE(attempt_cure(p, CardTier::Comum, rng) == CureOutcome::Burned);
    REQUIRE(rng.next_calls == 1);
    REQUIRE(rng.next_double_calls == 0);
}

TEST_CASE("turing_service: attempt_cure() NAO consome draw de rng quando um guard "
          "barra (nem tier, nem nao-diagnosticada)",
          "[domain][infection][turing_service][attempt_cure][determinism]") {
    CountingRandom rng;

    CardPhysicalState not_diagnosed;
    not_diagnosed.is_infected = true;
    REQUIRE(attempt_cure(not_diagnosed, CardTier::Comum, rng) ==
            CureOutcome::RejectedNotDiagnosed);
    REQUIRE(rng.next_calls == 0);

    CardPhysicalState protected_tier = infected_diagnosed();
    REQUIRE(attempt_cure(protected_tier, CardTier::Especial, rng) ==
            CureOutcome::RejectedProtectedTier);
    REQUIRE(rng.next_calls == 0);
}

// ---- Retry: reinfeccao pos-cura / reattempt pos-queima -----------------------------

TEST_CASE("turing_service: apos Cured a carta pode ser reinfectada (ciclo completo "
          "via set direto - a rolagem de contaminacao em si e onda futura)",
          "[domain][infection][turing_service][attempt_cure][retry]") {
    CardPhysicalState p = infected_diagnosed();
    FixedRandom cure_rng(0.5, 0);  // <62 => Cured
    REQUIRE(attempt_cure(p, CardTier::Comum, cure_rng) == CureOutcome::Cured);
    REQUIRE_NOTHROW(p.validate());

    // Reinfeccao (fora do escopo deste servico - so verifica que o estado limpo
    // aceita reentrar no ciclo InfectedHidden -> Diagnosed -> AttemptCure).
    p.is_infected = true;
    p.virus_kind = VirusKind::ZipBomb;
    REQUIRE_NOTHROW(p.validate());
    REQUIRE(diagnose(p) == DiagnoseOutcome::Diagnosed);

    FixedRandom burn_rng(0.5, 99);
    REQUIRE(attempt_cure(p, CardTier::Comum, burn_rng) == CureOutcome::Burned);
    REQUIRE(p.is_burned_out);
}

TEST_CASE("turing_service: attempt_cure() numa carta JA sucata ainda rola (nao ha "
          "gate de is_burned_out neste servico - a carta so nao roda mais em "
          "combate, gate de gameplay futuro)",
          "[domain][infection][turing_service][attempt_cure][retry]") {
    CardPhysicalState p = infected_diagnosed();
    p.is_burned_out = true;  // ja sucata de uma tentativa anterior
    FixedRandom rng(0.5, 0);  // <62 => Cured

    REQUIRE(attempt_cure(p, CardTier::Comum, rng) == CureOutcome::Cured);
    // Cured limpa infeccao mas este servico NAO reverte is_burned_out (permanente,
    // so a redencao scriptada do Dante restaura - fora deste sistema geral).
    REQUIRE(p.is_burned_out);
    REQUIRE_FALSE(p.is_infected);
    REQUIRE_NOTHROW(p.validate());
}

// ---- Fiacao ao deck REAL (CARDS-HW-QA1-A1): diagnose_in_deck/attempt_cure_in_deck -
// alcancam a instancia JA VIVA no deck ATIVO via CardCollection::apply_to_physical -
// fecha o gap que a auditoria achou (diagnose()/attempt_cure() sozinhos so mexem num
// CardPhysicalState solto que o chamador mantinha por fora).

TEST_CASE("turing_service: diagnose_in_deck diagnostica a instancia REAL do deck "
          "ativo (nao uma copia perdida)",
          "[domain][infection][turing_service][in_deck]") {
    CardCollection deck(kDeckCapacityTier1);
    const auto inst = deck.add_to_active("pulso_infectado");
    // Injeta infeccao oculta direto no container (a rolagem de contaminacao em si e
    // de outro servico - aqui so precisamos de uma instancia infectada JA no deck).
    deck.apply_to_physical(inst.instance_id, [](const std::string&, CardPhysicalState& p) {
        p.is_infected = true;
        p.virus_kind = VirusKind::LogicBomb;
    });
    REQUIRE_FALSE(deck.active().front().physical.is_diagnosed);

    const DiagnoseOutcome outcome = diagnose_in_deck(deck, inst.instance_id);

    REQUIRE(outcome == DiagnoseOutcome::Diagnosed);
    // A mutacao esta no CONTAINER real, nao numa copia solta.
    REQUIRE(deck.active().front().physical.is_diagnosed);
}

TEST_CASE("turing_service: diagnose_in_deck e no-op numa instancia limpa (nada muta "
          "no deck real)",
          "[domain][infection][turing_service][in_deck]") {
    CardCollection deck(kDeckCapacityTier1);
    const auto inst = deck.add_to_active("pulso_limpo");

    const DiagnoseOutcome outcome = diagnose_in_deck(deck, inst.instance_id);

    REQUIRE(outcome == DiagnoseOutcome::RejectedNotInfected);
    REQUIRE_FALSE(deck.active().front().physical.is_diagnosed);
}

TEST_CASE("turing_service: diagnose_in_deck de instance_id ausente do ativo propaga "
          "fail-fast (apply_to_physical)",
          "[domain][infection][turing_service][in_deck]") {
    CardCollection deck(kDeckCapacityTier1);
    REQUIRE_THROWS_AS(diagnose_in_deck(deck, 999), std::invalid_argument);
}

TEST_CASE("turing_service: attempt_cure_in_deck cura (roll<62) a instancia REAL do "
          "deck ativo",
          "[domain][infection][turing_service][in_deck]") {
    CardCollection deck(kDeckCapacityTier1);
    const auto inst = deck.add_to_active("pulso_infectado");
    deck.apply_to_physical(inst.instance_id, [](const std::string&, CardPhysicalState& p) {
        p.is_infected = true;
        p.virus_kind = VirusKind::Worm;
        p.is_diagnosed = true;
    });

    FixedRandom cure_rng(0.5, 0);  // <62 => Cured
    const CureOutcome outcome =
        attempt_cure_in_deck(deck, inst.instance_id, fake_tier_of, cure_rng);

    REQUIRE(outcome == CureOutcome::Cured);
    const CardPhysicalState& real = deck.active().front().physical;
    REQUIRE_FALSE(real.is_infected);
    REQUIRE(real.virus_kind == VirusKind::None);
    REQUIRE_FALSE(real.is_diagnosed);
    REQUIRE_NOTHROW(real.validate());
}

TEST_CASE("turing_service: attempt_cure_in_deck queima (roll>=62) a instancia REAL "
          "do deck ativo - sucata, permanece no ativo (AMB-T1)",
          "[domain][infection][turing_service][in_deck]") {
    CardCollection deck(kDeckCapacityTier1);
    const auto inst = deck.add_to_active("pulso_infectado");
    deck.apply_to_physical(inst.instance_id, [](const std::string&, CardPhysicalState& p) {
        p.is_infected = true;
        p.virus_kind = VirusKind::Worm;
        p.is_diagnosed = true;
    });

    FixedRandom burn_rng(0.5, 99);  // >=62 => Burned
    const CureOutcome outcome =
        attempt_cure_in_deck(deck, inst.instance_id, fake_tier_of, burn_rng);

    REQUIRE(outcome == CureOutcome::Burned);
    REQUIRE(deck.active_count() == 1);  // sucata NAO sai do ativo (AMB-T1)
    const CardPhysicalState& real = deck.active().front().physical;
    REQUIRE(real.is_burned_out);
    REQUIRE(real.is_infected);  // AMB-T1: queima nao reverte infeccao/virus_kind
    REQUIRE_NOTHROW(real.validate());
}

TEST_CASE("turing_service: attempt_cure_in_deck rejeita instancia nao-diagnosticada "
          "(guard) - nada muta no deck real",
          "[domain][infection][turing_service][in_deck][guard]") {
    CardCollection deck(kDeckCapacityTier1);
    const auto inst = deck.add_to_active("pulso_infectado");
    deck.apply_to_physical(inst.instance_id, [](const std::string&, CardPhysicalState& p) {
        p.is_infected = true;
        p.virus_kind = VirusKind::Worm;
        // is_diagnosed continua false de proposito.
    });
    const CardPhysicalState before = deck.active().front().physical;

    FixedRandom rng(0.5, 0);  // curaria se o guard nao barrasse
    const CureOutcome outcome = attempt_cure_in_deck(deck, inst.instance_id, fake_tier_of, rng);

    REQUIRE(outcome == CureOutcome::RejectedNotDiagnosed);
    REQUIRE(deck.active().front().physical == before);
}

TEST_CASE("turing_service: attempt_cure_in_deck rejeita tier PROTEGIDO (Especial/"
          "Super) resolvido via tier_of(card_id) - nada muta no deck real",
          "[domain][infection][turing_service][in_deck][guard]") {
    CardCollection deck(kDeckCapacityTier1);
    const auto inst = deck.add_to_active("mestre_hipotenusa_especial");
    deck.apply_to_physical(inst.instance_id, [](const std::string&, CardPhysicalState& p) {
        p.is_infected = true;
        p.virus_kind = VirusKind::Worm;
        p.is_diagnosed = true;
    });
    const CardPhysicalState before = deck.active().front().physical;

    FixedRandom rng(0.5, 0);
    const CureOutcome outcome = attempt_cure_in_deck(deck, inst.instance_id, fake_tier_of, rng);

    REQUIRE(outcome == CureOutcome::RejectedProtectedTier);
    REQUIRE(deck.active().front().physical == before);
}

TEST_CASE("turing_service: attempt_cure_in_deck de instance_id ausente do ativo "
          "propaga fail-fast (apply_to_physical)",
          "[domain][infection][turing_service][in_deck]") {
    CardCollection deck(kDeckCapacityTier1);
    FixedRandom rng(0.5, 0);
    REQUIRE_THROWS_AS(attempt_cure_in_deck(deck, 999, fake_tier_of, rng),
                       std::invalid_argument);
}

// ---- Log diegetico: mapeamento PURO outcome -> chave i18n -------------------------

TEST_CASE("turing_service: translation_key_for(CureOutcome) mapeia os 4 desfechos "
          "pra chaves UPPER_SNAKE_CASE distintas",
          "[domain][infection][turing_service][i18n]") {
    REQUIRE(translation_key_for(CureOutcome::Cured) == "TURING_CURE_SUCCESS");
    REQUIRE(translation_key_for(CureOutcome::Burned) == "TURING_CURE_BURNED");
    REQUIRE(translation_key_for(CureOutcome::RejectedNotDiagnosed) ==
            "TURING_CURE_REJECTED_NOT_DIAGNOSED");
    REQUIRE(translation_key_for(CureOutcome::RejectedProtectedTier) ==
            "TURING_CURE_REJECTED_PROTECTED_TIER");
}

TEST_CASE("turing_service: translation_key_for(DiagnoseOutcome) mapeia os 2 "
          "desfechos pra chaves UPPER_SNAKE_CASE distintas",
          "[domain][infection][turing_service][i18n]") {
    REQUIRE(translation_key_for(DiagnoseOutcome::Diagnosed) == "TURING_DIAGNOSE_SUCCESS");
    REQUIRE(translation_key_for(DiagnoseOutcome::RejectedNotInfected) ==
            "TURING_DIAGNOSE_REJECTED_NOT_INFECTED");
}

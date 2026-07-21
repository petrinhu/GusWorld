// contamination_service_test.cpp
//
// Spec executavel (Catch2 v3) da rolagem de contaminacao NA AQUISICAO (CARDS-HARDWARE-
// ENGINE fatia 3 incremento B, "CARDS-HW-3B", TODO.md; gus/domain/deck/
// contamination_service.hpp; docs/design/mecanicas/cartas-spec-logica.md secao 5.1,
// pseudocodigo on_card_acquired/AMB-07 RESOLVIDA; cartas-numeros-proposta.md secao
// 3/3a).
//
// Cobre:
//   - roll_contamination_on_acquisition(): guard defensivo Especial/Super (0%, nunca
//     infecta mesmo com RNG que SEMPRE infectaria); fronteira exata de cada classe nao-
//     protegida (draw==risco-1 infecta, draw==risco NAO infecta); forca infeccao via
//     RNG fake nas 4 classes reais; pos-estado sempre passa physical.validate();
//     determinismo de draws (1 draw quando limpo/guardado, 2 quando infecta).
//   - pick_weighted_payload(): SO os 4 tipos disparaveis hoje (Backdoor/Worm/
//     LogicBomb/ZipBomb - NUNCA None/FalseBenign/AdwareSterling/IndustrialWeapon);
//     distribuicao estatistica (N=40000, PropertyRandom) casa com a tabela por classe
//     dentro de tolerancia; fallback defensivo de EspecialSelada (pesos zero) sempre
//     Backdoor.
//   - translation_key_for(): mapeamento PURO outcome -> chave i18n; a chave de
//     Infected e AMBIGUA por design (nao contem "INFECT" - a certeza fica reservada
//     ao diagnostico do Turing, secao 6).
//
// Cross-ref: gus/domain/deck/contamination_service.hpp; gus/domain/hardware/
//            hardware_class.hpp (kContaminationPercentTable); gus/domain/deck/
//            turing_service.hpp (mesmo estilo, a ponta oposta do ciclo de vida);
//            fixed_random.hpp; counting_random.hpp; property_gen.hpp.

#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <cstdint>

#include "counting_random.hpp"
#include "fixed_random.hpp"
#include "property_gen.hpp"
#include "gus/domain/cards/card_enums.hpp"
#include "gus/domain/combat/random_source.hpp"
#include "gus/domain/deck/card_hardware.hpp"
#include "gus/domain/deck/contamination_service.hpp"

using gus::domain::cards::CardTier;
using gus::domain::deck::CardOrigin;
using gus::domain::deck::CardPhysicalState;
using gus::domain::deck::ContaminationRollOutcome;
using gus::domain::deck::HardwareClass;
using gus::domain::deck::pick_weighted_payload;
using gus::domain::deck::roll_contamination_on_acquisition;
using gus::domain::deck::translation_key_for;
using gus::domain::deck::VirusKind;
using gus::domain::tests::CountingRandom;
using gus::domain::tests::FixedRandom;
using gus::domain::tests::PropertyRandom;

namespace {

// Duplo deterministico local: crava next(max) num valor FIXO (clamp identico a
// FixedRandom.cs) E conta as chamadas (mesma ideia de counting_random.hpp, so que com
// o valor cravado CONFIGURAVEL em vez de fixo em 99 - precisamos forcar infeccao E
// contar draws no mesmo teste).
class CountingFixedRandom final : public gus::domain::combat::IRandomSource {
public:
    explicit CountingFixedRandom(int fixed_roll) : fixed_roll_(fixed_roll) {}

    double next_double() override {
        ++next_double_calls;
        return 0.5;
    }
    int next(int max_value) override {
        ++next_calls;
        if (max_value <= 0) return 0;
        return fixed_roll_ < max_value ? fixed_roll_ : max_value - 1;
    }

    int next_calls = 0;
    int next_double_calls = 0;

private:
    int fixed_roll_;
};

// Um payload dos 4 disparaveis hoje (secao 3a) - usado pra checar o dominio do
// resultado sem depender de qual exatamente saiu.
bool is_allowed_payload(VirusKind kind) {
    return kind == VirusKind::Backdoor || kind == VirusKind::Worm ||
           kind == VirusKind::LogicBomb || kind == VirusKind::ZipBomb;
}

}  // namespace

// ---- Guard defensivo: Especial/Super NUNCA infectam (0%, cartas-spec-logica secao
// 5.1) ---------------------------------------------------------------------------

TEST_CASE("contamination_service: Especial nunca infecta mesmo com RNG que SEMPRE "
          "infectaria (guard defensivo)",
          "[domain][deck][contamination_service][guard]") {
    CardPhysicalState p;  // origin default OriginalRom
    CardPhysicalState before = p;
    FixedRandom rng(0.5, 0);  // draw=0 - infectaria qualquer classe nao-protegida

    const ContaminationRollOutcome outcome =
        roll_contamination_on_acquisition(p, CardTier::Especial, /*mimics_special=*/false, rng);

    REQUIRE(outcome == ContaminationRollOutcome::SkippedProtectedTier);
    REQUIRE(p == before);
    REQUIRE_FALSE(p.is_infected);
    REQUIRE(p.virus_kind == VirusKind::None);
    REQUIRE_NOTHROW(p.validate());
}

TEST_CASE("contamination_service: Super nunca infecta mesmo com RNG que SEMPRE "
          "infectaria (guard defensivo)",
          "[domain][deck][contamination_service][guard]") {
    CardPhysicalState p;
    CardPhysicalState before = p;
    FixedRandom rng(0.5, 0);

    const ContaminationRollOutcome outcome =
        roll_contamination_on_acquisition(p, CardTier::Super, /*mimics_special=*/false, rng);

    REQUIRE(outcome == ContaminationRollOutcome::SkippedProtectedTier);
    REQUIRE(p == before);
    REQUIRE_NOTHROW(p.validate());
}

// ---- Forca infeccao via RNG fake, nas 4 classes reais (cartas-numeros secao 3) ----

TEST_CASE("contamination_service: forca infeccao (draw=0) em ComumOriginal (risco 1%)",
          "[domain][deck][contamination_service][force_infect]") {
    CardPhysicalState p;  // OriginalRom
    FixedRandom rng(0.5, 0);

    const ContaminationRollOutcome outcome =
        roll_contamination_on_acquisition(p, CardTier::Comum, /*mimics_special=*/false, rng);

    REQUIRE(outcome == ContaminationRollOutcome::Infected);
    REQUIRE(p.is_infected);
    REQUIRE(is_allowed_payload(p.virus_kind));
    REQUIRE_NOTHROW(p.validate());
}

TEST_CASE("contamination_service: forca infeccao (draw=0) em HomebrewEprom (risco 55%)",
          "[domain][deck][contamination_service][force_infect]") {
    CardPhysicalState p;
    p.origin = CardOrigin::HomebrewEprom;
    FixedRandom rng(0.5, 0);

    const ContaminationRollOutcome outcome =
        roll_contamination_on_acquisition(p, CardTier::Comum, /*mimics_special=*/false, rng);

    REQUIRE(outcome == ContaminationRollOutcome::Infected);
    REQUIRE(p.is_infected);
    REQUIRE(is_allowed_payload(p.virus_kind));
    REQUIRE_NOTHROW(p.validate());
}

TEST_CASE("contamination_service: forca infeccao (draw=0) em PirataComum (risco 21%)",
          "[domain][deck][contamination_service][force_infect]") {
    CardPhysicalState p;
    p.origin = CardOrigin::PirateClone;
    FixedRandom rng(0.5, 0);

    const ContaminationRollOutcome outcome =
        roll_contamination_on_acquisition(p, CardTier::Comum, /*mimics_special=*/false, rng);

    REQUIRE(outcome == ContaminationRollOutcome::Infected);
    REQUIRE(p.is_infected);
    REQUIRE(is_allowed_payload(p.virus_kind));
    REQUIRE_NOTHROW(p.validate());
}

TEST_CASE("contamination_service: forca infeccao (draw=0) em PirataEspecialFalso "
          "(clone-falso, mimics_special vence origin, risco 8%)",
          "[domain][deck][contamination_service][force_infect]") {
    CardPhysicalState p;  // origin qualquer - mimics_special VENCE (hardware_class_of)
    FixedRandom rng(0.5, 0);

    const ContaminationRollOutcome outcome =
        roll_contamination_on_acquisition(p, CardTier::Comum, /*mimics_special=*/true, rng);

    REQUIRE(outcome == ContaminationRollOutcome::Infected);
    REQUIRE(p.is_infected);
    REQUIRE(is_allowed_payload(p.virus_kind));
    REQUIRE_NOTHROW(p.validate());
}

// ---- Fronteira exata de cada classe (draw==risco-1 infecta, draw==risco NAO) ------

TEST_CASE("contamination_service: fronteira ComumOriginal (risco 1%) - draw=0 infecta, "
          "draw=1 NAO",
          "[domain][deck][contamination_service][boundary]") {
    {
        CardPhysicalState p;
        FixedRandom rng(0.5, 0);
        REQUIRE(roll_contamination_on_acquisition(p, CardTier::Comum, false, rng) ==
                ContaminationRollOutcome::Infected);
    }
    {
        CardPhysicalState p;
        CardPhysicalState before = p;
        FixedRandom rng(0.5, 1);
        REQUIRE(roll_contamination_on_acquisition(p, CardTier::Comum, false, rng) ==
                ContaminationRollOutcome::Clean);
        REQUIRE(p == before);
    }
}

TEST_CASE("contamination_service: fronteira PirataEspecialFalso (risco 8%) - draw=7 "
          "infecta, draw=8 NAO",
          "[domain][deck][contamination_service][boundary]") {
    {
        CardPhysicalState p;
        FixedRandom rng(0.5, 7);
        REQUIRE(roll_contamination_on_acquisition(p, CardTier::Comum, /*mimics=*/true, rng) ==
                ContaminationRollOutcome::Infected);
    }
    {
        CardPhysicalState p;
        CardPhysicalState before = p;
        FixedRandom rng(0.5, 8);
        REQUIRE(roll_contamination_on_acquisition(p, CardTier::Comum, /*mimics=*/true, rng) ==
                ContaminationRollOutcome::Clean);
        REQUIRE(p == before);
    }
}

TEST_CASE("contamination_service: fronteira PirataComum (risco 21%) - draw=20 infecta, "
          "draw=21 NAO",
          "[domain][deck][contamination_service][boundary]") {
    {
        CardPhysicalState p;
        p.origin = CardOrigin::PirateClone;
        FixedRandom rng(0.5, 20);
        REQUIRE(roll_contamination_on_acquisition(p, CardTier::Comum, false, rng) ==
                ContaminationRollOutcome::Infected);
    }
    {
        CardPhysicalState p;
        p.origin = CardOrigin::PirateClone;
        CardPhysicalState before = p;
        FixedRandom rng(0.5, 21);
        REQUIRE(roll_contamination_on_acquisition(p, CardTier::Comum, false, rng) ==
                ContaminationRollOutcome::Clean);
        REQUIRE(p == before);
    }
}

TEST_CASE("contamination_service: fronteira HomebrewEprom (risco 55%) - draw=54 "
          "infecta, draw=55 NAO",
          "[domain][deck][contamination_service][boundary]") {
    {
        CardPhysicalState p;
        p.origin = CardOrigin::HomebrewEprom;
        FixedRandom rng(0.5, 54);
        REQUIRE(roll_contamination_on_acquisition(p, CardTier::Comum, false, rng) ==
                ContaminationRollOutcome::Infected);
    }
    {
        CardPhysicalState p;
        p.origin = CardOrigin::HomebrewEprom;
        CardPhysicalState before = p;
        FixedRandom rng(0.5, 55);
        REQUIRE(roll_contamination_on_acquisition(p, CardTier::Comum, false, rng) ==
                ContaminationRollOutcome::Clean);
        REQUIRE(p == before);
    }
}

// ---- Aquisicao legitima (Comum 1%) raramente infecta (estatistico, N=40000) -------

TEST_CASE("contamination_service: aquisicao legitima ComumOriginal (1%) infecta "
          "raramente sob RNG seedado (N=40000, tolerancia generosa)",
          "[domain][deck][contamination_service][stats]") {
    constexpr int kTrials = 40000;
    int infected = 0;
    PropertyRandom rng(2026u);
    for (int i = 0; i < kTrials; ++i) {
        CardPhysicalState p;
        if (roll_contamination_on_acquisition(p, CardTier::Comum, false, rng) ==
            ContaminationRollOutcome::Infected)
            ++infected;
    }
    const double ratio = static_cast<double>(infected) / static_cast<double>(kTrials);
    // ~1% esperado; tolerancia +-0,5pp (bem acima do desvio-padrao esperado ~0,05pp
    // pra N=40000 - so garante que nao virou "sempre" nem "nunca").
    REQUIRE(std::abs(ratio - 0.01) < 0.005);
}

// ---- pick_weighted_payload(): dominio fechado nos 4 tipos disparaveis hoje --------

TEST_CASE("contamination_service: pick_weighted_payload() NUNCA sorteia "
          "None/FalseBenign/AdwareSterling/IndustrialWeapon (N=4000 por classe)",
          "[domain][deck][contamination_service][payload][domain]") {
    const HardwareClass classes[] = {HardwareClass::ComumOriginal, HardwareClass::HomebrewEprom,
                                      HardwareClass::PirataComum,
                                      HardwareClass::PirataEspecialFalso,
                                      HardwareClass::EspecialSelada};
    PropertyRandom rng(555u);
    for (const HardwareClass cls : classes) {
        for (int i = 0; i < 4000; ++i) {
            const VirusKind kind = pick_weighted_payload(cls, rng);
            REQUIRE(is_allowed_payload(kind));
        }
    }
}

TEST_CASE("contamination_service: pick_weighted_payload(EspecialSelada) e fallback "
          "defensivo SEMPRE Backdoor (pesos zero, classe inalcancavel em jogo normal)",
          "[domain][deck][contamination_service][payload][guard]") {
    for (const int draw : {0, 1, 50, 99}) {
        FixedRandom rng(0.5, draw);
        REQUIRE(pick_weighted_payload(HardwareClass::EspecialSelada, rng) == VirusKind::Backdoor);
    }
}

// ---- Distribuicao estatistica por classe (N=40000, cartas-numeros secao 3a/AMB-07) -

TEST_CASE("contamination_service: distribuicao ComumOriginal casa com 52/32/12/4% "
          "dentro de tolerancia",
          "[domain][deck][contamination_service][payload][stats]") {
    constexpr int kTrials = 40000;
    int backdoor = 0, worm = 0, logic_bomb = 0, zip_bomb = 0;
    PropertyRandom rng(11111u);
    for (int i = 0; i < kTrials; ++i) {
        switch (pick_weighted_payload(HardwareClass::ComumOriginal, rng)) {
            case VirusKind::Backdoor: ++backdoor; break;
            case VirusKind::Worm: ++worm; break;
            case VirusKind::LogicBomb: ++logic_bomb; break;
            case VirusKind::ZipBomb: ++zip_bomb; break;
            default: FAIL("payload fora do dominio permitido"); break;
        }
    }
    const double n = static_cast<double>(kTrials);
    REQUIRE(std::abs(backdoor / n - 0.520) < 0.02);
    REQUIRE(std::abs(worm / n - 0.320) < 0.02);
    REQUIRE(std::abs(logic_bomb / n - 0.120) < 0.02);
    REQUIRE(std::abs(zip_bomb / n - 0.040) < 0.02);
}

TEST_CASE("contamination_service: distribuicao HomebrewEprom (espelho EXATO de "
          "ComumOriginal) casa com 4/12/32/52% dentro de tolerancia",
          "[domain][deck][contamination_service][payload][stats]") {
    constexpr int kTrials = 40000;
    int backdoor = 0, worm = 0, logic_bomb = 0, zip_bomb = 0;
    PropertyRandom rng(22222u);
    for (int i = 0; i < kTrials; ++i) {
        switch (pick_weighted_payload(HardwareClass::HomebrewEprom, rng)) {
            case VirusKind::Backdoor: ++backdoor; break;
            case VirusKind::Worm: ++worm; break;
            case VirusKind::LogicBomb: ++logic_bomb; break;
            case VirusKind::ZipBomb: ++zip_bomb; break;
            default: FAIL("payload fora do dominio permitido"); break;
        }
    }
    const double n = static_cast<double>(kTrials);
    REQUIRE(std::abs(backdoor / n - 0.040) < 0.02);
    REQUIRE(std::abs(worm / n - 0.120) < 0.02);
    REQUIRE(std::abs(logic_bomb / n - 0.320) < 0.02);
    REQUIRE(std::abs(zip_bomb / n - 0.520) < 0.02);
}

TEST_CASE("contamination_service: distribuicao PirataComum casa com 12,5/20,8/33,3/"
          "33,3% dentro de tolerancia",
          "[domain][deck][contamination_service][payload][stats]") {
    constexpr int kTrials = 40000;
    int backdoor = 0, worm = 0, logic_bomb = 0, zip_bomb = 0;
    PropertyRandom rng(33333u);
    for (int i = 0; i < kTrials; ++i) {
        switch (pick_weighted_payload(HardwareClass::PirataComum, rng)) {
            case VirusKind::Backdoor: ++backdoor; break;
            case VirusKind::Worm: ++worm; break;
            case VirusKind::LogicBomb: ++logic_bomb; break;
            case VirusKind::ZipBomb: ++zip_bomb; break;
            default: FAIL("payload fora do dominio permitido"); break;
        }
    }
    const double n = static_cast<double>(kTrials);
    REQUIRE(std::abs(backdoor / n - 0.125) < 0.02);
    REQUIRE(std::abs(worm / n - 0.208) < 0.02);
    REQUIRE(std::abs(logic_bomb / n - 0.333) < 0.02);
    REQUIRE(std::abs(zip_bomb / n - 0.333) < 0.02);
}

TEST_CASE("contamination_service: distribuicao PirataEspecialFalso (espelho EXATO de "
          "PirataComum) casa com 33,3/33,3/20,8/12,5% dentro de tolerancia",
          "[domain][deck][contamination_service][payload][stats]") {
    constexpr int kTrials = 40000;
    int backdoor = 0, worm = 0, logic_bomb = 0, zip_bomb = 0;
    PropertyRandom rng(44444u);
    for (int i = 0; i < kTrials; ++i) {
        switch (pick_weighted_payload(HardwareClass::PirataEspecialFalso, rng)) {
            case VirusKind::Backdoor: ++backdoor; break;
            case VirusKind::Worm: ++worm; break;
            case VirusKind::LogicBomb: ++logic_bomb; break;
            case VirusKind::ZipBomb: ++zip_bomb; break;
            default: FAIL("payload fora do dominio permitido"); break;
        }
    }
    const double n = static_cast<double>(kTrials);
    REQUIRE(std::abs(backdoor / n - 0.333) < 0.02);
    REQUIRE(std::abs(worm / n - 0.333) < 0.02);
    REQUIRE(std::abs(logic_bomb / n - 0.208) < 0.02);
    REQUIRE(std::abs(zip_bomb / n - 0.125) < 0.02);
}

// ---- Determinismo de draws (secao 11: consumo de RNG e contrato canonico) ---------

TEST_CASE("contamination_service: roll_contamination_on_acquisition() consome "
          "EXATAMENTE 1 draw quando fica limpo",
          "[domain][deck][contamination_service][determinism]") {
    CardPhysicalState p;  // ComumOriginal, risco 1%
    CountingRandom rng;   // next(max) -> 99 fixo (>=1% => Clean)

    REQUIRE(roll_contamination_on_acquisition(p, CardTier::Comum, false, rng) ==
            ContaminationRollOutcome::Clean);
    REQUIRE(rng.next_calls == 1);
    REQUIRE(rng.next_double_calls == 0);
}

TEST_CASE("contamination_service: roll_contamination_on_acquisition() consome "
          "EXATAMENTE 2 draws quando infecta (risco + payload)",
          "[domain][deck][contamination_service][determinism]") {
    CardPhysicalState p;
    CountingFixedRandom rng(0);  // draw=0 - infecta qualquer classe nao-protegida

    REQUIRE(roll_contamination_on_acquisition(p, CardTier::Comum, false, rng) ==
            ContaminationRollOutcome::Infected);
    REQUIRE(rng.next_calls == 2);
    REQUIRE(rng.next_double_calls == 0);
}

TEST_CASE("contamination_service: roll_contamination_on_acquisition() NAO consome "
          "NENHUM draw quando o guard de tier protegido barra",
          "[domain][deck][contamination_service][determinism]") {
    CardPhysicalState p;
    CountingFixedRandom rng(0);

    REQUIRE(roll_contamination_on_acquisition(p, CardTier::Especial, false, rng) ==
            ContaminationRollOutcome::SkippedProtectedTier);
    REQUIRE(rng.next_calls == 0);
    REQUIRE(rng.next_double_calls == 0);
}

// ---- Log diegetico AMBIGUO: mapeamento PURO outcome -> chave i18n -----------------

TEST_CASE("contamination_service: translation_key_for() mapeia os 3 desfechos pra "
          "chaves UPPER_SNAKE_CASE distintas",
          "[domain][deck][contamination_service][i18n]") {
    const auto infected_key = translation_key_for(ContaminationRollOutcome::Infected);
    const auto clean_key = translation_key_for(ContaminationRollOutcome::Clean);
    const auto skipped_key = translation_key_for(ContaminationRollOutcome::SkippedProtectedTier);

    REQUIRE(infected_key != clean_key);
    REQUIRE(infected_key != skipped_key);
    REQUIRE(clean_key != skipped_key);
}

TEST_CASE("contamination_service: a chave de Infected e AMBIGUA por design - NAO "
          "confirma infeccao (a certeza fica reservada ao diagnostico do Turing, "
          "secao 6)",
          "[domain][deck][contamination_service][i18n]") {
    const std::string_view infected_key = translation_key_for(ContaminationRollOutcome::Infected);
    // "INFECT" nao aparece na chave - o log e um AVISO ambiguo ("checksum estranho"),
    // nao uma confirmacao. Decisao do lider, CARDS-HW-3B.
    REQUIRE(infected_key.find("INFECT") == std::string_view::npos);
}

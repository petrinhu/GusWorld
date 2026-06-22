// environment_catalog_test.cpp
//
// Spec executavel (Catch2 v3) do catalogo data-driven de ambientes, portada de
// engine/foundation/turn_combat/EnvironmentCatalog.cs (origem canonica) e dos casos de
// engine/tests/turn_combat/environments/EnvironmentCatalogTests.cs (xUnit = SPEC).
// POCO puro, ZERO Qt, headless. Marco M5 (chunk 3: subsistema de ambiente).
//
// Valida que TODOS os numeros (mults de familia 0.66..1.5, duracoes de periodo,
// status facilitado, hardware hooks) batem com a spec canonica secao 18 -
// gameplay_engineer NAO inventa valor.
//
// Cross-ref: engine/foundation/turn_combat/EnvironmentCatalog.cs;
//            engine/tests/turn_combat/environments/EnvironmentCatalogTests.cs;
//            docs/design/mecanicas/combat.md secao 18.

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <array>
#include <stdexcept>

#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/combat/environment_catalog.hpp"
#include "gus/domain/combat/environment_enums.hpp"

using namespace gus::domain::combat;
using Catch::Approx;

namespace {
constexpr std::array<CardFamily, 5> kAllFamilies = {
    CardFamily::Eletrico, CardFamily::Bioquimico, CardFamily::Sonico,
    CardFamily::Cinetico, CardFamily::Criptografico,
};
}  // namespace

// ---- Cobertura: 12 terrenos + 8 climas + 4 periodos + None = 28 (F2-PROD.4) -------

TEST_CASE("environment_catalog: catalogo completo contem None e todas as camadas",
          "[domain][combat][environment]") {
    const auto& all = EnvironmentCatalog::all();

    // None + 8 clima + 4 periodo + 15 terreno (12 visiveis + 3 codex) = 28 entradas.
    REQUIRE(all.size() == 28u);
    REQUIRE(all.contains(EnvironmentId::None));

    int clima = 0;
    int periodo = 0;
    int terreno = 0;
    for (const auto& [id, env] : all) {
        if (env.layer == EnvironmentLayer::Clima) ++clima;
        if (env.layer == EnvironmentLayer::Periodo) ++periodo;
        if (env.layer == EnvironmentLayer::Terreno) ++terreno;
    }
    REQUIRE(clima == 8);
    REQUIRE(periodo == 4);
    // 15 terrenos efetivos + None (marcado Terreno arbitrariamente) = 16.
    REQUIRE(terreno == 16);
}

TEST_CASE("environment_catalog: tres terrenos sao tier codex",
          "[domain][combat][environment]") {
    int codex = 0;
    for (const auto& [id, env] : EnvironmentCatalog::all())
        if (env.tier == EnvironmentTier::Codex) ++codex;
    REQUIRE(codex == 3);

    REQUIRE(EnvironmentCatalog::get(EnvironmentId::EspelhoRessonante).tier == EnvironmentTier::Codex);
    REQUIRE(EnvironmentCatalog::get(EnvironmentId::DutoCondutorPressurizado).tier == EnvironmentTier::Codex);
    REQUIRE(EnvironmentCatalog::get(EnvironmentId::ElevacaoDominante).tier == EnvironmentTier::Codex);
}

// ---- Mults de familia estao na faixa canonica 0.66..1.5 (canal 1) -----------------

TEST_CASE("environment_catalog: todos os mults de familia estao na faixa 0.66..1.5",
          "[domain][combat][environment]") {
    for (const auto& [id, env] : EnvironmentCatalog::all())
        for (const auto& [family, mult] : env.family_mults) {
            REQUIRE(mult >= 0.66f);
            REQUIRE(mult <= 1.5f);
        }
}

// ---- Valores de familia por ambiente (transcritos de secao 18.2/3/4/5) ------------

TEST_CASE("environment_catalog: mult_for bate com a spec",
          "[domain][combat][environment]") {
    struct Case {
        EnvironmentId id;
        CardFamily family;
        float expected;
    };
    const Case cases[] = {
        // CLIMA secao 18.2
        {EnvironmentId::Neblina, CardFamily::Sonico, 1.3f},
        {EnvironmentId::Neblina, CardFamily::Criptografico, 0.66f},
        {EnvironmentId::Chuva, CardFamily::Eletrico, 1.5f},
        {EnvironmentId::Chuva, CardFamily::Bioquimico, 0.66f},
        {EnvironmentId::Calor, CardFamily::Bioquimico, 1.3f},
        {EnvironmentId::Calor, CardFamily::Eletrico, 0.66f},
        {EnvironmentId::TempestadeEletrica, CardFamily::Eletrico, 1.5f},
        {EnvironmentId::TempestadeEletrica, CardFamily::Criptografico, 0.66f},
        {EnvironmentId::Vento, CardFamily::Sonico, 1.3f},
        {EnvironmentId::Vento, CardFamily::Bioquimico, 0.66f},
        {EnvironmentId::Estatica, CardFamily::Criptografico, 1.5f},
        {EnvironmentId::Estatica, CardFamily::Sonico, 0.66f},
        {EnvironmentId::Fumaca, CardFamily::Bioquimico, 1.3f},
        {EnvironmentId::Fumaca, CardFamily::Criptografico, 0.66f},
        {EnvironmentId::EscuridaoTotal, CardFamily::Sonico, 1.5f},
        {EnvironmentId::EscuridaoTotal, CardFamily::Criptografico, 0.85f},
        // PERIODO secao 18.3
        {EnvironmentId::Dia, CardFamily::Bioquimico, 1.3f},
        {EnvironmentId::Dia, CardFamily::Criptografico, 0.85f},
        {EnvironmentId::Noite, CardFamily::Criptografico, 1.5f},
        {EnvironmentId::Noite, CardFamily::Sonico, 1.3f},
        {EnvironmentId::Noite, CardFamily::Bioquimico, 0.85f},
        {EnvironmentId::Aurora, CardFamily::Eletrico, 1.3f},
        // TERRENO secao 18.4
        {EnvironmentId::Lamacento, CardFamily::Eletrico, 1.3f},
        {EnvironmentId::Lamacento, CardFamily::Cinetico, 0.66f},
        {EnvironmentId::Seco, CardFamily::Cinetico, 1.3f},
        {EnvironmentId::Seco, CardFamily::Eletrico, 0.66f},
        {EnvironmentId::Gelo, CardFamily::Cinetico, 1.3f},
        {EnvironmentId::Gelo, CardFamily::Bioquimico, 0.66f},
        {EnvironmentId::MetalCondutor, CardFamily::Eletrico, 1.3f},
        {EnvironmentId::MetalCondutor, CardFamily::Sonico, 0.66f},
        {EnvironmentId::PavimentoTesselado, CardFamily::Criptografico, 1.3f},
        {EnvironmentId::PavimentoTesselado, CardFamily::Sonico, 0.66f},
        {EnvironmentId::TaludeInstavel, CardFamily::Cinetico, 1.5f},
        {EnvironmentId::TaludeInstavel, CardFamily::Criptografico, 0.66f},
        {EnvironmentId::SoloFertilRecursivo, CardFamily::Bioquimico, 1.5f},
        {EnvironmentId::SoloFertilRecursivo, CardFamily::Cinetico, 0.66f},
        // TERRENO CODEX secao 18.5
        {EnvironmentId::EspelhoRessonante, CardFamily::Sonico, 1.5f},
        {EnvironmentId::EspelhoRessonante, CardFamily::Bioquimico, 0.66f},
        {EnvironmentId::DutoCondutorPressurizado, CardFamily::Eletrico, 1.3f},
        {EnvironmentId::ElevacaoDominante, CardFamily::Cinetico, 1.3f},
        {EnvironmentId::ElevacaoDominante, CardFamily::Sonico, 0.66f},
    };
    for (const auto& c : cases) {
        const auto& env = EnvironmentCatalog::get(c.id);
        REQUIRE(env.mult_for(c.family) == Approx(c.expected).margin(1e-4));
    }
}

TEST_CASE("environment_catalog: familia nao listada e neutra 1.0",
          "[domain][combat][environment]") {
    // Chuva afeta Eletrico (up) e Bioquimico (down); Sonico nao esta listado => 1.0.
    const auto& chuva = EnvironmentCatalog::get(EnvironmentId::Chuva);
    REQUIRE(chuva.mult_for(CardFamily::Sonico) == Approx(1.0f).margin(1e-4));
}

// ---- Status facilitado reusa o enum existente (secao 9) - NENHUM status novo -------

TEST_CASE("environment_catalog: status facilitado e o delta correto",
          "[domain][combat][environment]") {
    struct Case {
        EnvironmentId id;
        StatusId status;
        int mag_delta;
        int dur_delta;
    };
    const Case cases[] = {
        {EnvironmentId::Neblina, StatusId::Disrupt, 1, 0},           // Disrupt +1 mag
        {EnvironmentId::Chuva, StatusId::Stun, 0, 1},                // Stun +1 dur
        {EnvironmentId::Calor, StatusId::Corrode, 0, 1},             // Corrode +1 dur
        {EnvironmentId::PavimentoTesselado, StatusId::Expose, 13, 0},  // Expose mag 13 (Fibonacci)
        {EnvironmentId::TaludeInstavel, StatusId::Slow, 2, 0},       // pune inativo: Slow mag 2
        {EnvironmentId::Vinhas, StatusId::Slow, 1, 0},               // Root = Slow extremo
    };
    for (const auto& c : cases) {
        const auto& env = EnvironmentCatalog::get(c.id);
        REQUIRE(env.facilitated_status.has_value());
        const auto fs = env.facilitated_status.value();
        REQUIRE(fs.id == c.status);
        REQUIRE(fs.magnitude_delta == c.mag_delta);
        REQUIRE(fs.duration_delta == c.dur_delta);
    }
}

// ---- Periodo: duracoes Fibonacci 5/2/5/2 (secao 18.3) -----------------------------

TEST_CASE("environment_catalog: periodo tem duracao canonica 5/2/5/2",
          "[domain][combat][environment]") {
    REQUIRE(EnvironmentCatalog::get(EnvironmentId::Dia).period_duration == 5);
    REQUIRE(EnvironmentCatalog::get(EnvironmentId::Crepusculo).period_duration == 2);
    REQUIRE(EnvironmentCatalog::get(EnvironmentId::Noite).period_duration == 5);
    REQUIRE(EnvironmentCatalog::get(EnvironmentId::Aurora).period_duration == 2);
}

// ---- Hardware hooks (canal 4): Scan AP / Scan gratis / Prever ---------------------

TEST_CASE("environment_catalog: EscuridaoTotal encarece scan em 2 ap",
          "[domain][combat][environment]") {
    REQUIRE(EnvironmentCatalog::get(EnvironmentId::EscuridaoTotal).hardware.scan_ap_delta == 2);
}

TEST_CASE("environment_catalog: Dia e MetalCondutor dao scan gratis",
          "[domain][combat][environment]") {
    REQUIRE(EnvironmentCatalog::get(EnvironmentId::Dia).hardware.scan_free);
    REQUIRE(EnvironmentCatalog::get(EnvironmentId::MetalCondutor).hardware.scan_free);
}

TEST_CASE("environment_catalog: Neblina encurta Prever para 1 turno",
          "[domain][combat][environment]") {
    // "Prever so 1 turno a frente" = delta -1.
    REQUIRE(EnvironmentCatalog::get(EnvironmentId::Neblina).hardware.prever_turn_delta == -1);
}

// ---- T6 Anomalia Perlin (secao 18.4): NAO mexe no dano ----------------------------

TEST_CASE("environment_catalog: AnomaliaPerlin nao tem mult de familia nenhuma",
          "[domain][combat][environment]") {
    const auto& t6 = EnvironmentCatalog::get(EnvironmentId::AnomaliaPerlin);
    REQUIRE(t6.family_mults.empty());
    for (const auto f : kAllFamilies)
        REQUIRE(t6.mult_for(f) == Approx(1.0f).margin(1e-4));
}

// ---- None acessivel via helper dedicado -------------------------------------------

TEST_CASE("environment_catalog: none() devolve o ambiente neutro",
          "[domain][combat][environment]") {
    REQUIRE(EnvironmentCatalog::none().id == EnvironmentId::None);
}

// ---- Get id inexistente lanca (KeyNotFound -> out_of_range) ------------------------

TEST_CASE("environment_catalog: get de id inexistente lanca",
          "[domain][combat][environment]") {
    REQUIRE_THROWS_AS(EnvironmentCatalog::get(static_cast<EnvironmentId>(999u)), std::out_of_range);
}

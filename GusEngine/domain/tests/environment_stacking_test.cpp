// environment_stacking_test.cpp
//
// Spec executavel (Catch2 v3) do STACKING de 3 camadas e do CAP final de mult_ambiente
// (secao 11/18), portada de
// engine/tests/turn_combat/environments/EnvironmentStackingTests.cs (xUnit = SPEC) e da
// fonte EnvironmentCatalog.cs. POCO puro, ZERO Qt, headless. Marco M5 (chunk 3).
//
// Camadas ativas que afetam a MESMA familia se MULTIPLICAM, com cap final [0.44, 2.25].
// Foco em boundary e off-by-one (combinacao que daria 2.3 clampa 2.25; 0.4 clampa 0.44).
// Aritmetica em float, espelhando o C# (Math.Clamp sobre produto float).
//
// Cross-ref: engine/tests/turn_combat/environments/EnvironmentStackingTests.cs;
//            docs/design/mecanicas/combat.md secao 11 (stacking/cap)/18.

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <array>
#include <vector>

#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/combat/environment_catalog.hpp"
#include "gus/domain/combat/environment_enums.hpp"
#include "gus/domain/combat/environment_modifier.hpp"

using namespace gus::domain::combat;
using Catch::Approx;

namespace {

constexpr std::array<CardFamily, 5> kAllFamilies = {
    CardFamily::Eletrico, CardFamily::Bioquimico, CardFamily::Sonico,
    CardFamily::Cinetico, CardFamily::Criptografico,
};

// Camada sintetica: so o family_mults conta pro calculo. Id != None pra entrar no produto
// (o catalogo descarta None), espelhando o helper Env(...) do teste C#.
EnvironmentModifier env(CardFamily family, float mult) {
    EnvironmentModifier e{};
    e.id = EnvironmentId::Lamacento;
    e.layer = EnvironmentLayer::Terreno;
    e.family_mults = {{family, mult}};
    return e;
}

float mult(CardFamily family, std::vector<EnvironmentModifier> layers) {
    return EnvironmentCatalog::mult_ambiente(family, layers);
}

}  // namespace

// ---- Conjunto vazio / so-None => 1.0 (retrocompat secao 11) -----------------------

TEST_CASE("environment_stacking: conjunto vazio da 1.0", "[domain][combat][environment]") {
    REQUIRE(EnvironmentCatalog::mult_ambiente(CardFamily::Eletrico, {}) == Approx(1.0f).margin(1e-4));
}

TEST_CASE("environment_stacking: so None da 1.0", "[domain][combat][environment]") {
    REQUIRE(EnvironmentCatalog::mult_ambiente(CardFamily::Eletrico,
                                              {EnvironmentCatalog::none()}) == Approx(1.0f).margin(1e-4));
}

TEST_CASE("environment_stacking: None no meio e ignorado", "[domain][combat][environment]") {
    const float r = EnvironmentCatalog::mult_ambiente(CardFamily::Eletrico, {
        EnvironmentCatalog::none(),
        EnvironmentCatalog::get(EnvironmentId::Chuva),  // Eletrico x1.5
        EnvironmentCatalog::none(),
    });
    REQUIRE(r == Approx(1.5f).margin(1e-4));
}

// ---- Stacking: camadas que afetam a mesma familia MULTIPLICAM ---------------------

TEST_CASE("environment_stacking: duas camadas 1.3 x 1.3 multiplicam para 1.69",
          "[domain][combat][environment]") {
    const float r = mult(CardFamily::Sonico,
                         {env(CardFamily::Sonico, 1.3f), env(CardFamily::Sonico, 1.3f)});
    REQUIRE(r == Approx(1.69f).margin(1e-4));
}

TEST_CASE("environment_stacking: tres camadas que nao afetam a familia dao 1.0",
          "[domain][combat][environment]") {
    const float r = mult(CardFamily::Eletrico, {
        env(CardFamily::Sonico, 1.3f),
        env(CardFamily::Cinetico, 0.66f),
        env(CardFamily::Bioquimico, 1.5f),
    });
    REQUIRE(r == Approx(1.0f).margin(1e-4));
}

TEST_CASE("environment_stacking: camada up e down se cancelam parcialmente",
          "[domain][combat][environment]") {
    // 1.5 x 0.66 = 0.99.
    const float r = mult(CardFamily::Eletrico,
                         {env(CardFamily::Eletrico, 1.5f), env(CardFamily::Eletrico, 0.66f)});
    REQUIRE(r == Approx(0.99f).margin(1e-4));
}

// ---- CAP teto 2.25 - boundary + off-by-one ----------------------------------------

TEST_CASE("environment_stacking: cap teto exato 1.5 x 1.5 da 2.25",
          "[domain][combat][environment]") {
    const float r = mult(CardFamily::Eletrico,
                         {env(CardFamily::Eletrico, 1.5f), env(CardFamily::Eletrico, 1.5f)});
    REQUIRE(r == Approx(2.25f).margin(1e-4));
}

TEST_CASE("environment_stacking: cap teto estoura e clampa em 2.25",
          "[domain][combat][environment]") {
    // 1.5 x 1.6 = 2.40 => clampa 2.25.
    const float r = mult(CardFamily::Eletrico,
                         {env(CardFamily::Eletrico, 1.5f), env(CardFamily::Eletrico, 1.6f)});
    REQUIRE(r == Approx(2.25f).margin(1e-4));
}

TEST_CASE("environment_stacking: cap teto estoura com 3 camadas",
          "[domain][combat][environment]") {
    // 1.5 x 1.5 x 1.3 = 2.925 => clampa 2.25.
    const float r = mult(CardFamily::Eletrico, {
        env(CardFamily::Eletrico, 1.5f),
        env(CardFamily::Eletrico, 1.5f),
        env(CardFamily::Eletrico, 1.3f),
    });
    REQUIRE(r == Approx(2.25f).margin(1e-4));
}

TEST_CASE("environment_stacking: logo abaixo do teto nao clampa",
          "[domain][combat][environment]") {
    // 1.5 x 1.49 = 2.235 < 2.25 => intacto.
    const float r = mult(CardFamily::Eletrico,
                         {env(CardFamily::Eletrico, 1.5f), env(CardFamily::Eletrico, 1.49f)});
    REQUIRE(r == Approx(2.235f).margin(1e-4));
}

// ---- CAP piso 0.44 - boundary + off-by-one ----------------------------------------

TEST_CASE("environment_stacking: cap piso 0.66 x 0.66 clampa em 0.44",
          "[domain][combat][environment]") {
    // 0.66 x 0.66 = 0.4356 < 0.44 => clampa 0.44.
    const float r = mult(CardFamily::Eletrico,
                         {env(CardFamily::Eletrico, 0.66f), env(CardFamily::Eletrico, 0.66f)});
    REQUIRE(r == Approx(0.44f).margin(1e-4));
}

TEST_CASE("environment_stacking: cap piso estoura com 3 camadas",
          "[domain][combat][environment]") {
    // 0.66^3 = 0.2875 => clampa 0.44.
    const float r = mult(CardFamily::Eletrico, {
        env(CardFamily::Eletrico, 0.66f),
        env(CardFamily::Eletrico, 0.66f),
        env(CardFamily::Eletrico, 0.66f),
    });
    REQUIRE(r == Approx(0.44f).margin(1e-4));
}

TEST_CASE("environment_stacking: logo acima do piso nao clampa",
          "[domain][combat][environment]") {
    // 0.85 x 0.66 = 0.561 > 0.44 => intacto.
    const float r = mult(CardFamily::Eletrico,
                         {env(CardFamily::Eletrico, 0.85f), env(CardFamily::Eletrico, 0.66f)});
    REQUIRE(r == Approx(0.561f).margin(1e-4));
}

TEST_CASE("environment_stacking: valor exatamente no piso nao e alterado",
          "[domain][combat][environment]") {
    // 0.44 sozinho => fica 0.44 (boundary inclusivo).
    const float r = mult(CardFamily::Eletrico, {env(CardFamily::Eletrico, 0.44f)});
    REQUIRE(r == Approx(0.44f).margin(1e-4));
}

// ---- Stacking real do catalogo (3 camadas canonicas) ------------------------------

TEST_CASE("environment_stacking: stack canonico terreno+clima+periodo para eletrico",
          "[domain][combat][environment]") {
    // Lamacento (x1.3) + Chuva (x1.5) + Aurora (x1.3) = 2.535 => clampa 2.25.
    const float r = EnvironmentCatalog::mult_ambiente(CardFamily::Eletrico, {
        EnvironmentCatalog::get(EnvironmentId::Lamacento),
        EnvironmentCatalog::get(EnvironmentId::Chuva),
        EnvironmentCatalog::get(EnvironmentId::Aurora),
    });
    REQUIRE(r == Approx(2.25f).margin(1e-4));
}

TEST_CASE("environment_stacking: resultado sempre dentro do cap para qualquer par do catalogo",
          "[domain][combat][environment]") {
    std::vector<EnvironmentId> ids;
    for (const auto& [id, env] : EnvironmentCatalog::all()) ids.push_back(id);

    for (const auto a : ids)
        for (const auto b : ids)
            for (const auto f : kAllFamilies) {
                const float r = EnvironmentCatalog::mult_ambiente(f, {
                    EnvironmentCatalog::get(a),
                    EnvironmentCatalog::get(b),
                });
                REQUIRE(r >= 0.44f);
                REQUIRE(r <= 2.25f);
            }
}

// ---- Sobrecarga por id espelha a sobrecarga por modifier --------------------------

TEST_CASE("environment_stacking: sobrecarga por id le do catalogo",
          "[domain][combat][environment]") {
    const std::vector<EnvironmentId> ids = {EnvironmentId::Lamacento, EnvironmentId::Chuva};
    const float by_id = EnvironmentCatalog::mult_ambiente_ids(CardFamily::Eletrico, ids);
    const float by_mod = EnvironmentCatalog::mult_ambiente(CardFamily::Eletrico, {
        EnvironmentCatalog::get(EnvironmentId::Lamacento),
        EnvironmentCatalog::get(EnvironmentId::Chuva),
    });
    REQUIRE(by_id == Approx(by_mod).margin(1e-4));
}

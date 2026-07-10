// GusEngine/tests/core_smoke_test.cpp
// Teste dummy do M0. Prova que o framework de teste (Catch2) compila, linka
// contra as libs puras (core/ + domain/) e roda via ctest, headless (sem Qt).
//
// Exercita funcoes/constantes reais de core/ e domain/ (nao asserts vazios):
// clamp, a versao da engine e o schema de save. Em M3 esta pasta recebe a spec
// executavel portada dos ~390 testes C#.

#include <catch2/catch_test_macros.hpp>

#include "gus/core/math_util.hpp"
#include "gus/core/version.hpp"
#include "gus/domain/domain_info.hpp"

TEST_CASE("core::clamp restringe ao intervalo fechado", "[core][math]") {
    using gus::core::clamp;

    SECTION("valor abaixo do minimo vira o minimo") {
        REQUIRE(clamp(-5, 0, 10) == 0);
    }
    SECTION("valor acima do maximo vira o maximo") {
        REQUIRE(clamp(42, 0, 10) == 10);
    }
    SECTION("valor dentro do intervalo nao muda") {
        REQUIRE(clamp(7, 0, 10) == 7);
    }
    SECTION("bordas sao inclusivas") {
        REQUIRE(clamp(0, 0, 10) == 0);
        REQUIRE(clamp(10, 0, 10) == 10);
    }
}

TEST_CASE("core::engine_version expoe a versao semver", "[core][version]") {
    REQUIRE(gus::core::engine_version() == "0.1.0");
    REQUIRE(gus::core::kEngineVersionMajor == 0);
    REQUIRE(gus::core::kEngineVersionMinor == 1);
    REQUIRE(gus::core::kEngineVersionPatch == 0);
}

TEST_CASE("domain expoe identidade e schema de save", "[domain]") {
    REQUIRE(gus::domain::domain_label() == "gusengine-domain");
    // Schema forward-only: V5 (MODOS-MORTE Fase 0: +difficulty
    // +difficult_recovery_stage, por cima do V4 do ADR-007: +input_remap_backup
    // +controls_hash128 +slot_id).
    REQUIRE(gus::domain::kSaveSchemaVersion == 5);
}

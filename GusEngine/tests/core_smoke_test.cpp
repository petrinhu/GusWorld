// SPDX-License-Identifier: Apache-2.0
// GusEngine/tests/core_smoke_test.cpp
// Teste dummy do M0. Prova que o framework de teste (Catch2) compila, linka
// contra as libs puras (core/ + domain/) e roda via ctest, headless (sem Qt).
//
// Exercita funcoes/constantes reais de core/ e domain/ (nao asserts vazios):
// clamp, a versao da engine e o schema de save. Em M3 esta pasta recebe a spec
// executavel portada dos ~390 testes C#.

#include <catch2/catch_test_macros.hpp>

#include <regex>

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

TEST_CASE("core::engine_version expoe o esquema 4-componentes + build-metadata", "[core][version]") {
    // Esquema major.minor.patch.tweak (ver comentario grande em
    // GusEngine/CMakeLists.txt) + sufixo +sha[.dirty] opcional (SemVer 2.0.0
    // secao 10, capturado em tempo de CONFIGURE). O SHA/dirty NAO e testavel
    // por valor exato (muda a cada commit) -- so o FORMATO e uma invariante
    // estavel, entao o teste verifica a forma via regex, nao a string
    // literal completa.
    static const std::regex kVersionFormat(
        R"(^\d+\.\d+\.\d+\.\d+(\+[0-9a-f]+(\.dirty)?)?$)");
    REQUIRE(std::regex_match(std::string(gus::core::engine_version()), kVersionFormat));

    // Os componentes numericos continuam testaveis por valor exato: sao
    // constantes de compilacao, nao mudam a cada commit como o sha.
    REQUIRE(gus::core::kEngineVersionMajor == 0);
    REQUIRE(gus::core::kEngineVersionMinor == 3);
    REQUIRE(gus::core::kEngineVersionPatch == 86);
    REQUIRE(gus::core::kEngineVersionTweak == 0);

    // A string completa comeca com "major.minor.patch.tweak" literal (os 4
    // componentes numericos exatos), seguido ou nao do sufixo +sha[.dirty].
    static const std::string kExpectedPrefix = "0.3.86.0";
    const std::string version(gus::core::engine_version());
    const bool starts_with_expected_prefix = version.compare(0, kExpectedPrefix.size(), kExpectedPrefix) == 0;
    REQUIRE(starts_with_expected_prefix);
}

TEST_CASE("domain expoe identidade e schema de save", "[domain]") {
    REQUIRE(gus::domain::domain_label() == "gusengine-domain");
    // Schema forward-only: V7 (CARDS-HW-1: +CardPhysicalState por CardInstance,
    // por cima do V6/DECK-4: +CardCollectionState +credits +hand_selection).
    REQUIRE(gus::domain::kSaveSchemaVersion == 7);
}

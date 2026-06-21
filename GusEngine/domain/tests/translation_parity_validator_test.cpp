// translation_parity_validator_test.cpp
//
// Spec executavel (Catch2 v3) do validador de paridade estrutural i18n, portada
// de engine/tests/localization/TranslationParityValidatorTests.cs (xUnit). O
// xUnit e a SPEC do comportamento canonico (F2-S.12a); preservado 1:1.
//
// Subsistema: domain/i18n (engine-design.md secao 2/3, marco M3). POCO puro,
// ZERO Qt. Garante que os catalogos (pt_br/en_intl) NAO cresçam
// dessincronizados. Tres invariantes:
//   1. Paridade     - conjunto de chaves de source (pt_br) == target (en_intl).
//   2. Sem duplicata - nenhuma chave repetida no MESMO locale (o last-wins do
//                      parser e silencioso, so o conteudo cru revela).
//   3. Source nao-vazia - pt_br (lingua-fonte) NAO pode ter valor vazio; en_intl
//                      PODE (fallback empty-value aceito em G1).

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include "gus/domain/i18n/translation_parity_validator.hpp"

using gus::domain::i18n::find_duplicate_keys;
using gus::domain::i18n::TranslationParityReport;
using gus::domain::i18n::validate;
using Locale = std::map<std::string, std::string>;

namespace {

bool contains(const std::vector<std::string>& v, const std::string& s) {
    return std::find(v.begin(), v.end(), s) != v.end();
}

}  // namespace

// ---- Paridade -------------------------------------------------------------

TEST_CASE("paridade: conjuntos de chave identicos sem problema",
          "[domain][i18n][parity]") {
    const Locale source{{"MENU_START", "Iniciar"}, {"MENU_QUIT", "Sair"}};
    const Locale target{{"MENU_START", ""}, {"MENU_QUIT", ""}};

    const auto report = validate(source, target);

    REQUIRE(report.is_valid());
    REQUIRE(report.missing_in_target.empty());
    REQUIRE(report.missing_in_source.empty());
}

TEST_CASE("paridade: chave faltando no target e reportada",
          "[domain][i18n][parity]") {
    // Dev adicionou chave no source (pt_br) e esqueceu no target (en_intl).
    const Locale source{{"MENU_START", "Iniciar"}, {"MENU_NEW", "Novo"}};
    const Locale target{{"MENU_START", ""}};

    const auto report = validate(source, target);

    REQUIRE_FALSE(report.is_valid());
    REQUIRE(contains(report.missing_in_target, "MENU_NEW"));
    REQUIRE(report.missing_in_source.empty());
}

TEST_CASE("paridade: chave orfa no target (faltando no source) e reportada",
          "[domain][i18n][parity]") {
    // Chave vive so no target: orfa, nunca podera fazer fallback pro source.
    const Locale source{{"MENU_START", "Iniciar"}};
    const Locale target{{"MENU_START", ""}, {"GHOST_KEY", ""}};

    const auto report = validate(source, target);

    REQUIRE_FALSE(report.is_valid());
    REQUIRE(contains(report.missing_in_source, "GHOST_KEY"));
    REQUIRE(report.missing_in_target.empty());
}

TEST_CASE("paridade: divergencia nas duas direcoes reporta ambas",
          "[domain][i18n][parity]") {
    const Locale source{{"A", "a"}, {"ONLY_SOURCE", "x"}};
    const Locale target{{"A", ""}, {"ONLY_TARGET", ""}};

    const auto report = validate(source, target);

    REQUIRE_FALSE(report.is_valid());
    REQUIRE(contains(report.missing_in_target, "ONLY_SOURCE"));
    REQUIRE(contains(report.missing_in_source, "ONLY_TARGET"));
}

TEST_CASE("paridade: chaves faltando vem ordenadas deterministicamente",
          "[domain][i18n][parity]") {
    // Ordem estavel facilita diff de CI e leitura humana.
    const Locale source{{"ZEBRA", "z"}, {"ALPHA", "a"}, {"MIKE", "m"}};
    const Locale target{{"KEEP", ""}};

    const auto report = validate(source, target);

    const std::vector<std::string> expected{"ALPHA", "MIKE", "ZEBRA"};
    REQUIRE(report.missing_in_target == expected);
}

// ---- Source vazia (regra assimetrica) -------------------------------------

TEST_CASE("source vazia: valor vazio no target e permitido",
          "[domain][i18n][parity]") {
    // Canon: en_intl e fallback pos-v1.0.0; valores vazios esperados em G1.
    const Locale source{{"MENU_START", "Iniciar"}};
    const Locale target{{"MENU_START", ""}};

    const auto report = validate(source, target);

    REQUIRE(report.is_valid());
    REQUIRE(report.empty_in_source.empty());
}

TEST_CASE("source vazia: valor vazio no source e reportado",
          "[domain][i18n][parity]") {
    // pt_br e a lingua-fonte: valor vazio = bug (nada para exibir, nada para
    // fazer fallback).
    const Locale source{{"MENU_START", "Iniciar"}, {"BROKEN", ""}};
    const Locale target{{"MENU_START", ""}, {"BROKEN", ""}};

    const auto report = validate(source, target);

    REQUIRE_FALSE(report.is_valid());
    REQUIRE(contains(report.empty_in_source, "BROKEN"));
}

TEST_CASE("source vazia: valor so com espacos conta como vazio",
          "[domain][i18n][parity]") {
    // Valor so com espacos/tabs no source nao tem conteudo util.
    const Locale source{{"MENU_START", "Iniciar"}, {"BLANK", "   \t  "}};
    const Locale target{{"MENU_START", ""}, {"BLANK", ""}};

    const auto report = validate(source, target);

    REQUIRE_FALSE(report.is_valid());
    REQUIRE(contains(report.empty_in_source, "BLANK"));
}

TEST_CASE("source vazia: valores vazios vem ordenados deterministicamente",
          "[domain][i18n][parity]") {
    const Locale source{{"ZED", ""}, {"ABE", ""}, {"OK", "tem valor"}};
    const Locale target{{"ZED", ""}, {"ABE", ""}, {"OK", ""}};

    const auto report = validate(source, target);

    const std::vector<std::string> expected{"ABE", "ZED"};
    REQUIRE(report.empty_in_source == expected);
}

// ---- Duplicatas (sobre conteudo cru, pre-parse) ---------------------------

TEST_CASE("duplicatas: sem repeticao retorna vazio", "[domain][i18n][parity]") {
    const auto dups = find_duplicate_keys("## A\nalpha\n\n## B\nbeta\n\n## C\ngamma");

    REQUIRE(dups.empty());
}

TEST_CASE("duplicatas: chave repetida e detectada", "[domain][i18n][parity]") {
    // O parser faz last-wins SILENCIOSO; so varrendo o conteudo cru pega.
    const auto dups = find_duplicate_keys("## DUP\nprimeiro\n\n## OTHER\nx\n\n## DUP\nsegundo");

    REQUIRE(dups.size() == 1);
    REQUIRE(contains(dups, "DUP"));
}

TEST_CASE("duplicatas: tripla repeticao reportada uma vez", "[domain][i18n][parity]") {
    const auto dups = find_duplicate_keys("## TRIP\n1\n\n## TRIP\n2\n\n## TRIP\n3");

    REQUIRE(dups.size() == 1);
    REQUIRE(dups.front() == "TRIP");
}

TEST_CASE("duplicatas: headers de secao nao contam como chave",
          "[domain][i18n][parity]") {
    // "## §1. Menu" repetido NAO e duplicata de chave (nao e UPPER_SNAKE).
    const auto dups = find_duplicate_keys("## §1. Menu\n\n## A\nx\n\n## §1. Menu\n\n## B\ny");

    REQUIRE(dups.empty());
}

TEST_CASE("duplicatas: varias distintas reportadas ordenadas",
          "[domain][i18n][parity]") {
    const auto dups = find_duplicate_keys("## ZED\n1\n\n## ABE\n1\n\n## ZED\n2\n\n## ABE\n2");

    const std::vector<std::string> expected{"ABE", "ZED"};
    REQUIRE(dups == expected);
}

// ---- Report agregado / mensagem -------------------------------------------

TEST_CASE("report: valido tem sumario sem nome de categoria",
          "[domain][i18n][parity]") {
    const Locale source{{"A", "a"}};
    const Locale target{{"A", ""}};

    const auto report = validate(source, target);

    REQUIRE(report.is_valid());
    // Sumario de relatorio valido nao deve listar problema algum.
    REQUIRE(report.to_summary().find("missing_in_target") == std::string::npos);
    REQUIRE(report.to_summary().find("MissingInTarget") == std::string::npos);
}

TEST_CASE("report: invalido nomeia as chaves problematicas",
          "[domain][i18n][parity]") {
    const Locale source{{"A", "a"}, {"ONLY_SOURCE", "x"}};
    const Locale target{{"A", ""}};

    const auto report = validate(source, target);

    REQUIRE(report.to_summary().find("ONLY_SOURCE") != std::string::npos);
}

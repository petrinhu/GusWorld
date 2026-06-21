// md_translation_loader_test.cpp
//
// Spec executavel (Catch2 v3) do parser de catalogo .md de traducoes, portada
// de engine/tests/localization/MdTranslationLoaderTests.cs (xUnit). O xUnit e a
// SPEC do comportamento canonico; aqui o comportamento e preservado 1:1.
//
// Subsistema portado (engine-design.md secao 2/3, marco M3): domain/i18n.
// POCO puro, ZERO Qt, headless. O I/O de arquivo (Godot.FileAccess no C#) NAO
// e portado aqui: o parser recebe o CONTEUDO como string; o I/O fica em
// platform/fs num marco futuro. Logo, so o equivalente a Parse e exercitado.
//
// Comportamento coberto:
//   - parse de "## KEY" UPPER_SNAKE seguido do valor nas linhas abaixo
//   - headers de secao "## SECAO N" (com simbolos/minusculas) NAO viram chave
//   - multiplas chaves, valor multi-linha, trim das pontas
//   - chave duplicada = last-wins
//   - chave sem corpo = valor vazio (nao some do mapa)
//   - placeholders {0}/{1} preservados como texto literal (parser nao interpola)

#include <catch2/catch_test_macros.hpp>

#include <map>
#include <string>

#include "gus/domain/i18n/md_translation_loader.hpp"

using gus::domain::i18n::parse;

TEST_CASE("i18n::parse mapeia chave basica para valor", "[domain][i18n][loader]") {
    const auto result = parse("## MENU_START\nIniciar jogo");

    REQUIRE(result.size() == 1);
    REQUIRE(result.at("MENU_START") == "Iniciar jogo");
}

TEST_CASE("i18n::parse mapeia multiplas chaves", "[domain][i18n][loader]") {
    const auto result = parse("## A\nalpha\n\n## B\nbeta\n\n## C\ngamma");

    REQUIRE(result.size() == 3);
    REQUIRE(result.at("A") == "alpha");
    REQUIRE(result.at("B") == "beta");
    REQUIRE(result.at("C") == "gamma");
}

TEST_CASE("i18n::parse junta valor multi-linha com newline", "[domain][i18n][loader]") {
    const auto result = parse("## STORY\nLinha um\nLinha dois\nLinha tres");

    REQUIRE(result.at("STORY") == "Linha um\nLinha dois\nLinha tres");
}

TEST_CASE("i18n::parse apara linhas em branco em volta do valor", "[domain][i18n][loader]") {
    // Linhas em branco antes/depois do valor sao removidas (trim das pontas).
    const auto result = parse("## KEY\n\n   valor com espacos   \n\n");

    REQUIRE(result.at("KEY") == "valor com espacos");
}

TEST_CASE("i18n::parse filtra headers H1 e H3 dentro do valor", "[domain][i18n][loader]") {
    // "# X" e "### X" dentro de um valor sao filtrados (nao entram no texto).
    const auto result = parse("## KEY\nantes\n# titulo h1\nmeio\n### subtitulo h3\ndepois");

    REQUIRE(result.at("KEY") == "antes\nmeio\ndepois");
}

TEST_CASE("i18n::parse: header de secao nao vira chave", "[domain][i18n][loader]") {
    // PROVA do fix F2-S.11a: "## §1. Menu principal" NAO e chave (tem simbolo,
    // espaco, ponto, minusculas), some do mapa; a chave real seguinte e capturada.
    const auto result = parse("## §1. Menu principal\n\n## MENU_START_GAME\nIniciar jogo");

    REQUIRE(result.size() == 1);
    REQUIRE_FALSE(result.contains("§1. Menu principal"));
    REQUIRE(result.at("MENU_START_GAME") == "Iniciar jogo");
}

TEST_CASE("i18n::parse: header de secao entre chaves nao polui valores",
          "[domain][i18n][loader]") {
    // Header de secao entre duas chaves nao vira chave NEM entra no valor da
    // chave anterior.
    const auto result =
        parse("## FIRST\nvalor um\n\n## §2. Settings\n\n## SECOND\nvalor dois");

    REQUIRE(result.size() == 2);
    REQUIRE(result.at("FIRST") == "valor um");
    REQUIRE(result.at("SECOND") == "valor dois");
    for (const auto& [k, v] : result) {
        REQUIRE(k.find("Settings") == std::string::npos);
    }
}

TEST_CASE("i18n::parse: header nao UPPER_SNAKE nao vira chave", "[domain][i18n][loader]") {
    // Espelha o Theory parametrizado do xUnit: cada candidato invalido resulta
    // em mapa vazio.
    auto empty_for = [](const std::string& md) { return parse(md).empty(); };

    REQUIRE(empty_for("## key_lowercase\nx"));   // minusculas
    REQUIRE(empty_for("## Mixed_Case\nx"));       // case misto
    REQUIRE(empty_for("## HAS SPACE\nx"));        // espaco
    REQUIRE(empty_for("## HAS.DOT\nx"));          // ponto
    REQUIRE(empty_for("## §SECTION\nx"));         // comeca com simbolo
    REQUIRE(empty_for("## 1NUMERIC_START\nx"));   // comeca com digito
}

TEST_CASE("i18n::parse: header UPPER_SNAKE valido vira chave", "[domain][i18n][loader]") {
    auto single_value = [](const std::string& header) {
        const auto r = parse(header + "\nalgum valor");
        REQUIRE(r.size() == 1);
        return r.begin()->second;
    };

    REQUIRE(single_value("## A") == "algum valor");                // letra unica
    REQUIRE(single_value("## MENU_START_GAME") == "algum valor");  // upper snake tipico
    REQUIRE(single_value("## ERROR_404") == "algum valor");        // digitos depois de letra
    REQUIRE(single_value("## X_Y_Z") == "algum valor");            // underscores multiplos
}

TEST_CASE("i18n::parse: chave duplicada = last-wins", "[domain][i18n][loader]") {
    const auto result = parse("## DUP\nprimeiro\n\n## DUP\nsegundo");

    REQUIRE(result.size() == 1);
    REQUIRE(result.at("DUP") == "segundo");
}

TEST_CASE("i18n::parse: chave sem corpo mapeia para string vazia",
          "[domain][i18n][loader]") {
    // Chave sem valor (paridade estrutural en_intl em G1) -> valor vazio, nao
    // some do mapa.
    const auto result = parse("## EMPTY_KEY\n\n## NEXT_KEY\ntem valor");

    REQUIRE(result.size() == 2);
    REQUIRE(result.at("EMPTY_KEY").empty());
    REQUIRE(result.at("NEXT_KEY") == "tem valor");
}

TEST_CASE("i18n::parse: ultima chave sem corpo mapeia para string vazia",
          "[domain][i18n][loader]") {
    const auto result = parse("## A\nvalor a\n\n## TRAILING_EMPTY\n");

    REQUIRE(result.size() == 2);
    REQUIRE(result.at("TRAILING_EMPTY").empty());
}

TEST_CASE("i18n::parse: placeholders de interpolacao ficam literais",
          "[domain][i18n][loader]") {
    // O parser e puro: NAO interpola {0}/{1}. Os placeholders sobrevivem como
    // texto literal no valor (a interpolacao e responsabilidade de quem exibe).
    const auto result = parse("## GREETING\nOla {0}, voce tem {1} itens");

    REQUIRE(result.at("GREETING") == "Ola {0}, voce tem {1} itens");
}

TEST_CASE("i18n::parse: conteudo vazio retorna mapa vazio", "[domain][i18n][loader]") {
    REQUIRE(parse("").empty());
}

TEST_CASE("i18n::parse: so headers sem chaves retorna mapa vazio",
          "[domain][i18n][loader]") {
    const auto result =
        parse("# Titulo do arquivo\n\n## §1. Secao\n\n## §2. Outra secao\n\n> citacao");

    REQUIRE(result.empty());
}

TEST_CASE("i18n::parse: catalogo simulado 65 chaves + 7 headers retorna 65",
          "[domain][i18n][loader]") {
    // Prova integrada F2-S.11a: formato do pt_br.md real (chaves UPPER_SNAKE +
    // headers "## §N. ...") -> exatamente 65 chaves, zero chave-fantasma.
    std::string md = "# pt_br.md\n\n";
    int total_keys = 0;
    for (int section = 1; section <= 7; ++section) {
        md += "## §" + std::to_string(section) +
              ". Secao exemplo com espacos e ponto\n\n";
        const int keys_in_section = section <= 2 ? 10 : 9;  // 10+10+9*5 = 65
        for (int k = 0; k < keys_in_section && total_keys < 65; ++k) {
            ++total_keys;
            md += "## KEY_S" + std::to_string(section) + "_N" + std::to_string(k) +
                  "\nvalor " + std::to_string(total_keys) + "\n\n";
        }
    }
    REQUIRE(total_keys == 65);  // sanity do fixture

    const auto result = parse(md);

    REQUIRE(result.size() == 65);
    for (const auto& [k, v] : result) {
        REQUIRE(k.find(static_cast<char>(0xC2)) == std::string::npos);  // sem byte de §
    }
}

TEST_CASE("i18n::parse: ignora CR de finais de linha CRLF", "[domain][i18n][loader]") {
    // Robustez extra (alem do xUnit): catalogo salvo com CRLF nao deve deixar
    // \r preso no fim do valor nem corromper o casamento da chave.
    const auto result = parse("## KEY\r\nvalor\r\n");

    REQUIRE(result.size() == 1);
    REQUIRE(result.at("KEY") == "valor");
}

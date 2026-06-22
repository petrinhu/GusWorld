// controls_name_sanitize_test.cpp
//
// Spec executavel (Catch2 v3) do sanitize de nome de perfil de jogador (ADR-007,
// decisao do lider: multi-perfil, arquivo "[player]_controls.json"). FUNCAO PURA,
// headless: o headless forma o nome logico; o I/O real (escrever em disco, path,
// permissoes) e platform.
//
// Regra (decisao do lider):
//   - minusculas;
//   - espacos e caracteres problematicos de filesystem ( / \ : * ? " < > | e
//     espaco ) -> '_';
//   - exemplo: "Jose Silva" -> "jose_silva"; arquivo -> "jose_silva_controls.json".
//
// Contrato:
//   - sanitize_profile_name(name) -> nome de perfil saneado;
//   - controls_file_name(name) -> "<saneado>_controls.json";
//   - nome vazio/so-invalidos -> perfil "default" (fallback, multi-perfil precisa
//     de um nome sempre valido).
//
// Cross-ref: docs/tech/adr/ADR-007-controls-json-hash128-save-v4.md (fork 3 -> b),
//            gus/domain/input/controls_name.hpp.

#include <catch2/catch_test_macros.hpp>

#include <string>

#include "gus/domain/input/controls_name.hpp"

using gus::domain::input::controls_file_name;
using gus::domain::input::sanitize_profile_name;

// ---- exemplo canonico do lider ---------------------------------------------

TEST_CASE("name_sanitize: 'Jose Silva' -> 'jose_silva'",
          "[domain][input][controls_name]") {
    REQUIRE(sanitize_profile_name("Jose Silva") == "jose_silva");
}

TEST_CASE("name_sanitize: arquivo de 'Jose Silva' = 'jose_silva_controls.json'",
          "[domain][input][controls_name]") {
    REQUIRE(controls_file_name("Jose Silva") == "jose_silva_controls.json");
}

// ---- minusculas ------------------------------------------------------------

TEST_CASE("name_sanitize: caixa alta vira minuscula",
          "[domain][input][controls_name]") {
    REQUIRE(sanitize_profile_name("TESTER") == "tester");
}

// ---- caracteres problematicos de filesystem -> '_' -------------------------

TEST_CASE("name_sanitize: caracteres problematicos de FS viram '_'",
          "[domain][input][controls_name]") {
    // / \ : * ? " < > | e espaco
    REQUIRE(sanitize_profile_name("a/b\\c:d*e?f\"g<h>i|j k") ==
            "a_b_c_d_e_f_g_h_i_j_k");
}

// ---- nome ja valido fica intacto (alfa-num + underscore + minusculo) -------

TEST_CASE("name_sanitize: nome ja valido permanece",
          "[domain][input][controls_name]") {
    REQUIRE(sanitize_profile_name("iago_2") == "iago_2");
}

// ---- determinismo ----------------------------------------------------------

TEST_CASE("name_sanitize: deterministico",
          "[domain][input][controls_name]") {
    REQUIRE(sanitize_profile_name("Player One") ==
            sanitize_profile_name("Player One"));
}

// ---- fallback "default" para nome vazio ou so-invalido ---------------------

TEST_CASE("name_sanitize: nome vazio cai no perfil 'default'",
          "[domain][input][controls_name]") {
    REQUIRE(sanitize_profile_name("") == "default");
    REQUIRE(controls_file_name("") == "default_controls.json");
}

TEST_CASE("name_sanitize: nome so com caracteres invalidos cai em 'default'",
          "[domain][input][controls_name]") {
    // "///" sanearia para "___"; a regra de fallback evita um perfil so de '_'.
    REQUIRE(sanitize_profile_name("///") == "default");
}

// ---- acentos: minimo viavel (nao quebra; vira underscore se nao-ASCII) ------

TEST_CASE("name_sanitize: nao gera nome vazio nunca",
          "[domain][input][controls_name]") {
    REQUIRE_FALSE(sanitize_profile_name("Joao").empty());
    REQUIRE_FALSE(controls_file_name("Joao").empty());
}

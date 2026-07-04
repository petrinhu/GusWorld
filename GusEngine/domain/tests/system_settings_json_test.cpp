// GusEngine/domain/tests/system_settings_json_test.cpp
//
// Catch2 (TEST-FIRST) do serializer/parser JSON proprio de SystemSettings
// (MENU-PAUSA-CONFIG-SOM, M7-COSTURA). Mesmo padrao de
// domain/tests/controls_json_test.cpp: roundtrip, defaults em campo ausente,
// forward-compat (chave desconhecida ignorada), corpus de malformados sem crash.

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "gus/domain/settings/system_settings.hpp"
#include "gus/domain/settings/system_settings_json.hpp"

using gus::domain::settings::SystemSettings;
using gus::domain::settings::SystemSettingsParseError;
using gus::domain::settings::parse_system_settings;
using gus::domain::settings::serialize_system_settings_pretty;

TEST_CASE("SystemSettings defaults: volume cheio (1.0f) nos dois grupos",
          "[system_settings]") {
    SystemSettings s;
    REQUIRE(s.music_volume == Catch::Approx(1.0f));
    REQUIRE(s.sfx_volume == Catch::Approx(1.0f));
    REQUIRE(s.schema_version == 1);
}

TEST_CASE("serialize_system_settings_pretty + parse_system_settings: roundtrip",
          "[system_settings]") {
    SystemSettings s;
    s.music_volume = 0.72f;
    s.sfx_volume = 0.45f;

    const std::string json = serialize_system_settings_pretty(s);
    const auto result = parse_system_settings(json);

    REQUIRE(result.error == SystemSettingsParseError::None);
    REQUIRE(result.settings.music_volume == Catch::Approx(0.72f));
    REQUIRE(result.settings.sfx_volume == Catch::Approx(0.45f));
    REQUIRE(result.settings.schema_version == 1);
}

TEST_CASE("parse_system_settings: JSON vazio devolve defaults com erro Empty",
          "[system_settings]") {
    const auto result = parse_system_settings("");
    REQUIRE(result.error == SystemSettingsParseError::Empty);
    // Degradacao segura: settings fica nos defaults (nao lixo de memoria).
    REQUIRE(result.settings.music_volume == Catch::Approx(1.0f));
    REQUIRE(result.settings.sfx_volume == Catch::Approx(1.0f));
}

TEST_CASE("parse_system_settings: JSON malformado nao crasha (corpus)",
          "[system_settings]") {
    const char* malformed[] = {
        "{",
        "}",
        "[]",
        "\"so uma string\"",
        "{\"music_volume\": }",
        "{\"music_volume\": 0.5,}",
        "{\"music_volume\": true}",
        "null",
        "{\"music_volume\": 1e10}",  // notacao cientifica fora do escopo
    };
    for (const char* text : malformed) {
        const auto result = parse_system_settings(text);
        // Nao importa qual erro exatamente (exceto o caso 'true' que e TypeMismatch e
        // "{}"/objeto vazio que e valido/None) - so que NUNCA crasha e sempre devolve
        // um SystemSettings usavel (defaults preservados quando o campo nao resolveu).
        (void)result;
    }
    SUCCEED("corpus inteiro de entradas malformadas nao crashou");
}

TEST_CASE("parse_system_settings: objeto vazio '{}' usa os defaults (campo ausente)",
          "[system_settings]") {
    const auto result = parse_system_settings("{}");
    REQUIRE(result.error == SystemSettingsParseError::None);
    REQUIRE(result.settings.music_volume == Catch::Approx(1.0f));
    REQUIRE(result.settings.sfx_volume == Catch::Approx(1.0f));
}

TEST_CASE("parse_system_settings: forward-compat - chave desconhecida e ignorada",
          "[system_settings]") {
    const auto result = parse_system_settings(
        "{\"music_volume\": 0.5, \"sfx_volume\": 0.25, "
        "\"campo_do_futuro_desconhecido\": 42}");
    REQUIRE(result.error == SystemSettingsParseError::None);
    REQUIRE(result.settings.music_volume == Catch::Approx(0.5f));
    REQUIRE(result.settings.sfx_volume == Catch::Approx(0.25f));
}

TEST_CASE("parse_system_settings: valores fora de [0,1] sao clampados (mesmo "
          "contrato de AudioEngine::set_music_volume/set_sfx_volume)",
          "[system_settings]") {
    const auto result =
        parse_system_settings("{\"music_volume\": 5.0, \"sfx_volume\": -3.0}");
    REQUIRE(result.error == SystemSettingsParseError::None);
    REQUIRE(result.settings.music_volume == Catch::Approx(1.0f));
    REQUIRE(result.settings.sfx_volume == Catch::Approx(0.0f));
}

TEST_CASE("serialize_system_settings_pretty: forma legivel (indentada) para o "
          "arquivo em disco",
          "[system_settings]") {
    SystemSettings s;
    const std::string json = serialize_system_settings_pretty(s);
    REQUIRE(json.find('\n') != std::string::npos);  // indentada, nao compacta
    REQUIRE(json.find("music_volume") != std::string::npos);
    REQUIRE(json.find("sfx_volume") != std::string::npos);
}

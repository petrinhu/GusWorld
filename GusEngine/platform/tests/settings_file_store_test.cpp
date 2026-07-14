// GusEngine/platform/tests/settings_file_store_test.cpp
//
// Catch2 (TEST-FIRST) do SettingsFileStore (MENU-PAUSA-CONFIG-SOM, M7-COSTURA):
// I/O REAL em disco de ~/.gusworld/settings.json (perms 0700 dir / 0600 arquivo),
// reusando o serializer/parser JSON dep-free do domain (system_settings_json.hpp).
// Hermetico: cada teste usa um diretorio TEMPORARIO proprio (nunca o $HOME real do
// host que roda o teste) via o parametro dir_override das funcoes.

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>
#include <string>

#include "gus/platform/fs/settings_file_store.hpp"

using gus::domain::settings::SystemSettings;
using gus::platform::fs::load_system_settings;
using gus::platform::fs::save_system_settings;
using gus::platform::fs::settings_file_path;

namespace {

// Diretorio temporario UNICO por teste (evita colisao entre TEST_CASE e entre
// execucoes paralelas do ctest). Removido no fim de cada teste.
std::filesystem::path make_temp_dir(const char* suffix) {
    auto dir = std::filesystem::temp_directory_path() /
               (std::string("gusworld_settings_test_") + suffix);
    std::filesystem::remove_all(dir);
    return dir;
}

}  // namespace

TEST_CASE("load_system_settings: diretorio/arquivo AUSENTE devolve defaults "
          "(1a execucao do jogo - degradacao segura)",
          "[settings_file_store]") {
    const auto dir = make_temp_dir("ausente");
    REQUIRE_FALSE(std::filesystem::exists(dir));

    const SystemSettings s = load_system_settings(dir.string());
    REQUIRE(s.music_volume == Catch::Approx(1.0f));
    REQUIRE(s.sfx_volume == Catch::Approx(1.0f));

    std::filesystem::remove_all(dir);
}

TEST_CASE("save_system_settings + load_system_settings: roundtrip real em disco",
          "[settings_file_store]") {
    const auto dir = make_temp_dir("roundtrip");

    SystemSettings s;
    s.music_volume = 0.72f;
    s.sfx_volume = 0.45f;

    REQUIRE(save_system_settings(s, dir.string()));
    REQUIRE(std::filesystem::exists(settings_file_path(dir.string())));

    const SystemSettings loaded = load_system_settings(dir.string());
    REQUIRE(loaded.music_volume == Catch::Approx(0.72f));
    REQUIRE(loaded.sfx_volume == Catch::Approx(0.45f));

    std::filesystem::remove_all(dir);
}

TEST_CASE("save_system_settings: cria o diretorio com permissao 0700 e o "
          "arquivo com permissao 0600 (LGPD - dado do usuario so ele le/escreve)",
          "[settings_file_store]") {
    const auto dir = make_temp_dir("perms");
    SystemSettings s;

    REQUIRE(save_system_settings(s, dir.string()));

    const auto dir_perms = std::filesystem::status(dir).permissions();
    const auto file_perms =
        std::filesystem::status(settings_file_path(dir.string())).permissions();

    using std::filesystem::perms;
    // 0700 = rwx pro dono, NADA pro grupo/outros.
    REQUIRE((dir_perms & perms::owner_all) == perms::owner_all);
#ifndef _WIN32
    // std::filesystem::permissions no Windows NAO mapeia bits POSIX de
    // grupo/outros (o backend _wstat64 espelha os bits de owner em todos os 9
    // bits, exceto write quando o arquivo e read-only) - a privacidade real no
    // Windows vem das ACLs per-user de %APPDATA%, nao de bits 0700 POSIX.
    REQUIRE((dir_perms & perms::group_all) == perms::none);
    REQUIRE((dir_perms & perms::others_all) == perms::none);
#endif

    // 0600 = rw pro dono (sem exec), NADA pro grupo/outros.
    REQUIRE((file_perms & perms::owner_read) == perms::owner_read);
    REQUIRE((file_perms & perms::owner_write) == perms::owner_write);
#ifndef _WIN32
    // Mesmo motivo do bloco acima (grupo/outros POSIX nao existem no Windows).
    REQUIRE((file_perms & perms::group_all) == perms::none);
    REQUIRE((file_perms & perms::others_all) == perms::none);
#endif

    std::filesystem::remove_all(dir);
}

TEST_CASE("load_system_settings: arquivo CORROMPIDO (JSON malformado) degrada "
          "para defaults sem crashar",
          "[settings_file_store]") {
    const auto dir = make_temp_dir("corrompido");
    std::filesystem::create_directories(dir);
    {
        std::ofstream out(settings_file_path(dir.string()), std::ios::trunc);
        out << "{ isso nao e json valido !!!";
    }

    const SystemSettings s = load_system_settings(dir.string());
    REQUIRE(s.music_volume == Catch::Approx(1.0f));
    REQUIRE(s.sfx_volume == Catch::Approx(1.0f));

    std::filesystem::remove_all(dir);
}

TEST_CASE("save_system_settings: sobrescreve um arquivo existente (mudanca de "
          "volume persistida de novo)",
          "[settings_file_store]") {
    const auto dir = make_temp_dir("sobrescreve");
    SystemSettings s1;
    s1.music_volume = 0.3f;
    REQUIRE(save_system_settings(s1, dir.string()));

    SystemSettings s2;
    s2.music_volume = 0.9f;
    s2.sfx_volume = 0.1f;
    REQUIRE(save_system_settings(s2, dir.string()));

    const SystemSettings loaded = load_system_settings(dir.string());
    REQUIRE(loaded.music_volume == Catch::Approx(0.9f));
    REQUIRE(loaded.sfx_volume == Catch::Approx(0.1f));

    std::filesystem::remove_all(dir);
}

TEST_CASE("settings_file_path: nome do arquivo e settings.json dentro do dir dado",
          "[settings_file_store]") {
    const auto dir = make_temp_dir("path");
    const std::string path = settings_file_path(dir.string());
    // Comparacao path-EQUIVALENTE (nao string-crua): a producao junta com "/"
    // literal, mas fs::path normaliza o separador nativo (\ no Windows) na
    // igualdade - so a REPRESENTACAO em string difere entre plataformas, o
    // path continua semanticamente igual (Windows aceita "/" tambem).
    REQUIRE(std::filesystem::path(path) == dir / "settings.json");
}

TEST_CASE("resolve_settings_dir: sem override de env, resolve para $HOME/.gusworld",
          "[settings_file_store]") {
    const std::string dir = gus::platform::fs::resolve_settings_dir();
    // Nao assume qual e o $HOME do host que roda o teste - so que o sufixo
    // canonico esta presente (contrato da resolucao).
    REQUIRE(dir.size() >= std::string(".gusworld").size());
#ifdef _WIN32
    // Layout de producao do Windows: "%APPDATA%\gusworld" SEM ponto (decisao do
    // port, ver settings_file_store.cpp resolve_settings_dir). "gusworld" tambem
    // casa com ".gusworld" se o override GUSWORLD_HOME estiver setado no CI.
    REQUIRE(dir.find("gusworld") != std::string::npos);
#else
    REQUIRE(dir.find(".gusworld") != std::string::npos);
#endif
}

// GusEngine/platform/tests/controls_file_store_test.cpp
//
// Catch2 (TEST-FIRST) do ControlsFileStore (tela Controles/M2): I/O REAL em
// disco de "<perfil>_controls.json" (ADR-007), reusando o serializer/parser
// PRETTY do domain (controls_json.hpp) + o sanitize de perfil
// (controls_name.hpp). Hermetico: cada teste usa um diretorio TEMPORARIO
// proprio (nunca o $HOME real do host). MESMO padrao de
// settings_file_store_test.cpp.

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>
#include <string>

#include "gus/domain/input/controls_restore.hpp"
#include "gus/platform/fs/controls_file_store.hpp"

using gus::domain::input::default_controls;
using gus::domain::input::InputRemapConfig;
using gus::domain::input::KeyBinding;
using gus::platform::fs::controls_file_path;
using gus::platform::fs::load_controls;
using gus::platform::fs::save_controls;

namespace {

std::filesystem::path make_temp_dir(const char* suffix) {
    auto dir = std::filesystem::temp_directory_path() /
               (std::string("gusworld_controls_test_") + suffix);
    std::filesystem::remove_all(dir);
    return dir;
}

}  // namespace

TEST_CASE("load_controls: diretorio/arquivo AUSENTE devolve default_controls() "
          "(1a execucao/perfil novo - degradacao segura)",
          "[controls_file_store]") {
    const auto dir = make_temp_dir("ausente");
    REQUIRE_FALSE(std::filesystem::exists(dir));

    const InputRemapConfig cfg = load_controls(dir.string(), "default");
    REQUIRE(cfg == default_controls());

    std::filesystem::remove_all(dir);
}

TEST_CASE("save_controls + load_controls: roundtrip real em disco (config "
          "customizado sobrevive)",
          "[controls_file_store]") {
    const auto dir = make_temp_dir("roundtrip");

    InputRemapConfig cfg = default_controls();
    cfg.actions.front().keys = {KeyBinding{.keycode = 'K'}};

    REQUIRE(save_controls(cfg, dir.string(), "default"));
    REQUIRE(std::filesystem::exists(controls_file_path(dir.string(), "default")));

    const InputRemapConfig loaded = load_controls(dir.string(), "default");
    REQUIRE(loaded == cfg);

    std::filesystem::remove_all(dir);
}

TEST_CASE("controls_file_path: perfis DISTINTOS resolvem pra arquivos "
          "DISTINTOS (multi-perfil, ADR-007 fork 3)",
          "[controls_file_store]") {
    const auto dir = make_temp_dir("multiperfil");

    InputRemapConfig cfg_a = default_controls();
    cfg_a.actions.front().keys = {KeyBinding{.keycode = 'A'}};
    InputRemapConfig cfg_b = default_controls();
    cfg_b.actions.front().keys = {KeyBinding{.keycode = 'B'}};

    REQUIRE(save_controls(cfg_a, dir.string(), "tester"));
    REQUIRE(save_controls(cfg_b, dir.string(), "iago"));

    REQUIRE(load_controls(dir.string(), "tester") == cfg_a);
    REQUIRE(load_controls(dir.string(), "iago") == cfg_b);
    REQUIRE(controls_file_path(dir.string(), "tester") !=
            controls_file_path(dir.string(), "iago"));

    std::filesystem::remove_all(dir);
}

TEST_CASE("save_controls: cria o diretorio com permissao 0700 e o arquivo com "
          "permissao 0600 (LGPD)",
          "[controls_file_store]") {
    const auto dir = make_temp_dir("perms");
    REQUIRE(save_controls(default_controls(), dir.string(), "default"));

    const auto dir_perms = std::filesystem::status(dir).permissions();
    const auto file_perms =
        std::filesystem::status(controls_file_path(dir.string(), "default")).permissions();

    using std::filesystem::perms;
    REQUIRE((dir_perms & perms::owner_all) == perms::owner_all);
#ifndef _WIN32
    // std::filesystem::permissions no Windows NAO mapeia bits POSIX de
    // grupo/outros (o backend _wstat64 espelha os bits de owner em todos os 9
    // bits, exceto write quando o arquivo e read-only) - a privacidade real no
    // Windows vem das ACLs per-user de %APPDATA%, nao de bits 0700 POSIX.
    REQUIRE((dir_perms & perms::group_all) == perms::none);
    REQUIRE((dir_perms & perms::others_all) == perms::none);
#endif

    REQUIRE((file_perms & perms::owner_read) == perms::owner_read);
    REQUIRE((file_perms & perms::owner_write) == perms::owner_write);
#ifndef _WIN32
    // Mesmo motivo do bloco acima (grupo/outros POSIX nao existem no Windows).
    REQUIRE((file_perms & perms::group_all) == perms::none);
    REQUIRE((file_perms & perms::others_all) == perms::none);
#endif

    std::filesystem::remove_all(dir);
}

TEST_CASE("load_controls: arquivo CORROMPIDO (JSON malformado) degrada para "
          "default_controls() sem crashar",
          "[controls_file_store]") {
    const auto dir = make_temp_dir("corrompido");
    std::filesystem::create_directories(dir);
    {
        std::ofstream out(controls_file_path(dir.string(), "default"), std::ios::trunc);
        out << "{ isso nao e json valido !!!";
    }

    const InputRemapConfig cfg = load_controls(dir.string(), "default");
    REQUIRE(cfg == default_controls());

    std::filesystem::remove_all(dir);
}

TEST_CASE("save_controls: arquivo escrito e PRETTY/legivel (contem quebras de "
          "linha, nao e uma unica linha compacta)",
          "[controls_file_store]") {
    const auto dir = make_temp_dir("pretty");
    REQUIRE(save_controls(default_controls(), dir.string(), "default"));

    std::string content;
    {
        // Escopo proprio: fecha o ifstream (destrutor) ANTES do remove_all
        // abaixo - no Windows, remove_all com handle ainda aberto lanca
        // "process cannot access the file" (Linux permite unlink de arquivo
        // aberto, Windows nao).
        std::ifstream in(controls_file_path(dir.string(), "default"));
        content.assign(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
    }
    REQUIRE(content.find('\n') != std::string::npos);

    std::filesystem::remove_all(dir);
}

TEST_CASE("save_controls: sobrescreve um arquivo existente (remap novo "
          "persistido de novo)",
          "[controls_file_store]") {
    const auto dir = make_temp_dir("sobrescreve");

    InputRemapConfig cfg1 = default_controls();
    cfg1.actions.front().keys = {KeyBinding{.keycode = 'A'}};
    REQUIRE(save_controls(cfg1, dir.string(), "default"));

    InputRemapConfig cfg2 = default_controls();
    cfg2.actions.front().keys = {KeyBinding{.keycode = 'Z'}};
    REQUIRE(save_controls(cfg2, dir.string(), "default"));

    REQUIRE(load_controls(dir.string(), "default") == cfg2);

    std::filesystem::remove_all(dir);
}

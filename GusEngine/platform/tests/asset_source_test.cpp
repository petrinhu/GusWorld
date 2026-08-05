// SPDX-License-Identifier: Apache-2.0
// GusEngine/platform/tests/asset_source_test.cpp
//
// Catch2 (headless) do AssetSource/FilesystemAssetSource (ADR-013, ASSETS-VFS-F1).
// Prova, por FAMILIA de asset, que a resolucao NOVA (consolidada em
// FilesystemAssetSource) reproduz EXATAMENTE o comportamento dos resolvers duplicados
// que ela substitui (font_atlas::resolve_font_path, translator::resolve_translations_path,
// resolve_sprites_dir/resolve_gus_sprites_dir/resolve_assets_subdir_local/
// resolve_asset_dir, resolve_hit_sfx_path/resolve_ui_sfx_path/resolve_menu_sfx_path,
// resolve_music_path) - com e sem cada env setado (ver ADR-013 "achado da investigacao").
// Tambem prova read()/stat() via FilesystemAssetSource real (arquivos temporarios) e via
// FakeAssetSource in-memory (double reusavel pros testes dos consumidores do Passo 2).

#include <catch2/catch_test_macros.hpp>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "gus/platform/assets/asset_source.hpp"
#include "tmp_dir_test_support.hpp"

// Mesmas macros embutidas pelo CMake no alvo gusengine_platform (ver platform/CMakeLists.txt) -
// espelhadas aqui (platform/tests/CMakeLists.txt) SO pra as asserts "sem env" poderem
// computar o valor esperado sem depender do layout real do repo em runtime.
#ifndef GUSWORLD_ASSETS_DIR
#define GUSWORLD_ASSETS_DIR ""
#endif
#ifndef GUSWORLD_SFX_DIR
#define GUSWORLD_SFX_DIR ""
#endif
#ifndef GUSWORLD_MUSIC_DIR
#define GUSWORLD_MUSIC_DIR ""
#endif
#ifndef GUSWORLD_TRANSLATIONS_DIR
#define GUSWORLD_TRANSLATIONS_DIR ""
#endif
#ifndef GUSWORLD_DIALOGUES_DIR
#define GUSWORLD_DIALOGUES_DIR ""
#endif
#ifndef GUSWORLD_MAPS_DIR
#define GUSWORLD_MAPS_DIR ""
#endif

namespace fs = std::filesystem;
using gus::platform::assets::AssetInfo;
using gus::platform::assets::AssetSource;
using gus::platform::assets::FilesystemAssetSource;
using gus::platform::assets::read_raw_file;

namespace {

std::string join(const std::string& a, const std::string& b) {
    if (a.empty()) return b;
    if (a.back() == '/') return a + b;
    return a + "/" + b;
}

// setenv/unsetenv sao POSIX; MSVC nao tem (usa _putenv_s, e string vazia REMOVE a var).
// Helper portavel minimo pra manter a semantica identica nos dois lados.
void portable_setenv(const char* name, const char* value) {
#ifdef _WIN32
    _putenv_s(name, value);
#else
    setenv(name, value, /*overwrite=*/1);
#endif
}

void portable_unsetenv(const char* name) {
#ifdef _WIN32
    _putenv_s(name, "");
#else
    unsetenv(name);
#endif
}

// RAII: seta uma env var e restaura o valor anterior (ou remove) no fim do escopo -
// isola os testes uns dos outros mesmo que o binario Catch2 rode varios TEST_CASE no
// mesmo processo.
class ScopedEnv {
public:
    ScopedEnv(const char* name, const std::string& value) : name_(name) {
        const char* prev = std::getenv(name);
        if (prev != nullptr) {
            had_prev_ = true;
            prev_ = prev;
        }
        portable_setenv(name, value.c_str());
    }
    ~ScopedEnv() {
        if (had_prev_) {
            portable_setenv(name_.c_str(), prev_.c_str());
        } else {
            portable_unsetenv(name_.c_str());
        }
    }
    ScopedEnv(const ScopedEnv&) = delete;
    ScopedEnv& operator=(const ScopedEnv&) = delete;

private:
    std::string name_;
    bool had_prev_ = false;
    std::string prev_;
};

// Garante que a env var esta AUSENTE durante o escopo (restaura o valor anterior no fim).
class ScopedUnsetEnv {
public:
    explicit ScopedUnsetEnv(const char* name) : name_(name) {
        const char* prev = std::getenv(name);
        if (prev != nullptr) {
            had_prev_ = true;
            prev_ = prev;
        }
        portable_unsetenv(name);
    }
    ~ScopedUnsetEnv() {
        if (had_prev_) {
            portable_setenv(name_.c_str(), prev_.c_str());
        }
    }
    ScopedUnsetEnv(const ScopedUnsetEnv&) = delete;
    ScopedUnsetEnv& operator=(const ScopedUnsetEnv&) = delete;

private:
    std::string name_;
    bool had_prev_ = false;
    std::string prev_;
};

// Diretorio temporario proprio de um teste: alias do helper compartilhado
// gus::test_support::ScopedTempDir (GusEngine/tests/support/tmp_dir_test_support.hpp).
// O contador atomico POR-PROCESSO usado aqui ANTES nao evitava colisao entre
// TEST_CASEs: catch_discover_tests roda 1 PROCESSO por TEST_CASE, e o contador comeca
// em 0 a cada processo - dois processos concorrentes calculavam o MESMO path (mesmo
// defeito de FLAKY-PLAYER-SPRITES-ANIM). unique_temp_dir compoe PID + contador +
// sufixo aleatorio, unico mesmo entre processos concorrentes.
class TempDir : public gus::test_support::ScopedTempDir {
public:
    TempDir() : gus::test_support::ScopedTempDir("gusworld_asset_source_test") {}
};

// RAII: troca o diretorio de trabalho do processo e restaura no fim do escopo.
// ASSETS-PATH-CASCATA: e o unico jeito de exercitar o TERCEIRO nivel da cascata
// (`relativo ao CWD`) sem mexer nas macros, que sao fixadas em tempo de COMPILACAO.
// Declarar SEMPRE depois do TempDir que ele aponta, pra destruicao em ordem reversa
// restaurar o CWD ANTES do remove_all (remover a pasta em que se esta parado deixaria o
// processo com um CWD invalido).
class ScopedCwd {
public:
    explicit ScopedCwd(const fs::path& p) : prev_(fs::current_path()) {
        fs::current_path(p);
    }
    ~ScopedCwd() {
        std::error_code ec;
        fs::current_path(prev_, ec);
    }
    ScopedCwd(const ScopedCwd&) = delete;
    ScopedCwd& operator=(const ScopedCwd&) = delete;

private:
    fs::path prev_;
};

void write_file(const fs::path& p, const std::string& content) {
    std::error_code ec;
    fs::create_directories(p.parent_path(), ec);
    std::ofstream f(p, std::ios::binary);
    f << content;
}

}  // namespace

// ---------------------------------------------------------------- read_raw_file
TEST_CASE("read_raw_file le um caminho concreto, nullopt se ausente", "[asset_source]") {
    TempDir dir;
    const fs::path p = dir.path() / "arquivo.bin";
    write_file(p, "conteudo-de-teste");
    const auto bytes = read_raw_file(p.string());
    REQUIRE(bytes.has_value());
    REQUIRE(bytes->size() == std::string("conteudo-de-teste").size());

    const auto missing = read_raw_file((dir.path() / "nao_existe.bin").string());
    REQUIRE_FALSE(missing.has_value());
}

// ---------------------------------------------------------- familia GENERICA (sprites/images/vfx)
TEST_CASE("FilesystemAssetSource familia GENERICA: env GUSWORLD_ASSETS prefixa o id",
          "[asset_source]") {
    ScopedEnv env("GUSWORLD_ASSETS", "/tmp/gusworld_env_root_nao_usado_de_verdade");
    FilesystemAssetSource src;
    const std::string id = "sprites/caua_volt/walk/south/0.png";
    REQUIRE(src.resolve_path(id) ==
            join("/tmp/gusworld_env_root_nao_usado_de_verdade", id));
}

TEST_CASE("FilesystemAssetSource familia GENERICA: sem env cai no macro ou CWD 'resources/'",
          "[asset_source]") {
    ScopedUnsetEnv no_env("GUSWORLD_ASSETS");
    FilesystemAssetSource src;
    const std::string id = "sprites/caua_volt/south.png";
    const std::string compiled = GUSWORLD_ASSETS_DIR;
    const std::string expected =
        !compiled.empty() ? join(compiled, id) : join("resources", id);
    REQUIRE(src.resolve_path(id) == expected);
}

TEST_CASE("FilesystemAssetSource familia GENERICA: read()/stat() funcionam sobre o id resolvido",
          "[asset_source]") {
    TempDir dir;
    ScopedEnv env("GUSWORLD_ASSETS", dir.path().string());
    write_file(dir.path() / "sprites" / "x.png", "PNGDATA");
    FilesystemAssetSource src;
    const auto bytes = src.read("sprites/x.png");
    REQUIRE(bytes.has_value());
    REQUIRE(bytes->size() == std::string("PNGDATA").size());
    const auto info = src.stat("sprites/x.png");
    REQUIRE(info.has_value());
    REQUIRE(info->size == std::string("PNGDATA").size());

    REQUIRE_FALSE(src.read("sprites/nao_existe.png").has_value());
    REQUIRE_FALSE(src.stat("sprites/nao_existe.png").has_value());
}

// ---------------------------------------------------------------- familia FONTES
TEST_CASE(
    "FilesystemAssetSource familia FONTES: env so vence SE o arquivo existir la (nao "
    "sequestra)",
    "[asset_source]") {
    TempDir dir;  // NAO tem fonts/<arquivo> dentro - candidato do env nao existe
    ScopedEnv env("GUSWORLD_ASSETS", dir.path().string());
    FilesystemAssetSource src;
    const std::string id = "assets/fonts/PixelOperatorMono.ttf";
    const std::string p = src.resolve_path(id);
    // Nao deve ter "sequestrado" pro env (o candidato la nao existe de fato).
    REQUIRE(p.find(dir.path().string()) == std::string::npos);
}

TEST_CASE("FilesystemAssetSource familia FONTES: env vence quando o arquivo existe la",
          "[asset_source]") {
    TempDir dir;
    write_file(dir.path() / "fonts" / "Fake.ttf", "FAKEFONT");
    ScopedEnv env("GUSWORLD_ASSETS", dir.path().string());
    FilesystemAssetSource src;
    const std::string id = "assets/fonts/Fake.ttf";
    REQUIRE(src.resolve_path(id) ==
            join(join(dir.path().string(), "fonts"), "Fake.ttf"));
    const auto bytes = src.read(id);
    REQUIRE(bytes.has_value());
    REQUIRE(bytes->size() == std::string("FAKEFONT").size());
}

TEST_CASE(
    "FilesystemAssetSource familia FONTES: sem env cai no macro/CWD (fonte real do repo)",
    "[asset_source]") {
    ScopedUnsetEnv no_env("GUSWORLD_ASSETS");
    FilesystemAssetSource src;
    const std::string id = "assets/fonts/PixelOperatorMono.ttf";
    const auto bytes = src.read(id);
    // A Pixel Operator Mono e um asset CC0 versionado - deve existir via macro ou CWD.
    REQUIRE(bytes.has_value());
    REQUIRE_FALSE(bytes->empty());
}

// ---------------------------------------------------------------- familia I18N (traducoes)
TEST_CASE(
    "FilesystemAssetSource familia I18N: env e OVERRIDE LITERAL completo (ignora o id)",
    "[asset_source]") {
    TempDir dir;
    const fs::path literal = dir.path() / "qualquer_nome.md";
    write_file(literal, "## X\nvalor\n");
    ScopedEnv env("GUSWORLD_TRANSLATIONS", literal.string());
    FilesystemAssetSource src;
    // Mesmo pedindo um id de OUTRO arquivo, o override literal manda (paridade com
    // resolve_translations_path: devolve o env AS-IS, sem juntar com o id).
    REQUIRE(src.resolve_path("resources/translations/pt_br.md") == literal.string());
    const auto bytes = src.read("resources/translations/pt_br.md");
    REQUIRE(bytes.has_value());
}

TEST_CASE(
    "FilesystemAssetSource familia I18N: sem env usa macro/CWD + nome do arquivo do id",
    "[asset_source]") {
    ScopedUnsetEnv no_env("GUSWORLD_TRANSLATIONS");
    FilesystemAssetSource src;
    const std::string id = "resources/translations/pt_br.md";
    const std::string compiled = GUSWORLD_TRANSLATIONS_DIR;
    const std::string expected = !compiled.empty() ? join(compiled, "pt_br.md") : id;
    REQUIRE(src.resolve_path(id) == expected);
}

// ------------------------------------------------------------ familia DIALOGUES (M7-DIALOGO)
TEST_CASE(
    "FilesystemAssetSource familia DIALOGUES: env e OVERRIDE LITERAL completo (ignora "
    "o id)",
    "[asset_source]") {
    TempDir dir;
    const fs::path literal = dir.path() / "qualquer_grafo.dlg.txt";
    write_file(literal, "@dialogue npc_intro_bertoldo\n");
    ScopedEnv env("GUSWORLD_DIALOGUES", literal.string());
    FilesystemAssetSource src;
    // Mesmo pedindo um id de OUTRO arquivo, o override literal manda (paridade com
    // resolve_npc_intro_bertoldo_dialogue_path: devolve o env AS-IS, sem juntar com o
    // id).
    REQUIRE(src.resolve_path("resources/dialogues/npc_intro_bertoldo.dlg.txt") ==
            literal.string());
    const auto bytes = src.read("resources/dialogues/npc_intro_bertoldo.dlg.txt");
    REQUIRE(bytes.has_value());
}

TEST_CASE(
    "FilesystemAssetSource familia DIALOGUES: sem env usa macro/CWD + nome do arquivo "
    "do id",
    "[asset_source]") {
    ScopedUnsetEnv no_env("GUSWORLD_DIALOGUES");
    FilesystemAssetSource src;
    const std::string id = "resources/dialogues/npc_intro_bertoldo.dlg.txt";
    const std::string compiled = GUSWORLD_DIALOGUES_DIR;
    const std::string expected =
        !compiled.empty() ? join(compiled, "npc_intro_bertoldo.dlg.txt") : id;
    REQUIRE(src.resolve_path(id) == expected);
}

// ---------------------------------------------------------------- familia SFX
TEST_CASE("FilesystemAssetSource familia SFX: env GUSWORLD_SFX prefixa so o NOME do arquivo",
          "[asset_source]") {
    ScopedEnv env("GUSWORLD_SFX", "/tmp/gusworld_sfx_root_nao_usado_de_verdade");
    FilesystemAssetSource src;
    REQUIRE(src.resolve_path("assets/sfx/hit_digital_provisorio.wav") ==
            join("/tmp/gusworld_sfx_root_nao_usado_de_verdade",
                 "hit_digital_provisorio.wav"));
}

TEST_CASE("FilesystemAssetSource familia SFX: sem env cai no macro/CWD", "[asset_source]") {
    ScopedUnsetEnv no_env("GUSWORLD_SFX");
    FilesystemAssetSource src;
    const std::string id = "assets/sfx/hit_digital_provisorio.wav";
    const std::string compiled = GUSWORLD_SFX_DIR;
    const std::string expected =
        !compiled.empty() ? join(compiled, "hit_digital_provisorio.wav") : id;
    REQUIRE(src.resolve_path(id) == expected);
}

// ---------------------------------------------------------------- familia MUSICA
TEST_CASE(
    "FilesystemAssetSource familia MUSICA: env GUSWORLD_MUSIC prefixa so o NOME do "
    "arquivo",
    "[asset_source]") {
    ScopedEnv env("GUSWORLD_MUSIC", "/tmp/gusworld_music_root_nao_usado_de_verdade");
    FilesystemAssetSource src;
    REQUIRE(src.resolve_path("assets/music/cidade_tema_provisorio.mp3") ==
            join("/tmp/gusworld_music_root_nao_usado_de_verdade",
                 "cidade_tema_provisorio.mp3"));
}

TEST_CASE("FilesystemAssetSource familia MUSICA: sem env cai no macro/CWD",
          "[asset_source]") {
    ScopedUnsetEnv no_env("GUSWORLD_MUSIC");
    FilesystemAssetSource src;
    const std::string id = "assets/music/cidade_tema_provisorio.mp3";
    const std::string compiled = GUSWORLD_MUSIC_DIR;
    const std::string expected =
        !compiled.empty() ? join(compiled, "cidade_tema_provisorio.mp3") : id;
    REQUIRE(src.resolve_path(id) == expected);
}

// ---------------------------------------------------------------- familia MAPAS
TEST_CASE(
    "FilesystemAssetSource familia MAPAS: env GUSWORLD_MAPS prefixa so o NOME do "
    "arquivo",
    "[asset_source]") {
    ScopedEnv env("GUSWORLD_MAPS", "/tmp/gusworld_maps_root_nao_usado_de_verdade");
    FilesystemAssetSource src;
    REQUIRE(src.resolve_path("assets/maps/compiled/distritos_inferiores.gmap") ==
            join("/tmp/gusworld_maps_root_nao_usado_de_verdade",
                 "distritos_inferiores.gmap"));
}

TEST_CASE("FilesystemAssetSource familia MAPAS: sem env cai no macro/CWD",
          "[asset_source]") {
    ScopedUnsetEnv no_env("GUSWORLD_MAPS");
    FilesystemAssetSource src;
    const std::string id = "assets/maps/compiled/distritos_inferiores.gmap";
    const std::string compiled = GUSWORLD_MAPS_DIR;
    const std::string expected =
        !compiled.empty() ? join(compiled, "distritos_inferiores.gmap") : id;
    REQUIRE(src.resolve_path(id) == expected);
}

// ============================================================ ASSETS-PATH-CASCATA
// O macro embutido pelo CMake carrega o caminho ABSOLUTO DA MAQUINA DE BUILD
// (GUSWORLD_ASSETS_DIR="${CMAKE_SOURCE_DIR}/../resources" e irmas). Ate esta fatia, SEIS
// familias aceitavam esse macro SEM checar o disco, entao nunca chegavam ao 3o nivel da
// cadeia (`relativo ao CWD`): um binario compilado numa maquina e copiado pra outra (o caso
// EXATO do AppImage de RELEASE-DEMO-APPIMAGE) procurava em /home/<outro-usuario>/... e nunca
// olhava o `resources/` que estava ao lado dele. A familia FONTES ja fazia a cascata certa e
// e o modelo seguido aqui.
//
// COMO ESTES TESTES EXERCITAM O 3o NIVEL: as macros sao fixadas em tempo de COMPILACAO e nao
// dao pra variar em runtime; o que se varia e o OUTRO lado da comparacao - o processo passa a
// rodar de um CWD temporario onde o caminho relativo da familia EXISTE, enquanto o id pedido
// nao existe sob o macro. E o mesmo cenario do jogo empacotado.
//
// LIMITE DA MUDANCA (proposital): so o nivel do MACRO ganhou checagem de existencia. A env
// continua vencendo SEM checagem nas seis familias, com a semantica propria de cada uma
// (override LITERAL em I18N/DIALOGUES; PASTA em SFX/MUSICA/MAPAS/GENERICA) - os casos "env
// aponta pra caminho INEXISTENTE e AINDA ASSIM vence" abaixo travam isso.

TEST_CASE(
    "ASSETS-PATH-CASCATA familia GENERICA: macro inexistente cai no relativo ao CWD",
    "[asset_source]") {
    ScopedUnsetEnv no_env("GUSWORLD_ASSETS");
    TempDir dir;
    const std::string id = "sprites/zz_cascata_inexistente_sob_o_macro.png";
    const std::string cwd_rel = join("resources", id);
    write_file(dir.path() / cwd_rel, "PNGDATA");
    ScopedCwd cwd(dir.path());  // depois do TempDir: destroi antes dele (ordem reversa)

    FilesystemAssetSource src;
    REQUIRE(src.resolve_path(id) == cwd_rel);
    const auto bytes = src.read(id);
    REQUIRE(bytes.has_value());
    REQUIRE(bytes->size() == std::string("PNGDATA").size());
}

TEST_CASE(
    "ASSETS-PATH-CASCATA familia GENERICA: macro EXISTENTE continua vencendo o relativo",
    "[asset_source]") {
    ScopedUnsetEnv no_env("GUSWORLD_ASSETS");
    TempDir dir;
    const std::string id = "sprites/caua_volt/south.png";  // existe sob o macro (repo real)
    write_file(dir.path() / join("resources", id), "HOMONIMO");  // isca no CWD
    ScopedCwd cwd(dir.path());

    FilesystemAssetSource src;
    const std::string compiled = GUSWORLD_ASSETS_DIR;
    REQUIRE_FALSE(compiled.empty());
    REQUIRE(src.resolve_path(id) == join(compiled, id));
}

TEST_CASE(
    "ASSETS-PATH-CASCATA familia GENERICA: nada existe devolve o melhor chute (macro)",
    "[asset_source]") {
    ScopedUnsetEnv no_env("GUSWORLD_ASSETS");
    TempDir dir;
    ScopedCwd cwd(dir.path());

    FilesystemAssetSource src;
    const std::string id = "sprites/zz_nao_existe_em_lugar_nenhum.png";
    const std::string compiled = GUSWORLD_ASSETS_DIR;
    const std::string expected =
        !compiled.empty() ? join(compiled, id) : join("resources", id);
    REQUIRE(src.resolve_path(id) == expected);  // log aponta o caminho esperado
    REQUIRE_FALSE(src.read(id).has_value());    // e nao crasha
}

TEST_CASE(
    "ASSETS-PATH-CASCATA familia GENERICA: env vence mesmo apontando pra pasta inexistente",
    "[asset_source]") {
    TempDir dir;
    const std::string root = (dir.path() / "pasta_que_nao_existe").string();
    ScopedEnv env("GUSWORLD_ASSETS", root);
    ScopedCwd cwd(dir.path());

    FilesystemAssetSource src;
    const std::string id = "sprites/caua_volt/south.png";  // existe sob o macro
    REQUIRE(src.resolve_path(id) == join(root, id));       // env manda assim mesmo
}

TEST_CASE("ASSETS-PATH-CASCATA familia I18N: macro inexistente cai no relativo ao CWD",
          "[asset_source]") {
    ScopedUnsetEnv no_env("GUSWORLD_TRANSLATIONS");
    TempDir dir;
    const std::string id = "resources/translations/zz_cascata.md";
    write_file(dir.path() / id, "## X\nvalor\n");
    ScopedCwd cwd(dir.path());

    FilesystemAssetSource src;
    REQUIRE(src.resolve_path(id) == id);
    REQUIRE(src.read(id).has_value());
}

TEST_CASE("ASSETS-PATH-CASCATA familia I18N: macro EXISTENTE vence, e nada-existe cai no chute",
          "[asset_source]") {
    ScopedUnsetEnv no_env("GUSWORLD_TRANSLATIONS");
    TempDir dir;
    const std::string id = "resources/translations/pt_br.md";  // existe sob o macro
    write_file(dir.path() / id, "ISCA");
    ScopedCwd cwd(dir.path());

    FilesystemAssetSource src;
    const std::string compiled = GUSWORLD_TRANSLATIONS_DIR;
    REQUIRE_FALSE(compiled.empty());
    REQUIRE(src.resolve_path(id) == join(compiled, "pt_br.md"));

    const std::string ausente = "resources/translations/zz_nem_aqui_nem_la.md";
    REQUIRE(src.resolve_path(ausente) == join(compiled, "zz_nem_aqui_nem_la.md"));
    REQUIRE_FALSE(src.read(ausente).has_value());
}

TEST_CASE(
    "ASSETS-PATH-CASCATA familia I18N: env e override LITERAL e vence mesmo inexistente",
    "[asset_source]") {
    TempDir dir;
    const std::string literal = (dir.path() / "nao_gravado.md").string();
    ScopedEnv env("GUSWORLD_TRANSLATIONS", literal);
    ScopedCwd cwd(dir.path());

    FilesystemAssetSource src;
    // Ignora o id INTEIRO (nao junta com o nome do arquivo) e nao checa o disco.
    REQUIRE(src.resolve_path("resources/translations/pt_br.md") == literal);
}

TEST_CASE("ASSETS-PATH-CASCATA familia DIALOGUES: macro inexistente cai no relativo ao CWD",
          "[asset_source]") {
    ScopedUnsetEnv no_env("GUSWORLD_DIALOGUES");
    TempDir dir;
    const std::string id = "resources/dialogues/zz_cascata.dlg.txt";
    write_file(dir.path() / id, "@dialogue zz\n");
    ScopedCwd cwd(dir.path());

    FilesystemAssetSource src;
    REQUIRE(src.resolve_path(id) == id);
    REQUIRE(src.read(id).has_value());
}

TEST_CASE(
    "ASSETS-PATH-CASCATA familia DIALOGUES: macro EXISTENTE vence, e nada-existe cai no chute",
    "[asset_source]") {
    ScopedUnsetEnv no_env("GUSWORLD_DIALOGUES");
    TempDir dir;
    const std::string id = "resources/dialogues/npc_intro_bertoldo.dlg.txt";
    write_file(dir.path() / id, "ISCA");
    ScopedCwd cwd(dir.path());

    FilesystemAssetSource src;
    const std::string compiled = GUSWORLD_DIALOGUES_DIR;
    REQUIRE_FALSE(compiled.empty());
    REQUIRE(src.resolve_path(id) == join(compiled, "npc_intro_bertoldo.dlg.txt"));

    const std::string ausente = "resources/dialogues/zz_nem_aqui_nem_la.dlg.txt";
    REQUIRE(src.resolve_path(ausente) == join(compiled, "zz_nem_aqui_nem_la.dlg.txt"));
    REQUIRE_FALSE(src.read(ausente).has_value());
}

TEST_CASE(
    "ASSETS-PATH-CASCATA familia DIALOGUES: env e override LITERAL e vence mesmo inexistente",
    "[asset_source]") {
    TempDir dir;
    const std::string literal = (dir.path() / "nao_gravado.dlg.txt").string();
    ScopedEnv env("GUSWORLD_DIALOGUES", literal);
    ScopedCwd cwd(dir.path());

    FilesystemAssetSource src;
    REQUIRE(src.resolve_path("resources/dialogues/npc_intro_bertoldo.dlg.txt") == literal);
}

TEST_CASE("ASSETS-PATH-CASCATA familia SFX: macro inexistente cai no relativo ao CWD",
          "[asset_source]") {
    ScopedUnsetEnv no_env("GUSWORLD_SFX");
    TempDir dir;
    const std::string id = "assets/sfx/zz_cascata.wav";
    write_file(dir.path() / id, "RIFFFAKE");
    ScopedCwd cwd(dir.path());

    FilesystemAssetSource src;
    REQUIRE(src.resolve_path(id) == id);
    REQUIRE(src.read(id).has_value());
}

TEST_CASE("ASSETS-PATH-CASCATA familia SFX: macro EXISTENTE vence, e nada-existe cai no chute",
          "[asset_source]") {
    ScopedUnsetEnv no_env("GUSWORLD_SFX");
    TempDir dir;
    const std::string id = "assets/sfx/hit_digital_provisorio.wav";
    write_file(dir.path() / id, "ISCA");
    ScopedCwd cwd(dir.path());

    FilesystemAssetSource src;
    const std::string compiled = GUSWORLD_SFX_DIR;
    REQUIRE_FALSE(compiled.empty());
    REQUIRE(src.resolve_path(id) == join(compiled, "hit_digital_provisorio.wav"));

    const std::string ausente = "assets/sfx/zz_nem_aqui_nem_la.wav";
    REQUIRE(src.resolve_path(ausente) == join(compiled, "zz_nem_aqui_nem_la.wav"));
    REQUIRE_FALSE(src.read(ausente).has_value());
}

TEST_CASE("ASSETS-PATH-CASCATA familia SFX: env e PASTA e vence mesmo inexistente",
          "[asset_source]") {
    TempDir dir;
    const std::string root = (dir.path() / "pasta_que_nao_existe").string();
    ScopedEnv env("GUSWORLD_SFX", root);
    ScopedCwd cwd(dir.path());

    FilesystemAssetSource src;
    // Junta so o NOME do arquivo (nao o id inteiro) e nao checa o disco.
    REQUIRE(src.resolve_path("assets/sfx/hit_digital_provisorio.wav") ==
            join(root, "hit_digital_provisorio.wav"));
}

TEST_CASE("ASSETS-PATH-CASCATA familia MUSICA: macro inexistente cai no relativo ao CWD",
          "[asset_source]") {
    ScopedUnsetEnv no_env("GUSWORLD_MUSIC");
    TempDir dir;
    const std::string id = "assets/music/zz_cascata.mp3";
    write_file(dir.path() / id, "ID3FAKE");
    ScopedCwd cwd(dir.path());

    FilesystemAssetSource src;
    REQUIRE(src.resolve_path(id) == id);
    REQUIRE(src.read(id).has_value());
}

TEST_CASE(
    "ASSETS-PATH-CASCATA familia MUSICA: macro EXISTENTE vence, e nada-existe cai no chute",
    "[asset_source]") {
    ScopedUnsetEnv no_env("GUSWORLD_MUSIC");
    TempDir dir;
    const std::string id = "assets/music/cidade_tema_provisorio.mp3";
    write_file(dir.path() / id, "ISCA");
    ScopedCwd cwd(dir.path());

    FilesystemAssetSource src;
    const std::string compiled = GUSWORLD_MUSIC_DIR;
    REQUIRE_FALSE(compiled.empty());
    REQUIRE(src.resolve_path(id) == join(compiled, "cidade_tema_provisorio.mp3"));

    const std::string ausente = "assets/music/zz_nem_aqui_nem_la.mp3";
    REQUIRE(src.resolve_path(ausente) == join(compiled, "zz_nem_aqui_nem_la.mp3"));
    REQUIRE_FALSE(src.read(ausente).has_value());
}

TEST_CASE("ASSETS-PATH-CASCATA familia MUSICA: env e PASTA e vence mesmo inexistente",
          "[asset_source]") {
    TempDir dir;
    const std::string root = (dir.path() / "pasta_que_nao_existe").string();
    ScopedEnv env("GUSWORLD_MUSIC", root);
    ScopedCwd cwd(dir.path());

    FilesystemAssetSource src;
    REQUIRE(src.resolve_path("assets/music/cidade_tema_provisorio.mp3") ==
            join(root, "cidade_tema_provisorio.mp3"));
}

TEST_CASE("ASSETS-PATH-CASCATA familia MAPAS: macro inexistente cai no relativo ao CWD",
          "[asset_source]") {
    ScopedUnsetEnv no_env("GUSWORLD_MAPS");
    TempDir dir;
    const std::string id = "assets/maps/compiled/zz_cascata.gmap";
    write_file(dir.path() / id, "GMAPFAKE");
    ScopedCwd cwd(dir.path());

    FilesystemAssetSource src;
    REQUIRE(src.resolve_path(id) == id);
    REQUIRE(src.read(id).has_value());
}

TEST_CASE(
    "ASSETS-PATH-CASCATA familia MAPAS: macro EXISTENTE vence, e nada-existe cai no chute",
    "[asset_source]") {
    ScopedUnsetEnv no_env("GUSWORLD_MAPS");
    TempDir dir;
    const std::string id = "assets/maps/compiled/distritos_inferiores.gmap";
    write_file(dir.path() / id, "ISCA");
    ScopedCwd cwd(dir.path());

    FilesystemAssetSource src;
    const std::string compiled = GUSWORLD_MAPS_DIR;
    REQUIRE_FALSE(compiled.empty());
    REQUIRE(src.resolve_path(id) == join(compiled, "distritos_inferiores.gmap"));

    const std::string ausente = "assets/maps/compiled/zz_nem_aqui_nem_la.gmap";
    REQUIRE(src.resolve_path(ausente) == join(compiled, "zz_nem_aqui_nem_la.gmap"));
    REQUIRE_FALSE(src.read(ausente).has_value());
}

TEST_CASE("ASSETS-PATH-CASCATA familia MAPAS: env e PASTA e vence mesmo inexistente",
          "[asset_source]") {
    TempDir dir;
    const std::string root = (dir.path() / "pasta_que_nao_existe").string();
    ScopedEnv env("GUSWORLD_MAPS", root);
    ScopedCwd cwd(dir.path());

    FilesystemAssetSource src;
    REQUIRE(src.resolve_path("assets/maps/compiled/distritos_inferiores.gmap") ==
            join(root, "distritos_inferiores.gmap"));
}

TEST_CASE(
    "ASSETS-PATH-CASCATA familia FONTES: cascata de existencia INTACTA (modelo, nao "
    "regride)",
    "[asset_source]") {
    ScopedUnsetEnv no_env("GUSWORLD_ASSETS");
    TempDir dir;
    const std::string id = "assets/fonts/zz_cascata.ttf";
    write_file(dir.path() / id, "FAKEFONT");
    ScopedCwd cwd(dir.path());

    FilesystemAssetSource src;
    // A fonte ja fazia isto ANTES desta fatia - o teste existe pra travar o modelo.
    REQUIRE(src.resolve_path(id) == id);
    REQUIRE(src.read(id).has_value());
}

// ---------------------------------------------------------------- FakeAssetSource (double)
namespace {

// In-memory: pros testes dos CONSUMIDORES (Passo 2), sem tocar disco. Guarda um mapa
// id -> bytes; read()/stat() consultam so esse mapa. Reusavel por outros arquivos de
// teste que incluam este .cpp nao e possivel (Catch2 = 1 TU por arquivo); consumidores
// futuros que precisarem do double podem copiar esta classe pro proprio teste (mesmo
// padrao ja usado pelos "IRenderer falso" espalhados pelos testes de app/, ver
// player_sprites_layout_test.cpp/city_scene_test.cpp/battle_scene_test.cpp).
class FakeAssetSource final : public AssetSource {
public:
    void put(std::string id, std::string content) {
        store_[std::move(id)] = std::move(content);
    }

    [[nodiscard]] std::optional<std::vector<std::byte>> read(
        std::string_view id) const override {
        const auto it = store_.find(std::string(id));
        if (it == store_.end()) {
            return std::nullopt;
        }
        std::vector<std::byte> bytes(it->second.size());
        for (std::size_t i = 0; i < it->second.size(); ++i) {
            bytes[i] = static_cast<std::byte>(it->second[i]);
        }
        return bytes;
    }

    [[nodiscard]] std::optional<AssetInfo> stat(std::string_view id) const override {
        const auto it = store_.find(std::string(id));
        if (it == store_.end()) {
            return std::nullopt;
        }
        AssetInfo info;
        info.size = it->second.size();
        info.mtime = 0;
        return info;
    }

private:
    std::unordered_map<std::string, std::string> store_;
};

}  // namespace

TEST_CASE("FakeAssetSource: read/stat servem do mapa in-memory, sem tocar disco",
          "[asset_source]") {
    FakeAssetSource fake;
    fake.put("resources/translations/pt_br.md", "## X\nvalor\n");
    const auto bytes = fake.read("resources/translations/pt_br.md");
    REQUIRE(bytes.has_value());
    REQUIRE(bytes->size() == std::string("## X\nvalor\n").size());
    REQUIRE_FALSE(fake.read("ausente").has_value());
    const auto info = fake.stat("resources/translations/pt_br.md");
    REQUIRE(info.has_value());
    REQUIRE(info->size == std::string("## X\nvalor\n").size());
}

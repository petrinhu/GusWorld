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

#include <atomic>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "gus/platform/assets/asset_source.hpp"

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

// Diretorio temporario proprio de um teste (apagado no destrutor). Nome unico por
// contador atomico (nao por endereco de objeto - evita colisao entre TempDirs
// criados/destruidos em enderecos de pilha reaproveitados).
std::atomic<std::uint64_t> g_tempdir_seq{0};

class TempDir {
public:
    TempDir() {
        const std::uint64_t n = g_tempdir_seq.fetch_add(1);
        path_ = fs::temp_directory_path() /
                ("gusworld_asset_source_test_" + std::to_string(n));
        std::error_code ec;
        fs::remove_all(path_, ec);
        fs::create_directories(path_, ec);
    }
    ~TempDir() {
        std::error_code ec;
        fs::remove_all(path_, ec);
    }
    TempDir(const TempDir&) = delete;
    TempDir& operator=(const TempDir&) = delete;

    [[nodiscard]] const fs::path& path() const noexcept { return path_; }

private:
    fs::path path_;
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

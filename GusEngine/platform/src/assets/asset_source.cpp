// gus/platform/src/assets/asset_source.cpp
//
// Implementacao do porteiro de assets (ver header). FilesystemAssetSource consolida, POR
// FAMILIA, a cadeia `env > macro de compilacao > relativo ao CWD` que antes vivia
// duplicada em font_atlas::resolve_font_path, translator::resolve_translations_path,
// resolve_sprites_dir/resolve_gus_sprites_dir/resolve_assets_subdir_local/
// resolve_asset_dir (player_sprites_loader/anim_catalog/sdl_window/battle_preview),
// resolve_hit_sfx_path/resolve_ui_sfx_path (battle_preview) e resolve_menu_sfx_path
// (system_menu_loop), resolve_music_path (battle_preview),
// resolve_npc_intro_bertoldo_dialogue_path (npc_dialogue_catalog) e
// resolve_assets_subdir/resolve_dialogue_sfx_path (npc_dialogue_loop_gl), e
// resolve_distritos_inferiores_gmap (city_scene, familia MAPS, ASSETS-VFS-F1c). Estes
// agora DELEGAM pra ca (retrofit ASSETS-VFS-F1/ASSETS-VFS-F1b/ASSETS-VFS-F1c) - ver cada
// um deles pro comentario "MESMA logica que FilesystemAssetSource".

#include "gus/platform/assets/asset_source.hpp"

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <filesystem>

// Macros embutidas pelo CMake (platform/CMakeLists.txt), uma por familia de asset. Raizes
// DIFERENTES por familia (ver header) - preservadas exatamente como os resolvers
// duplicados que este arquivo substitui.
#ifndef GUSWORLD_FONTS_DIR
#define GUSWORLD_FONTS_DIR ""
#endif
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

namespace gus::platform::assets {

namespace {

namespace fs = std::filesystem;

std::string join(const std::string& a, const std::string& b) {
    if (a.empty()) {
        return b;
    }
    if (a.back() == '/') {
        return a + b;
    }
    return a + "/" + b;
}

bool exists_on_disk(const std::string& p) {
    if (p.empty()) {
        return false;
    }
    std::error_code ec;
    return fs::exists(p, ec);
}

// Devolve o valor de uma env var, ou string vazia se ausente/vazia.
std::string env_or_empty(const char* name) {
    const char* v = std::getenv(name);
    return (v != nullptr) ? std::string(v) : std::string();
}

// Remove o PREFIXO `prefix` de `id` se presente; senao devolve `id` inteiro (defensivo -
// nao deveria acontecer se o dispatcher classificou certo, mas nao lanca).
std::string strip_prefix(std::string_view id, std::string_view prefix) {
    if (id.size() >= prefix.size() && id.substr(0, prefix.size()) == prefix) {
        return std::string(id.substr(prefix.size()));
    }
    return std::string(id);
}

constexpr std::string_view kFontsPrefix = "assets/fonts/";
constexpr std::string_view kTranslationsPrefix = "game/translations/";
constexpr std::string_view kDialoguesPrefix = "game/dialogues/";
constexpr std::string_view kSfxPrefix = "assets/sfx/";
constexpr std::string_view kMusicPrefix = "assets/music/";
constexpr std::string_view kMapsPrefix = "assets/maps/compiled/";

bool starts_with(std::string_view s, std::string_view prefix) {
    return s.size() >= prefix.size() && s.substr(0, prefix.size()) == prefix;
}

// --- FAMILIA FONTES (paridade com font_atlas::resolve_font_path) ------------------
// Unica familia que faz cascata de EXISTENCIA: env nao pode "sequestrar" a fonte pra
// uma pasta sem o arquivo (comentario original preservado).
std::string resolve_font(std::string_view id) {
    const std::string filename = strip_prefix(id, kFontsPrefix);
    const std::string env = env_or_empty("GUSWORLD_ASSETS");
    if (!env.empty()) {
        const std::string cand = join(join(env, "fonts"), filename);
        if (exists_on_disk(cand)) {
            return cand;
        }
    }
    const std::string compiled = join(std::string(GUSWORLD_FONTS_DIR), filename);
    if (exists_on_disk(compiled)) {
        return compiled;
    }
    const std::string cwd_rel(id);
    if (exists_on_disk(cwd_rel)) {
        return cwd_rel;
    }
    // Nenhum existe: melhor chute (compilado), pro erro ficar claro no log - mesmo
    // fallback do resolve_font_path original.
    return compiled.empty() ? cwd_rel : compiled;
}

// --- FAMILIA I18N (paridade com translator::resolve_translations_path) ------------
// env e OVERRIDE LITERAL (ignora o id por completo) - unica familia assim, pois so
// existe 1 catalogo hoje e o lider aponta o .md inteiro. Nenhuma checagem de exists.
std::string resolve_translations(std::string_view id) {
    const std::string env = env_or_empty("GUSWORLD_TRANSLATIONS");
    if (!env.empty()) {
        return env;
    }
    const std::string filename = strip_prefix(id, kTranslationsPrefix);
    const std::string compiled = std::string(GUSWORLD_TRANSLATIONS_DIR);
    if (!compiled.empty()) {
        return join(compiled, filename);
    }
    return std::string(id);
}

// --- FAMILIA DIALOGUES (paridade com resolve_npc_intro_bertoldo_dialogue_path,
// npc_dialogue_catalog.cpp, M7-DIALOGO/ASSETS-VFS-F1b) -----------------------------
// env e OVERRIDE LITERAL (ignora o id por completo) - MESMO padrao da familia I18N
// (so existe 1 grafo hoje, o lider aponta o .dlg.txt inteiro). Nenhuma checagem de
// exists.
std::string resolve_dialogues(std::string_view id) {
    const std::string env = env_or_empty("GUSWORLD_DIALOGUES");
    if (!env.empty()) {
        return env;
    }
    const std::string filename = strip_prefix(id, kDialoguesPrefix);
    const std::string compiled = std::string(GUSWORLD_DIALOGUES_DIR);
    if (!compiled.empty()) {
        return join(compiled, filename);
    }
    return std::string(id);
}

// --- FAMILIA SFX (paridade com resolve_hit_sfx_path/resolve_ui_sfx_path/
// resolve_menu_sfx_path) -----------------------------------------------------------
std::string resolve_sfx(std::string_view id) {
    const std::string filename = strip_prefix(id, kSfxPrefix);
    const std::string env = env_or_empty("GUSWORLD_SFX");
    if (!env.empty()) {
        return join(env, filename);
    }
    const std::string compiled = std::string(GUSWORLD_SFX_DIR);
    if (!compiled.empty()) {
        return join(compiled, filename);
    }
    return std::string(id);
}

// --- FAMILIA MUSICA (paridade com resolve_music_path) -----------------------------
std::string resolve_music(std::string_view id) {
    const std::string filename = strip_prefix(id, kMusicPrefix);
    const std::string env = env_or_empty("GUSWORLD_MUSIC");
    if (!env.empty()) {
        return join(env, filename);
    }
    const std::string compiled = std::string(GUSWORLD_MUSIC_DIR);
    if (!compiled.empty()) {
        return join(compiled, filename);
    }
    return std::string(id);
}

// --- FAMILIA MAPAS (paridade com city_scene.cpp::resolve_distritos_inferiores_gmap,
// ASSETS-VFS-F1c) -------------------------------------------------------------------
// MESMO padrao de SFX/MUSICA: env e uma PASTA (juntada so ao NOME do arquivo, via
// strip_prefix), NAO um override literal do id inteiro (diferente de I18N/DIALOGUES).
// Nenhuma checagem de exists (paridade com o resolver original, que tambem nunca
// verificava o disco).
std::string resolve_maps(std::string_view id) {
    const std::string filename = strip_prefix(id, kMapsPrefix);
    const std::string env = env_or_empty("GUSWORLD_MAPS");
    if (!env.empty()) {
        return join(env, filename);
    }
    const std::string compiled = std::string(GUSWORLD_MAPS_DIR);
    if (!compiled.empty()) {
        return join(compiled, filename);
    }
    return std::string(id);
}

// --- FAMILIA GENERICA (paridade com resolve_sprites_dir/resolve_gus_sprites_dir/
// resolve_assets_subdir_local/resolve_asset_dir) - sprites/images/vfx sob resources/. ---
std::string resolve_generic(std::string_view id) {
    const std::string env = env_or_empty("GUSWORLD_ASSETS");
    if (!env.empty()) {
        return join(env, std::string(id));
    }
    const std::string compiled = std::string(GUSWORLD_ASSETS_DIR);
    if (!compiled.empty()) {
        return join(compiled, std::string(id));
    }
    return join("resources", std::string(id));
}

// Dispatcher por PREFIXO do id (ver header pra tabela completa por familia).
std::string resolve_by_family(std::string_view id) {
    if (starts_with(id, kFontsPrefix)) {
        return resolve_font(id);
    }
    if (starts_with(id, kTranslationsPrefix)) {
        return resolve_translations(id);
    }
    if (starts_with(id, kDialoguesPrefix)) {
        return resolve_dialogues(id);
    }
    if (starts_with(id, kSfxPrefix)) {
        return resolve_sfx(id);
    }
    if (starts_with(id, kMusicPrefix)) {
        return resolve_music(id);
    }
    if (starts_with(id, kMapsPrefix)) {
        return resolve_maps(id);
    }
    return resolve_generic(id);
}

}  // namespace

std::optional<std::vector<std::byte>> read_raw_file(std::string_view path) {
    if (path.empty()) {
        return std::nullopt;
    }
    const std::string p(path);
    std::FILE* f = std::fopen(p.c_str(), "rb");
    if (f == nullptr) {
        return std::nullopt;
    }
    std::fseek(f, 0, SEEK_END);
    const long size = std::ftell(f);
    std::fseek(f, 0, SEEK_SET);
    if (size < 0) {
        std::fclose(f);
        return std::nullopt;
    }
    std::vector<std::byte> bytes(static_cast<std::size_t>(size));
    if (size > 0) {
        const std::size_t got =
            std::fread(bytes.data(), 1, static_cast<std::size_t>(size), f);
        if (got != static_cast<std::size_t>(size)) {
            std::fclose(f);
            return std::nullopt;  // leitura incompleta: trata como falha
        }
    }
    std::fclose(f);
    return bytes;
}

std::string FilesystemAssetSource::resolve_path(std::string_view id) const {
    return resolve_by_family(id);
}

std::optional<std::vector<std::byte>> FilesystemAssetSource::read(
    std::string_view id) const {
    const std::string path = resolve_path(id);
    return read_raw_file(path);
}

std::optional<AssetInfo> FilesystemAssetSource::stat(std::string_view id) const {
    const std::string path = resolve_path(id);
    if (path.empty()) {
        return std::nullopt;
    }
    std::error_code ec;
    if (!fs::exists(path, ec) || ec) {
        return std::nullopt;
    }
    const std::uintmax_t size = fs::file_size(path, ec);
    if (ec) {
        return std::nullopt;
    }
    const auto ftime = fs::last_write_time(path, ec);
    std::int64_t mtime_epoch = 0;
    if (!ec) {
        // file_time_type -> system_clock -> epoch em SEGUNDOS (unidade cravada no
        // header). Conversao via offset entre relogios (idioma portavel):
        // MSVC/std:c++20 nao expoe file_clock::to_sys para o clock interno de
        // std::filesystem::file_time_type, mas este idioma compila igual em
        // GCC/Clang/MSVC e produz o mesmo instante.
        const auto sys_time = std::chrono::system_clock::now()
                             + (ftime - fs::file_time_type::clock::now());
        mtime_epoch = std::chrono::duration_cast<std::chrono::seconds>(
                          sys_time.time_since_epoch())
                          .count();
    }
    AssetInfo info;
    info.size = static_cast<std::uint64_t>(size);
    info.mtime = mtime_epoch;
    return info;
}

}  // namespace gus::platform::assets

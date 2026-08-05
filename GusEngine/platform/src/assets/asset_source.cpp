// SPDX-License-Identifier: Apache-2.0
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

// ASSETS-PATH-CASCATA: os 2 ULTIMOS niveis da cadeia (`macro de compilacao` > `relativo
// ao CWD`), com a MESMA regra que resolve_font (abaixo) ja aplicava: so aceita o candidato
// do macro se ele EXISTIR no disco; senao desce pro relativo; e se nenhum existir, devolve
// o "melhor chute" pro erro sair legivel no log.
//
// POR QUE ISTO IMPORTA: o macro carrega o caminho ABSOLUTO DA MAQUINA DE BUILD
// (GUSWORLD_ASSETS_DIR="${CMAKE_SOURCE_DIR}/../resources" e irmas, platform/CMakeLists.txt).
// Aceita-lo sem checar significa que um binario compilado aqui e copiado pra outra maquina
// (o caso EXATO do AppImage, RELEASE-DEMO-APPIMAGE) procura em /home/<outro-usuario>/... e
// NUNCA olha o `resources/` que esta ao lado dele. Seis familias faziam isso; so a de
// FONTES ja estava certa - dai ela ser o modelo, e nao ter sido tocada.
//
// `compiled_candidate` vazio = a macro da familia esta vazia (build sem -D): a cadeia
// degrada pro relativo, que era exatamente o comportamento anterior nesse caso.
//
// A ENV NAO PASSA POR AQUI, de proposito: nas seis familias ela e decisao consciente de
// quem roda o jogo e continua vencendo SEM checagem de existencia, com a semantica propria
// de cada uma (override LITERAL em I18N/DIALOGUES; PASTA em SFX/MUSICA/MAPAS/GENERICA).
// Ver os TEST_CASE "env ... vence mesmo inexistente" em platform/tests/asset_source_test.cpp.
std::string compiled_or_cwd(const std::string& compiled_candidate,
                            const std::string& cwd_relative) {
    if (exists_on_disk(compiled_candidate)) {
        return compiled_candidate;
    }
    if (exists_on_disk(cwd_relative)) {
        return cwd_relative;
    }
    return compiled_candidate.empty() ? cwd_relative : compiled_candidate;
}

constexpr std::string_view kFontsPrefix = "assets/fonts/";
// M8 decommission: prefixos migraram de game/translations/ e game/dialogues/ pra
// resources/translations/ e resources/dialogues/ (git mv preservando historico).
constexpr std::string_view kTranslationsPrefix = "resources/translations/";
constexpr std::string_view kDialoguesPrefix = "resources/dialogues/";
constexpr std::string_view kSfxPrefix = "assets/sfx/";
constexpr std::string_view kMusicPrefix = "assets/music/";
constexpr std::string_view kMapsPrefix = "assets/maps/compiled/";

bool starts_with(std::string_view s, std::string_view prefix) {
    return s.size() >= prefix.size() && s.substr(0, prefix.size()) == prefix;
}

// --- FAMILIA FONTES (paridade com font_atlas::resolve_font_path) ------------------
// Unica familia que checa EXISTENCIA TAMBEM NA ENV: env nao pode "sequestrar" a fonte pra
// uma pasta sem o arquivo (comentario original preservado). Nas demais familias a env e
// override consciente e vence sem checagem - so os 2 niveis de baixo passam por
// compiled_or_cwd (ASSETS-PATH-CASCATA). Esta funcao e o MODELO que as outras seis
// passaram a seguir; mexer aqui exige manter as duas propriedades (checar exists na env,
// e nunca lancar).
//
// ASSETS-FONTE-TELAS-GEMEO (2026-08-05): esta familia passou a honrar DUAS envs, nesta
// PRECEDENCIA, do mais ESPECIFICO pro mais GENERICO:
//   1. GUSWORLD_FONTS  - a pasta DAS FONTES: o .ttf mora DIRETO nela (<env>/<arquivo>).
//   2. GUSWORLD_ASSETS - a RAIZ de assets: o .ttf mora em <env>/fonts/<arquivo>.
//   3. macro GUSWORLD_FONTS_DIR + <arquivo>  (caminho absoluto da maquina de BUILD).
//   4. relativo ao CWD (o id como esta).
// POR QUE A PRECEDENCIA E ESSA: quem seta GUSWORLD_FONTS esta apontando a pasta de fonte
// NOMINALMENTE, uma decisao mais estreita e mais recente que "a raiz de assets inteira";
// deixar a generica ganhar da especifica tornaria GUSWORLD_FONTS inutil pra quem tem as
// duas setadas. Nao ha regressao pra quem so usa GUSWORLD_ASSETS: sem GUSWORLD_FONTS no
// ambiente, o nivel 1 e pulado e a cadeia e byte-a-byte a de antes.
// POR QUE GUSWORLD_FONTS EXISTE AQUI: ate esta fatia, SEIS telas de UI (title/system/
// save-load/difficulty/npc-dialogue/cockpit) liam a macro GUSWORLD_FONTS_DIR e a env
// GUSWORLD_FONTS DIRETO, por FORA deste porteiro, pra copiar os 2 .ttf pro stage do
// glintfx - sem checar existencia, sem cascata, e com o std::error_code do copy_file
// IGNORADO (numa maquina sem a macro, a interface ficava sem fonte EM SILENCIO). As telas
// agora delegam pra ca (gus::app::screens::stage_ui_fonts); se GUSWORLD_FONTS nao
// chegasse ate este resolver, quem ja a usava PERDERIA o override - regressao silenciosa.
// A regra do nao-sequestro vale para as DUAS envs (so vencem se o arquivo EXISTIR la),
// senao apontar pra uma pasta vazia deixaria a UI sem fonte em vez de degradar pro nivel
// de baixo. Travado por 6 TEST_CASE "FONTES/GEMEO" em platform/tests/asset_source_test.cpp.
std::string resolve_font(std::string_view id) {
    const std::string filename = strip_prefix(id, kFontsPrefix);
    const std::string env_fonts = env_or_empty("GUSWORLD_FONTS");
    if (!env_fonts.empty()) {
        const std::string cand = join(env_fonts, filename);
        if (exists_on_disk(cand)) {
            return cand;
        }
    }
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
// env e OVERRIDE LITERAL (ignora o id por completo) - unica familia assim junto de
// DIALOGUES, pois so existe 1 catalogo hoje e o lider aponta o .md inteiro. A env nao
// passa por checagem de exists (decisao consciente de quem roda vence); os 2 niveis
// abaixo dela sim, via compiled_or_cwd (ASSETS-PATH-CASCATA).
std::string resolve_translations(std::string_view id) {
    const std::string env = env_or_empty("GUSWORLD_TRANSLATIONS");
    if (!env.empty()) {
        return env;
    }
    const std::string filename = strip_prefix(id, kTranslationsPrefix);
    const std::string compiled_dir = std::string(GUSWORLD_TRANSLATIONS_DIR);
    const std::string compiled =
        compiled_dir.empty() ? std::string() : join(compiled_dir, filename);
    return compiled_or_cwd(compiled, std::string(id));
}

// --- FAMILIA DIALOGUES (paridade com resolve_npc_intro_bertoldo_dialogue_path,
// npc_dialogue_catalog.cpp, M7-DIALOGO/ASSETS-VFS-F1b) -----------------------------
// env e OVERRIDE LITERAL (ignora o id por completo) - MESMO padrao da familia I18N
// (so existe 1 grafo hoje, o lider aponta o .dlg.txt inteiro), inclusive em nao checar
// exists na env; os 2 niveis abaixo passam por compiled_or_cwd (ASSETS-PATH-CASCATA).
std::string resolve_dialogues(std::string_view id) {
    const std::string env = env_or_empty("GUSWORLD_DIALOGUES");
    if (!env.empty()) {
        return env;
    }
    const std::string filename = strip_prefix(id, kDialoguesPrefix);
    const std::string compiled_dir = std::string(GUSWORLD_DIALOGUES_DIR);
    const std::string compiled =
        compiled_dir.empty() ? std::string() : join(compiled_dir, filename);
    return compiled_or_cwd(compiled, std::string(id));
}

// --- FAMILIA SFX (paridade com resolve_hit_sfx_path/resolve_ui_sfx_path/
// resolve_menu_sfx_path) -----------------------------------------------------------
std::string resolve_sfx(std::string_view id) {
    const std::string filename = strip_prefix(id, kSfxPrefix);
    const std::string env = env_or_empty("GUSWORLD_SFX");
    if (!env.empty()) {
        return join(env, filename);
    }
    const std::string compiled_dir = std::string(GUSWORLD_SFX_DIR);
    const std::string compiled =
        compiled_dir.empty() ? std::string() : join(compiled_dir, filename);
    return compiled_or_cwd(compiled, std::string(id));
}

// --- FAMILIA MUSICA (paridade com resolve_music_path) -----------------------------
std::string resolve_music(std::string_view id) {
    const std::string filename = strip_prefix(id, kMusicPrefix);
    const std::string env = env_or_empty("GUSWORLD_MUSIC");
    if (!env.empty()) {
        return join(env, filename);
    }
    const std::string compiled_dir = std::string(GUSWORLD_MUSIC_DIR);
    const std::string compiled =
        compiled_dir.empty() ? std::string() : join(compiled_dir, filename);
    return compiled_or_cwd(compiled, std::string(id));
}

// --- FAMILIA MAPAS (paridade com city_scene.cpp::resolve_distritos_inferiores_gmap,
// ASSETS-VFS-F1c) -------------------------------------------------------------------
// MESMO padrao de SFX/MUSICA: env e uma PASTA (juntada so ao NOME do arquivo, via
// strip_prefix), NAO um override literal do id inteiro (diferente de I18N/DIALOGUES).
// A env segue sem checagem de exists (paridade com o resolver original); o macro e o
// relativo passam por compiled_or_cwd (ASSETS-PATH-CASCATA).
std::string resolve_maps(std::string_view id) {
    const std::string filename = strip_prefix(id, kMapsPrefix);
    const std::string env = env_or_empty("GUSWORLD_MAPS");
    if (!env.empty()) {
        return join(env, filename);
    }
    const std::string compiled_dir = std::string(GUSWORLD_MAPS_DIR);
    const std::string compiled =
        compiled_dir.empty() ? std::string() : join(compiled_dir, filename);
    return compiled_or_cwd(compiled, std::string(id));
}

// --- FAMILIA GENERICA (paridade com resolve_sprites_dir/resolve_gus_sprites_dir/
// resolve_assets_subdir_local/resolve_asset_dir) - sprites/images/vfx sob resources/. ---
std::string resolve_generic(std::string_view id) {
    const std::string env = env_or_empty("GUSWORLD_ASSETS");
    if (!env.empty()) {
        return join(env, std::string(id));
    }
    const std::string compiled_dir = std::string(GUSWORLD_ASSETS_DIR);
    const std::string compiled =
        compiled_dir.empty() ? std::string() : join(compiled_dir, std::string(id));
    // Unica familia cujo 3o nivel NAO e o id cru: a raiz relativa e "resources/".
    return compiled_or_cwd(compiled, join("resources", std::string(id)));
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

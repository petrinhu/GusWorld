// gus/app/src/screens/battle_assets.cpp
//
// Ver header. AC-E11 A3: implementacao dos resolvers/loaders de asset extraidos de
// battle_preview.cpp (ADR-019). Corpo das funcoes MOVIDO verbatim - nenhuma logica mudou,
// so o arquivo que a hospeda.

#include "gus/app/screens/battle_assets.hpp"

#include <cstdlib>  // std::getenv
#include <filesystem>

#include "gus/platform/assets/asset_source.hpp"  // ASSETS-VFS-F1 (ADR-013): porteiro

namespace gus::app::screens {

std::string join(const std::string& a, const std::string& b) {
    if (a.empty()) {
        return b;
    }
    if (a.back() == '/') {
        return a + b;
    }
    return a + "/" + b;
}

// Resolve um caminho RELATIVO de asset (do header central) - familia GENERICA (sprites/
// images/vfx sob resources/). ASSETS-VFS-F1 (ADR-013): CONSOLIDADO em
// FilesystemAssetSource::resolve_path (o `rel` JA E o id logico, sem prefixo de familia
// especifica - cai no dispatch generico). Assinatura INTOCADA.
std::string resolve_asset_dir(std::string_view rel) {
    return gus::platform::assets::FilesystemAssetSource().resolve_path(rel);
}

// Resolve o caminho do SFX de hit (M6 F3, ADR-011). ASSETS-VFS-F1: a cadeia `env
// GUSWORLD_SFX > macro GUSWORLD_SFX_DIR > CWD (kSfxDir)` foi CONSOLIDADA em
// FilesystemAssetSource (familia SFX, dispatch pelo prefixo "assets/sfx/"). O A/B
// GUSWORLD_HIT_SFX=alt continua decidido AQUI (e escolha de QUAL arquivo/id, nao de
// resolucao de raiz) - qualquer outro valor (ou ausente) usa o principal.
std::string resolve_hit_sfx_path() {
    const bool alt = [] {
        const char* e = std::getenv("GUSWORLD_HIT_SFX");
        return e != nullptr && std::string(e) == "alt";
    }();
    const std::string file = alt ? std::string(gus::core::assets::kHitSfxAltFile)
                                  : std::string(gus::core::assets::kHitSfxFile);
    const std::string id = join(std::string(gus::core::assets::kSfxDir), file);
    return gus::platform::assets::FilesystemAssetSource().resolve_path(id);
}

// COCKPIT-SFX-HOVER-CLIQUE: resolve o caminho de um SFX de UI (hover/click) - MESMA
// familia SFX de resolve_hit_sfx_path (agora ambos delegam pro MESMO porteiro, ASSETS-
// VFS-F1 elimina a duplicacao que o comentario anterior descrevia como "minima"), so que
// o NOME do arquivo vem por parametro (os 2 blips do menu: kMenuHoverSfxFile/
// kMenuClickSfxFile). REUSA os MESMOS 2 arquivos do menu de sistema (mesma identidade
// sonora de UI - pedido do lider ao vivo, nao gerar novos).
std::string resolve_ui_sfx_path(std::string_view file) {
    const std::string id = join(std::string(gus::core::assets::kSfxDir), std::string(file));
    return gus::platform::assets::FilesystemAssetSource().resolve_path(id);
}

// Carrega o SPRITE SET de batalha do GUS (W3): pra cada clip conhecido
// (battle_sprite_anim::clip_dir_name), varre <resources>/<kGusBattleAnimsDir>/<clip>/
// f0.png..fN.png em ordem e resolve cada frame em TextureId. fps/loop = defaults do
// modulo POCO; clip_frame_cap TRUNCA a carga (attack_melee_east: so f0..f5 entram -
// f6-f8 derivam e nunca chegam na memoria/render; decisao 2026-07-01, ver
// battle_sprite_anim.hpp). Devolve nullopt se NENHUM frame existir no disco
// (headless/CI sem assets) - a cena entao degrada pro retrato placeholder de hoje.
// So o Gus nesta onda; os demais atores ganham set quando as anims deles existirem
// (PixelLab). Clipes de perfil (run_east/run_west/attack_melee_east) entram pelo
// MESMO laco (o enum e a fonte); ausencia degrada via clip_fallback na cena.
std::optional<ActorSpriteSet> load_gus_sprite_set(
    gus::platform::render2d::IRenderer& renderer) {
    namespace fs = std::filesystem;
    const std::string base =
        resolve_asset_dir(gus::core::assets::kGusBattleAnimsDir);
    ActorSpriteSet set;
    bool any = false;
    for (int c = 0; c < kBattleClipCount; ++c) {
        const auto id = static_cast<BattleClipId>(c);
        const std::string dir = join(base, std::string(clip_dir_name(id)));
        auto& clip = set.clips[static_cast<std::size_t>(c)];
        clip.fps = default_clip_fps(id);
        clip.loop = default_clip_loop(id);
        const int cap = clip_frame_cap(id);
        for (int i = 0;; ++i) {
            if (cap > 0 && i >= cap) {
                break;  // frames alem do cap sao DERIVADOS: nao carregam (nem renderizam)
            }
            const std::string path = join(dir, "f" + std::to_string(i) + ".png");
            std::error_code ec;
            if (!fs::exists(path, ec)) {
                break;  // fim da sequencia (frames sao contiguos f0..fN)
            }
            const gus::platform::render2d::TextureId tex =
                renderer.load_texture(path.c_str());
            if (tex == gus::platform::render2d::kInvalidTexture) {
                break;  // backend sem textura (Null/headless): degrada
            }
            clip.frames.push_back(tex);
            any = true;
        }
    }
    if (!any) {
        return std::nullopt;
    }
    return set;
}

// Mapeia o id de ator de DEMO -> arquivo de retrato 48px. Os inimigos (inimigoN)
// compartilham retrato_inimigo. Ponto unico; quando os retratos forem por-personagem
// reais, troca-se aqui (ou vira data-driven).
std::string retrato_file_for(const std::string& actor_id) {
    // Gus na BATALHA usa o retrato de COMBATE (meio corpo do sprite de jogo: cabelo
    // revolto + oculos taticos + aparelho + antena + casaco tatico). O retrato_gus.png
    // (terno/formal) NAO entra na luta (vira quadro na casa dos pais / narracoes).
    if (actor_id == "gus") return "retrato_gus_combate.png";
    if (actor_id == "caua") return "retrato_caua.png";
    if (actor_id == "jaci") return "retrato_jaci.png";
    // inimigo1..4 e qualquer outro -> retrato generico de inimigo.
    return "retrato_inimigo.png";
}

std::string resolve_retratos_dir() {
    return resolve_asset_dir(gus::core::assets::kRetratosDir);
}

std::string resolve_status_icons_dir() {
    return resolve_asset_dir(gus::core::assets::kStatusIconsDir);
}

// M7-COSTURA Inc 2/3: resolve o caminho de uma faixa de MUSICA (M6 F4, ADR-011).
// ASSETS-VFS-F1 (ADR-013): a cadeia `env GUSWORLD_MUSIC > macro GUSWORLD_MUSIC_DIR >
// CWD (kMusicDir)` foi CONSOLIDADA em FilesystemAssetSource (familia MUSICA, dispatch
// pelo prefixo "assets/music/"). EXPORTADA (fora do namespace anonimo original, ver
// header) - a Maestro chama isto direto pra carregar o tema da cidade E o da arena
// (Inc 3, so o nome do arquivo muda) em init(), dona do AudioEngine. Assinatura INTOCADA.
std::string resolve_music_path(std::string_view file) {
    const std::string id =
        join(std::string(gus::core::assets::kMusicDir), std::string(file));
    return gus::platform::assets::FilesystemAssetSource().resolve_path(id);
}

std::string resolve_intent_icons_dir() {
    return resolve_asset_dir(gus::core::assets::kIntentIconsDir);
}

}  // namespace gus::app::screens

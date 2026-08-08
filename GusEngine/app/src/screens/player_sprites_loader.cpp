// SPDX-License-Identifier: Apache-2.0
// gus/app/src/screens/player_sprites_loader.cpp
//
// Ver header. Monta os caminhos dos PNGs (data-driven por SpriteLayout) e os carrega
// via IRenderer. O macro GUSWORLD_ASSETS_DIR (caminho absoluto do repo) e injetado
// pelo CMake.

#include "gus/app/screens/player_sprites_loader.hpp"

#include <array>
#include <filesystem>
#include <iostream>

#include "gus/app/screens/sprite_anchor.hpp"  // bottom_margin_fraction
#include "gus/core/asset_paths.hpp"           // caminhos de asset centralizados
#include "gus/platform/assets/asset_source.hpp"  // ASSETS-VFS-F1 (ADR-013): porteiro

namespace gus::app::screens {

namespace {

std::string join(const std::string& a, const std::string& b) {
    if (a.empty()) {
        return b;
    }
    if (a.back() == '/') {
        return a + b;
    }
    return a + "/" + b;
}

// Clampa uma contagem pedida ao teto do array (defensivo contra layout exagerado).
int clamp_count(int wanted, int ceil_count) noexcept {
    if (wanted < 1) return 1;
    return wanted > ceil_count ? ceil_count : wanted;
}

}  // namespace

SpriteLayout gus_layout() noexcept {
    SpriteLayout l;
    l.subdir = "gus";
    l.walk_frames = 7;
    l.walk_prefix = "f";
    // BUG-FIX (lider 2026-06-23): a arte do Gus (generate-8-rotations) veio com leste e
    // oeste TROCADOS na fonte. Slot Leste (enum 2) -> pasta "west" (perfil pra direita =
    // +X); slot Oeste (enum 3) -> pasta "east" (perfil pra esquerda = -X). Sul/Norte
    // ficam diretos. Correcao ESPECIFICA do Gus, sem mexer no input/facing global.
    l.walk_dir_names = {"south", "north", "west", "east"};
    l.idle_frames = 5;
    l.idle_dir = "anims/breathing_idle";
    l.idle_prefix = "f";
    return l;
}

SpriteLayout caua_layout() noexcept {
    SpriteLayout l;
    // ASSETS-VERSIONAR-SPRITES (2026-08-06, decisao do lider): os 68x68 antigos
    // (sprites/caua_volt/) perderam east/north/west.png num acidente sem backup; em
    // vez de tentar recupera-los, o Caua passou a apontar pra arte CIANO
    // (sprites/caua_volt_cyan_v2/, 180x180, 4 direcoes + 24 quadros de walk).
    //
    // REVERTIDO 2026-08-08 (decisao do lider): east/north/west.png + walk/ completo
    // recuperados de uma copia propria em "caua_volt/" - volta a ser o subdir ativo.
    // A ESTRUTURA de walk/ e IDENTICA entre as duas pastas (flat,
    // walk_<dir>_<f>.png, 6 quadros x 4 direcoes), entao nenhum outro campo do
    // layout mudou. "caua_volt_cyan_v2/" continua no disco, so parou de ser lida.
    l.subdir = "caua_volt";
    l.walk_frames = 6;              // 0..5 (24 quadros / 4 direcoes)
    l.walk_prefix = "walk_";        // <base>/walk/walk_<dir>_<f>.png
    l.walk_dir_subfolder = false;   // pasta walk/ PLANA (export do gerador), nao <dir>/
    // idle_dir/idle_frames/idle_prefix ficam no default (breathing_idle/5/"f"): a
    // pasta "anims/" existe mas esta VAZIA no disco do Caua, entao a sondagem do
    // loader falha nas 4 direcoes e cai pro walk f0 congelado - comportamento
    // identico ao de qualquer direcao do Gus sem breathing proprio.
    return l;
}

PlayerSpriteSet load_player_sprites(gus::platform::render2d::IRenderer& renderer,
                                    const std::string& base_dir,
                                    const SpriteLayout& layout) {
    PlayerSpriteSet set;

    const int walk_n = clamp_count(layout.walk_frames, kMaxWalkFrameCount);
    const int idle_n = clamp_count(layout.idle_frames, kMaxIdleFrameCount);

    for (int d = 0; d < kDirectionCount; ++d) {
        // --- WALK ---
        // Carregado ANTES do idle: o fix do BUG 1 (idle direcional) usa o walk f0
        // DAQUELA direcao como idle das direcoes sem breathing proprio.
        // A subpasta/direcao vem do layout (data-driven): default {south,north,east,
        // west}; o Gus troca leste<->oeste pra corrigir o rotulo invertido da fonte.
        //
        // DOIS layouts de PASTA possiveis (SpriteLayout::walk_dir_subfolder,
        // ASSETS-VERSIONAR-SPRITES 2026-08-06 - campo de DADO, nao if por personagem):
        //   true  (default, Gus): <base>/walk/<dir>/<pref><f>.png - subpasta por direcao.
        //   false (Caua, export flat do gerador): <base>/walk/<pref><dir>_<f>.png -
        //         pasta unica, direcao e frame no NOME do arquivo.
        set.walk_count[d] = walk_n;
        const std::string& dir_name = layout.walk_dir_names[static_cast<std::size_t>(d)];
        for (int f = 0; f < walk_n; ++f) {
            const std::string walk_file_name =
                layout.walk_dir_subfolder
                    ? std::string(layout.walk_prefix) + std::to_string(f) + ".png"
                    : std::string(layout.walk_prefix) + dir_name + "_" +
                          std::to_string(f) + ".png";
            const std::string walk_path =
                layout.walk_dir_subfolder
                    ? join(join(join(base_dir, "walk"), dir_name), walk_file_name)
                    : join(join(base_dir, "walk"), walk_file_name);
            const auto tex = renderer.load_texture(walk_path.c_str());
            if (tex == gus::platform::render2d::kInvalidTexture) {
                std::cerr << "player_sprites: sprite ausente/ilegivel: " << walk_path
                          << " (render segue com textura invalida)\n";
            }
            set.walk[d][f] = tex;
        }

        // --- IDLE (breathing animado POR DIRECAO, com fallback gracioso) ---
        // ARTE-RESP-4DIR (2026-07-23): o breathing tem pasta POR DIRECAO em
        // <base>/<idle_dir>/<dir>/<pref>f.png, reusando walk_dir_names[d] (MESMA
        // correcao leste/oeste da fonte que o walk usa). GRACIOSO: primeiro sonda
        // (std::filesystem::exists, sem tocar o renderer) se o quadro 0 daquela
        // direcao existe no disco. Existindo, carrega os idle_n quadros normalmente.
        // Faltando (personagem sem breathing pra este lado - todo NPC comum, os
        // companions incluindo o Caua ate ganharem a arte deles, e por ora QUALQUER
        // direcao sem a pasta), NAO tenta carregar (nem loga erro: ausencia esperada,
        // nao falha) e cai pro walk f0 congelado da mesma direcao. ASSETS-VERSIONAR-
        // SPRITES (2026-08-06): este e o UNICO comportamento de idle do loader agora -
        // o antigo ramo "idle congelado direcional" (<base>/<dir>.png), exclusivo do
        // Caua antigo e o unico condicional por-personagem do arquivo, foi REMOVIDO.
        const std::string idle_dir_path = join(join(base_dir, layout.idle_dir), dir_name);
        const std::string idle_f0_path =
            join(idle_dir_path, std::string(layout.idle_prefix) + "0.png");
        std::error_code ec;
        const bool has_directional_breathing =
            std::filesystem::exists(idle_f0_path, ec) && !ec;

        if (has_directional_breathing) {
            set.idle_count[d] = idle_n;
            for (int f = 0; f < idle_n; ++f) {
                const std::string p =
                    join(idle_dir_path,
                         std::string(layout.idle_prefix) + std::to_string(f) + ".png");
                const auto tex = renderer.load_texture(p.c_str());
                if (tex == gus::platform::render2d::kInvalidTexture) {
                    std::cerr << "player_sprites: sprite ausente/ilegivel: " << p
                              << " (render segue com textura invalida)\n";
                }
                set.idle_frames[d][f] = tex;
            }
            set.idle[d] = set.idle_frames[d][0];  // representativo = quadro 0
        } else {
            // Direcao sem breathing proprio: idle de 1 quadro = walk f0 daquela
            // direcao (arte direcional que ja existe), preservando o facing parado.
            set.idle[d] = set.walk[d][0];
            set.idle_frames[d][0] = set.walk[d][0];
            set.idle_count[d] = 1;
        }

        // ANCORAGEM AUTOMATICA (M1-BUG.SUL): mede a margem inferior TRANSPARENTE do
        // sprite IDLE REPRESENTATIVO desta direcao (alpha-bbox do PNG decodificado no
        // load) e a guarda como FRACAO do canvas. O render desce o desenho por isso pra
        // COLAR o pe na base da AABB - sem numero magico, por personagem/direcao.
        // Headless (bbox invalido) => bottom_margin() == 0 => fracao 0 => anchor legado.
        const gus::platform::render2d::ContentBbox bbox =
            renderer.texture_content_bbox(set.idle[d]);
        set.foot.bottom_fraction[d] =
            bottom_margin_fraction(bbox.bottom_margin(), bbox.canvas_h);
    }
    return set;
}

std::string resolve_sprites_dir(const std::string& rel_subpath) {
    // rel_subpath = caminho RELATIVO completo do header central (ex.: "sprites/caua_volt_cyan_v2").
    // ASSETS-VFS-F1 (ADR-013): a cadeia `env GUSWORLD_ASSETS > macro GUSWORLD_ASSETS_DIR >
    // CWD (resources/)` foi CONSOLIDADA em FilesystemAssetSource (familia GENERICA).
    // Assinatura INTOCADA.
    return gus::platform::assets::FilesystemAssetSource().resolve_path(rel_subpath);
}

// --- atalhos ----------------------------------------------------------------

PlayerSpriteSet load_gus_sprites(gus::platform::render2d::IRenderer& renderer,
                                 const std::string& base_dir) {
    return load_player_sprites(renderer, base_dir, gus_layout());
}
// resolve_gus_sprites_dir() NAO e definido aqui: ja existe em anim_catalog.cpp (sub-caminho
// do header central, kGusSpritesDir). Reusado pelo main/sdl_window via anim_catalog.hpp.

PlayerSpriteSet load_caua_sprites(gus::platform::render2d::IRenderer& renderer,
                                  const std::string& base_dir) {
    return load_player_sprites(renderer, base_dir, caua_layout());
}
std::string resolve_caua_sprites_dir() {
    return resolve_sprites_dir(std::string(gus::core::assets::kCauaSpritesDir));
}

}  // namespace gus::app::screens

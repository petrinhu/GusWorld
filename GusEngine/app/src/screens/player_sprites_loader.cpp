// gus/app/src/screens/player_sprites_loader.cpp
//
// Ver header. Monta os caminhos dos PNGs (data-driven por SpriteLayout) e os carrega
// via IRenderer. O macro GUSWORLD_ASSETS_DIR (caminho absoluto do repo) e injetado
// pelo CMake.

#include "gus/app/screens/player_sprites_loader.hpp"

#include <array>

#include "gus/app/screens/sprite_anchor.hpp"  // bottom_margin_fraction
#include "gus/core/asset_paths.hpp"           // caminhos de asset centralizados
#include "gus/platform/assets/asset_source.hpp"  // ASSETS-VFS-F1 (ADR-013): porteiro

namespace gus::app::screens {

namespace {

// Nomes do idle CONGELADO direcional (Caua), na mesma ordem do enum Direction.
constexpr std::array<const char*, kDirectionCount> kIdleFilesCaua = {
    "south.png", "north.png", "east.png", "west.png"};

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
    l.idle_animated = true;
    l.idle_frames = 5;
    l.idle_dir = "anims/breathing_idle";
    l.idle_prefix = "f";
    return l;
}

SpriteLayout caua_layout() noexcept {
    SpriteLayout l;
    l.subdir = "caua_volt";
    l.walk_frames = 4;
    l.walk_prefix = "";  // 0.png..3.png
    l.idle_animated = false;  // idle congelado direcional
    l.idle_frames = 1;
    return l;
}

PlayerSpriteSet load_player_sprites(gus::platform::render2d::IRenderer& renderer,
                                    const std::string& base_dir,
                                    const SpriteLayout& layout) {
    PlayerSpriteSet set;

    const int walk_n = clamp_count(layout.walk_frames, kMaxWalkFrameCount);
    const int idle_n = clamp_count(layout.idle_frames, kMaxIdleFrameCount);

    // IDLE ANIMADO NAO-DIRECIONAL (Gus): um unico loop em <base>/<idle_dir>/<pref>i.png,
    // reusado nas 4 direcoes. Carrega uma vez e replica os handles por direcao.
    std::array<gus::platform::render2d::TextureId, kMaxIdleFrameCount> shared_idle{};
    if (layout.idle_animated) {
        for (int f = 0; f < idle_n; ++f) {
            const std::string p =
                join(join(base_dir, layout.idle_dir),
                     std::string(layout.idle_prefix) + std::to_string(f) + ".png");
            shared_idle[static_cast<std::size_t>(f)] = renderer.load_texture(p.c_str());
        }
    }

    for (int d = 0; d < kDirectionCount; ++d) {
        // --- WALK: <base>/walk/<dir>/<pref>f.png  (f = 0..walk_n-1) ---
        // Carregado ANTES do idle: o fix do BUG 1 (idle direcional) usa o walk f0
        // DAQUELA direcao como idle das direcoes sem breathing proprio.
        // A subpasta vem do layout (data-driven): default {south,north,east,west};
        // o Gus troca leste<->oeste pra corrigir o rotulo invertido da fonte.
        set.walk_count[d] = walk_n;
        for (int f = 0; f < walk_n; ++f) {
            const std::string walk_path =
                join(join(join(base_dir, "walk"),
                          layout.walk_dir_names[static_cast<std::size_t>(d)]),
                     std::string(layout.walk_prefix) + std::to_string(f) + ".png");
            set.walk[d][f] = renderer.load_texture(walk_path.c_str());
        }

        // --- IDLE (breathing animado OU congelado direcional) ---
        // BUG 1 (lider 2026-06-23): o breathing animado so existe de FRENTE. Quando
        // idle_animated_only_one_facing, ele e usado SO na direcao idle_animated_facing
        // (Sul); as OUTRAS direcoes recebem o walk f0 DAQUELA direcao como idle de 1
        // quadro - assim parar olhando Norte/Leste/Oeste NAO mostra a pose de frente.
        const bool use_breathing_here =
            layout.idle_animated &&
            (!layout.idle_animated_only_one_facing ||
             d == static_cast<int>(layout.idle_animated_facing));
        if (use_breathing_here) {
            set.idle_count[d] = idle_n;
            for (int f = 0; f < idle_n; ++f) {
                set.idle_frames[d][f] = shared_idle[static_cast<std::size_t>(f)];
            }
            set.idle[d] = shared_idle[0];  // representativo = quadro 0 do breathing
        } else if (layout.idle_animated) {
            // Direcao sem breathing proprio: idle de 1 quadro = walk f0 daquela direcao
            // (arte direcional que ja existe), preservando o facing parado.
            set.idle[d] = set.walk[d][0];
            set.idle_frames[d][0] = set.walk[d][0];
            set.idle_count[d] = 1;
        } else {
            // Caua: idle congelado direcional, 1 quadro por direcao (<base>/<dir>.png).
            const std::string idle_path = join(base_dir, kIdleFilesCaua[d]);
            const gus::platform::render2d::TextureId t =
                renderer.load_texture(idle_path.c_str());
            set.idle[d] = t;
            set.idle_frames[d][0] = t;  // 1 quadro = congelado
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
    // rel_subpath = caminho RELATIVO completo do header central (ex.: "sprites/caua_volt").
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

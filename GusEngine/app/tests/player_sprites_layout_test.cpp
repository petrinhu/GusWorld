// GusEngine/app/tests/player_sprites_layout_test.cpp
//
// Catch2 do MAPEAMENTO DE DIRECAO -> subpasta de walk dos layouts de personagem
// (app/screens/player_sprites_loader). Dados PUROS (SpriteLayout): NAO toca filesystem,
// renderer nem SDL. Trava o bug-fix do lider (2026-06-23): a arte do Gus veio com
// leste/oeste TROCADOS na fonte (generate-8-rotations); gus_layout() corrige trocando
// SO esses dois slots, sem mexer no input/facing compartilhado nem no Caua.

#include <catch2/catch_test_macros.hpp>

#include <map>
#include <string>

#include "gus/app/screens/overworld_sim.hpp"  // PlayerSpriteSet
#include "gus/app/screens/player_sprites_loader.hpp"
#include "gus/app/screens/sprite_animation.hpp"
#include "gus/platform/render2d/i_renderer.hpp"

using gus::app::screens::caua_layout;
using gus::app::screens::Direction;
using gus::app::screens::gus_layout;
using gus::app::screens::load_player_sprites;
using gus::app::screens::PlayerSpriteSet;
using gus::app::screens::SpriteLayout;
using gus::platform::render2d::TextureId;

namespace {
const char* walk_dir(const SpriteLayout& l, Direction d) {
    return l.walk_dir_names[static_cast<std::size_t>(d)];
}

// Renderer FALSO que mapeia CADA caminho de arquivo a um TextureId estavel: dois loads
// do mesmo path devolvem o MESMO id; paths diferentes -> ids diferentes. Permite
// asserir QUAL arquivo cada slot de direcao carregou, sem tocar disco/GPU.
class PathRenderer : public gus::platform::render2d::IRenderer {
public:
    void begin_frame(const gus::core::spatial::Rect&, int, int) override {}
    void draw_filled_rect(const gus::core::spatial::Rect&,
                          const gus::platform::render2d::DrawColor&) override {}
    void draw_rect_outline(const gus::core::spatial::Rect&,
                           const gus::platform::render2d::DrawColor&, float) override {}
    void draw_textured_rect(const gus::core::spatial::Rect&, TextureId,
                            const gus::platform::render2d::UvRect&,
                            const gus::platform::render2d::DrawColor&) override {}
    gus::platform::render2d::ContentBbox texture_content_bbox(
        TextureId) const override {
        return gus::platform::render2d::ContentBbox{};
    }
    void draw_text(const char*, float, float, float,
                   const gus::platform::render2d::DrawColor&, bool) override {}
    void end_frame() override {}

    TextureId load_texture(const char* path) override {
        const std::string p = path ? path : "";
        auto it = ids.find(p);
        if (it != ids.end()) return it->second;
        const TextureId id = ++next;
        ids.emplace(p, id);
        return id;
    }
    std::map<std::string, TextureId> ids;
    TextureId next = 0;
};
}  // namespace

TEST_CASE("layout Gus: Leste/Oeste TROCADOS pra corrigir rotulo invertido da fonte",
          "[player_sprites][layout]") {
    const SpriteLayout l = gus_layout();
    // O slot Leste (+X, perfil pra DIREITA) carrega a pasta "west" do Gus, que e a que
    // de fato mostra o perfil pra direita. E o slot Oeste carrega "east".
    CHECK(std::string(walk_dir(l, Direction::East)) == "west");
    CHECK(std::string(walk_dir(l, Direction::West)) == "east");
    // Sul/Norte ficam DIRETOS (o bug era so na horizontal).
    CHECK(std::string(walk_dir(l, Direction::South)) == "south");
    CHECK(std::string(walk_dir(l, Direction::North)) == "north");
}

TEST_CASE("layout Caua: mapeamento DIRETO (arte correta, nao mexer)",
          "[player_sprites][layout]") {
    const SpriteLayout l = caua_layout();
    CHECK(std::string(walk_dir(l, Direction::South)) == "south");
    CHECK(std::string(walk_dir(l, Direction::North)) == "north");
    CHECK(std::string(walk_dir(l, Direction::East)) == "east");
    CHECK(std::string(walk_dir(l, Direction::West)) == "west");
}

TEST_CASE("layout default: mapeamento direto (Caua-like, sem surpresa)",
          "[player_sprites][layout]") {
    const SpriteLayout l;  // default
    CHECK(std::string(walk_dir(l, Direction::East)) == "east");
    CHECK(std::string(walk_dir(l, Direction::West)) == "west");
}

// --- BUG 1 (lider 2026-06-23): idle DIRECIONAL (nao "sempre Sul") ------------

TEST_CASE(
    "loader Gus: o IDLE de cada direcao usa arte DAQUELA direcao (nao a do Sul)",
    "[player_sprites][loader][facing]") {
    // RAIZ VISUAL do bug do Gus virar pra Sul ao parar: a arte de respiracao
    // (anims/breathing_idle) so existe de FRENTE (Sul). Antes o loader replicava esse
    // mesmo loop nas 4 direcoes -> parado, o Gus sempre parecia olhar pra baixo. Fix:
    // South mantem o breathing animado; Norte/Leste/Oeste usam o walk f0 DAQUELA direcao
    // (arte que ja existe), preservando o facing visualmente sem arte nova.
    PathRenderer r;
    const PlayerSpriteSet s = load_player_sprites(r, "BASE", gus_layout());

    const int south = static_cast<int>(Direction::South);
    const int north = static_cast<int>(Direction::North);
    const int east = static_cast<int>(Direction::East);
    const int west = static_cast<int>(Direction::West);

    // Sul: mantem o breathing animado (5 quadros), todos != invalido.
    REQUIRE(s.idle_count[south] >= 1);
    // O quadro representativo de cada direcao tem que DIFERIR do Sul (idle direcional).
    REQUIRE(s.idle[north] != s.idle[south]);
    REQUIRE(s.idle[east] != s.idle[south]);
    REQUIRE(s.idle[west] != s.idle[south]);
    // E o idle de Norte/Leste/Oeste e o respectivo walk f0 daquela direcao (arte que
    // existe), nao o breathing do Sul.
    REQUIRE(s.idle[north] == s.walk[north][0]);
    REQUIRE(s.idle[east] == s.walk[east][0]);
    REQUIRE(s.idle[west] == s.walk[west][0]);
    // Leste e Oeste continuam DISTINTOS entre si (perfis opostos).
    REQUIRE(s.idle[east] != s.idle[west]);
}

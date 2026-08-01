// SPDX-License-Identifier: Apache-2.0
// GusEngine/app/tests/player_sprites_layout_test.cpp
//
// Catch2 do MAPEAMENTO DE DIRECAO -> subpasta de walk/idle dos layouts de personagem
// (app/screens/player_sprites_loader). As TEST_CASEs de mapeamento puro (SpriteLayout)
// NAO tocam filesystem/renderer/SDL. Trava o bug-fix do lider (2026-06-23): a arte do
// Gus veio com leste/oeste TROCADOS na fonte (generate-8-rotations); gus_layout()
// corrige trocando SO esses dois slots, sem mexer no input/facing compartilhado nem no
// Caua.
//
// ARTE-RESP-4DIR (2026-07-23): as TEST_CASEs do LOADER (que exercitam
// load_player_sprites com breathing direcional) TOCAM uma arvore temporaria real em
// disco - mesmo padrao de app/tests/anim_catalog_test.cpp (touch_png em
// fs::temp_directory_path()) - porque o loader agora sonda std::filesystem::exists()
// pra decidir, POR DIRECAO, se ha breathing proprio ou se degrada pro walk f0.

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>
#include <map>
#include <string>

#include "gus/app/screens/overworld_sim.hpp"  // PlayerSpriteSet
#include "gus/app/screens/player_sprites_loader.hpp"
#include "gus/app/screens/sprite_animation.hpp"
#include "gus/platform/render2d/i_renderer.hpp"

namespace fs = std::filesystem;

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

// Cria um arquivo vazio (o loader so sonda existencia + delega o load ao renderer
// fake; o conteudo do PNG nunca e decodificado neste teste).
void touch_png(const fs::path& p) {
    fs::create_directories(p.parent_path());
    std::ofstream(p.string()).put('\0');
}

// Raiz temporaria unica por chamada (evita colisao entre TEST_CASEs).
fs::path make_temp_root(const char* tag) {
    static int counter = 0;
    const fs::path root = fs::temp_directory_path() /
                           ("gus_player_sprites_test_" + std::string(tag) + "_" +
                            std::to_string(counter++));
    fs::remove_all(root);
    fs::create_directories(root);
    return root;
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

// --- ARTE-RESP-4DIR (2026-07-23): breathing DIRECIONAL completo + degradacao -------

TEST_CASE(
    "loader Gus: breathing DIRECIONAL completo quando as 4 pastas existem no disco",
    "[player_sprites][loader][facing]") {
    // As 4 pastas fisicas de breathing existem (south/north/east/west - os MESMOS
    // nomes fisicos que walk usa, so que gus_layout() PERMUTA qual enum-direcao le
    // qual pasta). Cada direcao deve carregar o proprio loop de 5 quadros, e o slot
    // Leste deve ler da pasta fisica "west" (mesma correcao de rotulo que o walk).
    const fs::path root = make_temp_root("full4dir");
    for (const char* dir_name : {"south", "north", "east", "west"}) {
        for (int f = 0; f < 5; ++f) {
            touch_png(root / "anims" / "breathing_idle" / dir_name /
                      ("f" + std::to_string(f) + ".png"));
        }
    }

    PathRenderer r;
    const PlayerSpriteSet s = load_player_sprites(r, root.string(), gus_layout());

    const int south = static_cast<int>(Direction::South);
    const int north = static_cast<int>(Direction::North);
    const int east = static_cast<int>(Direction::East);
    const int west = static_cast<int>(Direction::West);

    // Todas as 4 direcoes carregam o loop completo (5 quadros), nao o fallback de 1.
    REQUIRE(s.idle_count[south] == 5);
    REQUIRE(s.idle_count[north] == 5);
    REQUIRE(s.idle_count[east] == 5);
    REQUIRE(s.idle_count[west] == 5);

    // Cada direcao usa arte PROPRIA (nao mais o walk f0 congelado de fallback).
    REQUIRE(s.idle[south] != s.walk[south][0]);
    REQUIRE(s.idle[north] != s.walk[north][0]);
    REQUIRE(s.idle[east] != s.walk[east][0]);
    REQUIRE(s.idle[west] != s.walk[west][0]);

    // Direcoes distintas carregam quadros distintos (sem flip, Pillar 3).
    REQUIRE(s.idle[north] != s.idle[south]);
    REQUIRE(s.idle[east] != s.idle[south]);
    REQUIRE(s.idle[west] != s.idle[south]);
    REQUIRE(s.idle[east] != s.idle[west]);

    // Leste (enum) le da pasta FISICA "west" (mesmo swap do walk); Oeste le de "east".
    const std::string east_f0 = (root / "anims" / "breathing_idle" / "west" / "f0.png").string();
    const std::string west_f0 = (root / "anims" / "breathing_idle" / "east" / "f0.png").string();
    REQUIRE(s.idle[east] == r.load_texture(east_f0.c_str()));
    REQUIRE(s.idle[west] == r.load_texture(west_f0.c_str()));
}

TEST_CASE(
    "loader Gus: direcao SEM pasta de breathing degrada pro walk f0 (graciosa)",
    "[player_sprites][loader][facing][fallback]") {
    // So o Sul tem breathing no disco (cenario real de um personagem que so ganhou a
    // arte de UM lado por enquanto - ex.: os companions ate ganharem a arte deles).
    // Norte, Leste e Oeste NAO tem a pasta - o loader tem que cair pro walk f0 de CADA
    // uma dessas direcoes, sem crash e sem exigir tudo-ou-nada.
    const fs::path root = make_temp_root("partial_south_only");
    for (int f = 0; f < 5; ++f) {
        touch_png(root / "anims" / "breathing_idle" / "south" /
                  ("f" + std::to_string(f) + ".png"));
    }

    PathRenderer r;
    const PlayerSpriteSet s = load_player_sprites(r, root.string(), gus_layout());

    const int south = static_cast<int>(Direction::South);
    const int north = static_cast<int>(Direction::North);
    const int east = static_cast<int>(Direction::East);
    const int west = static_cast<int>(Direction::West);

    // Sul: breathing completo, arte propria (nao o walk f0).
    REQUIRE(s.idle_count[south] == 5);
    REQUIRE(s.idle[south] != s.walk[south][0]);

    // As demais direcoes degradam pro walk f0 CONGELADO daquela direcao (comportamento
    // legado preservado quando falta a arte).
    REQUIRE(s.idle_count[north] == 1);
    REQUIRE(s.idle[north] == s.walk[north][0]);
    REQUIRE(s.idle_count[east] == 1);
    REQUIRE(s.idle[east] == s.walk[east][0]);
    REQUIRE(s.idle_count[west] == 1);
    REQUIRE(s.idle[west] == s.walk[west][0]);
}

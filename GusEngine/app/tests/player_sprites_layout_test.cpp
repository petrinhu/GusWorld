// GusEngine/app/tests/player_sprites_layout_test.cpp
//
// Catch2 do MAPEAMENTO DE DIRECAO -> subpasta de walk dos layouts de personagem
// (app/screens/player_sprites_loader). Dados PUROS (SpriteLayout): NAO toca filesystem,
// renderer nem SDL. Trava o bug-fix do lider (2026-06-23): a arte do Gus veio com
// leste/oeste TROCADOS na fonte (generate-8-rotations); gus_layout() corrige trocando
// SO esses dois slots, sem mexer no input/facing compartilhado nem no Caua.

#include <catch2/catch_test_macros.hpp>

#include <string>

#include "gus/app/screens/player_sprites_loader.hpp"
#include "gus/app/screens/sprite_animation.hpp"

using gus::app::screens::caua_layout;
using gus::app::screens::Direction;
using gus::app::screens::gus_layout;
using gus::app::screens::SpriteLayout;

namespace {
const char* walk_dir(const SpriteLayout& l, Direction d) {
    return l.walk_dir_names[static_cast<std::size_t>(d)];
}
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

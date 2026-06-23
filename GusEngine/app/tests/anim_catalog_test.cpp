// GusEngine/app/tests/anim_catalog_test.cpp
//
// Catch2 da varredura do AnimCatalog (app/screens/anim_catalog). Toca filesystem
// (a unica IO da unidade), mas NAO SDL/GPU: monta uma arvore temporaria de PNGs
// vazios em parametros conhecidos e valida o catalogo resultante.
//
// Cobre o que o viewer depende:
//   - anims/<NOME>/f*.png vira 1 anim por subpasta, rotulada pelo nome;
//   - walk/<dir>/f*.png vira "walk_<dir>";
//   - rotations/<i>_*.png vira a anim sintetica "turntable";
//   - ordenacao NUMERICA dos frames (f0,f1,...,f10 - nao lexicografica);
//   - pasta ausente => catalogo vazio (viewer reporta e sai limpo).

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "gus/app/screens/anim_catalog.hpp"

namespace fs = std::filesystem;

using gus::app::screens::AnimEntry;
using gus::app::screens::build_gus_anim_catalog;

namespace {

void touch_png(const fs::path& p) {
    fs::create_directories(p.parent_path());
    std::ofstream(p.string()).put('\0');  // arquivo vazio basta (catalog so olha o nome)
}

// Cria uma raiz temporaria unica e nela uma arvore de sprites de teste.
fs::path make_tree() {
    static int counter = 0;
    const fs::path root =
        fs::temp_directory_path() /
        ("gus_anim_catalog_test_" + std::to_string(counter++) + "_" +
         std::to_string(reinterpret_cast<std::uintptr_t>(&root)));
    fs::remove_all(root);
    // anims/attack_melee com 11 frames f0..f10 (testa ordenacao numerica).
    for (int i = 0; i <= 10; ++i) {
        touch_png(root / "anims" / "attack_melee" / ("f" + std::to_string(i) + ".png"));
    }
    // anims/cast com 3 frames.
    for (int i = 0; i < 3; ++i) {
        touch_png(root / "anims" / "cast" / ("f" + std::to_string(i) + ".png"));
    }
    // walk/south com 2 frames.
    touch_png(root / "walk" / "south" / "f0.png");
    touch_png(root / "walk" / "south" / "f1.png");
    // rotations: 8 estaticos.
    touch_png(root / "rotations" / "0_south.png");
    touch_png(root / "rotations" / "2_west.png");
    touch_png(root / "rotations" / "1_south-west.png");
    return root;
}

const AnimEntry* find(const std::vector<AnimEntry>& v, const std::string& label) {
    for (const auto& e : v) {
        if (e.label == label) {
            return &e;
        }
    }
    return nullptr;
}

}  // namespace

TEST_CASE("anim_catalog: pasta ausente devolve catalogo vazio", "[anim_catalog]") {
    const auto cat = build_gus_anim_catalog("/caminho/que/nao/existe/__nope__");
    REQUIRE(cat.empty());
}

TEST_CASE("anim_catalog: monta anims/, walk_<dir> e turntable", "[anim_catalog]") {
    const fs::path root = make_tree();
    const auto cat = build_gus_anim_catalog(root.string());

    REQUIRE(find(cat, "attack_melee") != nullptr);
    REQUIRE(find(cat, "cast") != nullptr);
    REQUIRE(find(cat, "walk_south") != nullptr);
    REQUIRE(find(cat, "turntable") != nullptr);

    CHECK(find(cat, "cast")->frames.size() == 3);
    CHECK(find(cat, "walk_south")->frames.size() == 2);
    CHECK(find(cat, "turntable")->frames.size() == 3);

    fs::remove_all(root);
}

TEST_CASE("anim_catalog: ordena frames por indice NUMERICO (f10 depois de f9)",
          "[anim_catalog]") {
    const fs::path root = make_tree();
    const auto cat = build_gus_anim_catalog(root.string());
    const AnimEntry* melee = find(cat, "attack_melee");
    REQUIRE(melee != nullptr);
    REQUIRE(melee->frames.size() == 11);
    // f0 primeiro, f10 por ULTIMO (lexicografico poria f10 antes de f2).
    CHECK(melee->frames.front().find("f0.png") != std::string::npos);
    CHECK(melee->frames.back().find("f10.png") != std::string::npos);
    // O penultimo deve ser f9 (prova ordem numerica, nao textual).
    CHECK(melee->frames[9].find("f9.png") != std::string::npos);

    fs::remove_all(root);
}

TEST_CASE("anim_catalog: turntable ordenado pelo indice da rotacao", "[anim_catalog]") {
    const fs::path root = make_tree();
    const auto cat = build_gus_anim_catalog(root.string());
    const AnimEntry* tt = find(cat, "turntable");
    REQUIRE(tt != nullptr);
    REQUIRE(tt->frames.size() == 3);
    CHECK(tt->frames[0].find("0_south.png") != std::string::npos);
    CHECK(tt->frames[1].find("1_south-west.png") != std::string::npos);
    CHECK(tt->frames[2].find("2_west.png") != std::string::npos);

    fs::remove_all(root);
}

TEST_CASE("anim_catalog: catalogo ordenado por rotulo", "[anim_catalog]") {
    const fs::path root = make_tree();
    const auto cat = build_gus_anim_catalog(root.string());
    for (std::size_t i = 1; i < cat.size(); ++i) {
        CHECK(cat[i - 1].label <= cat[i].label);
    }
    fs::remove_all(root);
}

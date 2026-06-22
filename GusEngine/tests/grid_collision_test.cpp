// GusEngine/tests/grid_collision_test.cpp
//
// Spec executavel (Catch2 v3) da colisao AABB-desliza-na-grade do M4. TEST-FIRST:
// estes casos DEFINEM o contrato.
//
// FEEL DECIDIDO PELO LIDER (nao reabrir): a caixa do personagem (AABB) DESLIZA
// ao longo das paredes. Resolucao SEPARADA POR EIXO (resolve X, depois Y), estilo
// Zelda ALttP / Stardew. Bateu numa parede num eixo, continua se movendo no outro.
// Movimento cinematico puro (sem impulso/fisica), deterministico.
//
// CONTRATO exercitado:
//   - Aabb { x, y (canto superior-esquerdo), w, h } em unidades de mundo;
//   - resolve_move(grid, box, dx, dy) -> MoveResult { Aabb box, bool hit_x, hit_y };
//   - resolve X primeiro (clampa dx contra celulas bloqueadas sobrepostas),
//     depois Y com o X ja resolvido;
//   - sair do mapa = bater em parede (celulas fora dos limites sao bloqueadas).

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "gus/core/spatial/grid_collision.hpp"
#include "gus/core/spatial/tile_grid.hpp"

using gus::core::spatial::Aabb;
using gus::core::spatial::MoveResult;
using gus::core::spatial::resolve_move;
using gus::core::spatial::TileGrid;
using Catch::Matchers::WithinAbs;

namespace {
constexpr float kEps = 1e-4f;
}

TEST_CASE("resolve_move: campo aberto move livre nos dois eixos",
          "[core][spatial][collision]") {
    // Mapa 5x5 todo livre, tile 16. Caixa 8x8 no centro.
    TileGrid g(5, 5, 16.0f);
    Aabb box{32.0f, 32.0f, 8.0f, 8.0f};

    MoveResult r = resolve_move(g, box, 5.0f, -3.0f);
    REQUIRE_THAT(r.box.x, WithinAbs(37.0f, kEps));
    REQUIRE_THAT(r.box.y, WithinAbs(29.0f, kEps));
    REQUIRE_FALSE(r.hit_x);
    REQUIRE_FALSE(r.hit_y);
}

TEST_CASE("resolve_move: parede a direita trava X e encosta na borda",
          "[core][spatial][collision]") {
    // Coluna x=2 bloqueada. Caixa colada a esquerda dela, empurrando para a direita.
    TileGrid g = TileGrid::from_rows({
        "..#..",
        "..#..",
        "..#..",
    }, 16.0f);
    // Celula bloqueada comeca em x=32. Caixa 8 largura em x=20: borda direita 28.
    Aabb box{20.0f, 20.0f, 8.0f, 8.0f};

    MoveResult r = resolve_move(g, box, 20.0f, 0.0f);
    // Deve encostar: borda direita == 32, logo x == 24.
    REQUIRE_THAT(r.box.x, WithinAbs(24.0f, kEps));
    REQUIRE(r.hit_x);
    REQUIRE_FALSE(r.hit_y);
}

TEST_CASE("resolve_move: parede em X nao impede deslizar em Y (FEEL)",
          "[core][spatial][collision]") {
    // Mesma parede vertical x=2. Personagem empurra na diagonal (direita+baixo).
    // X trava na parede, mas Y deve deslizar livre.
    TileGrid g = TileGrid::from_rows({
        "..#..",
        "..#..",
        "..#..",
    }, 16.0f);
    Aabb box{20.0f, 20.0f, 8.0f, 8.0f};

    MoveResult r = resolve_move(g, box, 20.0f, 10.0f);
    REQUIRE_THAT(r.box.x, WithinAbs(24.0f, kEps));  // travou na parede
    REQUIRE_THAT(r.box.y, WithinAbs(30.0f, kEps));  // deslizou em Y
    REQUIRE(r.hit_x);
    REQUIRE_FALSE(r.hit_y);
}

TEST_CASE("resolve_move: parede embaixo trava Y e encosta",
          "[core][spatial][collision]") {
    // Linha y=2 bloqueada. Caixa acima empurrando para baixo.
    TileGrid g = TileGrid::from_rows({
        ".....",
        ".....",
        "#####",
    }, 16.0f);
    Aabb box{20.0f, 20.0f, 8.0f, 8.0f};  // borda inferior em 28

    MoveResult r = resolve_move(g, box, 0.0f, 20.0f);
    // Celula bloqueada comeca em y=32; borda inferior encosta -> y == 24.
    REQUIRE_THAT(r.box.y, WithinAbs(24.0f, kEps));
    REQUIRE_FALSE(r.hit_x);
    REQUIRE(r.hit_y);
}

TEST_CASE("resolve_move: canto interno trava nos dois eixos",
          "[core][spatial][collision]") {
    // Parede em L: coluna x=2 e linha y=2 bloqueadas. Diagonal direita+baixo
    // bate nos dois.
    TileGrid g = TileGrid::from_rows({
        "..#..",
        "..#..",
        "#####",
    }, 16.0f);
    Aabb box{20.0f, 20.0f, 8.0f, 8.0f};

    MoveResult r = resolve_move(g, box, 20.0f, 20.0f);
    REQUIRE_THAT(r.box.x, WithinAbs(24.0f, kEps));
    REQUIRE_THAT(r.box.y, WithinAbs(24.0f, kEps));
    REQUIRE(r.hit_x);
    REQUIRE(r.hit_y);
}

TEST_CASE("resolve_move: sair do mapa pela esquerda trava na borda 0",
          "[core][spatial][collision]") {
    TileGrid g(5, 5, 16.0f);  // todo livre, mas fora-do-mapa = bloqueado
    Aabb box{4.0f, 32.0f, 8.0f, 8.0f};

    MoveResult r = resolve_move(g, box, -20.0f, 0.0f);
    // Nao pode passar de x=0 (celula x=-1 bloqueada).
    REQUIRE_THAT(r.box.x, WithinAbs(0.0f, kEps));
    REQUIRE(r.hit_x);
}

TEST_CASE("resolve_move: sair do mapa pela direita trava na borda",
          "[core][spatial][collision]") {
    TileGrid g(5, 5, 16.0f);  // mundo vai de 0 a 80
    Aabb box{68.0f, 32.0f, 8.0f, 8.0f};  // borda direita em 76

    MoveResult r = resolve_move(g, box, 20.0f, 0.0f);
    // Borda direita nao passa de 80 -> x == 72.
    REQUIRE_THAT(r.box.x, WithinAbs(72.0f, kEps));
    REQUIRE(r.hit_x);
}

TEST_CASE("resolve_move: deslocamento zero nao move e nao colide",
          "[core][spatial][collision]") {
    TileGrid g(5, 5, 16.0f);
    Aabb box{32.0f, 32.0f, 8.0f, 8.0f};

    MoveResult r = resolve_move(g, box, 0.0f, 0.0f);
    REQUIRE_THAT(r.box.x, WithinAbs(32.0f, kEps));
    REQUIRE_THAT(r.box.y, WithinAbs(32.0f, kEps));
    REQUIRE_FALSE(r.hit_x);
    REQUIRE_FALSE(r.hit_y);
}

TEST_CASE("resolve_move: movimento negativo em Y (para cima) desliza",
          "[core][spatial][collision]") {
    // Parede a esquerda (x=0), empurra para cima-esquerda. X trava, Y sobe.
    TileGrid g = TileGrid::from_rows({
        "#....",
        "#....",
        "#....",
    }, 16.0f);
    Aabb box{20.0f, 36.0f, 8.0f, 8.0f};  // borda esquerda em 20, celula x=1

    MoveResult r = resolve_move(g, box, -10.0f, -10.0f);
    // Parede ocupa x=[0,16). Borda esquerda nao passa de 16 -> x == 16.
    REQUIRE_THAT(r.box.x, WithinAbs(16.0f, kEps));
    REQUIRE_THAT(r.box.y, WithinAbs(26.0f, kEps));
    REQUIRE(r.hit_x);
    REQUIRE_FALSE(r.hit_y);
}

TEST_CASE("resolve_move: ordem de eixo permite contornar quina (slide)",
          "[core][spatial][collision]") {
    // So a celula (2,1) bloqueada. Caixa abaixo-esquerda dela movendo direita+cima.
    // Ao resolver X primeiro a caixa avanca em X enquanto ainda esta na linha y=2
    // (livre), depois sobe em Y sem entrar no bloco. Garante que o slide nao
    // "gruda" em quinas isoladas.
    TileGrid g = TileGrid::from_rows({
        ".....",
        "..#..",
        ".....",
    }, 16.0f);
    Aabb box{12.0f, 36.0f, 8.0f, 8.0f};  // na linha y=2 (livre)

    MoveResult r = resolve_move(g, box, 8.0f, 0.0f);  // so move em X, fica na y=2
    REQUIRE_THAT(r.box.x, WithinAbs(20.0f, kEps));
    REQUIRE_FALSE(r.hit_x);
}

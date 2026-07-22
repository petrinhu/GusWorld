// GusEngine/tests/obstacle_collision_test.cpp
//
// Spec executavel (Catch2 v3) dos OBSTACULOS PONTUAIS do resolve_move/resolve_move_
// with_corner_assist (M7-COSTURA/M7-DIALOGO, colisao SOLIDA de NPC/inimigo). TEST-
// FIRST: estes casos DEFINEM o contrato de gus/core/spatial/grid_collision.hpp::
// ObstacleSpan.
//
// CONTEXTO (playtest ao vivo do lider): o Gus atravessava POR CIMA do Bertoldo (o
// NPC sumia embaixo do sprite do jogador ao aproximar pelo norte) - so existia a
// hitbox de TRIGGER (dialogo/combate), nunca uma colisao FISICA. Padrao canonico
// (Zelda ALttP / Stardew Valley): o corpo do NPC/inimigo bloqueia o movimento como
// uma "parede pontual" pequena (ObstacleSpan), NAO faz parte do tilemap estatico -
// o jogador contorna pelos tiles adjacentes (NAO bloqueia o corredor inteiro).
//
// CONTRATO exercitado:
//   - obstaculo vazio (ObstacleSpan{} default) => byte-identico ao resolve_move sem
//     obstaculos (nenhuma regressao no comportamento legado, ja coberto por
//     grid_collision_test.cpp/corner_assist_test.cpp);
//   - um obstaculo PONTUAL bloqueia o eixo que colidiria com ele, MESMO em campo
//     aberto (sem nenhuma parede da grade por perto) - encosta na borda REAL do
//     obstaculo (nao alinhada a celula);
//   - o obstaculo NAO bloqueia os tiles vizinhos/adjacentes (o jogador contorna
//     livremente por cima, por baixo, pelos lados);
//   - o obstaculo tambem "trava" o corner-assist (nao empurra o jogador pra dentro
//     dele) - mesma garantia de nao-atravessar que a grade ja tem;
//   - a parede da grade E um obstaculo pontual podem concorrer: o mais restritivo
//     (menor deslocamento) vence, seja qual for a ordem.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "gus/core/spatial/grid_collision.hpp"
#include "gus/core/spatial/tile_grid.hpp"

using gus::core::spatial::Aabb;
using gus::core::spatial::CornerAssistOptions;
using gus::core::spatial::MoveResult;
using gus::core::spatial::ObstacleSpan;
using gus::core::spatial::resolve_move;
using gus::core::spatial::resolve_move_with_corner_assist;
using gus::core::spatial::TileGrid;
using Catch::Matchers::WithinAbs;

namespace {
constexpr float kEps = 1e-4f;
}

TEST_CASE("resolve_move: ObstacleSpan vazio (default) e byte-identico ao legado",
          "[core][spatial][collision][obstacle]") {
    TileGrid g = TileGrid::from_rows({
        "..#..",
        "..#..",
        "..#..",
    }, 16.0f);
    Aabb box{20.0f, 20.0f, 8.0f, 8.0f};

    MoveResult with_default = resolve_move(g, box, 20.0f, 10.0f);
    MoveResult with_explicit_empty = resolve_move(g, box, 20.0f, 10.0f, ObstacleSpan{});

    REQUIRE_THAT(with_default.box.x, WithinAbs(with_explicit_empty.box.x, kEps));
    REQUIRE_THAT(with_default.box.y, WithinAbs(with_explicit_empty.box.y, kEps));
    REQUIRE(with_default.hit_x == with_explicit_empty.hit_x);
    REQUIRE(with_default.hit_y == with_explicit_empty.hit_y);
}

TEST_CASE("resolve_move: obstaculo pontual bloqueia MESMO em campo aberto (sem parede)",
          "[core][spatial][collision][obstacle]") {
    // Mapa 10x10 TODO LIVRE (sem nenhuma parede da grade) - prova que o bloqueio
    // vem SO do obstaculo, nao de tile algum.
    TileGrid g(10, 10, 16.0f);
    Aabb box{20.0f, 20.0f, 8.0f, 8.0f};  // borda direita em 28

    // Obstaculo (o "corpo" do Bertoldo/inimigo) logo a direita da caixa, NAO
    // alinhado a nenhuma celula (x=30, borda REAL, nao face de tile).
    Aabb npc{30.0f, 16.0f, 12.0f, 12.0f};
    ObstacleSpan obstacles{&npc, 1};

    MoveResult r = resolve_move(g, box, 20.0f, 0.0f, obstacles);
    // Borda direita da caixa nao pode passar de npc.x=30 -> x == 22 (30 - w=8).
    REQUIRE_THAT(r.box.x, WithinAbs(22.0f, kEps));
    REQUIRE(r.hit_x);
    REQUIRE_FALSE(r.hit_y);
}

TEST_CASE("resolve_move: obstaculo NAO bloqueia tiles adjacentes/vizinhos (contorno livre)",
          "[core][spatial][collision][obstacle]") {
    // MESMO obstaculo do teste acima, mas o jogador tenta ir por CIMA dele (Y
    // negativo, contornando) - o obstaculo ocupa so a faixa Y=[16,28); uma caixa
    // que passa acima (ex.: terminando em y<=16) NAO deve ser afetada.
    //
    // TUNNELING-CLAMP-GUARD (step_clamp.hpp): dx=15.0 (nao mais 20.0) - fica
    // ABAIXO do teto (<=0.95*tile=15.2 pro tile=16), avanca numa unica chamada
    // sem ser clampado, e ainda atravessa a faixa X do obstaculo (30..42) o
    // bastante pra provar a MESMA coisa (Y disjunto = nunca colide).
    TileGrid g(10, 10, 16.0f);
    Aabb box{20.0f, 0.0f, 8.0f, 8.0f};  // faixa Y=[0,8), bem acima do obstaculo

    Aabb npc{30.0f, 16.0f, 12.0f, 12.0f};
    ObstacleSpan obstacles{&npc, 1};

    // Anda para a direita passando "por cima" do obstaculo (nao ha overlap em Y).
    MoveResult r = resolve_move(g, box, 15.0f, 0.0f, obstacles);
    REQUIRE_THAT(r.box.x, WithinAbs(35.0f, kEps));  // moveu LIVRE, sem tocar o obstaculo
    REQUIRE_FALSE(r.hit_x);
}

TEST_CASE("resolve_move: obstaculo pontual nao bloqueia o corredor inteiro (contorna por baixo)",
          "[core][spatial][collision][obstacle]") {
    // Corredor largo (nenhuma parede da grade). O obstaculo fica no meio do
    // caminho reto, mas o jogador consegue desviar por BAIXO dele livremente (o
    // obstaculo e PONTUAL, nao uma parede que cobre a largura toda do corredor).
    //
    // TUNNELING-CLAMP-GUARD (step_clamp.hpp): o deslocamento total (40) excede o
    // teto de UMA chamada (0.95*tile=15.2 pro tile=16) - caminha em VARIOS passos
    // pequenos (exatamente como o jogo real faz por tick), acumulando o mesmo
    // deslocamento total. Nunca deveria colidir em NENHUM passo, pois as faixas Y
    // sao disjuntas o tempo todo.
    TileGrid g(10, 10, 16.0f);
    Aabb npc{48.0f, 48.0f, 16.0f, 16.0f};  // obstaculo no centro do corredor
    ObstacleSpan obstacles{&npc, 1};

    Aabb box{20.0f, 80.0f, 8.0f, 8.0f};
    for (int i = 0; i < 3; ++i) {
        MoveResult r = resolve_move(g, box, 15.0f, 0.0f, obstacles);
        REQUIRE_FALSE(r.hit_x);
        box = r.box;
    }
    REQUIRE_THAT(box.x, WithinAbs(65.0f, kEps));  // 20 + 3*15 = 65, bem alem do obstaculo
}

TEST_CASE("resolve_move: parede da grade E obstaculo concorrem - o mais restritivo vence",
          "[core][spatial][collision][obstacle]") {
    // Parede da grade em x=2 (face esquerda em 32) e obstaculo (faixa [28,36)) MAIS
    // PERTO: o obstaculo deve vencer (trava antes da parede) - candidato do
    // obstaculo = 28 - 8 = 20 (< 24 da parede).
    //
    // TUNNELING-CLAMP-GUARD (step_clamp.hpp): o deslocamento total (30) excede o
    // teto de UMA chamada (0.95*tile=15.2 pro tile=16) - caminha em 2 passos de 15
    // (soma 30, identica ao delta original). O 1o passo (x=4->19) nao alcanca nem
    // o obstaculo nem a parede (fica livre); o 2o passo (x=19, alvo cru 34, mesmo
    // alvo final do delta original de 30 a partir de x=4) e onde os dois
    // bloqueadores concorrem de fato - reproduz EXATAMENTE o mesmo alvo cru do
    // teste original, so que em 2 chamadas.
    TileGrid g = TileGrid::from_rows({
        "..#.",
        "..#.",
    }, 16.0f);
    Aabb box{4.0f, 4.0f, 8.0f, 8.0f};  // borda direita em 12

    Aabb npc{28.0f, 0.0f, 8.0f, 16.0f};
    ObstacleSpan obstacles{&npc, 1};

    MoveResult r1 = resolve_move(g, box, 15.0f, 0.0f, obstacles);
    REQUIRE_FALSE(r1.hit_x);
    MoveResult r2 = resolve_move(g, r1.box, 15.0f, 0.0f, obstacles);
    REQUIRE_THAT(r2.box.x, WithinAbs(20.0f, kEps));
    REQUIRE(r2.hit_x);
}

TEST_CASE("resolve_move_with_corner_assist: nao empurra o jogador pra dentro do obstaculo",
          "[core][spatial][collision][obstacle]") {
    // Sem NENHUMA parede da grade (campo aberto): so o obstaculo restringe. O
    // corner-assist so ativa quando o eixo principal bateu (hit_x/hit_y) - com
    // campo aberto e obstaculo bem alinhado (nao e uma "quina"), a caixa colide de
    // frente e o corner-assist NAO deve encontrar abertura que atravesse o corpo do
    // obstaculo.
    TileGrid g(10, 10, 16.0f);
    Aabb box{20.0f, 20.0f, 8.0f, 8.0f};
    Aabb npc{28.0f, 12.0f, 24.0f, 24.0f};  // obstaculo GRANDE, cobre bem mais que a
                                            // faixa Y da caixa - sem abertura lateral
    ObstacleSpan obstacles{&npc, 1};

    CornerAssistOptions opts{};  // enabled=true, default max_assist_fraction
    MoveResult r =
        resolve_move_with_corner_assist(g, box, 20.0f, 0.0f, opts, obstacles);
    // A caixa final NUNCA deve sobrepor o obstaculo.
    const bool overlaps = box.x < npc.x + npc.w && r.box.x + box.w > npc.x &&
                          r.box.y < npc.y + npc.h && r.box.y + box.h > npc.y;
    REQUIRE_FALSE(overlaps);
    REQUIRE(r.hit_x);
}

TEST_CASE("resolve_move_with_corner_assist: ObstacleSpan vazio reproduz o corner-assist legado",
          "[core][spatial][collision][obstacle]") {
    TileGrid g = TileGrid::from_rows({
        "....",
        "..#.",
        "....",
    }, 16.0f);
    Aabb box{4.0f, 4.0f, 8.0f, 8.0f};
    CornerAssistOptions opts{};

    MoveResult with_default = resolve_move_with_corner_assist(g, box, 8.0f, 0.0f, opts);
    MoveResult with_explicit_empty =
        resolve_move_with_corner_assist(g, box, 8.0f, 0.0f, opts, ObstacleSpan{});

    REQUIRE_THAT(with_default.box.x, WithinAbs(with_explicit_empty.box.x, kEps));
    REQUIRE_THAT(with_default.box.y, WithinAbs(with_explicit_empty.box.y, kEps));
    REQUIRE(with_default.hit_x == with_explicit_empty.hit_x);
    REQUIRE(with_default.hit_y == with_explicit_empty.hit_y);
}

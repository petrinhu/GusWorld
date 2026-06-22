// GusEngine/tests/corner_assist_test.cpp
//
// Spec executavel (Catch2 v3) do corner-correction (corner-assist) sobre a colisao
// AABB-desliza-na-grade. TEST-FIRST: estes casos DEFINEM o contrato da extensao.
//
// FEEL DECIDIDO PELO LIDER: quando o jogador anda contra uma parede e SO A QUINA
// pega (esta levemente desalinhado com uma abertura adjacente, e a maior parte da
// largura dele passaria pela abertura), o movimento o EMPURRA lateralmente o
// suficiente pra alinhar e contornar (estilo Stardew/Zelda/Celeste). So ajuda
// quando HA abertura (celula lateral/diagonal livre); NUNCA atravessa parede
// solida. Quanto "perdoa" = threshold configuravel (fracao do tile, default ~0.35)
// + on/off.
//
// IMPORTANTE: o resolve_move ORIGINAL e seus testes (grid_collision_test.cpp)
// permanecem INTACTOS. O corner-assist e uma EXTENSAO:
//   resolve_move_with_corner_assist(grid, box, dx, dy, opts) -> MoveResult.
// Com corner-assist desligado (ou threshold 0), reproduz o resolve_move atual.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "gus/core/spatial/grid_collision.hpp"
#include "gus/core/spatial/tile_grid.hpp"

using gus::core::spatial::Aabb;
using gus::core::spatial::CornerAssistOptions;
using gus::core::spatial::MoveResult;
using gus::core::spatial::resolve_move;
using gus::core::spatial::resolve_move_with_corner_assist;
using gus::core::spatial::TileGrid;
using Catch::Matchers::WithinAbs;

namespace {
constexpr float kEps = 1e-4f;
}

TEST_CASE("corner_assist: quina perdoavel escorrega e passa pela abertura",
          "[core][spatial][corner]") {
    // Tile 16. Parede so na celula (1,0) (topo-meio). A celula (1,1) abaixo e
    // LIVRE (a abertura). Jogador 8 de largura quase alinhado com a abertura, mas
    // com um pingo (4 u) ainda na faixa da parede de cima, empurrando para a
    // DIREITA. Sem assist travaria; com assist, empurra pra baixo o suficiente
    // pra alinhar com a fileira (1,1) e segue para a direita.
    //   col:  0    1    2
    //   y=0: '.'  '#'  '.'
    //   y=1: '.'  '.'  '.'
    TileGrid g = TileGrid::from_rows({
        ".#.",
        "...",
    }, 16.0f);
    // Jogador 8x8 em x=8 (borda dir 16, encosta na col 1), y=12: ocupa y=[12,20),
    // pega 4 u na linha 0 (parede em (1,0)) e 4 u na linha 1 (livre). Desalinhamento
    // = 4 u = 0.25 do tile, < 0.35 -> perdoa.
    Aabb box{8.0f, 12.0f, 8.0f, 8.0f};

    CornerAssistOptions opts;  // enabled=true, threshold=0.35 (defaults)
    MoveResult r = resolve_move_with_corner_assist(g, box, 8.0f, 0.0f, opts);

    // Empurrou pra baixo ate alinhar com a linha 1 (y=16) e avancou em X.
    REQUIRE(r.box.y > 12.0f);                 // foi empurrado para baixo
    REQUIRE_THAT(r.box.y, WithinAbs(16.0f, kEps));  // alinhou com a fileira livre
    REQUIRE(r.box.x > 8.0f);                  // conseguiu avancar em X
}

TEST_CASE("corner_assist: desalinhamento grande demais NAO ajuda (trava como hoje)",
          "[core][spatial][corner]") {
    // Mesmo mapa, mas o jogador esta MUITO dentro da faixa da parede (so um pingo
    // na abertura): desalinhamento > threshold -> nao perdoa, comporta como o
    // resolve_move normal (trava em X, nao empurra).
    TileGrid g = TileGrid::from_rows({
        ".#.",
        "...",
    }, 16.0f);
    // y=2: ocupa y=[2,10), 8 u na linha 0 (so 0 na linha 1). Quase todo na parede.
    Aabb box{8.0f, 2.0f, 8.0f, 8.0f};

    CornerAssistOptions opts;  // threshold 0.35
    MoveResult r = resolve_move_with_corner_assist(g, box, 8.0f, 0.0f, opts);
    MoveResult base = resolve_move(g, box, 8.0f, 0.0f);

    // Identico ao comportamento sem assist: trava em X, Y inalterado.
    REQUIRE_THAT(r.box.x, WithinAbs(base.box.x, kEps));
    REQUIRE_THAT(r.box.y, WithinAbs(base.box.y, kEps));
    REQUIRE(r.hit_x);
}

TEST_CASE("corner_assist: parede solida sem abertura NAO e atravessada",
          "[core][spatial][corner]") {
    // Coluna inteira x=1 bloqueada (sem abertura em cima nem embaixo). Mesmo com
    // assist, nao ha pra onde empurrar -> nao atravessa, trava na borda.
    TileGrid g = TileGrid::from_rows({
        ".#.",
        ".#.",
        ".#.",
    }, 16.0f);
    Aabb box{8.0f, 20.0f, 8.0f, 8.0f};  // encostado na col 1 (parede solida)

    CornerAssistOptions opts;
    MoveResult r = resolve_move_with_corner_assist(g, box, 8.0f, 0.0f, opts);
    // Encostou na face esquerda da parede (x=16-8=8); nunca entrou nela.
    REQUIRE_THAT(r.box.x, WithinAbs(8.0f, kEps));
    REQUIRE(r.hit_x);
    REQUIRE_THAT(r.box.y, WithinAbs(20.0f, kEps));  // nao empurrou (sem abertura)
}

TEST_CASE("corner_assist: movimento reto sem quina fica inalterado",
          "[core][spatial][corner]") {
    // Campo aberto: o assist nao deve introduzir nenhum empurrao espurio.
    TileGrid g(5, 5, 16.0f);
    Aabb box{32.0f, 32.0f, 8.0f, 8.0f};

    CornerAssistOptions opts;
    MoveResult r = resolve_move_with_corner_assist(g, box, 5.0f, 0.0f, opts);
    REQUIRE_THAT(r.box.x, WithinAbs(37.0f, kEps));
    REQUIRE_THAT(r.box.y, WithinAbs(32.0f, kEps));  // Y intocado
    REQUIRE_FALSE(r.hit_x);
}

TEST_CASE("corner_assist: desligado reproduz o resolve_move atual",
          "[core][spatial][corner]") {
    // Com enabled=false, qualquer cenario deve bater BYTE A BYTE com resolve_move.
    TileGrid g = TileGrid::from_rows({
        ".#.",
        "...",
    }, 16.0f);
    Aabb box{8.0f, 12.0f, 8.0f, 8.0f};  // o cenario "perdoavel" do 1o teste

    CornerAssistOptions off;
    off.enabled = false;
    MoveResult r = resolve_move_with_corner_assist(g, box, 8.0f, 0.0f, off);
    MoveResult base = resolve_move(g, box, 8.0f, 0.0f);

    REQUIRE_THAT(r.box.x, WithinAbs(base.box.x, kEps));
    REQUIRE_THAT(r.box.y, WithinAbs(base.box.y, kEps));
    REQUIRE(r.hit_x == base.hit_x);
    REQUIRE(r.hit_y == base.hit_y);
}

TEST_CASE("corner_assist: threshold zero nunca perdoa (equivale a desligado)",
          "[core][spatial][corner]") {
    TileGrid g = TileGrid::from_rows({
        ".#.",
        "...",
    }, 16.0f);
    Aabb box{8.0f, 12.0f, 8.0f, 8.0f};

    CornerAssistOptions opts;
    opts.max_assist_fraction = 0.0f;  // nao perdoa nada
    MoveResult r = resolve_move_with_corner_assist(g, box, 8.0f, 0.0f, opts);
    MoveResult base = resolve_move(g, box, 8.0f, 0.0f);
    REQUIRE_THAT(r.box.y, WithinAbs(base.box.y, kEps));  // sem empurrao
}

TEST_CASE("corner_assist: empurra para CIMA quando a abertura esta em cima",
          "[core][spatial][corner]") {
    // Abertura agora na linha de cima (parede em (1,1)). Jogador desalinhado pra
    // baixo deve ser empurrado pra CIMA (direcao da abertura), nao pra baixo.
    //   y=0: '.'  '.'  '.'
    //   y=1: '.'  '#'  '.'
    TileGrid g = TileGrid::from_rows({
        "...",
        ".#.",
    }, 16.0f);
    // Jogador ocupa y=[12,20): 4 u na linha 0 (livre) e 4 u na linha 1 (parede).
    Aabb box{8.0f, 12.0f, 8.0f, 8.0f};

    CornerAssistOptions opts;
    MoveResult r = resolve_move_with_corner_assist(g, box, 8.0f, 0.0f, opts);
    REQUIRE(r.box.y < 12.0f);                 // empurrado para CIMA
    REQUIRE_THAT(r.box.y, WithinAbs(8.0f, kEps));   // alinhou com a fileira 0 (livre)
    REQUIRE(r.box.x > 8.0f);                  // avancou em X
}

TEST_CASE("corner_assist: funciona no eixo vertical (empurra em X)",
          "[core][spatial][corner]") {
    // Simetrico: jogador indo pra BAIXO, parede so na celula (0,1), abertura em
    // (1,1). Empurra para a DIREITA pra contornar.
    //   x:    0    1
    //   y=0: '.'  '.'
    //   y=1: '#'  '.'
    TileGrid g = TileGrid::from_rows({
        "..",
        "#.",
    }, 16.0f);
    // Jogador ocupa x=[12,20): 4 u na col 0 (parede em (0,1)) e 4 u na col 1 (livre).
    Aabb box{12.0f, 8.0f, 8.0f, 8.0f};

    CornerAssistOptions opts;
    MoveResult r = resolve_move_with_corner_assist(g, box, 0.0f, 8.0f, opts);
    REQUIRE(r.box.x > 12.0f);                 // empurrado para a direita
    REQUIRE_THAT(r.box.x, WithinAbs(16.0f, kEps));  // alinhou com a coluna 1 (livre)
    REQUIRE(r.box.y > 8.0f);                  // avancou em Y
}

TEST_CASE("corner_assist: NAO empurra para dentro de parede (empurrao validado)",
          "[core][spatial][corner]") {
    // O empurrao perpendicular e validado contra a grade: se o unico lado que
    // "abriria" o eixo principal mete o corpo numa parede, nao empurra. Aqui a
    // parede a frente (1,0) so teria abertura empurrando pra baixo, mas a celula
    // (0,1) abaixo-esquerda esta bloqueada, entao o empurrao pra baixo meteria o
    // jogador (que ocupa col 0 e 1) na parede (0,1) -> rejeitado, trava como hoje.
    //   x:    0    1
    //   y=0: '.'  '#'
    //   y=1: '#'  '.'
    TileGrid g = TileGrid::from_rows({
        ".#",
        "#.",
    }, 16.0f);
    // Jogador 8x8 em x=8 (cobre col 0 e 1 ao empurrar), y=12: pega a parede (1,0).
    Aabb box{8.0f, 12.0f, 8.0f, 8.0f};

    CornerAssistOptions opts;
    MoveResult r = resolve_move_with_corner_assist(g, box, 8.0f, 0.0f, opts);
    MoveResult base = resolve_move(g, box, 8.0f, 0.0f);
    // Empurrar pra baixo meteria na parede (0,1); empurrar pra cima sai do mapa
    // (parede). Nenhum empurrao valido -> identico a base (trava).
    REQUIRE_THAT(r.box.x, WithinAbs(base.box.x, kEps));
    REQUIRE_THAT(r.box.y, WithinAbs(base.box.y, kEps));
}

TEST_CASE("corner_assist: passo pequeno tipico nao atravessa parede (sem tunneling)",
          "[core][spatial][corner]") {
    // Regime normal do M1 (passo << 1 tile): o corner-assist nunca fura parede.
    // Parede solida x=1; jogador encostado empurrando pra direita com passo curto.
    // (Nota: passo MAIOR que um tile pode sofrer tunneling como o resolve_move
    // base - limitacao conhecida do movimento cinematico sem swept; fora do M1.)
    TileGrid g = TileGrid::from_rows({
        ".#.",
        ".#.",
        ".#.",
    }, 16.0f);
    Aabb box{8.0f, 20.0f, 8.0f, 8.0f};  // borda direita 16 = face da parede

    CornerAssistOptions opts;
    MoveResult r = resolve_move_with_corner_assist(g, box, 1.0f, 0.0f, opts);
    REQUIRE(r.box.x <= 8.0f + kEps);  // nao entrou na parede solida
    REQUIRE(r.hit_x);
}

// GusEngine/tests/step_clamp_test.cpp
//
// Spec executavel (Catch2 v3) do teto anti-tunneling POR EIXO
// (TUNNELING-CLAMP-GUARD, gus/core/spatial/step_clamp.hpp). TEST-FIRST: estes
// casos DEFINEM o contrato de clamp_step_axis e a fronteira onde ele e aplicado
// (resolve_move/resolve_move_with_corner_assist, grid_collision.hpp/.cpp).
//
// DECISAO DE DESIGN JA TOMADA (nao reabrir): CLAMP, nao swept/CCD. O algoritmo
// de colisao continua target-only; este modulo so IMPEDE que o delta cru chegue
// grande demais na resolucao (ver rationale completo em step_clamp.hpp).
//
// SECOES:
//   a. Unidade: clamp_step_axis isolado (identidade abaixo do teto, clamp acima,
//      validacao de float, tile_size invalido).
//   b. Integracao: resolve_move/resolve_move_with_corner_assist atraves de uma
//      parede real, NOS DOIS SENTIDOS de CADA eixo (reforco do lider via Gus:
//      "e se o personagem andar de costas, velocidade negativa?" - o clamp em
//      |delta| ja e simetrico por construcao; aqui a SUITE PROVA a simetria).

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <cmath>
#include <limits>
#include <vector>

#include "gus/core/spatial/grid_collision.hpp"
#include "gus/core/spatial/step_clamp.hpp"
#include "gus/core/spatial/tile_grid.hpp"

using gus::core::spatial::Aabb;
using gus::core::spatial::clamp_step_axis;
using gus::core::spatial::CornerAssistOptions;
using gus::core::spatial::kMaxStepPerAxisTileFraction;
using gus::core::spatial::MoveResult;
using gus::core::spatial::ObstacleSpan;
using gus::core::spatial::resolve_move;
using gus::core::spatial::resolve_move_with_corner_assist;
using gus::core::spatial::TileGrid;
using Catch::Matchers::WithinAbs;

namespace {

constexpr float kEps = 1e-3f;
constexpr float kInf = std::numeric_limits<float>::infinity();
constexpr float kNan = std::numeric_limits<float>::quiet_NaN();

// ---------------------------------------------------------------------------
// a. UNIDADE - clamp_step_axis isolado.
// ---------------------------------------------------------------------------

TEST_CASE("clamp_step_axis: identidade abaixo do teto (magnitude e sinal preservados)",
          "[core][spatial][step_clamp]") {
    constexpr float ts = 16.0f;
    const float fractions[] = {0.0f, 0.075f, 0.12f, 0.5f, 0.95f};  // ate o teto INCLUSIVE
    for (const float f : fractions) {
        const float d = f * ts;
        CAPTURE(f, d);
        REQUIRE_THAT(clamp_step_axis(d, ts), WithinAbs(d, kEps));
        // Reforco (Gus): a mesma fracao NEGATIVA (andar de costas) e identidade
        // tambem - o clamp preserva o SINAL, so limita a MAGNITUDE.
        REQUIRE_THAT(clamp_step_axis(-d, ts), WithinAbs(-d, kEps));
    }
}

TEST_CASE("clamp_step_axis: clampa acima do teto, preservando o SINAL",
          "[core][spatial][step_clamp]") {
    constexpr float ts = 16.0f;
    const float cap = kMaxStepPerAxisTileFraction * ts;
    const float fractions[] = {1.0f, 1.6f};
    for (const float f : fractions) {
        const float d = f * ts;
        CAPTURE(f, d, cap);
        REQUIRE_THAT(clamp_step_axis(d, ts), WithinAbs(cap, kEps));
        REQUIRE_THAT(clamp_step_axis(-d, ts), WithinAbs(-cap, kEps));
    }
    // Absurdo (lag spike/tuning quebrado) e infinito: mesmo teto, MESMO sinal.
    REQUIRE_THAT(clamp_step_axis(1.0e6f, ts), WithinAbs(cap, kEps));
    REQUIRE_THAT(clamp_step_axis(-1.0e6f, ts), WithinAbs(-cap, kEps));
    REQUIRE_THAT(clamp_step_axis(kInf, ts), WithinAbs(cap, kEps));
    REQUIRE_THAT(clamp_step_axis(-kInf, ts), WithinAbs(-cap, kEps));
}

TEST_CASE("clamp_step_axis: delta NaN nunca move (devolve 0)",
          "[core][spatial][step_clamp]") {
    constexpr float ts = 16.0f;
    REQUIRE(clamp_step_axis(kNan, ts) == 0.0f);
    // -NaN (bit de sinal setado) tambem: NaN nao tem "sentido", so ha um caso.
    REQUIRE(clamp_step_axis(-kNan, ts) == 0.0f);
}

TEST_CASE("clamp_step_axis: tile_size invalido (<=0 ou nao-finito) nunca move",
          "[core][spatial][step_clamp]") {
    const float tile_sizes[] = {0.0f, -16.0f, kNan, kInf, -kInf};
    const float deltas[] = {0.0f, 1.0f, -1.0f, 8.0f, -8.0f, kInf, -kInf};
    for (const float ts : tile_sizes) {
        for (const float d : deltas) {
            CAPTURE(ts, d);
            REQUIRE(clamp_step_axis(d, ts) == 0.0f);
        }
    }
}

TEST_CASE("clamp_step_axis: e constexpr (avaliavel em tempo de compilacao)",
          "[core][spatial][step_clamp]") {
    // Nao testa comportamento novo - so trava que a assinatura continua constexpr
    // (um refactor descuidado que a tire quebraria isto em tempo de COMPILACAO).
    static_assert(clamp_step_axis(8.0f, 16.0f) == 8.0f);
    static_assert(clamp_step_axis(100.0f, 16.0f) == kMaxStepPerAxisTileFraction * 16.0f);
    SUCCEED();
}

// ---------------------------------------------------------------------------
// b. INTEGRACAO - resolve_move / resolve_move_with_corner_assist atraves de
//    uma parede REAL, nos DOIS SENTIDOS de cada eixo.
// ---------------------------------------------------------------------------
//
// Fixture unica por eixo: uma coluna (ou linha) INTEIRAMENTE bloqueada (todas as
// celulas, sem abertura lateral nenhuma) - o corner-assist nao acha por onde
// escapar, entao resolve_move e resolve_move_with_corner_assist devem produzir o
// MESMO resultado (nenhuma quina pra contornar).

constexpr float kTile = 16.0f;
constexpr int kGridCells = 12;  // 12x12 celulas -> mundo 192x192.

// Coluna cx INTEIRAMENTE bloqueada (todas as kGridCells linhas).
TileGrid wall_column(int cx) {
    TileGrid g(kGridCells, kGridCells, kTile);
    for (int cy = 0; cy < kGridCells; ++cy) {
        g.set_blocked(cx, cy, true);
    }
    return g;
}

// Linha cy INTEIRAMENTE bloqueada (todas as kGridCells colunas).
TileGrid wall_row(int cy) {
    TileGrid g(kGridCells, kGridCells, kTile);
    for (int cx = 0; cx < kGridCells; ++cx) {
        g.set_blocked(cx, cy, true);
    }
    return g;
}

// Deltas "absurdos" (excedem o teto de sobra, magnitude CRESCENTE) - devem
// produzir o MESMO resultado clampado, provando que o guarda e insensivel a
// QUAO absurdo o delta cru e (1.6*tile, 5*tile e 1e6 tratados identicamente).
std::vector<float> kHugeMagnitudes() {
    return {1.6f * kTile, 5.0f * kTile, 1.0e6f};
}

CornerAssistOptions default_corner() { return CornerAssistOptions{}; }  // enabled, 0.35

}  // namespace

TEST_CASE("resolve_move: parede a DIREITA + delta positivo absurdo - nunca cruza",
          "[core][spatial][step_clamp][integration]") {
    // Coluna cx=5 bloqueada -> mundo x=[80,96). Box a 1 tile de folga da face
    // esquerda da parede (borda direita da caixa em 72, face da parede em 80).
    const TileGrid g = wall_column(5);
    const Aabb box{64.0f, 64.0f, 8.0f, 8.0f};
    for (const float dx : kHugeMagnitudes()) {
        CAPTURE(dx);
        const MoveResult r = resolve_move(g, box, dx, 0.0f);
        REQUIRE(r.box.x + r.box.w <= 80.0f + kEps);  // nunca do outro lado
        REQUIRE(r.hit_x);
        REQUIRE(r.step_clamped);
        REQUIRE_THAT(r.box.x, WithinAbs(72.0f, kEps));  // encosta na face da parede
    }
}

TEST_CASE("resolve_move: parede a ESQUERDA + delta negativo absurdo (andar de "
          "costas) - nunca cruza",
          "[core][spatial][step_clamp][integration]") {
    // MESMA coluna cx=5 (mundo x=[80,96)), agora a caixa vem do lado DIREITO e
    // anda pra TRAS (dx negativo). Espelho exato do teste acima.
    const TileGrid g = wall_column(5);
    const Aabb box{104.0f, 64.0f, 8.0f, 8.0f};
    for (const float dx_mag : kHugeMagnitudes()) {
        const float dx = -dx_mag;
        CAPTURE(dx);
        const MoveResult r = resolve_move(g, box, dx, 0.0f);
        REQUIRE(r.box.x >= 96.0f - kEps);  // nunca do outro lado
        REQUIRE(r.hit_x);
        REQUIRE(r.step_clamped);
        REQUIRE_THAT(r.box.x, WithinAbs(96.0f, kEps));  // encosta na face da parede
    }
}

TEST_CASE("resolve_move: parede EMBAIXO + delta positivo absurdo - nunca cruza",
          "[core][spatial][step_clamp][integration]") {
    // Linha cy=5 bloqueada -> mundo y=[80,96). Espelho vertical do teste da
    // parede a direita.
    const TileGrid g = wall_row(5);
    const Aabb box{64.0f, 64.0f, 8.0f, 8.0f};
    for (const float dy : kHugeMagnitudes()) {
        CAPTURE(dy);
        const MoveResult r = resolve_move(g, box, 0.0f, dy);
        REQUIRE(r.box.y + r.box.h <= 80.0f + kEps);
        REQUIRE(r.hit_y);
        REQUIRE(r.step_clamped);
        REQUIRE_THAT(r.box.y, WithinAbs(72.0f, kEps));
    }
}

TEST_CASE("resolve_move: parede EM CIMA + delta negativo absurdo (andar de "
          "costas) - nunca cruza",
          "[core][spatial][step_clamp][integration]") {
    const TileGrid g = wall_row(5);
    const Aabb box{64.0f, 104.0f, 8.0f, 8.0f};
    for (const float dy_mag : kHugeMagnitudes()) {
        const float dy = -dy_mag;
        CAPTURE(dy);
        const MoveResult r = resolve_move(g, box, 0.0f, dy);
        REQUIRE(r.box.y >= 96.0f - kEps);
        REQUIRE(r.hit_y);
        REQUIRE(r.step_clamped);
        REQUIRE_THAT(r.box.y, WithinAbs(96.0f, kEps));
    }
}

TEST_CASE("resolve_move: em campo LIVRE (sem parede), delta absurdo avanca "
          "EXATAMENTE 0.95*tile, nos dois sentidos, nos dois eixos",
          "[core][spatial][step_clamp][integration]") {
    const TileGrid g(kGridCells, kGridCells, kTile);  // todo livre
    const Aabb box{64.0f, 64.0f, 8.0f, 8.0f};
    const float cap = kMaxStepPerAxisTileFraction * kTile;

    for (const float mag : kHugeMagnitudes()) {
        // +X
        {
            const MoveResult r = resolve_move(g, box, mag, 0.0f);
            REQUIRE_FALSE(r.hit_x);
            REQUIRE(r.step_clamped);
            REQUIRE_THAT(r.box.x, WithinAbs(64.0f + cap, kEps));
        }
        // -X (de costas)
        {
            const MoveResult r = resolve_move(g, box, -mag, 0.0f);
            REQUIRE_FALSE(r.hit_x);
            REQUIRE(r.step_clamped);
            REQUIRE_THAT(r.box.x, WithinAbs(64.0f - cap, kEps));
        }
        // +Y
        {
            const MoveResult r = resolve_move(g, box, 0.0f, mag);
            REQUIRE_FALSE(r.hit_y);
            REQUIRE(r.step_clamped);
            REQUIRE_THAT(r.box.y, WithinAbs(64.0f + cap, kEps));
        }
        // -Y (de costas)
        {
            const MoveResult r = resolve_move(g, box, 0.0f, -mag);
            REQUIRE_FALSE(r.hit_y);
            REQUIRE(r.step_clamped);
            REQUIRE_THAT(r.box.y, WithinAbs(64.0f - cap, kEps));
        }
    }
}

TEST_CASE("resolve_move: delta legitimo (<=0.95*tile) NAO e marcado step_clamped",
          "[core][spatial][step_clamp][integration]") {
    const TileGrid g(kGridCells, kGridCells, kTile);
    const Aabb box{64.0f, 64.0f, 8.0f, 8.0f};
    const MoveResult r = resolve_move(g, box, 0.95f * kTile, -0.95f * kTile);
    REQUIRE_FALSE(r.step_clamped);
}

// --- MESMOS 4 cenarios de parede, agora via resolve_move_with_corner_assist ---
// (corner-assist LIGADO, default 0.35 - a parede e uma coluna/linha INTEIRA sem
// abertura lateral, entao o resultado deve bater com resolve_move byte a byte).

TEST_CASE("resolve_move_with_corner_assist: parede a DIREITA + delta positivo "
          "absurdo - nunca cruza",
          "[core][spatial][step_clamp][integration][corner-assist]") {
    const TileGrid g = wall_column(5);
    const Aabb box{64.0f, 64.0f, 8.0f, 8.0f};
    for (const float dx : kHugeMagnitudes()) {
        const MoveResult r =
            resolve_move_with_corner_assist(g, box, dx, 0.0f, default_corner());
        REQUIRE(r.box.x + r.box.w <= 80.0f + kEps);
        REQUIRE(r.hit_x);
        REQUIRE(r.step_clamped);
        REQUIRE_THAT(r.box.x, WithinAbs(72.0f, kEps));
    }
}

TEST_CASE("resolve_move_with_corner_assist: parede a ESQUERDA + delta negativo "
          "absurdo (andar de costas) - nunca cruza",
          "[core][spatial][step_clamp][integration][corner-assist]") {
    const TileGrid g = wall_column(5);
    const Aabb box{104.0f, 64.0f, 8.0f, 8.0f};
    for (const float dx_mag : kHugeMagnitudes()) {
        const float dx = -dx_mag;
        const MoveResult r =
            resolve_move_with_corner_assist(g, box, dx, 0.0f, default_corner());
        REQUIRE(r.box.x >= 96.0f - kEps);
        REQUIRE(r.hit_x);
        REQUIRE(r.step_clamped);
        REQUIRE_THAT(r.box.x, WithinAbs(96.0f, kEps));
    }
}

TEST_CASE("resolve_move_with_corner_assist: parede EMBAIXO + delta positivo "
          "absurdo - nunca cruza",
          "[core][spatial][step_clamp][integration][corner-assist]") {
    const TileGrid g = wall_row(5);
    const Aabb box{64.0f, 64.0f, 8.0f, 8.0f};
    for (const float dy : kHugeMagnitudes()) {
        const MoveResult r =
            resolve_move_with_corner_assist(g, box, 0.0f, dy, default_corner());
        REQUIRE(r.box.y + r.box.h <= 80.0f + kEps);
        REQUIRE(r.hit_y);
        REQUIRE(r.step_clamped);
        REQUIRE_THAT(r.box.y, WithinAbs(72.0f, kEps));
    }
}

TEST_CASE("resolve_move_with_corner_assist: parede EM CIMA + delta negativo "
          "absurdo (andar de costas) - nunca cruza",
          "[core][spatial][step_clamp][integration][corner-assist]") {
    const TileGrid g = wall_row(5);
    const Aabb box{64.0f, 104.0f, 8.0f, 8.0f};
    for (const float dy_mag : kHugeMagnitudes()) {
        const float dy = -dy_mag;
        const MoveResult r =
            resolve_move_with_corner_assist(g, box, 0.0f, dy, default_corner());
        REQUIRE(r.box.y >= 96.0f - kEps);
        REQUIRE(r.hit_y);
        REQUIRE(r.step_clamped);
        REQUIRE_THAT(r.box.y, WithinAbs(96.0f, kEps));
    }
}

TEST_CASE("resolve_move_with_corner_assist: ObstacleSpan vazio + delta absurdo "
          "continua nunca cruzando (fronteira do obstaculo tambem clampa)",
          "[core][spatial][step_clamp][integration][corner-assist]") {
    // Obstaculo pontual (nao a grade) tambem e uma "parede" pro clamp: prova que
    // a fronteira do clamp cobre TODOS os chamadores de resolve_move_with_corner_
    // assist, independente de a colisao vir da grade ou de um ObstacleSpan.
    const TileGrid g(kGridCells, kGridCells, kTile);  // sem parede de grade
    const Aabb box{64.0f, 64.0f, 8.0f, 8.0f};
    const Aabb npc{80.0f, 0.0f, 16.0f, 192.0f};  // faixa vertical INTEIRA, sem abertura
    const ObstacleSpan obstacles{&npc, 1};

    const MoveResult r = resolve_move_with_corner_assist(g, box, 1.0e6f, 0.0f,
                                                          default_corner(), obstacles);
    REQUIRE(r.box.x + r.box.w <= 80.0f + kEps);
    REQUIRE(r.hit_x);
    REQUIRE(r.step_clamped);
}

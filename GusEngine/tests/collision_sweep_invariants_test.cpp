// GusEngine/tests/collision_sweep_invariants_test.cpp
//
// M7-FB1 fatia 1: varredura ADVERSARIAL sub-pixel deterministica da colisao
// AABB-desliza-na-grade (gus/core/spatial/grid_collision.hpp). Motivada por
// feedback de playtest ao vivo do lider: colisao pixel-perfect e dificil de
// testar manualmente; um clip acidental de 1 pixel vira exploit em produto
// (atravessar parede, ficar preso dentro da geometria). A classe de bug caçada
// aqui e TUNNELING/penetracao acidental, NAO simples overlap discreto - por
// isso um ORACULO INDEPENDENTE (reimplementado do zero, sem reusar nada do
// .cpp de producao) mede a profundidade real de sobreposicao caixa-vs-celula
// e caixa-vs-obstaculo, sem epsilon de tolerancia de encosto (kEdgeEps e um
// detalhe interno da implementacao, nao do contrato).
//
// ESTE ARQUIVO NAO MODIFICA NENHUMA LINHA DE PRODUCAO. Os 3 arquivos de teste
// example-based existentes (grid_collision_test.cpp, corner_assist_test.cpp,
// obstacle_collision_test.cpp) permanecem intocados - eles fixam o CONTRATO
// caso a caso; este arquivo VARRE o espaco de entrada em busca de violacoes
// que um punhado de casos manuais nao cobre.
//
// INVARIANTES (tier ESTRITO - devem passar SEMPRE, ver check_resolve_move_
// invariants/check_corner_assist_invariants abaixo):
//   I1 penetracao final <= tolerancia contra a grade E contra cada obstaculo;
//   I2 sem retrocesso nem overshoot por eixo (eixo perpendicular do corner-
//      assist pode deslocar ate max_assist_fraction*tile, com folga);
//   I3 w/h da caixa preservados byte-identicos;
//   I4 determinismo: chamar duas vezes com a mesma entrada da o mesmo
//      resultado byte-identico;
//   I5 hit coerente: hit_x==false com dx!=0 implica movimento PLENO (x_final
//      == x0+dx), idem em Y.
//
// TIER ADVERSARIAL (ver TEST_CASE marcado [!mayfail] no fim do arquivo): o
// algoritmo atual resolve so a posicao ALVO (sem swept/CCD - continuous
// collision detection). Se o deslocamento por passo for grande o bastante
// para o rodape da caixa pular INTEIRO por cima de uma parede fina, a colisao
// nunca e detectada (hit_x fica false) e a caixa "teleporta" para o outro
// lado - a caixa nunca chega a SOBREPOR a parede em nenhuma posicao
// verificada, entao I1 sozinho nao pega esse bug; por isso o oraculo de
// tunneling (horizontal_tunnel_detected) reconstroi o "corredor varrido"
// entre a borda inicial e a final e verifica se alguma celula bloqueada ficou
// inteiramente pulada. Esse tier NAO bloqueia CI (tag [!mayfail]) - o
// proposito e documentar a fronteira exata e produzir o contraexemplo minimo,
// nao consertar o .cpp (fora de escopo desta fatia).

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <random>
#include <string_view>
#include <utility>
#include <vector>

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

// Tolerancia de aceite: folga ACIMA do kEdgeEps=1e-4 interno de encosto da
// producao. Penetracao acima disso = a caixa esta de fato DENTRO da parede.
constexpr float kTolFraction = 1e-3f;

// Fracao do tile usada como caixa do player (kPlayerHitboxTileFraction do
// jogo real, ver app/src/maestro.cpp).
constexpr float kBoxTileFraction = 0.6f;

// Seed fixa (sem relogio): a amostragem PRNG e deterministica e reproduzivel.
constexpr std::uint32_t kSweepSeed = 20260721u;

// tile_size real do .gmap e 2.0; 16 e 32 cobrem escalas usadas em fixtures de
// teste/mockup.
constexpr float kTileSizes[] = {2.0f, 16.0f, 32.0f};

// ---------------------------------------------------------------------------
// ORACULO INDEPENDENTE (reimplementado do zero - NAO reusa overlaps_blocked/
// overlaps_aabb do grid_collision.cpp de producao).
// ---------------------------------------------------------------------------

// Profundidade de sobreposicao entre dois retangulos [ax0,ax1)x[ay0,ay1) e
// [bx0,bx1)x[by0,by1): a menor das duas invasoes (MTV de 1 eixo). 0 se os
// retangulos nao se sobrepoem (SEM epsilon - interseccao real decide).
float rect_overlap_depth(float ax0, float ay0, float ax1, float ay1, float bx0,
                        float by0, float bx1, float by1) noexcept {
    const float ox = std::min(ax1, bx1) - std::max(ax0, bx0);
    const float oy = std::min(ay1, by1) - std::max(ay0, by0);
    if (ox <= 0.0f || oy <= 0.0f) {
        return 0.0f;
    }
    return std::min(ox, oy);
}

// Maior profundidade de sobreposicao entre a caixa e QUALQUER celula
// bloqueada da grade. Varre so as celulas que o retangulo da caixa toca (via
// world_to_cell nas 2 bordas), sem epsilon - a interseccao real decide se ha
// overlap (celulas fora dos limites do mapa contam via TileGrid::is_blocked,
// que ja trata borda-do-mapa como parede).
float grid_penetration_depth(const TileGrid& grid, const Aabb& box) noexcept {
    const float tile = grid.tile_size();
    const float bx0 = box.x;
    const float by0 = box.y;
    const float bx1 = box.x + box.w;
    const float by1 = box.y + box.h;
    const int cx0 = grid.world_to_cell(bx0);
    const int cy0 = grid.world_to_cell(by0);
    const int cx1 = grid.world_to_cell(bx1);
    const int cy1 = grid.world_to_cell(by1);

    float max_depth = 0.0f;
    for (int cy = cy0; cy <= cy1; ++cy) {
        for (int cx = cx0; cx <= cx1; ++cx) {
            if (!grid.is_blocked(cx, cy)) {
                continue;
            }
            const float depth = rect_overlap_depth(
                bx0, by0, bx1, by1, static_cast<float>(cx) * tile,
                static_cast<float>(cy) * tile, static_cast<float>(cx + 1) * tile,
                static_cast<float>(cy + 1) * tile);
            max_depth = std::max(max_depth, depth);
        }
    }
    return max_depth;
}

// Maior profundidade de sobreposicao entre a caixa e QUALQUER obstaculo
// pontual (AABB vs AABB comum, sem alinhamento a celula).
float obstacles_penetration_depth(const Aabb& box, const Aabb* obstacles,
                                  int count) noexcept {
    float max_depth = 0.0f;
    for (int i = 0; i < count; ++i) {
        const Aabb& ob = obstacles[i];
        const float depth = rect_overlap_depth(box.x, box.y, box.x + box.w,
                                               box.y + box.h, ob.x, ob.y,
                                               ob.x + ob.w, ob.y + ob.h);
        max_depth = std::max(max_depth, depth);
    }
    return max_depth;
}

bool is_valid_start(const TileGrid& grid, const Aabb& box, const Aabb* obstacles,
                    int count, float tol) noexcept {
    return grid_penetration_depth(grid, box) <= tol &&
           obstacles_penetration_depth(box, obstacles, count) <= tol;
}

// Oraculo de TUNNELING (tier adversarial): dado um movimento puramente
// horizontal (dy==0) sem hit reportado, reconstroi o "corredor varrido" entre
// a borda direita/esquerda inicial e a borda oposta final e verifica se
// alguma celula bloqueada ficou INTEIRAMENTE contida nesse corredor - prova
// de que a caixa pulou por cima dela sem nunca ser barrada.
bool horizontal_tunnel_detected(const TileGrid& grid, const Aabb& start,
                                const MoveResult& result, float dx) noexcept {
    if (result.hit_x || dx == 0.0f) {
        return false;
    }
    const float tile = grid.tile_size();
    float sweep_lo;
    float sweep_hi;
    if (dx > 0.0f) {
        sweep_lo = start.x + start.w;
        sweep_hi = result.box.x;
    } else {
        sweep_lo = result.box.x + result.box.w;
        sweep_hi = start.x;
    }
    if (sweep_hi <= sweep_lo) {
        return false;  // nao sobrou "corredor" varrido: sem espaco pra pular nada.
    }

    const int cy0 = grid.world_to_cell(start.y);
    const int cy1 = grid.world_to_cell(start.y + start.h - 1e-4f);
    const int cx_from = grid.world_to_cell(sweep_lo);
    const int cx_to = grid.world_to_cell(sweep_hi - 1e-4f);
    for (int cy = cy0; cy <= cy1; ++cy) {
        for (int cx = cx_from; cx <= cx_to; ++cx) {
            const float cell_l = static_cast<float>(cx) * tile;
            const float cell_r = static_cast<float>(cx + 1) * tile;
            const bool fully_inside_sweep =
                cell_l >= sweep_lo - 1e-4f && cell_r <= sweep_hi + 1e-4f;
            if (fully_inside_sweep && grid.is_blocked(cx, cy)) {
                return true;
            }
        }
    }
    return false;
}

// ---------------------------------------------------------------------------
// Fixtures (grades) - as 4 topologias pedidas.
// ---------------------------------------------------------------------------

// (a) Quina isolada num campo aberto: uma unica celula bloqueada no meio de
// um campo 7x7 livre.
TileGrid fixture_open_corner(float tile) {
    return TileGrid::from_rows(
        {
            ".......",
            ".......",
            ".......",
            "...#...",
            ".......",
            ".......",
            ".......",
        },
        tile);
}

// (b) Corredor de 1 tile de largura (linha livre entre duas paredes, com a
// borda do mapa fechando as pontas).
TileGrid fixture_corridor(float tile) {
    return TileGrid::from_rows(
        {
            "#######",
            "#######",
            ".......",
            "#######",
            "#######",
        },
        tile);
}

// (c) Sala com pilares em xadrez: toda celula livre toca uma bloqueada.
TileGrid fixture_checkerboard(float tile) {
    return TileGrid::from_rows(
        {
            ".#.#.#.",
            "#.#.#.#",
            ".#.#.#.",
            "#.#.#.#",
            ".#.#.#.",
            "#.#.#.#",
            ".#.#.#.",
        },
        tile);
}

// (d) Canto da borda do mapa: grade TODA livre - o unico "bloqueador" e a
// borda do mapa (TileGrid::is_blocked trata fora-dos-limites como parede).
// Testado explicitamente nos 4 cantos (ver cells_to_sweep abaixo), nao via
// boundary_free_cells (que so acha vizinhanca de celula bloqueada DENTRO da
// grade).
TileGrid fixture_map_border_corner(float tile) {
    return TileGrid(6, 6, tile);
}

struct FixtureSpec {
    std::string_view name;
    TileGrid (*build)(float tile);
};

const std::vector<FixtureSpec>& fixtures() {
    static const std::vector<FixtureSpec> specs = {
        {"quina-isolada-campo-aberto", fixture_open_corner},
        {"corredor-1-tile", fixture_corridor},
        {"pilares-xadrez", fixture_checkerboard},
        {"canto-borda-mapa", fixture_map_border_corner},
    };
    return specs;
}

// true se (cx,cy) esta DENTRO dos limites da grade E bloqueada. Vizinhos FORA
// dos limites nao contam aqui de proposito (a borda do mapa e tratada a parte
// pela fixture (d) via cells_to_sweep) - senao TODA celula de borda de TODA
// fixture entraria na varredura exaustiva e explodiria a combinatoria sem
// agregar cobertura nova (a borda ja e o foco dedicado da fixture (d)).
bool is_blocked_in_bounds(const TileGrid& grid, int cx, int cy) noexcept {
    if (cx < 0 || cy < 0 || cx >= grid.width() || cy >= grid.height()) {
        return false;
    }
    return grid.is_blocked(cx, cy);
}

// Todas as celulas LIVRES que tocam (8 vizinhos) uma celula bloqueada dentro
// da propria grade - candidatas a expor bug de quina/borda de parede.
std::vector<std::pair<int, int>> boundary_free_cells(const TileGrid& grid) {
    std::vector<std::pair<int, int>> out;
    for (int cy = 0; cy < grid.height(); ++cy) {
        for (int cx = 0; cx < grid.width(); ++cx) {
            if (grid.is_blocked(cx, cy)) {
                continue;
            }
            const bool touches_blocked =
                is_blocked_in_bounds(grid, cx - 1, cy) ||
                is_blocked_in_bounds(grid, cx + 1, cy) ||
                is_blocked_in_bounds(grid, cx, cy - 1) ||
                is_blocked_in_bounds(grid, cx, cy + 1) ||
                is_blocked_in_bounds(grid, cx - 1, cy - 1) ||
                is_blocked_in_bounds(grid, cx + 1, cy - 1) ||
                is_blocked_in_bounds(grid, cx - 1, cy + 1) ||
                is_blocked_in_bounds(grid, cx + 1, cy + 1);
            if (touches_blocked) {
                out.push_back({cx, cy});
            }
        }
    }
    return out;
}

// Celulas a varrer por fixture: a fixture (d) (sem parede interna nenhuma)
// usa os 4 cantos do mapa explicitamente; as demais usam boundary_free_cells.
std::vector<std::pair<int, int>> cells_to_sweep(std::string_view fixture_name,
                                                const TileGrid& grid) {
    if (fixture_name == "canto-borda-mapa") {
        return {
            {0, 0},
            {grid.width() - 1, 0},
            {0, grid.height() - 1},
            {grid.width() - 1, grid.height() - 1},
        };
    }
    return boundary_free_cells(grid);
}

// ---------------------------------------------------------------------------
// Ancoras e ofertas de posicao de partida (encostadas nas fronteiras de
// celula, nos dois lados/eixos, com offsets sub-pixel).
// ---------------------------------------------------------------------------

// Uma ancora e uma posicao-base (x,y) da caixa DENTRO da celula livre (cx,cy),
// encostada em UMA das 4 bordas da propria celula; offset_dx/offset_dy (0 ou
// +-1) indicam em qual eixo/sentido um offset sub-pixel positivo afasta a
// caixa daquela borda (nunca em direcao a parede - a pre-condicao exige
// posicao de partida valida).
struct Anchor {
    float x;
    float y;
    float offset_dx;
    float offset_dy;
};

std::vector<Anchor> cell_anchors(int cx, int cy, float tile,
                                 float box_size) noexcept {
    const float cell_x = static_cast<float>(cx) * tile;
    const float cell_y = static_cast<float>(cy) * tile;
    const float centered_x = cell_x + (tile - box_size) * 0.5f;
    const float centered_y = cell_y + (tile - box_size) * 0.5f;
    return {
        {cell_x, centered_y, 1.0f, 0.0f},                     // encostado esquerda
        {cell_x + tile - box_size, centered_y, -1.0f, 0.0f},  // encostado direita
        {centered_x, cell_y, 0.0f, 1.0f},                     // encostado topo
        {centered_x, cell_y + tile - box_size, 0.0f, -1.0f},  // encostado base
    };
}

Aabb apply_offset(const Anchor& anchor, float offset, float box_size) noexcept {
    return Aabb{anchor.x + offset * anchor.offset_dx,
               anchor.y + offset * anchor.offset_dy, box_size, box_size};
}

std::vector<float> sub_pixel_offsets(float tile) {
    return {0.0f, tile / 256.0f, tile / 64.0f, tile / 16.0f};
}

// Deltas cardinais + diagonais, magnitude por eixo no envelope pedido (ate
// 0.95 tile - ainda garantido matematicamente pelo algoritmo target-only,
// falha aqui = bug real, nao limitacao conhecida).
std::vector<std::pair<float, float>> sweep_deltas(float tile) {
    static constexpr float kMagnitudeFractions[] = {
        1.0f / 64.0f, 1.0f / 16.0f, 0.12f, 0.24f, 0.30f, 0.60f, 0.95f,
    };
    std::vector<std::pair<float, float>> out;
    for (const float frac : kMagnitudeFractions) {
        const float m = frac * tile;
        out.push_back({m, 0.0f});
        out.push_back({-m, 0.0f});
        out.push_back({0.0f, m});
        out.push_back({0.0f, -m});
        out.push_back({m, m});
        out.push_back({m, -m});
        out.push_back({-m, m});
        out.push_back({-m, -m});
    }
    return out;
}

// So os cardinais (o corner-assist so atua em movimento cardinal - ver
// grid_collision.hpp).
std::vector<std::pair<float, float>> cardinal_deltas(float tile) {
    static constexpr float kMagnitudeFractions[] = {
        1.0f / 64.0f, 1.0f / 16.0f, 0.12f, 0.24f, 0.30f, 0.60f, 0.95f,
    };
    std::vector<std::pair<float, float>> out;
    for (const float frac : kMagnitudeFractions) {
        const float m = frac * tile;
        out.push_back({m, 0.0f});
        out.push_back({-m, 0.0f});
        out.push_back({0.0f, m});
        out.push_back({0.0f, -m});
    }
    return out;
}

// ---------------------------------------------------------------------------
// Verificadores de invariante (tier ESTRITO). REQUIRE aborta no primeiro
// contraexemplo (CAPTURE ja deixou os parametros de repro no relatorio).
// ---------------------------------------------------------------------------

void check_resolve_move_invariants(const TileGrid& grid, const Aabb& box0,
                                   float dx, float dy, const Aabb* obstacles,
                                   int ob_count, float tile,
                                   std::string_view fixture_name) {
    const float tol = kTolFraction * tile;
    if (!is_valid_start(grid, box0, obstacles, ob_count, tol)) {
        return;  // descarta posicao de partida invalida (precondicao do contrato).
    }
    const ObstacleSpan span{obstacles, ob_count};

    const MoveResult r1 = resolve_move(grid, box0, dx, dy, span);
    const MoveResult r2 = resolve_move(grid, box0, dx, dy, span);  // I4

    CAPTURE(fixture_name, tile, box0.x, box0.y, box0.w, box0.h, dx, dy, ob_count,
            r1.box.x, r1.box.y, r1.hit_x, r1.hit_y);

    // I3: w/h preservados byte-identicos.
    REQUIRE(r1.box.w == box0.w);
    REQUIRE(r1.box.h == box0.h);

    // I4: determinismo byte-identico entre duas chamadas iguais.
    REQUIRE(r1.box.x == r2.box.x);
    REQUIRE(r1.box.y == r2.box.y);
    REQUIRE(r1.hit_x == r2.hit_x);
    REQUIRE(r1.hit_y == r2.hit_y);

    // I1: penetracao final <= tolerancia contra a grade E contra obstaculos.
    REQUIRE(grid_penetration_depth(grid, r1.box) <= tol);
    REQUIRE(obstacles_penetration_depth(r1.box, obstacles, ob_count) <= tol);

    // I2: sem retrocesso nem overshoot por eixo (resolve_move puro).
    if (dx > 0.0f) {
        REQUIRE(r1.box.x >= box0.x - tol);
        REQUIRE(r1.box.x <= box0.x + dx + tol);
    } else if (dx < 0.0f) {
        REQUIRE(r1.box.x <= box0.x + tol);
        REQUIRE(r1.box.x >= box0.x + dx - tol);
    } else {
        REQUIRE(r1.box.x == box0.x);
    }
    if (dy > 0.0f) {
        REQUIRE(r1.box.y >= box0.y - tol);
        REQUIRE(r1.box.y <= box0.y + dy + tol);
    } else if (dy < 0.0f) {
        REQUIRE(r1.box.y <= box0.y + tol);
        REQUIRE(r1.box.y >= box0.y + dy - tol);
    } else {
        REQUIRE(r1.box.y == box0.y);
    }

    // I5: hit coerente - sem hit implica movimento PLENO no eixo.
    if (!r1.hit_x && dx != 0.0f) {
        REQUIRE_THAT(r1.box.x, WithinAbs(box0.x + dx, tol));
    }
    if (!r1.hit_y && dy != 0.0f) {
        REQUIRE_THAT(r1.box.y, WithinAbs(box0.y + dy, tol));
    }
}

void check_corner_assist_invariants(const TileGrid& grid, const Aabb& box0,
                                    float dx, float dy, const Aabb* obstacles,
                                    int ob_count, float tile,
                                    std::string_view fixture_name) {
    const float tol = kTolFraction * tile;
    if (!is_valid_start(grid, box0, obstacles, ob_count, tol)) {
        return;
    }
    const ObstacleSpan span{obstacles, ob_count};
    const CornerAssistOptions opts{};  // defaults: enabled=true, fraction=0.35f
    const float max_assist = opts.max_assist_fraction * tile;

    const MoveResult r1 =
        resolve_move_with_corner_assist(grid, box0, dx, dy, opts, span);
    const MoveResult r2 =
        resolve_move_with_corner_assist(grid, box0, dx, dy, opts, span);  // I4

    CAPTURE(fixture_name, tile, box0.x, box0.y, dx, dy, ob_count, r1.box.x,
            r1.box.y, r1.hit_x, r1.hit_y);

    // I3
    REQUIRE(r1.box.w == box0.w);
    REQUIRE(r1.box.h == box0.h);

    // I4
    REQUIRE(r1.box.x == r2.box.x);
    REQUIRE(r1.box.y == r2.box.y);
    REQUIRE(r1.hit_x == r2.hit_x);
    REQUIRE(r1.hit_y == r2.hit_y);

    // I1
    REQUIRE(grid_penetration_depth(grid, r1.box) <= tol);
    REQUIRE(obstacles_penetration_depth(r1.box, obstacles, ob_count) <= tol);

    // I2 (assist): eixo PRINCIPAL sem retrocesso/overshoot; eixo PERPENDICULAR
    // pode deslocar ate max_assist_fraction*tile (com folga de tolerancia).
    if (dx != 0.0f) {
        if (dx > 0.0f) {
            REQUIRE(r1.box.x >= box0.x - tol);
            REQUIRE(r1.box.x <= box0.x + dx + tol);
        } else {
            REQUIRE(r1.box.x <= box0.x + tol);
            REQUIRE(r1.box.x >= box0.x + dx - tol);
        }
        REQUIRE(std::abs(r1.box.y - box0.y) <= max_assist + tol);
    }
    if (dy != 0.0f) {
        if (dy > 0.0f) {
            REQUIRE(r1.box.y >= box0.y - tol);
            REQUIRE(r1.box.y <= box0.y + dy + tol);
        } else {
            REQUIRE(r1.box.y <= box0.y + tol);
            REQUIRE(r1.box.y >= box0.y + dy - tol);
        }
        REQUIRE(std::abs(r1.box.x - box0.x) <= max_assist + tol);
    }

    // I5
    if (!r1.hit_x && dx != 0.0f) {
        REQUIRE_THAT(r1.box.x, WithinAbs(box0.x + dx, tol));
    }
    if (!r1.hit_y && dy != 0.0f) {
        REQUIRE_THAT(r1.box.y, WithinAbs(box0.y + dy, tol));
    }
}

// ---------------------------------------------------------------------------
// Cenarios de obstaculo (0/1/2 AABBs de 1.0*tile), colados em parede/quina.
// Obstaculos DE PROPOSITO alinhados a celula (nao e exigencia do contrato,
// mas deixa a mesma cell_anchors() varrer a celula livre "espremida" entre
// parede-da-grade e obstaculo, ou entre dois obstaculos, sem duplicar codigo
// de ancoragem).
// ---------------------------------------------------------------------------

struct ObstacleScenario {
    std::string_view name;
    TileGrid grid;
    std::vector<Aabb> obstacles;
    int sweep_cx;
    int sweep_cy;
};

std::vector<ObstacleScenario> obstacle_scenarios(float tile) {
    std::vector<ObstacleScenario> out;

    // (1) "gap-parede-obstaculo": parede da grade na celula (2,1); obstaculo
    // cell-aligned na celula (4,1); a celula livre (3,1) fica ESPREMIDA entre
    // os dois - corredor de 1 tile criado por parede+NPC, nao pelo tilemap
    // (o cenario exato do playtest: Bertoldo virando parede pontual).
    {
        TileGrid g = TileGrid::from_rows(
            {
                ".....",
                "..#..",
                ".....",
            },
            tile);
        std::vector<Aabb> obs = {Aabb{4.0f * tile, 1.0f * tile, tile, tile}};
        out.push_back({"gap-parede-obstaculo", g, obs, 3, 1});
    }

    // (2) "quina-obstaculo": parede da grade na celula (1,1); obstaculo
    // cell-aligned na celula (2,2) (diagonal). A celula livre (2,1) toca a
    // parede pela esquerda E o obstaculo por baixo - quina mista grade+NPC.
    {
        TileGrid g = TileGrid::from_rows(
            {
                ".....",
                ".#...",
                ".....",
            },
            tile);
        std::vector<Aabb> obs = {Aabb{2.0f * tile, 2.0f * tile, tile, tile}};
        out.push_back({"quina-obstaculo", g, obs, 2, 1});
    }

    // (3) "dois-obstaculos-espremido": campo TODO livre (so a borda do mapa e
    // parede); dois obstaculos flanqueiam a celula livre (4,4) dos dois
    // lados - espremido so por obstaculos, ZERO parede da grade envolvida.
    {
        TileGrid g(9, 9, tile);
        std::vector<Aabb> obs = {
            Aabb{3.0f * tile, 4.0f * tile, tile, tile},
            Aabb{5.0f * tile, 4.0f * tile, tile, tile},
        };
        out.push_back({"dois-obstaculos-espremido", g, obs, 4, 4});
    }

    return out;
}

}  // namespace

// ---------------------------------------------------------------------------
// TIER ESTRITO - devem passar SEMPRE.
// ---------------------------------------------------------------------------

TEST_CASE("collision_sweep: grade sem obstaculos - varredura exaustiva de "
          "fronteiras (tier estrito)",
          "[core][spatial][collision][sweep]") {
    std::size_t checked = 0;
    for (const FixtureSpec& fx : fixtures()) {
        for (const float tile : kTileSizes) {
            const TileGrid grid = fx.build(tile);
            const float box_size = kBoxTileFraction * tile;
            for (const auto& cell : cells_to_sweep(fx.name, grid)) {
                for (const Anchor& anchor :
                     cell_anchors(cell.first, cell.second, tile, box_size)) {
                    for (const float offset : sub_pixel_offsets(tile)) {
                        const Aabb start = apply_offset(anchor, offset, box_size);
                        for (const auto& delta : sweep_deltas(tile)) {
                            check_resolve_move_invariants(
                                grid, start, delta.first, delta.second, nullptr,
                                0, tile, fx.name);
                            ++checked;
                        }
                    }
                }
            }
        }
    }
    INFO("casos exaustivos (grade, sem obstaculos) verificados: " << checked);
    REQUIRE(checked > 0);
}

TEST_CASE("collision_sweep: obstaculos pontuais colados em parede/quina "
          "(tier estrito)",
          "[core][spatial][collision][sweep][obstacle]") {
    std::size_t checked = 0;
    for (const float tile : kTileSizes) {
        const float box_size = kBoxTileFraction * tile;
        for (const ObstacleScenario& sc : obstacle_scenarios(tile)) {
            for (const Anchor& anchor :
                 cell_anchors(sc.sweep_cx, sc.sweep_cy, tile, box_size)) {
                for (const float offset : sub_pixel_offsets(tile)) {
                    const Aabb start = apply_offset(anchor, offset, box_size);
                    for (const auto& delta : sweep_deltas(tile)) {
                        check_resolve_move_invariants(
                            sc.grid, start, delta.first, delta.second,
                            sc.obstacles.data(),
                            static_cast<int>(sc.obstacles.size()), tile, sc.name);
                        ++checked;
                    }
                }
            }
        }
    }
    INFO("casos com obstaculos verificados: " << checked);
    REQUIRE(checked > 0);
}

TEST_CASE("collision_sweep: corner-assist - varredura cardinal (tier "
          "estrito)",
          "[core][spatial][collision][sweep][corner]") {
    std::size_t checked = 0;
    for (const FixtureSpec& fx : fixtures()) {
        for (const float tile : kTileSizes) {
            const TileGrid grid = fx.build(tile);
            const float box_size = kBoxTileFraction * tile;
            for (const auto& cell : cells_to_sweep(fx.name, grid)) {
                for (const Anchor& anchor :
                     cell_anchors(cell.first, cell.second, tile, box_size)) {
                    for (const float offset : sub_pixel_offsets(tile)) {
                        const Aabb start = apply_offset(anchor, offset, box_size);
                        for (const auto& delta : cardinal_deltas(tile)) {
                            check_corner_assist_invariants(
                                grid, start, delta.first, delta.second, nullptr,
                                0, tile, fx.name);
                            ++checked;
                        }
                    }
                }
            }
        }
    }
    INFO("casos de corner-assist verificados: " << checked);
    REQUIRE(checked > 0);
}

TEST_CASE("collision_sweep: amostragem PRNG deterministica de 20000 casos "
          "(tier estrito)",
          "[core][spatial][collision][sweep][random]") {
    std::minstd_rand rng(kSweepSeed);
    std::uniform_int_distribution<int> fixture_pick(
        0, static_cast<int>(fixtures().size()) - 1);
    std::uniform_int_distribution<int> tile_pick(
        0, static_cast<int>(std::size(kTileSizes)) - 1);
    std::uniform_real_distribution<float> unit(0.0f, 1.0f);

    constexpr int kSamples = 20000;
    int accepted = 0;
    int rejected = 0;
    for (int i = 0; i < kSamples; ++i) {
        const FixtureSpec& fx =
            fixtures()[static_cast<std::size_t>(fixture_pick(rng))];
        const float tile = kTileSizes[static_cast<std::size_t>(tile_pick(rng))];
        const TileGrid grid = fx.build(tile);
        const float box_size = kBoxTileFraction * tile;

        const std::vector<std::pair<int, int>> free_cells =
            cells_to_sweep(fx.name, grid);
        if (free_cells.empty()) {
            continue;
        }
        std::uniform_int_distribution<std::size_t> cell_pick(
            0, free_cells.size() - 1);
        const std::pair<int, int> cell = free_cells[cell_pick(rng)];
        const float cell_x = static_cast<float>(cell.first) * tile;
        const float cell_y = static_cast<float>(cell.second) * tile;
        const float margin = tile - box_size;  // caixa cabe inteira na celula.
        const Aabb start{cell_x + unit(rng) * margin, cell_y + unit(rng) * margin,
                         box_size, box_size};

        const float dx = (unit(rng) * 2.0f - 1.0f) * 0.95f * tile;
        const float dy = (unit(rng) * 2.0f - 1.0f) * 0.95f * tile;

        if (!is_valid_start(grid, start, nullptr, 0, kTolFraction * tile)) {
            ++rejected;  // descarta posicao de partida invalida (precondicao).
            continue;
        }
        ++accepted;
        check_resolve_move_invariants(grid, start, dx, dy, nullptr, 0, tile,
                                      fx.name);
    }
    INFO("PRNG seed=" << kSweepSeed << " aceitos=" << accepted
                       << " rejeitados=" << rejected);
    REQUIRE(accepted > 0);
}

// ---------------------------------------------------------------------------
// TIER ADVERSARIAL (caracterizacao) - NAO bloqueia CI.
// ---------------------------------------------------------------------------

TEST_CASE("collision_sweep: tunneling atraves de parede fina de 1 tile - "
          "caracterizacao (tier adversarial)",
          "[core][spatial][collision][sweep][!mayfail]") {
    // ATUALIZADO (TUNNELING-CLAMP-GUARD, ver core/spatial/step_clamp.hpp): o
    // resolve_move continua resolvendo SO a posicao alvo (sem swept/CCD) - o
    // algoritmo em si nao mudou. O que mudou e a FRONTEIRA: resolve_move agora
    // clampa CADA delta em +-0.95*tile_size ANTES de chamar a resolucao (ver
    // grid_collision.cpp). Como este teste usa magnitudes de 1.0 a 3.0*tile
    // POR CHAMADA UNICA, todo delta cru chega clampado a 0.95*tile - abaixo do
    // limiar teorico de tunneling (tile+box_size = 1.6*tile) - entao o CHECK_
    // FALSE(tunneled) abaixo passa DETERMINISTICAMENTE agora: este bloco deixou
    // de ser um "achado esperado" (documentacao de bug conhecido) e passou a
    // ser uma CONFIRMACAO do cerco (o guarda impede o tiro-unico de tunelar).
    // Mantido como tag [!mayfail] por seguranca de regressao: se um refactor
    // futuro remover ou enfraquecer o clamp, este teste volta a falhar aqui
    // (voz de alarme) sem quebrar o CI (tier adversarial, nao bloqueante) -
    // mas o objetivo original de "caracterizar o limite conhecido" nao se
    // aplica mais como estava escrito.
    for (const float tile : kTileSizes) {
        // Parede de 1 tile de espessura na coluna cx=5, campo aberto ao redor.
        TileGrid g = TileGrid::from_rows(
            {
                ".........",
                ".........",
                ".....#...",
                ".........",
                ".........",
            },
            tile);
        const float box_size = kBoxTileFraction * tile;
        // Caixa encostada na face ESQUERDA da parede, tentando atravessar
        // para a direita.
        const Aabb start{5.0f * tile - box_size,
                        2.0f * tile + (tile - box_size) * 0.5f, box_size,
                        box_size};
        REQUIRE(is_valid_start(g, start, nullptr, 0, kTolFraction * tile));

        static constexpr float kMagnitudes[] = {1.0f, 1.2f, 1.5f,
                                                1.8f, 2.0f, 2.5f, 3.0f};
        for (const float mf : kMagnitudes) {
            const float dx = mf * tile;
            const MoveResult r = resolve_move(g, start, dx, 0.0f);
            const bool tunneled = horizontal_tunnel_detected(g, start, r, dx);

            CAPTURE(tile, mf, dx, start.x, start.y, r.box.x, r.hit_x);
            // CERCADO PELO CLAMP (TUNNELING-CLAMP-GUARD): dx aqui e ate 3.0*tile,
            // mas resolve_move clampa o delta cru pra +-0.95*tile ANTES de
            // resolver - sempre abaixo do limiar teorico de tunneling
            // (tile+box_size = 1.6*tile). tunneled fica false pra TODA
            // magnitude testada, pra TODO tile_size. Usa CHECK (nao REQUIRE)
            // pra nao abortar o loop e reportar TODAS as magnitudes/tile_sizes
            // num unico relatorio, do mesmo jeito que caracterizava o bug antes.
            CHECK_FALSE(tunneled);
        }
    }
}

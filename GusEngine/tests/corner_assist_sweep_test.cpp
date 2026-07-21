// GusEngine/tests/corner_assist_sweep_test.cpp
//
// M7-FB1 fatia 2: varredura sub-pixel ADVERSARIAL do corner-assist
// (resolve_move_with_corner_assist), cacando o exploit classico de
// corner-correction: o empurrao perpendicular "pular" por cima de um
// bloqueador (grade OU obstaculo) porque find_corner_push valida SO o
// DESTINO de cada candidato de empurrao, nao o caminho entre eles (ver
// grid_collision.cpp, find_corner_push, linhas 166-217: o loop de busca NAO
// para no primeiro candidato bloqueado, ele continua tentando magnitudes
// maiores - se existir espaco livre do OUTRO LADO de um bloqueador dentro de
// max_assist, o empurrao pode "pousar" la sem nunca ter sido barrado no
// caminho).
//
// ESTE ARQUIVO NAO MODIFICA NENHUMA LINHA DE PRODUCAO. Os 4 arquivos de teste
// existentes (grid_collision_test.cpp, corner_assist_test.cpp,
// obstacle_collision_test.cpp, collision_sweep_invariants_test.cpp da fatia 1)
// permanecem intocados. O oraculo de penetracao (rect_overlap_depth /
// grid_penetration_depth / obstacles_penetration_depth) e reimplementado
// LOCALMENTE aqui (mesma ideia da fatia 1, arquivos de teste nao compartilham
// header hoje - duplicar o helper pequeno mantem os arquivos independentes,
// conforme instrucao do CTO).
//
// INVARIANTES (tier ESTRITO - DEVEM passar sempre):
//   C1 resultado nunca penetra grade nem obstaculo alem de 1e-3*tile
//      (oraculo proprio de profundidade de sobreposicao);
//   C2 deslocamento perpendicular |push| <= max_assist_fraction*tile + tol;
//   C3 eixo principal: mesmo dominio [x0, x0+dx] (sem retrocesso/overshoot);
//   C4 enabled=false e max_assist_fraction=0 sao byte-identicos a
//      resolve_move para TODA a varredura (nao so um caso - mata mutantes
//      no flag de liga/desliga);
//   C5 fronteira de tuning SEGURA: para max_assist_fraction em
//      {0.35 default, 0.5, 0.75, 1.0} contra bloqueadores de >= 1 tile de
//      espessura, o assist NUNCA termina do outro lado do bloqueador - o
//      CENTRO da caixa nao pode ter cruzado o intervalo ocupado pelo
//      bloqueador no eixo do empurrao (crossed_band abaixo). Isto trava por
//      teste o teto seguro que o lider ajusta vendo.
//
// TIER ADVERSARIAL (ver TEST_CASE [!mayfail] no fim do arquivo): obstaculo
// FINO (espessura 0.05*tile a 0.30*tile - mais fino que a caixa do jogador,
// 0.6*tile) colado entre a caixa e uma abertura livre, com
// max_assist_fraction ate 1.5. A matematica do algoritmo (ver comentario
// acima de find_corner_push): o empurrao minimo para o CENTRO da caixa
// cruzar o obstaculo fino e aproximadamente (box_size + espessura_do_fino),
// que para box_size=0.6*tile fica entre 0.65*tile e 0.9*tile - DENTRO do
// alcance de max_assist_fraction >= ~0.65, mesmo abaixo de 1.0. Documenta o
// contraexemplo (CAPTURE) quando ocorre; NAO conserta o .cpp de producao
// (fora de escopo desta fatia). O resultado da varredura e o argumento
// numerico de por que max_assist_fraction tem que ficar ABAIXO de
// box_size_fraction (0.6) pra ser seguro contra QUALQUER espessura de
// bloqueador (inclusive um obstaculo infinitesimalmente fino) - o default
// 0.35 ja fica com folga desse teto.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <random>
#include <string_view>
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

// Tolerancia de aceite: folga acima do kEdgeEps=1e-4 interno de encosto da
// producao. Penetracao acima disso = a caixa esta de fato DENTRO do
// bloqueador.
constexpr float kTolFraction = 1e-3f;

// Fracao do tile usada como caixa do player (kPlayerHitboxTileFraction do
// jogo real, ver app/src/maestro.cpp) - pedida explicitamente pelo CTO.
constexpr float kBoxTileFraction = 0.6f;

// Seed fixa (sem relogio): amostragem PRNG deterministica e reproduzivel.
// MESMA seed da fatia 1 (mesma politica de projeto: 20260721u).
constexpr std::uint32_t kSweepSeed = 20260721u;

constexpr float kTileSizes[] = {2.0f, 16.0f, 32.0f};

// ---------------------------------------------------------------------------
// ORACULO INDEPENDENTE (reimplementado do zero, duplicado da fatia 1 de
// proposito - ver nota no cabecalho do arquivo).
// ---------------------------------------------------------------------------

float rect_overlap_depth(float ax0, float ay0, float ax1, float ay1, float bx0,
                         float by0, float bx1, float by1) noexcept {
    const float ox = std::min(ax1, bx1) - std::max(ax0, bx0);
    const float oy = std::min(ay1, by1) - std::max(ay0, by0);
    if (ox <= 0.0f || oy <= 0.0f) {
        return 0.0f;
    }
    return std::min(ox, oy);
}

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

// C5 / tier adversarial: o CENTRO da caixa cruzou o intervalo [band_lo,
// band_hi] ocupado por um bloqueador ao longo do eixo do empurrao? Comeca
// estritamente de um lado, termina estritamente do lado oposto - prova de
// "salto por cima" mesmo que a caixa nunca tenha penetrado o bloqueador em
// nenhum instante observavel (destino limpo dos dois lados).
bool crossed_band(float center_before, float center_after, float band_lo,
                  float band_hi) noexcept {
    const bool before_lo = center_before < band_lo;
    const bool before_hi = center_before > band_hi;
    const bool after_lo = center_after < band_lo;
    const bool after_hi = center_after > band_hi;
    return (before_lo && after_hi) || (before_hi && after_lo);
}

// ---------------------------------------------------------------------------
// Fixture unica pras 4 orientacoes: grade 3x3, SO a celula central (1,1)
// bloqueada, campo aberto ao redor - abertura livre nos DOIS lados
// perpendiculares ao movimento (a "abertura de cada lado" pedida: qual lado o
// assist usa depende do sinal do desalinhamento, varrido abaixo dos dois
// lados do centro).
// ---------------------------------------------------------------------------

TileGrid fixture_center_corner(float tile) {
    return TileGrid::from_rows(
        {
            "...",
            ".#.",
            "...",
        },
        tile);
}

struct Orientation {
    std::string_view name;
    float dx_sign;  // +1/-1 se o eixo principal e X; 0 caso contrario.
    float dy_sign;  // +1/-1 se o eixo principal e Y; 0 caso contrario.
};

const std::vector<Orientation>& orientations() {
    static const std::vector<Orientation> ors = {
        {"leste", 1.0f, 0.0f},
        {"oeste", -1.0f, 0.0f},
        {"sul", 0.0f, 1.0f},
        {"norte", 0.0f, -1.0f},
    };
    return ors;
}

// Posicao de partida da caixa pra uma orientacao dada, tile, box_size e
// desalinhamento `off` (offset perpendicular ao movimento, relativo ao centro
// da celula central bloqueada (1,1)). A caixa comeca encostada na celula
// (1,1) pelo lado de onde vem o movimento (pronta pra ser barrada por ela e
// disparar o corner-assist quando o desalinhamento for pequeno).
Aabb start_for_orientation(const Orientation& o, float tile, float box_size,
                          float off) noexcept {
    const float centered = 1.0f * tile + (tile - box_size) * 0.5f;
    if (o.dx_sign > 0.0f) {  // leste: vem da coluna 0, encosta na col 1.
        return Aabb{1.0f * tile - box_size, centered + off, box_size, box_size};
    }
    if (o.dx_sign < 0.0f) {  // oeste: vem da coluna 2, encosta na col 1.
        return Aabb{2.0f * tile, centered + off, box_size, box_size};
    }
    if (o.dy_sign > 0.0f) {  // sul: vem da linha 0, encosta na linha 1.
        return Aabb{centered + off, 1.0f * tile - box_size, box_size, box_size};
    }
    // norte: vem da linha 2, encosta na linha 1.
    return Aabb{centered + off, 2.0f * tile, box_size, box_size};
}

// Deltas cardinais pedidos: tile/64, 0.12, 0.24, 0.30 (fracoes do tile).
std::vector<float> cardinal_magnitude_fractions() {
    return {1.0f / 64.0f, 0.12f, 0.24f, 0.30f};
}

// Desalinhamentos: 0 a 0.6*tile em passos de tile/128, nos dois sentidos a
// partir do centro (o parametro central do assist - abaixo de max_assist
// destrava, acima trava).
std::vector<float> misalignments(float tile) {
    std::vector<float> out;
    const float step = tile / 128.0f;
    const int kSteps = 77;  // 77 * (1/128) = 0.6015625 >= 0.6 pedido.
    for (int i = -kSteps; i <= kSteps; ++i) {
        out.push_back(static_cast<float>(i) * step);
    }
    return out;
}

// ---------------------------------------------------------------------------
// Verificador de invariantes (tier ESTRITO) - C1 a C4. REQUIRE aborta no
// primeiro contraexemplo; CAPTURE ja deixou os parametros de repro no
// relatorio.
// ---------------------------------------------------------------------------

void check_corner_assist_invariants(const TileGrid& grid, const Aabb& box0,
                                    float dx, float dy, const Aabb* obstacles,
                                    int ob_count, float tile,
                                    const CornerAssistOptions& opts,
                                    std::string_view case_name) {
    const float tol = kTolFraction * tile;
    if (!is_valid_start(grid, box0, obstacles, ob_count, tol)) {
        return;  // descarta posicao de partida invalida (precondicao).
    }
    const ObstacleSpan span{obstacles, ob_count};
    const float max_assist = opts.max_assist_fraction * tile;

    const MoveResult r1 =
        resolve_move_with_corner_assist(grid, box0, dx, dy, opts, span);
    const MoveResult r2 =
        resolve_move_with_corner_assist(grid, box0, dx, dy, opts, span);

    CAPTURE(case_name, tile, opts.max_assist_fraction, box0.x, box0.y, dx, dy,
            ob_count, r1.box.x, r1.box.y, r1.hit_x, r1.hit_y);

    // determinismo (base de C1-C5: sem isso nenhum contraexemplo e
    // reproduzivel).
    REQUIRE(r1.box.x == r2.box.x);
    REQUIRE(r1.box.y == r2.box.y);
    REQUIRE(r1.hit_x == r2.hit_x);
    REQUIRE(r1.hit_y == r2.hit_y);
    REQUIRE(r1.box.w == box0.w);
    REQUIRE(r1.box.h == box0.h);

    // C1: penetracao final <= tolerancia contra grade E obstaculos.
    REQUIRE(grid_penetration_depth(grid, r1.box) <= tol);
    REQUIRE(obstacles_penetration_depth(r1.box, obstacles, ob_count) <= tol);

    // C3: eixo principal sem retrocesso/overshoot - mesmo dominio [x0,x0+dx].
    // C2: eixo PERPENDICULAR ao movimento principal pode deslocar ate
    // max_assist+tol (e o proprio empurrao do assist - NAO e um bug ficar
    // diferente de box0 quando o delta original daquele eixo era zero: e
    // exatamente isso que o corner-assist faz). So exige coordenada
    // INALTERADA quando os DOIS deltas sao zero (nenhum movimento pedido).
    if (dx != 0.0f) {
        if (dx > 0.0f) {
            REQUIRE(r1.box.x >= box0.x - tol);
            REQUIRE(r1.box.x <= box0.x + dx + tol);
        } else {
            REQUIRE(r1.box.x <= box0.x + tol);
            REQUIRE(r1.box.x >= box0.x + dx - tol);
        }
    } else if (dy != 0.0f) {
        REQUIRE(std::abs(r1.box.x - box0.x) <= max_assist + tol);  // C2
    } else {
        REQUIRE(r1.box.x == box0.x);
    }
    if (dy != 0.0f) {
        if (dy > 0.0f) {
            REQUIRE(r1.box.y >= box0.y - tol);
            REQUIRE(r1.box.y <= box0.y + dy + tol);
        } else {
            REQUIRE(r1.box.y <= box0.y + tol);
            REQUIRE(r1.box.y >= box0.y + dy - tol);
        }
    } else if (dx != 0.0f) {
        REQUIRE(std::abs(r1.box.y - box0.y) <= max_assist + tol);  // C2
    } else {
        REQUIRE(r1.box.y == box0.y);
    }

    // C4: enabled=false E max_assist_fraction=0 sao byte-identicos ao
    // resolve_move puro - PARA ESTE MESMO CASO (nao so um exemplo isolado:
    // roda em toda a varredura, mata mutante que trocasse a condicao do
    // flag por outra coisa "quase equivalente").
    const MoveResult base = resolve_move(grid, box0, dx, dy, span);

    CornerAssistOptions off_opts = opts;
    off_opts.enabled = false;
    const MoveResult r_off =
        resolve_move_with_corner_assist(grid, box0, dx, dy, off_opts, span);
    REQUIRE(r_off.box.x == base.box.x);
    REQUIRE(r_off.box.y == base.box.y);
    REQUIRE(r_off.hit_x == base.hit_x);
    REQUIRE(r_off.hit_y == base.hit_y);

    CornerAssistOptions zero_opts = opts;
    zero_opts.max_assist_fraction = 0.0f;
    const MoveResult r_zero =
        resolve_move_with_corner_assist(grid, box0, dx, dy, zero_opts, span);
    REQUIRE(r_zero.box.x == base.box.x);
    REQUIRE(r_zero.box.y == base.box.y);
    REQUIRE(r_zero.hit_x == base.hit_x);
    REQUIRE(r_zero.hit_y == base.hit_y);
}

}  // namespace

// ---------------------------------------------------------------------------
// TIER ESTRITO - devem passar SEMPRE.
// ---------------------------------------------------------------------------

TEST_CASE("corner_assist_sweep: quinas nas 4 orientacoes, desalinhamento "
          "sub-pixel 0 a 0.6 tile (tier estrito, C1-C4)",
          "[core][spatial][corner][sweep]") {
    std::size_t checked = 0;
    const CornerAssistOptions opts{};  // defaults: enabled=true, fraction=0.35f
    for (const Orientation& orient : orientations()) {
        for (const float tile : kTileSizes) {
            const TileGrid grid = fixture_center_corner(tile);
            const float box_size = kBoxTileFraction * tile;
            for (const float off : misalignments(tile)) {
                const Aabb start =
                    start_for_orientation(orient, tile, box_size, off);
                for (const float frac : cardinal_magnitude_fractions()) {
                    const float m = frac * tile;
                    const float dx = orient.dx_sign * m;
                    const float dy = orient.dy_sign * m;
                    check_corner_assist_invariants(grid, start, dx, dy, nullptr,
                                                   0, tile, opts, orient.name);
                    ++checked;
                }
            }
        }
    }
    INFO("casos de quina (4 orientacoes) verificados: " << checked);
    REQUIRE(checked > 0);
}

TEST_CASE("corner_assist_sweep: obstaculo de 1 tile colado na quina, offsets "
          "sub-pixel (tier estrito, C1-C4)",
          "[core][spatial][corner][sweep][obstacle]") {
    // Mesma geometria das 4 orientacoes, mas o bloqueador e um OBSTACULO
    // pontual (1.0*tile) colado na posicao da quina, numa grade totalmente
    // livre - o assist tambem nao pode empurrar atraves dele nem pra dentro
    // dele (mesmas invariantes, agora contra ObstacleSpan).
    std::size_t checked = 0;
    const CornerAssistOptions opts{};
    for (const Orientation& orient : orientations()) {
        for (const float tile : kTileSizes) {
            const TileGrid grid(5, 5, tile);  // toda livre.
            const Aabb obstacle{1.0f * tile, 1.0f * tile, tile, tile};
            const float box_size = kBoxTileFraction * tile;
            for (const float off : misalignments(tile)) {
                const Aabb start =
                    start_for_orientation(orient, tile, box_size, off);
                for (const float frac : cardinal_magnitude_fractions()) {
                    const float m = frac * tile;
                    const float dx = orient.dx_sign * m;
                    const float dy = orient.dy_sign * m;
                    check_corner_assist_invariants(grid, start, dx, dy,
                                                   &obstacle, 1, tile, opts,
                                                   orient.name);
                    ++checked;
                }
            }
        }
    }
    INFO("casos de obstaculo-na-quina verificados: " << checked);
    REQUIRE(checked > 0);
}

TEST_CASE("corner_assist_sweep: amostragem PRNG deterministica de 10000 "
          "casos (tier estrito, C1-C4)",
          "[core][spatial][corner][sweep][random]") {
    std::minstd_rand rng(kSweepSeed);
    std::uniform_int_distribution<int> orient_pick(
        0, static_cast<int>(orientations().size()) - 1);
    std::uniform_int_distribution<int> tile_pick(
        0, static_cast<int>(std::size(kTileSizes)) - 1);
    std::uniform_real_distribution<float> unit(-1.0f, 1.0f);
    std::uniform_real_distribution<float> unit01(0.0f, 1.0f);
    std::uniform_int_distribution<int> use_obstacle_pick(0, 1);

    const CornerAssistOptions opts{};
    constexpr int kSamples = 10000;
    int accepted = 0;
    int rejected = 0;
    for (int i = 0; i < kSamples; ++i) {
        const Orientation& orient =
            orientations()[static_cast<std::size_t>(orient_pick(rng))];
        const float tile = kTileSizes[static_cast<std::size_t>(tile_pick(rng))];
        const float box_size = kBoxTileFraction * tile;
        const bool use_obstacle = use_obstacle_pick(rng) == 1;

        TileGrid grid = use_obstacle ? TileGrid(5, 5, tile)
                                     : fixture_center_corner(tile);
        Aabb obstacle{1.0f * tile, 1.0f * tile, tile, tile};
        const Aabb* obs_ptr = use_obstacle ? &obstacle : nullptr;
        const int obs_count = use_obstacle ? 1 : 0;

        const float off = unit(rng) * 0.6f * tile;
        const Aabb start = start_for_orientation(orient, tile, box_size, off);
        const float m = (unit01(rng) * 0.29f + 0.01f) * tile;  // (0.01,0.30]*tile
        const float dx = orient.dx_sign * m;
        const float dy = orient.dy_sign * m;

        if (!is_valid_start(grid, start, obs_ptr, obs_count, kTolFraction * tile)) {
            ++rejected;
            continue;
        }
        ++accepted;
        check_corner_assist_invariants(grid, start, dx, dy, obs_ptr, obs_count,
                                       tile, opts, orient.name);
    }
    INFO("PRNG seed=" << kSweepSeed << " aceitos=" << accepted
                       << " rejeitados=" << rejected);
    REQUIRE(accepted > 0);
}

TEST_CASE("corner_assist_sweep: C5 - teto seguro contra bloqueador de grade "
          ">= 1 tile de espessura (tier estrito)",
          "[core][spatial][corner][sweep][tuning]") {
    // Bloqueador de grade com espessura T tiles (T=1,2,3) diretamente na
    // MESMA coluna/linha onde a caixa esta antes do empurrao (o eixo do
    // empurrao perpendicular); a caixa comeca colada na face do bloqueador
    // (pior caso: menor distancia possivel ate o bloqueador). O CTO pediu as
    // fracoes {0.35 default, 0.5, 0.75, 1.0} contra thickness >= 1 tile -
    // verificamos que o CENTRO da caixa nunca cruza o intervalo ocupado pelo
    // bloqueador (crossed_band) para nenhuma dessas fracoes.
    static constexpr float kFractions[] = {0.35f, 0.5f, 0.75f, 1.0f};
    static constexpr int kThicknesses[] = {1, 2, 3};

    std::size_t checked = 0;
    for (const Orientation& orient : orientations()) {
        for (const float tile : kTileSizes) {
            const float box_size = kBoxTileFraction * tile;
            for (const int thickness : kThicknesses) {
                // Grade: bloqueador "ahead" (causa hit no eixo principal) na
                // celula (1,1) [ou linha/coluna analoga conforme
                // orientacao], e um bloqueador PERPENDICULAR de `thickness`
                // tiles colado logo depois dele, na mesma coluna/linha onde a
                // caixa comeca (o alvo do empurrao). Construida em coordenadas
                // absolutas (grid width/height cobrem tudo).
                const int grid_w = (orient.dx_sign != 0.0f) ? 3 : (3 + thickness);
                const int grid_h = (orient.dy_sign != 0.0f) ? 3 : (3 + thickness);
                TileGrid grid(std::max(grid_w, 4), std::max(grid_h, 4), tile);
                grid.set_blocked(1, 1, true);  // "ahead": causa hit no eixo principal.

                // Coluna/linha 0 (leste/oeste) ou 0 (norte/sul): banda extra
                // perpendicular, colada logo apos a celula central, na mesma
                // faixa onde a caixa comeca (permite testar se o empurrao
                // consegue pular por cima dela).
                float band_lo = 0.0f;
                float band_hi = 0.0f;
                if (orient.dx_sign != 0.0f) {
                    // empurrao no eixo Y: banda ocupa linhas 2..(1+thickness)
                    // na coluna 1 (mesma coluna do bloqueador ahead, mas o que
                    // importa e a coluna onde a caixa fica DURANTE o
                    // empurrao, que e a coluna de origem - usamos a coluna 0
                    // por simplicidade, replicando o bloqueio la tambem).
                    for (int row = 2; row < 2 + thickness; ++row) {
                        grid.set_blocked(0, row, true);
                        grid.set_blocked(1, row, true);
                    }
                    band_lo = 2.0f * tile;
                    band_hi = (2.0f + static_cast<float>(thickness)) * tile;
                } else {
                    for (int col = 2; col < 2 + thickness; ++col) {
                        grid.set_blocked(col, 0, true);
                        grid.set_blocked(col, 1, true);
                    }
                    band_lo = 2.0f * tile;
                    band_hi = (2.0f + static_cast<float>(thickness)) * tile;
                }

                const Aabb start =
                    start_for_orientation(orient, tile, box_size, 0.0f);
                if (!is_valid_start(grid, start, nullptr, 0, kTolFraction * tile)) {
                    continue;
                }

                for (const float frac : kFractions) {
                    CornerAssistOptions opts;
                    opts.max_assist_fraction = frac;
                    const ObstacleSpan span{};
                    const MoveResult r = resolve_move_with_corner_assist(
                        grid, start, orient.dx_sign * (0.30f * tile),
                        orient.dy_sign * (0.30f * tile), opts, span);

                    const float center_before = (orient.dx_sign != 0.0f)
                                                    ? start.y + box_size * 0.5f
                                                    : start.x + box_size * 0.5f;
                    const float center_after = (orient.dx_sign != 0.0f)
                                                   ? r.box.y + box_size * 0.5f
                                                   : r.box.x + box_size * 0.5f;
                    const bool crossed =
                        crossed_band(center_before, center_after, band_lo, band_hi);

                    CAPTURE(orient.name, tile, thickness, frac, start.x, start.y,
                            r.box.x, r.box.y, center_before, center_after,
                            band_lo, band_hi);
                    REQUIRE_FALSE(crossed);
                    ++checked;
                }
            }
        }
    }
    INFO("casos C5 (bloqueador grade >= 1 tile) verificados: " << checked);
    REQUIRE(checked > 0);
}

// ---------------------------------------------------------------------------
// TIER ADVERSARIAL (caracterizacao) - NAO bloqueia CI.
// ---------------------------------------------------------------------------

TEST_CASE("corner_assist_sweep: obstaculo FINO (mais fino que a caixa) - "
          "salto por cima do bloqueador - caracterizacao (tier adversarial)",
          "[core][spatial][corner][sweep][!mayfail]") {
    // find_corner_push (grid_collision.cpp linhas 166-217) so valida o
    // DESTINO de cada candidato de empurrao - o loop de magnitude crescente
    // NAO para quando um candidato esta bloqueado, ele so continua tentando
    // o proximo. Se existir espaco livre do OUTRO LADO de um obstaculo fino
    // (mais fino que a caixa, 0.6*tile) dentro de max_assist, o empurrao
    // "pousa" la sem nunca ter sido barrado no meio do caminho - o centro da
    // caixa CRUZA o obstaculo. Matematica esperada (ver cabecalho do
    // arquivo): empurrao minimo para cruzar ~= box_size + espessura_fina,
    // que fica entre 0.65*tile e 0.9*tile para thickness em [0.05,0.30]*tile
    // - DENTRO do max_assist_fraction ate 1.5 pedido (e ate MENOR que 1.0).
    static constexpr float kThinFractions[] = {0.05f, 0.10f, 0.15f,
                                               0.20f, 0.25f, 0.30f};
    static constexpr float kAssistFractions[] = {0.35f, 0.5f,  0.65f,
                                                 0.75f, 0.9f,  1.0f,
                                                 1.25f, 1.5f};

    int first_crossed_fraction_index = -1;
    float min_crossing_fraction = 999.0f;
    for (const float tile : kTileSizes) {
        const float box_size = kBoxTileFraction * tile;
        for (const float thin_frac : kThinFractions) {
            const float thin = thin_frac * tile;

            // grade totalmente livre - so obstaculos (A = "ahead", causa
            // hit; B = fino, o alvo do teste) definem a geometria.
            TileGrid grid(6, 6, tile);
            const float r0 = 2.0f * tile;  // linha base (buffer arbitrario).
            const Aabb obstacle_ahead{1.0f * tile, r0, tile, tile};
            const Aabb obstacle_thin{0.0f, r0 + tile, tile, thin};
            const Aabb obstacles[2] = {obstacle_ahead, obstacle_thin};

            // caixa colada na face esquerda do obstaculo ahead, E colada no
            // topo do obstaculo fino (pior caso: menor distancia possivel).
            const Aabb start{1.0f * tile - box_size, r0 + tile - box_size,
                             box_size, box_size};
            REQUIRE(is_valid_start(grid, start, obstacles, 2, kTolFraction * tile));

            for (const float assist_frac : kAssistFractions) {
                CornerAssistOptions opts;
                opts.max_assist_fraction = assist_frac;
                const ObstacleSpan span{obstacles, 2};
                const MoveResult r = resolve_move_with_corner_assist(
                    grid, start, 0.30f * tile, 0.0f, opts, span);

                const float band_lo = obstacle_thin.y;
                const float band_hi = obstacle_thin.y + obstacle_thin.h;
                const float center_before = start.y + box_size * 0.5f;
                const float center_after = r.box.y + box_size * 0.5f;
                const bool crossed =
                    crossed_band(center_before, center_after, band_lo, band_hi);
                const bool penetrated =
                    obstacles_penetration_depth(r.box, obstacles, 2) >
                    kTolFraction * tile;

                CAPTURE(tile, thin_frac, assist_frac, start.y, r.box.y,
                        center_before, center_after, band_lo, band_hi,
                        penetrated);
                // ACHADO ESPERADO: para assist_frac grande o bastante
                // (aproximadamente >= 0.6 + thin_frac), o assist "salta" por
                // cima do obstaculo fino (crossed vira true) SEM nunca ter
                // penetrado nele (penetrated fica false - destino limpo dos
                // dois lados, exatamente a assinatura do exploit). Usa CHECK
                // (nao REQUIRE) pra nao abortar o loop e caracterizar TODAS as
                // combinacoes num unico relatorio.
                CHECK_FALSE(crossed);

                if (crossed && assist_frac < min_crossing_fraction) {
                    min_crossing_fraction = assist_frac;
                    first_crossed_fraction_index = 1;
                }
            }
        }
    }
    if (first_crossed_fraction_index >= 0) {
        INFO("menor max_assist_fraction onde o salto por cima do obstaculo "
             "fino foi observado: " << min_crossing_fraction);
    } else {
        INFO("nenhum salto observado ate max_assist_fraction=1.5 nesta "
             "varredura (thin ate 0.30*tile, box_size 0.6*tile)");
    }
}

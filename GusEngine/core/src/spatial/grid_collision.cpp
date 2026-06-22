// gus/core/spatial/grid_collision.cpp
// Colisao AABB-desliza-na-grade (M4). Ver grid_collision.hpp para o contrato e
// o feel (deslizar, resolucao por eixo). Movimento cinematico puro.

#include "gus/core/spatial/grid_collision.hpp"

#include <cmath>

#include "gus/core/spatial/tile_grid.hpp"

namespace gus::core::spatial {

namespace {

// Verifica se a caixa em (px,py) com (w,h) sobrepoe ALGUMA celula bloqueada.
// Usa um epsilon ao calcular a celula da borda direita/inferior: a borda em
// p+size e EXCLUSIVA (a caixa encosta na parede sem entrar nela), entao
// subtraimos um epsilon antes do floor para nao "vazar" para a celula seguinte.
bool overlaps_blocked(const TileGrid& grid, float px, float py, float w,
                      float h) noexcept {
    constexpr float kEdgeEps = 1e-4f;

    const int cx0 = grid.world_to_cell(px);
    const int cy0 = grid.world_to_cell(py);
    const int cx1 = grid.world_to_cell(px + w - kEdgeEps);
    const int cy1 = grid.world_to_cell(py + h - kEdgeEps);

    for (int cy = cy0; cy <= cy1; ++cy) {
        for (int cx = cx0; cx <= cx1; ++cx) {
            if (grid.is_blocked(cx, cy)) {
                return true;
            }
        }
    }
    return false;
}

// Resolve o movimento em UM eixo. Recebe a posicao corrente da caixa
// (cur_x,cur_y) e tenta deslocar a coordenada `moving` (x ou y) por `delta`.
// Se a posicao alvo sobrepoe parede, encosta a caixa exatamente na borda da
// parede (resolucao por borda de celula) e marca hit. Retorna a nova coordenada
// do eixo movido; `hit` recebe se houve colisao.
struct AxisOutcome {
    float coord = 0.0f;
    bool hit = false;
};

// Resolve no eixo X. cur_x/cur_y = posicao atual; tile = tamanho da celula.
AxisOutcome resolve_x(const TileGrid& grid, float cur_x, float cur_y, float w,
                      float h, float dx) noexcept {
    AxisOutcome out{cur_x, false};
    if (dx == 0.0f) {
        return out;
    }
    const float target = cur_x + dx;
    if (!overlaps_blocked(grid, target, cur_y, w, h)) {
        out.coord = target;
        return out;
    }

    // Colidiu: encosta na borda da parede. tile_size > 0 garantido pela grade.
    const float tile = grid.tile_size();
    out.hit = true;
    if (dx > 0.0f) {
        // Movendo para a direita: a borda direita (x+w) bate na face esquerda
        // da primeira coluna bloqueada alcancada. Essa face e cell*tile.
        const int blocking_cell = grid.world_to_cell(target + w - 1e-4f);
        out.coord = static_cast<float>(blocking_cell) * tile - w;
    } else {
        // Movendo para a esquerda: a borda esquerda (x) bate na face direita da
        // coluna bloqueada, que e (cell+1)*tile.
        const int blocking_cell = grid.world_to_cell(target);
        out.coord = static_cast<float>(blocking_cell + 1) * tile;
    }
    return out;
}

// Resolve no eixo Y, dado o X ja resolvido (fixed_x).
AxisOutcome resolve_y(const TileGrid& grid, float fixed_x, float cur_y, float w,
                      float h, float dy) noexcept {
    AxisOutcome out{cur_y, false};
    if (dy == 0.0f) {
        return out;
    }
    const float target = cur_y + dy;
    if (!overlaps_blocked(grid, fixed_x, target, w, h)) {
        out.coord = target;
        return out;
    }

    const float tile = grid.tile_size();
    out.hit = true;
    if (dy > 0.0f) {
        const int blocking_cell = grid.world_to_cell(target + h - 1e-4f);
        out.coord = static_cast<float>(blocking_cell) * tile - h;
    } else {
        const int blocking_cell = grid.world_to_cell(target);
        out.coord = static_cast<float>(blocking_cell + 1) * tile;
    }
    return out;
}

}  // namespace

MoveResult resolve_move(const TileGrid& grid, const Aabb& box, float dx,
                        float dy) noexcept {
    // Eixo X primeiro, a partir da posicao corrente.
    const AxisOutcome rx = resolve_x(grid, box.x, box.y, box.w, box.h, dx);
    // Eixo Y depois, com o X ja resolvido (slide).
    const AxisOutcome ry = resolve_y(grid, rx.coord, box.y, box.w, box.h, dy);

    MoveResult result;
    result.box = Aabb{rx.coord, ry.coord, box.w, box.h};
    result.hit_x = rx.hit;
    result.hit_y = ry.hit;
    return result;
}

}  // namespace gus::core::spatial

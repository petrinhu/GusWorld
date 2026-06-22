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

// Procura o MENOR empurrao perpendicular (em modulo) que destrava o eixo
// principal, dentro de [0, max_assist]. Varredura fina deterministica (passo
// tile/64): pra cada sentido (+ e -) testa empurroes crescentes; um empurrao
// candidato `p` so vale se (a) a posicao empurrada NAO esta em parede (o empurrao
// em si e livre) E (b) a partir dela o passo do eixo principal deixa de bater.
// Devolve o empurrao com menor |p| (preferindo o lado que destrava antes); 0 se
// nenhum destrava. `horizontal` = true se o movimento principal e em X (empurra em
// Y); false se e em Y (empurra em X). `main_delta` = dx ou dy (o sinal do passo).
float find_corner_push(const TileGrid& grid, const Aabb& box, bool horizontal,
                       float main_delta, float max_assist) noexcept {
    if (max_assist <= 0.0f) {
        return 0.0f;
    }
    const float tile = grid.tile_size();
    float step = tile / 64.0f;
    if (step <= 0.0f) {
        return 0.0f;
    }

    // Testa se, empurrando o eixo perpendicular por `push`, o eixo principal
    // passa a estar livre (sem sobrepor parede) E o empurrao em si e livre.
    auto destrava = [&](float push) -> bool {
        const float bx = horizontal ? box.x : box.x + push;
        const float by = horizontal ? box.y + push : box.y;
        // (a) o empurrao perpendicular nao pode meter o corpo numa parede.
        if (overlaps_blocked(grid, bx, by, box.w, box.h)) {
            return false;
        }
        // (b) a partir da posicao empurrada, o passo do eixo principal e livre.
        const float tx = horizontal ? bx + main_delta : bx;
        const float ty = horizontal ? by : by + main_delta;
        return !overlaps_blocked(grid, tx, ty, box.w, box.h);
    };

    // Varre |p| crescente; nos dois sentidos no mesmo nivel, pegando o que
    // destravar primeiro (menor modulo). Empate (ambos no mesmo |p|): prefere +.
    for (float mag = step; mag <= max_assist + 1e-4f; mag += step) {
        if (destrava(+mag)) {
            return +mag;
        }
        if (destrava(-mag)) {
            return -mag;
        }
    }
    return 0.0f;
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

MoveResult resolve_move_with_corner_assist(const TileGrid& grid, const Aabb& box,
                                           float dx, float dy,
                                           const CornerAssistOptions& opts) noexcept {
    // Resolve normal primeiro: e a base e tambem o fallback se nada destravar.
    const MoveResult base = resolve_move(grid, box, dx, dy);
    if (!opts.enabled || opts.max_assist_fraction <= 0.0f) {
        return base;
    }

    const float max_assist = opts.max_assist_fraction * grid.tile_size();

    // Corner-assist so atua em movimento CARDINAL (um eixo) que bateu. Em
    // diagonal a resolucao por eixo do resolve_move ja contorna quinas isoladas
    // (mantemos o comportamento da base; ver nota no header). Detecta o eixo:
    const bool horizontal = (dx != 0.0f && dy == 0.0f && base.hit_x);
    const bool vertical = (dy != 0.0f && dx == 0.0f && base.hit_y);
    if (!horizontal && !vertical) {
        return base;
    }

    const float main_delta = horizontal ? dx : dy;
    const float push = find_corner_push(grid, box, horizontal, main_delta, max_assist);
    if (push == 0.0f) {
        return base;  // nenhuma abertura dentro do limite: trava como hoje
    }

    // Aplica o empurrao perpendicular e resolve o movimento a partir dali. O
    // resolve_move encosta o eixo perpendicular na borda da celula (snap limpo) e
    // leva o eixo principal pela abertura agora livre.
    const Aabb pushed = horizontal ? Aabb{box.x, box.y + push, box.w, box.h}
                                   : Aabb{box.x + push, box.y, box.w, box.h};
    if (horizontal) {
        // Resolve Y (o empurrao, encostando na borda) e depois X (o movimento).
        const AxisOutcome ry = resolve_y(grid, pushed.x, box.y, box.w, box.h, push);
        const AxisOutcome rx = resolve_x(grid, pushed.x, ry.coord, box.w, box.h, main_delta);
        MoveResult result;
        result.box = Aabb{rx.coord, ry.coord, box.w, box.h};
        result.hit_x = rx.hit;
        result.hit_y = false;  // o empurrao perpendicular nao e "bater"
        return result;
    }
    // vertical: empurra em X, depois move em Y.
    const AxisOutcome rx = resolve_x(grid, box.x, pushed.y, box.w, box.h, push);
    const AxisOutcome ry = resolve_y(grid, rx.coord, box.y, box.w, box.h, main_delta);
    MoveResult result;
    result.box = Aabb{rx.coord, ry.coord, box.w, box.h};
    result.hit_x = false;
    result.hit_y = ry.hit;
    return result;
}

}  // namespace gus::core::spatial

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

// true se os retangulos (px,py,w,h) e o obstaculo `ob` se sobrepoem (AABB-vs-AABB
// comum, meio-aberto - NAO e alinhado a grade, entao sem o epsilon de celula de
// overlaps_blocked acima).
bool overlaps_aabb(float px, float py, float w, float h, const Aabb& ob) noexcept {
    return px < ob.x + ob.w && px + w > ob.x && py < ob.y + ob.h && py + h > ob.y;
}

// Resolve no eixo X. cur_x/cur_y = posicao atual; tile = tamanho da celula.
// `obstacles` (opcional): obstaculos PONTUAIS adicionais (NPC/inimigo parado - ver
// ObstacleSpan no header), tratados como paredes extras, avaliados TODOS contra o
// alvo CRU (target) - a grade e cada obstaculo sao bloqueadores independentes; o
// candidato de encosto MAIS RESTRITIVO (menor deslocamento a partir de cur_x)
// vence, ordem-independente e deterministico.
AxisOutcome resolve_x(const TileGrid& grid, float cur_x, float cur_y, float w,
                      float h, float dx, ObstacleSpan obstacles = {}) noexcept {
    AxisOutcome out{cur_x, false};
    if (dx == 0.0f) {
        return out;
    }
    const float target = cur_x + dx;
    float best = target;  // so vale se NENHUM bloqueador (hit permanece false).
    bool hit = false;

    if (overlaps_blocked(grid, target, cur_y, w, h)) {
        // Colidiu com a GRADE: candidato de encosto na borda da parede. tile_size >
        // 0 garantido pela grade.
        const float tile = grid.tile_size();
        float wall_candidate;
        if (dx > 0.0f) {
            // Movendo para a direita: a borda direita (x+w) bate na face esquerda
            // da primeira coluna bloqueada alcancada. Essa face e cell*tile.
            const int blocking_cell = grid.world_to_cell(target + w - 1e-4f);
            wall_candidate = static_cast<float>(blocking_cell) * tile - w;
        } else {
            // Movendo para a esquerda: a borda esquerda (x) bate na face direita da
            // coluna bloqueada, que e (cell+1)*tile.
            const int blocking_cell = grid.world_to_cell(target);
            wall_candidate = static_cast<float>(blocking_cell + 1) * tile;
        }
        best = wall_candidate;
        hit = true;
    }

    // Obstaculos pontuais: cada um que colidiria com o alvo CRU contribui seu
    // proprio candidato de encosto (borda REAL do obstaculo, nao alinhada a
    // celula); fica-se com o MAIS RESTRITIVO entre TODOS ja vistos (parede +
    // obstaculos).
    for (int i = 0; i < obstacles.count; ++i) {
        const Aabb& ob = obstacles.items[i];
        if (!overlaps_aabb(target, cur_y, w, h, ob)) {
            continue;
        }
        const float candidate = (dx > 0.0f) ? (ob.x - w) : (ob.x + ob.w);
        if (!hit || (dx > 0.0f ? candidate < best : candidate > best)) {
            best = candidate;
            hit = true;
        }
    }

    out.coord = best;
    out.hit = hit;
    return out;
}

// Resolve no eixo Y, dado o X ja resolvido (fixed_x). `obstacles` (opcional): ver
// resolve_x acima (mesma tecnica, so que no eixo Y).
AxisOutcome resolve_y(const TileGrid& grid, float fixed_x, float cur_y, float w,
                      float h, float dy, ObstacleSpan obstacles = {}) noexcept {
    AxisOutcome out{cur_y, false};
    if (dy == 0.0f) {
        return out;
    }
    const float target = cur_y + dy;
    float best = target;
    bool hit = false;

    if (overlaps_blocked(grid, fixed_x, target, w, h)) {
        const float tile = grid.tile_size();
        float wall_candidate;
        if (dy > 0.0f) {
            const int blocking_cell = grid.world_to_cell(target + h - 1e-4f);
            wall_candidate = static_cast<float>(blocking_cell) * tile - h;
        } else {
            const int blocking_cell = grid.world_to_cell(target);
            wall_candidate = static_cast<float>(blocking_cell + 1) * tile;
        }
        best = wall_candidate;
        hit = true;
    }

    for (int i = 0; i < obstacles.count; ++i) {
        const Aabb& ob = obstacles.items[i];
        if (!overlaps_aabb(fixed_x, target, w, h, ob)) {
            continue;
        }
        const float candidate = (dy > 0.0f) ? (ob.y - h) : (ob.y + ob.h);
        if (!hit || (dy > 0.0f ? candidate < best : candidate > best)) {
            best = candidate;
            hit = true;
        }
    }

    out.coord = best;
    out.hit = hit;
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
// `obstacles` (opcional): obstaculos pontuais - o empurrao tambem NAO pode meter o
// corpo num obstaculo, e o passo do eixo principal so "destrava" se estiver livre
// da grade E dos obstaculos (mesmo criterio de resolve_x/resolve_y).
float find_corner_push(const TileGrid& grid, const Aabb& box, bool horizontal,
                       float main_delta, float max_assist,
                       ObstacleSpan obstacles = {}) noexcept {
    if (max_assist <= 0.0f) {
        return 0.0f;
    }
    const float tile = grid.tile_size();
    float step = tile / 64.0f;
    if (step <= 0.0f) {
        return 0.0f;
    }

    const auto blocked_here = [&](float px, float py) -> bool {
        if (overlaps_blocked(grid, px, py, box.w, box.h)) {
            return true;
        }
        for (int i = 0; i < obstacles.count; ++i) {
            if (overlaps_aabb(px, py, box.w, box.h, obstacles.items[i])) {
                return true;
            }
        }
        return false;
    };

    // Testa se, empurrando o eixo perpendicular por `push`, o eixo principal
    // passa a estar livre (sem sobrepor parede/obstaculo) E o empurrao em si e
    // livre.
    auto destrava = [&](float push) -> bool {
        const float bx = horizontal ? box.x : box.x + push;
        const float by = horizontal ? box.y + push : box.y;
        // (a) o empurrao perpendicular nao pode meter o corpo numa parede/obstaculo.
        if (blocked_here(bx, by)) {
            return false;
        }
        // (b) a partir da posicao empurrada, o passo do eixo principal e livre.
        const float tx = horizontal ? bx + main_delta : bx;
        const float ty = horizontal ? by : by + main_delta;
        return !blocked_here(tx, ty);
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

MoveResult resolve_move(const TileGrid& grid, const Aabb& box, float dx, float dy,
                        ObstacleSpan obstacles) noexcept {
    // Eixo X primeiro, a partir da posicao corrente.
    const AxisOutcome rx = resolve_x(grid, box.x, box.y, box.w, box.h, dx, obstacles);
    // Eixo Y depois, com o X ja resolvido (slide).
    const AxisOutcome ry =
        resolve_y(grid, rx.coord, box.y, box.w, box.h, dy, obstacles);

    MoveResult result;
    result.box = Aabb{rx.coord, ry.coord, box.w, box.h};
    result.hit_x = rx.hit;
    result.hit_y = ry.hit;
    return result;
}

MoveResult resolve_move_with_corner_assist(const TileGrid& grid, const Aabb& box,
                                           float dx, float dy,
                                           const CornerAssistOptions& opts,
                                           ObstacleSpan obstacles) noexcept {
    // Resolve normal primeiro: e a base e tambem o fallback se nada destravar.
    const MoveResult base = resolve_move(grid, box, dx, dy, obstacles);
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
    const float push =
        find_corner_push(grid, box, horizontal, main_delta, max_assist, obstacles);
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
        const AxisOutcome ry =
            resolve_y(grid, pushed.x, box.y, box.w, box.h, push, obstacles);
        const AxisOutcome rx = resolve_x(grid, pushed.x, ry.coord, box.w, box.h,
                                         main_delta, obstacles);
        MoveResult result;
        result.box = Aabb{rx.coord, ry.coord, box.w, box.h};
        result.hit_x = rx.hit;
        result.hit_y = false;  // o empurrao perpendicular nao e "bater"
        return result;
    }
    // vertical: empurra em X, depois move em Y.
    const AxisOutcome rx =
        resolve_x(grid, box.x, pushed.y, box.w, box.h, push, obstacles);
    const AxisOutcome ry = resolve_y(grid, rx.coord, box.y, box.w, box.h, main_delta,
                                     obstacles);
    MoveResult result;
    result.box = Aabb{rx.coord, ry.coord, box.w, box.h};
    result.hit_x = false;
    result.hit_y = ry.hit;
    return result;
}

}  // namespace gus::core::spatial

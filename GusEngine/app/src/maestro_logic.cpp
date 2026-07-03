// gus/app/src/maestro_logic.cpp
//
// Implementacao da logica PURA da Maestro (M7-COSTURA). Ver header - travado por
// app/tests/maestro_logic_test.cpp (TEST-FIRST, headless, sem SDL/janela).

#include "gus/app/maestro_logic.hpp"

#include <cmath>   // std::abs
#include <cstddef> // std::size_t
#include <utility> // std::pair
#include <vector>

#include "gus/app/screens/sprite_anchor.hpp"  // sprite_top_y - MESMA formula do desenho

namespace gus::app {

namespace {

// BUG real (lider, playtest ao vivo): a espiral antiga parava na PRIMEIRA celula
// LIVRE (is_blocked==false) mais proxima do alvo, mas "livre" != "alcancavel a pe" -
// uma celula pode ser chao solto numa SALA FECHADA, isolada do spawn por paredes (o
// caso de distritos_inferiores.gmap: o offset caia numa sala sem porta). Corrigido
// filtrando por ALCANCABILIDADE (mesma componente conectada do spawn), nao so
// ausencia de parede.
//
// Flood-fill 4-conectado (mesma nocao de "andavel" que o movimento real do jogador
// usa: TileGrid::is_blocked, consumida por resolve_move[_with_corner_assist] em
// overworld_sim.cpp - MESMA fonte de verdade, sem divergencia). Devolve um vetor
// flat width*height (row-major, index = y*width+x); true = alcancavel a pe partindo
// de (start_cx,start_cy). Se a propria celula de partida estiver fora dos limites ou
// bloqueada (nao deveria acontecer - e onde o jogador nasceu), devolve tudo false.
// Deterministico (BFS classico, ordem fixa de vizinhos E/S/O/N).
std::vector<bool> flood_fill_reachable(const gus::core::spatial::TileGrid& grid,
                                        int start_cx, int start_cy) {
    const int w = grid.width();
    const int h = grid.height();
    std::vector<bool> reachable(static_cast<std::size_t>(w) * static_cast<std::size_t>(h),
                                 false);
    if (w <= 0 || h <= 0) {
        return reachable;
    }
    if (start_cx < 0 || start_cx >= w || start_cy < 0 || start_cy >= h) {
        return reachable;
    }
    if (grid.is_blocked(start_cx, start_cy)) {
        return reachable;
    }

    auto index = [w](int x, int y) -> std::size_t {
        return static_cast<std::size_t>(y) * static_cast<std::size_t>(w) +
               static_cast<std::size_t>(x);
    };

    std::vector<std::pair<int, int>> queue;
    queue.reserve(static_cast<std::size_t>(w) * static_cast<std::size_t>(h));
    reachable[index(start_cx, start_cy)] = true;
    queue.emplace_back(start_cx, start_cy);

    static constexpr int kDx[4] = {1, 0, -1, 0};  // E, S, O, N - ordem fixa
    static constexpr int kDy[4] = {0, 1, 0, -1};

    for (std::size_t head = 0; head < queue.size(); ++head) {
        const auto [cx, cy] = queue[head];
        for (int i = 0; i < 4; ++i) {
            const int nx = cx + kDx[i];
            const int ny = cy + kDy[i];
            if (nx < 0 || nx >= w || ny < 0 || ny >= h) {
                continue;
            }
            if (reachable[index(nx, ny)] || grid.is_blocked(nx, ny)) {
                continue;
            }
            reachable[index(nx, ny)] = true;
            queue.emplace_back(nx, ny);
        }
    }
    return reachable;
}

}  // namespace

bool aabb_overlaps(const gus::core::spatial::Aabb& a,
                    const gus::core::spatial::Aabb& b) noexcept {
    return a.x < b.x + b.w && a.x + a.w > b.x && a.y < b.y + b.h && a.y + a.h > b.y;
}

bool should_trigger_battle(const gus::core::spatial::Aabb& player,
                            const gus::core::spatial::Aabb& enemy,
                            bool enemy_defeated) noexcept {
    if (enemy_defeated) {
        return false;
    }
    return aabb_overlaps(player, enemy);
}

bool outcome_marks_enemy_defeated(
    gus::domain::combat::CombatOutcome outcome) noexcept {
    return outcome == gus::domain::combat::CombatOutcome::Victory;
}

gus::core::spatial::Aabb pick_fixed_enemy_position(
    const gus::core::spatial::TileGrid& grid,
    const gus::core::spatial::Aabb& player_start, int offset_tiles_x,
    int offset_tiles_y) noexcept {
    const float ts = grid.tile_size() > 0.0f ? grid.tile_size() : 1.0f;

    const int spawn_cx = grid.world_to_cell(player_start.x + player_start.w * 0.5f);
    const int spawn_cy = grid.world_to_cell(player_start.y + player_start.h * 0.5f);
    const int target_cx = spawn_cx + offset_tiles_x;
    const int target_cy = spawn_cy + offset_tiles_y;

    // Alcancabilidade PRIMEIRO (flood-fill a partir do spawn) - so entao a espiral
    // escolhe, DENTRE as celulas alcancaveis, a mais proxima do alvo. Isto e o que
    // garante que o inimigo nunca cai numa sala fechada/isolada (ver comentario do
    // flood_fill_reachable acima).
    const std::vector<bool> reachable = flood_fill_reachable(grid, spawn_cx, spawn_cy);
    const int grid_w = grid.width();
    const int grid_h = grid.height();
    const auto is_reachable = [&](int cx, int cy) -> bool {
        if (cx < 0 || cx >= grid_w || cy < 0 || cy >= grid_h) {
            return false;
        }
        return reachable[static_cast<std::size_t>(cy) * static_cast<std::size_t>(grid_w) +
                          static_cast<std::size_t>(cx)];
    };

    // Espiral de raio crescente ao redor do alvo (varre o quadrado [-r,r]x[-r,r] a cada
    // raio, mas so os pontos NOVOS daquele anel - determinismo total: sempre varre na
    // MESMA ordem). Para na primeira celula ALCANCAVEL (nao so "sem parede"). Raio
    // maximo cobre a grade inteira (w+h e suficiente pra alcancar qualquer celula a
    // partir de qualquer alvo, dentro ou fora dos limites do mapa).
    const int max_radius = grid_w + grid_h;
    int best_cx = spawn_cx;  // fallback: a propria celula de spawn (garantida alcancavel)
    int best_cy = spawn_cy;
    bool found = false;
    for (int r = 0; r <= max_radius && !found; ++r) {
        for (int dy = -r; dy <= r && !found; ++dy) {
            for (int dx = -r; dx <= r && !found; ++dx) {
                // So o ANEL deste raio (bordas do quadrado) - pula pontos ja varridos
                // em raios menores.
                if (std::abs(dx) != r && std::abs(dy) != r) {
                    continue;
                }
                const int cx = target_cx + dx;
                const int cy = target_cy + dy;
                if (is_reachable(cx, cy)) {
                    best_cx = cx;
                    best_cy = cy;
                    found = true;
                }
            }
        }
    }

    gus::core::spatial::Aabb enemy;
    enemy.w = player_start.w;
    enemy.h = player_start.h;
    enemy.x = (static_cast<float>(best_cx) + 0.5f) * ts - enemy.w * 0.5f;
    enemy.y = (static_cast<float>(best_cy) + 0.5f) * ts - enemy.h * 0.5f;
    return enemy;
}

gus::core::spatial::Aabb enemy_sprite_footprint_aabb(
    const gus::core::spatial::Aabb& anchor, float sprite_height_tiles,
    float tile_size) noexcept {
    // MESMA formula de overworld_sim.cpp (MARCADOR DE INIMIGO FIXO): quad quadrado,
    // centrado em X sobre o anchor, base do quad = base do anchor (bottom_fraction=0,
    // manual_offset=0 - sem foot-inset, e um busto/icone, nao um sprite de corpo com
    // pes medidos).
    const float esprite_h = sprite_height_tiles * tile_size;
    const float esprite_w = esprite_h;  // retrato quadrado

    gus::core::spatial::Aabb fp;
    fp.w = esprite_w;
    fp.h = esprite_h;
    fp.x = anchor.x + anchor.w * 0.5f - esprite_w * 0.5f;
    fp.y = gus::app::screens::sprite_top_y(anchor.y + anchor.h, esprite_h,
                                            /*bottom_fraction=*/0.0f,
                                            /*manual_offset_world=*/0.0f);
    return fp;
}

}  // namespace gus::app

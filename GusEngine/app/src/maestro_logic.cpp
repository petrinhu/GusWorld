// gus/app/src/maestro_logic.cpp
//
// Implementacao da logica PURA da Maestro (M7-COSTURA). Ver header - travado por
// app/tests/maestro_logic_test.cpp (TEST-FIRST, headless, sem SDL/janela).

#include "gus/app/maestro_logic.hpp"

#include <cmath>  // std::abs

namespace gus::app {

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

    // Espiral de raio crescente ao redor do alvo (varre o quadrado [-r,r]x[-r,r] a cada
    // raio, mas so os pontos NOVOS daquele anel - suficiente pra um mapa pequeno/medio;
    // determinismo total: sempre varre na MESMA ordem). Para no primeiro livre.
    constexpr int kMaxRadius = 12;
    int best_cx = spawn_cx;  // fallback: a propria celula de spawn (garantida livre)
    int best_cy = spawn_cy;
    bool found = false;
    for (int r = 0; r <= kMaxRadius && !found; ++r) {
        for (int dy = -r; dy <= r && !found; ++dy) {
            for (int dx = -r; dx <= r && !found; ++dx) {
                // So o ANEL deste raio (bordas do quadrado) - pula pontos ja varridos
                // em raios menores.
                if (std::abs(dx) != r && std::abs(dy) != r) {
                    continue;
                }
                const int cx = target_cx + dx;
                const int cy = target_cy + dy;
                if (!grid.is_blocked(cx, cy)) {
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

}  // namespace gus::app

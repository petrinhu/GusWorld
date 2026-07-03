// GusEngine/app/tests/maestro_logic_test.cpp
//
// Catch2 (headless, SEM SDL_Init/janela) da logica PURA da Maestro (M7-COSTURA, ADR-012
// Onda 1): aabb_overlaps, should_trigger_battle, outcome_marks_enemy_defeated e
// pick_fixed_enemy_position. Ver gus/app/maestro_logic.hpp.

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "gus/app/maestro_logic.hpp"

using gus::app::aabb_overlaps;
using gus::app::EncounterId;
using gus::app::outcome_marks_enemy_defeated;
using gus::app::pick_fixed_enemy_position;
using gus::app::should_trigger_battle;
using gus::core::spatial::Aabb;
using gus::core::spatial::TileGrid;
using gus::domain::combat::CombatOutcome;

TEST_CASE("aabb_overlaps: sobreposicao franca", "[maestro][logic]") {
    const Aabb a{0.0f, 0.0f, 10.0f, 10.0f};
    const Aabb b{5.0f, 5.0f, 10.0f, 10.0f};
    CHECK(aabb_overlaps(a, b));
    CHECK(aabb_overlaps(b, a));  // simetrico
}

TEST_CASE("aabb_overlaps: separados nao sobrepoem", "[maestro][logic]") {
    const Aabb a{0.0f, 0.0f, 10.0f, 10.0f};
    const Aabb b{100.0f, 100.0f, 10.0f, 10.0f};
    CHECK_FALSE(aabb_overlaps(a, b));
}

TEST_CASE("aabb_overlaps: bordas coincidentes NAO contam (meio-aberto)",
          "[maestro][logic]") {
    const Aabb a{0.0f, 0.0f, 10.0f, 10.0f};
    const Aabb b{10.0f, 0.0f, 10.0f, 10.0f};  // encosta exatamente em x=10
    CHECK_FALSE(aabb_overlaps(a, b));
}

TEST_CASE("should_trigger_battle: inimigo vivo + sobreposicao -> true",
          "[maestro][logic]") {
    const Aabb player{0.0f, 0.0f, 10.0f, 10.0f};
    const Aabb enemy{5.0f, 5.0f, 10.0f, 10.0f};
    CHECK(should_trigger_battle(player, enemy, /*enemy_defeated=*/false));
}

TEST_CASE("should_trigger_battle: sem sobreposicao -> false", "[maestro][logic]") {
    const Aabb player{0.0f, 0.0f, 10.0f, 10.0f};
    const Aabb enemy{100.0f, 100.0f, 10.0f, 10.0f};
    CHECK_FALSE(should_trigger_battle(player, enemy, /*enemy_defeated=*/false));
}

TEST_CASE("should_trigger_battle: inimigo DERROTADO nunca dispara, mesmo sobreposto",
          "[maestro][logic]") {
    const Aabb player{0.0f, 0.0f, 10.0f, 10.0f};
    const Aabb enemy{5.0f, 5.0f, 10.0f, 10.0f};  // sobreposto
    CHECK_FALSE(should_trigger_battle(player, enemy, /*enemy_defeated=*/true));
}

TEST_CASE("outcome_marks_enemy_defeated: so Victory marca", "[maestro][logic]") {
    CHECK(outcome_marks_enemy_defeated(CombatOutcome::Victory));
    CHECK_FALSE(outcome_marks_enemy_defeated(CombatOutcome::Defeat));
    CHECK_FALSE(outcome_marks_enemy_defeated(CombatOutcome::Fled));
    CHECK_FALSE(outcome_marks_enemy_defeated(CombatOutcome::Ongoing));
}

TEST_CASE("pick_fixed_enemy_position: celula-alvo livre -> senta exatamente nela",
          "[maestro][logic]") {
    // Grade 10x10 tudo livre, tile_size=2.0. Spawn no canto sup-esq (celula 1,1).
    TileGrid grid(10, 10, 2.0f);
    const Aabb player_start{2.0f + 0.5f, 2.0f + 0.5f, 1.0f, 1.0f};  // centro ~ celula(1,1)

    const Aabb enemy = pick_fixed_enemy_position(grid, player_start,
                                                  /*offset_tiles_x=*/3,
                                                  /*offset_tiles_y=*/0);

    // Celula-alvo = (1+3, 1) = (4,1), livre -> senta exatamente centrada nela.
    const float expected_cx = (4.0f + 0.5f) * 2.0f;
    const float expected_cy = (1.0f + 0.5f) * 2.0f;
    CHECK(enemy.w == player_start.w);
    CHECK(enemy.h == player_start.h);
    CHECK(enemy.x == Catch::Approx(expected_cx - enemy.w * 0.5f));
    CHECK(enemy.y == Catch::Approx(expected_cy - enemy.h * 0.5f));
}

TEST_CASE("pick_fixed_enemy_position: celula-alvo bloqueada -> vizinha livre mais proxima",
          "[maestro][logic]") {
    TileGrid grid(10, 10, 2.0f);
    const Aabb player_start{0.5f, 0.5f, 1.0f, 1.0f};  // spawn na celula (0,0)

    // Bloqueia a celula-alvo EXATA (offset 2,0 a partir do spawn -> celula (2,0)).
    grid.set_blocked(2, 0, true);

    const Aabb enemy =
        pick_fixed_enemy_position(grid, player_start, /*offset_tiles_x=*/2,
                                   /*offset_tiles_y=*/0);

    // Nao pode ter caido na celula bloqueada (2,0).
    const float blocked_cx = (2.0f + 0.5f) * 2.0f - enemy.w * 0.5f;
    CHECK(enemy.x != Catch::Approx(blocked_cx));

    // A celula escolhida deve estar LIVRE de fato.
    const int cx = grid.world_to_cell(enemy.x + enemy.w * 0.5f);
    const int cy = grid.world_to_cell(enemy.y + enemy.h * 0.5f);
    CHECK_FALSE(grid.is_blocked(cx, cy));
}

TEST_CASE("pick_fixed_enemy_position: tudo bloqueado ao redor -> cai no spawn (livre)",
          "[maestro][logic]") {
    // Grade pequena so com a celula de spawn livre (todo o resto bloqueado).
    TileGrid grid(5, 5, 2.0f);
    for (int y = 0; y < 5; ++y) {
        for (int x = 0; x < 5; ++x) {
            if (x != 0 || y != 0) {
                grid.set_blocked(x, y, true);
            }
        }
    }
    const Aabb player_start{0.5f, 0.5f, 1.0f, 1.0f};  // celula (0,0), a UNICA livre

    const Aabb enemy =
        pick_fixed_enemy_position(grid, player_start, /*offset_tiles_x=*/3,
                                   /*offset_tiles_y=*/0);

    const int cx = grid.world_to_cell(enemy.x + enemy.w * 0.5f);
    const int cy = grid.world_to_cell(enemy.y + enemy.h * 0.5f);
    CHECK(cx == 0);
    CHECK(cy == 0);
    CHECK_FALSE(grid.is_blocked(cx, cy));
}

TEST_CASE("EncounterId: kFixedEnemy1 existe (item 1 do escopo M7-COSTURA)",
          "[maestro][logic]") {
    constexpr auto id = EncounterId::kFixedEnemy1;
    CHECK(static_cast<int>(id) == 0);
}

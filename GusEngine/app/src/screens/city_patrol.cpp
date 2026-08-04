// SPDX-License-Identifier: Apache-2.0
// gus/app/src/screens/city_patrol.cpp
//
// Implementação do construtor de rota de ronda. Ver o header. Travado por
// app/tests/city_props_test.cpp (TEST-FIRST).

#include "gus/app/screens/city_patrol.hpp"

namespace gus::app::screens {

gus::domain::world::PatrolRoute build_horizontal_patrol_route(
    const gus::core::spatial::TileGrid& grid, int cell_x, int cell_y,
    int reach_tiles, float speed_tiles_per_sec, float pause_seconds) noexcept {
    gus::domain::world::PatrolRoute route;
    if (reach_tiles <= 0 || speed_tiles_per_sec <= 0.0f) {
        return route;  // inválida: personagem parado
    }
    if (grid.is_blocked(cell_x, cell_y)) {
        // Origem fora do mapa ou dentro de parede (is_blocked cobre os dois).
        return route;
    }

    // Varre para cada lado ATÉ a primeira célula bloqueada: a rota encurta em vez
    // de atravessar. Parar na PRIMEIRA parede (e não pular por cima dela) é o que
    // garante que todo ponto do trilho é chão contíguo.
    int left = cell_x;
    for (int i = 1; i <= reach_tiles; ++i) {
        if (grid.is_blocked(cell_x - i, cell_y)) {
            break;
        }
        left = cell_x - i;
    }
    int right = cell_x;
    for (int i = 1; i <= reach_tiles; ++i) {
        if (grid.is_blocked(cell_x + i, cell_y)) {
            break;
        }
        right = cell_x + i;
    }

    if (left == right) {
        return route;  // parede colada dos dois lados: sem ronda possível
    }

    route.waypoints[0] = {static_cast<float>(left), static_cast<float>(cell_y)};
    route.waypoints[1] = {static_cast<float>(right), static_cast<float>(cell_y)};
    route.count = 2;
    route.speed_tiles_per_sec = speed_tiles_per_sec;
    route.pause_seconds = pause_seconds;
    return route;
}

}  // namespace gus::app::screens

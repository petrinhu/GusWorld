// SPDX-License-Identifier: Apache-2.0
// gus/app/src/screens/city_actors.cpp
//
// Implementação do povoamento da cidade. Ver o header. Travado por
// app/tests/city_actors_test.cpp (TEST-FIRST).

#include "gus/app/screens/city_actors.hpp"

#include <string>

#include <glintfx/log.hpp>  // FW-LOG: o log da casa vem do glintfx, nunca do SDL

#include "gus/app/screens/city_patrol.hpp"

namespace gus::app::screens {

std::vector<CityActorPlacement> resolve_city_actors(
    const gus::core::spatial::TileGrid& grid, const CityActorRow* rows, int count) {
    std::vector<CityActorPlacement> out;
    if (rows == nullptr || count <= 0) {
        return out;
    }

    out.reserve(static_cast<std::size_t>(count));
    int descartados = 0;
    int parados = 0;
    for (int i = 0; i < count; ++i) {
        const CityActorRow& row = rows[i];
        // is_blocked cobre "fora do mapa" e "dentro de parede" numa consulta só (a
        // borda do mapa é parede implícita, ver TileGrid).
        if (grid.is_blocked(row.cell_x, row.cell_y)) {
            ++descartados;
            continue;
        }

        CityActorPlacement place;
        place.role = row.role;
        place.sprite_dir = row.sprite_dir;
        place.sprite_file = row.sprite_file;
        place.cell_x = row.cell_x;
        place.cell_y = row.cell_y;
        // A rota nasce JÁ conferida contra as paredes reais: encurta onde bate, e
        // volta inválida quando não há para onde andar. Ninguém checa colisão
        // depois disto - a ronda é um trilho (ver city_patrol.hpp).
        place.route = build_horizontal_patrol_route(
            grid, row.cell_x, row.cell_y, row.patrol_reach_tiles,
            kDemoPatrolSpeedTilesPerSec, kDemoPatrolPauseSeconds);
        if (row.patrol_reach_tiles > 0 && !place.route.valid()) {
            ++parados;
        }
        out.push_back(place);
    }

    if (descartados > 0) {
        // via PLANA: número, texto interno de diagnóstico.
        glintfx::log(glintfx::LogLevel::Warn,
                     ("city_actors: " + std::to_string(descartados) +
                      " personagem(ns) caiu(ram) fora do mapa ou dentro de parede - "
                      "descartado(s) do elenco desta cidade.")
                         .c_str());
    }
    if (parados > 0) {
        // Não é erro fatal (o personagem entra parado), mas é sinal de que o
        // espaço de patrulha que o level design prometeu não existe no mapa.
        glintfx::log(glintfx::LogLevel::Warn,
                     ("city_actors: " + std::to_string(parados) +
                      " personagem(ns) pediu(ram) ronda e nao achou(aram) espaco "
                      "livre - monta(m) guarda parado(s).")
                         .c_str());
    }
    return out;
}

}  // namespace gus::app::screens

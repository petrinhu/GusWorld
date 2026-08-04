// SPDX-License-Identifier: Apache-2.0
// gus/app/screens/city_patrol.hpp
//
// CONSTRUÇÃO DA ROTA DE RONDA CONTRA O MAPA REAL (DEMO-CIDADE-VESTIDA fatia B3).
//
// A ronda em si é dado puro (gus/domain/world/patrol_route.hpp) e não conhece
// mapa nenhum: ela é um TRILHO, e ninguém verifica colisão a cada passo dela.
// Isso é de propósito (ronda que colide vira busca de caminho, que é outro
// sistema), mas transfere uma obrigação para cá: a rota precisa nascer VÁLIDA.
// Se ela atravessar parede, o jogador vê um androide andando dentro de um
// prédio - e o defeito não aparece em teste de unidade nenhum do domínio,
// porque lá a rota está formalmente correta.
//
// Por isso este módulo é separado do dado: aqui mora o único lugar que cruza a
// rota com a TileGrid carregada.
//
// Cross-ref: app/tests/city_props_test.cpp (seção de ronda).

#ifndef GUS_APP_SCREENS_CITY_PATROL_HPP
#define GUS_APP_SCREENS_CITY_PATROL_HPP

#include "gus/core/spatial/tile_grid.hpp"
#include "gus/domain/world/patrol_route.hpp"

namespace gus::app::screens {

// ===========================================================================
//  NÚMEROS DA RONDA DO DEMO (provisórios, à espera de playtest do líder)
//
//  Ponto único: mexer aqui muda a ronda inteira sem tocar em lógica nenhuma.
// ===========================================================================

// Velocidade da ronda, em tiles/s. O Gus caminha a 4.5 (OverworldTuning::
// walk_speed_tiles_per_sec) e corre a 7.2. A ronda a 1.8 é 40% da caminhada:
// devagar o bastante para o jogador conseguir contornar o inimigo andando, e
// rápido o bastante para ler como "ele está patrulhando" e não como "ele está
// derretendo". //PLAYTEST
inline constexpr float kDemoPatrolSpeedTilesPerSec = 1.8f;

// Quantos tiles a ronda tenta alcançar para cada lado do ponto de origem. Três
// tiles dão um vaivém de 6 tiles, visível dentro do enquadramento da câmera
// (~30x17 tiles) sem que o inimigo suma de vista no meio da ronda. //PLAYTEST
inline constexpr int kDemoPatrolReachTiles = 3;

// Pausa (s) ao chegar em cada ponta. É o que faz a ronda ler como "ele para e
// olha ao redor" em vez de "ele desliza num trilho". //PLAYTEST
inline constexpr float kDemoPatrolPauseSeconds = 0.8f;

// Rota horizontal de ida e volta centrada na célula (cell_x, cell_y), tentando
// alcançar `reach_tiles` células para cada lado e ENCURTANDO onde houver parede
// ou borda de mapa (a varredura para na primeira célula bloqueada de cada lado).
//
// Devolve rota INVÁLIDA (ou seja, personagem parado - a degradação de sempre)
// quando não há para onde andar: célula de origem fora do mapa, alcance zero,
// velocidade não positiva, ou parede colada dos dois lados.
[[nodiscard]] gus::domain::world::PatrolRoute build_horizontal_patrol_route(
    const gus::core::spatial::TileGrid& grid, int cell_x, int cell_y,
    int reach_tiles, float speed_tiles_per_sec, float pause_seconds) noexcept;

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_CITY_PATROL_HPP

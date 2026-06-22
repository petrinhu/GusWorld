// gus/core/spatial/grid_collision.hpp
//
// Colisao AABB-desliza-na-grade (M4) - POCO C++ puro, ZERO Qt, ZERO I/O.
// Movimento cinematico puro (sem impulso/fisica), deterministico.
//
// FEEL (decidido pelo lider, nao reabrir): a caixa do personagem DESLIZA ao
// longo das paredes. A resolucao e SEPARADA POR EIXO: tenta mover em X e resolve
// contra as celulas bloqueadas sobrepostas; depois tenta mover em Y a partir do
// X ja resolvido. Bateu numa parede num eixo, continua se movendo no outro
// (estilo Zelda ALttP / Stardew). Nao e "trava ao bater".
//
// CONTRATO:
//   - Aabb: x,y = canto SUPERIOR-ESQUERDO em unidades de mundo; w,h = largura e
//     altura (>= 0). Eixo +X direita, +Y baixo (ver tile_grid.hpp).
//   - resolve_move(grid, box, dx, dy) devolve a posicao final ja resolvida e
//     flags hit_x/hit_y (true se o movimento naquele eixo foi limitado por uma
//     parede). A caixa de entrada NAO e modificada.
//   - Pre-condicao: assume-se que a posicao de entrada nao esta DENTRO de uma
//     parede (estado valido). Borda do mapa e parede (TileGrid::is_blocked).

#ifndef GUS_CORE_SPATIAL_GRID_COLLISION_HPP
#define GUS_CORE_SPATIAL_GRID_COLLISION_HPP

namespace gus::core::spatial {

class TileGrid;

struct Aabb {
    float x = 0.0f;  // canto superior-esquerdo
    float y = 0.0f;
    float w = 0.0f;  // largura
    float h = 0.0f;  // altura
};

struct MoveResult {
    Aabb box;            // posicao final (w/h preservados)
    bool hit_x = false;  // movimento em X foi limitado por parede
    bool hit_y = false;  // movimento em Y foi limitado por parede
};

// Resolve um deslocamento desejado (dx,dy) contra a grade, deslizando nas
// paredes (resolucao por eixo: X primeiro, depois Y). Deterministico.
MoveResult resolve_move(const TileGrid& grid, const Aabb& box, float dx,
                        float dy) noexcept;

}  // namespace gus::core::spatial

#endif  // GUS_CORE_SPATIAL_GRID_COLLISION_HPP

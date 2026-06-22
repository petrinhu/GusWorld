// gus/core/spatial/tile_grid.cpp
// Implementacao do modelo de mapa de grade (M4). Ver tile_grid.hpp para o
// contrato (unidades, origem, eixos, borda-como-parede).

#include "gus/core/spatial/tile_grid.hpp"

#include <cmath>

namespace gus::core::spatial {

TileGrid::TileGrid(int width, int height, float tile_size)
    : width_(width > 0 ? width : 0),
      height_(height > 0 ? height : 0),
      tile_size_(tile_size),
      blocked_(static_cast<std::size_t>(width_) *
                   static_cast<std::size_t>(height_),
               false) {}

TileGrid TileGrid::from_rows(std::initializer_list<std::string_view> rows,
                             float tile_size) {
    const int height = static_cast<int>(rows.size());
    int width = 0;
    if (rows.size() > 0) {
        width = static_cast<int>(rows.begin()->size());
    }

    TileGrid grid(width, height, tile_size);

    int y = 0;
    for (std::string_view row : rows) {
        const int row_len = static_cast<int>(row.size());
        for (int x = 0; x < width && x < row_len; ++x) {
            if (row[static_cast<std::size_t>(x)] == '#') {
                grid.set_blocked(x, y, true);
            }
        }
        ++y;
    }
    return grid;
}

bool TileGrid::in_bounds(int cell_x, int cell_y) const noexcept {
    return cell_x >= 0 && cell_x < width_ && cell_y >= 0 && cell_y < height_;
}

std::size_t TileGrid::index(int cell_x, int cell_y) const noexcept {
    return static_cast<std::size_t>(cell_y) * static_cast<std::size_t>(width_) +
           static_cast<std::size_t>(cell_x);
}

bool TileGrid::is_blocked(int cell_x, int cell_y) const noexcept {
    if (!in_bounds(cell_x, cell_y)) {
        return true;  // fora do mapa = parede
    }
    return blocked_[index(cell_x, cell_y)];
}

void TileGrid::set_blocked(int cell_x, int cell_y, bool blocked) noexcept {
    if (!in_bounds(cell_x, cell_y)) {
        return;  // borda implicita, nao armazenada
    }
    blocked_[index(cell_x, cell_y)] = blocked;
}

int TileGrid::world_to_cell(float world_coord) const noexcept {
    return static_cast<int>(std::floor(world_coord / tile_size_));
}

}  // namespace gus::core::spatial

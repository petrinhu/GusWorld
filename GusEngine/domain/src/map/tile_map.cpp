// gus/domain/src/map/tile_map.cpp
//
// Implementacao do TileMap (POCO do mapa de tiles). Ver tile_map.hpp para o
// contrato (convencao de ids, eixos, invariantes). ZERO Qt/SDL/I/O.

#include "gus/domain/map/tile_map.hpp"

#include <stdexcept>
#include <string>

namespace gus::domain::map {

bool is_tile_blocking(std::uint16_t tile_id) noexcept {
    return tile_id == static_cast<std::uint16_t>(TileKind::Parede);
}

TileMap::TileMap(std::int32_t width, std::int32_t height, float tile_size)
    : width_(width),
      height_(height),
      tile_size_(tile_size),
      tiles_(),
      spawn_{0, 0},
      portals_() {
    if (width < 1)
        throw std::invalid_argument("TileMap: width deve ser >= 1 (recebido " +
                                    std::to_string(width) + ").");
    if (height < 1)
        throw std::invalid_argument("TileMap: height deve ser >= 1 (recebido " +
                                    std::to_string(height) + ").");
    if (!(tile_size > 0.0f))
        throw std::invalid_argument(
            "TileMap: tile_size deve ser > 0 (recebido " +
            std::to_string(tile_size) + ").");
    tiles_.assign(static_cast<std::size_t>(width) *
                      static_cast<std::size_t>(height),
                  static_cast<std::uint16_t>(TileKind::Chao));
}

TileMap::TileMap() : TileMap(1, 1, 1.0f) {}

bool TileMap::in_bounds(std::int32_t x, std::int32_t y) const noexcept {
    return x >= 0 && x < width_ && y >= 0 && y < height_;
}

std::size_t TileMap::index(std::int32_t x, std::int32_t y) const noexcept {
    return static_cast<std::size_t>(y) * static_cast<std::size_t>(width_) +
           static_cast<std::size_t>(x);
}

std::uint16_t TileMap::at(std::int32_t x, std::int32_t y) const {
    if (!in_bounds(x, y))
        throw std::out_of_range("TileMap::at fora dos limites (" +
                                std::to_string(x) + "," + std::to_string(y) + ").");
    return tiles_[index(x, y)];
}

void TileMap::set(std::int32_t x, std::int32_t y, std::uint16_t tile_id) {
    if (!in_bounds(x, y))
        throw std::out_of_range("TileMap::set fora dos limites (" +
                                std::to_string(x) + "," + std::to_string(y) + ").");
    tiles_[index(x, y)] = tile_id;
}

void TileMap::validate() const {
    if (width_ < 1 || height_ < 1)
        throw std::invalid_argument("TileMap: dims invalidas (< 1).");
    if (!(tile_size_ > 0.0f))
        throw std::invalid_argument("TileMap: tile_size invalido (<= 0).");
    const std::size_t esperado = static_cast<std::size_t>(width_) *
                                 static_cast<std::size_t>(height_);
    if (tiles_.size() != esperado)
        throw std::invalid_argument(
            "TileMap: tamanho da matriz (" + std::to_string(tiles_.size()) +
            ") != width*height (" + std::to_string(esperado) + ").");
    if (!in_bounds(spawn_.x, spawn_.y))
        throw std::invalid_argument("TileMap: spawn fora dos limites (" +
                                    std::to_string(spawn_.x) + "," +
                                    std::to_string(spawn_.y) + ").");
    for (const Portal& p : portals_) {
        if (p.id.empty())
            throw std::invalid_argument("TileMap: portal com id vazio.");
        if (!in_bounds(p.cell.x, p.cell.y))
            throw std::invalid_argument("TileMap: portal '" + p.id +
                                        "' fora dos limites.");
    }
}

gus::core::spatial::TileGrid TileMap::to_tile_grid() const {
    gus::core::spatial::TileGrid grid(width_, height_, tile_size_);
    for (std::int32_t y = 0; y < height_; ++y) {
        for (std::int32_t x = 0; x < width_; ++x) {
            if (is_tile_blocking(tiles_[index(x, y)]))
                grid.set_blocked(x, y, true);
        }
    }
    return grid;
}

bool operator==(const TileMap& a, const TileMap& b) noexcept {
    return a.width_ == b.width_ && a.height_ == b.height_ &&
           a.tile_size_ == b.tile_size_ && a.tiles_ == b.tiles_ &&
           a.spawn_ == b.spawn_ && a.portals_ == b.portals_ &&
           a.map_id_ == b.map_id_;
}

}  // namespace gus::domain::map

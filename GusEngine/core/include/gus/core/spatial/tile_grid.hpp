// gus/core/spatial/tile_grid.hpp
//
// Modelo de mapa de grade (M4) - POCO C++ puro, ZERO Qt, ZERO I/O real.
// Logica calculavel-sem-janela do top-down.
//
// CONTRATO (convencoes tecnicas, fixadas aqui):
//   - A grade tem width x height celulas (em celulas, nao em pixels).
//   - Cada celula e livre (false) ou bloqueada (true). is_blocked(x,y) consulta
//     uma celula; QUALQUER celula fora dos limites [0,width) x [0,height) e
//     considerada BLOQUEADA (a borda do mapa e uma parede solida).
//   - tile_size: tamanho de uma celula em UNIDADES DE MUNDO (float). A celula
//     (cx,cy) ocupa o retangulo de mundo
//       [cx*tile_size, (cx+1)*tile_size) x [cy*tile_size, (cy+1)*tile_size).
//   - ORIGEM do mundo em (0,0), no canto superior-esquerdo da celula (0,0).
//     Eixo +X cresce para a direita, eixo +Y cresce para BAIXO (convencao
//     top-down/tela: linha = Y, coluna = X). O mundo do mapa vai de (0,0) ate
//     (width*tile_size, height*tile_size).
//   - world_to_cell(coord) = floor(coord / tile_size) (pode ser negativo fora
//     do mapa; nesse caso is_blocked devolve true).
//
// Sem alocacao escondida alem do std::vector<bool> do mapa.

#ifndef GUS_CORE_SPATIAL_TILE_GRID_HPP
#define GUS_CORE_SPATIAL_TILE_GRID_HPP

#include <cstddef>
#include <initializer_list>
#include <string_view>
#include <vector>

namespace gus::core::spatial {

class TileGrid {
public:
    // Cria uma grade width x height, tile_size unidades de mundo por celula,
    // TODAS as celulas livres. Pre-condicao: width >= 0, height >= 0,
    // tile_size > 0. Valores invalidos sao saturados (width/height negativos
    // viram 0); a grade resultante e sempre coerente.
    TileGrid(int width, int height, float tile_size);

    // Fixture de conveniencia: cada string e uma linha (eixo Y cresce para
    // baixo). '#' = celula bloqueada, qualquer outro caractere = livre. Todas
    // as linhas devem ter o mesmo comprimento (a largura e a da primeira linha;
    // colunas faltantes em linhas curtas viram livres, sobras sao ignoradas).
    static TileGrid from_rows(std::initializer_list<std::string_view> rows,
                              float tile_size);

    int width() const noexcept { return width_; }
    int height() const noexcept { return height_; }
    float tile_size() const noexcept { return tile_size_; }

    // true se a celula esta bloqueada OU fora dos limites do mapa.
    bool is_blocked(int cell_x, int cell_y) const noexcept;

    // Marca/desmarca uma celula. Indices fora dos limites sao ignorados
    // (no-op): a borda e parede implicita, nao armazenada.
    void set_blocked(int cell_x, int cell_y, bool blocked) noexcept;

    // floor(world_coord / tile_size). Vale para X e Y (a grade e quadrada por
    // celula). Pode devolver indice negativo para coordenadas fora do mapa.
    int world_to_cell(float world_coord) const noexcept;

private:
    bool in_bounds(int cell_x, int cell_y) const noexcept;
    std::size_t index(int cell_x, int cell_y) const noexcept;

    int width_;
    int height_;
    float tile_size_;
    std::vector<bool> blocked_;  // row-major: index = y*width + x
};

}  // namespace gus::core::spatial

#endif  // GUS_CORE_SPATIAL_TILE_GRID_HPP

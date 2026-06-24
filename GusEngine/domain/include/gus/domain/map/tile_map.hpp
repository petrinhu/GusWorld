// gus/domain/map/tile_map.hpp
//
// MAPA DE TILES do overworld: matriz row-major de tile-ids + metadados minimos.
// POCO C++ puro, ZERO Qt, ZERO SDL, ZERO I/O real (engine-design.md secao 2,
// ADR-008). E o DADO; a serializacao selada vive em map_serializer.hpp e o I/O de
// arquivo so na fronteira app/. Decisao do lider 2026-06-23: o mapa e uma matriz
// guardada num binario proprio SELADO com HMAC (anti-tamper), editavel no dev via
// uma fonte CSV compilada.
//
// CONVENCAO DE TILE-IDS (enum TileKind, extensivel): o id e um uint16_t (cabe
// ~65k tipos de tile; folga para variantes futuras de arte sem mexer no formato).
// Os ids BAIXOS tem semantica de COLISAO fixada aqui (a conversao para TileGrid
// depende so disso); ids >= kMaxReservado sao livres para a camada de arte/render
// e tratados como CHAO (nao bloqueiam) ate ganharem semantica propria.
//
//   0 Chao      livre, andavel
//   1 Parede    bloqueado (a colisao do overworld trata como solido)
//   2 Marco     livre, mas marca um ponto de interesse (landmark / no de design)
//   3 Entrada   livre, ponto de entrada da area (rampa norte etc.)
//   4 Saida     livre, ponto de saida da area (portao sul etc.)
//
// COLISAO: por padrao, SO Parede bloqueia. is_tile_blocking(id) centraliza essa
// regra; to_tile_grid() a usa para gerar o core::spatial::TileGrid (livre/bloqueado)
// que a colisao do overworld JA consome.
//
// EIXOS (espelha TileGrid): grade width x height celulas, row-major
//   index = y*width + x, eixo +X para a direita, +Y para BAIXO (linha = Y). A
//   celula (cx,cy) ocupa [cx*tile_size,(cx+1)*tile_size) x [...] em unidades de
//   mundo. tile_size = lado da celula em unidades de mundo (metros, no GusWorld).
//
// METADADOS: spawn do player (celula cx,cy) + lista opcional de portais/saidas
// nomeados (id textual + celula que ele ocupa). Minimo necessario para o slice.
//
// IDENTIDADE (map_id, decisao do lider 2026-06-23): cada mapa carrega um UUID textual
// ESTAVEL (gerado na FONTE .csv via diretiva #map_id, embutido pelo compilador). Ele
// entra no payload .gmap DENTRO do HMAC (a partir da v2): o selo prova autenticidade,
// o map_id prova IDENTIDADE. O loader, dado um expected_map_id, recusa um .gmap
// CORRETAMENTE SELADO mas de OUTRO mapa (defesa contra map-swap). O TileMap em si nao
// exige map_id (um mapa em construcao/legado v1 pode ter id vazio); a obrigatoriedade
// e da serializacao v2 (fail-fast em serialize_map) e do binding (load_map).
//
// Cross-ref: gus/core/spatial/tile_grid.hpp (consumidor da colisao),
//            gus/domain/map/map_serializer.hpp (formato .gmap selado),
//            docs/design/levels/blockout-distritos-inferiores.md (1o mapa).

#ifndef GUS_DOMAIN_MAP_TILE_MAP_HPP
#define GUS_DOMAIN_MAP_TILE_MAP_HPP

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "gus/core/spatial/tile_grid.hpp"

namespace gus::domain::map {

// Convencao de tile-ids (extensivel). Os valores numericos sao ESTAVEIS (entram no
// binario .gmap): nunca renumere os existentes; novos ids so crescem.
enum class TileKind : std::uint16_t {
    Chao = 0,     // livre, andavel
    Parede = 1,   // bloqueado (solido para a colisao)
    Marco = 2,    // livre, ponto de interesse (landmark / no de design)
    Entrada = 3,  // livre, ponto de entrada da area
    Saida = 4,    // livre, ponto de saida da area
};

// Primeiro id ainda NAO atribuido a uma TileKind. Ids >= este sao "reservados ao
// futuro": validos no formato, tratados como Chao (nao bloqueiam) ate ganharem
// semantica. Mantenha em sincronia com o maior TileKind + 1.
inline constexpr std::uint16_t kMaxReservado = 5;

// Regra de colisao centralizada: SO Parede bloqueia. Qualquer outro id (inclusive
// reservados ao futuro) e andavel. Total para todo uint16_t.
[[nodiscard]] bool is_tile_blocking(std::uint16_t tile_id) noexcept;

// Celula em coordenadas de grade (inteiras). x = coluna, y = linha.
struct Cell {
    std::int32_t x = 0;
    std::int32_t y = 0;

    friend bool operator==(const Cell& a, const Cell& b) noexcept {
        return a.x == b.x && a.y == b.y;
    }
};

// Portal/saida nomeada (metadado opcional). id textual estavel (ex: "saida_sul") +
// a celula que ele ocupa neste mapa. O destino logico (outra area) e resolvido pela
// camada de jogo; aqui guardamos so o ancoramento espacial + o nome.
struct Portal {
    std::string id;  // identificador textual (sem acento, convencao do projeto)
    Cell cell;       // celula que o portal ocupa neste mapa

    friend bool operator==(const Portal& a, const Portal& b) noexcept {
        return a.id == b.id && a.cell == b.cell;
    }
};

// Mapa de tiles + metadados. Invariantes (validate(), fail-fast):
//   - width >= 1, height >= 1, tile_size > 0;
//   - tiles.size() == width*height (matriz row-major coerente);
//   - spawn dentro dos limites [0,width) x [0,height);
//   - cada portal com id nao-vazio e celula dentro dos limites.
class TileMap {
public:
    // Mapa width x height, tile_size unidades de mundo por celula, TODAS as celulas
    // = Chao (0). Pre-condicao: width>=1, height>=1, tile_size>0. Valores invalidos
    // lancam std::invalid_argument (objeto invalido nao deve ser construivel).
    TileMap(std::int32_t width, std::int32_t height, float tile_size);

    // Mapa 1x1 de Chao (coerente). Util para deserializacao incremental.
    TileMap();

    [[nodiscard]] std::int32_t width() const noexcept { return width_; }
    [[nodiscard]] std::int32_t height() const noexcept { return height_; }
    [[nodiscard]] float tile_size() const noexcept { return tile_size_; }

    // Tile-id da celula (x,y). Pre-condicao: in_bounds(x,y). Fora dos limites lanca
    // std::out_of_range (acesso indevido e bug do chamador, fail-fast).
    [[nodiscard]] std::uint16_t at(std::int32_t x, std::int32_t y) const;

    // Define o tile-id da celula (x,y). Fora dos limites lanca std::out_of_range.
    void set(std::int32_t x, std::int32_t y, std::uint16_t tile_id);

    [[nodiscard]] bool in_bounds(std::int32_t x, std::int32_t y) const noexcept;

    // Acesso direto a matriz (row-major, tamanho width*height). Para o serializer.
    [[nodiscard]] const std::vector<std::uint16_t>& tiles() const noexcept {
        return tiles_;
    }

    // --- metadados ---
    [[nodiscard]] Cell spawn() const noexcept { return spawn_; }
    void set_spawn(Cell c) noexcept { spawn_ = c; }

    [[nodiscard]] const std::vector<Portal>& portals() const noexcept {
        return portals_;
    }
    void add_portal(Portal p) { portals_.push_back(std::move(p)); }
    void clear_portals() noexcept { portals_.clear(); }

    // Identidade do mapa (UUID textual estavel, ver doc acima). Vazio = sem binding
    // (mapa em construcao ou legado v1). Setado pelo compilador a partir de #map_id.
    [[nodiscard]] const std::string& map_id() const noexcept { return map_id_; }
    void set_map_id(std::string id) { map_id_ = std::move(id); }

    // Valida todas as invariantes. Lanca std::invalid_argument na primeira violacao
    // (fail-fast). Chamado pelo serializer antes de empacotar e pelo loader depois de
    // materializar (defesa em profundidade, mesmo padrao do save).
    void validate() const;

    // Gera o core::spatial::TileGrid (livre/bloqueado) que a colisao do overworld JA
    // consome: cada celula vira bloqueada sse is_tile_blocking(tile-id). Mesmo
    // width/height/tile_size. A borda do mapa ja e parede implicita no TileGrid.
    [[nodiscard]] gus::core::spatial::TileGrid to_tile_grid() const;

    friend bool operator==(const TileMap& a, const TileMap& b) noexcept;

    // Acesso INTERNO para o serializer materializar a matriz crua na deserializacao
    // (sem validar in_bounds tile a tile). NAO usar em codigo de jogo: prefira set().
    // A coerencia tiles.size()==w*h e checada por validate() logo apos.
    void assign_tiles_raw(std::vector<std::uint16_t> tiles) {
        tiles_ = std::move(tiles);
    }

private:
    [[nodiscard]] std::size_t index(std::int32_t x, std::int32_t y) const noexcept;

    std::int32_t width_;
    std::int32_t height_;
    float tile_size_;
    std::vector<std::uint16_t> tiles_;  // row-major: index = y*width + x
    Cell spawn_;
    std::vector<Portal> portals_;
    std::string map_id_;  // UUID textual estavel; vazio = sem binding (legado/em construcao)
};

}  // namespace gus::domain::map

#endif  // GUS_DOMAIN_MAP_TILE_MAP_HPP

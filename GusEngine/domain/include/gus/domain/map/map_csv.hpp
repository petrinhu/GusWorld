// gus/domain/map/map_csv.hpp
//
// Compilador de mapa (lado PURO): parser CSV -> TileMap. O CSV e a FONTE editavel
// no dev (versionavel em git); o compilador o transforma no TileMap que o
// map_serializer sela em .gmap. POCO puro, ZERO Qt/SDL, ZERO I/O: opera sobre uma
// string ja em memoria (a leitura/escrita de arquivo fica na fronteira app/).
//
// FORMATO CSV:
//   - Cada LINHA de grade = uma linha do mapa (eixo Y para baixo). Os valores sao
//     numeros (tile-ids) separados por virgula; cada numero = uma celula (X cresce
//     da esquerda para a direita). TODAS as linhas de grade devem ter a MESMA
//     largura (a do primeiro; largura divergente = erro).
//   - Linhas-DIRETIVA com prefixo '#' carregam metadados:
//       #tile_size <float>     lado da celula em unidades de mundo (default 1.0)
//       #spawn <x> <y>         celula de spawn do player (default 0 0)
//       #portal <id> <x> <y>   portal/saida nomeada (pode repetir)
//   - Linhas em branco e comentarios '//' (linha inteira) sao ignorados.
//
// VALIDACAO: numero invalido / vazio / negativo / fora de uint16, largura
// divergente, ausencia de grade -> MapCsvError (com a linha 1-based no texto).
// Metadados incoerentes com a grade (spawn/portal fora dos limites) caem no
// TileMap::validate() -> std::invalid_argument (fail-fast, mesma disciplina).
//
// Cross-ref: gus/domain/map/tile_map.hpp, gus/domain/map/map_serializer.hpp.

#ifndef GUS_DOMAIN_MAP_MAP_CSV_HPP
#define GUS_DOMAIN_MAP_MAP_CSV_HPP

#include <stdexcept>
#include <string>
#include <string_view>

#include "gus/domain/map/tile_map.hpp"

namespace gus::domain::map {

// CSV malformado (largura divergente, numero invalido, sem grade). Carrega a
// mensagem com a linha 1-based. NAO cobre spawn/portal fora dos limites: isso e
// invariante do TileMap (std::invalid_argument via validate()).
class MapCsvError : public std::runtime_error {
public:
    explicit MapCsvError(const std::string& message)
        : std::runtime_error(message) {}
};

// Parser POCO: CSV (string) -> TileMap validado. Lanca MapCsvError (formato CSV) ou
// std::invalid_argument (metadado fora dos limites, via TileMap::validate()).
[[nodiscard]] TileMap parse_csv_to_tilemap(std::string_view csv);

}  // namespace gus::domain::map

#endif  // GUS_DOMAIN_MAP_MAP_CSV_HPP

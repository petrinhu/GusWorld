// gus/domain/map/map_serializer.hpp
//
// Formato binario proprio .gmap SELADO com HMAC-SHA256 (do core/) anti-tamper do
// TileMap. MESMO esquema do save (ADR-006): o jogador NAO edita o mapa para
// atravessar parede; quem altera a matriz quebra o selo. POCO puro, ZERO Qt/SDL,
// ZERO I/O (opera sobre bytes; o I/O de arquivo fica na fronteira app/).
//
// ENVELOPE (little-endian), identico em estrutura ao do save:
//   [4]      MAGIC   = "GMAP" (Gus MAP). Distingue de save("GDS2")/templates("GDT1").
//   [4]      LENGTH  = uint32 LE = tamanho do PAYLOAD em bytes.
//   [LENGTH] PAYLOAD = serializacao binaria propria do TileMap (abaixo).
//   [32]     HMAC    = HMAC-SHA256( MAGIC || LENGTH || PAYLOAD ), chave DERIVADA
//                      PROPRIA do mapa (contexto "map-hmac", separada do save).
// O HMAC cobre header + payload: flip de QUALQUER byte quebra o selo (HmacInvalid).
//
// PAYLOAD (formato proprio; little-endian). Inicia com a VERSAO de schema, para o
// load saber migrar ANTES de materializar (forward-only, igual ao save):
//   u32 schema_version |
//   i32 width | i32 height | f32 tile_size |
//   u32 tile_count (== width*height) | repeat tile_count: u16 tile_id |
//   i32 spawn_x | i32 spawn_y |
//   u32 portal_count | repeat: str id (u32 len + bytes) | i32 cell_x | i32 cell_y
//
// ORDEM DO LOAD (load_map, forward-only, espelha deserialize_save):
//   1. valida HMAC do envelope ANTES de versao (nunca migra bytes adulterados);
//   2. le schema_version do payload validado;
//   3. version > atual -> VersionTooNew (forward-only nao le o futuro);
//   4. version < atual -> roda a chain de migracao antes de materializar;
//   5. materializa o TileMap e valida invariantes (defesa em profundidade).
// load_map NAO lanca: sinaliza por valor (MapLoadResult), igual ao load_save do T1.1.
//
// MIGRACAO: forward-only desde a v1 (kMapSchemaVersion). Hoje so existe a v1; o
// gancho de chain fica preparado (migrate_to_current) para campos aditivos futuros.
//
// Cross-ref: gus/domain/save/save_serializer.hpp (mesmo envelope/disciplina),
//            gus/core/crypto/hmac_sha256.hpp, ADR-006.

#ifndef GUS_DOMAIN_MAP_MAP_SERIALIZER_HPP
#define GUS_DOMAIN_MAP_MAP_SERIALIZER_HPP

#include <cstdint>
#include <vector>

#include "gus/domain/map/tile_map.hpp"

namespace gus::domain::map {

// Versao de schema atual do payload .gmap (forward-only).
inline constexpr int kMapSchemaVersion = 1;

// Resultado de um load (camada PURA, sem UI). A app decide o que avisar.
enum class MapLoadResult {
    Ok,             // carregou e validou
    HmacInvalid,    // HMAC nao bate: mapa adulterado
    Corrupt,        // malformado (magic/length/truncado/payload invalido)
    VersionTooNew,  // schema_version > atual (forward-only)
    Invalid,        // payload bem-formado/selado mas invariante violada (dims etc.)
};

// Saida de load_map: o resultado + o mapa (valido sse result == Ok).
struct MapLoadOutcome {
    MapLoadResult result = MapLoadResult::Corrupt;
    TileMap map;  // default 1x1 ate o load preencher
};

// ---- serialize / load ------------------------------------------------------

// Serializa o TileMap no formato atual (v1), selado. Valida invariantes antes
// (fail-fast: lanca std::invalid_argument se o mapa for invalido).
[[nodiscard]] std::vector<std::uint8_t> serialize_map(const TileMap& map);

// Load NAO-lancante (espelha load_save): valida HMAC, le versao, rejeita futuro,
// migra se atras, materializa e valida invariantes. Sinaliza por valor.
[[nodiscard]] MapLoadOutcome load_map(const std::vector<std::uint8_t>& data);

// ---- helpers de TESTE (forjam envelopes selados-mas-divergentes) -----------
//
// A chave do HMAC mora no binario por design (integridade casual local, nao
// sigilo): os testes provam que mesmo um envelope CORRETAMENTE SELADO e recusado
// quando a versao e futura ou a invariante e violada. Nao usar em producao.

// Serializa selando com uma versao de schema ARBITRARIA (para o teste de
// VersionTooNew). Com version == kMapSchemaVersion equivale a serialize_map.
[[nodiscard]] std::vector<std::uint8_t> serialize_map_with_version(
    const TileMap& map, int version);

// Monta um envelope v1 CORRETAMENTE SELADO cujo payload anuncia tile_count
// inconsistente com width*height (invariante violada). load_map deve devolver
// Invalid (defesa em profundidade pos-HMAC).
[[nodiscard]] std::vector<std::uint8_t> forge_bad_dims_envelope();

}  // namespace gus::domain::map

#endif  // GUS_DOMAIN_MAP_MAP_SERIALIZER_HPP

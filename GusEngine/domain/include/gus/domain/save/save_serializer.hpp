// gus/domain/save/save_serializer.hpp
//
// Serializacao BINARIA PROPRIA + HMAC-SHA256 (do core/) anti-tamper do estado de
// SAVE. ADR-006: formato binario proprio (NAO JSON, NAO o byte-format do C#),
// selado pelo HMAC proprio de gus::core::crypto. POCO puro, ZERO Qt.
//
// ENVELOPE (little-endian), mesma estrutura do template serializer:
//   [4]      MAGIC   = "GDS2" (GusDragon Save v2). Distingue de templates ("GDT1").
//   [4]      LENGTH  = uint32 LE = tamanho do PAYLOAD em bytes.
//   [LENGTH] PAYLOAD = serializacao binaria compacta PROPRIA do SaveData (abaixo).
//   [32]     HMAC    = HMAC-SHA256( MAGIC || LENGTH || PAYLOAD ), chave embutida
//                      PROPRIA do save (distinta da chave dos templates).
//
// O HMAC cobre header + payload: flip de QUALQUER byte (magic/length/payload/hmac)
// quebra o selo e lanca SaveIntegrityError. Sem reparo.
//
// PAYLOAD (formato proprio; little-endian). Inicia com a VERSAO de schema, para que
// o load saiba migrar saves antigos ANTES de materializar:
//   u32 schema_version | i64 timestamp_ms | f64 playtime_seconds |
//   str current_scene_path | vec3 player_position | vec3 player_rotation |
//   list<str> party_roster | list<str> party_active |
//   map<str,u8> flags | map<str,i32> inventory | map<str,i32> quest_progress |
//   map<str,i32> relations |
//   map<str, CharacterSaveState{ i32 current_hp | i32 xp | list<str> deck }>
// (mapas gravados em ordem de chave: std::map = serializacao deterministica).
// O layout V1 (sem character_states) e produzido por serialize_save_v1, para as
// fixtures de migracao.
//
// ORDEM DO LOAD (deserialize_save, forward-only, CONTRACT §7):
//   1. valida HMAC do envelope (integridade ANTES de versao: nunca migra bytes
//      adulterados) -> SaveIntegrityError em mismatch;
//   2. le schema_version do payload validado;
//   3. version > atual -> SaveVersionTooNewError (forward-only nao le o futuro);
//   4. version < atual -> roda a chain (V1->V2->...) antes de materializar;
//   5. version == atual -> materializa direto.
//
// Cross-ref: engine/foundation/save_system/SaveSerializer.cs (ref semantica),
//            gus/domain/templates/template_serializer.hpp (mesmo envelope),
//            gus/domain/save/save_migrators.hpp, ADR-006.

#ifndef GUS_DOMAIN_SAVE_SAVE_SERIALIZER_HPP
#define GUS_DOMAIN_SAVE_SAVE_SERIALIZER_HPP

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include "gus/domain/save/save_data.hpp"

namespace gus::domain::save {

// HMAC do save nao bate: arquivo adulterado/tampered. Analoga a
// SaveIntegrityException do C#.
class SaveIntegrityError : public std::runtime_error {
   public:
    explicit SaveIntegrityError(const std::string& message)
        : std::runtime_error(message) {}
};

// Save estruturalmente corrompido (magic errado, truncado, length inconsistente,
// payload binario invalido, schema_version ausente/invalido). Analoga a
// SaveCorruptException do C#.
class SaveCorruptError : public std::runtime_error {
   public:
    explicit SaveCorruptError(const std::string& message)
        : std::runtime_error(message) {}
};

// Save de versao FUTURA (schema_version > versao atual do schema). Forward-only:
// migracao e unidirecional, o jogo nao le um save de versao mais nova de si mesmo.
// Analoga a SaveVersionTooNewException do C#.
class SaveVersionTooNewError : public std::runtime_error {
   public:
    SaveVersionTooNewError(int save_version, int current_version)
        : std::runtime_error("Save versao " + std::to_string(save_version) +
                             " e mais nova que a suportada " +
                             std::to_string(current_version) +
                             " (forward-only: atualize o jogo)."),
          save_version_(save_version),
          current_version_(current_version) {}

    [[nodiscard]] int save_version() const noexcept { return save_version_; }
    [[nodiscard]] int current_version() const noexcept { return current_version_; }

   private:
    int save_version_;
    int current_version_;
};

// ---- serialize / deserialize ----------------------------------------------

// Serializa SaveData no formato atual (V2). Valida invariantes antes (fail-fast:
// lanca std::invalid_argument se invalido). Carimbo timestamp_ms ja deve estar
// preenchido pelo chamador (injetado, ADR-006 item 4).
[[nodiscard]] std::vector<std::uint8_t> serialize_save(const SaveData& data);

// IGUAL a serialize_save mas SEM validar invariantes: usado SO por testes para
// forjar payloads schema-divergentes (HP negativo etc.) e provar que o LOAD
// rejeita. Nao usar em codigo de producao.
[[nodiscard]] std::vector<std::uint8_t> serialize_save_unchecked(
    const SaveData& data);

// Load version-aware (forward-only). Valida HMAC, le versao, rejeita futuro, roda
// a chain de migracao se atras, materializa e valida invariantes. Lanca
// SaveIntegrityError / SaveCorruptError / SaveVersionTooNewError / invalid_argument.
[[nodiscard]] SaveData deserialize_save(const std::vector<std::uint8_t>& data);

// ---- envelope cru (pack/unpack), testavel diretamente ----------------------

// Monta o envelope: MAGIC || LENGTH || payload || HMAC(MAGIC||LENGTH||payload).
[[nodiscard]] std::vector<std::uint8_t> pack_save(
    const std::vector<std::uint8_t>& payload);

// Valida o envelope e extrai o PAYLOAD. Lanca SaveCorruptError (formato) ou
// SaveIntegrityError (HMAC mismatch).
[[nodiscard]] std::vector<std::uint8_t> unpack_save(
    const std::vector<std::uint8_t>& data);

}  // namespace gus::domain::save

#endif  // GUS_DOMAIN_SAVE_SAVE_SERIALIZER_HPP

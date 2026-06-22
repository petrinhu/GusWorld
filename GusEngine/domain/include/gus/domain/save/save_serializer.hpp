// gus/domain/save/save_serializer.hpp
//
// Serializacao BINARIA PROPRIA + HMAC-SHA256 (do core/) anti-tamper do estado de
// SAVE. ADR-006: formato binario proprio (NAO JSON, NAO o byte-format do C#),
// selado pelo HMAC proprio de gus::core::crypto. POCO puro, ZERO Qt.
//
// ENVELOPE (little-endian), mesma estrutura do template serializer:
//   [4]      MAGIC   = "GDS2" (GusDragon Save). Distingue de templates ("GDT1"). O
//                      sufixo do magic NAO e a versao do schema (que vive no payload).
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
//   map<str, CharacterSaveState{ i32 current_hp | i32 xp | list<str> deck }> |
//   map<str,i32> enemy_knowledge (V3) |
//   input_remap_backup (V4: u32 config_version | u32 actions_count | repeat:
//     str action_name | f32 deadzone |
//     u32 keys_count{ i64 keycode | u8 ctrl | u8 shift | u8 alt } |
//     u32 gamepad_buttons_count{ i32 } | u32 mouse_buttons_count{ i32 } |
//     u32 gamepad_axes_count{ i32 axis | f32 axis_value }) |
//   controls_hash128 (V4: 16 bytes crus) | i32 slot_id (V4)
// (mapas gravados em ordem de chave; actions do backup na ordem do vetor que o caller
// monta na ordem do ActionRegistry: std::map/ordem = serializacao deterministica).
// Os layouts antigos (V1 sem character_states; V2 sem enemy_knowledge; V3 sem campos
// V4) sao produzidos por serialize_save_v1/v2/v3, para as fixtures de migracao.
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

// Serializa SaveData no formato atual (V4). Valida invariantes antes (fail-fast:
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

// ---- T1.1 detect-and-respond: load NAO-lancante por valor ------------------
//
// Resultado de um load (camada PURA, sem UI). A camada app decide o que avisar:
// HmacInvalid (adulterado) / Corrupt (malformado) / VersionTooNew (futuro) =
// oferecer o slot anterior; WrongSlot = avisar troca de arquivo entre slots (T1.2).
enum class LoadResult {
    Ok,             // carregou e validou
    HmacInvalid,    // HMAC nao bate: adulterado
    Corrupt,        // malformado (magic/length/truncado/payload invalido)
    VersionTooNew,  // schema_version > atual (forward-only)
    Invalid,        // payload bem-formado mas schema-divergente (invariante violada)
    WrongSlot,      // slot_id selado != expected_slot (arquivo trocado de slot)
};

// Saida de load_save: o resultado + os dados (validos sse result == Ok ou WrongSlot;
// em WrongSlot os dados estao integros, so a origem do slot diverge, e a app decide).
struct LoadOutcome {
    LoadResult result = LoadResult::Corrupt;
    SaveData data;
};

// Load NAO-lancante (T1.1): mesma logica de deserialize_save, mas sinaliza por valor
// em vez de lancar, para a camada app decidir avisar/oferecer o slot anterior. Pura.
//
// expected_slot (T1.2): o slot de onde o arquivo foi lido (a camada de I/O sabe).
// Se o slot_id selado no payload divergir de expected_slot, o resultado e WrongSlot
// (mas data fica preenchida). expected_slot < 0 = "nao verificar slot" (import
// avulso): nunca reporta WrongSlot.
[[nodiscard]] LoadOutcome load_save(const std::vector<std::uint8_t>& data,
                                    int expected_slot);

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

// gus/domain/save/save_serializer.hpp
//
// Serializacao BINARIA PROPRIA + AEAD XChaCha20-Poly1305 (do core/, sobre o
// Monocypher vendorizado) do estado de SAVE. ADR-015 (SAVE-CRYPTO-V2-ENVELOPE):
// envelope GDS3 substitui o GDS2/HMAC-SHA256 do ADR-006 - cifra E autentica numa
// unica operacao (confidencialidade + integridade), em vez de so integridade. O
// HMAC-SHA256 proprio de core/ NAO e removido: continua servindo outros
// consumidores que nao sao save (controls_hash128 ADR-007, selo de conteudo
// ADR-012), fora do escopo deste ADR. POCO puro, ZERO Qt.
//
// ENVELOPE GDS3 (little-endian):
//   [4]   MAGIC          = "GDS3" (GusDragon Save v3). Distingue de templates
//                          ("GDT1", que segue INTOCADO no formato GDS2/HMAC - ADR-015
//                          e escopo SO do save). O sufixo do magic NAO e a versao do
//                          schema do domain (que vive no payload).
//   [2]   ENVELOPE_VER   = uint16 LE = versao do FORMATO do envelope (distinta do
//                          schema_version do payload).
//   [4]   SLOT_ID        = int32 LE = slot fisico, AUTENTICADO por AAD (nao mais
//                          HMAC). Mesmo papel do T1.2 (ADR-007), so que agora a
//                          checagem estrutural mora dentro da propria primitiva AEAD
//                          (qualquer troca deste campo sem a chave quebra a tag).
//   [8]   ROLLBACK_CTR   = uint64 LE = contador anti-rollback (ADR-015 decisao 4).
//                          SEMPRE 0 nos saves normais nesta onda (o slot Hardcore com
//                          anti-rollback estrito e a Onda 4, fora deste ADR).
//   [24]  NONCE          = XChaCha nonce ALEATORIO por escrita (CSPRNG, nunca
//                          reusado - core::crypto::generate_nonce()).
//   [4]   CIPHERTEXT_LEN = uint32 LE = tamanho do CIPHERTEXT em bytes.
//   [N]   CIPHERTEXT     = AEAD-lock do payload (schema_version + campos V1..V5 do
//                          domain, layout INALTERADO - so o que ENVOLVE o payload
//                          mudou de "concatenar+HMAC" para "cifrar+autenticar").
//   [16]  TAG            = Poly1305, autentica CIPHERTEXT + AAD.
//
// AAD (dado autenticado, NAO cifrado) = MAGIC || ENVELOPE_VER || SLOT_ID ||
// ROLLBACK_CTR (os primeiros 18 bytes do envelope). Flip de QUALQUER byte do
// envelope (magic/ver/slot_id/rollback_ctr/nonce/ciphertext/tag) quebra a
// verificacao AEAD e lanca SaveIntegrityError. Sem reparo.
//
// Chave: Argon2id (piso OWASP m=19456 KiB/t=2/p=1, ADR-015 decisao 3) sobre o
// segredo-base embutido + sal fixo "gusworld-save-v2". SEM fingerprint de maquina
// nesta onda (saves normais sao PORTAVEIS cross-PC; machine-bind e a Onda 3, so
// Hardcore). Cache estatico: a derivacao roda uma vez por processo, nao por save.
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
//   1. valida a tag AEAD do envelope (integridade+confidencialidade ANTES de
//      versao: nunca migra bytes adulterados) -> SaveIntegrityError em mismatch;
//   2. le schema_version do payload validado;
//   3. version > atual -> SaveVersionTooNewError (forward-only nao le o futuro);
//   4. version < atual -> roda a chain (V1->V2->...) antes de materializar;
//   5. version == atual -> materializa direto.
//
// Cross-ref: engine/foundation/save_system/SaveSerializer.cs (ref semantica),
//            gus/domain/templates/template_serializer.hpp (envelope IRMAO, GDS2/
//            HMAC - o save divergiu pro GDS3/AEAD no ADR-015; templates ficaram
//            de fora do escopo),
//            gus/domain/save/save_migrators.hpp, ADR-006, ADR-015.

#ifndef GUS_DOMAIN_SAVE_SAVE_SERIALIZER_HPP
#define GUS_DOMAIN_SAVE_SAVE_SERIALIZER_HPP

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include "gus/domain/save/save_data.hpp"

namespace gus::domain::save {

// Tag AEAD do save nao bate (nome historico "Integrity", da era GDS2/HMAC do
// ADR-006; desde o ADR-015/GDS3 cobre qualquer falha da verificacao AEAD -
// ciphertext/tag/nonce/AAD adulterados, ou chave errada): arquivo
// adulterado/tampered. Analoga a SaveIntegrityException do C#.
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

// Serializa SaveData no formato atual (V5), envelope GDS3. Valida invariantes
// antes (fail-fast: lanca std::invalid_argument se invalido). Carimbo
// timestamp_ms ja deve estar preenchido pelo chamador (injetado, ADR-006 item 4).
// O slot_id do envelope (AAD) vem de data.slot_id; rollback_ctr = 0 (saves
// normais, ADR-015 Onda 2 - anti-rollback estrito e so o slot Hardcore, Onda 4).
[[nodiscard]] std::vector<std::uint8_t> serialize_save(const SaveData& data);

// IGUAL a serialize_save mas SEM validar invariantes: usado SO por testes para
// forjar payloads schema-divergentes (HP negativo etc.) e provar que o LOAD
// rejeita. Nao usar em codigo de producao.
[[nodiscard]] std::vector<std::uint8_t> serialize_save_unchecked(
    const SaveData& data);

// Load version-aware (forward-only). Valida a tag AEAD do envelope, le versao,
// rejeita futuro, roda a chain de migracao se atras, materializa e valida
// invariantes. Lanca SaveIntegrityError / SaveCorruptError /
// SaveVersionTooNewError / invalid_argument.
[[nodiscard]] SaveData deserialize_save(const std::vector<std::uint8_t>& data);

// ---- T1.1 detect-and-respond: load NAO-lancante por valor ------------------
//
// Resultado de um load (camada PURA, sem UI). A camada app decide o que avisar:
// HmacInvalid (adulterado) / Corrupt (malformado) / VersionTooNew (futuro) =
// oferecer o slot anterior; WrongSlot = avisar troca de arquivo entre slots (T1.2).
enum class LoadResult {
    Ok,             // carregou e validou
    // Nome historico do ADR-006 (HMAC-SHA256); desde o ADR-015/GDS3 cobre
    // qualquer falha de verificacao do envelope AEAD (tag Poly1305 nao bate:
    // adulterado, chave errada, ou AAD divergente). Mantido para nao repercutir
    // o rename em app/platform (fora do escopo deste ADR).
    HmacInvalid,    // tag AEAD nao bate: adulterado
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

// Monta o envelope GDS3: MAGIC || ENVELOPE_VER || SLOT_ID || ROLLBACK_CTR ||
// NONCE || CIPHERTEXT_LEN || AEAD-lock(payload, aad=os 4 primeiros campos) ||
// TAG. slot_id/rollback_ctr default (-1/0) = "nao vinculado a slot" (import
// avulso/testes de envelope cru); os callers de producao (serialize_save) passam
// o slot_id real do SaveData.
[[nodiscard]] std::vector<std::uint8_t> pack_save(
    const std::vector<std::uint8_t>& payload, std::int32_t slot_id = -1,
    std::uint64_t rollback_ctr = 0);

// Valida a tag AEAD do envelope e extrai o PAYLOAD decifrado. Lanca
// SaveCorruptError (formato: magic/envelope_ver/length) ou SaveIntegrityError
// (tag AEAD nao bate: adulterado, chave errada, ou slot_id/rollback_ctr do AAD
// divergentes dos usados no pack).
[[nodiscard]] std::vector<std::uint8_t> unpack_save(
    const std::vector<std::uint8_t>& data);

}  // namespace gus::domain::save

#endif  // GUS_DOMAIN_SAVE_SAVE_SERIALIZER_HPP

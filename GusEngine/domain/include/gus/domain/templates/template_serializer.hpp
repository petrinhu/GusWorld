// gus/domain/templates/template_serializer.hpp
//
// Serializacao BINARIA PROPRIA + HMAC-SHA256 (do core/) anti-tamper para templates
// .gdt. ADR-006: formato binario proprio (NAO JSON, NAO o byte-format do C#), selado
// pelo HMAC proprio de gus::core::crypto. POCO puro, ZERO Qt.
//
// ENVELOPE (little-endian), igual em estrutura ao .gdt do C#:
//   [4]      MAGIC   = "GDT1" (0x47 0x44 0x54 0x31). Detecta formato/versao errados.
//   [4]      LENGTH  = uint32 LE = tamanho do PAYLOAD em bytes.
//   [LENGTH] PAYLOAD = serializacao binaria compacta PROPRIA (ver abaixo).
//   [32]     HMAC    = HMAC-SHA256( MAGIC || LENGTH || PAYLOAD ), chave embutida.
//
// O HMAC cobre header + payload: flip de QUALQUER byte (magic/length/payload/hmac)
// quebra o selo e lanca TemplateIntegrityError. Sem reparo.
//
// PAYLOAD (formato proprio, distinto do JSON do C#; little-endian):
//   CharacterTemplate:
//     u32 id_len | id[id_len] | i32 max_hp | i32 atk | i32 def | i32 spd |
//     u32 family | u8 is_universal_compiler | u32 deck_count |
//       (u32 card_len | card[card_len])*deck_count
//   EnemyTemplate:
//     u32 id_len | id[id_len] | i32 max_hp | i32 atk | i32 def | i32 spd |
//     u32 family | u32 brain | u8 is_boss | u32 deck_count |
//       (u32 card_len | card[card_len])*deck_count | u32 kind | u8 central_command
//     (kind = MODOS-MORTE Fase 0, EnemyKind: Creature/Human - campo aditivo no
//     FINAL do payload, ver enemy_template.hpp; central_command = CARD-ENGINE-MANIFESTO
//     item 9, Mises/Calc-Edge - campo aditivo SEGUINTE, MESMO padrao append-only. NOTA:
//     este formato NAO tem discriminador de versao (nenhum campo de schema-version no
//     envelope) - cada campo novo append-only e seguro SOMENTE enquanto nao houver .gdt
//     persistido com o schema anterior; nao ha nenhum .gdt em disco/repo hoje.)
//
// O int32 e gravado como 4 bytes LE do bit-pattern (stats sao >= 0 por invariante,
// mas o codec e fiel ao tipo). Chave de integridade FIXA embutida (integridade
// casual local, nao sigilo: ADR-006).
//
// Cross-ref: engine/foundation/data/TemplateSerializer.cs (referencia SEMANTICA, nao
//            binaria), gus/core/crypto/hmac_sha256.hpp, ADR-006.

#ifndef GUS_DOMAIN_TEMPLATES_TEMPLATE_SERIALIZER_HPP
#define GUS_DOMAIN_TEMPLATES_TEMPLATE_SERIALIZER_HPP

#include <cstdint>
#include <stdexcept>
#include <vector>

#include "gus/domain/templates/character_template.hpp"
#include "gus/domain/templates/enemy_template.hpp"

namespace gus::domain::templates {

// Lancada quando o HMAC do .gdt nao bate: arquivo adulterado/tampered. Analoga a
// TemplateIntegrityException do C#.
class TemplateIntegrityError : public std::runtime_error {
   public:
    explicit TemplateIntegrityError(const std::string& message)
        : std::runtime_error(message) {}
};

// Lancada quando o .gdt esta estruturalmente corrompido (magic errado, truncado,
// length inconsistente, payload binario invalido/incompleto). Analoga a
// TemplateCorruptException do C#.
class TemplateCorruptError : public std::runtime_error {
   public:
    explicit TemplateCorruptError(const std::string& message)
        : std::runtime_error(message) {}
};

// ---- CharacterTemplate -----------------------------------------------------

// Serializa um CharacterTemplate em bytes .gdt (valida invariantes antes, fail-fast:
// lanca std::invalid_argument se invalido).
[[nodiscard]] std::vector<std::uint8_t> serialize_character(
    const CharacterTemplate& tpl);

// Desserializa bytes .gdt num CharacterTemplate. Valida HMAC (TemplateIntegrityError)
// + formato (TemplateCorruptError) + invariantes (std::invalid_argument).
[[nodiscard]] CharacterTemplate deserialize_character(
    const std::vector<std::uint8_t>& data);

// ---- EnemyTemplate ---------------------------------------------------------

// Serializa um EnemyTemplate em bytes .gdt (valida invariantes antes).
[[nodiscard]] std::vector<std::uint8_t> serialize_enemy(const EnemyTemplate& tpl);

// Desserializa bytes .gdt num EnemyTemplate. Valida HMAC + formato + invariantes.
[[nodiscard]] EnemyTemplate deserialize_enemy(const std::vector<std::uint8_t>& data);

// ---- Envelope cru (pack/unpack), testavel diretamente ----------------------

// Monta o envelope: MAGIC || LENGTH || payload || HMAC(MAGIC||LENGTH||payload).
[[nodiscard]] std::vector<std::uint8_t> pack(
    const std::vector<std::uint8_t>& payload);

// Valida o envelope e extrai o PAYLOAD. Lanca TemplateCorruptError (formato) ou
// TemplateIntegrityError (HMAC mismatch).
[[nodiscard]] std::vector<std::uint8_t> unpack(
    const std::vector<std::uint8_t>& data);

}  // namespace gus::domain::templates

#endif  // GUS_DOMAIN_TEMPLATES_TEMPLATE_SERIALIZER_HPP

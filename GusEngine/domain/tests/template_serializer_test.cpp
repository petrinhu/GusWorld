// template_serializer_test.cpp
//
// Spec executavel (Catch2 v3) do TemplateSerializer (envelope BINARIO PROPRIO +
// HMAC-SHA256 do core/, ADR-006). Portado de engine/tests/data/TemplateSerializerTests.cs
// (xUnit), mas com ORACULO DE EQUIVALENCIA SEMANTICA (nao byte-a-byte vs C#): o
// formato e NOSSO. NAO ha leitura de bytes do C#.
//
// Oraculo (ADR-006 decisao 3):
//   (a) roundtrip: objeto -> serialize -> deserialize -> objeto IDENTICO;
//   (b) tamper: flip de 1 byte do envelope -> deserializacao REJEITA (HMAC nao bate);
//   (c) determinismo: mesmo objeto + mesma chave -> mesmo selo (bytes identicos).
//
// Envelope (little-endian): MAGIC "GDT1" || LENGTH(uint32) || PAYLOAD || HMAC(32),
// HMAC sobre MAGIC||LENGTH||PAYLOAD. Payload = binario compacto proprio (NAO JSON,
// NAO o byte-format do C#).
//
// Subsistema: domain/templates (marco M3). POCO puro, ZERO Qt, headless.
//
// Cross-ref: engine/foundation/data/TemplateSerializer.cs (referencia semantica),
//            docs/tech/adr/ADR-006-crypto-hmac-formato-domain.md.

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include "gus/domain/templates/character_template.hpp"
#include "gus/domain/templates/enemy_template.hpp"
#include "gus/domain/templates/template_serializer.hpp"

using gus::domain::templates::BrainKind;
using gus::domain::templates::CardFamily;
using gus::domain::templates::CharacterTemplate;
using gus::domain::templates::deserialize_character;
using gus::domain::templates::deserialize_enemy;
using gus::domain::templates::EnemyKind;
using gus::domain::templates::EnemyTemplate;
using gus::domain::templates::pack;
using gus::domain::templates::serialize_character;
using gus::domain::templates::serialize_enemy;
using gus::domain::templates::TemplateCorruptError;
using gus::domain::templates::TemplateIntegrityError;
using gus::domain::templates::unpack;

namespace {

CharacterTemplate gus_fixture() {
    return CharacterTemplate{"gus",  34,   8,    5, 9, CardFamily::Eletrico,
                             true,   {"pulso_eletrico", "scan_basico", "glifo_root"}};
}

CharacterTemplate caua_fixture() {
    return CharacterTemplate{"caua", 55,   14,   8, 13, CardFamily::Eletrico,
                             false,  {"pulso_eletrico", "stream_raio"}};
}

EnemyTemplate daemon_fixture() {
    return EnemyTemplate{"daemon_guard",     144,  11, 14, 6, CardFamily::Cinetico,
                         BrainKind::Scripted, false, {"pulso_cinetico"}};
}

EnemyTemplate sentinela_fixture() {
    return EnemyTemplate{"sentinela_bit",     55,    6, 8, 4, CardFamily::Cinetico,
                         BrainKind::Scripted, false, {}};
}

}  // namespace

// ---- (a) roundtrip: CharacterTemplate -------------------------------------

TEST_CASE("serializer: character roundtrip preserva todos os campos",
          "[domain][templates][serializer]") {
    for (const auto& original : {gus_fixture(), caua_fixture()}) {
        const auto bytes = serialize_character(original);
        const auto restored = deserialize_character(bytes);
        REQUIRE(restored == original);  // record igualdade por valor (oraculo a)
    }
}

TEST_CASE("serializer: deck vazio roundtrippa como vazio, nao some",
          "[domain][templates][serializer]") {
    auto original = gus_fixture();
    original.base_deck = {};
    const auto restored = deserialize_character(serialize_character(original));
    REQUIRE(restored.base_deck.empty());
    REQUIRE(restored == original);
}

// ---- (a) roundtrip: EnemyTemplate -----------------------------------------

TEST_CASE("serializer: enemy roundtrip preserva todos os campos",
          "[domain][templates][serializer]") {
    for (const auto& original : {sentinela_fixture(), daemon_fixture()}) {
        const auto bytes = serialize_enemy(original);
        const auto restored = deserialize_enemy(bytes);
        REQUIRE(restored == original);
    }
}

// ---- MODOS-MORTE Fase 0: EnemyKind ----------------------------------------

TEST_CASE("serializer: EnemyKind::Human roundtrippa (default e Creature)",
          "[domain][templates][serializer][modos-morte]") {
    auto original = daemon_fixture();
    REQUIRE(original.kind == EnemyKind::Creature);  // default, sem consumidor ainda
    original.kind = EnemyKind::Human;
    const auto restored = deserialize_enemy(serialize_enemy(original));
    REQUIRE(restored.kind == EnemyKind::Human);
    REQUIRE(restored == original);
}

TEST_CASE("serializer: EnemyTemplate.kind com ordinal fora do dominio rejeitado",
          "[domain][templates][serializer][modos-morte]") {
    // MESMO hardening de family/brain (A1) aplicado ao campo novo.
    auto tpl = daemon_fixture();
    REQUIRE_NOTHROW(tpl.validate());  // sanity: valido hoje
    tpl.kind = static_cast<EnemyKind>(99u);
    REQUIRE_THROWS_AS(tpl.validate(), std::invalid_argument);
}

// ---- header binario valido ------------------------------------------------

TEST_CASE("serializer: layout magic || length || payload || hmac",
          "[domain][templates][serializer]") {
    const auto bytes = serialize_enemy(sentinela_fixture());

    // MAGIC nos 4 primeiros bytes = "GDT1".
    REQUIRE(bytes[0] == 'G');
    REQUIRE(bytes[1] == 'D');
    REQUIRE(bytes[2] == 'T');
    REQUIRE(bytes[3] == '1');

    // LENGTH (uint32 LE) = total - header(8) - hmac(32).
    const std::uint32_t declared = static_cast<std::uint32_t>(bytes[4]) |
                                   (static_cast<std::uint32_t>(bytes[5]) << 8) |
                                   (static_cast<std::uint32_t>(bytes[6]) << 16) |
                                   (static_cast<std::uint32_t>(bytes[7]) << 24);
    REQUIRE(declared == bytes.size() - 8u - 32u);
    REQUIRE(bytes.size() > 40u);
}

// ---- (b) tamper: byte-flip rejeitado em qualquer regiao -------------------

TEST_CASE("serializer: flip no payload rejeita (HMAC nao bate)",
          "[domain][templates][serializer]") {
    auto bytes = serialize_character(gus_fixture());
    bytes[bytes.size() / 2] ^= 0xFF;  // meio do envelope (payload)
    REQUIRE_THROWS_AS(deserialize_character(bytes), TemplateIntegrityError);
}

TEST_CASE("serializer: flip no bloco HMAC rejeita",
          "[domain][templates][serializer]") {
    auto bytes = serialize_enemy(daemon_fixture());
    bytes.back() ^= 0xFF;  // ultimo byte = dentro do HMAC
    REQUIRE_THROWS_AS(deserialize_enemy(bytes), TemplateIntegrityError);
}

TEST_CASE("serializer: flip no magic rejeita como corrupcao (antes do HMAC)",
          "[domain][templates][serializer]") {
    auto bytes = serialize_character(gus_fixture());
    bytes[0] ^= 0xFF;
    REQUIRE_THROWS_AS(deserialize_character(bytes), TemplateCorruptError);
}

TEST_CASE("serializer: flip no campo length rejeita como corrupcao",
          "[domain][templates][serializer]") {
    auto bytes = serialize_character(gus_fixture());
    bytes[4] ^= 0xFF;  // length declarado != real
    REQUIRE_THROWS_AS(deserialize_character(bytes), TemplateCorruptError);
}

// ---- (c) determinismo: mesmo objeto + mesma chave => mesmo selo -----------

TEST_CASE("serializer: serializacao deterministica (mesmo selo)",
          "[domain][templates][serializer]") {
    REQUIRE(serialize_enemy(daemon_fixture()) == serialize_enemy(daemon_fixture()));
    REQUIRE(serialize_character(gus_fixture()) == serialize_character(gus_fixture()));
}

TEST_CASE("serializer: objetos diferentes geram envelopes diferentes",
          "[domain][templates][serializer]") {
    REQUIRE(serialize_character(gus_fixture()) != serialize_character(caua_fixture()));
}

// ---- corrupt / truncado ---------------------------------------------------

TEST_CASE("serializer: dados vazios rejeitam como corrupcao",
          "[domain][templates][serializer]") {
    REQUIRE_THROWS_AS(unpack({}), TemplateCorruptError);
}

TEST_CASE("serializer: dados truncados rejeitam como corrupcao",
          "[domain][templates][serializer]") {
    auto bytes = serialize_enemy(sentinela_fixture());
    bytes.resize(bytes.size() - 10);  // corta cauda do HMAC
    REQUIRE_THROWS_AS(unpack(bytes), TemplateCorruptError);
}

TEST_CASE("serializer: curto demais (so header) rejeita",
          "[domain][templates][serializer]") {
    const std::vector<std::uint8_t> tiny{'G', 'D', 'T', '1', 0, 0, 0, 0};
    REQUIRE_THROWS_AS(unpack(tiny), TemplateCorruptError);
}

// ---- payload com HMAC valido mas conteudo invalido ------------------------

TEST_CASE("serializer: payload lixo com HMAC valido rejeita como corrupcao",
          "[domain][templates][serializer]") {
    const std::vector<std::uint8_t> garbage{0xDE, 0xAD, 0xBE, 0xEF};
    const auto packed = pack(garbage);  // HMAC valido sobre lixo
    REQUIRE_THROWS_AS(deserialize_character(packed), TemplateCorruptError);
}

// ---- Validate no load: payload bem-formado (HMAC valido) mas viola invariante

TEST_CASE("serializer: load valida invariantes (max_hp 0 num payload forjado)",
          "[domain][templates][serializer]") {
    // Constroi a MAO um payload binario de CharacterTemplate com max_hp=0 (o
    // serialize normal recusaria no fail-fast; um .gdt forjado/schema-divergente
    // poderia te-lo). Empacota com HMAC valido. O load DEVE chamar validate() e
    // rejeitar. Layout do payload (LE): u32 id_len|id|i32 max_hp|atk|def|spd|
    // u32 family|u8 is_universal|u32 deck_count.
    std::vector<std::uint8_t> payload;
    auto put_u32 = [&](std::uint32_t v) {
        payload.push_back(static_cast<std::uint8_t>(v & 0xFF));
        payload.push_back(static_cast<std::uint8_t>((v >> 8) & 0xFF));
        payload.push_back(static_cast<std::uint8_t>((v >> 16) & 0xFF));
        payload.push_back(static_cast<std::uint8_t>((v >> 24) & 0xFF));
    };
    const std::string id = "x";
    put_u32(static_cast<std::uint32_t>(id.size()));
    payload.insert(payload.end(), id.begin(), id.end());
    put_u32(0u);  // max_hp = 0 (invalido)
    put_u32(1u);  // atk
    put_u32(1u);  // def
    put_u32(1u);  // spd
    put_u32(0u);  // family = Eletrico
    payload.push_back(0u);  // is_universal_compiler = false
    put_u32(0u);  // deck_count = 0

    const auto packed = pack(payload);  // HMAC valido sobre o payload forjado
    REQUIRE_THROWS_AS(deserialize_character(packed), std::invalid_argument);
}

TEST_CASE("serializer: serialize de template invalido lanca (fail-fast)",
          "[domain][templates][serializer]") {
    auto bad = gus_fixture();
    bad.max_hp = 0;
    REQUIRE_THROWS_AS(serialize_character(bad), std::invalid_argument);

    auto bad_enemy = sentinela_fixture();
    bad_enemy.id = "";
    REQUIRE_THROWS_AS(serialize_enemy(bad_enemy), std::invalid_argument);
}

// ---- pack/unpack roundtrip do envelope cru --------------------------------

TEST_CASE("serializer: pack/unpack devolve o payload identico",
          "[domain][templates][serializer]") {
    const std::vector<std::uint8_t> payload{1, 2, 3, 4, 5, 250, 0, 99};
    REQUIRE(unpack(pack(payload)) == payload);
}

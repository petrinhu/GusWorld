// save_serializer_test.cpp
//
// Spec executavel (Catch2 v3) do SaveSerializer (envelope BINARIO PROPRIO +
// HMAC-SHA256 do core/, ADR-006). Portado de
// engine/foundation/save_system/SaveSerializer.cs (C#: JSON + HMAC) com ORACULO DE
// EQUIVALENCIA SEMANTICA: o formato e NOSSO (binario compacto), NAO o JSON do C#.
// NAO ha leitura de bytes do C#.
//
// Oraculo (ADR-006 decisao 3):
//   (a) roundtrip: SaveData -> serialize -> deserialize -> SaveData IDENTICO;
//   (b) tamper: flip de 1 byte (magic/length/payload/hmac) -> REJEITA;
//   (c) determinismo: mesmo objeto + mesma chave -> mesmo selo (bytes identicos);
//   (d) validate no load: payload bem-formado mas schema-divergente -> rejeita.
//
// Envelope (little-endian): MAGIC "GDS2" || LENGTH(uint32) || PAYLOAD || HMAC(32),
// HMAC sobre MAGIC||LENGTH||PAYLOAD. Payload = binario compacto proprio, inicia com
// u32 schema_version (V2 atual, gus::domain::kSaveSchemaVersion).
//
// CARIMBO (ADR-006 item 4): SaveData carrega timestamp_ms (data+hora+ms) como
// metadado, INJETADO (nunca chama relogio dentro do domain). Roundtrippa intacto.
//
// Subsistema: domain/save (marco M3). POCO puro, ZERO Qt, headless.
//
// Cross-ref: engine/foundation/save_system/SaveSerializer.cs (ref semantica),
//            docs/tech/adr/ADR-006-crypto-hmac-formato-domain.md.

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include "gus/domain/domain_info.hpp"
#include "gus/domain/save/save_data.hpp"
#include "gus/domain/save/save_serializer.hpp"

using gus::domain::save::CharacterSaveState;
using gus::domain::save::deserialize_save;
using gus::domain::save::pack_save;
using gus::domain::save::SaveCorruptError;
using gus::domain::save::SaveData;
using gus::domain::save::SaveIntegrityError;
using gus::domain::save::serialize_save;
using gus::domain::save::unpack_save;

namespace {

// Fixture canonica congelada (oraculo): estado de save rico, todos os campos
// preenchidos, timestamp_ms fixo (1718900000123 = determinismo, sem relogio).
SaveData rich_fixture() {
    SaveData s;
    s.schema_version = gus::domain::kSaveSchemaVersion;  // V2
    s.timestamp_ms = 1718900000123LL;
    s.playtime_seconds = 3661.5;
    s.current_scene_path = "res://world/gusworld_city.tscn";
    s.player_position = {12.5, 0.0, -88.25};
    s.player_rotation = {0.0, 135.0, 0.0};
    s.party_roster = {"gus", "caua", "iara"};
    s.party_active = {"gus", "caua", "iara"};
    s.flags = {{"intro_done", true}, {"door_north", false}};
    s.inventory = {{"bio_ampola", 3}, {"credito", 144}};
    s.quest_progress = {{"q_gambito", 2}, {"q_main", 1}};
    s.relations = {{"caua", 5}, {"iara", 8}};
    s.character_states = {
        {"gus", CharacterSaveState{34, 120, {"pulso_eletrico", "scan_basico"}}},
        {"caua", CharacterSaveState{40, 89, {"stream_raio"}}},
    };
    // V3: conhecimento de bestiario por TIPO (enemy_type_id -> kills). std::map =
    // serializacao deterministica (selo estavel).
    s.enemy_knowledge = {{"sentinela_bit", 8}, {"daemon_fork", 13}};
    // V5 (MODOS-MORTE Fase 0): valor NAO-default, pra provar que roundtrippa de
    // verdade (nao so por acaso bater com o default do campo).
    s.difficulty = gus::domain::save::DifficultyLevel::Dificil;
    s.difficult_recovery_stage = 2;
    return s;
}

SaveData minimal_fixture() {
    SaveData s;
    s.schema_version = gus::domain::kSaveSchemaVersion;
    s.timestamp_ms = 1LL;
    s.current_scene_path = "res://boot.tscn";
    return s;
}

}  // namespace

// ---- (a) roundtrip ---------------------------------------------------------

TEST_CASE("save: roundtrip preserva TODOS os campos (oraculo a)",
          "[domain][save][serializer]") {
    const auto original = rich_fixture();
    const auto bytes = serialize_save(original);
    const auto restored = deserialize_save(bytes);
    REQUIRE(restored == original);
}

TEST_CASE("save: roundtrip de save minimo (mapas/listas vazios)",
          "[domain][save][serializer]") {
    const auto original = minimal_fixture();
    const auto restored = deserialize_save(serialize_save(original));
    REQUIRE(restored == original);
    REQUIRE(restored.party_roster.empty());
    REQUIRE(restored.character_states.empty());
}

TEST_CASE("save: carimbo de timestamp roundtrippa intacto (metadado)",
          "[domain][save][serializer]") {
    auto original = rich_fixture();
    original.timestamp_ms = 1718900000999LL;
    const auto restored = deserialize_save(serialize_save(original));
    REQUIRE(restored.timestamp_ms == 1718900000999LL);
}

TEST_CASE("save: conhecimento de inimigos roundtrippa por tipo (oraculo a)",
          "[domain][save][serializer]") {
    auto original = rich_fixture();
    const auto restored = deserialize_save(serialize_save(original));
    REQUIRE(restored.enemy_knowledge == original.enemy_knowledge);
    REQUIRE(restored.enemy_knowledge.at("sentinela_bit") == 8);
    REQUIRE(restored.enemy_knowledge.at("daemon_fork") == 13);
}

TEST_CASE("save: conhecimento vazio roundtrippa (1o encontro, variancia maxima)",
          "[domain][save][serializer]") {
    const auto restored = deserialize_save(serialize_save(minimal_fixture()));
    REQUIRE(restored.enemy_knowledge.empty());
}

TEST_CASE("save: conhecimento diferente muda o selo (determinismo por campo)",
          "[domain][save][serializer]") {
    auto a = rich_fixture();
    auto b = rich_fixture();
    b.enemy_knowledge["sentinela_bit"] = 9;  // delta de 1 kill
    REQUIRE(serialize_save(a) != serialize_save(b));
}

// ---- V5 (MODOS-MORTE Fase 0): difficulty + difficult_recovery_stage --------

TEST_CASE("save: difficulty + difficult_recovery_stage roundtrippam (V5)",
          "[domain][save][serializer][v5]") {
    auto original = rich_fixture();
    original.difficulty = gus::domain::save::DifficultyLevel::Hardcore;
    original.difficult_recovery_stage = 4;
    const auto restored = deserialize_save(serialize_save(original));
    REQUIRE(restored.difficulty == gus::domain::save::DifficultyLevel::Hardcore);
    REQUIRE(restored.difficult_recovery_stage == 4);
}

TEST_CASE("save: save novo (minimal_fixture) nasce em Medio (default §2.1)",
          "[domain][save][serializer][v5]") {
    const auto restored = deserialize_save(serialize_save(minimal_fixture()));
    REQUIRE(restored.difficulty == gus::domain::save::DifficultyLevel::Medio);
    REQUIRE(restored.difficult_recovery_stage == 0);
}

TEST_CASE("save: dificuldade diferente muda o selo (determinismo por campo)",
          "[domain][save][serializer][v5]") {
    auto a = rich_fixture();
    auto b = rich_fixture();
    b.difficulty = gus::domain::save::DifficultyLevel::Facil;
    REQUIRE(serialize_save(a) != serialize_save(b));
}

TEST_CASE("save: difficulty com ordinal fora do dominio rejeitado no load (forjado)",
          "[domain][save][serializer][v5]") {
    // MESMO padrao de hardening de EnemyTemplate::validate() (A1) - um payload
    // selado mas schema-divergente nao passa silenciosamente.
    auto s = rich_fixture();
    s.difficulty = static_cast<gus::domain::save::DifficultyLevel>(99u);
    const auto bytes = gus::domain::save::serialize_save_unchecked(s);
    REQUIRE_THROWS_AS(deserialize_save(bytes), std::invalid_argument);
}

TEST_CASE("save: difficult_recovery_stage fora de [0,4] rejeitado no load (forjado)",
          "[domain][save][serializer][v5]") {
    auto s = rich_fixture();
    s.difficult_recovery_stage = 5;
    const auto bytes = gus::domain::save::serialize_save_unchecked(s);
    REQUIRE_THROWS_AS(deserialize_save(bytes), std::invalid_argument);
}

// ---- header binario valido -------------------------------------------------

TEST_CASE("save: layout magic 'GDS2' || length || payload || hmac",
          "[domain][save][serializer]") {
    const auto bytes = serialize_save(minimal_fixture());

    REQUIRE(bytes[0] == 'G');
    REQUIRE(bytes[1] == 'D');
    REQUIRE(bytes[2] == 'S');
    REQUIRE(bytes[3] == '2');

    const std::uint32_t declared = static_cast<std::uint32_t>(bytes[4]) |
                                   (static_cast<std::uint32_t>(bytes[5]) << 8) |
                                   (static_cast<std::uint32_t>(bytes[6]) << 16) |
                                   (static_cast<std::uint32_t>(bytes[7]) << 24);
    REQUIRE(declared == bytes.size() - 8u - 32u);
}

// ---- (b) tamper: byte-flip rejeitado em qualquer regiao --------------------

TEST_CASE("save: flip no payload rejeita (HMAC nao bate)",
          "[domain][save][serializer]") {
    auto bytes = serialize_save(rich_fixture());
    bytes[bytes.size() / 2] ^= 0xFF;
    REQUIRE_THROWS_AS(deserialize_save(bytes), SaveIntegrityError);
}

TEST_CASE("save: flip no bloco HMAC rejeita",
          "[domain][save][serializer]") {
    auto bytes = serialize_save(rich_fixture());
    bytes.back() ^= 0xFF;
    REQUIRE_THROWS_AS(deserialize_save(bytes), SaveIntegrityError);
}

TEST_CASE("save: flip no magic rejeita como corrupcao (antes do HMAC)",
          "[domain][save][serializer]") {
    auto bytes = serialize_save(rich_fixture());
    bytes[0] ^= 0xFF;
    REQUIRE_THROWS_AS(deserialize_save(bytes), SaveCorruptError);
}

TEST_CASE("save: flip no campo length rejeita como corrupcao",
          "[domain][save][serializer]") {
    auto bytes = serialize_save(rich_fixture());
    bytes[4] ^= 0xFF;
    REQUIRE_THROWS_AS(deserialize_save(bytes), SaveCorruptError);
}

// ---- (c) determinismo ------------------------------------------------------

TEST_CASE("save: serializacao deterministica (mesmo selo)",
          "[domain][save][serializer]") {
    REQUIRE(serialize_save(rich_fixture()) == serialize_save(rich_fixture()));
}

TEST_CASE("save: objetos diferentes geram envelopes diferentes",
          "[domain][save][serializer]") {
    auto a = rich_fixture();
    auto b = rich_fixture();
    b.playtime_seconds = a.playtime_seconds + 1.0;
    REQUIRE(serialize_save(a) != serialize_save(b));
}

// ---- corrupt / truncado ----------------------------------------------------

TEST_CASE("save: dados vazios rejeitam como corrupcao",
          "[domain][save][serializer]") {
    REQUIRE_THROWS_AS(unpack_save({}), SaveCorruptError);
}

TEST_CASE("save: dados truncados rejeitam como corrupcao",
          "[domain][save][serializer]") {
    auto bytes = serialize_save(minimal_fixture());
    bytes.resize(bytes.size() - 10);
    REQUIRE_THROWS_AS(unpack_save(bytes), SaveCorruptError);
}

TEST_CASE("save: payload lixo com HMAC valido rejeita como corrupcao",
          "[domain][save][serializer]") {
    const std::vector<std::uint8_t> garbage{0xDE, 0xAD, 0xBE, 0xEF};
    const auto packed = pack_save(garbage);  // HMAC valido sobre lixo
    REQUIRE_THROWS_AS(deserialize_save(packed), SaveCorruptError);
}

// ---- (d) validate no load: schema-divergente apos HMAC valido --------------

TEST_CASE("save: load valida invariantes (HP negativo em payload forjado)",
          "[domain][save][serializer]") {
    // Um save normal recusaria HP negativo no fail-fast; um payload forjado com
    // HMAC valido (chave vazou) ainda deve ser rejeitado no load por validate().
    auto s = rich_fixture();
    // Forja HP negativo direto na struct, serializa SEM validar (helper de teste),
    // e prova que o LOAD rejeita.
    s.character_states = {{"gus", CharacterSaveState{-1, 0, {}}}};
    const auto bytes = gus::domain::save::serialize_save_unchecked(s);
    REQUIRE_THROWS_AS(deserialize_save(bytes), std::invalid_argument);
}

TEST_CASE("save: serialize de save invalido lanca (fail-fast)",
          "[domain][save][serializer]") {
    auto bad = rich_fixture();
    bad.character_states = {{"gus", CharacterSaveState{0, -5, {}}}};  // Xp < 0
    REQUIRE_THROWS_AS(serialize_save(bad), std::invalid_argument);
}

TEST_CASE("save: kills de conhecimento negativo rejeitado no load (forjado)",
          "[domain][save][serializer]") {
    auto s = rich_fixture();
    s.enemy_knowledge = {{"sentinela_bit", -1}};  // kills < 0: impossivel honesto
    const auto bytes = gus::domain::save::serialize_save_unchecked(s);
    REQUIRE_THROWS_AS(deserialize_save(bytes), std::invalid_argument);
}

TEST_CASE("save: chave de conhecimento vazia rejeitada (fail-fast)",
          "[domain][save][serializer]") {
    auto bad = rich_fixture();
    bad.enemy_knowledge = {{"", 1}};  // enemy_type_id vazio: invalido
    REQUIRE_THROWS_AS(serialize_save(bad), std::invalid_argument);
}

// ---- pack/unpack roundtrip do envelope cru ---------------------------------

TEST_CASE("save: pack/unpack devolve o payload identico",
          "[domain][save][serializer]") {
    const std::vector<std::uint8_t> payload{9, 8, 7, 0, 255, 1, 2, 3};
    REQUIRE(unpack_save(pack_save(payload)) == payload);
}

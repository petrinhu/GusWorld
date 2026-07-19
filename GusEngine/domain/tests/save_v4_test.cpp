// save_v4_test.cpp
//
// Spec executavel (Catch2 v3) do SAVE V4 (ADR-007 item 3 + camadas T1.1/T1.2/T2.2).
// Estende o ORACULO de equivalencia semantica do save (roundtrip + tamper) aos
// campos NOVOS do V4, e cobre as 3 camadas anti-tamper escolhidas pelo lider:
//   T1.1 detect-and-respond: load_save(...) devolve LoadResult (Ok / HmacInvalid /
//        Corrupt / VersionTooNew / WrongSlot) em vez de so lancar (logica PURA, sem UI).
//   T1.2 slot-id selado: SaveData::slot_id roundtrippa selado; load_save com
//        expected_slot != slot_id -> WrongSlot (troca de arquivo entre slots).
//   T2.2 KDF: a chave do HMAC e derivada (transparente ao layout); coberto pelo
//        roundtrip continuar valido (o re-baseline do HMAC nao quebra o oraculo).
//
// Campos V4 aditivos no SaveData:
//   - input_remap_backup (InputRemapConfig): backup integral dos controles;
//   - controls_hash128 (array<uint8,16>): hash do controls.json canonico vigente;
//   - slot_id (int): id/origem do slot, selado dentro do payload (T1.2).
//
// Migrator forward-only migrate_v3_to_v4: backup = default canonico, hash = hash do
// default, slot_id = o slot lido.
//
// POCO puro, ZERO Qt, headless. Carimbo timestamp_ms injetado (sem relogio).
//
// Cross-ref: docs/tech/adr/ADR-007-controls-json-hash128-save-v4.md,
//            gus/domain/save/save_serializer.hpp, save_data.hpp, save_migrators.hpp,
//            gus/domain/input/{controls_hash,controls_restore}.hpp.

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "gus/domain/domain_info.hpp"
#include "gus/domain/input/controls_hash.hpp"
#include "gus/domain/input/controls_restore.hpp"
#include "gus/domain/input/input_binding.hpp"
#include "gus/domain/save/save_data.hpp"
#include "gus/domain/save/save_migrators.hpp"
#include "gus/domain/save/save_serializer.hpp"

using gus::domain::input::default_controls;
using gus::domain::input::InputRemapConfig;
using gus::domain::input::KeyBinding;
using gus::domain::save::deserialize_save;
using gus::domain::save::load_save;
using gus::domain::save::LoadResult;
using gus::domain::save::SaveData;
using gus::domain::save::SaveIntegrityError;
using gus::domain::save::serialize_save;
using gus::domain::save::serialize_save_v3;

namespace {

// Fixture V4 rica: todos os campos V1+V2+V3 preenchidos + os 3 campos V4.
SaveData rich_v4() {
    SaveData s;
    s.schema_version = gus::domain::kSaveSchemaVersion;  // 4
    s.timestamp_ms = 1718900000123LL;
    s.playtime_seconds = 3661.5;
    s.current_scene_path = "res://world/gusworld_city.tscn";
    s.player_position = {12.5, 0.0, -88.25};
    s.party_roster = {"gus", "caua"};
    s.party_active = {"gus", "caua"};
    s.inventory = {{"credito", 144}};
    s.enemy_knowledge = {{"sentinela_bit", 8}};

    // V4: backup nao-trivial (default + 1 binding alterado), hash arbitrario, slot.
    InputRemapConfig backup = default_controls();
    backup.actions[0].keys = {KeyBinding{.keycode = 12345, .ctrl_pressed = true}};
    s.input_remap_backup = backup;
    s.controls_hash128 = {0xDE, 0xAD, 0xBE, 0xEF, 1, 2, 3, 4,
                          5, 6, 7, 8, 9, 10, 11, 12};
    s.slot_id = 3;
    return s;
}

}  // namespace

// ---- ancora: kSaveSchemaVersion (V4 nasceu aqui; hoje aponta pro topo V7,
//      CARDS-HW-1 - o teste segue a ancora, nao um numero fixo) --------

TEST_CASE("save_v4: kSaveSchemaVersion aponta pro topo da chain (ancora)",
          "[domain][save][v4]") {
    REQUIRE(gus::domain::kSaveSchemaVersion == 7);
    REQUIRE(gus::domain::save::current_schema_version() ==
            gus::domain::kSaveSchemaVersion);
}

// ---- (a) roundtrip dos campos V4 (oraculo) ---------------------------------

TEST_CASE("save_v4: roundtrip preserva backup + hash128 + slot_id",
          "[domain][save][v4]") {
    const auto original = rich_v4();
    const auto restored = deserialize_save(serialize_save(original));
    REQUIRE(restored == original);
    REQUIRE(restored.input_remap_backup == original.input_remap_backup);
    REQUIRE(restored.controls_hash128 == original.controls_hash128);
    REQUIRE(restored.slot_id == original.slot_id);
}

// ---- determinismo por campo: mudar o backup muda o selo --------------------

TEST_CASE("save_v4: backup diferente muda o selo (determinismo por campo)",
          "[domain][save][v4]") {
    auto a = rich_v4();
    auto b = rich_v4();
    b.input_remap_backup.actions[0].keys[0].keycode = 99999;
    REQUIRE(serialize_save(a) != serialize_save(b));
}

TEST_CASE("save_v4: hash128 diferente muda o selo",
          "[domain][save][v4]") {
    auto a = rich_v4();
    auto b = rich_v4();
    b.controls_hash128[0] ^= 0xFF;
    REQUIRE(serialize_save(a) != serialize_save(b));
}

TEST_CASE("save_v4: slot_id diferente muda o selo",
          "[domain][save][v4]") {
    auto a = rich_v4();
    auto b = rich_v4();
    b.slot_id = 4;
    REQUIRE(serialize_save(a) != serialize_save(b));
}

// ---- (b) tamper nas regioes novas quebra o selo ----------------------------

TEST_CASE("save_v4: byte-flip em qualquer parte do ciphertext rejeita (AEAD)",
          "[domain][save][v4]") {
    // O backup e o hash sao os ULTIMOS campos do payload (cifrado); um flip no
    // ultimo byte do ciphertext (imediatamente antes da tag, ADR-015 GDS3) cai na
    // regiao nova. A tag AEAD cobre o ciphertext inteiro.
    auto bytes = serialize_save(rich_v4());
    const std::size_t tag_len = 16;  // ADR-015: tag Poly1305 (16 bytes no fim)
    REQUIRE(bytes.size() > tag_len + 4);
    bytes[bytes.size() - tag_len - 1] ^= 0xFF;  // ultimo byte do ciphertext
    REQUIRE_THROWS_AS(deserialize_save(bytes), SaveIntegrityError);
}

// ---- migrator V3 -> V4 ------------------------------------------------------

TEST_CASE("save_v4: save V3 carregado sobe para V4 (backup=default, hash=hash default)",
          "[domain][save][v4][migrators]") {
    SaveData v3;
    v3.schema_version = 3;
    v3.timestamp_ms = 7LL;
    v3.current_scene_path = "res://v3.tscn";
    v3.enemy_knowledge = {{"sentinela_bit", 8}};
    const auto bytes_v3 = serialize_save_v3(v3);  // envelope V3 selado

    // O load version-aware sobe ATE o topo (V3->V4->V5, MODOS-MORTE Fase 0) - a
    // materializacao intermediaria V4 (backup/hash default) continua acontecendo,
    // so o schema_version final ja e o atual.
    const auto loaded = deserialize_save(bytes_v3);
    REQUIRE(loaded.schema_version == gus::domain::kSaveSchemaVersion);
    // backup = default canonico
    REQUIRE(loaded.input_remap_backup == default_controls());
    // hash = hash 128 do default canonico
    REQUIRE(loaded.controls_hash128 ==
            gus::domain::input::controls_hash128(default_controls()));
    // campos V3 preservados
    REQUIRE(loaded.enemy_knowledge.at("sentinela_bit") == 8);
}

TEST_CASE("save_v4: migrator V3->V4 popula slot_id com o slot do proprio save",
          "[domain][save][v4][migrators]") {
    // O load por slot (load_save) sabe o slot lido; ao migrar um V3 (sem slot_id),
    // o slot_id resultante e o slot informado no load.
    SaveData v3;
    v3.schema_version = 3;
    v3.timestamp_ms = 7LL;
    v3.current_scene_path = "res://v3.tscn";
    const auto bytes_v3 = serialize_save_v3(v3);

    const auto outcome = load_save(bytes_v3, /*expected_slot=*/2);
    REQUIRE(outcome.result == LoadResult::Ok);
    REQUIRE(outcome.data.schema_version == gus::domain::kSaveSchemaVersion);
    REQUIRE(outcome.data.slot_id == 2);
}

// ---- T1.1 detect-and-respond: load_save devolve LoadResult -----------------

TEST_CASE("save_v4: load_save de save valido devolve Ok + dados",
          "[domain][save][v4][load_result]") {
    auto s = rich_v4();
    s.slot_id = 1;
    const auto outcome = load_save(serialize_save(s), /*expected_slot=*/1);
    REQUIRE(outcome.result == LoadResult::Ok);
    REQUIRE(outcome.data == s);
}

TEST_CASE("save_v4: load_save de save adulterado devolve HmacInvalid (nao lanca)",
          "[domain][save][v4][load_result]") {
    auto bytes = serialize_save(rich_v4());
    bytes[bytes.size() / 2] ^= 0xFF;  // adultera o payload
    const auto outcome = load_save(bytes, /*expected_slot=*/3);
    REQUIRE(outcome.result == LoadResult::HmacInvalid);
}

TEST_CASE("save_v4: load_save de bytes corrompidos devolve Corrupt (nao lanca)",
          "[domain][save][v4][load_result]") {
    std::vector<std::uint8_t> garbage{0x00, 0x01, 0x02};  // curto demais
    const auto outcome = load_save(garbage, /*expected_slot=*/0);
    REQUIRE(outcome.result == LoadResult::Corrupt);
}

TEST_CASE("save_v4: load_save de versao futura devolve VersionTooNew (nao lanca)",
          "[domain][save][v4][load_result]") {
    const auto bytes_future = gus::domain::save::make_v1_payload(99);
    const auto outcome = load_save(bytes_future, /*expected_slot=*/0);
    REQUIRE(outcome.result == LoadResult::VersionTooNew);
}

// ---- T1.2 slot-id selado: deteccao de troca de arquivo entre slots ---------

TEST_CASE("save_v4: slot_id batendo com expected_slot => Ok",
          "[domain][save][v4][wrong_slot]") {
    auto s = rich_v4();
    s.slot_id = 2;
    const auto outcome = load_save(serialize_save(s), /*expected_slot=*/2);
    REQUIRE(outcome.result == LoadResult::Ok);
}

TEST_CASE("save_v4: arquivo do slot 2 colocado no slot 5 => WrongSlot",
          "[domain][save][v4][wrong_slot]") {
    auto s = rich_v4();
    s.slot_id = 2;  // gravado como slot 2
    // O jogador copiou o arquivo para a posicao do slot 5: o payload selado ainda
    // diz slot 2. A integridade (HMAC) esta OK; a origem do slot e que diverge.
    const auto outcome = load_save(serialize_save(s), /*expected_slot=*/5);
    REQUIRE(outcome.result == LoadResult::WrongSlot);
    // Os dados continuam disponiveis (a camada app decide avisar/aceitar).
    REQUIRE(outcome.data.slot_id == 2);
}

TEST_CASE("save_v4: expected_slot negativo (origem desconhecida) nao reporta WrongSlot",
          "[domain][save][v4][wrong_slot]") {
    // -1 = "nao verificar slot" (ex.: import avulso). Aceita como Ok.
    auto s = rich_v4();
    s.slot_id = 2;
    const auto outcome = load_save(serialize_save(s), /*expected_slot=*/-1);
    REQUIRE(outcome.result == LoadResult::Ok);
}

// ---- deserialize_save (throwing) continua existindo p/ os testes legados ----

TEST_CASE("save_v4: deserialize_save (throwing) ainda funciona no caminho feliz",
          "[domain][save][v4]") {
    const auto s = rich_v4();
    REQUIRE(deserialize_save(serialize_save(s)) == s);
}

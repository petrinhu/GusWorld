// save_migrators_test.cpp
//
// Spec executavel (Catch2 v3) dos MIGRATORS forward-only do save (domain/save).
// Portado de engine/foundation/save_system/Migrators/* (C#: arvore JsonNode) com a
// MUDANCA do ADR-006: migrators operam sobre STRUCTS VERSIONADAS (V1 -> V2), NAO
// sobre arvore JSON. Cada versao tem sua FIXTURE.
//
// Schema atual = V3 (gus::domain::kSaveSchemaVersion):
//   V1 = schema inicial, SEM character_states (per-character);
//   V2 = + character_states. MigrateV1ToV2 popula character_states vazio (semantica
//        honesta de um save v1: party usa stats do template, sem delta).
//   V3 = + enemy_knowledge (conhecimento de bestiario por TIPO de inimigo).
//        MigrateV2ToV3 popula enemy_knowledge VAZIO: um save v2 nunca rastreou
//        Knowledge por tipo, logo "nenhum tipo conhecido" (variancia maxima no 1o
//        encontro) e a semantica honesta. NAO se deriva de character_states (keyed
//        por COMPANION, nao por tipo de inimigo: as chaves nao se mapeiam).
//
// Forward-only: rejeita versao FUTURA (save mais novo que o schema suportado).
//
// Cross-ref: engine/foundation/save_system/Migrators/MigrateV1ToV2.cs +
//            SaveMigratorChain.cs + SaveMigrators.cs (ref semantica), CONTRACT §7,
//            ADR-006.

#include <catch2/catch_test_macros.hpp>

#include <vector>

#include "gus/domain/domain_info.hpp"
#include "gus/domain/save/save_data.hpp"
#include "gus/domain/save/save_migrators.hpp"
#include "gus/domain/save/save_serializer.hpp"

using gus::domain::save::CharacterSaveState;
using gus::domain::save::current_schema_version;
using gus::domain::save::deserialize_save;
using gus::domain::save::make_v1_payload;
using gus::domain::save::SaveData;
using gus::domain::save::SaveVersionTooNewError;
using gus::domain::save::serialize_save_v1;

namespace {

// Fixture de um save V1 (estado antes de character_states existir). Helper de teste
// serializa este record V1 no formato de payload V1 (a "geracao antiga" do jogo).
SaveData v1_state_fixture() {
    SaveData s;
    s.schema_version = 1;  // V1
    s.timestamp_ms = 1000LL;
    s.playtime_seconds = 42.0;
    s.current_scene_path = "res://old_city.tscn";
    s.player_position = {1.0, 2.0, 3.0};
    s.player_rotation = {0.0, 90.0, 0.0};
    s.party_roster = {"gus"};
    s.party_active = {"gus"};
    s.flags = {{"f1", true}};
    s.inventory = {{"credito", 13}};
    s.quest_progress = {{"q1", 0}};
    s.relations = {{"caua", 1}};
    // V1 NAO tem character_states (vazio por construcao no payload V1).
    return s;
}

}  // namespace

// ---- versao atual da chain bate com o ancora kSaveSchemaVersion ------------

TEST_CASE("migrators: CurrentVersion da chain == kSaveSchemaVersion (guarda)",
          "[domain][save][migrators]") {
    REQUIRE(current_schema_version() == gus::domain::kSaveSchemaVersion);
    REQUIRE(current_schema_version() == 5);
}

// ---- migracao V1 -> V5: load de save antigo sobe todos os saltos da chain ---

TEST_CASE("migrators: save V1 carregado sobe ate V5 (campos novos vazios/default)",
          "[domain][save][migrators]") {
    const auto v1 = v1_state_fixture();
    const auto bytes_v1 = serialize_save_v1(v1);  // envelope com payload V1

    // O load version-aware roda a chain inteira (V1->V2->V3->V4->V5) ANTES de
    // materializar.
    const auto loaded = deserialize_save(bytes_v1);

    REQUIRE(loaded.schema_version == 5);              // bumpado ate o topo
    REQUIRE(loaded.character_states.empty());          // campo V2, vazio
    REQUIRE(loaded.enemy_knowledge.empty());           // campo V3, vazio
    // V5 (MODOS-MORTE Fase 0): saves pre-dificuldade sobem como Medio.
    REQUIRE(loaded.difficulty == gus::domain::save::DifficultyLevel::Medio);
    REQUIRE(loaded.difficult_recovery_stage == 0);
    // Campos que existiam em V1 preservados:
    REQUIRE(loaded.current_scene_path == "res://old_city.tscn");
    REQUIRE(loaded.playtime_seconds == 42.0);
    REQUIRE(loaded.party_roster == std::vector<std::string>{"gus"});
    REQUIRE(loaded.inventory.at("credito") == 13);
    REQUIRE(loaded.timestamp_ms == 1000LL);
}

// ---- migracao V2 -> V5: save com character_states sobe os saltos restantes --

TEST_CASE("migrators: save V2 carregado sobe para V5 (campos V3/V4/V5 vazios/default)",
          "[domain][save][migrators]") {
    // Forja um envelope V2 (layout antigo, SEM enemy_knowledge) via helper de teste.
    SaveData v2;
    v2.schema_version = 2;
    v2.timestamp_ms = 7LL;
    v2.current_scene_path = "res://v2.tscn";
    v2.character_states = {{"gus", CharacterSaveState{34, 89, {"glifo_root"}}}};
    v2.relations = {{"caua", 21}};
    const auto bytes_v2 = gus::domain::save::serialize_save_v2(v2);

    const auto loaded = deserialize_save(bytes_v2);

    REQUIRE(loaded.schema_version == 5);              // bumpado V2->V3->V4->V5
    REQUIRE(loaded.enemy_knowledge.empty());           // campo V3, vazio (honesto)
    REQUIRE(loaded.difficulty == gus::domain::save::DifficultyLevel::Medio);
    // Campos que ja existiam em V2 preservados:
    REQUIRE(loaded.character_states.at("gus").xp == 89);
    REQUIRE(loaded.relations.at("caua") == 21);
}

// ---- migracao V4 -> V5: save do ADR-007 sobe so o ultimo salto --------------

TEST_CASE("migrators: save V4 carregado sobe para V5 (difficulty vira Medio)",
          "[domain][save][migrators]") {
    SaveData v4;
    v4.schema_version = 4;
    v4.timestamp_ms = 9LL;
    v4.current_scene_path = "res://v4.tscn";
    v4.character_states = {{"gus", CharacterSaveState{34, 55, {"glifo_root"}}}};
    v4.enemy_knowledge = {{"sentinela_bit", 3}};
    v4.slot_id = 2;
    const auto bytes_v4 = gus::domain::save::serialize_save_v4(v4);

    const auto loaded = deserialize_save(bytes_v4);

    REQUIRE(loaded.schema_version == 5);
    REQUIRE(loaded.difficulty == gus::domain::save::DifficultyLevel::Medio);
    REQUIRE(loaded.difficult_recovery_stage == 0);
    // Campos que ja existiam em V4 preservados:
    REQUIRE(loaded.character_states.at("gus").xp == 55);
    REQUIRE(loaded.slot_id == 2);
}

// ---- save V5 carrega direto, sem migracao (chain no-op) --------------------

TEST_CASE("migrators: save V5 carrega direto (chain no-op)",
          "[domain][save][migrators]") {
    SaveData v5;
    v5.schema_version = 5;
    v5.timestamp_ms = 5LL;
    v5.current_scene_path = "res://now.tscn";
    v5.character_states = {{"gus", CharacterSaveState{34, 0, {"glifo_root"}}}};
    v5.enemy_knowledge = {{"sentinela_bit", 8}, {"daemon_fork", 13}};
    v5.difficulty = gus::domain::save::DifficultyLevel::Dificil;
    v5.difficult_recovery_stage = 2;

    const auto loaded =
        deserialize_save(gus::domain::save::serialize_save(v5));
    REQUIRE(loaded == v5);
}

// ---- forward-only: rejeita versao FUTURA -----------------------------------

TEST_CASE("migrators: save de versao FUTURA rejeitado (forward-only)",
          "[domain][save][migrators]") {
    // Forja um payload com schema_version > atual (ex.: V99), HMAC valido.
    const auto bytes_future = make_v1_payload(99);  // helper: payload versao 99
    REQUIRE_THROWS_AS(deserialize_save(bytes_future), SaveVersionTooNewError);
}

// ---- HMAC primeiro, versao depois: nunca migra bytes adulterados -----------

TEST_CASE("migrators: HMAC e checado ANTES de decidir versao/migrar",
          "[domain][save][migrators]") {
    auto bytes = serialize_save_v1(v1_state_fixture());
    bytes[bytes.size() / 2] ^= 0xFF;  // adultera o payload
    // Integridade falha ANTES de qualquer leitura de versao/migracao.
    REQUIRE_THROWS_AS(deserialize_save(bytes),
                      gus::domain::save::SaveIntegrityError);
}

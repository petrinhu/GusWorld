// save_migrators_test.cpp
//
// Spec executavel (Catch2 v3) dos MIGRATORS forward-only do save (domain/save).
// Portado de engine/foundation/save_system/Migrators/* (C#: arvore JsonNode) com a
// MUDANCA do ADR-006: migrators operam sobre STRUCTS VERSIONADAS (V1 -> V2), NAO
// sobre arvore JSON. Cada versao tem sua FIXTURE.
//
// Schema atual = V2 (gus::domain::kSaveSchemaVersion):
//   V1 = schema inicial, SEM character_states (per-character);
//   V2 = + character_states. MigrateV1ToV2 popula character_states vazio (semantica
//        honesta de um save v1: party usa stats do template, sem delta).
//
// O C++ para em V2 de proposito (kSaveSchemaVersion=2). O C# ja foi a v3
// (EnemyKnowledge); aquele bump NAO e portado neste marco (seria mexer no ancora
// kSaveSchemaVersion, fora do escopo). A chain e extensivel: somar MigrateV2ToV3
// aqui + bumpar o ancora quando o marco chegar.
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
    REQUIRE(current_schema_version() == 2);
}

// ---- migracao V1 -> V2: load de save antigo sobe pela chain ----------------

TEST_CASE("migrators: save V1 carregado sobe para V2 (character_states vazio)",
          "[domain][save][migrators]") {
    const auto v1 = v1_state_fixture();
    const auto bytes_v1 = serialize_save_v1(v1);  // envelope com payload V1

    // O load version-aware roda a chain ANTES de materializar.
    const auto loaded = deserialize_save(bytes_v1);

    REQUIRE(loaded.schema_version == 2);              // bumpado
    REQUIRE(loaded.character_states.empty());          // campo novo, vazio
    // Campos que existiam em V1 preservados:
    REQUIRE(loaded.current_scene_path == "res://old_city.tscn");
    REQUIRE(loaded.playtime_seconds == 42.0);
    REQUIRE(loaded.party_roster == std::vector<std::string>{"gus"});
    REQUIRE(loaded.inventory.at("credito") == 13);
    REQUIRE(loaded.timestamp_ms == 1000LL);
}

// ---- save V2 carrega direto, sem dupla-migracao ----------------------------

TEST_CASE("migrators: save V2 carrega direto (chain no-op)",
          "[domain][save][migrators]") {
    SaveData v2;
    v2.schema_version = 2;
    v2.timestamp_ms = 5LL;
    v2.current_scene_path = "res://now.tscn";
    v2.character_states = {{"gus", CharacterSaveState{34, 0, {"glifo_root"}}}};

    const auto loaded =
        deserialize_save(gus::domain::save::serialize_save(v2));
    REQUIRE(loaded == v2);
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

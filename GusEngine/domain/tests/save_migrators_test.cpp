// save_migrators_test.cpp
//
// Spec executavel (Catch2 v3) dos MIGRATORS forward-only do save (domain/save).
// Portado de engine/foundation/save_system/Migrators/* (C#: arvore JsonNode) com a
// MUDANCA do ADR-006: migrators operam sobre STRUCTS VERSIONADAS (V1 -> V2), NAO
// sobre arvore JSON. Cada versao tem sua FIXTURE.
//
// Schema atual = V6 (gus::domain::kSaveSchemaVersion):
//   V1 = schema inicial, SEM character_states (per-character);
//   V2 = + character_states. MigrateV1ToV2 popula character_states vazio (semantica
//        honesta de um save v1: party usa stats do template, sem delta).
//   V3 = + enemy_knowledge (conhecimento de bestiario por TIPO de inimigo).
//        MigrateV2ToV3 popula enemy_knowledge VAZIO: um save v2 nunca rastreou
//        Knowledge por tipo, logo "nenhum tipo conhecido" (variancia maxima no 1o
//        encontro) e a semantica honesta. NAO se deriva de character_states (keyed
//        por COMPANION, nao por tipo de inimigo: as chaves nao se mapeiam).
//   V4 = + input_remap_backup/controls_hash128/slot_id (ADR-007).
//   V5 = + difficulty/difficult_recovery_stage (MODOS-MORTE Fase 0).
//   V6 = + CardCollectionState (deck ativo/morto) + hand_selection por
//        personagem, EM SUBSTITUICAO ao `deck` legado (DECK-4); + credits (i64)
//        UMA VEZ no nivel do SaveData (carteira UNICA da party, docs/design/
//        mecanicas/economia.md - NAO per-character). MigrateV5ToV6 converte o
//        deck legado (list<str> de card_id) de CADA personagem em instancias
//        sequenciais (instance_id 1..N, LOCAL ao personagem) no deck ATIVO novo;
//        deck morto vazio, hand_selection vazia; credits=0 no SaveData; o campo
//        deck legado e ESVAZIADO apos a conversao.
//
// Forward-only: rejeita versao FUTURA (save mais novo que o schema suportado).
//
// Cross-ref: engine/foundation/save_system/Migrators/MigrateV1ToV2.cs +
//            SaveMigratorChain.cs + SaveMigrators.cs (ref semantica), CONTRACT §7,
//            ADR-006, docs/design/mecanicas/deck-mao-sistema.md (DECK-4).

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
    REQUIRE(current_schema_version() == 6);
}

// ---- migracao V1 -> V6: load de save antigo sobe todos os saltos da chain ---

TEST_CASE("migrators: save V1 carregado sobe ate V6 (campos novos vazios/default)",
          "[domain][save][migrators]") {
    const auto v1 = v1_state_fixture();
    const auto bytes_v1 = serialize_save_v1(v1);  // envelope com payload V1

    // O load version-aware roda a chain inteira (V1->V2->V3->V4->V5->V6) ANTES de
    // materializar.
    const auto loaded = deserialize_save(bytes_v1);

    REQUIRE(loaded.schema_version == 6);                // bumpado ate o topo
    REQUIRE(loaded.character_states.empty());           // campo V2, vazio
    REQUIRE(loaded.enemy_knowledge.empty());            // campo V3, vazio
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

// ---- migracao V2 -> V6: save com character_states sobe os saltos restantes --

TEST_CASE("migrators: save V2 carregado sobe para V6 (deck legado vira card_collection)",
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

    REQUIRE(loaded.schema_version == 6);                // bumpado V2->V3->V4->V5->V6
    REQUIRE(loaded.enemy_knowledge.empty());            // campo V3, vazio (honesto)
    REQUIRE(loaded.difficulty == gus::domain::save::DifficultyLevel::Medio);
    // Campos que ja existiam em V2 preservados:
    const auto& gus_state = loaded.character_states.at("gus");
    REQUIRE(gus_state.xp == 89);
    REQUIRE(loaded.relations.at("caua") == 21);
    // DECK-4: o deck legado ["glifo_root"] virou 1 instancia no deck ATIVO,
    // instance_id sequencial comecando em 1, deck morto vazio, mao default.
    REQUIRE(gus_state.deck.empty());  // esvaziado apos a conversao
    REQUIRE(gus_state.card_collection.active.size() == 1);
    REQUIRE(gus_state.card_collection.active[0].instance_id == 1);
    REQUIRE(gus_state.card_collection.active[0].card_id == "glifo_root");
    REQUIRE(gus_state.card_collection.dead.empty());
    REQUIRE(gus_state.card_collection.next_instance_id == 2);
    REQUIRE(gus_state.hand_selection.empty());
    // credits: carteira UNICA da party, no nivel do SaveData (nao per-character).
    REQUIRE(loaded.credits == 0);
}

// ---- migracao V4 -> V6: save do ADR-007 sobe os 2 ultimos saltos -----------

TEST_CASE("migrators: save V4 carregado sobe para V6 (difficulty vira Medio + "
          "deck legado vira card_collection)",
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

    REQUIRE(loaded.schema_version == 6);
    REQUIRE(loaded.difficulty == gus::domain::save::DifficultyLevel::Medio);
    REQUIRE(loaded.difficult_recovery_stage == 0);
    // Campos que ja existiam em V4 preservados:
    const auto& gus_state = loaded.character_states.at("gus");
    REQUIRE(gus_state.xp == 55);
    REQUIRE(loaded.slot_id == 2);
    // DECK-4: mesma conversao do teste V2->V6 acima.
    REQUIRE(gus_state.deck.empty());
    REQUIRE(gus_state.card_collection.active.size() == 1);
    REQUIRE(gus_state.card_collection.active[0].card_id == "glifo_root");
    REQUIRE(gus_state.card_collection.next_instance_id == 2);
    REQUIRE(loaded.credits == 0);
}

// ---- migracao V5 -> V6: DECK-4, deck legado com VARIAS cartas + varios personagens

TEST_CASE("migrators: save V5 carregado sobe para V6 (DECK-4: deck legado "
          "POPULADO vira card_collection, instance_id sequencial por personagem)",
          "[domain][save][migrators]") {
    SaveData v5;
    v5.schema_version = 5;
    v5.timestamp_ms = 11LL;
    v5.current_scene_path = "res://v5.tscn";
    v5.character_states = {
        {"gus", CharacterSaveState{34, 120, {"pulso_eletrico", "scan_basico", "glifo_root"}}},
        {"caua", CharacterSaveState{40, 89, {"stream_raio"}}},
    };
    v5.difficulty = gus::domain::save::DifficultyLevel::Dificil;
    v5.difficult_recovery_stage = 1;
    const auto bytes_v5 = gus::domain::save::serialize_save_v5(v5);

    const auto loaded = deserialize_save(bytes_v5);

    REQUIRE(loaded.schema_version == 6);
    // difficulty/difficult_recovery_stage ja existiam em V5, preservados intactos.
    REQUIRE(loaded.difficulty == gus::domain::save::DifficultyLevel::Dificil);
    REQUIRE(loaded.difficult_recovery_stage == 1);

    const auto& gus_state = loaded.character_states.at("gus");
    REQUIRE(gus_state.deck.empty());
    REQUIRE(gus_state.card_collection.active.size() == 3);
    REQUIRE(gus_state.card_collection.active[0].instance_id == 1);
    REQUIRE(gus_state.card_collection.active[0].card_id == "pulso_eletrico");
    REQUIRE(gus_state.card_collection.active[1].instance_id == 2);
    REQUIRE(gus_state.card_collection.active[1].card_id == "scan_basico");
    REQUIRE(gus_state.card_collection.active[2].instance_id == 3);
    REQUIRE(gus_state.card_collection.active[2].card_id == "glifo_root");
    REQUIRE(gus_state.card_collection.dead.empty());
    REQUIRE(gus_state.card_collection.next_instance_id == 4);
    REQUIRE(gus_state.hand_selection.empty());
    REQUIRE(loaded.credits == 0);

    // caua: instance_id reinicia em 1 (LOCAL ao personagem, nao compartilha
    // contador entre companions).
    const auto& caua_state = loaded.character_states.at("caua");
    REQUIRE(caua_state.card_collection.active.size() == 1);
    REQUIRE(caua_state.card_collection.active[0].instance_id == 1);
    REQUIRE(caua_state.card_collection.active[0].card_id == "stream_raio");
    REQUIRE(caua_state.card_collection.next_instance_id == 2);
}

TEST_CASE("migrators: save V5 com deck legado VAZIO sobe para V6 sem instancias",
          "[domain][save][migrators]") {
    SaveData v5;
    v5.schema_version = 5;
    v5.timestamp_ms = 12LL;
    v5.current_scene_path = "res://v5_empty.tscn";
    v5.character_states = {{"gus", CharacterSaveState{34, 0, {}}}};
    const auto bytes_v5 = gus::domain::save::serialize_save_v5(v5);

    const auto loaded = deserialize_save(bytes_v5);

    REQUIRE(loaded.schema_version == 6);
    const auto& gus_state = loaded.character_states.at("gus");
    REQUIRE(gus_state.card_collection.active.empty());
    REQUIRE(gus_state.card_collection.dead.empty());
    // next_instance_id honesto (proximo id emitido seria o 1o - deck vazio nao
    // avancou o contador): mesma semantica de um CardCollection novo.
    REQUIRE(gus_state.card_collection.next_instance_id == 1);
    REQUIRE(gus_state.hand_selection.empty());
    REQUIRE(loaded.credits == 0);
}

// ---- save V6 carrega direto, sem migracao (chain no-op) --------------------

TEST_CASE("migrators: save V6 carrega direto (chain no-op)",
          "[domain][save][migrators]") {
    SaveData v6;
    v6.schema_version = 6;
    v6.timestamp_ms = 5LL;
    v6.current_scene_path = "res://now.tscn";

    CharacterSaveState gus_state;
    gus_state.current_hp = 34;
    gus_state.xp = 0;
    gus_state.card_collection.active = {{1, "glifo_root"}, {2, "scan_basico"}};
    gus_state.card_collection.dead = {{3, "glifo_obsoleto"}};
    gus_state.card_collection.next_instance_id = 4;
    gus_state.hand_selection = {1, 2};
    v6.character_states = {{"gus", gus_state}};
    v6.credits = 21;  // carteira UNICA da party, nivel do SaveData

    v6.enemy_knowledge = {{"sentinela_bit", 8}, {"daemon_fork", 13}};
    v6.difficulty = gus::domain::save::DifficultyLevel::Dificil;
    v6.difficult_recovery_stage = 2;

    const auto loaded =
        deserialize_save(gus::domain::save::serialize_save(v6));
    REQUIRE(loaded == v6);
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

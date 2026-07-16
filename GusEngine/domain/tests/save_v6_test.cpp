// save_v6_test.cpp
//
// Spec executavel (Catch2 v3) do SAVE V6 (DECK-4, docs/design/mecanicas/
// deck-mao-sistema.md). Estende o ORACULO de equivalencia semantica do save
// (roundtrip + migracao + validate fail-fast + forward-only) aos campos NOVOS:
//
//   - CharacterSaveState::card_collection (CardCollectionState): fotografia do
//     CardCollection (DECK-1) de cada personagem - active[]/dead[]
//     (CardInstance{instance_id, card_id}) + next_instance_id. SUBSTITUI o `deck`
//     legado (list<str>, V2..V5).
//   - CharacterSaveState::hand_selection (list<u64>): a MAO (HandLoadout, DECK-2)
//     persistida - instance_ids que devem estar presentes em
//     card_collection.active.
//   - SaveData::credits (int64_t): a carteira UNICA da PARTY (docs/design/
//     mecanicas/economia.md - economia single-currency, sources/sinks todos a
//     nivel de party). NAO mora em CharacterSaveState - correcao do lider sobre a
//     1a versao desta onda, que erroneamente colocou credits per-character.
//
// Migrator V5->V6 (save_migrators.cpp): converte o deck legado (list<str> de
// card_id) de CADA personagem em instancias sequenciais NOVAS no deck ativo
// (instance_id 1..N, determinístico, LOCAL ao personagem), deck morto vazio,
// hand_selection vazia por personagem; credits=0 UMA VEZ no SaveData (nao
// per-character). O deck legado e ESVAZIADO apos a conversao.
//
// POCO puro, ZERO Qt, headless. Carimbo timestamp_ms injetado (sem relogio).
//
// Cross-ref: gus/domain/deck/card_collection.hpp (o agregado real - este arquivo
//            so persiste a FOTOGRAFIA), gus/domain/save/save_data.hpp,
//            save_migrators.hpp, save_serializer.hpp, save_migrators_test.cpp
//            (cobre tambem V2/V4/V5 -> V6 na chain completa).

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include "gus/domain/deck/deck_records.hpp"
#include "gus/domain/domain_info.hpp"
#include "gus/domain/save/save_data.hpp"
#include "gus/domain/save/save_migrators.hpp"
#include "gus/domain/save/save_serializer.hpp"

using gus::domain::deck::CardInstance;
using gus::domain::save::CardCollectionState;
using gus::domain::save::CharacterSaveState;
using gus::domain::save::deserialize_save;
using gus::domain::save::load_save;
using gus::domain::save::LoadResult;
using gus::domain::save::SaveData;
using gus::domain::save::serialize_save;
using gus::domain::save::serialize_save_v5;

namespace {

// Fixture V6 rica: 2 personagens, card_collection com ativo+morto+hand_selection
// (per-character) + credits (carteira UNICA da party, no SaveData).
SaveData rich_v6() {
    SaveData s;
    s.schema_version = gus::domain::kSaveSchemaVersion;  // 6
    s.timestamp_ms = 1721140000000LL;
    s.playtime_seconds = 512.0;
    s.current_scene_path = "res://world/gusworld_city.tscn";
    s.party_roster = {"gus", "caua"};
    s.party_active = {"gus", "caua"};

    CharacterSaveState gus_state;
    gus_state.current_hp = 34;
    gus_state.xp = 144;
    gus_state.card_collection.active = {
        CardInstance{1, "pulso_eletrico"},
        CardInstance{2, "scan_basico"},
        CardInstance{4, "glifo_gauss"},
    };
    gus_state.card_collection.dead = {CardInstance{3, "glifo_obsoleto"}};
    gus_state.card_collection.next_instance_id = 5;
    gus_state.hand_selection = {1, 4};

    CharacterSaveState caua_state;
    caua_state.current_hp = 40;
    caua_state.xp = 55;
    caua_state.card_collection.active = {CardInstance{1, "stream_raio"}};
    caua_state.card_collection.next_instance_id = 2;

    s.character_states = {{"gus", gus_state}, {"caua", caua_state}};
    s.credits = 89;  // carteira UNICA da party (SaveData, nao per-character)
    return s;
}

}  // namespace

// ---- ancora: kSaveSchemaVersion aponta pro topo V6 (DECK-4) ----------------

TEST_CASE("save_v6: kSaveSchemaVersion aponta pro topo da chain (ancora)",
          "[domain][save][v6]") {
    REQUIRE(gus::domain::kSaveSchemaVersion == 6);
    REQUIRE(gus::domain::save::current_schema_version() ==
            gus::domain::kSaveSchemaVersion);
}

// ---- (a) roundtrip: card_collection + hand_selection + credits (oraculo) ---

TEST_CASE("save_v6: roundtrip preserva card_collection + hand_selection + "
          "credits (carteira unica da party)",
          "[domain][save][v6]") {
    const auto original = rich_v6();
    const auto restored = deserialize_save(serialize_save(original));
    REQUIRE(restored == original);

    const auto& gus_state = restored.character_states.at("gus");
    REQUIRE(gus_state.card_collection.active.size() == 3);
    REQUIRE(gus_state.card_collection.dead.size() == 1);
    REQUIRE(gus_state.card_collection.dead[0].card_id == "glifo_obsoleto");
    REQUIRE(gus_state.card_collection.next_instance_id == 5);
    REQUIRE(gus_state.hand_selection == std::vector<std::uint64_t>{1, 4});
    // deck legado nunca populado num save V6 nativo.
    REQUIRE(gus_state.deck.empty());
    // credits: carteira UNICA da party, no SaveData (nao per-character).
    REQUIRE(restored.credits == 89);
}

TEST_CASE("save_v6: creditar a carteira unica da party roundtrippa (wallet "
          "compartilhada entre TODOS os personagens)",
          "[domain][save][v6]") {
    auto s = rich_v6();
    // sell()/upload()/acquire() (DECK-3) mutam UM std::int64_t& compartilhado -
    // aqui simulamos o resultado: a party recebeu credito (upload de uma carta do
    // "gus", mas o saldo e da PARTY, nao dele).
    s.credits += 5;  // 89 -> 94, upload de kUploadCreditMax (deck_constants.hpp)
    REQUIRE(s.credits == 94);

    const auto restored = deserialize_save(serialize_save(s));
    REQUIRE(restored.credits == 94);
    // o credito e UM SO valor pro save inteiro - nao existe "credits do gus" nem
    // "credits do caua" (CharacterSaveState nao tem mais esse campo).
    REQUIRE(restored == s);
}

TEST_CASE("save_v6: roundtrip com card_collection/hand_selection vazios (personagem novo)",
          "[domain][save][v6]") {
    SaveData s;
    s.timestamp_ms = 1LL;
    s.current_scene_path = "res://boot.tscn";
    s.character_states = {{"gus", CharacterSaveState{}}};
    const auto restored = deserialize_save(serialize_save(s));
    REQUIRE(restored == s);
    REQUIRE(restored.character_states.at("gus").card_collection.active.empty());
    REQUIRE(restored.character_states.at("gus").card_collection.next_instance_id == 1);
    REQUIRE(restored.credits == 0);  // save novo: carteira da party comeca zerada
}

// ---- determinismo por campo: cada campo novo muda o selo -------------------

TEST_CASE("save_v6: card_collection diferente muda o selo (determinismo por campo)",
          "[domain][save][v6]") {
    auto a = rich_v6();
    auto b = rich_v6();
    b.character_states.at("gus").card_collection.active[0].card_id = "outra_carta";
    REQUIRE(serialize_save(a) != serialize_save(b));
}

TEST_CASE("save_v6: credits (carteira da party) diferente muda o selo",
          "[domain][save][v6]") {
    auto a = rich_v6();
    auto b = rich_v6();
    b.credits += 1;
    REQUIRE(serialize_save(a) != serialize_save(b));
}

TEST_CASE("save_v6: hand_selection diferente muda o selo", "[domain][save][v6]") {
    auto a = rich_v6();
    auto b = rich_v6();
    b.character_states.at("gus").hand_selection = {2};
    REQUIRE(serialize_save(a) != serialize_save(b));
}

// ---- migrator V5 -> V6: deck legado POPULADO vira card_collection ----------
//
// Cobertura adicional aqui (save_migrators_test.cpp ja cobre a chain inteira);
// este caso exercita especificamente o caminho NAO-lancante (load_save, T1.1).

TEST_CASE("save_v6: load_save de um save V5 com deck legado sobe para V6 (Ok)",
          "[domain][save][v6][migrators]") {
    SaveData v5;
    v5.schema_version = 5;
    v5.timestamp_ms = 21LL;
    v5.current_scene_path = "res://v5_load.tscn";
    v5.character_states = {
        {"gus", CharacterSaveState{34, 8, {"pulso_eletrico", "scan_basico"}}}};
    v5.slot_id = 1;
    const auto bytes_v5 = serialize_save_v5(v5);

    const auto outcome = load_save(bytes_v5, /*expected_slot=*/1);
    REQUIRE(outcome.result == LoadResult::Ok);
    REQUIRE(outcome.data.schema_version == 6);
    const auto& gus_state = outcome.data.character_states.at("gus");
    REQUIRE(gus_state.deck.empty());
    REQUIRE(gus_state.card_collection.active.size() == 2);
    REQUIRE(gus_state.card_collection.active[0].instance_id == 1);
    REQUIRE(gus_state.card_collection.active[1].instance_id == 2);
    REQUIRE(gus_state.card_collection.next_instance_id == 3);
    REQUIRE(gus_state.hand_selection.empty());
    // credits: carteira UNICA da party, nasce zerada no SaveData (nao havia
    // carteira registrada antes desta onda).
    REQUIRE(outcome.data.credits == 0);
}

// ---- validate() fail-fast (CardCollectionState + hand_selection + credits) -

TEST_CASE("save_v6: instance_id == 0 no ativo e rejeitado (fail-fast)",
          "[domain][save][v6][validate]") {
    auto s = rich_v6();
    s.character_states.at("gus").card_collection.active.push_back(
        CardInstance{0, "carta_invalida"});
    REQUIRE_THROWS_AS(serialize_save(s), std::invalid_argument);
}

TEST_CASE("save_v6: instance_id == 0 no morto e rejeitado (fail-fast)",
          "[domain][save][v6][validate]") {
    auto s = rich_v6();
    s.character_states.at("gus").card_collection.dead.push_back(
        CardInstance{0, "carta_invalida"});
    REQUIRE_THROWS_AS(serialize_save(s), std::invalid_argument);
}

TEST_CASE("save_v6: card_id vazio e rejeitado (fail-fast)",
          "[domain][save][v6][validate]") {
    auto s = rich_v6();
    s.character_states.at("gus").card_collection.active.push_back(
        CardInstance{99, ""});
    REQUIRE_THROWS_AS(serialize_save(s), std::invalid_argument);
}

TEST_CASE("save_v6: instance_id duplicado entre ativo e morto e rejeitado "
          "(inv.1 - EXATAMENTE um container)",
          "[domain][save][v6][validate]") {
    auto s = rich_v6();
    // instance_id 1 ja esta no ativo do gus; duplica no morto.
    s.character_states.at("gus").card_collection.dead.push_back(
        CardInstance{1, "duplicata"});
    REQUIRE_THROWS_AS(serialize_save(s), std::invalid_argument);
}

TEST_CASE("save_v6: instance_id duplicado DENTRO do ativo e rejeitado",
          "[domain][save][v6][validate]") {
    auto s = rich_v6();
    s.character_states.at("gus").card_collection.active.push_back(
        CardInstance{1, "duplicata_ativo"});
    REQUIRE_THROWS_AS(serialize_save(s), std::invalid_argument);
}

TEST_CASE("save_v6: next_instance_id <= maior instance_id existente e "
          "rejeitado (contador nunca reusa)",
          "[domain][save][v6][validate]") {
    auto s = rich_v6();
    // gus tem instance_id ate 4 (active) - next_instance_id=4 colide.
    s.character_states.at("gus").card_collection.next_instance_id = 4;
    REQUIRE_THROWS_AS(serialize_save(s), std::invalid_argument);
}

TEST_CASE("save_v6: next_instance_id == 0 e rejeitado", "[domain][save][v6][validate]") {
    auto s = rich_v6();
    s.character_states.at("gus").card_collection = CardCollectionState{};
    s.character_states.at("gus").card_collection.next_instance_id = 0;
    s.character_states.at("gus").hand_selection.clear();
    REQUIRE_THROWS_AS(serialize_save(s), std::invalid_argument);
}

TEST_CASE("save_v6: credits (carteira da party) negativo e rejeitado (fail-fast)",
          "[domain][save][v6][validate]") {
    auto s = rich_v6();
    s.credits = -1;
    REQUIRE_THROWS_AS(serialize_save(s), std::invalid_argument);
}

TEST_CASE("save_v6: hand_selection referenciando instance_id fora do ativo e "
          "rejeitado (inv.6 - mao so puxa do ativo)",
          "[domain][save][v6][validate]") {
    auto s = rich_v6();
    s.character_states.at("gus").hand_selection.push_back(999);
    REQUIRE_THROWS_AS(serialize_save(s), std::invalid_argument);
}

TEST_CASE("save_v6: hand_selection referenciando instance_id do deck MORTO "
          "(nao do ativo) e rejeitado",
          "[domain][save][v6][validate]") {
    auto s = rich_v6();
    s.character_states.at("gus").hand_selection.push_back(3);  // 3 esta no morto
    REQUIRE_THROWS_AS(serialize_save(s), std::invalid_argument);
}

TEST_CASE("save_v6: hand_selection com instance_id duplicado e rejeitado",
          "[domain][save][v6][validate]") {
    auto s = rich_v6();
    s.character_states.at("gus").hand_selection = {1, 1};
    REQUIRE_THROWS_AS(serialize_save(s), std::invalid_argument);
}

// ---- validate no LOAD: payload forjado (tag AEAD valida, schema-divergente) -

TEST_CASE("save_v6: load rejeita credits (carteira da party) negativo forjado "
          "(defesa em profundidade)",
          "[domain][save][v6][validate]") {
    auto s = rich_v6();
    s.credits = -5;
    const auto bytes = gus::domain::save::serialize_save_unchecked(s);
    REQUIRE_THROWS_AS(deserialize_save(bytes), std::invalid_argument);
}

TEST_CASE("save_v6: load rejeita hand_selection orfa forjada (defesa em profundidade)",
          "[domain][save][v6][validate]") {
    auto s = rich_v6();
    s.character_states.at("gus").hand_selection.push_back(12345);
    const auto bytes = gus::domain::save::serialize_save_unchecked(s);
    REQUIRE_THROWS_AS(deserialize_save(bytes), std::invalid_argument);
}

// ---- forward-only: rejeita versao FUTURA (V7, alem do topo atual V6) ------

TEST_CASE("save_v6: save de versao FUTURA (V7) rejeitado (forward-only)",
          "[domain][save][v6]") {
    const auto bytes_future = gus::domain::save::make_v1_payload(7);
    const auto outcome = load_save(bytes_future, /*expected_slot=*/0);
    REQUIRE(outcome.result == LoadResult::VersionTooNew);
    REQUIRE_THROWS_AS(deserialize_save(bytes_future),
                       gus::domain::save::SaveVersionTooNewError);
}

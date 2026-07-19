// save_v7_test.cpp
//
// Spec executavel (Catch2 v3) do SAVE V7 (CARDS-HW-1, docs/design/mecanicas/
// cartas-spec-dados.md secao 10). Estende o ORACULO de equivalencia semantica do
// save (roundtrip + migracao + validate fail-fast + forward-only) ao campo NOVO:
//
//   - CardInstance::physical (CardPhysicalState): estado fisico MUTAVEL de cada
//     copia de carta (origin/bateria/integridade-virus/queima), gravado dentro de
//     CADA CardInstance de card_collection.active/dead (deck_records.hpp,
//     card_hardware.hpp).
//
// Migrator V6->V7 (save_migrators.cpp): popula physical = CardPhysicalState{}
// (ROM original, bateria cheia, sem infeccao - "zero e seguro") em CADA
// CardInstance ja existente (ativo e morto), sem RNG/relogio (funcao PURA,
// CONTRACT.md secao 7). Nenhum outro campo de SaveData/CharacterSaveState muda
// neste bump - so o SHAPE de CardInstance ganhou physical.
//
// POCO puro, ZERO Qt, headless. Carimbo timestamp_ms injetado (sem relogio).
//
// Cross-ref: gus/domain/deck/card_hardware.hpp (o modelo fisico -
//            card_hardware_test.cpp cobre validate()/funcoes puras
//            isoladamente), gus/domain/deck/deck_records.hpp
//            (CardInstance::physical), gus/domain/save/save_data.hpp,
//            save_migrators.hpp, save_serializer.hpp, save_v6_test.cpp
//            (oraculo irmao do bump anterior), save_migrators_test.cpp (cobre
//            tambem a chain completa V1..V5 -> V7).

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include "gus/domain/deck/card_hardware.hpp"
#include "gus/domain/deck/deck_records.hpp"
#include "gus/domain/domain_info.hpp"
#include "gus/domain/save/save_data.hpp"
#include "gus/domain/save/save_migrators.hpp"
#include "gus/domain/save/save_serializer.hpp"

using gus::domain::deck::CardInstance;
using gus::domain::deck::CardOrigin;
using gus::domain::deck::CardPhysicalState;
using gus::domain::deck::VirusKind;
using gus::domain::save::CardCollectionState;
using gus::domain::save::CharacterSaveState;
using gus::domain::save::deserialize_save;
using gus::domain::save::load_save;
using gus::domain::save::LoadResult;
using gus::domain::save::SaveData;
using gus::domain::save::serialize_save;
using gus::domain::save::serialize_save_unchecked;
using gus::domain::save::serialize_save_v6;

namespace {

// Fixture V7 rica: instancias com physical NAO-default (pirata infectada
// diagnosticada, homebrew queimada) misturadas com instancias default (a imensa
// maioria - carta legitima nova).
SaveData rich_v7() {
    SaveData s;
    s.schema_version = gus::domain::kSaveSchemaVersion;  // 7
    s.timestamp_ms = 1721150000000LL;
    s.playtime_seconds = 610.0;
    s.current_scene_path = "res://world/gusworld_city.tscn";
    s.party_roster = {"gus", "caua"};
    s.party_active = {"gus", "caua"};

    CardPhysicalState pirate_infected;
    pirate_infected.origin = CardOrigin::PirateClone;
    pirate_infected.battery_recharge_cycles = 3;
    pirate_infected.battery_charge_deficit = 21;
    pirate_infected.is_infected = true;
    pirate_infected.virus_kind = VirusKind::Worm;
    pirate_infected.is_diagnosed = true;

    CardPhysicalState burned;
    burned.origin = CardOrigin::HomebrewEprom;
    burned.is_burned_out = true;

    CharacterSaveState gus_state;
    gus_state.current_hp = 34;
    gus_state.xp = 144;
    gus_state.card_collection.active = {
        CardInstance{1, "pulso_eletrico", CardPhysicalState{}},
        CardInstance{2, "scan_basico", pirate_infected},
        CardInstance{4, "glifo_gauss", CardPhysicalState{}},
    };
    gus_state.card_collection.dead = {CardInstance{3, "glifo_obsoleto", burned}};
    gus_state.card_collection.next_instance_id = 5;
    gus_state.hand_selection = {1, 4};

    CharacterSaveState caua_state;
    caua_state.current_hp = 40;
    caua_state.xp = 55;
    caua_state.card_collection.active = {CardInstance{1, "stream_raio", CardPhysicalState{}}};
    caua_state.card_collection.next_instance_id = 2;

    s.character_states = {{"gus", gus_state}, {"caua", caua_state}};
    s.credits = 89;
    return s;
}

// Fixture V6 "legada" (ANTES do bump - sem physical), mesmo shape ilustrativo de
// rich_v6() em save_v6_test.cpp, para exercitar o migrator V6->V7 isoladamente
// deste arquivo (nao depende de save_v6_test.cpp).
SaveData legacy_v6_fixture() {
    SaveData s;
    s.schema_version = 6;
    s.timestamp_ms = 1721140000000LL;
    s.current_scene_path = "res://world/gusworld_city.tscn";
    s.party_roster = {"gus"};
    s.party_active = {"gus"};

    CharacterSaveState gus_state;
    gus_state.current_hp = 34;
    gus_state.xp = 89;
    gus_state.card_collection.active = {
        CardInstance{1, "pulso_eletrico"},
        CardInstance{2, "scan_basico"},
    };
    gus_state.card_collection.dead = {CardInstance{3, "glifo_obsoleto"}};
    gus_state.card_collection.next_instance_id = 4;
    gus_state.hand_selection = {1};

    s.character_states = {{"gus", gus_state}};
    s.credits = 21;
    return s;
}

}  // namespace

// ---- ancora: kSaveSchemaVersion aponta pro topo V7 (CARDS-HW-1) ------------

TEST_CASE("save_v7: kSaveSchemaVersion aponta pro topo da chain (ancora)",
          "[domain][save][v7]") {
    REQUIRE(gus::domain::kSaveSchemaVersion == 7);
    REQUIRE(gus::domain::save::current_schema_version() ==
            gus::domain::kSaveSchemaVersion);
}

// ---- (a) roundtrip: physical (ativo + morto) e o oraculo --------------------

TEST_CASE("save_v7: roundtrip preserva CardPhysicalState em cada CardInstance "
          "(ativo e morto)",
          "[domain][save][v7]") {
    const auto original = rich_v7();
    const auto restored = deserialize_save(serialize_save(original));
    REQUIRE(restored == original);

    const auto& gus_state = restored.character_states.at("gus");
    REQUIRE(gus_state.card_collection.active[0].physical == CardPhysicalState{});
    REQUIRE(gus_state.card_collection.active[1].physical.is_infected);
    REQUIRE(gus_state.card_collection.active[1].physical.is_diagnosed);
    REQUIRE(gus_state.card_collection.active[1].physical.virus_kind == VirusKind::Worm);
    REQUIRE(gus_state.card_collection.active[1].physical.battery_recharge_cycles == 3);
    REQUIRE(gus_state.card_collection.active[1].physical.battery_charge_deficit == 21);
    REQUIRE(gus_state.card_collection.dead[0].physical.is_burned_out);
    REQUIRE(gus_state.card_collection.dead[0].physical.origin == CardOrigin::HomebrewEprom);
}

TEST_CASE("save_v7: roundtrip com physical 100% default (personagem novo, sem "
          "nenhuma carta pirata/infectada) preserva o shape",
          "[domain][save][v7]") {
    SaveData s;
    s.timestamp_ms = 1LL;
    s.current_scene_path = "res://boot.tscn";
    s.character_states = {{"gus", CharacterSaveState{}}};
    const auto restored = deserialize_save(serialize_save(s));
    REQUIRE(restored == s);
}

// ---- determinismo: physical diferente muda o selo ---------------------------

TEST_CASE("save_v7: physical diferente muda o selo (determinismo por campo)",
          "[domain][save][v7]") {
    auto a = rich_v7();
    auto b = rich_v7();
    b.character_states.at("gus").card_collection.active[0].physical.is_infected = true;
    b.character_states.at("gus").card_collection.active[0].physical.virus_kind =
        VirusKind::LogicBomb;
    REQUIRE(serialize_save(a) != serialize_save(b));
}

TEST_CASE("save_v7: battery_recharge_cycles diferente muda o selo",
          "[domain][save][v7]") {
    auto a = rich_v7();
    auto b = rich_v7();
    b.character_states.at("gus").card_collection.dead[0].physical.battery_recharge_cycles = 9;
    REQUIRE(serialize_save(a) != serialize_save(b));
}

// ---- migrator V6 -> V7: fixture legada nao ganha nem perde nada alem de physical --

TEST_CASE("save_v7: fixture serialize_save_v6 (SEM physical) migra para V7 com "
          "physical DEFAULT em toda CardInstance (ativo e morto) - zero e seguro",
          "[domain][save][v7][migrators]") {
    const auto v6 = legacy_v6_fixture();
    const auto bytes_v6 = serialize_save_v6(v6);

    const auto loaded = deserialize_save(bytes_v6);

    REQUIRE(loaded.schema_version == 7);
    const auto& gus_state = loaded.character_states.at("gus");
    // Nada além de physical mudou: mesmo card_id/instance_id/next_instance_id/
    // hand_selection/credits/current_scene_path da fixture legada.
    REQUIRE(gus_state.current_hp == 34);
    REQUIRE(gus_state.xp == 89);
    REQUIRE(gus_state.card_collection.active.size() == 2);
    REQUIRE(gus_state.card_collection.active[0].instance_id == 1);
    REQUIRE(gus_state.card_collection.active[0].card_id == "pulso_eletrico");
    REQUIRE(gus_state.card_collection.active[1].instance_id == 2);
    REQUIRE(gus_state.card_collection.active[1].card_id == "scan_basico");
    REQUIRE(gus_state.card_collection.dead.size() == 1);
    REQUIRE(gus_state.card_collection.dead[0].card_id == "glifo_obsoleto");
    REQUIRE(gus_state.card_collection.next_instance_id == 4);
    REQUIRE(gus_state.hand_selection == std::vector<std::uint64_t>{1});
    REQUIRE(loaded.credits == 21);
    REQUIRE(loaded.current_scene_path == "res://world/gusworld_city.tscn");

    // physical: DEFAULT em TODA instancia (ativo E morto) - migrator V6->V7 puro.
    for (const auto& inst : gus_state.card_collection.active)
        REQUIRE(inst.physical == CardPhysicalState{});
    for (const auto& inst : gus_state.card_collection.dead)
        REQUIRE(inst.physical == CardPhysicalState{});
}

TEST_CASE("save_v7: load_save de uma fixture V6 sobe para V7 (Ok, T1.1)",
          "[domain][save][v7][migrators]") {
    auto v6 = legacy_v6_fixture();
    v6.slot_id = 2;
    const auto bytes_v6 = serialize_save_v6(v6);

    const auto outcome = load_save(bytes_v6, /*expected_slot=*/2);
    REQUIRE(outcome.result == LoadResult::Ok);
    REQUIRE(outcome.data.schema_version == 7);
    const auto& gus_state = outcome.data.character_states.at("gus");
    REQUIRE(gus_state.card_collection.active[0].physical == CardPhysicalState{});
}

TEST_CASE("save_v7: fixture serialize_save_v6 com card_collection vazio migra "
          "sem instancias (nenhum loop de physical a rodar)",
          "[domain][save][v7][migrators]") {
    SaveData v6;
    v6.schema_version = 6;
    v6.timestamp_ms = 3LL;
    v6.current_scene_path = "res://v6_empty.tscn";
    v6.character_states = {{"gus", CharacterSaveState{}}};
    const auto bytes_v6 = serialize_save_v6(v6);

    const auto loaded = deserialize_save(bytes_v6);
    REQUIRE(loaded.schema_version == 7);
    REQUIRE(loaded.character_states.at("gus").card_collection.active.empty());
    REQUIRE(loaded.character_states.at("gus").card_collection.dead.empty());
}

// ---- validate() fail-fast: CardPhysicalState invalido propagado -------------

TEST_CASE("save_v7: CardPhysicalState com virus_kind sem is_infected e rejeitado "
          "na escrita (fail-fast, propagado via CardCollectionState::validate())",
          "[domain][save][v7][validate]") {
    auto s = rich_v7();
    s.character_states.at("gus").card_collection.active[0].physical.virus_kind =
        VirusKind::Backdoor;
    // is_infected fica false (default) - viola secao 6 inv.1.
    REQUIRE_THROWS_AS(serialize_save(s), std::invalid_argument);
}

TEST_CASE("save_v7: CardPhysicalState com is_diagnosed sem is_infected no deck "
          "MORTO tambem e rejeitado (validate() cobre ativo E morto)",
          "[domain][save][v7][validate]") {
    auto s = rich_v7();
    s.character_states.at("gus").card_collection.dead[0].physical.is_diagnosed = true;
    s.character_states.at("gus").card_collection.dead[0].physical.is_infected = false;
    REQUIRE_THROWS_AS(serialize_save(s), std::invalid_argument);
}

// ---- defesa em profundidade: ordinal fora de faixa em payload forjado -------

TEST_CASE("save_v7: load rejeita CardPhysicalState::origin ordinal fora do "
          "dominio forjado (defesa em profundidade)",
          "[domain][save][v7][validate]") {
    auto s = rich_v7();
    s.character_states.at("gus").card_collection.active[0].physical.origin =
        static_cast<CardOrigin>(99);
    const auto bytes = serialize_save_unchecked(s);
    REQUIRE_THROWS_AS(deserialize_save(bytes), std::invalid_argument);
}

TEST_CASE("save_v7: load rejeita CardPhysicalState::virus_kind ordinal fora do "
          "dominio forjado (defesa em profundidade)",
          "[domain][save][v7][validate]") {
    auto s = rich_v7();
    auto& phys = s.character_states.at("gus").card_collection.active[0].physical;
    phys.is_infected = true;  // isola a causa: so o ordinal de virus_kind e invalido
    phys.virus_kind = static_cast<VirusKind>(255);
    const auto bytes = serialize_save_unchecked(s);
    REQUIRE_THROWS_AS(deserialize_save(bytes), std::invalid_argument);
}

// ---- forward-only: rejeita versao FUTURA (V8, alem do topo atual V7) -------

TEST_CASE("save_v7: save de versao FUTURA (V8) rejeitado (forward-only)",
          "[domain][save][v7]") {
    const auto bytes_future = gus::domain::save::make_v1_payload(8);
    const auto outcome = load_save(bytes_future, /*expected_slot=*/0);
    REQUIRE(outcome.result == LoadResult::VersionTooNew);
    REQUIRE_THROWS_AS(deserialize_save(bytes_future),
                       gus::domain::save::SaveVersionTooNewError);
}

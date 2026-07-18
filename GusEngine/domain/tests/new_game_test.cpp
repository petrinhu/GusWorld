// new_game_test.cpp
//
// Spec executavel (Catch2 v3) de gus::domain::save::fresh_new_game_save_data
// (MENU-INICIAL, ACHADO 1). Prova que o "estado fresco" de um jogo novo e
// EQUIVALENTE a SaveData default-construido (o mesmo estado implicito que
// Maestro::save_{} ja tinha no boot), com a dificuldade escolhida aplicada - e
// SO ela, nenhum outro campo diverge do default.

#include <catch2/catch_test_macros.hpp>

#include "gus/domain/domain_info.hpp"
#include "gus/domain/save/new_game.hpp"

using gus::domain::save::DifficultyLevel;
using gus::domain::save::fresh_new_game_save_data;
using gus::domain::save::SaveData;

TEST_CASE("fresh_new_game_save_data: equivale ao default-construction + dificuldade",
          "[domain][save][new_game]") {
    const auto data = fresh_new_game_save_data(DifficultyLevel::Dificil);

    SaveData expected{};
    expected.difficulty = DifficultyLevel::Dificil;

    // operator== (default, campo a campo) - qualquer divergencia futura acidental
    // entre o que o boot considera "fresco" e o que esta funcao devolve quebra
    // este teste na hora.
    REQUIRE(data == expected);
}

TEST_CASE("fresh_new_game_save_data: schema atual + estado de partida zerado",
          "[domain][save][new_game]") {
    const auto data = fresh_new_game_save_data(DifficultyLevel::Medio);

    REQUIRE(data.schema_version == gus::domain::kSaveSchemaVersion);
    REQUIRE(data.timestamp_ms == 0);
    REQUIRE(data.playtime_seconds == 0.0);
    REQUIRE(data.current_scene_path.empty());
    REQUIRE(data.player_position == gus::domain::save::Vec3{});
    REQUIRE(data.player_rotation == gus::domain::save::Vec3{});
    REQUIRE(data.party_roster.empty());
    REQUIRE(data.party_active.empty());
    REQUIRE(data.flags.empty());
    REQUIRE(data.inventory.empty());
    REQUIRE(data.quest_progress.empty());
    REQUIRE(data.relations.empty());
    REQUIRE(data.character_states.empty());
    REQUIRE(data.enemy_knowledge.empty());
    REQUIRE(data.slot_id == -1);
    REQUIRE(data.difficult_recovery_stage == 0);
    REQUIRE(data.credits == 0);
}

TEST_CASE("fresh_new_game_save_data: dificuldade escolhida e a UNICA que diverge",
          "[domain][save][new_game]") {
    for (const auto difficulty :
         {DifficultyLevel::Facil, DifficultyLevel::Medio, DifficultyLevel::Dificil,
          DifficultyLevel::Hardcore}) {
        const auto data = fresh_new_game_save_data(difficulty);
        REQUIRE(data.difficulty == difficulty);
    }
}

TEST_CASE("fresh_new_game_save_data: invariantes de SaveData::validate() satisfeitas",
          "[domain][save][new_game]") {
    // Um save novo tem que nascer VALIDO (fail-fast em validate() provaria bug de
    // construcao) - nao lanca.
    REQUIRE_NOTHROW(fresh_new_game_save_data(DifficultyLevel::Facil).validate());
}

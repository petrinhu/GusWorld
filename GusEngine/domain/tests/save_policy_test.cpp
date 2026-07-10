// GusEngine/domain/tests/save_policy_test.cpp
//
// Catch2 (TEST-FIRST) de save_policy.hpp (SAVE-LOAD-UI etapa 5, AUTOSAVE). Cobre o
// predicado autosave_allowed_at pela matriz completa de LocationKind x overrides -
// ver o header pra regra canonica (cidade sempre ON; dungeon so ON com PEM
// descoberto OU carta Gaiola de Faraday).

#include <catch2/catch_test_macros.hpp>

#include "gus/domain/save/save_policy.hpp"

using namespace gus::domain::save;

TEST_CASE("autosave_allowed_at: cidade e SEMPRE permitida, com ou sem os overrides",
          "[save_policy]") {
    REQUIRE(autosave_allowed_at(LocationKind::City, /*has_pem_discovered=*/false,
                                 /*has_faraday_card=*/false));
    REQUIRE(autosave_allowed_at(LocationKind::City, /*has_pem_discovered=*/true,
                                 /*has_faraday_card=*/false));
    REQUIRE(autosave_allowed_at(LocationKind::City, /*has_pem_discovered=*/false,
                                 /*has_faraday_card=*/true));
    REQUIRE(autosave_allowed_at(LocationKind::City, /*has_pem_discovered=*/true,
                                 /*has_faraday_card=*/true));
}

TEST_CASE("autosave_allowed_at: dungeon SEM PEM descoberto e SEM carta Faraday e "
          "NEGADA (estilo Zelda: save so na porta)",
          "[save_policy]") {
    REQUIRE_FALSE(autosave_allowed_at(LocationKind::Dungeon,
                                       /*has_pem_discovered=*/false,
                                       /*has_faraday_card=*/false));
}

TEST_CASE("autosave_allowed_at: dungeon COM PEM descoberto (sem a carta) e PERMITIDA",
          "[save_policy]") {
    REQUIRE(autosave_allowed_at(LocationKind::Dungeon, /*has_pem_discovered=*/true,
                                 /*has_faraday_card=*/false));
}

TEST_CASE("autosave_allowed_at: dungeon COM a carta Gaiola de Faraday (sem PEM) e "
          "PERMITIDA",
          "[save_policy]") {
    REQUIRE(autosave_allowed_at(LocationKind::Dungeon, /*has_pem_discovered=*/false,
                                 /*has_faraday_card=*/true));
}

TEST_CASE("autosave_allowed_at: dungeon com OS 2 overrides e PERMITIDA (OR logico)",
          "[save_policy]") {
    REQUIRE(autosave_allowed_at(LocationKind::Dungeon, /*has_pem_discovered=*/true,
                                 /*has_faraday_card=*/true));
}

TEST_CASE("autosave_allowed_at: e constexpr (avaliavel em tempo de compilacao)",
          "[save_policy]") {
    static_assert(autosave_allowed_at(LocationKind::City, false, false));
    static_assert(!autosave_allowed_at(LocationKind::Dungeon, false, false));
    SUCCEED();
}

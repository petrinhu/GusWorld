// xp_differential_test.cpp
//
// Spec executavel (Catch2 v3) da formula PURA de XP differential por zona,
// portada de engine/tests/knowledge/XpDifferentialTests.cs (xUnit). O xUnit e a
// SPEC do comportamento canonico (F2-G.XP); preservado 1:1.
//
// Subsistema portado (engine-design.md secao 2, marco M3): domain/progression
// (o CMakeLists do dominio reserva 'progression/ = EnemyKnowledgeTracker +
// XpDifferential'). POCO puro, ZERO Qt, headless.
//
// Formula canonica (combat.md secao 11):
//   xp = base_xp x max(0, 1 - (player_zone - enemy_zone) x 0.15)
//
// Penaliza farmar inimigo de zona muito ABAIXO da do player (anti-grind, KP
// secao 4). Casos-ancora: gap de 7 zonas zera o XP; player ABAIXO do inimigo
// NUNCA bonifica acima de base (clamp superior em 1.0 por design, KP secao 7
// proibe rubber-band/pity/catch-up). Ver impl para a decisao do clamp.
//
// Cross-ref: engine/foundation/knowledge/XpDifferential.cs (origem);
//            docs/design/mecanicas/combat.md secao 11, knowledge-progression.md secao 4/7.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <stdexcept>

#include "gus/domain/progression/xp_differential.hpp"

using gus::domain::progression::xp_award;
using gus::domain::progression::xp_factor;

// ---- mesmo nivel: sem penalidade ------------------------------------------

TEST_CASE("xp: mesma zona concede XP cheio", "[domain][progression][xp]") {
    // player_zone == enemy_zone -> fator 1.0 -> xp = base.
    REQUIRE(xp_award(/*base_xp=*/100, /*player_zone=*/3, /*enemy_zone=*/3) == 100);
}

// ---- player ACIMA do inimigo: penalidade linear 0.15 por zona de gap -------

TEST_CASE("xp: player acima do inimigo penaliza linearmente",
          "[domain][progression][xp]") {
    // gap n -> fator 1 - n*0.15. Espelha o Theory parametrizado do xUnit.
    constexpr double eps = 1e-9;
    REQUIRE_THAT(xp_factor(/*player_zone=*/1, /*enemy_zone=*/0),
                 Catch::Matchers::WithinAbs(0.85, eps));
    REQUIRE_THAT(xp_factor(2, 0), Catch::Matchers::WithinAbs(0.70, eps));
    REQUIRE_THAT(xp_factor(3, 0), Catch::Matchers::WithinAbs(0.55, eps));
    REQUIRE_THAT(xp_factor(6, 0), Catch::Matchers::WithinAbs(0.10, eps));
}

TEST_CASE("xp: award com player acima aplica penalidade e arredonda",
          "[domain][progression][xp]") {
    // base 100, gap 2 -> 0.70 -> 70.
    REQUIRE(xp_award(/*base_xp=*/100, /*player_zone=*/5, /*enemy_zone=*/3) == 70);
}

// ---- gap de 7 zonas zera (ancora da spec, KP secao 4) ----------------------

TEST_CASE("xp: gap de 7 zonas zera o XP", "[domain][progression][xp]") {
    // 1 - 7*0.15 = 1 - 1.05 = -0.05 -> clamp max(0, ...) -> 0.
    REQUIRE(xp_award(/*base_xp=*/100, /*player_zone=*/7, /*enemy_zone=*/0) == 0);
}

TEST_CASE("xp: gap enorme permanece em zero, nunca negativo",
          "[domain][progression][xp]") {
    const int xp = xp_award(/*base_xp=*/100, /*player_zone=*/20, /*enemy_zone=*/0);
    REQUIRE(xp == 0);
    REQUIRE(xp >= 0);
}

TEST_CASE("xp: fator nunca negativo", "[domain][progression][xp]") {
    constexpr double eps = 1e-9;
    REQUIRE_THAT(xp_factor(/*player_zone=*/10, /*enemy_zone=*/0),
                 Catch::Matchers::WithinAbs(0.0, eps));
}

// ---- player ABAIXO do inimigo: clamp superior em 1.0 (anti catch-up) -------
// Por design (KP secao 7, sem rubber-band/pity): o differential PUNE farmar
// fraco mas NAO bonifica cacar acima do nivel. Cap 1.0.

TEST_CASE("xp: player abaixo do inimigo tem fator limitado a 1.0",
          "[domain][progression][xp]") {
    // 1 - (2 - 5)*0.15 = 1 + 0.45 = 1.45 -> clamp -> 1.0 (sem bonus reverso).
    constexpr double eps = 1e-9;
    REQUIRE_THAT(xp_factor(/*player_zone=*/2, /*enemy_zone=*/5),
                 Catch::Matchers::WithinAbs(1.0, eps));
}

TEST_CASE("xp: award com player abaixo da base do inimigo sem bonus",
          "[domain][progression][xp]") {
    REQUIRE(xp_award(/*base_xp=*/100, /*player_zone=*/2, /*enemy_zone=*/9) == 100);
}

// ---- rounding determinístico (round-half-away-from-zero) -------------------

TEST_CASE("xp: arredonda half-away-from-zero", "[domain][progression][xp]") {
    // base 10, gap 3 -> 0.55 -> 5.5 -> arredonda para 6.
    REQUIRE(xp_award(/*base_xp=*/10, /*player_zone=*/3, /*enemy_zone=*/0) == 6);
}

// ---- invariantes de entrada (fail-fast) -----------------------------------

TEST_CASE("xp: base_xp negativo lanca", "[domain][progression][xp]") {
    REQUIRE_THROWS_AS(xp_award(/*base_xp=*/-1, /*player_zone=*/1, /*enemy_zone=*/1),
                      std::invalid_argument);
}

TEST_CASE("xp: player_zone negativo lanca", "[domain][progression][xp]") {
    REQUIRE_THROWS_AS(xp_award(/*base_xp=*/100, /*player_zone=*/-1, /*enemy_zone=*/1),
                      std::invalid_argument);
    REQUIRE_THROWS_AS(xp_factor(/*player_zone=*/-1, /*enemy_zone=*/1),
                      std::invalid_argument);
}

TEST_CASE("xp: enemy_zone negativo lanca", "[domain][progression][xp]") {
    REQUIRE_THROWS_AS(xp_award(/*base_xp=*/100, /*player_zone=*/1, /*enemy_zone=*/-1),
                      std::invalid_argument);
    REQUIRE_THROWS_AS(xp_factor(/*player_zone=*/1, /*enemy_zone=*/-1),
                      std::invalid_argument);
}

TEST_CASE("xp: base_xp zero concede zero", "[domain][progression][xp]") {
    REQUIRE(xp_award(/*base_xp=*/0, /*player_zone=*/1, /*enemy_zone=*/1) == 0);
}

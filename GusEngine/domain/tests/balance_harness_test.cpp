// balance_harness_test.cpp
//
// Spec do harness de balanceamento OFFLINE (COMBATE-TEORIA-JOGOS item [1]).
//
// TDD da AGREGACAO (aggregate/percentile) com dados SINTETICOS, pura e rapida, sem rodar
// o motor (secao a). Sanity da integracao real contra o motor JA existente, deterministica
// (secao b). O RUN GRANDE e opt-in via env GUSWORLD_BALANCE_HARNESS=1 (secao c): sem a env,
// roda uma amostra pequena (rapida, sempre verde no CI); com a env, roda milhares por
// cenario e imprime o relatorio completo pro lider/economy-designer calibrar encontros.
//
// Cross-ref: balance_harness.hpp; docs/design/mecanicas/combat.md secao 15/17/19;
//            TODO.md item COMBATE-TEORIA-JOGOS.

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <cstdlib>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "balance_harness.hpp"
#include "gus/domain/combat/combat_enums.hpp"

using namespace gus::domain::tests;
using gus::domain::combat::CombatOutcome;

namespace {

BattleOutcome make_outcome(CombatOutcome outcome, int rounds,
                          std::unordered_map<std::string, int> actions = {}) {
    BattleOutcome o;
    o.outcome = outcome;
    o.rounds = rounds;
    o.action_counts = std::move(actions);
    return o;
}

}  // namespace

// ============================================================================
// (a) aggregate(): PURA, dados sinteticos conhecidos (sem rodar o motor).
// ============================================================================

TEST_CASE("balance_harness: aggregate calcula win-rate em % sobre o total",
          "[domain][balance_harness]") {
    const std::vector<BattleOutcome> outcomes{
        make_outcome(CombatOutcome::Victory, 5),
        make_outcome(CombatOutcome::Victory, 5),
        make_outcome(CombatOutcome::Victory, 5),
        make_outcome(CombatOutcome::Defeat, 5),
    };
    const BalanceReport r = aggregate("cenario-x", outcomes);
    REQUIRE(r.label == "cenario-x");
    REQUIRE(r.battles == 4);
    REQUIRE(r.victories == 3);
    REQUIRE(r.defeats == 1);
    REQUIRE(r.fled == 0);
    REQUIRE(r.win_rate_pct == Catch::Approx(75.0));
}

TEST_CASE("balance_harness: aggregate com vetor vazio nao explode (battles=0, tudo zerado)",
          "[domain][balance_harness]") {
    const BalanceReport r = aggregate("vazio", {});
    REQUIRE(r.battles == 0);
    REQUIRE(r.win_rate_pct == 0.0);
    REQUIRE(r.mean_rounds == 0.0);
    REQUIRE_FALSE(r.window_4_8_ok);
    REQUIRE(r.action_share_pct.empty());
}

TEST_CASE("balance_harness: aggregate calcula media/mediana/p95 de rounds (dados conhecidos)",
          "[domain][balance_harness]") {
    // rounds ordenados: 2,4,4,6,6,6,8,8,10,20 (N=10)
    const std::vector<BattleOutcome> outcomes{
        make_outcome(CombatOutcome::Victory, 20), make_outcome(CombatOutcome::Victory, 4),
        make_outcome(CombatOutcome::Victory, 6),  make_outcome(CombatOutcome::Victory, 8),
        make_outcome(CombatOutcome::Victory, 2),  make_outcome(CombatOutcome::Victory, 6),
        make_outcome(CombatOutcome::Victory, 10), make_outcome(CombatOutcome::Victory, 8),
        make_outcome(CombatOutcome::Victory, 6),  make_outcome(CombatOutcome::Victory, 4),
    };
    const BalanceReport r = aggregate("duracao", outcomes);
    // media = (2+4+4+6+6+6+8+8+10+20)/10 = 74/10 = 7.4
    REQUIRE(r.mean_rounds == Catch::Approx(7.4));
    // mediana (rank continuo, indices 0..9): rank=0.5*9=4.5 -> interp entre valores[4]=6 e
    // valores[5]=6 -> 6.0
    REQUIRE(r.median_rounds == Catch::Approx(6.0));
    // p95: rank=0.95*9=8.55 -> interp entre valores[8]=10 e valores[9]=20 -> 10+0.55*10=15.5
    REQUIRE(r.p95_rounds == Catch::Approx(15.5));
}

TEST_CASE("balance_harness: window_4_8_ok exige MEDIA e MEDIANA dentro de [4,8] (secao 15)",
          "[domain][balance_harness]") {
    const std::vector<BattleOutcome> dentro{
        make_outcome(CombatOutcome::Victory, 4),
        make_outcome(CombatOutcome::Victory, 6),
        make_outcome(CombatOutcome::Victory, 8),
    };
    REQUIRE(aggregate("dentro", dentro).window_4_8_ok);

    const std::vector<BattleOutcome> fora_rapido{
        make_outcome(CombatOutcome::Victory, 1),
        make_outcome(CombatOutcome::Victory, 1),
        make_outcome(CombatOutcome::Victory, 2),
    };
    REQUIRE_FALSE(aggregate("fora-rapido", fora_rapido).window_4_8_ok);

    const std::vector<BattleOutcome> fora_lento{
        make_outcome(CombatOutcome::Victory, 15),
        make_outcome(CombatOutcome::Victory, 20),
        make_outcome(CombatOutcome::Victory, 25),
    };
    REQUIRE_FALSE(aggregate("fora-lento", fora_lento).window_4_8_ok);
}

TEST_CASE("balance_harness: aggregate computa dominancia de acao em % do total",
          "[domain][balance_harness]") {
    const std::vector<BattleOutcome> outcomes{
        make_outcome(CombatOutcome::Victory, 5, {{"party:attack", 3}, {"enemy:attack", 1}}),
        make_outcome(CombatOutcome::Victory, 5, {{"party:attack", 3}, {"enemy:pass", 1}}),
    };
    const BalanceReport r = aggregate("dominancia", outcomes);
    // total = 3+1+3+1 = 8; party:attack = 6/8 = 75%; enemy:attack = 1/8 = 12.5%;
    // enemy:pass = 1/8 = 12.5%.
    REQUIRE(r.action_share_pct.at("party:attack") == Catch::Approx(75.0));
    REQUIRE(r.action_share_pct.at("enemy:attack") == Catch::Approx(12.5));
    REQUIRE(r.action_share_pct.at("enemy:pass") == Catch::Approx(12.5));
}

TEST_CASE("balance_harness: Fled entra na contagem sem quebrar win-rate (so Victory conta)",
          "[domain][balance_harness]") {
    const std::vector<BattleOutcome> outcomes{
        make_outcome(CombatOutcome::Victory, 5),
        make_outcome(CombatOutcome::Fled, 3),
    };
    const BalanceReport r = aggregate("fuga", outcomes);
    REQUIRE(r.fled == 1);
    REQUIRE(r.win_rate_pct == Catch::Approx(50.0));
}

// ============================================================================
// (b) run_single_battle / run_scenario: integracao real, deterministica, contra o motor
//     JA EXISTENTE (CombatStateMachine::run_until_end). Sanity rapida, nao o run grande.
// ============================================================================

TEST_CASE("balance_harness: run_single_battle e deterministico pra mesma seed",
          "[domain][balance_harness]") {
    const BalanceScenario cenario{
        "sanity",
        {{"gus", 34, 8, 5, 9, gus::domain::combat::CardFamily::Eletrico}},
        {{"enemy", 20, 4, 2, 3, gus::domain::combat::CardFamily::Cinetico}},
    };
    const BattleOutcome a = run_single_battle(cenario, /*seed=*/42);
    const BattleOutcome b = run_single_battle(cenario, /*seed=*/42);
    REQUIRE(a.outcome == b.outcome);
    REQUIRE(a.rounds == b.rounds);
    REQUIRE(a.action_counts == b.action_counts);
}

TEST_CASE("balance_harness: party overwhelming (atk alto) vence rapido contra trash fraco",
          "[domain][balance_harness]") {
    const BalanceScenario cenario{
        "overwhelming",
        {{"gus", 999, 999, 5, 9, gus::domain::combat::CardFamily::Eletrico}},
        {{"trash", 5, 1, 0, 3, gus::domain::combat::CardFamily::Cinetico}},
    };
    const BattleOutcome o = run_single_battle(cenario, /*seed=*/1);
    REQUIRE(o.outcome == CombatOutcome::Victory);
    REQUIRE(o.rounds <= 2);
    REQUIRE(o.action_counts.at("party:attack") >= 1);
}

TEST_CASE("balance_harness: party fragil sozinha perde contra inimigo overwhelming",
          "[domain][balance_harness]") {
    const BalanceScenario cenario{
        "fragil",
        {{"gus", 1, 1, 0, 3, gus::domain::combat::CardFamily::Eletrico}},
        {{"boss", 999, 999, 0, 9, gus::domain::combat::CardFamily::Cinetico}},
    };
    const BattleOutcome o = run_single_battle(cenario, /*seed=*/1);
    REQUIRE(o.outcome == CombatOutcome::Defeat);
}

TEST_CASE("balance_harness: run_scenario agrega N batalhas com seeds diferentes",
          "[domain][balance_harness]") {
    const BalanceScenario cenario{
        "amostra",
        {{"gus", 999, 50, 5, 9, gus::domain::combat::CardFamily::Eletrico}},
        {{"trash", 40, 3, 2, 3, gus::domain::combat::CardFamily::Cinetico}},
    };
    const BalanceReport r = run_scenario(cenario, /*n_battles=*/20, /*base_seed=*/100);
    REQUIRE(r.battles == 20);
    REQUIRE(r.victories + r.defeats + r.fled == 20);
    REQUIRE(r.win_rate_pct > 0.0);  // party bem mais forte: alguma vitoria esperada
}

// ============================================================================
// (c) Run grande opt-in (env GUSWORLD_BALANCE_HARNESS=1). Sem a env: amostra pequena
//     (rapida, sempre roda no CI). Com a env: milhares por cenario, relatorio completo
//     impresso pro lider/economy-designer (entregavel de valor do item [1]).
// ============================================================================

TEST_CASE("balance_harness: relatorio dos cenarios canonicos (secao 17) via AutoResolveBrain",
          "[domain][balance_harness][balance_report]") {
    const bool full_run = std::getenv("GUSWORLD_BALANCE_HARNESS") != nullptr;
    const int n_battles = full_run ? 5000 : 50;
    const std::uint32_t base_seed = 20260710;

    std::cout << "\n=== Harness de balanceamento OFFLINE (COMBATE-TEORIA-JOGOS [1]) ===\n"
             << "N=" << n_battles << " por cenario ("
             << (full_run ? "RUN COMPLETO" : "amostra CI, setar GUSWORLD_BALANCE_HARNESS=1 pro run grande")
             << ")\n";

    for (const BalanceScenario& cenario : canonical_scenarios()) {
        const BalanceReport r = run_scenario(cenario, n_battles, base_seed);
        print_report(std::cout, r);
        // Sanidade estrutural (nao e calibracao de balanceamento, so garante que o
        // pipeline produziu numeros coerentes): toda batalha termina, taxas em [0,100].
        REQUIRE(r.battles == n_battles);
        REQUIRE(r.victories + r.defeats + r.fled == r.battles);
        REQUIRE(r.win_rate_pct >= 0.0);
        REQUIRE(r.win_rate_pct <= 100.0);
    }
}

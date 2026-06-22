// environment_fsm_test.cpp
//
// Spec executavel (Catch2 v3) da integracao do ambiente na CombatStateMachine:
// set_environment, avanco automatico do periodo por rodada de fila, verb Scan-ambiente
// (1 AP), e o buffer environment_changes. Portado de
// engine/tests/turn_combat/environments/EnvironmentFsmTests.cs. POCO puro, ZERO Qt.
//
// Cross-ref: engine/tests/turn_combat/environments/EnvironmentFsmTests.cs;
//            docs/design/mecanicas/combat.md secao 18.3/18.5/18.10/16.

#include <catch2/catch_test_macros.hpp>

#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <algorithm>
#include <string>
#include <vector>

#include "gus/domain/combat/combat_actor.hpp"
#include "gus/domain/combat/combat_constants.hpp"
#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/combat/combat_records.hpp"
#include "gus/domain/combat/combat_state.hpp"
#include "gus/domain/combat/combat_state_machine.hpp"
#include "gus/domain/combat/environment_enums.hpp"

using namespace gus::domain::combat;
using Catch::Matchers::WithinAbs;

namespace {

CombatActor hero(const std::string& id = "gus", int spd = 20) {
    return CombatActor(id, id, 50, 8, 2, spd, CardFamily::Eletrico, /*player=*/true);
}

CombatActor foe(const std::string& id = "enemy", int spd = 10) {
    return CombatActor(id, id, 500, 6, 1, spd, CardFamily::Cinetico, /*player=*/false);
}

CombatAction always_pass(CombatActor&, const CombatState&) { return CombatAction::pass(); }

bool changes_contains(const CombatStateMachine& sm, EnvironmentId id) {
    const auto& c = sm.environment_changes();
    return std::find(c.begin(), c.end(), id) != c.end();
}

bool log_has(const CombatStateMachine& sm, CombatActionType action,
             const std::string& needle) {
    for (const auto& e : sm.log())
        if (e.action == action && e.message.find(needle) != std::string::npos) return true;
    return false;
}

}  // namespace

// ===== set_environment =====

TEST_CASE("env-fsm: set_environment registra camadas ativas e environment_changes",
          "[domain][combat][env]") {
    CombatActor h = hero();
    CombatActor e = foe();
    CombatStateMachine sm({&h, &e}, always_pass);
    sm.set_environment({EnvironmentId::Lamacento, EnvironmentId::Chuva});
    REQUIRE(sm.active_environments().size() == 2);
    REQUIRE(sm.environment_changes() ==
            std::vector<EnvironmentId>{EnvironmentId::Lamacento, EnvironmentId::Chuva});
}

TEST_CASE("env-fsm: set_environment ignora None", "[domain][combat][env]") {
    CombatActor h = hero();
    CombatActor e = foe();
    CombatStateMachine sm({&h, &e}, always_pass);
    sm.set_environment({EnvironmentId::None, EnvironmentId::Chuva, EnvironmentId::None});
    REQUIRE(sm.active_environments().size() == 1);
    REQUIRE(sm.environment_changes() == std::vector<EnvironmentId>{EnvironmentId::Chuva});
}

TEST_CASE("env-fsm: set_environment com periodo instancia o relogio",
          "[domain][combat][env]") {
    CombatActor h = hero();
    CombatActor e = foe();
    CombatStateMachine sm({&h, &e}, always_pass);
    sm.set_environment({EnvironmentId::Noite});
    REQUIRE(sm.period_clock() != nullptr);
    REQUIRE(sm.period_clock()->current_phase() == EnvironmentId::Noite);
}

TEST_CASE("env-fsm: set_environment sem periodo deixa relogio nulo",
          "[domain][combat][env]") {
    CombatActor h = hero();
    CombatActor e = foe();
    CombatStateMachine sm({&h, &e}, always_pass);
    sm.set_environment({EnvironmentId::Chuva});
    REQUIRE(sm.period_clock() == nullptr);
}

// ===== Periodo avanca automaticamente por rodada de fila =====

TEST_CASE("env-fsm: periodo avanca uma fase por rodada completa de fila",
          "[domain][combat][env]") {
    CombatActor h = hero();
    CombatActor e = foe();
    CombatStateMachine sm({&h, &e}, always_pass);
    sm.set_environment({EnvironmentId::Dia});

    for (int rodada = 0; rodada < 5; ++rodada) {
        sm.begin_turn(); sm.run_active_turn_to_end(); sm.check_end(); sm.advance_to_next_actor();
        sm.begin_turn(); sm.run_active_turn_to_end(); sm.check_end(); sm.advance_to_next_actor();
    }

    REQUIRE(sm.period_clock()->current_phase() == EnvironmentId::Crepusculo);
    REQUIRE(changes_contains(sm, EnvironmentId::Crepusculo));
}

TEST_CASE("env-fsm: mudanca de periodo muda o mult_ambiente da familia",
          "[domain][combat][env]") {
    CombatActor h = hero();
    CombatActor e = foe();
    CombatStateMachine sm({&h, &e}, always_pass);
    sm.set_environment({EnvironmentId::Dia});
    REQUIRE_THAT(sm.mult_ambiente_for(CardFamily::Bioquimico), WithinAbs(1.3f, 1e-4));

    for (int rodada = 0; rodada < 5; ++rodada) {
        sm.begin_turn(); sm.run_active_turn_to_end(); sm.check_end(); sm.advance_to_next_actor();
        sm.begin_turn(); sm.run_active_turn_to_end(); sm.check_end(); sm.advance_to_next_actor();
    }

    REQUIRE(sm.period_clock()->current_phase() == EnvironmentId::Crepusculo);
    REQUIRE_THAT(sm.mult_ambiente_for(CardFamily::Bioquimico), WithinAbs(1.0f, 1e-4));
}

// ===== Verb Scan-ambiente (1 AP) =====

TEST_CASE("env-fsm: scan-ambiente gasta 1 ap e marca environment_scanned",
          "[domain][combat][env]") {
    CombatActor h = hero();
    CombatActor e = foe();
    auto scanned = std::make_shared<bool>(false);
    CombatStateMachine sm({&h, &e}, [scanned](CombatActor& a, const CombatState&) -> CombatAction {
        if (!a.is_player_side() || *scanned) return CombatAction::pass();
        *scanned = true;
        return CombatAction::scan_environment();
    });
    sm.set_environment({EnvironmentId::EspelhoRessonante});  // terreno Codex
    REQUIRE_FALSE(sm.environment_scanned());

    sm.begin_turn();
    REQUIRE(h.ap() == combat_constants::kBaseApPerTurn);
    sm.run_active_turn_to_end();

    REQUIRE(sm.environment_scanned());
    REQUIRE(h.ap() == combat_constants::kBaseApPerTurn - 1);
    REQUIRE(log_has(sm, CombatActionType::ScanEnvironment, ""));
}

TEST_CASE("env-fsm: scan-ambiente loga proxima troca quando ha periodo",
          "[domain][combat][env]") {
    CombatActor h = hero();
    CombatActor e = foe();
    auto played = std::make_shared<bool>(false);
    CombatStateMachine sm({&h, &e}, [played](CombatActor& a, const CombatState&) -> CombatAction {
        if (!a.is_player_side() || *played) return CombatAction::pass();
        *played = true;
        return CombatAction::scan_environment();
    });
    sm.set_environment({EnvironmentId::Dia});
    sm.begin_turn();
    sm.run_active_turn_to_end();
    REQUIRE(log_has(sm, CombatActionType::ScanEnvironment, "Crepusculo"));
}

TEST_CASE("env-fsm: scan-ambiente factory custa 1 ap sem alvo",
          "[domain][combat][env]") {
    const CombatAction action = CombatAction::scan_environment();
    REQUIRE(action.type == CombatActionType::ScanEnvironment);
    REQUIRE(action.ap_cost == 1);
    REQUIRE_FALSE(action.target_id.has_value());
}

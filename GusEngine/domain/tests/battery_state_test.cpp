// battery_state_test.cpp
//
// Spec executavel (Catch2 v3) da PECA de energia/bateria de uma carta possuida
// (ATOM-1, decomposicao atomica de CardPhysicalState em pecas componiveis,
// gus/domain/hardware/battery_state.hpp): BatteryState, state_of_health_percent(),
// is_battery_dead(), battery_charge_remaining(). Cobertura FOCADA na peca isolada; a
// spec exaustiva do AGREGADO CardPhysicalState continua em card_hardware_test.cpp
// (zero diff, criterio de aceite do ATOM-1).
//
// Cross-ref: docs/design/mecanicas/cartas-numeros-proposta.md secao 1b;
//            gus/domain/deck/card_hardware.hpp (fachada agregada, herda desta peca);
//            card_hardware_test.cpp (oraculo do agregado).

#include <catch2/catch_test_macros.hpp>

#include <cstdint>

#include "gus/domain/hardware/battery_state.hpp"

using gus::domain::hardware::battery_charge_remaining;
using gus::domain::hardware::BatteryState;
using gus::domain::hardware::is_battery_dead;
using gus::domain::hardware::kBatteryDeadSohFloorPercent;
using gus::domain::hardware::kBatteryDegradationPerRechargeCyclePp;
using gus::domain::hardware::state_of_health_percent;

TEST_CASE("battery_state: default e bateria cheia (0 ciclos, 0 deficit)",
          "[domain][hardware][battery_state]") {
    BatteryState b;
    REQUIRE(b.battery_recharge_cycles == 0);
    REQUIRE(b.battery_charge_deficit == 0);
}

TEST_CASE("battery_state: operator== compara por valor",
          "[domain][hardware][battery_state]") {
    BatteryState a;
    BatteryState b;
    REQUIRE(a == b);
    b.battery_recharge_cycles = 1;
    REQUIRE_FALSE(a == b);
}

TEST_CASE("battery_state: constantes de degradacao/piso bate o valor fechado (13pp, "
          "piso 21%)",
          "[domain][hardware][battery_state]") {
    REQUIRE(kBatteryDegradationPerRechargeCyclePp == 13);
    REQUIRE(kBatteryDeadSohFloorPercent == 21);
}

TEST_CASE("battery_state: state_of_health_percent comeca em 100% e cai 13pp por ciclo",
          "[domain][hardware][battery_state][soh]") {
    BatteryState b;
    REQUIRE(state_of_health_percent(b) == 100);
    b.battery_recharge_cycles = 1;
    REQUIRE(state_of_health_percent(b) == 87);
    b.battery_recharge_cycles = 2;
    REQUIRE(state_of_health_percent(b) == 74);
}

TEST_CASE("battery_state: state_of_health_percent com cycles ENORME (uint16 max) "
          "clampa em 0 sem overflow/UB",
          "[domain][hardware][battery_state][soh][extremo]") {
    BatteryState b;
    b.battery_recharge_cycles = 65535;
    REQUIRE(state_of_health_percent(b) == 0);
}

TEST_CASE("battery_state: is_battery_dead segue o piso de 21% SoH (<=, nao <)",
          "[domain][hardware][battery_state][soh]") {
    BatteryState b;
    b.battery_recharge_cycles = 6;
    REQUIRE(state_of_health_percent(b) == 22);
    REQUIRE_FALSE(is_battery_dead(b));
    b.battery_recharge_cycles = 7;
    REQUIRE(state_of_health_percent(b) == 9);
    REQUIRE(is_battery_dead(b));
}

TEST_CASE("battery_state: battery_charge_remaining = capacidade - deficit",
          "[domain][hardware][battery_state][battery]") {
    BatteryState b;
    b.battery_charge_deficit = 20;
    REQUIRE(battery_charge_remaining(b, 55) == 35);
}

TEST_CASE("battery_state: battery_charge_remaining com deficit > capacidade clampa "
          "em 0 SEM underflow de unsigned",
          "[domain][hardware][battery_state][battery][extremo]") {
    BatteryState b;
    b.battery_charge_deficit = 999999;
    REQUIRE(battery_charge_remaining(b, 55) == 0);
}

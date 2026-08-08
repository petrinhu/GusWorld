// SPDX-License-Identifier: Apache-2.0
// healer_card_test.cpp
//
// Spec executavel (Catch2 v3) de healer_card.hpp (TODO.md CURANDEIRA-LIMITE-USOS, W2):
// resolve_healer_card_use, o motor PURO da carta de cura da Jaci (Sylvesse-Religação)
// contra a bateria FISICA da copia jogada. Ver o doc-comment do header pro racional
// completo (por que NAO e EffectKind, gate de camadas, escopo fora desta fatia).
//
// RNG deterministico via ScriptedRandom (helper LOCAL deste arquivo - fixed_random.hpp/
// counting_random.hpp existentes cravam o MESMO valor em toda chamada, mas
// resolve_healer_card_use pode consumir ATE 3 rolagens distintas por uso - falha/cura/
// dreno -, entao precisa de uma FILA de valores, nao um valor fixo repetido).
//
// Cross-ref: gus/domain/combat/healer_card.hpp; docs/design/mecanicas/
// cartas-hardware-pirataria-energia.md §3; TODO.md CURANDEIRA-LIMITE-USOS.

#include <catch2/catch_test_macros.hpp>

#include <stdexcept>
#include <vector>

#include "gus/domain/combat/combat_actor.hpp"
#include "gus/domain/combat/combat_records.hpp"
#include "gus/domain/combat/healer_card.hpp"
#include "gus/domain/combat/random_source.hpp"
#include "gus/domain/hardware/battery_state.hpp"
#include "gus/domain/hardware/hardware_class.hpp"
#include "gus/domain/save/save_data.hpp"

using gus::domain::combat::CombatActor;
using gus::domain::combat::CombatLogEntry;
using gus::domain::combat::HealerCardTuning;
using gus::domain::combat::HealerCardUseResult;
using gus::domain::combat::healer_card_row_for;
using gus::domain::combat::healer_card_tuning_for;
using gus::domain::combat::IRandomSource;
using gus::domain::combat::resolve_healer_card_use;
using gus::domain::hardware::BatteryState;
using gus::domain::hardware::HardwareClass;
using gus::domain::save::DifficultyLevel;

namespace {

// Duplo deterministico que devolve uma FILA de valores para next(int) (1 por chamada,
// consumida na ordem) e um double fixo para next_double() (nao usado por
// resolve_healer_card_use, mas a porta exige a implementacao). Lanca std::logic_error
// se a fila esvaziar (destrava um teste que sub-estimou o consumo real).
class ScriptedRandom final : public IRandomSource {
public:
    explicit ScriptedRandom(std::vector<int> next_queue) : queue_(std::move(next_queue)) {}

    double next_double() override { return 0.5; }

    int next(int max_value) override {
        if (cursor_ >= queue_.size())
            throw std::logic_error("ScriptedRandom: fila de next() esgotada - o teste "
                                    "sub-estimou o consumo de RNG.");
        const int scripted = queue_[cursor_++];
        ++calls;
        return max_value <= 0 ? 0 : scripted;
    }

    std::size_t calls = 0;

private:
    std::vector<int> queue_;
    std::size_t cursor_ = 0;
};

CombatActor make_actor(const std::string& id, int hp) {
    return CombatActor(id, id, hp, /*atk=*/5, /*def=*/5, /*spd=*/5,
                        gus::domain::combat::CardFamily::Bioquimico,
                        /*is_player_side=*/true);
}

constexpr std::uint32_t kOriginalMedioCapacity = 55;   // kBatteryCapacityTable[0][Medio]
constexpr std::uint32_t kPirataComumMedioCapacity = 21;  // kBatteryCapacityTable[2][Medio]

}  // namespace

// ---- Tabela: valores fixos travados (mesmo padrao de enemy_difficulty_constants_test) --

TEST_CASE("healer_card: tabela ComumOriginal Medio - cura 12 fixo, dreno 50% fixo "
          "(valor REAL/definitivo desta fatia)",
          "[domain][combat][healer_card]") {
    const HealerCardTuning& tuning =
        healer_card_tuning_for(HardwareClass::ComumOriginal, DifficultyLevel::Medio);
    REQUIRE(tuning.heal_min == 12);
    REQUIRE(tuning.heal_max == 12);
    REQUIRE(tuning.drain_percent_min == 50);
    REQUIRE(tuning.drain_percent_max == 50);
}

TEST_CASE("healer_card: tabela PirataComum Medio - cura 9-11, dreno 55-70 (valor REAL/"
          "definitivo desta fatia)",
          "[domain][combat][healer_card]") {
    const HealerCardTuning& tuning =
        healer_card_tuning_for(HardwareClass::PirataComum, DifficultyLevel::Medio);
    REQUIRE(tuning.heal_min == 9);
    REQUIRE(tuning.heal_max == 11);
    REQUIRE(tuning.drain_percent_min == 55);
    REQUIRE(tuning.drain_percent_max == 70);
}

TEST_CASE("healer_card: Facil/Dificil/Hardcore replicam Medio (PLACEHOLDER, nao "
          "calibrado - mesmo padrao de DIFICULDADE-TABELA-DADO)",
          "[domain][combat][healer_card]") {
    for (const auto hw : {HardwareClass::ComumOriginal, HardwareClass::PirataComum}) {
        const HealerCardTuning& medio = healer_card_tuning_for(hw, DifficultyLevel::Medio);
        for (const auto diff :
             {DifficultyLevel::Facil, DifficultyLevel::Dificil, DifficultyLevel::Hardcore}) {
            const HealerCardTuning& other = healer_card_tuning_for(hw, diff);
            REQUIRE(other.heal_min == medio.heal_min);
            REQUIRE(other.heal_max == medio.heal_max);
            REQUIRE(other.drain_percent_min == medio.drain_percent_min);
            REQUIRE(other.drain_percent_max == medio.drain_percent_max);
        }
    }
}

TEST_CASE("healer_card: HardwareClass sem tuning definido lanca logic_error (fail-fast, "
          "nenhum numero foi inventado pra Homebrew/PirataEspecialFalso/EspecialSelada)",
          "[domain][combat][healer_card]") {
    REQUIRE_THROWS_AS(healer_card_row_for(HardwareClass::HomebrewEprom), std::logic_error);
    REQUIRE_THROWS_AS(healer_card_row_for(HardwareClass::PirataEspecialFalso),
                       std::logic_error);
    REQUIRE_THROWS_AS(healer_card_row_for(HardwareClass::EspecialSelada), std::logic_error);
}

// ---- resolve_healer_card_use: carta ORIGINAL -----------------------------------------

TEST_CASE("healer_card: original cura 12 HP fixo e drena 50% da carga ATUAL "
          "(round-half-up: bateria cheia 55 -> deficit 28); ZERO consumo de RNG "
          "(SoH=100, cura fixa, dreno fixo)",
          "[domain][combat][healer_card]") {
    BatteryState battery{};  // cheia, SoH=100 (cycles=0)
    ScriptedRandom rng({});  // fila VAZIA - qualquer next() aqui e um teste quebrado
    CombatActor jaci = make_actor("jaci", 40);
    CombatActor ally = make_actor("gus", 30);
    ally.take_damage(20);  // hp=10, espaco de sobra pra curar 12 sem clampar em max_hp
    std::vector<CombatLogEntry> log;

    const HealerCardUseResult result = resolve_healer_card_use(
        HardwareClass::ComumOriginal, DifficultyLevel::Medio, battery,
        kOriginalMedioCapacity, jaci, ally, rng, &log);

    REQUIRE_FALSE(result.failed);
    REQUIRE(result.healed_amount == 12);
    REQUIRE(result.drain_applied == 28);  // round((55 * 50) / 100) = round(27.5) = 28
    REQUIRE(result.roll_fail_percent == -1);
    REQUIRE(rng.calls == 0);
    REQUIRE(ally.hp() == 22);
    REQUIRE(battery.battery_charge_deficit == 28);
    REQUIRE_FALSE(log.empty());
}

TEST_CASE("healer_card: original - usos consecutivos NUNCA deixam a carga negativa E "
          "SEMPRE alcancam exatamente 0 apos usos finitos (round-half-up nao deixa "
          "residuo de 1 preso pra sempre, ao contrario de floor)",
          "[domain][combat][healer_card]") {
    BatteryState battery{};
    ScriptedRandom rng({});
    CombatActor jaci = make_actor("jaci", 40);
    CombatActor ally = make_actor("gus", 100);

    const auto r1 = resolve_healer_card_use(HardwareClass::ComumOriginal,
                                             DifficultyLevel::Medio, battery,
                                             kOriginalMedioCapacity, jaci, ally, rng);
    REQUIRE(r1.drain_applied == 28);
    REQUIRE(gus::domain::hardware::battery_charge_remaining(
                battery, kOriginalMedioCapacity) == 27);

    const auto r2 = resolve_healer_card_use(HardwareClass::ComumOriginal,
                                             DifficultyLevel::Medio, battery,
                                             kOriginalMedioCapacity, jaci, ally, rng);
    // 2o uso drena round(27*50/100) = round(13.5) = 14 -> deficit total 42,
    // remanescente 13.
    REQUIRE(r2.drain_applied == 14);
    const std::uint32_t remaining_after_2 =
        gus::domain::hardware::battery_charge_remaining(battery, kOriginalMedioCapacity);
    REQUIRE(remaining_after_2 == 13);

    // Usos adicionais ate esvaziar - bound generoso (20) so pra nao rodar pra sempre
    // se uma regressao reintroduzir o bug do residuo-de-1 preso (floor). A carga REAL
    // chega a 0 em poucos usos (13 -> 7 -> 3 -> 1 -> 0, round-half-up).
    int guard = 0;
    while (gus::domain::hardware::battery_charge_remaining(battery, kOriginalMedioCapacity) >
               0 &&
           guard < 20) {
        const auto r = resolve_healer_card_use(HardwareClass::ComumOriginal,
                                                DifficultyLevel::Medio, battery,
                                                kOriginalMedioCapacity, jaci, ally, rng);
        REQUIRE_FALSE(r.failed);
        REQUIRE(r.drain_applied >= 1);  // round-half-up nunca estagna em drain 0
        ++guard;
    }
    REQUIRE(gus::domain::hardware::battery_charge_remaining(
                battery, kOriginalMedioCapacity) == 0);
    REQUIRE(guard < 20);  // provou que convergiu, nao que o bound estourou
}

TEST_CASE("healer_card: carta esgotada (carga remanescente == 0) lanca logic_error - "
          "bug de call site, o chamador deve gatear ANTES de invocar (mesmo padrao de "
          "CombatActor::spend_ap/spend_mana)",
          "[domain][combat][healer_card]") {
    BatteryState battery{};
    battery.battery_charge_deficit = kOriginalMedioCapacity;  // 0 remanescente
    ScriptedRandom rng({});
    CombatActor jaci = make_actor("jaci", 40);
    CombatActor ally = make_actor("gus", 30);

    REQUIRE_THROWS_AS(resolve_healer_card_use(HardwareClass::ComumOriginal,
                                               DifficultyLevel::Medio, battery,
                                               kOriginalMedioCapacity, jaci, ally, rng),
                       std::logic_error);
}

// ---- resolve_healer_card_use: carta PIRATA (RNG) --------------------------------------

TEST_CASE("healer_card: pirata cura no PISO da faixa (9) e drena no PISO (55%) quando o "
          "RNG sorteia o menor valor de cada faixa",
          "[domain][combat][healer_card]") {
    BatteryState battery{};  // SoH=100 -> 0 rolagem de falha
    // 1a chamada de next() = cura (range 3: 9,10,11 -> indice 0 = 9);
    // 2a chamada de next() = dreno (range 16: 55..70 -> indice 0 = 55%).
    ScriptedRandom rng({0, 0});
    CombatActor jaci = make_actor("jaci", 40);
    CombatActor ally = make_actor("gus", 30);
    ally.take_damage(20);  // hp=10

    const HealerCardUseResult result = resolve_healer_card_use(
        HardwareClass::PirataComum, DifficultyLevel::Medio, battery,
        kPirataComumMedioCapacity, jaci, ally, rng);

    REQUIRE_FALSE(result.failed);
    REQUIRE(result.healed_amount == 9);
    REQUIRE(result.drain_applied == 12);  // round((21 * 55 + 50) / 100) == round(11.55) == 12
    REQUIRE(rng.calls == 2);
    REQUIRE(ally.hp() == 19);
}

TEST_CASE("healer_card: pirata cura no TETO da faixa (11) e drena no TETO (70%) quando "
          "o RNG sorteia o maior valor de cada faixa",
          "[domain][combat][healer_card]") {
    BatteryState battery{};
    // cura: range 3 (9,10,11), indice 2 = 11. dreno: range 16 (55..70), indice 15 = 70%.
    ScriptedRandom rng({2, 15});
    CombatActor jaci = make_actor("jaci", 40);
    CombatActor ally = make_actor("gus", 30);
    ally.take_damage(20);

    const HealerCardUseResult result = resolve_healer_card_use(
        HardwareClass::PirataComum, DifficultyLevel::Medio, battery,
        kPirataComumMedioCapacity, jaci, ally, rng);

    REQUIRE_FALSE(result.failed);
    REQUIRE(result.healed_amount == 11);
    REQUIRE(result.drain_applied == 15);  // round((21 * 70 + 50) / 100) == round(14.7) == 15
    REQUIRE(rng.calls == 2);
}

// ---- resolve_healer_card_use: falha por degradacao (SoH) ------------------------------

TEST_CASE("healer_card: SoH=100 (bateria nova) NUNCA rola falha - 0 consumo de RNG "
          "pro eixo de falha, determinismo por construcao (nao por sorte)",
          "[domain][combat][healer_card]") {
    BatteryState battery{};  // cycles=0 -> SoH=100
    REQUIRE(gus::domain::hardware::state_of_health_percent(battery) == 100);
    ScriptedRandom rng({});  // cura/dreno FIXOS (original) -> fila vazia e correta
    CombatActor jaci = make_actor("jaci", 40);
    CombatActor ally = make_actor("gus", 30);

    const auto result =
        resolve_healer_card_use(HardwareClass::ComumOriginal, DifficultyLevel::Medio,
                                 battery, kOriginalMedioCapacity, jaci, ally, rng);
    REQUIRE_FALSE(result.failed);
    REQUIRE(result.roll_fail_percent == -1);
    REQUIRE(rng.calls == 0);
}

TEST_CASE("healer_card: SoH degradado - fail_percent = 100-SoH%, roll < fail_percent "
          "FALHA (nem cura nem drena)",
          "[domain][combat][healer_card]") {
    BatteryState battery{};
    battery.battery_recharge_cycles = 5;  // SoH = 100 - 13*5 = 35% -> fail_percent = 65
    REQUIRE(gus::domain::hardware::state_of_health_percent(battery) == 35);
    // 1a chamada de next(100) = rolagem de falha; roll=64 < 65 -> FALHA (nao consome
    // mais nada: cura/dreno nunca sao rolados apos falha).
    ScriptedRandom rng({64});
    CombatActor jaci = make_actor("jaci", 40);
    CombatActor ally = make_actor("gus", 30);
    ally.take_damage(20);  // hp=10
    std::vector<CombatLogEntry> log;

    const auto result =
        resolve_healer_card_use(HardwareClass::ComumOriginal, DifficultyLevel::Medio,
                                 battery, kOriginalMedioCapacity, jaci, ally, rng, &log);

    REQUIRE(result.failed);
    REQUIRE(result.healed_amount == 0);
    REQUIRE(result.drain_applied == 0);
    REQUIRE(result.roll_fail_percent == 65);
    REQUIRE(rng.calls == 1);
    REQUIRE(ally.hp() == 10);  // intocado
    REQUIRE(battery.battery_charge_deficit == 0);  // intocado
    REQUIRE_FALSE(log.empty());
}

TEST_CASE("healer_card: SoH degradado - roll >= fail_percent NAO falha, segue pro "
          "fluxo normal de cura/dreno (consome os 2 eixos seguintes tambem, se pirata)",
          "[domain][combat][healer_card]") {
    BatteryState battery{};
    battery.battery_recharge_cycles = 5;  // SoH=35, fail_percent=65
    // 1a chamada = rolagem de falha; roll=65 (nao < 65 -> NAO falha). Original e
    // fixo, entao nenhuma chamada extra e consumida.
    ScriptedRandom rng({65});
    CombatActor jaci = make_actor("jaci", 40);
    CombatActor ally = make_actor("gus", 30);

    const auto result =
        resolve_healer_card_use(HardwareClass::ComumOriginal, DifficultyLevel::Medio,
                                 battery, kOriginalMedioCapacity, jaci, ally, rng);

    REQUIRE_FALSE(result.failed);
    REQUIRE(result.healed_amount == 12);
    REQUIRE(result.roll_fail_percent == 65);
    REQUIRE(rng.calls == 1);
}

TEST_CASE("healer_card: SoH=0 (degradacao extrema) e falha AUTOMATICA (fail_percent=100, "
          "qualquer roll < 100 falha)",
          "[domain][combat][healer_card]") {
    BatteryState battery{};
    battery.battery_recharge_cycles = 20;  // 13*20=260 -> clamp 0
    REQUIRE(gus::domain::hardware::state_of_health_percent(battery) == 0);
    ScriptedRandom rng({0});  // qualquer valor < 100 falha; 0 e o minimo
    CombatActor jaci = make_actor("jaci", 40);
    CombatActor ally = make_actor("gus", 30);

    const auto result =
        resolve_healer_card_use(HardwareClass::ComumOriginal, DifficultyLevel::Medio,
                                 battery, kOriginalMedioCapacity, jaci, ally, rng);
    REQUIRE(result.failed);
    REQUIRE(result.roll_fail_percent == 100);
}

TEST_CASE("healer_card: is_battery_dead() (piso de RECARGA, SoH<=21%) e ORTOGONAL a "
          "esta funcao - bateria 'morta' pra recarga ainda pode ter chance de sucesso "
          "de uso (fail_percent < 100 quando SoH > 0)",
          "[domain][combat][healer_card]") {
    BatteryState battery{};
    battery.battery_recharge_cycles = 6;  // SoH = 100-78 = 22% (> piso 21, ainda nao dead)
    REQUIRE_FALSE(gus::domain::hardware::is_battery_dead(battery));
    // fail_percent = 78. roll=77 < 78 -> falha (so confirma que o calculo usa SoH, nao
    // is_battery_dead como veto binario separado).
    ScriptedRandom rng_fail({77});
    CombatActor jaci1 = make_actor("jaci", 40);
    CombatActor ally1 = make_actor("gus", 30);
    const auto r_fail =
        resolve_healer_card_use(HardwareClass::ComumOriginal, DifficultyLevel::Medio,
                                 battery, kOriginalMedioCapacity, jaci1, ally1, rng_fail);
    REQUIRE(r_fail.failed);
    REQUIRE(r_fail.roll_fail_percent == 78);

    // Bateria JA e is_battery_dead (SoH=0, abaixo do piso 21) mas com roll ALTO (99,
    // >= fail_percent=100 e IMPOSSIVEL - fail_percent=100 sempre falha; este caso
    // prova que SoH=0 e o UNICO jeito de fail_percent chegar a 100, nao
    // is_battery_dead() per se).
    BatteryState battery_barely_dead{};
    battery_barely_dead.battery_recharge_cycles = 7;  // SoH = 100-91 = 9%
    REQUIRE(gus::domain::hardware::is_battery_dead(battery_barely_dead));
    ScriptedRandom rng_pass({90});  // fail_percent=91; roll=90 < 91 -> ainda falha aqui
    CombatActor jaci2 = make_actor("jaci", 40);
    CombatActor ally2 = make_actor("gus", 30);
    const auto r2 = resolve_healer_card_use(HardwareClass::ComumOriginal,
                                             DifficultyLevel::Medio, battery_barely_dead,
                                             kOriginalMedioCapacity, jaci2, ally2, rng_pass);
    REQUIRE(r2.roll_fail_percent == 91);  // trava o numero: dead != automatico-100
}

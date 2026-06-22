// combat_formula_property_test.cpp
//
// REFORCO DE QA (marco M5) da formula de dano UseCard (secao 11) por PROPRIEDADE + FUZZ.
// Recomendado pela auditoria AUD-M5-MOTOR-2026-06-22. Varre milhares de (carta x ator x
// kills x RNG) deterministicos pela superficie PUBLICA da FSM (begin_turn +
// run_active_turn_to_end) e afirma os INVARIANTES que valem para QUALQUER entrada:
//
//   INV-1  dano aplicado (queda de HP do alvo) e SEMPRE >= 0, em qualquer canal/seed.
//   INV-5a soma dos canais: P(FALHA)+P(CRIT)+P(COMUM) = 100 enquanto fumble+crit <= 100.
//          (fumble in [0,5], crit in [5,100]; a faixa COMUM e o resto ate 100.)
//   INV-5b contagem de consumo de RNG por canal bate sob muitos seeds:
//          FALHA=1 next(100); CRIT=1 next(100); COMUM=1 next(100)+1 next_double = 2.
//   INV-6a fumbleChance(kills) e NAO-CRESCENTE em kills; chega a 0 (>= 5 kills).
//   INV-6b a variancia v(kills) e NAO-CRESCENTE em kills; floor exato 0.05.
//
// As funcoes-modelo (fumble_model / variance_model) reproduzem a secao 11 e sao
// CONFRONTADAS com o comportamento real da FSM (via CountingRandom: contamos consumos e
// inferimos o canal escolhido pelo dano observado). NAO altera codigo de producao.
// Um caso que viole um invariante e BUG -> reportar ao lider com a seed (reproduz exato).
//
// property-based "a mao" (property_gen.hpp): PropertyRandom (LCG) varre TODOS os canais;
// CountingRandom crava o canal e conta consumos. POCO puro, ZERO Qt, headless.

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cmath>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "fixed_random.hpp"
#include "gus/domain/combat/combat_actor.hpp"
#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/combat/combat_records.hpp"
#include "gus/domain/combat/combat_state.hpp"
#include "gus/domain/combat/combat_state_machine.hpp"
#include "property_gen.hpp"

using namespace gus::domain::combat;
using gus::domain::tests::Lcg;
using gus::domain::tests::PropertyRandom;

namespace {

constexpr int kCasesPerProperty = 3000;

// Modelos da secao 11 (espelham combat_state_machine.cpp linhas 616/621-623).
int fumble_model(int kills) {
    return static_cast<int>(std::lround(5.0 * std::exp(-static_cast<double>(kills) * 0.50)));
}
float variance_model(int kills) {
    return std::max(0.05f, 0.30f * std::exp(-static_cast<float>(kills) * 0.10f));
}
int crit_model(int card_crit) { return std::max(5, card_crit); }

Card make_card(const std::string& id, CardFamily family, int power, int crit_chance) {
    Card c;
    c.id = id;
    c.display_name = id;
    c.family = family;
    c.base_type = CardBaseType::Pulso;
    c.mana_cost = 1;
    c.ap_cost = 1;
    c.power = power;
    c.target_shape = TargetShape::Single;
    c.crit_chance = crit_chance;
    return c;
}

std::unordered_map<std::string, Card> registry(const Card& c) {
    std::unordered_map<std::string, Card> d;
    d.emplace(c.id, c);
    return d;
}

CombatActionProvider play_once(CombatAction action) {
    auto done = std::make_shared<bool>(false);
    return [action, done](CombatActor& a, const CombatState&) -> CombatAction {
        if (!a.is_player_side() || *done) return CombatAction::pass();
        *done = true;
        return action;
    };
}

bool log_has(const CombatStateMachine& sm, const std::string& needle) {
    for (const auto& e : sm.log())
        if (e.message.find(needle) != std::string::npos)
            return true;
    return false;
}

// RNG deterministico que conta consumos por tipo e crava um canal via next_int.
class CountingRandom final : public IRandomSource {
public:
    CountingRandom(double nd, int ni) : nd_(nd), ni_(ni) {}
    double next_double() override {
        ++double_calls;
        return nd_;
    }
    int next(int max_value) override {
        ++int_calls;
        return max_value <= 0 ? 0 : std::min(ni_, max_value - 1);
    }
    int int_calls = 0;
    int double_calls = 0;

private:
    double nd_;
    int ni_;
};

}  // namespace

// ===== INV-1: dano end-to-end nunca negativo, em qualquer canal/seed =====

TEST_CASE("property: dano de UseCard nunca e negativo sob qualquer carta/ator/RNG",
          "[domain][combat][property][formula]") {
    for (int c = 0; c < kCasesPerProperty; ++c) {
        Lcg g(0xF1000000u + static_cast<unsigned>(c));

        const int hp = g.in_range(1, 800);
        const int atk = g.in_range(0, 60);
        const int def_t = g.in_range(0, 200);
        const auto fam_a = static_cast<CardFamily>(g.in_range(0, 4));
        const auto fam_t = static_cast<CardFamily>(g.in_range(0, 4));
        const int kills = g.in_range(0, 50);
        const int power = g.in_range(0, 120);
        const int crit = g.in_range(0, 100);

        CombatActor hero("gus", "gus", 50, atk, 2, 20, fam_a, /*player=*/true);
        CombatActor foe("enemy", "enemy", hp, 6, def_t, 10, fam_t, /*player=*/false,
                        /*boss=*/false, kills);
        Card card = make_card("c", fam_a, power, crit);
        auto reg = registry(card);

        PropertyRandom rng(0x1234u + static_cast<unsigned>(c));
        CombatStateMachine sm({&hero, &foe}, play_once(CombatAction::use_card(card.id, foe.id())),
                              &reg, nullptr, &rng);
        sm.begin_turn();
        sm.run_active_turn_to_end();

        const int dmg = foe.max_hp() - foe.hp();
        // INV-1: dano aplicado nunca negativo; alvo nunca curado por um ataque.
        REQUIRE(dmg >= 0);
        REQUIRE(foe.hp() >= 0);
        REQUIRE(foe.hp() <= foe.max_hp());
    }
}

// ===== INV-5a: P(FALHA) + P(CRIT) + P(COMUM) = 100 =====

TEST_CASE("property: as 3 probabilidades de canal somam 100 enquanto fumble+crit <= 100",
          "[domain][combat][property][formula]") {
    for (int c = 0; c < 2000; ++c) {
        Lcg g(0xF2000000u + static_cast<unsigned>(c));
        const int kills = g.in_range(0, 40);
        const int card_crit = g.in_range(0, 95);

        const int fumble = fumble_model(kills);    // 0..5
        const int crit = crit_model(card_crit);    // 5..100

        REQUIRE(fumble >= 0);
        REQUIRE(fumble <= 5);
        REQUIRE(crit >= 5);
        REQUIRE(crit <= 100);

        // Faixas contiguas (secao 11): FALHA [0,fumble), CRIT [fumble, fumble+crit),
        // COMUM [fumble+crit, 100). Soma das larguras = 100 quando fumble+crit <= 100.
        if (fumble + crit <= 100) {
            const int p_falha = fumble;
            const int p_crit = crit;
            const int p_comum = 100 - fumble - crit;
            REQUIRE(p_comum >= 0);
            REQUIRE(p_falha + p_crit + p_comum == 100);
        }
    }
}

// INV-5a confrontado com a FSM: para CADA roll em [0,99], o canal observado (dano==0 +
// log FALHA / log CRITICO / senao COMUM) coincide com a particao de faixas do modelo.
TEST_CASE("property: o canal escolhido pela FSM coincide com as faixas do modelo para todo roll",
          "[domain][combat][property][formula]") {
    for (int c = 0; c < 400; ++c) {
        Lcg g(0xF3000000u + static_cast<unsigned>(c));
        const int kills = g.in_range(0, 8);
        const int card_crit = g.in_range(0, 60);
        const int fumble = fumble_model(kills);
        const int crit = crit_model(card_crit);

        for (int roll = 0; roll < 100; ++roll) {
            CombatActor hero("gus", "gus", 50, 0, 2, 20, CardFamily::Eletrico, /*player=*/true);
            // power alto + def 0 garante dano COMUM > 0 (distingue COMUM de FALHA por dano).
            CombatActor foe("enemy", "enemy", 100000, 6, 0, 10, CardFamily::Eletrico,
                            /*player=*/false, /*boss=*/false, kills);
            Card card = make_card("c", CardFamily::Eletrico, 200, card_crit);
            auto reg = registry(card);
            // nd=0.5 => variancia ZERO no COMUM (dano COMUM == base_damage, > 0 aqui).
            gus::domain::tests::FixedRandom rng(0.5, roll);
            CombatStateMachine sm({&hero, &foe},
                                  play_once(CombatAction::use_card(card.id, foe.id())), &reg,
                                  nullptr, &rng);
            sm.begin_turn();
            sm.run_active_turn_to_end();

            const bool is_falha = log_has(sm, "FALHA DE COMPILACAO");
            const bool is_crit = log_has(sm, "[CRITICO]");
            const bool is_comum = !is_falha && !is_crit;

            if (roll < fumble) {
                REQUIRE(is_falha);
            } else if (roll < fumble + crit) {
                REQUIRE(is_crit);
            } else {
                REQUIRE(is_comum);
            }
        }
    }
}

// ===== INV-5b: contagem de consumo de RNG por canal =====

TEST_CASE("property: consumo de RNG por canal e 1/1/2 (FALHA/CRIT/COMUM) sob muitos casos",
          "[domain][combat][property][formula]") {
    for (int c = 0; c < 1500; ++c) {
        Lcg g(0xF4000000u + static_cast<unsigned>(c));
        const int kills = g.in_range(0, 6);
        const int card_crit = g.in_range(0, 60);
        const int fumble = fumble_model(kills);
        const int crit = crit_model(card_crit);
        const int roll = g.in_range(0, 99);

        CombatActor hero("gus", "gus", 50, 0, 2, 20, CardFamily::Eletrico, /*player=*/true);
        CombatActor foe("enemy", "enemy", 100000, 6, 0, 10, CardFamily::Eletrico,
                        /*player=*/false, /*boss=*/false, kills);
        Card card = make_card("c", CardFamily::Eletrico, 200, card_crit);
        auto reg = registry(card);
        CountingRandom rng(0.5, roll);
        CombatStateMachine sm({&hero, &foe}, play_once(CombatAction::use_card(card.id, foe.id())),
                              &reg, nullptr, &rng);
        sm.begin_turn();
        sm.run_active_turn_to_end();

        // SEMPRE exatamente 1 sorteio de canal (next(100)).
        REQUIRE(rng.int_calls == 1);

        if (roll < fumble) {
            // FALHA: 0 next_double.
            REQUIRE(rng.double_calls == 0);
        } else if (roll < fumble + crit) {
            // CRIT: 0 next_double.
            REQUIRE(rng.double_calls == 0);
        } else {
            // COMUM: exatamente 1 next_double (total 2 consumos).
            REQUIRE(rng.double_calls == 1);
        }
    }
}

// ===== INV-6a: fumbleChance NAO-CRESCENTE em kills; chega a 0 =====

TEST_CASE("property: fumbleChance(kills) e nao-crescente e atinge 0 a partir de 5 kills",
          "[domain][combat][property][formula]") {
    int prev = fumble_model(0);
    REQUIRE(prev == 5);  // kills=0 => 5%
    for (int kills = 1; kills <= 200; ++kills) {
        const int cur = fumble_model(kills);
        // INV-6a: monotonia nao-crescente.
        REQUIRE(cur <= prev);
        REQUIRE(cur >= 0);
        prev = cur;
    }
    // INV-6a: zera (e segue zerado).
    for (int kills = 5; kills <= 200; ++kills)
        REQUIRE(fumble_model(kills) == 0);
}

// ===== INV-6b: variancia NAO-CRESCENTE em kills; floor exato 0.05 =====

TEST_CASE("property: v(kills) e nao-crescente e converge para o floor 0.05",
          "[domain][combat][property][formula]") {
    float prev = variance_model(0);
    REQUIRE(prev == 0.30f);  // kills=0 => 30%
    for (int kills = 1; kills <= 1000; ++kills) {
        const float cur = variance_model(kills);
        // INV-6b: nao-crescente e nunca abaixo do floor.
        REQUIRE(cur <= prev + 1e-7f);
        REQUIRE(cur >= 0.05f);
        prev = cur;
    }
    // Floor exato 0.05 atingido para kills muito altos.
    REQUIRE(variance_model(1000) == 0.05f);
}

// INV-6 confrontado com a FSM: spread de dano (topo - fundo no COMUM) e nao-crescente em
// kills, refletindo a variancia decrescente. Comparamos kills crescentes em pares.
TEST_CASE("property: o spread de dano observado encolhe (ou estabiliza) conforme kills sobem",
          "[domain][combat][property][formula]") {
    auto spread_for = [](int kills) {
        auto roll = [&](double nd) {
            CombatActor hero("gus", "gus", 50, 0, 2, 20, CardFamily::Eletrico, /*player=*/true);
            CombatActor foe("enemy", "enemy", 1000000, 6, 0, 10, CardFamily::Eletrico,
                            /*player=*/false, /*boss=*/false, kills);
            Card card = make_card("c", CardFamily::Eletrico, 1000, /*crit=*/0);
            auto reg = registry(card);
            // next_int=99 garante canal COMUM (fora de FALHA/CRIT). nd varia a variancia.
            gus::domain::tests::FixedRandom rng(nd, 99);
            CombatStateMachine sm({&hero, &foe},
                                  play_once(CombatAction::use_card(card.id, foe.id())), &reg,
                                  nullptr, &rng);
            sm.begin_turn();
            sm.run_active_turn_to_end();
            return foe.max_hp() - foe.hp();
        };
        return roll(1.0) - roll(0.0);  // topo - fundo
    };

    int prev = spread_for(0);
    for (int kills = 1; kills <= 60; ++kills) {
        const int cur = spread_for(kills);
        REQUIRE(cur <= prev);  // spread nao cresce com mais maestria
        REQUIRE(cur >= 0);
        prev = cur;
    }
}

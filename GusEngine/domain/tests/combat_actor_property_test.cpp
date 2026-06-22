// combat_actor_property_test.cpp
//
// REFORCO DE QA (nao-bloqueante) do CombatActor (marco M5) por testes de PROPRIEDADE +
// FUZZ. Recomendado pela auditoria AUD-M5-MOTOR-2026-06-22 (0 criticos/0 importantes).
// Em vez de casos pontuais, varremos MILHARES de sequencias aleatorias deterministicas
// de take_damage/heal/reduce_def/spend_ap/spend_mana/status e afirmamos os INVARIANTES de
// estado que DEVEM valer para QUALQUER historico de acoes:
//
//   INV-2  HP sempre em [0, max_hp] apos qualquer sequencia.
//   INV-1  dano efetivo (queda de HP) nunca negativo (HP e monotono nao-crescente sob dano).
//   INV-3  Shield absorve no maximo o pool; pool nunca negativo; some quando zera.
//   INV-4  AP gasto <= AP disponivel; AP nunca negativo; spend insuficiente => logic_error.
//          Mana idem (mesma porta de recurso).
//
// property-based "a mao" (property_gen.hpp): LCG semeado por indice de caso; sem RNG real.
// NAO altera codigo de producao. Um caso que FALHE aqui e um BUG do motor -> reportar ao
// lider com o input minimo (a seed reproduz exatamente), NAO consertar o motor neste PR.
//
// Subsistema: domain/combat (CombatActor). POCO puro, ZERO Qt, headless.

#include <catch2/catch_test_macros.hpp>

#include <stdexcept>
#include <string>

#include "gus/domain/combat/combat_actor.hpp"
#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/combat/combat_records.hpp"
#include "property_gen.hpp"

using namespace gus::domain::combat;
using gus::domain::tests::Lcg;

namespace {

constexpr int kCasesPerProperty = 1500;
constexpr int kStepsPerCase = 40;

CombatActor make_actor(Lcg& g) {
    const int max_hp = g.in_range(1, 999);
    const int atk = g.in_range(0, 99);
    const int def = g.in_range(0, 99);
    const int spd = g.in_range(0, 99);
    const auto family = static_cast<CardFamily>(g.in_range(0, 4));
    return CombatActor("a", "a", max_hp, atk, def, spd, family, /*player=*/true);
}

}  // namespace

// ===== INV-2 / INV-1: HP em [0, max_hp]; dano nunca aumenta HP =====

TEST_CASE("property: HP fica sempre em [0, max_hp] sob qualquer sequencia de dano/cura/shield",
          "[domain][combat][property][actor]") {
    for (int c = 0; c < kCasesPerProperty; ++c) {
        Lcg g(0xA5000000u + static_cast<unsigned>(c));
        CombatActor a = make_actor(g);
        const int max_hp = a.max_hp();

        for (int step = 0; step < kStepsPerCase; ++step) {
            const int op = g.in_range(0, 3);
            switch (op) {
                case 0:
                    a.take_damage(g.in_range(0, max_hp * 3));
                    break;
                case 1:
                    a.heal(g.in_range(0, max_hp * 3));
                    break;
                case 2: {
                    // Aplica um Shield com pool aleatorio (pode absorver dano subsequente).
                    StatusEffect sh;
                    sh.id = StatusId::Shield;
                    sh.magnitude = g.in_range(0, 500);
                    sh.duration = g.in_range(1, 5);
                    a.add_status(sh);
                    break;
                }
                default:
                    a.reduce_def(g.in_range(0, 99));
                    break;
            }

            // INV-2: HP nunca sai da faixa, em NENHUM ponto da sequencia.
            REQUIRE(a.hp() >= 0);
            REQUIRE(a.hp() <= max_hp);
            // is_alive coerente com HP.
            REQUIRE(a.is_alive() == (a.hp() > 0));
            // Def nunca negativa (reduce_def faz clamp em 0).
            REQUIRE(a.def() >= 0);
        }
    }
}

TEST_CASE("property: take_damage nunca AUMENTA o HP (dano efetivo >= 0)",
          "[domain][combat][property][actor]") {
    for (int c = 0; c < kCasesPerProperty; ++c) {
        Lcg g(0xA6000000u + static_cast<unsigned>(c));
        CombatActor a = make_actor(g);

        for (int step = 0; step < kStepsPerCase; ++step) {
            const int before = a.hp();
            a.take_damage(g.in_range(0, a.max_hp() * 2));
            // INV-1: HP so pode cair ou ficar igual; queda (dano efetivo) nunca negativa.
            REQUIRE(a.hp() <= before);
            REQUIRE((before - a.hp()) >= 0);
        }
    }
}

// ===== INV-3: Shield absorve no maximo o pool; pool nunca negativo =====

TEST_CASE("property: Shield absorve no maximo o pool e nunca fica com pool negativo",
          "[domain][combat][property][actor][shield]") {
    for (int c = 0; c < kCasesPerProperty; ++c) {
        Lcg g(0xA7000000u + static_cast<unsigned>(c));
        CombatActor a = make_actor(g);

        for (int step = 0; step < kStepsPerCase; ++step) {
            // (Re)aplica um Shield aleatorio de tempos em tempos.
            if (g.coin()) {
                StatusEffect sh;
                sh.id = StatusId::Shield;
                sh.magnitude = g.in_range(0, 400);
                sh.duration = g.in_range(1, 6);
                a.add_status(sh);
            }

            // Estado pre-dano.
            const int hp_before = a.hp();
            int shield_before = 0;
            const int idx_before = a.index_of_status(StatusId::Shield);
            if (idx_before >= 0)
                shield_before = a.status_effects()[static_cast<std::size_t>(idx_before)].magnitude;

            const int dmg = g.in_range(0, 600);
            a.take_damage(dmg);

            // Pool de Shield resultante (0 se expirou/ausente).
            const int idx_after = a.index_of_status(StatusId::Shield);
            int shield_after = 0;
            if (idx_after >= 0)
                shield_after = a.status_effects()[static_cast<std::size_t>(idx_after)].magnitude;

            // INV-3a: pool nunca negativo.
            REQUIRE(shield_before >= 0);
            REQUIRE(shield_after >= 0);

            // INV-3b: o que o shield absorveu = quanto o pool caiu; <= dano e <= pool anterior.
            const int absorbed = shield_before - shield_after;
            REQUIRE(absorbed >= 0);
            REQUIRE(absorbed <= dmg);
            REQUIRE(absorbed <= shield_before);

            // INV-3c: dano que chegou ao HP = dano total - absorvido (com clamp em 0 no HP).
            const int hp_loss = hp_before - a.hp();
            REQUIRE(hp_loss >= 0);
            REQUIRE(hp_loss <= (dmg - absorbed));
        }
    }
}

// ===== INV-4: AP/Mana: gasto <= disponivel; recurso nunca negativo =====

TEST_CASE("property: spend_ap nunca gasta mais que o disponivel; AP nunca fica negativo",
          "[domain][combat][property][actor][resource]") {
    for (int c = 0; c < kCasesPerProperty; ++c) {
        Lcg g(0xA8000000u + static_cast<unsigned>(c));
        CombatActor a = make_actor(g);

        for (int step = 0; step < kStepsPerCase; ++step) {
            if (g.coin())
                a.refresh_resources_for_turn(g.in_range(0, 30));

            const int ap_before = a.ap();
            const int cost = g.in_range(0, a.max_ap() + 2);

            if (cost > ap_before) {
                // INV-4: gasto > disponivel DEVE falhar (logic_error) e NAO mexer no AP.
                REQUIRE_THROWS_AS(a.spend_ap(cost), std::logic_error);
                REQUIRE(a.ap() == ap_before);
            } else {
                a.spend_ap(cost);
                REQUIRE(a.ap() == ap_before - cost);
            }
            // AP nunca negativo, em nenhum ponto.
            REQUIRE(a.ap() >= 0);
        }
    }
}

TEST_CASE("property: spend_mana nunca gasta mais que o disponivel; mana nunca fica negativa",
          "[domain][combat][property][actor][resource]") {
    for (int c = 0; c < kCasesPerProperty; ++c) {
        Lcg g(0xA9000000u + static_cast<unsigned>(c));
        CombatActor a = make_actor(g);

        for (int step = 0; step < kStepsPerCase; ++step) {
            if (g.coin())
                a.refresh_resources_for_turn(g.in_range(0, 30));

            const int mana_before = a.mana();
            const int cost = g.in_range(0, a.max_mana() + 3);

            if (cost > mana_before) {
                REQUIRE_THROWS_AS(a.spend_mana(cost), std::logic_error);
                REQUIRE(a.mana() == mana_before);
            } else {
                a.spend_mana(cost);
                REQUIRE(a.mana() == mana_before - cost);
            }
            REQUIRE(a.mana() >= 0);
            // Mana nunca passa do max do turno corrente.
            REQUIRE(a.mana() <= a.max_mana());
        }
    }
}

// ===== Inputs invalidos: dano/cura/reducao negativos sao erro de chamador =====

TEST_CASE("property: take_damage/heal/reduce_def negativos lancam out_of_range sem mutar estado",
          "[domain][combat][property][actor]") {
    for (int c = 0; c < 500; ++c) {
        Lcg g(0xAA000000u + static_cast<unsigned>(c));
        CombatActor a = make_actor(g);
        const int hp_before = a.hp();
        const int def_before = a.def();
        const int neg = -g.in_range(1, 1000);

        REQUIRE_THROWS_AS(a.take_damage(neg), std::out_of_range);
        REQUIRE_THROWS_AS(a.heal(neg), std::out_of_range);
        REQUIRE_THROWS_AS(a.reduce_def(neg), std::out_of_range);

        REQUIRE(a.hp() == hp_before);
        REQUIRE(a.def() == def_before);
    }
}

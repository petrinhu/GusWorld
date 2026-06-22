// universal_compiler_defense_test.cpp
//
// Spec executavel (Catch2 v3) da defesa NEUTRA do compilador universal (criador
// 2026-06-03, secao 6.1): alvo universal (Gus) => mult_fraqueza 1.0 em qualquer familia
// atacante; companions/inimigos seguem a roda normal. Portado de
// engine/tests/turn_combat/UniversalCompilerDefenseTests.cs. POCO puro, ZERO Qt.
//
// RNG cravado (FixedRandom 0.5) zera a variancia => dano == round(base).
//
// Cross-ref: engine/tests/turn_combat/UniversalCompilerDefenseTests.cs;
//            docs/design/mecanicas/combat.md secao 6/6.1/11.

#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "fixed_random.hpp"
#include "gus/domain/combat/combat_actor.hpp"
#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/combat/combat_records.hpp"
#include "gus/domain/combat/combat_state.hpp"
#include "gus/domain/combat/combat_state_machine.hpp"

using namespace gus::domain::combat;
using gus::domain::tests::FixedRandom;

namespace {

// Alvo do ataque. universal=true marca defesa neutra (Gus). Atk/Def 0 isola a formula no
// mult_fraqueza; family define a relacao com a carta atacante.
CombatActor target(CardFamily family, bool universal) {
    return CombatActor("alvo", "alvo", 500, 0, 0, 10, family, /*player=*/false, /*boss=*/false,
                       /*kills=*/0, universal);
}

CombatActor attacker() {
    return CombatActor("atacante", "atacante", 50, 0, 2, 20, CardFamily::Eletrico,
                       /*player=*/true);
}

Card make_card(CardFamily family, int power = 10) {
    Card c;
    c.id = "pulso." + std::to_string(static_cast<int>(family));
    c.display_name = c.id;
    c.family = family;
    c.base_type = CardBaseType::Pulso;
    c.mana_cost = 1;
    c.ap_cost = 1;
    c.power = power;
    c.target_shape = TargetShape::Single;
    return c;
}

CombatActionProvider play_once(CombatAction action) {
    auto done = std::make_shared<bool>(false);
    return [action, done](CombatActor& a, const CombatState&) -> CombatAction {
        if (!a.is_player_side() || *done) return CombatAction::pass();
        *done = true;
        return action;
    };
}

int damage_against(CombatActor& tgt, const Card& card) {
    CombatActor atk = attacker();
    std::unordered_map<std::string, Card> reg;
    reg.emplace(card.id, card);
    FixedRandom rng;  // default (0.5, 99): canal COMUM, variancia zero (secao 11)
    CombatStateMachine sm({&atk, &tgt}, play_once(CombatAction::use_card(card.id, tgt.id())),
                          &reg, nullptr, &rng);
    sm.begin_turn();
    sm.run_active_turn_to_end();
    return tgt.max_hp() - tgt.hp();
}

}  // namespace

// Alvo family=Cinetico expoe as tres relacoes da roda; universal forca todas a 1.0 => 10.
TEST_CASE("universal: alvo universal recebe mult 1.0 independente da familia atacante",
          "[domain][combat][universal]") {
    for (const CardFamily atk_family :
         {CardFamily::Eletrico, CardFamily::Criptografico, CardFamily::Cinetico}) {
        CombatActor alvo = target(CardFamily::Cinetico, /*universal=*/true);
        const Card carta = make_card(atk_family, 10);
        REQUIRE(damage_against(alvo, carta) == 10);
    }
}

// Regressao: alvo normal segue a roda de fraqueza.
TEST_CASE("universal: alvo normal segue a roda de fraqueza", "[domain][combat][universal]") {
    struct Case {
        CardFamily atk_family;
        int expected;
    };
    const Case cases[] = {
        {CardFamily::Eletrico, 15},       // Fraco 1.5
        {CardFamily::Criptografico, 7},   // Resistente 0.66 -> round(6.6)
        {CardFamily::Cinetico, 10},       // Neutro 1.0
    };
    for (const auto& c : cases) {
        CombatActor alvo = target(CardFamily::Cinetico, /*universal=*/false);
        const Card carta = make_card(c.atk_family, 10);
        REQUIRE(damage_against(alvo, carta) == c.expected);
    }
}

// backdoor_signal_test.cpp
//
// Spec executavel (Catch2 v3) da fatia C da onda CARDS-HW-2 (CARDS-HW-2C, Backdoor
// SIGNAL-ONLY): docs/design/mecanicas/cartas-spec-logica.md secao 4.2. Cobre SO o sinal
// `CombatState::leaked_intel()`, populado pela CombatStateMachine a partir do
// integrity_ledger (owner_actor_id de toda carta infectada com VirusKind::Backdoor).
//
// POR QUE SO O SINAL (nao a ponderacao): o UtilityBrain que ponderaria o vies (secao 4.2,
// `BACKDOOR_TARGETING_BIAS`) NAO existe ainda - so ha ScriptedBrain, deterministico, que
// nao pontua utilidade e ignora leaked_intel por completo (balance_harness.hpp: "UtilityBrain
// e jogo posterior"). Implementar a ponderacao agora seria codigo MORTO sem consumidor.
//
// Cobertura:
//   - leaked_intel populado com o owner_actor_id certo quando ha 1 carta Backdoor-infectada;
//   - vazio quando nao ha infeccao Backdoor no ledger;
//   - multiplas cartas Backdoor de aliados DIFERENTES -> multiplos ids;
//   - carta infectada com OUTRO virus (Worm) NAO entra no leaked_intel;
//   - ScriptedBrain produz decisao BYTE-IDENTICA com e sem o sinal (telegraph honesto -
//     o sinal existe, mas ninguem age nele ainda);
//   - ledger nulo -> leaked_intel vazio (fail-safe, mesmo padrao dos demais payloads);
//   - Worm mid-combat (infect_with_worm) seta VirusKind::Worm, NUNCA Backdoor - a
//     propagacao do worm nao "poluiu" o sinal de spyware.
//
// Cross-ref: gus/domain/combat/combat_state.hpp (leaked_intel);
//            gus/domain/combat/combat_state_machine.cpp (collect_leaked_intel,
//            dispatch_virus_payload_pre_cast case Backdoor);
//            gus/domain/combat/scripted_brain.cpp; card_virus_combat_test.cpp (padrao).

#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "fixed_random.hpp"
#include "gus/domain/combat/card_integrity_ledger.hpp"
#include "gus/domain/combat/combat_actor.hpp"
#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/combat/combat_records.hpp"
#include "gus/domain/combat/combat_state.hpp"
#include "gus/domain/combat/combat_state_machine.hpp"
#include "gus/domain/combat/enemy_brain.hpp"
#include "gus/domain/combat/random_source.hpp"
#include "gus/domain/combat/scripted_brain.hpp"
#include "gus/domain/infection/integrity_state.hpp"

using namespace gus::domain::combat;
using gus::domain::infection::IntegrityState;
using gus::domain::infection::VirusKind;
using gus::domain::tests::FixedRandom;

namespace {

CombatActor make_actor(const std::string& id, bool player_side, int hp = 100, int atk = 0,
                       int def = 0, int spd = 20, CardFamily family = CardFamily::Eletrico) {
    return CombatActor(id, id, hp, atk, def, spd, family, player_side);
}

Card damage_card(const std::string& id, int power = 0, CardTier tier = CardTier::Comum) {
    Card c;
    c.id = id;
    c.display_name = id;
    c.family = CardFamily::Cinetico;
    c.base_type = CardBaseType::Pulso;
    c.mana_cost = 0;
    c.ap_cost = 1;
    c.power = power;
    c.target_shape = TargetShape::Single;
    c.tier = tier;
    return c;
}

std::unordered_map<std::string, Card> registry(std::initializer_list<Card> cards) {
    std::unordered_map<std::string, Card> d;
    for (const auto& c : cards) d.emplace(c.id, c);
    return d;
}

// Provider que captura o leaked_intel() do PRIMEIRO CombatState visto (qualquer lado) e
// sempre passa (0 AP) - so quer observar o sinal, nunca disputa o pipeline de acoes.
CombatActionProvider capture_and_pass(std::vector<int>* out_leaked, bool* out_captured) {
    return [out_leaked, out_captured](CombatActor&, const CombatState& state) -> CombatAction {
        if (!*out_captured) {
            *out_leaked = state.leaked_intel();
            *out_captured = true;
        }
        return CombatAction::pass();
    };
}

}  // namespace

// ===== leaked_intel: populacao basica ==================================================

TEST_CASE("backdoor signal: leaked_intel populado com o owner_actor_id certo quando ha "
         "1 carta Backdoor-infectada no ledger",
         "[domain][combat][virus][backdoor][signal]") {
    CombatActor caster = make_actor("h", true, /*hp=*/100, /*atk=*/0, /*def=*/0, /*spd=*/50);
    CombatActor filler = make_actor("e", false, /*hp=*/100, /*atk=*/0, /*def=*/0, /*spd=*/10);

    Card plain = damage_card("backdoor.card.a");
    auto reg = registry({plain});

    IntegrityState spy;
    spy.is_infected = true;
    spy.virus_kind = VirusKind::Backdoor;
    std::vector<CardIntegrityRef> ledger = {
        CardIntegrityRef{/*instance_id=*/1, plain.id, /*owner_actor_id=*/7, &spy}};

    std::vector<int> leaked;
    bool captured = false;
    CombatStateMachine sm({&caster, &filler}, capture_and_pass(&leaked, &captured), &reg,
                          nullptr, nullptr, &ledger);

    sm.begin_turn();
    sm.run_active_turn_to_end();

    REQUIRE(captured);
    REQUIRE(leaked == std::vector<int>{7});
}

TEST_CASE("backdoor signal: leaked_intel VAZIO quando nao ha infeccao Backdoor no ledger",
         "[domain][combat][virus][backdoor][signal]") {
    CombatActor caster = make_actor("h", true, /*hp=*/100, /*atk=*/0, /*def=*/0, /*spd=*/50);
    CombatActor filler = make_actor("e", false, /*hp=*/100, /*atk=*/0, /*def=*/0, /*spd=*/10);

    Card plain = damage_card("backdoor.card.clean");
    auto reg = registry({plain});

    IntegrityState clean;  // sem infeccao (default).
    std::vector<CardIntegrityRef> ledger = {
        CardIntegrityRef{/*instance_id=*/1, plain.id, /*owner_actor_id=*/7, &clean}};

    std::vector<int> leaked;
    bool captured = false;
    CombatStateMachine sm({&caster, &filler}, capture_and_pass(&leaked, &captured), &reg,
                          nullptr, nullptr, &ledger);

    sm.begin_turn();
    sm.run_active_turn_to_end();

    REQUIRE(captured);
    REQUIRE(leaked.empty());
}

TEST_CASE("backdoor signal: multiplas cartas Backdoor de aliados DIFERENTES -> multiplos ids",
         "[domain][combat][virus][backdoor][signal]") {
    CombatActor caster = make_actor("h", true, /*hp=*/100, /*atk=*/0, /*def=*/0, /*spd=*/50);
    CombatActor filler = make_actor("e", false, /*hp=*/100, /*atk=*/0, /*def=*/0, /*spd=*/10);

    Card card_a = damage_card("backdoor.card.multi.a");
    Card card_b = damage_card("backdoor.card.multi.b");
    auto reg = registry({card_a, card_b});

    IntegrityState spy_a;
    spy_a.is_infected = true;
    spy_a.virus_kind = VirusKind::Backdoor;
    IntegrityState spy_b;
    spy_b.is_infected = true;
    spy_b.virus_kind = VirusKind::Backdoor;
    std::vector<CardIntegrityRef> ledger = {
        CardIntegrityRef{/*instance_id=*/1, card_a.id, /*owner_actor_id=*/7, &spy_a},
        CardIntegrityRef{/*instance_id=*/2, card_b.id, /*owner_actor_id=*/9, &spy_b}};

    std::vector<int> leaked;
    bool captured = false;
    CombatStateMachine sm({&caster, &filler}, capture_and_pass(&leaked, &captured), &reg,
                          nullptr, nullptr, &ledger);

    sm.begin_turn();
    sm.run_active_turn_to_end();

    REQUIRE(captured);
    REQUIRE(leaked.size() == 2);
    REQUIRE(leaked[0] == 7);
    REQUIRE(leaked[1] == 9);
}

TEST_CASE("backdoor signal: carta infectada com OUTRO virus (Worm) NAO entra no leaked_intel",
         "[domain][combat][virus][backdoor][signal]") {
    CombatActor caster = make_actor("h", true, /*hp=*/100, /*atk=*/0, /*def=*/0, /*spd=*/50);
    CombatActor filler = make_actor("e", false, /*hp=*/100, /*atk=*/0, /*def=*/0, /*spd=*/10);

    Card worm_card = damage_card("backdoor.card.notworm");
    auto reg = registry({worm_card});

    IntegrityState worm;
    worm.is_infected = true;
    worm.virus_kind = VirusKind::Worm;
    std::vector<CardIntegrityRef> ledger = {
        CardIntegrityRef{/*instance_id=*/1, worm_card.id, /*owner_actor_id=*/7, &worm}};

    std::vector<int> leaked;
    bool captured = false;
    CombatStateMachine sm({&caster, &filler}, capture_and_pass(&leaked, &captured), &reg,
                          nullptr, nullptr, &ledger);

    sm.begin_turn();
    sm.run_active_turn_to_end();

    REQUIRE(captured);
    REQUIRE(leaked.empty());
}

// ===== ledger nulo: fail-safe ===========================================================

TEST_CASE("backdoor signal: ledger nulo (default, todo call site pre-existente) -> "
         "leaked_intel vazio",
         "[domain][combat][virus][backdoor][signal][regression]") {
    CombatActor caster = make_actor("h", true, /*hp=*/100, /*atk=*/0, /*def=*/0, /*spd=*/50);
    CombatActor filler = make_actor("e", false, /*hp=*/100, /*atk=*/0, /*def=*/0, /*spd=*/10);

    Card plain = damage_card("backdoor.card.noledger");
    auto reg = registry({plain});

    std::vector<int> leaked{-1};  // sentinela pra provar que foi sobrescrito por vazio.
    bool captured = false;
    CombatStateMachine sm({&caster, &filler}, capture_and_pass(&leaked, &captured), &reg);
    // sem integrity_ledger (default nullptr).

    sm.begin_turn();
    sm.run_active_turn_to_end();

    REQUIRE(captured);
    REQUIRE(leaked.empty());
}

// ===== ScriptedBrain: determinismo (telegraph honesto) ==================================

TEST_CASE("backdoor signal: ScriptedBrain produz decisao BYTE-IDENTICA com e sem o sinal - "
         "o sinal existe, mas ninguem pondera nele ainda (UtilityBrain e onda futura)",
         "[domain][combat][virus][backdoor][signal][determinism]") {
    CombatActor enemy = make_actor("trash", false, /*hp=*/100, /*atk=*/10, /*def=*/0,
                                  /*spd=*/50);
    CombatActor victim = make_actor("gus", true, /*hp=*/100, /*atk=*/0, /*def=*/0, /*spd=*/10);
    CombatActor other = make_actor("iara", true, /*hp=*/100, /*atk=*/0, /*def=*/0, /*spd=*/5);

    InitiativeQueue queue({&enemy, &victim, &other});
    ScriptedBrain brain;

    const CombatState state_no_signal(queue, &enemy, /*round_index=*/0, /*card_registry=*/nullptr,
                                      /*leaked_intel=*/{});
    const CombatAction action_no_signal = brain.decide_action(state_no_signal, enemy);
    const IntentPreview preview_no_signal = brain.preview_intent(state_no_signal, enemy);

    const CombatState state_with_signal(queue, &enemy, /*round_index=*/0,
                                        /*card_registry=*/nullptr,
                                        /*leaked_intel=*/std::vector<int>{99});
    const CombatAction action_with_signal = brain.decide_action(state_with_signal, enemy);
    const IntentPreview preview_with_signal = brain.preview_intent(state_with_signal, enemy);

    REQUIRE(action_no_signal == action_with_signal);
    REQUIRE(preview_no_signal == preview_with_signal);
    REQUIRE(action_no_signal.target_id == victim.id());  // 1o player vivo, sempre - sem vies.
}

// ===== Worm mid-combat: infect() seta Worm, NUNCA Backdoor ==============================

TEST_CASE("backdoor signal: worm mid-combat (propagacao) seta VirusKind::Worm na instancia "
         "alvo - NUNCA Backdoor - a propagacao do worm nao polui o sinal de spyware",
         "[domain][combat][virus][backdoor][signal][worm]") {
    CombatActor caster = make_actor("h", true, /*hp=*/100, /*atk=*/0, /*def=*/0, /*spd=*/50);
    CombatActor filler = make_actor("e", false, /*hp=*/100, /*atk=*/0, /*def=*/0, /*spd=*/10);

    Card worm_card = damage_card("backdoor.worm.source");
    Card candidate_card = damage_card("backdoor.worm.candidate");
    auto reg = registry({worm_card, candidate_card});

    IntegrityState source_state;
    source_state.is_infected = true;
    source_state.virus_kind = VirusKind::Worm;
    IntegrityState candidate_state;  // ainda limpa.

    std::vector<CardIntegrityRef> ledger = {
        CardIntegrityRef{/*instance_id=*/1, worm_card.id, /*owner_actor_id=*/1, &source_state},
        CardIntegrityRef{/*instance_id=*/2, candidate_card.id, /*owner_actor_id=*/2,
                         &candidate_state}};

    CombatAction cast = CombatAction::use_card(worm_card.id, filler.id());
    cast.card_instance_id = 1;

    auto acts = std::make_shared<std::vector<CombatAction>>(std::vector<CombatAction>{cast});
    auto idx = std::make_shared<std::size_t>(0);
    std::vector<int> leaked_after_propagation;
    bool captured_after = false;
    auto provider = [acts, idx, &leaked_after_propagation, &captured_after](
                        CombatActor& a, const CombatState& state) -> CombatAction {
        if (a.is_player_side() && *idx < acts->size()) return (*acts)[(*idx)++];
        // Depois da propagacao (ja disparada dentro de resolve_action do cast acima),
        // captura o leaked_intel visto na PROXIMA chamada ao provider - seja ainda no
        // proprio turno do caster (se sobrar AP) ou no turno seguinte.
        if (!captured_after) {
            leaked_after_propagation = state.leaked_intel();
            captured_after = true;
        }
        return CombatAction::pass();
    };

    FixedRandom rng(/*next_double=*/0.5, /*next_int=*/0);  // chance 0<13%; direcao 0=EnemyDeck.
    CombatStateMachine sm({&caster, &filler}, provider, &reg, nullptr, &rng, &ledger);

    sm.begin_turn();
    sm.run_active_turn_to_end();

    REQUIRE(candidate_state.is_infected);
    REQUIRE(candidate_state.virus_kind == VirusKind::Worm);  // NUNCA Backdoor.
    REQUIRE_NOTHROW(candidate_state.validate());

    // O ledger inteiro (fonte Worm + candidato recem-infectado com Worm) nao contribui NADA
    // pro leaked_intel - nenhuma entrada e Backdoor.
    sm.advance_to_next_actor();
    sm.begin_turn();
    sm.run_active_turn_to_end();
    REQUIRE(captured_after);
    REQUIRE(leaked_after_propagation.empty());
}

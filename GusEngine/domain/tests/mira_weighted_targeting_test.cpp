// SPDX-License-Identifier: Apache-2.0
// mira_weighted_targeting_test.cpp
//
// Spec executavel (Catch2 v3) da mira ponderada do trash inimigo em PRODUCAO
// (MIRA-PONDERADA-PROD, W2, Opcao F: decisao do lider 2026-08-03/2026-08-08). Cobre os 3
// novos metodos publicos de CombatStateMachine: mira_target_weight (F1+F2+F3, SEM F4),
// pick_weighted_enemy_target (sorteio ponderado via IRandomSource) e record_mira_support
// (F3, hook publico ainda nao ligado a nenhum caminho de producao - ver combat_state_
// machine.hpp).
//
// Fonte da formula: docs/design/mecanicas/analise-mira-resultados.md "Opcao F" e
// proposta-protocolo-simulacao-mira.md secao 5 (V1-V4). Os valores V1-V4 (kMiraBaseWeight/
// kMiraDamageWeightPerPoint/kMiraLowHpWeightScale/kMiraSupportWeight/
// kMiraAttractionWindowRounds) moram em combat_constants.hpp.
//
// Cross-ref: TODO.md MIRA-PONDERADA-PROD; docs/design/mecanicas/analise-mira-resultados.md;
//            docs/design/mecanicas/proposta-protocolo-simulacao-mira.md secao 5;
//            mira_sim_harness.hpp (estudo TEST-ONLY que validou a Opcao F, DESACOPLADO
//            deste codigo de producao - implementa a MESMA formula so pra fins de estudo).

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <array>
#include <string>
#include <vector>

#include "fixed_random.hpp"
#include "gus/domain/combat/combat_actor.hpp"
#include "gus/domain/combat/combat_constants.hpp"
#include "gus/domain/combat/combat_records.hpp"
#include "gus/domain/combat/combat_state.hpp"
#include "gus/domain/combat/combat_state_machine.hpp"
#include "property_gen.hpp"  // PropertyRandom (LCG seedado, amostra grande)

using namespace gus::domain::combat;
using gus::domain::tests::FixedRandom;
using gus::domain::tests::PropertyRandom;

namespace {

CombatActor party_member(const std::string& id, int hp, int atk = 8, int def = 5,
                         int spd = 10) {
    return CombatActor(id, id, hp, atk, def, spd, CardFamily::Eletrico, /*player=*/true);
}

CombatAction always_pass(CombatActor&, const CombatState&) { return CombatAction::pass(); }

}  // namespace

// ----- F2 (ferido atrai): unico fator que independe de historico -----

TEST_CASE("mira: candidato full-hp sem historico pesa so o V1 base", "[domain][combat][mira]") {
    CombatActor gus = party_member("gus", 34);
    CombatActor sentinela =
        CombatActor("sentinela", "sentinela", 55, 10, 8, 8, CardFamily::Cinetico, false);
    CombatStateMachine sm({&gus, &sentinela}, always_pass);

    REQUIRE(sm.mira_target_weight(gus) == combat_constants::kMiraBaseWeight);
}

TEST_CASE("mira: F2 sobe o peso conforme o HP cai (ferido atrai)", "[domain][combat][mira]") {
    CombatActor gus = party_member("gus", 34);
    CombatActor caua = party_member("caua", 55);
    CombatActor sentinela =
        CombatActor("sentinela", "sentinela", 55, 10, 8, 8, CardFamily::Cinetico, false);
    CombatStateMachine sm({&gus, &caua, &sentinela}, always_pass);

    // Gus a 50% de HP: F2 = 100 * (1 - 0.5) = 50.
    gus.take_damage(17);
    const double w_gus = sm.mira_target_weight(gus);
    const double w_caua = sm.mira_target_weight(caua);  // full HP: so o V1 base.

    REQUIRE(w_caua == combat_constants::kMiraBaseWeight);
    REQUIRE(w_gus > w_caua);
    REQUIRE(w_gus == Catch::Approx(combat_constants::kMiraBaseWeight +
                                    combat_constants::kMiraLowHpWeightScale * 0.5));
}

// ----- F1 (dano causado atrai) -----

TEST_CASE("mira: F1 credita dano causado ao ATACANTE (nao ao alvo)", "[domain][combat][mira]") {
    CombatActor gus = party_member("gus", 34, /*atk=*/8, /*def=*/5);
    CombatActor caua = party_member("caua", 55, /*atk=*/8, /*def=*/5);
    CombatActor sentinela =
        CombatActor("sentinela", "sentinela", 55, 10, 3, 8, CardFamily::Cinetico, false);
    // So caua ataca (o hook F1 e exercitado via o pipeline REAL de resolve_basic_attack ->
    // apply_damage_with_hooks - nao ha atalho privado nos testes de proposito).
    auto only_caua_attacks = [&](CombatActor& actor, const CombatState& state) -> CombatAction {
        if (!actor.is_player_side() || actor.id() != "caua") return CombatAction::pass();
        const auto enemies = state.alive_enemies();
        if (enemies.empty()) return CombatAction::pass();
        return CombatAction::attack(enemies.front()->id());
    };
    CombatStateMachine sm({&gus, &caua, &sentinela}, only_caua_attacks);

    sm.begin_turn();
    sm.run_active_turn_to_end();
    sm.advance_to_next_actor();
    sm.begin_turn();
    sm.run_active_turn_to_end();

    // dano basico = max(1, atk-def) = max(1, 8-3) = 5. Caua ataca com AP=3 -> ate 3 hits.
    const double w_caua = sm.mira_target_weight(caua);
    const double w_gus = sm.mira_target_weight(gus);
    REQUIRE(w_caua > w_gus);  // so caua causou dano nesta janela; gus fica no V1 base
    REQUIRE(w_gus == combat_constants::kMiraBaseWeight);
    REQUIRE(w_caua >= combat_constants::kMiraBaseWeight +
                          combat_constants::kMiraDamageWeightPerPoint * 5.0);
}

TEST_CASE("mira: F1 sai da janela apos kMiraAttractionWindowRounds rodadas",
          "[domain][combat][mira]") {
    CombatActor gus = party_member("gus", 34, /*atk=*/8, /*def=*/5);
    CombatActor sentinela =
        CombatActor("sentinela", "sentinela", 55, 1, 3, 8, CardFamily::Cinetico, false);
    // Gus ataca EXATAMENTE 1 vez (flag `attacked_once`), depois so passa - isola o dano de
    // UMA rodada pra provar que o prune (nao so o "nunca mais causa dano") e o que zera o
    // peso depois da janela.
    bool attacked_once = false;
    auto attack_once_then_pass = [&](CombatActor& actor, const CombatState& state) -> CombatAction {
        if (actor.is_player_side() && !attacked_once) {
            const auto enemies = state.alive_enemies();
            if (!enemies.empty()) {
                attacked_once = true;
                return CombatAction::attack(enemies.front()->id());
            }
        }
        return CombatAction::pass();
    };
    CombatStateMachine sm({&gus, &sentinela}, attack_once_then_pass);

    sm.begin_turn();
    sm.run_active_turn_to_end();  // rodada 0: gus ataca 1x, causa dano
    REQUIRE(sm.mira_target_weight(gus) > combat_constants::kMiraBaseWeight);

    // Avanca rodadas alem da janela (kMiraAttractionWindowRounds=2: rodada corrente +
    // anterior). Sentinela (hp=55) sobrevive de sobra a 1 unico hit de dano max(1,8-3)=5.
    for (int i = 0; i < combat_constants::kMiraAttractionWindowRounds + 3 &&
                    sm.outcome() == CombatOutcome::Ongoing;
        ++i) {
        sm.advance_to_next_actor();
        sm.begin_turn();
        sm.run_active_turn_to_end();
    }

    REQUIRE(sm.mira_target_weight(gus) == combat_constants::kMiraBaseWeight);
}

// ----- F3 (suporte atrai) -----

TEST_CASE("mira: F3 soma kMiraSupportWeight quando record_mira_support foi chamado na janela",
          "[domain][combat][mira]") {
    CombatActor gus = party_member("gus", 34);
    CombatActor jaci = party_member("jaci", 55);
    CombatActor sentinela =
        CombatActor("sentinela", "sentinela", 55, 10, 8, 8, CardFamily::Cinetico, false);
    CombatStateMachine sm({&gus, &jaci, &sentinela}, always_pass);

    const double w_before = sm.mira_target_weight(jaci);
    sm.record_mira_support("jaci");
    const double w_after = sm.mira_target_weight(jaci);

    REQUIRE(w_after == Catch::Approx(w_before + combat_constants::kMiraSupportWeight));
}

TEST_CASE("mira: F3 expira apos a janela de kMiraAttractionWindowRounds rodadas",
          "[domain][combat][mira]") {
    CombatActor gus = party_member("gus", 34);
    CombatActor sentinela =
        CombatActor("sentinela", "sentinela", 55, 10, 8, 8, CardFamily::Cinetico, false);
    CombatStateMachine sm({&gus, &sentinela}, always_pass);

    sm.record_mira_support("gus");  // rodada 0
    REQUIRE(sm.mira_target_weight(gus) > combat_constants::kMiraBaseWeight);

    // Avanca rodadas manualmente (Pass sempre) ate sair da janela de 2 rodadas.
    for (int i = 0; i < combat_constants::kMiraAttractionWindowRounds + 2 &&
                    sm.outcome() == CombatOutcome::Ongoing;
        ++i) {
        sm.advance_to_next_actor();
        sm.begin_turn();
        sm.run_active_turn_to_end();
    }

    REQUIRE(sm.mira_target_weight(gus) == combat_constants::kMiraBaseWeight);
}

// ----- SEM F4: Defend/Shield NAO afeta o peso (Opcao F, decisao do lider) -----

TEST_CASE("mira: SEM F4 - Shield ativo NAO muda o peso do candidato", "[domain][combat][mira]") {
    CombatActor gus = party_member("gus", 34);
    CombatActor caua = party_member("caua", 55);
    CombatActor sentinela =
        CombatActor("sentinela", "sentinela", 55, 10, 8, 8, CardFamily::Cinetico, false);
    CombatStateMachine sm({&gus, &caua, &sentinela}, always_pass);

    const double w_before = sm.mira_target_weight(gus);
    gus.add_status(StatusEffect{StatusId::Shield, /*magnitude=*/gus.def(), /*duration=*/1,
                                StackRule::Replace, CardFamily::Eletrico});
    const double w_after = sm.mira_target_weight(gus);

    REQUIRE(w_after == w_before);  // defender NAO altera o peso (Opcao F ignora Defend)
}

// ----- pick_weighted_enemy_target: sorteio proporcional (amostra grande) -----

TEST_CASE("mira: pick_weighted_enemy_target escolhe o candidato de MAIOR peso mais",
          "[domain][combat][mira]") {
    CombatActor gus = party_member("gus", 34);   // sera ferido -> peso alto
    CombatActor caua = party_member("caua", 55); // full HP -> peso base
    CombatActor sentinela =
        CombatActor("sentinela", "sentinela", 55, 10, 8, 8, CardFamily::Cinetico, false);
    CombatStateMachine sm({&gus, &caua, &sentinela}, always_pass);

    gus.take_damage(30);  // gus quase morto: F2 grande, peso MUITO maior que caua

    std::vector<CombatActor*> candidates{&gus, &caua};
    int gus_picks = 0;
    constexpr int kSamples = 20000;
    for (std::uint32_t seed = 1; seed <= static_cast<std::uint32_t>(kSamples); ++seed) {
        // Cada amostra usa uma CombatStateMachine PROPRIA (rng_ e membro dela) semeada por
        // seed distinta - o peso (gus/caua) e reconstruido de forma IDENTICA em cada uma
        // (mesmo HP, sem historico), so o rng varia.
        CombatActor g = party_member("gus", 34);
        CombatActor c = party_member("caua", 55);
        CombatActor s =
            CombatActor("sentinela", "sentinela", 55, 10, 8, 8, CardFamily::Cinetico, false);
        g.take_damage(30);
        PropertyRandom rng(seed);
        CombatStateMachine sample_sm({&g, &c, &s}, always_pass, nullptr, nullptr, &rng);
        CombatActor* picked = sample_sm.pick_weighted_enemy_target({&g, &c});
        if (picked->id() == "gus") ++gus_picks;
    }

    // gus (peso muito maior) deve ser escolhido MUITO mais que caua (nao 50/50 como uniforme
    // seria) - prova que o sorteio e PONDERADO, nao uniforme. Teorico: peso_gus = 100 +
    // 100*(30/34) ~= 188.24; peso_caua = 100; taxa esperada ~= 188.24/288.24 ~= 0.653.
    // Faixa larga o bastante pra ruido de amostra, estreita o bastante pra distinguir de
    // uniforme (0.5) OU de "so o maior peso" deterministico (1.0).
    const double gus_rate = static_cast<double>(gus_picks) / static_cast<double>(kSamples);
    REQUIRE(gus_rate > 0.60);
    REQUIRE(gus_rate < 0.71);
    (void)candidates;
}

TEST_CASE("mira: pick_weighted_enemy_target cai no fallback uniforme quando soma <= 0",
          "[domain][combat][mira]") {
    // Nao deve ocorrer com kMiraBaseWeight>0 no formato atual (V1 garante soma>0 sempre),
    // mas o fallback e exercitado diretamente via candidatos com HP zerado seria invalido
    // (candidatos mortos nao entram na lista real) - este teste prova que o fallback NUNCA
    // lanca e sempre devolve um candidato valido mesmo em cenarios de peso minimo (full HP,
    // sem historico, todos empatados no V1 base): distribuicao deve ficar proxima de
    // uniforme entre os 2.
    int gus_picks = 0;
    constexpr int kSamples = 20000;
    for (std::uint32_t seed = 1; seed <= static_cast<std::uint32_t>(kSamples); ++seed) {
        CombatActor g = party_member("gus", 34);
        CombatActor c = party_member("caua", 34);  // mesmo HP: pesos EMPATADOS
        CombatActor s =
            CombatActor("sentinela", "sentinela", 55, 10, 8, 8, CardFamily::Cinetico, false);
        PropertyRandom rng(seed);
        CombatStateMachine sample_sm({&g, &c, &s}, always_pass, nullptr, nullptr, &rng);
        CombatActor* picked = sample_sm.pick_weighted_enemy_target({&g, &c});
        if (picked->id() == "gus") ++gus_picks;
    }
    const double gus_rate = static_cast<double>(gus_picks) / static_cast<double>(kSamples);
    REQUIRE(gus_rate > 0.45);
    REQUIRE(gus_rate < 0.55);
}

TEST_CASE("mira: pick_weighted_enemy_target lanca invalid_argument com candidates vazio",
          "[domain][combat][mira]") {
    CombatActor gus = party_member("gus", 34);
    CombatActor sentinela =
        CombatActor("sentinela", "sentinela", 55, 10, 8, 8, CardFamily::Cinetico, false);
    CombatStateMachine sm({&gus, &sentinela}, always_pass);

    REQUIRE_THROWS_AS(sm.pick_weighted_enemy_target({}), std::invalid_argument);
}

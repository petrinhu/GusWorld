// SPDX-License-Identifier: Apache-2.0
// mira_sim_harness_test.cpp
//
// Spec do harness de simulacao estatistica da mira inimiga (MIRA-SIM). Cobre:
//   (a) unidades puras (pesos, janela de atracao, MCID, agregacao) com dados sinteticos;
//   (b) integracao real deterministica contra o motor JA EXISTENTE (sanidade C=D=E=F em
//       L1, cap de rodadas em L3, pareamento por seed);
//   (c) smoke test estrutural dos 7 lotes x 6 bracos com N pequeno (sempre roda no CI).
//       O RUN GRANDE (10 milhoes) NAO roda aqui - fica atras de GUSWORLD_MIRA_SIM_FULL,
//       e mesmo assim so o economy-designer/lider dispara (fora do escopo desta fatia).
//
// Cross-ref: mira_sim_harness.hpp; docs/design/mecanicas/proposta-protocolo-simulacao-mira.md;
//            balance_harness_test.cpp (padrao de spec ja existente).

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <vector>

#include "mira_sim_harness.hpp"

using namespace gus::domain::tests::mira_sim;
using gus::domain::combat::CombatOutcome;

// ============================================================================
// (a) Unidades puras.
// ============================================================================

TEST_CASE("mira_sim: arm_label/lote_label sao unicos e nao lancam", "[domain][mira_sim]") {
    std::vector<std::string> labels;
    for (MiraArm arm : kAllArms) labels.push_back(arm_label(arm));
    for (std::size_t i = 0; i < labels.size(); ++i)
        for (std::size_t j = i + 1; j < labels.size(); ++j)
            REQUIRE(labels[i] != labels[j]);

    std::vector<std::string> lotes;
    for (Lote l : kAllLotes) lotes.push_back(lote_label(l));
    for (std::size_t i = 0; i < lotes.size(); ++i)
        for (std::size_t j = i + 1; j < lotes.size(); ++j)
            REQUIRE(lotes[i] != lotes[j]);
}

TEST_CASE("mira_sim: f4_multiplier_for bate com a tabela V5 do protocolo (secao 2.1/5)",
          "[domain][mira_sim]") {
    REQUIRE(f4_multiplier_for(MiraArm::C_F4Soft) == Catch::Approx(0.5));
    REQUIRE(f4_multiplier_for(MiraArm::D_F4Strong) == Catch::Approx(0.1));
    REQUIRE(f4_multiplier_for(MiraArm::E_F4Inverted) == Catch::Approx(2.0));
    REQUIRE(f4_multiplier_for(MiraArm::F_NoF4) == Catch::Approx(1.0));

    REQUIRE_FALSE(is_weighted_arm(MiraArm::A_StatusQuo));
    REQUIRE_FALSE(is_weighted_arm(MiraArm::B_Uniform));
    REQUIRE(is_weighted_arm(MiraArm::C_F4Soft));
    REQUIRE(is_weighted_arm(MiraArm::D_F4Strong));
    REQUIRE(is_weighted_arm(MiraArm::E_F4Inverted));
    REQUIRE(is_weighted_arm(MiraArm::F_NoF4));
}

TEST_CASE("mira_sim: scenario_for respeita 'fixo em todos os lotes' + variacoes (secao 2.2)",
          "[domain][mira_sim]") {
    for (Lote l : kAllLotes) {
        const LoteScenario s = scenario_for(l);
        REQUIRE(s.party.size() == 3);
        REQUIRE(s.party[0].id == "gus");
        REQUIRE(s.party[1].id == "caua");
        REQUIRE(s.party[2].id == "jaci");
        REQUIRE(s.party[0].is_universal_compiler);  // Gus-centric (check_end real)
    }

    REQUIRE(scenario_for(Lote::L1_Vanilla).enemies.size() == 3);
    REQUIRE(scenario_for(Lote::L1_Vanilla).party_hp_fraction_start == Catch::Approx(1.0));

    const LoteScenario l6 = scenario_for(Lote::L6_VantagemInimigos);
    REQUIRE(l6.enemies.size() == 3);
    REQUIRE(l6.enemies[0].id == "daemon");
    REQUIRE(l6.party_hp_fraction_start == Catch::Approx(0.70));

    const LoteScenario l7 = scenario_for(Lote::L7_AtacanteUnico);
    REQUIRE(l7.enemies.size() == 1);
    REQUIRE(l7.enemies[0].id == "daemon");
    REQUIRE(l7.party_hp_fraction_start == Catch::Approx(1.0));
}

TEST_CASE("mira_sim: roles_for casa com a tabela do protocolo (secao 2.2)", "[domain][mira_sim]") {
    using R = PartyRole;

    const auto l1 = roles_for(Lote::L1_Vanilla);
    REQUIRE(l1 == std::array<R, 3>{R::AttackFront, R::AttackFront, R::AttackFront});

    const auto l2 = roles_for(Lote::L2_ParedeDedicada);
    REQUIRE(l2[2] == R::DefendThenAttack);  // Jaci = parede
    REQUIRE(l2[0] == R::AttackFront);
    REQUIRE(l2[1] == R::AttackFront);

    const auto l3 = roles_for(Lote::L3_TurtleTotal);
    REQUIRE(l3 == std::array<R, 3>{R::DefendOnlyTurtle, R::DefendOnlyTurtle, R::DefendOnlyTurtle});

    const auto l4 = roles_for(Lote::L4_SuporteAtivo);
    REQUIRE(l4[2] == R::HealMostInjured);  // Jaci cura

    const auto l5 = roles_for(Lote::L5_CartasDeAgro);
    REQUIRE(l5[1] == R::HoneypotEntaoAtaca);  // Caua
    REQUIRE(l5[0] == R::FirewallEntaoAtaca);  // Gus
    REQUIRE(l5[2] == R::AttackFront);         // Jaci

    REQUIRE(roles_for(Lote::L6_VantagemInimigos)[2] == R::DefendThenAttack);
    REQUIRE(roles_for(Lote::L7_AtacanteUnico)[2] == R::DefendThenAttack);
}

TEST_CASE("mira_sim: AttractionTracker janela de 2 rodadas (F1/F3) exclui rodadas antigas",
          "[domain][mira_sim]") {
    AttractionTracker tr;
    tr.record_damage(PartyMember::Caua, /*round=*/0, 10);
    tr.record_damage(PartyMember::Caua, /*round=*/1, 5);
    // janela em round=1 inclui rounds {0,1}: 10+5=15
    REQUIRE(tr.damage_last_window(PartyMember::Caua, 1) == 15);
    // janela em round=2 inclui rounds {1,2}: so o 5 sobrevive (round=0 caiu fora)
    REQUIRE(tr.damage_last_window(PartyMember::Caua, 2) == 5);
    // janela em round=4 inclui rounds {3,4}: nem o round=1 (5) nem o round=0 (10) sobrevivem
    REQUIRE(tr.damage_last_window(PartyMember::Caua, 4) == 0);

    tr.record_support(PartyMember::Jaci, /*round=*/0);
    REQUIRE(tr.supported_last_window(PartyMember::Jaci, 0));
    REQUIRE(tr.supported_last_window(PartyMember::Jaci, 1));
    REQUIRE_FALSE(tr.supported_last_window(PartyMember::Jaci, 2));
}

TEST_CASE("mira_sim: AttractionTracker observe_round decrementa honeypot/firewall por rodada",
          "[domain][mira_sim]") {
    AttractionTracker tr;
    tr.honeypot_rounds_left[idx(PartyMember::Caua)] = kHoneypotDurationRounds;
    tr.firewall_rounds_left[idx(PartyMember::Gus)] = kFirewallDurationRounds;

    tr.observe_round(0);
    REQUIRE(tr.honeypot_rounds_left[idx(PartyMember::Caua)] == kHoneypotDurationRounds);

    tr.observe_round(0);  // mesma rodada: sem decremento
    REQUIRE(tr.honeypot_rounds_left[idx(PartyMember::Caua)] == kHoneypotDurationRounds);

    tr.observe_round(1);
    REQUIRE(tr.honeypot_rounds_left[idx(PartyMember::Caua)] == kHoneypotDurationRounds - 1);
    REQUIRE(tr.firewall_rounds_left[idx(PartyMember::Gus)] == kFirewallDurationRounds - 1);

    tr.observe_round(2);
    REQUIRE(tr.honeypot_rounds_left[idx(PartyMember::Caua)] == 0);
    REQUIRE(tr.firewall_rounds_left[idx(PartyMember::Gus)] == 0);

    tr.observe_round(3);  // clamp em 0, nunca negativo
    REQUIRE(tr.honeypot_rounds_left[idx(PartyMember::Caua)] == 0);
}

namespace {

// Ator sintetico minimo pra testar compute_target_weight sem rodar o motor inteiro.
gus::domain::combat::CombatActor make_actor(std::string id, int hp, int max_hp, int def) {
    gus::domain::combat::CombatActor a(id, id, max_hp, /*atk=*/1, def, /*spd=*/1,
                                       gus::domain::combat::CardFamily::Eletrico,
                                       /*is_player_side=*/true);
    if (hp < max_hp) a.take_damage(max_hp - hp);
    return a;
}

}  // namespace

// Valores literais (nao simbolicos) nas asserções de resultado: comparar contra a MESMA
// constante usada internamente e tautologico (se kBaseWeight mudar de valor, o teste
// segue passando sem nunca ter provado nada). V1=100, V2=+1/ponto, V3=100x(1-hp/hpmax),
// V4=+60 sao os valores do protocolo secao 5 - sao ELES que o teste tem de fixar.
TEST_CASE("mira_sim: compute_target_weight aplica V1+F1+F2+F3 antes do F4/F5/F6 (valores "
          "literais do protocolo, nao a constante contra ela mesma)",
          "[domain][mira_sim]") {
    AttractionTracker tr;
    auto full_hp = make_actor("caua", 55, 55, 8);
    const double w_full = compute_target_weight(PartyMember::Caua, full_hp, 0, tr, MiraArm::F_NoF4);
    REQUIRE(w_full == Catch::Approx(100.0));  // V1 sozinho: hp cheio, sem dano, sem suporte

    // F2 (V3): magnitude EXATA, nao so ordem. 27/55 hp = 49.0909...%; F2 = 100*(1-27/55).
    auto half_hp = make_actor("caua", 27, 55, 8);
    const double w_half = compute_target_weight(PartyMember::Caua, half_hp, 0, tr, MiraArm::F_NoF4);
    REQUIRE(w_half == Catch::Approx(100.0 + 100.0 * (1.0 - 27.0 / 55.0)).epsilon(0.0001));
    REQUIRE(w_half == Catch::Approx(150.909090909).epsilon(0.0001));

    tr.record_damage(PartyMember::Caua, 0, 20);  // F1 (V2): +1 de peso por ponto de dano
    const double w_after_dmg =
        compute_target_weight(PartyMember::Caua, full_hp, 0, tr, MiraArm::F_NoF4);
    REQUIRE(w_after_dmg == Catch::Approx(120.0));  // 100 + 20

    // F3 (V4): magnitude EXATA (+60), isolada num tracker limpo (sem F1 do bloco acima).
    AttractionTracker tr_support;
    tr_support.record_support(PartyMember::Caua, 0);
    const double w_support =
        compute_target_weight(PartyMember::Caua, full_hp, 0, tr_support, MiraArm::F_NoF4);
    REQUIRE(w_support == Catch::Approx(160.0));  // 100 + 60
}

TEST_CASE("mira_sim: compute_target_weight aplica F4 SO quando o alvo tem Shield ativo",
          "[domain][mira_sim]") {
    AttractionTracker tr;
    auto no_shield = make_actor("jaci", 55, 55, 10);
    const double w_no_shield =
        compute_target_weight(PartyMember::Jaci, no_shield, 0, tr, MiraArm::D_F4Strong);
    REQUIRE(w_no_shield == Catch::Approx(100.0));  // F4 nao entra sem Shield

    auto shielded = make_actor("jaci", 55, 55, 10);
    shielded.add_status(gus::domain::combat::StatusEffect{
        gus::domain::combat::StatusId::Shield, /*magnitude=*/10, /*duration=*/1,
        gus::domain::combat::StackRule::Replace, gus::domain::combat::CardFamily::Eletrico});
    const double w_shield_soft =
        compute_target_weight(PartyMember::Jaci, shielded, 0, tr, MiraArm::C_F4Soft);
    const double w_shield_strong =
        compute_target_weight(PartyMember::Jaci, shielded, 0, tr, MiraArm::D_F4Strong);
    const double w_shield_inverted =
        compute_target_weight(PartyMember::Jaci, shielded, 0, tr, MiraArm::E_F4Inverted);

    REQUIRE(w_shield_soft == Catch::Approx(50.0));    // 100 * 0.5 (V5 braco C)
    REQUIRE(w_shield_strong == Catch::Approx(10.0));  // 100 * 0.1 (V5 braco D)
    REQUIRE(w_shield_inverted == Catch::Approx(200.0));  // 100 * 2.0 (V5 braco E)
    // H1 do lider (proposta-protocolo secao 1.1): E (invertido) deve SEMPRE pesar mais
    // que D (forte) e C (suave) quando defendendo - e a ordem que os bracos codificam.
    REQUIRE(w_shield_inverted > w_shield_soft);
    REQUIRE(w_shield_soft > w_shield_strong);
}

TEST_CASE("mira_sim: compute_target_weight aplica F5 (honeypot x3) e F6 (firewall x0)",
          "[domain][mira_sim]") {
    AttractionTracker tr;
    auto actor = make_actor("caua", 55, 55, 8);

    tr.honeypot_rounds_left[idx(PartyMember::Caua)] = 1;
    const double w_honeypot =
        compute_target_weight(PartyMember::Caua, actor, 0, tr, MiraArm::F_NoF4);
    REQUIRE(w_honeypot == Catch::Approx(300.0));  // 100 * 3.0 (V1 x V6)

    AttractionTracker tr2;
    tr2.firewall_rounds_left[idx(PartyMember::Caua)] = 1;
    const double w_firewall =
        compute_target_weight(PartyMember::Caua, actor, 0, tr2, MiraArm::F_NoF4);
    REQUIRE(w_firewall == Catch::Approx(0.0));
}

TEST_CASE("mira_sim: pick_weighted cai pro fallback uniforme quando total de pesos e 0 (V7)",
          "[domain][mira_sim]") {
    auto a1 = make_actor("gus", 34, 34, 5);
    auto a2 = make_actor("caua", 55, 55, 8);
    std::vector<gus::domain::combat::CombatActor*> alive{&a1, &a2};
    std::vector<double> weights{0.0, 0.0};
    gus::domain::tests::PropertyRandom rng(1);
    gus::domain::combat::CombatActor* picked = pick_weighted(alive, weights, rng);
    REQUIRE((picked == &a1 || picked == &a2));  // nao lanca, sempre escolhe alguem vivo
}

// PROVA ESTATISTICA de uniformidade (achado do QA: o teste acima so provava "nao lanca",
// nao "e uniforme" - um mutante que sempre devolvesse alive[0] passaria igual). Roda 2000
// sorteios com pesos zerados e diferentes seeds; ambos os candidatos tem de aparecer numa
// faixa compativel com 50/50 (nao um sempre-o-primeiro).
TEST_CASE("mira_sim: pick_weighted fallback (V7) e ESTATISTICAMENTE uniforme, nao sempre "
          "o primeiro vivo",
          "[domain][mira_sim]") {
    auto a1 = make_actor("gus", 34, 34, 5);
    auto a2 = make_actor("caua", 55, 55, 8);
    std::vector<gus::domain::combat::CombatActor*> alive{&a1, &a2};
    std::vector<double> weights{0.0, 0.0};

    constexpr int kTrials = 2000;
    int picked_first = 0;
    for (std::uint32_t seed = 1; seed <= kTrials; ++seed) {
        gus::domain::tests::PropertyRandom rng(seed);
        if (pick_weighted(alive, weights, rng) == &a1) ++picked_first;
    }
    const double frac_first = static_cast<double>(picked_first) / kTrials;
    // Faixa generosa (35%-65%) pra nao ser fragil ao LCG especifico, mas decisiva o
    // bastante pra reprovar "sempre o primeiro" (100%) ou "sempre o segundo" (0%).
    REQUIRE(frac_first > 0.35);
    REQUIRE(frac_first < 0.65);
}

TEST_CASE("mira_sim: mcid_compare_pct/rounds respeita o piso do lider (secao 3)",
          "[domain][mira_sim]") {
    REQUIRE(mcid_compare_pct(50.0, 46.0).verdict == McidVerdict::Empate);          // diff=4 <5
    REQUIRE(mcid_compare_pct(50.0, 44.9).verdict == McidVerdict::DiferencaRelevante);  // diff>5
    REQUIRE(mcid_compare_pct(50.0, 45.0).verdict == McidVerdict::DiferencaRelevante);  // diff==5, NAO empate (estrito <)

    REQUIRE(mcid_compare_rounds(4.0, 4.4).verdict == McidVerdict::Empate);  // diff=0.4<0.5
    REQUIRE(mcid_compare_rounds(4.0, 4.5).verdict == McidVerdict::DiferencaRelevante);  // diff==0.5
    REQUIRE(mcid_compare_rounds(4.0, 4.6).verdict == McidVerdict::DiferencaRelevante);
}

TEST_CASE("mira_sim: proportion_metric_pct/mean_metric batem com formula conhecida",
          "[domain][mira_sim]") {
    const MetricWithCi p = proportion_metric_pct(75, 100);
    REQUIRE(p.value == Catch::Approx(75.0));
    REQUIRE(p.moe95 == Catch::Approx(1.96 * std::sqrt(0.75 * 0.25 / 100.0) * 100.0));

    REQUIRE(proportion_metric_pct(0, 0).value == Catch::Approx(0.0));

    const MetricWithCi m = mean_metric({2.0, 4.0, 6.0});
    REQUIRE(m.value == Catch::Approx(4.0));
    REQUIRE(m.moe95 > 0.0);

    REQUIRE(mean_metric({}).value == Catch::Approx(0.0));
    REQUIRE(mean_metric({5.0}).moe95 == Catch::Approx(0.0));  // n=1: sem desvio-padrao
}

TEST_CASE("mira_sim: aggregate_mira calcula E1/E2/E3/E4/E5 sobre BattleTrace sintetico",
          "[domain][mira_sim]") {
    BattleTrace vitoria_rapida;
    vitoria_rapida.outcome = CombatOutcome::Victory;
    vitoria_rapida.rounds = 4;
    vitoria_rapida.damage_taken = {30, 0, 0};  // 100% no Gus: saco de pancadas
    vitoria_rapida.fell = {false, false, false};
    vitoria_rapida.enemy_target_turns = 4;
    vitoria_rapida.enemy_target_repeats = 4;  // sempre o mesmo alvo (previsivel)

    BattleTrace derrota_lenta;
    derrota_lenta.outcome = CombatOutcome::Defeat;
    derrota_lenta.rounds = 12;
    derrota_lenta.damage_taken = {10, 10, 10};  // espalhado
    derrota_lenta.fell = {true, false, false};
    derrota_lenta.first_fall_round = 5;
    derrota_lenta.enemy_target_turns = 4;
    derrota_lenta.enemy_target_repeats = 1;

    const LoteArmReport r = aggregate_mira("lote-teste", "braco-teste", {vitoria_rapida, derrota_lenta});
    REQUIRE(r.n == 2);
    REQUIRE(r.win_rate_pct.value == Catch::Approx(50.0));
    REQUIRE(r.mean_rounds == Catch::Approx(8.0));
    // mediana (percentile p=0.5), NAO p90: com rounds=[4,12] os dois divergem (p50=8.0,
    // p90=11.2) - discrimina um mutante que troque o percentil usado.
    REQUIRE(r.median_rounds == Catch::Approx(8.0));
    REQUIRE(r.pct_in_window_3_5.value == Catch::Approx(50.0));  // so a luta de 4 rounds entra
    REQUIRE(r.concentration_pct.value == Catch::Approx((100.0 + 33.333333333) / 2.0).epsilon(0.001));
    REQUIRE(r.pct_saco_de_pancadas.value == Catch::Approx(50.0));  // so a 1a bate >70%
    REQUIRE(r.pct_any_fall.value == Catch::Approx(50.0));
    REQUIRE(r.pct_fall_by_member[0].value == Catch::Approx(50.0));  // Gus caiu 1x em 2
    REQUIRE(r.pct_fall_by_member[1].value == Catch::Approx(0.0));
    REQUIRE(r.repeat_target_pct.value == Catch::Approx(100.0 * 5.0 / 8.0));  // (4+1)/(4+4)
    REQUIRE(r.mean_first_fall_round == Catch::Approx(5.0));
}

// E6/E7 com valores ASSIMETRICOS (raw != absorvido; final_hp_pct != soma bruta; capped
// misto): protocolo secao 4 chama E6 "a pergunta-mae em termos do motor" - uma inversao
// absorvido<->bruto tem de dar um numero CLARAMENTE diferente, nunca coincidir por acaso.
TEST_CASE("mira_sim: aggregate_mira E6 (shield_absorption_pct) nao troca absorvido por bruto",
          "[domain][mira_sim]") {
    BattleTrace b1;
    b1.outcome = CombatOutcome::Victory;
    b1.rounds = 4;
    b1.shield_raw_total = 100;
    b1.shield_absorbed_total = 60;  // absorcao PARCIAL (60% do bruto)

    BattleTrace b2;
    b2.outcome = CombatOutcome::Victory;
    b2.rounds = 4;
    b2.shield_raw_total = 50;
    b2.shield_absorbed_total = 50;  // absorcao TOTAL

    const LoteArmReport r = aggregate_mira("lote-e6", "braco-e6", {b1, b2});
    // pooled: absorvido=110, bruto=150 -> 73.33%. Se trocado (bruto/absorvido): 136.36%.
    REQUIRE(r.shield_absorption_pct.value == Catch::Approx(100.0 * 110.0 / 150.0));
    REQUIRE(r.shield_absorption_pct.value < 100.0);  // absorcao nunca pode exceder o bruto
}

TEST_CASE("mira_sim: aggregate_mira E7 (avg_final_hp_pct/pct_hit_round_cap) com dados "
          "assimetricos e mistos",
          "[domain][mira_sim]") {
    BattleTrace capped_battle;
    capped_battle.outcome = CombatOutcome::Defeat;
    capped_battle.rounds = 30;
    capped_battle.capped = true;
    capped_battle.final_hp_fraction = {1.0, 0.5, 0.0};

    BattleTrace normal_battle;
    normal_battle.outcome = CombatOutcome::Victory;
    normal_battle.rounds = 4;
    normal_battle.capped = false;
    normal_battle.final_hp_fraction = {0.2, 0.2, 0.2};

    const LoteArmReport r = aggregate_mira("lote-e7", "braco-e7", {capped_battle, normal_battle});
    // media de 6 valores: (1.0+0.5+0.0+0.2+0.2+0.2)/6 = 0.35 -> 35%. Sem dividir por N
    // (mutante) daria 210%.
    REQUIRE(r.avg_final_hp_pct == Catch::Approx(35.0));
    // 1 de 2 lutas capped -> 50%. Um mutante fixo em 100% falharia aqui.
    REQUIRE(r.pct_hit_round_cap.value == Catch::Approx(50.0));
}

TEST_CASE("mira_sim: aggregate_mira com vetor vazio nao explode", "[domain][mira_sim]") {
    const LoteArmReport r = aggregate_mira("vazio", "vazio", {});
    REQUIRE(r.n == 0);
    REQUIRE(r.win_rate_pct.value == Catch::Approx(0.0));
}

// ============================================================================
// (b) Integracao real, deterministica, contra o motor JA EXISTENTE.
// ============================================================================

TEST_CASE("mira_sim: run_single_mira_battle e deterministico pra mesma seed/lote/braco",
          "[domain][mira_sim]") {
    const BattleTrace a = run_single_mira_battle(Lote::L1_Vanilla, MiraArm::C_F4Soft, 42);
    const BattleTrace b = run_single_mira_battle(Lote::L1_Vanilla, MiraArm::C_F4Soft, 42);
    REQUIRE(a.outcome == b.outcome);
    REQUIRE(a.rounds == b.rounds);
    REQUIRE(a.damage_taken == b.damage_taken);
}

// Regressao do DEFEITO reportado pelo team-lead (2026-08-01, contra battle_scene.cpp:
// 210-224): o inimigo tem que agir 1x por turno (economia de AP da producao), a party
// tem que usar os 3 AP normalmente. Cenario controlado (HP altissimo dos 2 lados, atk/def
// simples, sem RNG - ataque basico e deterministico secao 17) pra poder prever o dano
// EXATO por rodada e provar a razao certa (nao 3x inflada de nenhum dos dois lados).
TEST_CASE("mira_sim: economia de AP - inimigo 1x/turno, party 3x/turno (regressao do "
          "defeito reportado, nao mais gateado por cima)",
          "[domain][mira_sim]") {
    using gus::domain::combat::CombatAction;
    using gus::domain::combat::CombatActor;
    using gus::domain::combat::CombatState;
    using gus::domain::combat::CombatStateMachine;

    // HP altissimo dos 2 lados: a luta so termina pelo cap de rodadas (V12=30), entao
    // rounds_elapsed e conhecido de antemao (>= kRoundCap), permitindo prever o dano total.
    CombatActor hero("hero", "hero", /*max_hp=*/1'000'000, /*atk=*/10, /*def=*/0, /*spd=*/10,
                     CardFamily::Eletrico, /*is_player_side=*/true);
    CombatActor foe("foe", "foe", /*max_hp=*/1'000'000, /*atk=*/7, /*def=*/0, /*spd=*/1,
                    CardFamily::Cinetico, /*is_player_side=*/false);
    std::vector<CombatActor*> actor_ptrs{&hero, &foe};

    BattleTrace trace;
    AttractionTracker tr;
    std::optional<std::string> last_enemy_target;
    gus::domain::tests::PropertyRandom rng(1);
    const CombatStateMachine* sm_ptr = nullptr;
    const std::array<PartyRole, kPartySize> roles{PartyRole::AttackFront, PartyRole::AttackFront,
                                                   PartyRole::AttackFront};

    auto provider = [&](CombatActor& actor, const CombatState& state) -> CombatAction {
        const int round = state.round_index();
        tr.observe_round(round);

        if (round >= kRoundCap) {
            // Mesmo mecanismo de cap de run_single_mira_battle (ver "CAP DE RODADAS" no
            // cabecalho): wipe deterministico via take_damage(), nao mais Fuga forcada.
            for (CombatActor* p : state.alive_players())
                if (p->is_alive()) p->take_damage(p->hp());
            return CombatAction::pass();
        }
        if (actor.is_player_side()) {
            // "hero" nao e gus/caua/jaci; decide_party_action so precisa do papel e do
            // AttractionTracker (F1), que aqui nao importa - usamos Gus como PartyMember
            // dummy so pra bater a assinatura (o teste nao consulta F1/F2/F3 de ninguem).
            return decide_party_action(PartyMember::Gus, PartyRole::AttackFront, actor, state,
                                       *sm_ptr, tr, trace, round);
        }
        if (actor.ap() < actor.max_ap()) return CombatAction::pass();
        // "foe" nao e um PartyMember: chamamos a MESMA logica de braco A (front) direto,
        // pra nao depender de member_of() (que so conhece gus/caua/jaci).
        trace.enemy_target_turns++;
        const int net = sm_ptr->preview_basic_attack_damage(actor, hero);
        trace.damage_taken[idx(PartyMember::Gus)] += net;
        return CombatAction::attack(hero.id());
    };

    CombatStateMachine sm(actor_ptrs, provider, nullptr, nullptr, &rng);
    sm_ptr = &sm;
    const auto result = sm.run_until_end();

    // Wipe deterministico (nao mais Fuga): o resultado e Defeat, nao Fled.
    REQUIRE(result.outcome == CombatOutcome::Defeat);
    REQUIRE(result.rounds_elapsed >= kRoundCap);

    // Inimigo: 1 ataque de 7 por rodada (nao 3x21=63). Tolerancia de 1 rodada de folga na
    // fronteira do cap (a ultima rodada pode nao completar o turno do inimigo).
    const double dmg_per_round_to_hero =
        static_cast<double>(trace.damage_taken[idx(PartyMember::Gus)]) / result.rounds_elapsed;
    REQUIRE(dmg_per_round_to_hero < 10.0);   // 3x inflado daria ~21; o certo e ~7
    REQUIRE(dmg_per_round_to_hero > 5.0);

    // Party: 3 ataques de 10 por rodada = ~30 (nao ~10, que seria o gate incorreto que o
    // team-lead apontou vazando pro lado da party).
    const int hero_dmg_total = 1'000'000 - foe.hp();
    const double dmg_per_round_from_hero =
        static_cast<double>(hero_dmg_total) / result.rounds_elapsed;
    REQUIRE(dmg_per_round_from_hero > 25.0);  // 1x nao-gateado daria ~10; o certo e ~30
    REQUIRE(dmg_per_round_from_hero < 35.0);
}

TEST_CASE("mira_sim: L1 vanilla - C/D/E/F sao IDENTICOS (F4 nunca dispara, sanidade do "
          "harness, protocolo secao 2.2 L1)",
          "[domain][mira_sim]") {
    const BattleTrace c = run_single_mira_battle(Lote::L1_Vanilla, MiraArm::C_F4Soft, 7);
    const BattleTrace d = run_single_mira_battle(Lote::L1_Vanilla, MiraArm::D_F4Strong, 7);
    const BattleTrace e = run_single_mira_battle(Lote::L1_Vanilla, MiraArm::E_F4Inverted, 7);
    const BattleTrace f = run_single_mira_battle(Lote::L1_Vanilla, MiraArm::F_NoF4, 7);

    REQUIRE(c.rounds == d.rounds);
    REQUIRE(c.rounds == e.rounds);
    REQUIRE(c.rounds == f.rounds);
    REQUIRE(c.outcome == d.outcome);
    REQUIRE(c.damage_taken == d.damage_taken);
    REQUIRE(c.damage_taken == e.damage_taken);
    REQUIRE(c.damage_taken == f.damage_taken);
}

TEST_CASE("mira_sim: pareamento por seed - MESMA seed sob 6 bracos, cada braco enfrenta a "
          "MESMA luta (protocolo secao 2.1/7)",
          "[domain][mira_sim]") {
    // Nao exige resultado IGUAL entre A/B e C-F (as politicas divergem por desenho); exige
    // so que a construcao do cenario (party/inimigos/seed) seja identica entre chamadas -
    // verificado indiretamente pelo determinismo (teste acima) + este smoke de nao-excecao.
    for (MiraArm arm : kAllArms) {
        REQUIRE_NOTHROW(run_single_mira_battle(Lote::L2_ParedeDedicada, arm, 123));
    }
}

TEST_CASE("mira_sim: L3 turtle total bate no cap de 30 rodadas (empate tecnico, V12) - "
          "estatlines canonicas absorvem 100% do dano bruto (H1 do lider)",
          "[domain][mira_sim]") {
    // Determinístico: Sentinela atk=10 vs qualquer Def da party (5/8/10) da raw<=Def,
      // logo o Shield (magnitude=Def) absorve TUDO sempre - stalemate ate o cap.
    const BattleTrace t = run_single_mira_battle(Lote::L3_TurtleTotal, MiraArm::A_StatusQuo, 99);
    REQUIRE(t.capped);
    REQUIRE(t.rounds >= kRoundCap);
    // Cap garantido pelo harness (wipe deterministico, ver "CAP DE RODADAS" no cabecalho):
    // o resultado e Defeat (a party e zerada), nao mais Fled (a Fuga forcada podia falhar
    // pra sempre - achado CRITICO do QA).
    REQUIRE(t.outcome == CombatOutcome::Defeat);
    // H1 (parede renovavel): dano bruto tentado > 0, mas absorvido quase/totalmente.
    REQUIRE(t.shield_raw_total > 0);
    REQUIRE(t.shield_absorbed_total == t.shield_raw_total);
}

// Regressao OBRIGATORIA do achado CRITICO do QA (2026-08-01): a Fuga forcada do cap
// comparava SPD top da party (so entre vivos) com SPD top dos inimigos (so entre vivos).
// Se a Caua (SPD 13, a maior da party) cai e so Gus(9)/Jaci(7) seguem vivos contra um
// Daemon-Guard (SPD 10), o SPD top da party vira 9 < 10 - a Fuga falha PRA SEMPRE (so o
// Gus caido encerra o combate, secao 3), e o motor so pararia ao lancar a excecao do teto
// de 10000 turnos (o QA mediu 3303 rodadas ALEM do cap antes disso). Reproduz o cenario
// EXATO: Caua morta, Gus e Jaci vivos com HP altissimo, Daemon vivo com HP altissimo
// (nada morre "de verdade" antes do cap - a UNICA forma de terminar e o cap disparar).
TEST_CASE("mira_sim: cap de rodadas e garantido mesmo com a party perdendo o maior SPD "
          "(regressao do CRITICO achado pelo QA - Fuga forcada podia falhar pra sempre)",
          "[domain][mira_sim]") {
    using gus::domain::combat::CombatAction;
    using gus::domain::combat::CombatActor;
    using gus::domain::combat::CombatState;
    using gus::domain::combat::CombatStateMachine;

    CombatActor gus("gus", "gus", /*max_hp=*/1'000'000, /*atk=*/1, /*def=*/1000,
                    /*spd=*/9, CardFamily::Eletrico, /*is_player_side=*/true, false, 0,
                    /*is_universal_compiler=*/true);
    CombatActor caua("caua", "caua", /*max_hp=*/55, /*atk=*/14, /*def=*/8, /*spd=*/13,
                     CardFamily::Eletrico, /*is_player_side=*/true);
    CombatActor jaci("jaci", "jaci", /*max_hp=*/1'000'000, /*atk=*/1, /*def=*/1000,
                     /*spd=*/7, CardFamily::Bioquimico, /*is_player_side=*/true);
    CombatActor daemon("daemon", "daemon", /*max_hp=*/1'000'000, /*atk=*/1, /*def=*/1000,
                       kDaemonGuardSpd, CardFamily::Cinetico, /*is_player_side=*/false);
    caua.take_damage(caua.hp());  // Caua morta ANTES da luta comecar (SPD top da party = 9)
    REQUIRE_FALSE(caua.is_alive());
    REQUIRE(gus.spd() < daemon.spd());   // 9 < 10: a condicao EXATA que quebrava a Fuga
    REQUIRE(jaci.spd() < daemon.spd());  // 7 < 10

    std::vector<CombatActor*> actor_ptrs{&gus, &caua, &jaci, &daemon};

    BattleTrace trace;
    AttractionTracker tr;
    std::optional<std::string> last_enemy_target;
    gus::domain::tests::PropertyRandom rng(7);
    const CombatStateMachine* sm_ptr = nullptr;
    bool cap_triggered = false;

    // MESMO mecanismo de run_single_mira_battle (ver "CAP DE RODADAS" no cabecalho): wipe
    // deterministico via take_damage(), NUNCA Fuga. Def altissima dos 2 lados (raw clamp em
    // kMinDamage=1 sempre) garante que ninguem morre "de verdade" antes do cap disparar.
    auto provider = [&](CombatActor& actor, const CombatState& state) -> CombatAction {
        const int round = state.round_index();
        tr.observe_round(round);
        if (round >= kRoundCap) {
            cap_triggered = true;
            for (CombatActor* p : state.alive_players())
                if (p->is_alive()) p->take_damage(p->hp());
            return CombatAction::pass();
        }
        if (actor.is_player_side()) {
            const auto enemies = state.alive_enemies();
            return enemies.empty() ? CombatAction::pass()
                                   : CombatAction::attack(enemies.front()->id());
        }
        if (actor.ap() < actor.max_ap()) return CombatAction::pass();
        return decide_enemy_action(MiraArm::A_StatusQuo, actor, state, *sm_ptr, tr, trace, rng,
                                   round, last_enemy_target);
    };

    CombatStateMachine sm(actor_ptrs, provider, nullptr, nullptr, &rng);
    sm_ptr = &sm;

    CombatResult result;
    REQUIRE_NOTHROW(result = sm.run_until_end());  // NENHUMA excecao pode escapar (contrato 2)

    REQUIRE(cap_triggered);                     // o cap disparou (contrato 3, capped confiavel)
    REQUIRE(result.rounds_elapsed <= kRoundCap + 1);  // nunca passa do cap (contrato 1)
    REQUIRE(result.outcome == CombatOutcome::Defeat);  // wipe da party, nao Fled
}

TEST_CASE("mira_sim: L2 parede dedicada - Jaci nunca cai a 0 enquanto so ela defende e "
          "absorve o trash canonico",
          "[domain][mira_sim]") {
    const BattleTrace t = run_single_mira_battle(Lote::L2_ParedeDedicada, MiraArm::D_F4Strong, 55);
    // Jaci (indice 2) e o unico defensor; com F4 forte (D), o inimigo quase nunca a mira -
    // e mesmo mirando, o Shield (Def 10) absorve o ataque do trash (raw=max(1,10-10)=1).
    REQUIRE_FALSE(t.fell[idx(PartyMember::Jaci)]);
}

// TESTE PONTA-A-PONTA de L4 (achado do QA: "L4 e L5 nao tem NENHUM teste ponta a ponta...
// o QA matou o papel do honeypot... e a cura (V10 = 0) e a suite nao percebeu"). Roda o
// motor REAL e prova que a cura teve efeito MENSURAVEL: o HP final perdido de verdade
// (max_hp - hp final, somado nos 3 membros) tem de ser MENOR que o dano bruto recebido
// (damage_taken, que NAO desconta cura - e o dano no momento em que o golpe landou) -
// se a cura virasse no-op (V10=0 ou o heal() nunca fosse chamado), os dois valores
// bateriam exatamente (nenhuma cura pra compensar o dano recebido).
TEST_CASE("mira_sim: L4 suporte ativo - a cura da Jaci reduz o HP final perdido "
          "abaixo do dano bruto recebido (teste ponta-a-ponta, prova a cura de verdade)",
          "[domain][mira_sim]") {
    const BattleTrace t = run_single_mira_battle(Lote::L4_SuporteAtivo, MiraArm::A_StatusQuo, 11);
    const int total_damage_taken = t.damage_taken[0] + t.damage_taken[1] + t.damage_taken[2];
    REQUIRE(total_damage_taken > 0);  // sanidade: a luta realmente trocou golpes

    const std::array<int, kPartySize> max_hp{34, 55, 55};  // Gus/Caua/Jaci, combat.md secao 17
    int total_hp_lost_de_verdade = 0;
    for (int i = 0; i < kPartySize; ++i) {
        const int final_hp = static_cast<int>(std::lround(t.final_hp_fraction[i] * max_hp[i]));
        total_hp_lost_de_verdade += max_hp[i] - final_hp;
    }
    REQUIRE(total_hp_lost_de_verdade < total_damage_taken);
}

// TESTES PONTA-A-PONTA + UNITARIO de L5 (achado do QA: honeypot podia virar no-op sem
// quebrar a suite). O unitario prova que decide_party_action realmente marca os
// trackers; o E2E prova que o motor real completa a luta sem excecao com esses papeis.
TEST_CASE("mira_sim: L5 cartas de agro - HoneypotEntaoAtaca/FirewallEntaoAtaca marcam os "
          "trackers na 1a decisao da rodada 1 (unitario, mata o mutante 'vira no-op')",
          "[domain][mira_sim]") {
    using gus::domain::combat::CombatActor;

    // Cenario minimo: 1 heroi (Caua ou Gus) vs 1 inimigo, so pra ter um CombatState valido.
    CombatActor caua("caua", "caua", 55, 14, 8, 13, CardFamily::Eletrico, true);
    CombatActor enemy("sentinela1", "sentinela1", 55, 10, 8, 8, CardFamily::Cinetico, false);
    std::vector<CombatActor*> actor_ptrs{&caua, &enemy};

    AttractionTracker tr;
    BattleTrace trace;
    gus::domain::tests::PropertyRandom rng(1);
    auto noop_provider = [](CombatActor&, const gus::domain::combat::CombatState&) {
        return CombatAction::pass();
    };
    gus::domain::combat::CombatStateMachine sm(actor_ptrs, noop_provider, nullptr, nullptr, &rng);
    // refresh_resources_for_turn() e o que begin_turn() chama de verdade (secao 5): sem
    // isto ap()==0 (default de construcao) != max_ap(), e first_decision_this_turn daria
    // falso mesmo sendo a 1a decisao - o teste tem de espelhar o estado real de turno.
    caua.refresh_resources_for_turn(/*round_index=*/0);
    const gus::domain::combat::CombatState state(sm.queue(), &caua, /*round_index=*/0);

    REQUIRE(tr.honeypot_rounds_left[idx(PartyMember::Caua)] == 0);  // antes: inativo
    const CombatAction honeypot_action = decide_party_action(
        PartyMember::Caua, PartyRole::HoneypotEntaoAtaca, caua, state, sm, tr, trace, /*round=*/0);
    REQUIRE(tr.honeypot_rounds_left[idx(PartyMember::Caua)] == kHoneypotDurationRounds);
    REQUIRE(honeypot_action.type == gus::domain::combat::CombatActionType::ScanEnvironment);

    CombatActor gus_actor("gus", "gus", 34, 8, 5, 9, CardFamily::Eletrico, true);
    gus_actor.refresh_resources_for_turn(/*round_index=*/0);
    const gus::domain::combat::CombatState state_gus(sm.queue(), &gus_actor, /*round_index=*/0);
    REQUIRE(tr.firewall_rounds_left[idx(PartyMember::Gus)] == 0);  // antes: inativo
    const CombatAction firewall_action =
        decide_party_action(PartyMember::Gus, PartyRole::FirewallEntaoAtaca, gus_actor, state_gus,
                            sm, tr, trace, /*round=*/0);
    REQUIRE(tr.firewall_rounds_left[idx(PartyMember::Gus)] == kFirewallDurationRounds);
    REQUIRE(firewall_action.type == gus::domain::combat::CombatActionType::ScanEnvironment);
}

TEST_CASE("mira_sim: L5 cartas de agro - motor real completa a luta sem excecao "
          "(ponta-a-ponta)",
          "[domain][mira_sim]") {
    for (MiraArm arm : {MiraArm::A_StatusQuo, MiraArm::F_NoF4}) {
        const BattleTrace t = run_single_mira_battle(Lote::L5_CartasDeAgro, arm, 321);
        REQUIRE(t.outcome != CombatOutcome::Ongoing);
        REQUIRE(t.rounds >= 1);
    }
}

TEST_CASE("mira_sim: L6 aplica 70% do HP inicial na party (protocolo secao 2.2)",
          "[domain][mira_sim]") {
    // Sanidade indireta: roda 1 luta e confirma que o cenario nao lanca e produz um
      // resultado valido (a fracao de HP em si e testada via scenario_for acima).
    REQUIRE_NOTHROW(run_single_mira_battle(Lote::L6_VantagemInimigos, MiraArm::E_F4Inverted, 321));
}

// ============================================================================
// (c) Smoke estrutural dos 7 lotes x 6 bracos, N pequeno (sempre roda no CI). O RUN
//     GRANDE (10 milhoes, protocolo secao 3) NAO roda aqui - so o economy-designer/lider
//     dispara essa execucao depois de verificar esta entrega (fora do escopo desta fatia).
// ============================================================================

TEST_CASE("mira_sim: smoke - 7 lotes x 6 bracos, N pequeno, progresso no formato exato",
          "[domain][mira_sim][mira_sim_smoke]") {
    const bool full_run = std::getenv("GUSWORLD_MIRA_SIM_FULL") != nullptr;
    const int n_per_arm = full_run ? 240000 : 10;
    const std::uint32_t base_seed = 20260801;

    std::ostringstream progress;
    int lote_idx = 0;
    for (Lote lote : kAllLotes) {
        ++lote_idx;
        const std::vector<LoteArmReport> reports = run_lote_all_arms(
            lote, n_per_arm, base_seed, &progress, lote_idx, static_cast<int>(kAllLotes.size()));
        REQUIRE(reports.size() == kAllArms.size());
        for (const LoteArmReport& r : reports) {
            REQUIRE(r.n == n_per_arm);
            REQUIRE(r.win_rate_pct.value >= 0.0);
            REQUIRE(r.win_rate_pct.value <= 100.0);
            REQUIRE(r.mean_rounds >= 1.0);
        }
    }
    // Formato exato pedido: "lote [x] de [7], simulação [n] de [N_total]".
    const std::string out = progress.str();
    REQUIRE(out.find("lote [1] de [7], simulação [") != std::string::npos);
    REQUIRE(out.find("] de [" + std::to_string(n_per_arm * static_cast<int>(kAllArms.size())) + "]") !=
            std::string::npos);
    if (!full_run) std::cout << out;
}

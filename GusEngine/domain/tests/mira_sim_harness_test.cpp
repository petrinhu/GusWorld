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
    REQUIRE(l2[2] == R::DefendSelf);  // Jaci = parede
    REQUIRE(l2[0] == R::AttackFront);
    REQUIRE(l2[1] == R::AttackFront);

    const auto l3 = roles_for(Lote::L3_TurtleTotal);
    REQUIRE(l3 == std::array<R, 3>{R::DefendSelf, R::DefendSelf, R::DefendSelf});

    const auto l4 = roles_for(Lote::L4_SuporteAtivo);
    REQUIRE(l4[2] == R::HealMostInjured);  // Jaci cura

    const auto l5 = roles_for(Lote::L5_CartasDeAgro);
    REQUIRE(l5[1] == R::HoneypotEntaoAtaca);  // Caua
    REQUIRE(l5[0] == R::FirewallEntaoAtaca);  // Gus
    REQUIRE(l5[2] == R::AttackFront);         // Jaci

    REQUIRE(roles_for(Lote::L6_VantagemInimigos)[2] == R::DefendSelf);
    REQUIRE(roles_for(Lote::L7_AtacanteUnico)[2] == R::DefendSelf);
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

TEST_CASE("mira_sim: compute_target_weight aplica V1+F1+F2+F3 antes do F4/F5/F6",
          "[domain][mira_sim]") {
    AttractionTracker tr;
    auto full_hp = make_actor("caua", 55, 55, 8);
    const double w_full = compute_target_weight(PartyMember::Caua, full_hp, 0, tr, MiraArm::F_NoF4);
    REQUIRE(w_full == Catch::Approx(kBaseWeight));  // V1 sozinho: hp cheio, sem dano, sem suporte

    auto half_hp = make_actor("caua", 27, 55, 8);  // ~49% hp -> F2 soma ~51
    const double w_half = compute_target_weight(PartyMember::Caua, half_hp, 0, tr, MiraArm::F_NoF4);
    REQUIRE(w_half > w_full);  // F2 (ferido atrai) aumenta o peso

    tr.record_damage(PartyMember::Caua, 0, 20);  // F1: +20 de peso
    const double w_after_dmg =
        compute_target_weight(PartyMember::Caua, full_hp, 0, tr, MiraArm::F_NoF4);
    REQUIRE(w_after_dmg == Catch::Approx(kBaseWeight + 20.0));
}

TEST_CASE("mira_sim: compute_target_weight aplica F4 SO quando o alvo tem Shield ativo",
          "[domain][mira_sim]") {
    AttractionTracker tr;
    auto no_shield = make_actor("jaci", 55, 55, 10);
    const double w_no_shield =
        compute_target_weight(PartyMember::Jaci, no_shield, 0, tr, MiraArm::D_F4Strong);
    REQUIRE(w_no_shield == Catch::Approx(kBaseWeight));  // F4 nao entra sem Shield

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

    REQUIRE(w_shield_soft == Catch::Approx(kBaseWeight * 0.5));
    REQUIRE(w_shield_strong == Catch::Approx(kBaseWeight * 0.1));
    REQUIRE(w_shield_inverted == Catch::Approx(kBaseWeight * 2.0));
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
    REQUIRE(w_honeypot == Catch::Approx(kBaseWeight * kHoneypotWeightMultiplier));

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
    REQUIRE(r.pct_in_window_3_5.value == Catch::Approx(50.0));  // so a luta de 4 rounds entra
    REQUIRE(r.concentration_pct.value == Catch::Approx((100.0 + 33.333333333) / 2.0).epsilon(0.001));
    REQUIRE(r.pct_saco_de_pancadas.value == Catch::Approx(50.0));  // so a 1a bate >70%
    REQUIRE(r.pct_any_fall.value == Catch::Approx(50.0));
    REQUIRE(r.pct_fall_by_member[0].value == Catch::Approx(50.0));  // Gus caiu 1x em 2
    REQUIRE(r.pct_fall_by_member[1].value == Catch::Approx(0.0));
    REQUIRE(r.repeat_target_pct.value == Catch::Approx(100.0 * 5.0 / 8.0));  // (4+1)/(4+4)
    REQUIRE(r.mean_first_fall_round == Catch::Approx(5.0));
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
    REQUIRE(t.outcome == CombatOutcome::Fled);
    // H1 (parede renovavel): dano bruto tentado > 0, mas absorvido quase/totalmente.
    REQUIRE(t.shield_raw_total > 0);
    REQUIRE(t.shield_absorbed_total == t.shield_raw_total);
}

TEST_CASE("mira_sim: L2 parede dedicada - Jaci nunca cai a 0 enquanto so ela defende e "
          "absorve o trash canonico",
          "[domain][mira_sim]") {
    const BattleTrace t = run_single_mira_battle(Lote::L2_ParedeDedicada, MiraArm::D_F4Strong, 55);
    // Jaci (indice 2) e o unico defensor; com F4 forte (D), o inimigo quase nunca a mira -
    // e mesmo mirando, o Shield (Def 10) absorve o ataque do trash (raw=max(1,10-10)=1).
    REQUIRE_FALSE(t.fell[idx(PartyMember::Jaci)]);
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

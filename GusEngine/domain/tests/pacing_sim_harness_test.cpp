// SPDX-License-Identifier: Apache-2.0
// pacing_sim_harness_test.cpp
//
// Spec do harness de simulacao de PACING (MIRA-SIM 2). Ver pacing_sim_harness.hpp para o
// contrato completo. Reusa o motor real e a infraestrutura ja auditada de
// mira_sim_harness.hpp; os testes aqui cobrem SO o que e NOVO neste estudo-irmao.
//
// Cross-ref: pacing_sim_harness.hpp; docs/design/mecanicas/proposta-protocolo-simulacao-pacing.md.

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <cstdlib>
#include <sstream>

#include "pacing_sim_harness.hpp"

using namespace gus::domain::tests::pacing_sim;

// ============================================================================
// Eixos experimentais: os spec-builders aplicam X1 (HP-mult) e X2 (Atk) corretamente, com
// numeros DERIVADOS A MAO (nao adivinhados).
// ============================================================================

TEST_CASE("pacing_sim: sentinela_spec_x aplica X1/X2 sobre a referencia trash", "[domain][pacing_sim]") {
    PacingAxes axes;
    axes.hp_mult = 2.0;
    axes.atk = 25;
    const ActorSpec s = sentinela_spec_x("sentinela1", axes);
    REQUIRE(s.hp == 110);  // 55 * 2.0
    REQUIRE(s.atk == 25);
    REQUIRE(s.def == kTrashDef);
    CHECK(s.id == "sentinela1");
}

TEST_CASE("pacing_sim: sentinela_spec_x com hp_mult=1.0 reproduz a referencia exata", "[domain][pacing_sim]") {
    PacingAxes axes;  // defaults: hp_mult=1.0, atk=10
    const ActorSpec s = sentinela_spec_x("s", axes);
    REQUIRE(s.hp == kTrashHpReference);
    REQUIRE(s.atk == 10);
}

TEST_CASE("pacing_sim: sentinela_spec_reference ignora os eixos (escolta de P4 fica fixa)",
          "[domain][pacing_sim]") {
    const ActorSpec s = sentinela_spec_reference("escolta1");
    REQUIRE(s.hp == kTrashHpReference);
    REQUIRE(s.atk == 10);
    REQUIRE(s.def == kTrashDef);
}

TEST_CASE("pacing_sim: daemon_spec_x aplica X1/X2 sobre a referencia elite", "[domain][pacing_sim]") {
    PacingAxes axes;
    axes.hp_mult = 3.0 / 144.0;  // usado pelo teste de E11 abaixo - fixar o numero aqui tambem
    axes.atk = 1;
    const ActorSpec d = daemon_spec_x(axes);
    REQUIRE(d.hp == 3);
    REQUIRE(d.atk == 1);
    REQUIRE(d.def == kEliteDef);
    CHECK(d.id == "daemon");
}

// ============================================================================
// Tier / rotulo / braco fixo por tier (protocolo secao 2.1: trash=F_NoF4, elite=
// C_F4Soft - tabela tier-mira do economy-designer, fechada em 2026-08-01, substitui o
// placeholder inicial de B_Uniform; o lider rejeitou o sorteio uniforme explicitamente).
// ============================================================================

TEST_CASE("pacing_sim: tier_of classifica os 6 cenarios corretamente", "[domain][pacing_sim]") {
    CHECK(tier_of(Scenario::P1_TrashVanilla) == Tier::Trash);
    CHECK(tier_of(Scenario::P2_TrashParede) == Tier::Trash);
    CHECK(tier_of(Scenario::P3_TrashHealer) == Tier::Trash);
    CHECK(tier_of(Scenario::P4_EliteEscolta) == Tier::Elite);
    CHECK(tier_of(Scenario::P5_EliteSolo) == Tier::Elite);
    CHECK(tier_of(Scenario::P6_TurtleTotal) == Tier::Trash);
}

TEST_CASE("pacing_sim: fixed_arm_for casa com a tabela tier-mira do economy-designer (2026-08-01)",
          "[domain][pacing_sim]") {
    CHECK(fixed_arm_for(Tier::Trash) == MiraArm::F_NoF4);
    CHECK(fixed_arm_for(Tier::Elite) == MiraArm::C_F4Soft);
}

TEST_CASE("pacing_sim: scenario_label nao repete rotulo entre cenarios", "[domain][pacing_sim]") {
    const std::array<Scenario, 6> all = {Scenario::P1_TrashVanilla, Scenario::P2_TrashParede,
                                         Scenario::P3_TrashHealer, Scenario::P4_EliteEscolta,
                                         Scenario::P5_EliteSolo, Scenario::P6_TurtleTotal};
    for (std::size_t i = 0; i < all.size(); ++i)
        for (std::size_t j = i + 1; j < all.size(); ++j)
            CHECK(scenario_label(all[i]) != scenario_label(all[j]));
}

// ============================================================================
// Papeis de party por cenario (protocolo secao 2.2/2.3: sem honeypot/firewall).
// ============================================================================

TEST_CASE("pacing_sim: roles_for P1 e vanilla, os 3 atacam", "[domain][pacing_sim]") {
    const auto roles = roles_for(Scenario::P1_TrashVanilla);
    for (const PacingRole r : roles) CHECK(r == PacingRole::AttackFront);
}

TEST_CASE("pacing_sim: roles_for P2 poe a Jaci de parede", "[domain][pacing_sim]") {
    const auto roles = roles_for(Scenario::P2_TrashParede);
    CHECK(roles[idx(PartyMember::Gus)] == PacingRole::AttackFront);
    CHECK(roles[idx(PartyMember::Caua)] == PacingRole::AttackFront);
    CHECK(roles[idx(PartyMember::Jaci)] == PacingRole::DefendThenAttack);
}

TEST_CASE("pacing_sim: roles_for P3 poe a Jaci curando", "[domain][pacing_sim]") {
    const auto roles = roles_for(Scenario::P3_TrashHealer);
    CHECK(roles[idx(PartyMember::Jaci)] == PacingRole::HealMostInjured);
}

TEST_CASE("pacing_sim: roles_for P6 e turtle total, os 3 so defendem", "[domain][pacing_sim]") {
    const auto roles = roles_for(Scenario::P6_TurtleTotal);
    for (const PacingRole r : roles) CHECK(r == PacingRole::DefendOnlyTurtle);
}

// ============================================================================
// Composicao de cenario (party/inimigos/fracao de HP inicial).
// ============================================================================

TEST_CASE("pacing_sim: setup_for P1/P2/P3/P6 tem 3 sentinelas e party a 100% HP",
          "[domain][pacing_sim]") {
    PacingAxes axes;
    for (const Scenario s : {Scenario::P1_TrashVanilla, Scenario::P2_TrashParede,
                             Scenario::P3_TrashHealer, Scenario::P6_TurtleTotal}) {
        const PacingScenarioSetup setup = setup_for(s, axes);
        CHECK(setup.enemies.size() == 3);
        CHECK(setup.party.size() == kPartySize);
        CHECK(setup.party_hp_fraction_start == Catch::Approx(1.0));
    }
}

TEST_CASE("pacing_sim: setup_for P4 tem 1 Daemon-Guard(X) + 2 Sentinela(referencia), party a 70%",
          "[domain][pacing_sim]") {
    PacingAxes axes;
    axes.hp_mult = 2.0;
    axes.atk = 99;
    const PacingScenarioSetup setup = setup_for(Scenario::P4_EliteEscolta, axes);
    REQUIRE(setup.enemies.size() == 3);
    CHECK(setup.enemies[0].id == "daemon");
    CHECK(setup.enemies[0].hp == kEliteHpReference * 2);
    CHECK(setup.enemies[0].atk == 99);
    // Escolta fica na REFERENCIA - NAO segue os eixos do daemon (protocolo secao 2.2).
    CHECK(setup.enemies[1].hp == kTrashHpReference);
    CHECK(setup.enemies[1].atk == 10);
    CHECK(setup.enemies[2].hp == kTrashHpReference);
    CHECK(setup.party_hp_fraction_start == Catch::Approx(0.70));
}

TEST_CASE("pacing_sim: setup_for P5 tem so 1 Daemon-Guard(X), party a 100%", "[domain][pacing_sim]") {
    PacingAxes axes;
    axes.hp_mult = 5.0;
    const PacingScenarioSetup setup = setup_for(Scenario::P5_EliteSolo, axes);
    REQUIRE(setup.enemies.size() == 1);
    CHECK(setup.enemies[0].id == "daemon");
    CHECK(setup.enemies[0].hp == kEliteHpReference * 5);
    CHECK(setup.party_hp_fraction_start == Catch::Approx(1.0));
}

// ============================================================================
// E11 (golpes para derrubar o 1o inimigo abatido) - numero DERIVADO A MAO: daemon com
// HP=3 (axes.hp_mult=3/144), atk=1 (dano ao daemon E da party sempre = kMinDamage=1, ja
// que atk-def < 0 ou = 0 em toda combinacao usada aqui - ver contas no corpo do arquivo de
// harness). 3 pontos de HP / 1 dano por golpe = exatamente 3 golpes ate a queda, qualquer
// que seja a ordem de turno (CTB por SPD) ou quantos golpes cabem no mesmo turno (AP=3).
// ============================================================================

TEST_CASE("pacing_sim: E11 conta exatamente 3 golpes para derrubar um daemon de HP=3",
          "[domain][pacing_sim]") {
    PacingAxes axes;
    axes.hp_mult = 3.0 / 144.0;  // -> hp=3 (ver teste do spec-builder acima)
    axes.atk = 1;                // dano do daemon = max(1, 1-def) = 1 sempre (nao mata a party)
    const PacingBattleTrace trace = run_single_pacing_battle(Scenario::P5_EliteSolo, axes, 12345u);

    REQUIRE_FALSE(trace.base.internal_error);
    REQUIRE(trace.base.outcome == CombatOutcome::Victory);  // daemon HP=3 cai rapido, ninguem morre
    REQUIRE(trace.hits_to_kill_first_enemy == 3);
}

TEST_CASE("pacing_sim: E11 fica -1 quando a luta capa sem nenhum inimigo cair",
          "[domain][pacing_sim]") {
    PacingAxes axes;
    axes.hp_mult = 1000.0;  // daemon INDESTRUTIVEL dentro do cap
    axes.atk = 1;
    const PacingBattleTrace trace = run_single_pacing_battle(Scenario::P5_EliteSolo, axes, 777u);

    REQUIRE_FALSE(trace.base.internal_error);
    REQUIRE(trace.base.capped);
    REQUIRE(trace.hits_to_kill_first_enemy == -1);
}

// ============================================================================
// Gate de AP generalizado (X3): elite_ap=2 deixa o daemon agir 2x por turno PROPRIO, contra
// 1x do trash (sempre) e do elite quando X3=1. Cenario com daemon indestrutivel (mesmo dos
// dois testes acima) pra isolar SO o efeito do gate, sem risco de a luta acabar antes.
// ============================================================================

TEST_CASE("pacing_sim: elite_ap=2 produz mais acoes do inimigo que elite_ap=1 na mesma luta capada",
          "[domain][pacing_sim]") {
    PacingAxes axes_ap1;
    axes_ap1.hp_mult = 1000.0;
    axes_ap1.atk = 1;
    axes_ap1.elite_ap = 1;
    const PacingBattleTrace t1 = run_single_pacing_battle(Scenario::P5_EliteSolo, axes_ap1, 42u);

    PacingAxes axes_ap2 = axes_ap1;
    axes_ap2.elite_ap = 2;
    const PacingBattleTrace t2 = run_single_pacing_battle(Scenario::P5_EliteSolo, axes_ap2, 42u);

    REQUIRE_FALSE(t1.base.internal_error);
    REQUIRE_FALSE(t2.base.internal_error);
    REQUIRE(t1.base.capped);
    REQUIRE(t2.base.capped);
    // Mesmo numero de rodadas (o cap e por RODADA, nao por acao - ver kRoundCap), mas o
    // daemon com AP=2 agiu estritamente mais vezes que o de AP=1.
    CHECK(t1.base.rounds == t2.base.rounds);
    CHECK(t2.base.enemy_target_turns > t1.base.enemy_target_turns);
}

// ============================================================================
// Cura parametrizavel (X4): heal_amount maior cura mais HP por uso (P3).
// ============================================================================

TEST_CASE("pacing_sim: heal_amount (X4) maior deixa a party com HP final mais alto em P3",
          "[domain][pacing_sim]") {
    PacingAxes axes_low;
    axes_low.heal_amount = 1;
    const PacingBattleTrace low = run_single_pacing_battle(Scenario::P3_TrashHealer, axes_low, 2026u);

    PacingAxes axes_high = axes_low;
    axes_high.heal_amount = 50;
    const PacingBattleTrace high = run_single_pacing_battle(Scenario::P3_TrashHealer, axes_high, 2026u);

    REQUIRE_FALSE(low.base.internal_error);
    REQUIRE_FALSE(high.base.internal_error);
    const double sum_low = low.base.final_hp_fraction[0] + low.base.final_hp_fraction[1] +
                          low.base.final_hp_fraction[2];
    const double sum_high = high.base.final_hp_fraction[0] + high.base.final_hp_fraction[1] +
                           high.base.final_hp_fraction[2];
    CHECK(sum_high >= sum_low);
}

// ============================================================================
// aggregate_pacing sobre PacingBattleTrace SINTETICO (numeros DERIVADOS A MAO, mesmo
// estilo do mira_sim_harness_test.cpp - nao roda o motor real, isola so a agregacao).
// ============================================================================

TEST_CASE("pacing_sim: aggregate_pacing calcula E1/E2/E3/E4/E6/E8/E9/E10/E11 sobre trace sintetico",
          "[domain][pacing_sim]") {
    PacingBattleTrace vitoria_limpa;
    vitoria_limpa.base.outcome = CombatOutcome::Victory;
    vitoria_limpa.base.rounds = 4;
    vitoria_limpa.base.damage_taken = {0, 0, 0};  // E9: sem dano nenhum
    vitoria_limpa.base.fell = {false, false, false};
    vitoria_limpa.base.final_hp_fraction = {1.0, 1.0, 1.0};
    vitoria_limpa.hits_to_kill_first_enemy = 3;  // E11

    PacingBattleTrace derrota_espalhada;
    derrota_espalhada.base.outcome = CombatOutcome::Defeat;
    derrota_espalhada.base.rounds = 6;
    derrota_espalhada.base.damage_taken = {20, 10, 5};  // total=35, concentracao=20/35
    derrota_espalhada.base.fell = {true, false, false};
    derrota_espalhada.base.first_fall_round = 3;
    derrota_espalhada.base.final_hp_fraction = {0.0, 0.5, 0.7};
    derrota_espalhada.hits_to_kill_first_enemy = -1;  // ninguem do inimigo caiu

    const PacingPointReport r =
        aggregate_pacing("P1_teste", Tier::Trash, {vitoria_limpa, derrota_espalhada});

    REQUIRE(r.n == 2);
    // E1
    REQUIRE(r.win_rate_pct.value == Catch::Approx(50.0));
    REQUIRE(r.defeat_rate_pct.value == Catch::Approx(50.0));
    REQUIRE(r.internal_error_pct.value == Catch::Approx(0.0));
    // E2 (mediana != p90 com estas 2 amostras, discrimina mutante de percentil)
    REQUIRE(r.mean_rounds == Catch::Approx(5.0));
    REQUIRE(r.median_rounds == Catch::Approx(5.0));
    REQUIRE(r.p10_rounds == Catch::Approx(4.2));
    REQUIRE(r.p90_rounds == Catch::Approx(5.8));
    REQUIRE(r.pct_in_window_3_5.value == Catch::Approx(50.0));  // so a luta de 4 rounds entra
    // E3 (so 1 amostra valida - a de dano zero vira E9, nao entra na concentracao)
    REQUIRE(r.concentration_pct.value == Catch::Approx(100.0 * 20.0 / 35.0));
    REQUIRE(r.pct_saco_de_pancadas.value == Catch::Approx(0.0));  // 57.1% < 70%
    // E4
    REQUIRE(r.pct_any_fall.value == Catch::Approx(50.0));
    REQUIRE(r.pct_fall_by_member[idx(PartyMember::Gus)].value == Catch::Approx(50.0));
    REQUIRE(r.pct_fall_by_member[idx(PartyMember::Caua)].value == Catch::Approx(0.0));
    // E8
    REQUIRE(r.mean_first_fall_round == Catch::Approx(3.0));
    REQUIRE(r.median_first_fall_round == Catch::Approx(3.0));
    // E9: 1 de 2 lutas sem dano (50%); HP final medio = (3.0+1.2)/6 = 70%
    REQUIRE(r.pct_no_damage_taken.value == Catch::Approx(50.0));
    REQUIRE(r.avg_final_hp_pct == Catch::Approx(70.0));
    // E10: dano medio = (0+35)/2 = 17.5 HP; em % do pool de 144 = (0+24.305...)/2
    REQUIRE(r.mean_damage_taken_hp.value == Catch::Approx(17.5));
    REQUIRE(r.mean_damage_taken_pct_pool.value == Catch::Approx((0.0 + 100.0 * 35.0 / 144.0) / 2.0));
    // E11: SO a luta com hits_to_kill_first_enemy>=0 entra na amostra (1 de 2)
    REQUIRE(r.battles_with_a_kill == 1);
    REQUIRE(r.mean_hits_to_kill == Catch::Approx(3.0));
    REQUIRE(r.median_hits_to_kill == Catch::Approx(3.0));
}

TEST_CASE("pacing_sim: aggregate_pacing com vetor vazio nao explode", "[domain][pacing_sim]") {
    const PacingPointReport r = aggregate_pacing("vazio", Tier::Trash, {});
    REQUIRE(r.n == 0);
    REQUIRE(r.battles_with_a_kill == 0);
}

// Erro interno PULA o resto do bookkeeping (mesma disciplina do MIRA-SIM, achado do
// team-lead 2026-08-01): a luta com erro nao pode poluir E9/E10/E11 com dado fabricado.
TEST_CASE("pacing_sim: aggregate_pacing exclui luta com erro interno de E9/E10/E11",
          "[domain][pacing_sim]") {
    PacingBattleTrace erro;
    erro.base.internal_error = true;
    erro.base.internal_error_message = "erro sintetico de teste";
    erro.base.damage_taken = {999, 999, 999};  // lixo - NUNCA deve aparecer em E10
    erro.hits_to_kill_first_enemy = 999;        // lixo - NUNCA deve aparecer em E11

    PacingBattleTrace normal;
    normal.base.outcome = CombatOutcome::Victory;
    normal.base.rounds = 4;
    normal.base.damage_taken = {5, 0, 0};
    normal.base.final_hp_fraction = {0.9, 1.0, 1.0};
    normal.hits_to_kill_first_enemy = 2;

    const PacingPointReport r = aggregate_pacing("erro-teste", Tier::Trash, {erro, normal});
    REQUIRE(r.n == 2);
    REQUIRE(r.internal_errors_count == 1);
    REQUIRE(r.internal_error_pct.value == Catch::Approx(50.0));
    REQUIRE(r.first_internal_error_message == "erro sintetico de teste");
    // valid_n=1: so a luta normal conta nos denominadores validos.
    REQUIRE(r.mean_damage_taken_hp.value == Catch::Approx(5.0));  // NAO 502.0 (media com o 999)
    REQUIRE(r.battles_with_a_kill == 1);
    REQUIRE(r.mean_hits_to_kill == Catch::Approx(2.0));  // NAO poluido pelo 999 do erro
}

// ============================================================================
// Guarda-corpos (protocolo secao 4.1): verde/vermelho contra os numeros FIXOS do
// pre-registro do lider, aprovados 2026-08-01 ANTES de qualquer dado.
// ============================================================================

TEST_CASE("pacing_sim: evaluate_guardrails aprova um ponto trash dentro de todas as faixas",
          "[domain][pacing_sim]") {
    PacingPointReport r;
    r.tier = Tier::Trash;
    r.win_rate_pct.value = 93.0;                              // dentro de 90-97
    r.pct_fall_by_member[idx(PartyMember::Gus)].value = 10.0;  // dentro do teto 12
    r.p10_rounds = 3.0;
    r.p90_rounds = 5.0;  // dentro de 2-7
    r.pct_no_damage_taken.value = 20.0;  // <= 40
    r.avg_final_hp_pct = 70.0;           // dentro de 55-90

    const auto verdicts = evaluate_guardrails(r);
    REQUIRE(verdicts.size() == 5);  // trash: E1+E4+E2-cauda+E9-trivial+E9-hp
    for (const GuardrailVerdict& v : verdicts) CHECK(v.green);
}

TEST_CASE("pacing_sim: evaluate_guardrails reprova taxa de vitoria trash fora da faixa",
          "[domain][pacing_sim]") {
    PacingPointReport r;
    r.tier = Tier::Trash;
    r.win_rate_pct.value = 50.0;  // MUITO abaixo de 90-97
    r.pct_fall_by_member[idx(PartyMember::Gus)].value = 10.0;
    r.p10_rounds = 3.0;
    r.p90_rounds = 5.0;
    r.pct_no_damage_taken.value = 20.0;
    r.avg_final_hp_pct = 70.0;

    const auto verdicts = evaluate_guardrails(r);
    REQUIRE_FALSE(verdicts[0].green);  // E1 e sempre o 1o
    CHECK(verdicts[0].label.find("E1") != std::string::npos);
    // os demais continuam verdes - um guarda-corpo vermelho nao contamina os outros.
    for (std::size_t i = 1; i < verdicts.size(); ++i) CHECK(verdicts[i].green);
}

TEST_CASE("pacing_sim: evaluate_guardrails reprova quedas do Gus acima do teto elite",
          "[domain][pacing_sim]") {
    PacingPointReport r;
    r.tier = Tier::Elite;
    r.win_rate_pct.value = 65.0;  // dentro de 55-80
    r.pct_fall_by_member[idx(PartyMember::Gus)].value = 41.0;  // MUITO acima do teto 40
    r.p10_rounds = 3.0;
    r.p90_rounds = 6.0;
    r.avg_final_hp_pct = 60.0;  // dentro do teto elite (<=70)

    const auto verdicts = evaluate_guardrails(r);
    REQUIRE(verdicts.size() == 4);  // elite: E1+E4+E2-cauda+E9-hp (so 1 check de E9)
    CHECK(verdicts[0].green);
    REQUIRE_FALSE(verdicts[1].green);  // E4 e o 2o
    CHECK(verdicts[1].label.find("E4") != std::string::npos);
}

TEST_CASE("pacing_sim: evaluate_guardrails reprova cauda de E2 fora do envelope",
          "[domain][pacing_sim]") {
    PacingPointReport r;
    r.tier = Tier::Trash;
    r.win_rate_pct.value = 93.0;
    r.pct_fall_by_member[idx(PartyMember::Gus)].value = 10.0;
    r.p10_rounds = 1.0;  // abaixo do piso de 2
    r.p90_rounds = 9.0;  // acima do teto de 7
    r.pct_no_damage_taken.value = 20.0;
    r.avg_final_hp_pct = 70.0;

    const auto verdicts = evaluate_guardrails(r);
    REQUIRE_FALSE(verdicts[2].green);  // E2-cauda e o 3o
    CHECK(verdicts[2].label.find("E2") != std::string::npos);
}

// ============================================================================
// Orquestracao de UM ponto: progresso no formato EXATO do protocolo (secao 8 item 6),
// mesma disciplina do smoke test de run_lote_all_arms no MIRA-SIM.
// ============================================================================

TEST_CASE("pacing_sim: run_point_battles imprime progresso no formato exato",
          "[domain][pacing_sim]") {
    PacingAxes axes;
    std::ostringstream progress;
    const std::vector<PacingBattleTrace> traces = run_point_battles(
        Scenario::P1_TrashVanilla, axes, /*n=*/5, /*base_seed=*/20260801u, &progress,
        /*phase=*/'A', /*point_idx_1based=*/2, /*point_total=*/7);

    REQUIRE(traces.size() == 5);
    const std::string out = progress.str();
    REQUIRE(out.find("fase [A], ponto [2] de [7], simulação [") != std::string::npos);
    REQUIRE(out.find("] de [5]") != std::string::npos);
}

// ============================================================================
// Relatorio de duas camadas: smoke - roda um ponto pequeno de verdade (motor real),
// agrega, imprime as 2 camadas e confere que as secoes esperadas aparecem. N pequeno de
// proposito (o disparo do estudo cheio e do team-lead/lider, protocolo secao 7 item 1 -
// mesma disciplina do run_full_study do MIRA-SIM, nunca chamado por este harness).
// ============================================================================

TEST_CASE("pacing_sim: relatorio de 2 camadas imprime E1-E11 e os guarda-corpos",
          "[domain][pacing_sim]") {
    PacingAxes axes;
    const std::vector<PacingBattleTrace> traces =
        run_point_battles(Scenario::P1_TrashVanilla, axes, /*n=*/20, /*base_seed=*/42u, nullptr,
                          'A', 1, 1);
    const PacingPointReport r = aggregate_pacing(scenario_label(Scenario::P1_TrashVanilla),
                                                 tier_of(Scenario::P1_TrashVanilla), traces);

    std::ostringstream out;
    REQUIRE_NOTHROW(print_point_report_layer1(out, r));
    REQUIRE_NOTHROW(print_point_guardrails_layer2(out, r));
    const std::string text = out.str();
    REQUIRE_FALSE(text.empty());
    CHECK(text.find("E1") != std::string::npos);
    CHECK(text.find("E9") != std::string::npos);
    CHECK(text.find("E10") != std::string::npos);
    CHECK(text.find("E11") != std::string::npos);
    CHECK(text.find("guarda-corpos") != std::string::npos);
    CHECK(text.find("erros internos: 0 de 20") != std::string::npos);
}

// ============================================================================
// Reuso-seguranca: mira_sim_harness.hpp continua intacto (a suite dele, ja auditada por 2
// QAs, segue existindo e passando - ver CMakeLists.txt, ambos os arquivos .cpp registrados
// no mesmo executavel gusengine_domain_tests).
// ============================================================================

TEST_CASE("pacing_sim: reutiliza kRoundCap do estudo-pai sem redefinir", "[domain][pacing_sim]") {
    REQUIRE(kRoundCap == 30);
}

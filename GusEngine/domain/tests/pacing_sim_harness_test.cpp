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

// Achado do QA 2026-08-01: `>=` na soma final aceita EMPATE, e um mutante que
// desconecta o eixo (heal(heal_amount) -> heal(12) fixo) faz as duas lutas ficarem
// BYTE-IDENTICAS sob a mesma seed - sum_high==sum_low, e o `>=` engole a igualdade em
// silencio ("o teste passa justamente porque o eixo parou de funcionar"). Duas defesas
// agora: (1) `>` estrito em vez de `>=`; (2) as arrays de final_hp_fraction NAO podem
// ser identicas - isso mata o mutante de raiz, mesmo que algum dia a soma empate por
// coincidencia com o eixo ainda ligado.
TEST_CASE("pacing_sim: heal_amount (X4) maior deixa a party com HP final estritamente mais alto em P3",
          "[domain][pacing_sim]") {
    PacingAxes axes_low;
    axes_low.heal_amount = 1;
    const PacingBattleTrace low = run_single_pacing_battle(Scenario::P3_TrashHealer, axes_low, 2026u);

    PacingAxes axes_high = axes_low;
    axes_high.heal_amount = 50;
    const PacingBattleTrace high = run_single_pacing_battle(Scenario::P3_TrashHealer, axes_high, 2026u);

    REQUIRE_FALSE(low.base.internal_error);
    REQUIRE_FALSE(high.base.internal_error);
    // Prova direta de que o eixo teve EFEITO (nao so na soma - nas arrays inteiras):
    // se um mutante desconectar heal_amount, as duas lutas viram identicas sob a mesma
    // seed e esta comparacao falha antes mesmo de olhar a soma.
    REQUIRE_FALSE(low.base.final_hp_fraction == high.base.final_hp_fraction);

    const double sum_low = low.base.final_hp_fraction[0] + low.base.final_hp_fraction[1] +
                          low.base.final_hp_fraction[2];
    const double sum_high = high.base.final_hp_fraction[0] + high.base.final_hp_fraction[1] +
                           high.base.final_hp_fraction[2];
    CHECK(sum_high > sum_low);  // ESTRITO - `>=` aceitaria o empate do eixo desligado
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

// Achado do QA 2026-08-01: trocar valid_n por r.n em pct_in_window_3_5 (E2) ou em
// pct_no_damage_taken (E9) sobrevivia a suite - nenhum teste distinguia os dois
// denominadores. 1 erro + 3 lutas validas (2 na janela 3-5, 1 fora; 2 sem dano, 1 com
// dano): valid_n=3 da 66.67%, mas r.n=4 (INCLUI o erro) daria 50% - os dois numeros sao
// bem separados, entao qualquer substituicao do denominador quebra esta asserção.
TEST_CASE("pacing_sim: aggregate_pacing usa valid_n (nao r.n) como denominador de E2 e do "
          "lado sem-dano de E9",
          "[domain][pacing_sim]") {
    PacingBattleTrace erro;
    erro.base.internal_error = true;
    erro.base.internal_error_message = "erro sintetico";

    PacingBattleTrace vitoria_janela_sem_dano_1;
    vitoria_janela_sem_dano_1.base.outcome = CombatOutcome::Victory;
    vitoria_janela_sem_dano_1.base.rounds = 4;  // dentro de [3,5]
    vitoria_janela_sem_dano_1.base.damage_taken = {0, 0, 0};

    PacingBattleTrace vitoria_janela_sem_dano_2 = vitoria_janela_sem_dano_1;

    PacingBattleTrace derrota_fora_com_dano;
    derrota_fora_com_dano.base.outcome = CombatOutcome::Defeat;
    derrota_fora_com_dano.base.rounds = 10;  // FORA de [3,5]
    derrota_fora_com_dano.base.damage_taken = {5, 0, 0};  // dano > 0

    const PacingPointReport r = aggregate_pacing(
        "denominador-teste", Tier::Trash,
        {erro, vitoria_janela_sem_dano_1, vitoria_janela_sem_dano_2, derrota_fora_com_dano});

    REQUIRE(r.n == 4);
    REQUIRE(r.internal_errors_count == 1);
    // valid_n=3: 2 de 3 na janela (66.67%). Se o denominador fosse r.n=4, daria 50%.
    REQUIRE(r.pct_in_window_3_5.value == Catch::Approx(200.0 / 3.0));
    REQUIRE(r.pct_in_window_3_5.value != Catch::Approx(50.0));
    // valid_n=3: 2 de 3 sem dano (66.67%). Mesma prova para o lado "sem-dano" de E9.
    REQUIRE(r.pct_no_damage_taken.value == Catch::Approx(200.0 / 3.0));
    REQUIRE(r.pct_no_damage_taken.value != Catch::Approx(50.0));
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
    r.pct_any_fall.value = 30.0;         // ha queda...
    r.median_first_fall_round = 4.0;     // ...mas fora das rodadas 1-2 (E8 verde)

    const auto verdicts = evaluate_guardrails(r);
    REQUIRE(verdicts.size() == 6);  // trash: E1+E4+E2-cauda+E9-trivial+E9-hp+E8
    for (const GuardrailVerdict& v : verdicts) CHECK(v.green);
}

// Achado do QA 2026-08-01: o harness JA calculava median_first_fall_round mas nenhum
// guarda-corpo bloqueava - um candidato com paulada na rodada 1 ou 2 passava por todos
// os checks. Regra do protocolo (secao 4, E8): "Nenhum candidato com mediana de 1a
// queda nas rodadas 1-2 no trash".
TEST_CASE("pacing_sim: evaluate_guardrails reprova E8 (mediana da 1a queda na rodada 1-2, trash)",
          "[domain][pacing_sim]") {
    PacingPointReport r;
    r.tier = Tier::Trash;
    r.win_rate_pct.value = 93.0;
    r.pct_fall_by_member[idx(PartyMember::Gus)].value = 10.0;
    r.p10_rounds = 3.0;
    r.p90_rounds = 5.0;
    r.pct_no_damage_taken.value = 20.0;
    r.avg_final_hp_pct = 70.0;
    r.pct_any_fall.value = 40.0;       // ha queda de verdade
    r.median_first_fall_round = 1.5;   // mediana DENTRO das rodadas 1-2 (paulada)

    const auto verdicts = evaluate_guardrails(r);
    REQUIRE(verdicts.size() == 6);
    REQUIRE_FALSE(verdicts[5].green);  // E8 e o ultimo (so trash)
    CHECK(verdicts[5].label.find("E8") != std::string::npos);
    // os demais nao sao contaminados por este guarda-corpo vermelho.
    for (std::size_t i = 0; i < 5; ++i) CHECK(verdicts[i].green);
}

// Achado do QA 2026-08-01 (segunda rodada): o teste acima usa mediana=1.5 e o teste
// "vacuamente verde" usa mediana=0.0 - nenhum dos dois exercita EXATAMENTE 1.0 ou 2.0,
// entao mutar kFirstFallBadRoundMin (1.0->0.0), kFirstFallBadRoundMax (2.0->3.0), ou o
// operador de inclusivo pra exclusivo sobrevivia em silencio. Mesma disciplina dos 5
// guarda-corpos numericos: fronteira EXATA (reprova em cima do limite) MAIS 1 passo pra
// FORA (aprova logo apos o limite) - so as DUAS pontas juntas provam largura E operador.
// So a fronteira exata reprovando nao bastaria: alargar o limite (min 1.0->0.0 ou max
// 2.0->3.0) mantem 1.0/2.0 dentro do intervalo ruim alargado, e sobreviveria em
// silencio - e o exato mesmo mutante que o QA reportou.
TEST_CASE("pacing_sim: evaluate_guardrails reprova E8 na fronteira EXATA e aprova 1 passo fora",
          "[domain][pacing_sim]") {
    auto make_baseline = []() {
        PacingPointReport r;
        r.tier = Tier::Trash;
        r.win_rate_pct.value = 93.0;
        r.pct_fall_by_member[idx(PartyMember::Gus)].value = 10.0;
        r.p10_rounds = 3.0;
        r.p90_rounds = 5.0;
        r.pct_no_damage_taken.value = 20.0;
        r.avg_final_hp_pct = 70.0;
        r.pct_any_fall.value = 40.0;  // ha queda de verdade, entao E8 realmente avalia
        return r;
    };

    {
        PacingPointReport r = make_baseline();
        r.median_first_fall_round = 1.0;  // exatamente o piso do intervalo ruim [1,2]
        REQUIRE_FALSE(evaluate_guardrails(r)[5].green);
    }
    {
        PacingPointReport r = make_baseline();
        r.median_first_fall_round = 2.0;  // exatamente o teto do intervalo ruim [1,2]
        REQUIRE_FALSE(evaluate_guardrails(r)[5].green);
    }
    {
        // Mata o mutante que alarga kFirstFallBadRoundMin (1.0->0.0): sob o codigo
        // correto isto APROVA (0.9 esta fora do intervalo ruim); sob o mutante
        // alargado, 0.9 cairia dentro de [0,2] e reprovaria.
        PacingPointReport r = make_baseline();
        r.median_first_fall_round = 0.9;
        CHECK(evaluate_guardrails(r)[5].green);
    }
    {
        // Mata o mutante que alarga kFirstFallBadRoundMax (2.0->3.0): sob o codigo
        // correto isto APROVA (2.1 esta fora do intervalo ruim); sob o mutante
        // alargado, 2.1 cairia dentro de [1,3] e reprovaria.
        PacingPointReport r = make_baseline();
        r.median_first_fall_round = 2.1;
        CHECK(evaluate_guardrails(r)[5].green);
    }
}

// E8 nao pode reprovar quando NINGUEM cai (senao o default de percentile() sobre
// amostra vazia - 0.0 - viraria uma coincidencia fragil, nao uma regra provada).
TEST_CASE("pacing_sim: evaluate_guardrails E8 e vacuamente verde quando ninguem cai",
          "[domain][pacing_sim]") {
    PacingPointReport r;
    r.tier = Tier::Trash;
    r.win_rate_pct.value = 93.0;
    r.pct_fall_by_member[idx(PartyMember::Gus)].value = 10.0;
    r.p10_rounds = 3.0;
    r.p90_rounds = 5.0;
    r.pct_no_damage_taken.value = 20.0;
    r.avg_final_hp_pct = 70.0;
    r.pct_any_fall.value = 0.0;         // ninguem caiu em luta nenhuma
    r.median_first_fall_round = 0.0;    // default de aggregate_pacing sobre amostra vazia

    const auto verdicts = evaluate_guardrails(r);
    REQUIRE(verdicts.size() == 6);
    CHECK(verdicts[5].green);  // E8 nao reprova por falta de dado
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
// Fronteira exata dos guarda-corpos numericos (achado do QA 2026-08-01: trocar
// >=/<= por >/< sobrevivia a suite, porque nenhum teste usava o valor EXATO da
// fronteira - 90.0, 97.0, 12.0, 40.0, 2.0, 7.0, 55.0, 90.0, 70.0). O pre-registro do
// protocolo (secao 4.1) documenta teto/piso INCLUSIVOS; estes testes provam qual dos
// dois o codigo de fato faz.
// ============================================================================

// Decisao do lider 2026-08-01 (respondendo a pergunta do team-lead sobre o achado do
// QA): SO O TETO da queda do Gus importa, DE PROPOSITO, sem piso. Gus caindo quase
// nunca (aqui 0%) NAO reprova E4 - a trivialidade tem regua propria em E9. Este teste
// e o registro que evita reabrir a mesma pergunta numa proxima rodada de QA.
TEST_CASE("pacing_sim: evaluate_guardrails E4 nao tem piso - Gus caindo 0% e sempre verde",
          "[domain][pacing_sim]") {
    PacingPointReport r_trash;
    r_trash.tier = Tier::Trash;
    r_trash.win_rate_pct.value = 93.0;
    r_trash.pct_fall_by_member[idx(PartyMember::Gus)].value = 0.0;  // MUITO abaixo de 8-12%
    r_trash.p10_rounds = 3.0;
    r_trash.p90_rounds = 5.0;
    r_trash.pct_no_damage_taken.value = 20.0;
    r_trash.avg_final_hp_pct = 70.0;
    r_trash.pct_any_fall.value = 0.0;
    const auto verdicts_trash = evaluate_guardrails(r_trash);
    CHECK(verdicts_trash[1].green);  // E4 e o 2o - PASSA mesmo com queda zero do Gus

    PacingPointReport r_elite;
    r_elite.tier = Tier::Elite;
    r_elite.win_rate_pct.value = 65.0;
    r_elite.pct_fall_by_member[idx(PartyMember::Gus)].value = 0.0;  // MUITO abaixo de 30-40%
    r_elite.p10_rounds = 3.0;
    r_elite.p90_rounds = 6.0;
    r_elite.avg_final_hp_pct = 60.0;
    const auto verdicts_elite = evaluate_guardrails(r_elite);
    CHECK(verdicts_elite[1].green);  // mesma regra vale pro elite
}

TEST_CASE("pacing_sim: evaluate_guardrails aceita a fronteira MINIMA exata (inclusive), trash",
          "[domain][pacing_sim]") {
    PacingPointReport r;
    r.tier = Tier::Trash;
    r.win_rate_pct.value = 90.0;                              // exatamente o piso de E1
    r.pct_fall_by_member[idx(PartyMember::Gus)].value = 12.0;  // exatamente o teto de E4
    r.p10_rounds = 2.0;                                        // exatamente o piso de E2
    r.p90_rounds = 5.0;                                         // interior (nao testa o teto aqui)
    r.pct_no_damage_taken.value = 40.0;                         // exatamente o teto de E9
    r.avg_final_hp_pct = 55.0;                                  // exatamente o piso de E9 (trash)
    r.pct_any_fall.value = 0.0;

    const auto verdicts = evaluate_guardrails(r);
    for (const GuardrailVerdict& v : verdicts) CHECK(v.green);
}

TEST_CASE("pacing_sim: evaluate_guardrails aceita a fronteira MAXIMA exata (inclusive), trash",
          "[domain][pacing_sim]") {
    PacingPointReport r;
    r.tier = Tier::Trash;
    r.win_rate_pct.value = 97.0;  // exatamente o teto de E1
    r.pct_fall_by_member[idx(PartyMember::Gus)].value = 0.0;  // interior
    r.p10_rounds = 3.0;                                        // interior
    r.p90_rounds = 7.0;                                         // exatamente o teto de E2
    r.pct_no_damage_taken.value = 10.0;                         // interior
    r.avg_final_hp_pct = 90.0;                                  // exatamente o teto de E9 (trash)
    r.pct_any_fall.value = 0.0;

    const auto verdicts = evaluate_guardrails(r);
    for (const GuardrailVerdict& v : verdicts) CHECK(v.green);
}

TEST_CASE("pacing_sim: evaluate_guardrails aceita a fronteira exata do elite (E1 min/max, E4, E9)",
          "[domain][pacing_sim]") {
    PacingPointReport r_min;
    r_min.tier = Tier::Elite;
    r_min.win_rate_pct.value = 55.0;  // exatamente o piso de E1 elite
    r_min.pct_fall_by_member[idx(PartyMember::Gus)].value = 40.0;  // exatamente o teto de E4 elite
    r_min.p10_rounds = 3.0;
    r_min.p90_rounds = 6.0;
    r_min.avg_final_hp_pct = 70.0;  // exatamente o teto de E9 elite
    for (const GuardrailVerdict& v : evaluate_guardrails(r_min)) CHECK(v.green);

    PacingPointReport r_max = r_min;
    r_max.win_rate_pct.value = 80.0;  // exatamente o teto de E1 elite
    for (const GuardrailVerdict& v : evaluate_guardrails(r_max)) CHECK(v.green);
}

// Cada metrica, isolada, reprova a UM PASSO da fronteira - prova que o operador e o
// esperado (nao so que o valor exato passa, mas que passar do valor falha).
TEST_CASE("pacing_sim: evaluate_guardrails reprova 1 passo alem de CADA fronteira, isoladamente",
          "[domain][pacing_sim]") {
    auto make_baseline = []() {
        PacingPointReport r;
        r.tier = Tier::Trash;
        r.win_rate_pct.value = 93.0;
        r.pct_fall_by_member[idx(PartyMember::Gus)].value = 10.0;
        r.p10_rounds = 3.0;
        r.p90_rounds = 5.0;
        r.pct_no_damage_taken.value = 20.0;
        r.avg_final_hp_pct = 70.0;
        r.pct_any_fall.value = 0.0;
        return r;
    };

    {
        PacingPointReport r = make_baseline();
        r.win_rate_pct.value = 89.9;  // 1 passo abaixo do piso 90.0
        REQUIRE_FALSE(evaluate_guardrails(r)[0].green);
    }
    {
        PacingPointReport r = make_baseline();
        r.win_rate_pct.value = 97.1;  // 1 passo acima do teto 97.0
        REQUIRE_FALSE(evaluate_guardrails(r)[0].green);
    }
    {
        PacingPointReport r = make_baseline();
        r.pct_fall_by_member[idx(PartyMember::Gus)].value = 12.1;  // 1 passo acima do teto 12.0
        REQUIRE_FALSE(evaluate_guardrails(r)[1].green);
    }
    {
        PacingPointReport r = make_baseline();
        r.p10_rounds = 1.9;  // 1 passo abaixo do piso 2.0
        REQUIRE_FALSE(evaluate_guardrails(r)[2].green);
    }
    {
        PacingPointReport r = make_baseline();
        r.p90_rounds = 7.1;  // 1 passo acima do teto 7.0
        REQUIRE_FALSE(evaluate_guardrails(r)[2].green);
    }
    {
        PacingPointReport r = make_baseline();
        r.pct_no_damage_taken.value = 40.1;  // 1 passo acima do teto 40.0
        REQUIRE_FALSE(evaluate_guardrails(r)[3].green);
    }
    {
        PacingPointReport r = make_baseline();
        r.avg_final_hp_pct = 54.9;  // 1 passo abaixo do piso 55.0 (trash)
        REQUIRE_FALSE(evaluate_guardrails(r)[4].green);
    }
    {
        PacingPointReport r = make_baseline();
        r.avg_final_hp_pct = 90.1;  // 1 passo acima do teto 90.0 (trash)
        REQUIRE_FALSE(evaluate_guardrails(r)[4].green);
    }
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
    // Nota do QA 2026-08-01: E11 e definida no protocolo (secao 4) para P1; em P1 nao
    // deve aparecer o aviso de escopo (a leitura oficial e aqui mesmo).
    CHECK(text.find("NOTA E11") == std::string::npos);
}

// Achado do QA 2026-08-01: E11 e calculada e impressa em TODOS os 6 cenarios, mas o
// protocolo (secao 4) so a define para P1. Nao e bug (o numero e legitimo em qualquer
// cenario), mas quem ler o relatorio precisa saber que a leitura OFICIAL e so a linha
// de P1 - o aviso de escopo tem que aparecer nos outros 5 cenarios.
TEST_CASE("pacing_sim: relatorio avisa o escopo de E11 (protocolo define so P1) fora de P1",
          "[domain][pacing_sim]") {
    PacingAxes axes;
    const std::vector<PacingBattleTrace> traces =
        run_point_battles(Scenario::P5_EliteSolo, axes, /*n=*/10, /*base_seed=*/7u, nullptr,
                          'A', 1, 1);
    const PacingPointReport r = aggregate_pacing(scenario_label(Scenario::P5_EliteSolo),
                                                 tier_of(Scenario::P5_EliteSolo), traces);

    std::ostringstream out;
    print_point_report_layer1(out, r);
    CHECK(out.str().find("NOTA E11") != std::string::npos);
}

// ============================================================================
// Grade da Fase A (protocolo secao 2.1/3): 116 pontos = 80 trash + 36 elite. Numeros
// DERIVADOS A MAO da propria conta do protocolo (16 x 5 = 80; 18 x 2 = 36), conferida
// tambem pelo team-lead de forma independente.
// ============================================================================

TEST_CASE("pacing_sim: build_phase_a_trash_grid tem exatamente 80 pontos (16 x 5)",
          "[domain][pacing_sim]") {
    const auto grid = build_phase_a_trash_grid();
    REQUIRE(grid.size() == 80);
    for (const PacingGridPoint& gp : grid) REQUIRE(tier_of(gp.scenario) == Tier::Trash);
}

TEST_CASE("pacing_sim: build_phase_a_elite_grid tem exatamente 36 pontos (18 x 2)",
          "[domain][pacing_sim]") {
    const auto grid = build_phase_a_elite_grid();
    REQUIRE(grid.size() == 36);
    for (const PacingGridPoint& gp : grid) REQUIRE(tier_of(gp.scenario) == Tier::Elite);
}

TEST_CASE("pacing_sim: build_phase_a_grid concatena trash+elite em 116 pontos",
          "[domain][pacing_sim]") {
    const auto grid = build_phase_a_grid();
    REQUIRE(grid.size() == 116);  // 80 + 36, conferido tambem pelo team-lead
}

// X1 elite usa o multiplicador EXATO (HP alvo / referencia), nao o percentual
// arredondado do texto do protocolo ("62%/80%/100%") - 144*0.62 daria 89 (lround), nao
// 90. Prova que os 3 pontos batem EXATAMENTE em 90/115/144.
TEST_CASE("pacing_sim: grade elite da Fase A produz HP EXATO 90/115/144, nao arredondado",
          "[domain][pacing_sim]") {
    for (double mult : kPhaseAEliteHpMult) {
        PacingAxes axes;
        axes.hp_mult = mult;
        const int hp = daemon_spec_x(axes).hp;
        CHECK((hp == 90 || hp == 115 || hp == 144));
    }
    // os 3 valores estao todos presentes (nenhum colapsou em duplicata por
    // arredondamento).
    std::vector<int> hps;
    for (double mult : kPhaseAEliteHpMult) {
        PacingAxes axes;
        axes.hp_mult = mult;
        hps.push_back(daemon_spec_x(axes).hp);
    }
    std::sort(hps.begin(), hps.end());
    REQUIRE(hps == std::vector<int>{90, 115, 144});
}

TEST_CASE("pacing_sim: grade trash da Fase A produz HP EXATO 22/33/44/55", "[domain][pacing_sim]") {
    std::vector<int> hps;
    for (double mult : kPhaseATrashHpMult) {
        PacingAxes axes;
        axes.hp_mult = mult;
        hps.push_back(sentinela_spec_x("s", axes).hp);
    }
    REQUIRE(hps == std::vector<int>{22, 33, 44, 55});
}

// Cada ponto de P3 (trash) desdobra em 3 valores de X4 - prova que os 3 pontos gerados
// pra UM (X1,X2) fixo tem heal_amount 6/9/12 e nada mais se repete.
TEST_CASE("pacing_sim: build_phase_a_trash_grid desdobra P3 nos 3 valores de X4",
          "[domain][pacing_sim]") {
    const auto grid = build_phase_a_trash_grid();
    std::vector<int> heals;
    for (const PacingGridPoint& gp : grid)
        if (gp.scenario == Scenario::P3_TrashHealer) heals.push_back(gp.axes.heal_amount);
    // 16 combinacoes de (X1,X2) x 3 valores de X4 = 48 pontos de P3.
    REQUIRE(heals.size() == 48);
    const long count_6 = std::count(heals.begin(), heals.end(), 6);
    const long count_9 = std::count(heals.begin(), heals.end(), 9);
    const long count_12 = std::count(heals.begin(), heals.end(), 12);
    CHECK(count_6 == 16);
    CHECK(count_9 == 16);
    CHECK(count_12 == 16);
}

// ============================================================================
// run_and_evaluate_point / run_phase_a: smoke com N pequeno (motor real). O run
// COMPLETO (N=240.000, 116 pontos, ~28 milhoes de lutas) e disparo do team-lead/lider
// (protocolo secao 7 item 1), nunca deste harness - mesma disciplina do run_full_study
// do MIRA-SIM.
// ============================================================================

TEST_CASE("pacing_sim: run_and_evaluate_point roda 1 ponto e devolve veredicto coerente",
          "[domain][pacing_sim]") {
    PacingGridPoint gp{Scenario::P1_TrashVanilla, PacingAxes{}, "teste"};
    const PacingPointResult r = run_and_evaluate_point(gp, /*n=*/20, /*base_seed=*/99u, nullptr,
                                                       'A', 1, 1);
    REQUIRE(r.report.n == 20);
    REQUIRE(r.guardrails.size() == 6);  // trash: E1+E4+E2-cauda+E9-trivial+E9-hp+E8
    REQUIRE(r.approved == point_approved(r.guardrails));  // veredicto bate com a funcao pura
}

TEST_CASE("pacing_sim: run_phase_a smoke - progresso, 116 resultados, banner de conclusao",
          "[domain][pacing_sim][pacing_sim_smoke]") {
    // N pequeno de proposito - o teste e estrutural (progresso + contagem + formato),
    // nao o run pesado real.
    const int n_per_point = 3;
    const std::uint32_t base_seed = 20260801;

    std::ostringstream out;
    const std::vector<PacingPointResult> results = run_phase_a(n_per_point, base_seed, out);

    REQUIRE(results.size() == 116);
    for (const PacingPointResult& r : results) REQUIRE(r.report.n == n_per_point);

    const std::string text = out.str();
    REQUIRE(text.find("Fase A") != std::string::npos);
    REQUIRE(text.find("116 pontos") != std::string::npos);
    // formato exato de progresso (protocolo secao 8 item 6).
    REQUIRE(text.find("fase [A], ponto [1] de [116], simulação [") != std::string::npos);
    REQUIRE(text.find("VEREDICTO DO PONTO") != std::string::npos);
    REQUIRE(text.find("Fase A concluida") != std::string::npos);
}

// ============================================================================
// Reuso-seguranca: mira_sim_harness.hpp continua intacto (a suite dele, ja auditada por 2
// QAs, segue existindo e passando - ver CMakeLists.txt, ambos os arquivos .cpp registrados
// no mesmo executavel gusengine_domain_tests).
// ============================================================================

TEST_CASE("pacing_sim: reutiliza kRoundCap do estudo-pai sem redefinir", "[domain][pacing_sim]") {
    REQUIRE(kRoundCap == 30);
}

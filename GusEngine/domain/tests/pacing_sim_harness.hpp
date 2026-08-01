// SPDX-License-Identifier: Apache-2.0
// pacing_sim_harness.hpp (dev tool, test-only)
//
// Harness de simulacao de PACING (MIRA-SIM 2), estudo-irmao do MIRA-SIM. Implementa o
// protocolo em docs/design/mecanicas/proposta-protocolo-simulacao-pacing.md.
//
// REUSA `mira_sim_harness.hpp` (motor real, pareamento por seed, cap sem wipe, contador
// de erro interno, MetricWithCi/MCID, TeeOstream) SEM MODIFICA-LO - aquele arquivo ja foi
// auditado por dois QAs independentes ("nao reinvente", ordem do team-lead 2026-08-01). A
// UNICA peca nova e ADITIVA: `PacingBattleTrace` COMPOE (nao herda, nao modifica) um
// `mira_sim::BattleTrace` e acrescenta so o que o estudo de pacing precisa e o estudo da
// mira nao tinha (E11, ver abaixo).
//
// O QUE MUDA em relacao ao MIRA-SIM: la a variavel experimental era a POLITICA de mira (6
// bracos fixos); aqui a mira fica FIXA POR TIER e a variavel experimental vira o ESPACO
// DE NUMEROS do inimigo (4 eixos: X1 HP-mult, X2 Atk, X3 AP do elite, X4 cura da Jaci).
// V1 (braco por tier) NAO e mais placeholder: a tabela tier-mira do economy-designer
// saiu e fechou em 2026-08-01 (docs/design/mecanicas/proposta-economia-comedimento.md,
// tabela de tiers), com a ultima pergunta decidida pelo lider no mesmo dia. Trash comum =
// MiraArm::F_NoF4 (ponderada por F1/F2/F3, sem F4 - o lider rejeitou o sorteio uniforme
// (B) explicitamente: "espalha mas e roleta sem porque, fere o Pillar 1"); Elite =
// MiraArm::C_F4Soft. Ver fixed_arm_for() abaixo pra escada completa de tiers.
//
// GAP DE ENGINE reaproveitado do MIRA-SIM (mesma nota, nao repetida em profundidade aqui):
// CombatActor nao diferencia AP por tier - o harness gateia o inimigo pela CONTAGEM de
// acoes ja tomadas no turno (`max_ap() - ap()`) contra um limite PARAMETRIZAVEL (1 pro
// trash sempre; 1 OU 2 pro elite, eixo X3) em vez do valor fixo "1" do MIRA-SIM. Nota do
// protocolo (secao 8 item 3): "e variacao de teste, NAO mudanca do motor de producao" - o
// ScriptedBrain real de producao continua gateado a 1 acao/turno, sempre.
//
// UNIDADE (achado do orquestrador 2026-08-01, verificado pelo team-lead, decidido pelo
// lider - ver protocolo secao 1.1): a inflacao de HP de 2026-06-03 foi calibrada contra
// "TTK alvo 3-5 TURNOS" (turno de ATOR, nao rodada - a mesma secao/commit define AP como
// "por-ator"), NAO contra "4-8 rodadas" (essa frase nao existe em nenhum doc antes de
// 2026-07-19). O alvo ATIVO deste estudo continua sendo "3-5 RODADAS" (combat.md secao
// 15.1, decisao do lider com o Gus, rodada definida sem ambiguidade na secao 4.1 como
// "uma volta completa da fila CTB"). E11 (abaixo) recupera a intencao original de junho
// como metrica PROPRIA, separada - os dois alvos SO coincidem no numero "3 a 5" por
// acidente historico de terminologia, nunca confundir os dois.
//
// Cross-ref: docs/design/mecanicas/proposta-protocolo-simulacao-pacing.md;
//            docs/design/mecanicas/analise-mira-resultados.md (achados que motivam);
//            mira_sim_harness.hpp (metodo-pai, reusado aqui).

#ifndef GUS_DOMAIN_TESTS_PACING_SIM_HARNESS_HPP
#define GUS_DOMAIN_TESTS_PACING_SIM_HARNESS_HPP

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <deque>
#include <optional>
#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "mira_sim_harness.hpp"

namespace gus::domain::tests::pacing_sim {

// Reaproveita os tipos/aliases do estudo-pai (ja reexportam o vocabulario do motor real).
using mira_sim::ActorSpec;
using mira_sim::AttractionTracker;
using mira_sim::BattleTrace;
using mira_sim::CardFamily;
using mira_sim::CombatAction;
using mira_sim::CombatActor;
using mira_sim::CombatOutcome;
using mira_sim::CombatState;
using mira_sim::CombatStateMachine;
using mira_sim::idx;
using mira_sim::kDaemonGuardDef;
using mira_sim::kDaemonGuardHp;
using mira_sim::kDaemonGuardSpd;
using mira_sim::kPartySize;
using mira_sim::kRoundCap;
using mira_sim::kSentinelaSpd;
using mira_sim::member_of;
using mira_sim::McidComparison;
using mira_sim::mcid_compare_pct;
using mira_sim::mcid_compare_rounds;
using mira_sim::McidVerdict;
using mira_sim::mean_metric;
using mira_sim::MetricWithCi;
using mira_sim::MiraArm;
using mira_sim::PartyMember;
using mira_sim::percentile;
using mira_sim::proportion_metric_pct;
using gus::domain::tests::PropertyRandom;
using mira_sim::reference_def_of;
using mira_sim::run_bounded;
using mira_sim::BoundedCombatResult;
using mira_sim::TeeOstream;

// ============================================================================
// Eixos experimentais (protocolo secao 2.1) e tier.
// ============================================================================

enum class Tier { Trash, Elite };

// X1 (HP-mult), X2 (Atk absoluto), X3 (AP do elite, so relevante em cenarios com Daemon),
// X4 (cura da Jaci por uso, so relevante no cenario P3). Um "ponto da grade" e uma
// combinacao destes 4 valores.
struct PacingAxes {
    double hp_mult = 1.0;   // X1: multiplicador sobre o HP de referencia do tier
    int atk = 10;           // X2: valor ABSOLUTO (protocolo secao 2.1 da a grade em valores
                            //     absolutos, nao multiplicador)
    int elite_ap = 1;       // X3: 1 ou 2 (§13.1 manda 2 na regra de design; o motor
                            //     entrega 1; so importa quando ha Daemon-Guard no cenario)
    int heal_amount = 12;   // X4: so relevante no cenario P3 (Jaci cura)
};

// Referencia canonica (protocolo secao 2.1, "corte de dimensao"): Def, SPD e a formula de
// dano ficam FIXOS fora da variacao. So HP (X1) e Atk (X2) do INIMIGO variam.
inline constexpr int kTrashHpReference = 55;
inline constexpr int kTrashDef = 8;
inline constexpr int kEliteHpReference = kDaemonGuardHp;  // 144
inline constexpr int kEliteDef = kDaemonGuardDef;         // 14

[[nodiscard]] inline mira_sim::ActorSpec sentinela_spec_x(std::string id, const PacingAxes& axes) {
    const int hp = static_cast<int>(std::lround(static_cast<double>(kTrashHpReference) * axes.hp_mult));
    return mira_sim::ActorSpec{std::move(id), hp, axes.atk, kTrashDef, kSentinelaSpd,
                               CardFamily::Cinetico, false};
}

// Escolta de P4 (2 Sentinela-Bit): protocolo secao 2.2 diz "1 Daemon-Guard (X2, X3) + 2
// Sentinela-Bit" - SO o Daemon varia nos eixos elite; a escolta fica na REFERENCIA (nao
// varia com os eixos do screening de elite, que sao so 36 pontos = X1(3)xX2(3)xX3(2) do
// Daemon, protocolo secao 3).
[[nodiscard]] inline mira_sim::ActorSpec sentinela_spec_reference(std::string id) {
    return mira_sim::ActorSpec{std::move(id), kTrashHpReference, 10, kTrashDef, kSentinelaSpd,
                               CardFamily::Cinetico, false};
}

[[nodiscard]] inline mira_sim::ActorSpec daemon_spec_x(const PacingAxes& axes) {
    const int hp = static_cast<int>(std::lround(static_cast<double>(kEliteHpReference) * axes.hp_mult));
    return mira_sim::ActorSpec{"daemon", hp, axes.atk, kEliteDef, kDaemonGuardSpd,
                               CardFamily::Cinetico, false};
}

// ============================================================================
// Cenarios P1-P6 (protocolo secao 2.2, herdados/enxugados do estudo-pai).
// ============================================================================

enum class Scenario {
    P1_TrashVanilla,
    P2_TrashParede,
    P3_TrashHealer,
    P4_EliteEscolta,
    P5_EliteSolo,
    P6_TurtleTotal,  // so validacao, Fase C (protocolo secao 2.2)
};

[[nodiscard]] inline Tier tier_of(Scenario s) {
    switch (s) {
        case Scenario::P1_TrashVanilla:
        case Scenario::P2_TrashParede:
        case Scenario::P3_TrashHealer:
        case Scenario::P6_TurtleTotal:
            return Tier::Trash;
        case Scenario::P4_EliteEscolta:
        case Scenario::P5_EliteSolo:
            return Tier::Elite;
    }
    throw std::logic_error("Scenario desconhecido em tier_of.");
}

[[nodiscard]] inline std::string scenario_label(Scenario s) {
    switch (s) {
        case Scenario::P1_TrashVanilla: return "P1_trash_vanilla";
        case Scenario::P2_TrashParede: return "P2_trash_parede";
        case Scenario::P3_TrashHealer: return "P3_trash_healer";
        case Scenario::P4_EliteEscolta: return "P4_elite_escolta";
        case Scenario::P5_EliteSolo: return "P5_elite_solo";
        case Scenario::P6_TurtleTotal: return "P6_turtle_total";
    }
    throw std::logic_error("Scenario desconhecido em scenario_label.");
}

// Papel de party por cenario. Reduzido em relacao ao MIRA-SIM (sem honeypot/firewall -
// nenhum cenario de pacing usa cartas de agro, protocolo secao 2.3) mas com cura
// PARAMETRIZAVEL (X4) e contabilidade de E11 (golpes pro 1o inimigo abatido).
enum class PacingRole {
    AttackFront,
    DefendThenAttack,  // defende 1x por turno, ataca com o AP restante (ver MIRA-SIM,
                       // mesma tecnica "ap()==max_ap()")
    DefendOnlyTurtle,  // so P6: defende 1x, passa o resto de proposito
    HealMostInjured,   // so P3: cura o mais ferido (quantidade = X4), ataca com o resto
};

[[nodiscard]] inline std::array<PacingRole, mira_sim::kPartySize> roles_for(Scenario s) {
    using R = PacingRole;
    switch (s) {
        case Scenario::P1_TrashVanilla:
            return {R::AttackFront, R::AttackFront, R::AttackFront};
        case Scenario::P2_TrashParede:
            return {R::AttackFront, R::AttackFront, R::DefendThenAttack};  // Jaci = parede
        case Scenario::P3_TrashHealer:
            return {R::AttackFront, R::AttackFront, R::HealMostInjured};  // Jaci cura
        case Scenario::P4_EliteEscolta:
        case Scenario::P5_EliteSolo:
            // Suposicao explicita (protocolo herda de L6/L7 do estudo-pai, que tinham
            // Jaci defendendo "sob pressao"): mesma convencao aqui. Ver relatorio.
            return {R::AttackFront, R::AttackFront, R::DefendThenAttack};
        case Scenario::P6_TurtleTotal:
            return {R::DefendOnlyTurtle, R::DefendOnlyTurtle, R::DefendOnlyTurtle};
    }
    throw std::logic_error("Scenario desconhecido em roles_for.");
}

// Cenario completo: party (fixa, referencia canonica) + inimigos (parametrizados pelos
// eixos) + fracao de HP inicial da party (P4 = 0.70, "heróis a 70% HP", herdado de L6).
struct PacingScenarioSetup {
    Scenario scenario;
    std::vector<mira_sim::ActorSpec> party;
    std::vector<mira_sim::ActorSpec> enemies;
    double party_hp_fraction_start = 1.0;
};

// Braco de mira FIXO por tier (V1, protocolo secao 2.1). NAO e mais placeholder: a
// tabela tier-mira do economy-designer saiu e fechou em 2026-08-01 (fonte:
// docs/design/mecanicas/proposta-economia-comedimento.md, tabela de tiers), com a
// ultima pergunta decidida pelo lider no mesmo dia. Trash comum = F_NoF4 (ponderada por
// F1/F2/F3, SEM F4 - reage a quem bate e a quem esta ferido, mas ignora quem se
// defende; o lider rejeitou o sorteio uniforme (B) explicitamente: "espalha mas e
// roleta sem porque, o jogador nao consegue formar teoria sobre o inimigo, fere o
// Pillar 1"). Elite = C_F4Soft, coerente com a escalada de periculosidade por
// dificuldade (doutrina do comedido/esbanjador). Escada completa de tiers (D.1
// tabela): Trash comum=F, Trash avancado=C, Elite=C (com UtilityBrain, sem F8),
// Mini-boss=D (com contrapesos), Boss=D+F8, Boss final=D+F7+F8 (Patch-Zero fora da
// escada, mantem is_chaotic) - este estudo so exercita trash comum e elite.
[[nodiscard]] inline MiraArm fixed_arm_for(Tier tier) {
    return tier == Tier::Trash ? MiraArm::F_NoF4 : MiraArm::C_F4Soft;
}

[[nodiscard]] inline PacingScenarioSetup setup_for(Scenario s, const PacingAxes& axes) {
    PacingScenarioSetup setup;
    setup.scenario = s;
    setup.party = mira_sim::party_reference();
    switch (s) {
        case Scenario::P1_TrashVanilla:
        case Scenario::P2_TrashParede:
        case Scenario::P3_TrashHealer:
        case Scenario::P6_TurtleTotal:
            setup.enemies = {sentinela_spec_x("sentinela1", axes), sentinela_spec_x("sentinela2", axes),
                            sentinela_spec_x("sentinela3", axes)};
            setup.party_hp_fraction_start = 1.0;
            break;
        case Scenario::P4_EliteEscolta:
            setup.enemies = {daemon_spec_x(axes), sentinela_spec_reference("sentinela1"),
                            sentinela_spec_reference("sentinela2")};
            setup.party_hp_fraction_start = 0.70;
            break;
        case Scenario::P5_EliteSolo:
            setup.enemies = {daemon_spec_x(axes)};
            setup.party_hp_fraction_start = 1.0;
            break;
    }
    return setup;
}

// ============================================================================
// PacingBattleTrace: COMPOE mira_sim::BattleTrace (nao herda, nao modifica - cabecalho do
// arquivo) e acrescenta so o que este estudo-irmao precisa e o da mira nao tinha.
// ============================================================================

struct PacingBattleTrace {
    mira_sim::BattleTrace base;
    // E11 (nova, protocolo secao 4, 2026-08-01): golpes da party ate o 1o inimigo morrer,
    // contando o golpe fatal. -1 = nenhum inimigo caiu na luta (cap ou empate sem queda
    // inimiga - fora do escopo normal dos 6 cenarios, mas possivel em teoria).
    int hits_to_kill_first_enemy = -1;
};

// ============================================================================
// Decisao de acao da party (protocolo secao 2.2, enxugado do MIRA-SIM: sem honeypot/
// firewall - nenhum cenario de pacing usa cartas de agro - e com cura PARAMETRIZAVEL X4).
// Mesma tecnica de economia de AP do estudo-pai: `actor.ap() == actor.max_ap()` identifica
// a 1a decisao do turno (efeito test-only so dispara ali); as chamadas seguintes do MESMO
// turno atacam, igual ao AttackFront.
// ============================================================================

[[nodiscard]] inline CombatAction decide_party_action_pacing(
    PartyMember m, PacingRole role, CombatActor& actor, const CombatState& state,
    const CombatStateMachine& sm, AttractionTracker& tr, mira_sim::BattleTrace& base_trace,
    int round, int heal_amount, std::unordered_map<std::string, int>& hits_per_enemy,
    bool& first_kill_recorded, int& hits_to_kill_first_enemy) {
    const bool first_decision_this_turn = actor.ap() == actor.max_ap();

    auto attack_front = [&]() -> CombatAction {
        const auto enemies = state.alive_enemies();
        if (enemies.empty()) return CombatAction::pass();
        CombatActor* target = enemies.front();
        const int predicted = sm.preview_basic_attack_damage(actor, *target);
        tr.record_damage(m, round, predicted);  // F1 bookkeeping (alimenta C_F4Soft do elite)

        // E11: conta o golpe NESTE alvo, e se e o golpe fatal do 1o inimigo a cair na luta,
        // fixa o total (o golpe fatal CONTA, protocolo secao 4: "ate ele morrer").
        const int hits_on_target = ++hits_per_enemy[target->id()];
        if (!first_kill_recorded && predicted >= target->hp()) {
            hits_to_kill_first_enemy = hits_on_target;
            first_kill_recorded = true;
        }
        return CombatAction::attack(target->id());
    };

    switch (role) {
        case PacingRole::AttackFront:
            return attack_front();

        case PacingRole::DefendThenAttack:
            if (first_decision_this_turn) {
                base_trace.defend_count[idx(m)]++;
                return CombatAction::defend();
            }
            return attack_front();

        case PacingRole::DefendOnlyTurtle:
            // EXCECAO deliberada (P6, so validacao): defende 1x e joga fora os 2 AP
            // restantes de proposito, igual ao L3 do MIRA-SIM.
            if (first_decision_this_turn) {
                base_trace.defend_count[idx(m)]++;
                return CombatAction::defend();
            }
            return CombatAction::pass();

        case PacingRole::HealMostInjured:
            if (first_decision_this_turn) {
                const auto players = state.alive_players();
                if (!players.empty()) {
                    CombatActor* target = *std::min_element(
                        players.begin(), players.end(),
                        [](const CombatActor* a, const CombatActor* b) {
                            const double fa = a->max_hp() > 0
                                                  ? static_cast<double>(a->hp()) / a->max_hp()
                                                  : 0.0;
                            const double fb = b->max_hp() > 0
                                                  ? static_cast<double>(b->hp()) / b->max_hp()
                                                  : 0.0;
                            return fa < fb;
                        });
                    // X4 (cura parametrizavel, so P3): heal() e o metodo PUBLICO real (nao
                    // reimplementa regra). Custa 1 AP via scan_environment() (inerte nestes
                    // cenarios - nenhum seta ambiente -, ver mira_sim_harness.hpp cabecalho
                    // "ECONOMIA DE AP"), preservando 2 AP pra atacar no mesmo turno.
                    target->heal(heal_amount);
                    tr.record_support(m, round);
                }
                return CombatAction::scan_environment();
            }
            return attack_front();
    }
    throw std::logic_error("PacingRole desconhecido.");
}

// ============================================================================
// Execucao de UMA luta de pacing (pareavel por seed, mesmo padrao do MIRA-SIM).
// ============================================================================

[[nodiscard]] inline PacingBattleTrace run_single_pacing_battle(Scenario scenario,
                                                                 const PacingAxes& axes,
                                                                 std::uint32_t seed) {
    const PacingScenarioSetup setup = setup_for(scenario, axes);
    const std::array<PacingRole, mira_sim::kPartySize> roles = roles_for(scenario);
    const MiraArm arm = fixed_arm_for(tier_of(scenario));

    std::deque<CombatActor> actors;
    for (const mira_sim::ActorSpec& s : setup.party)
        actors.emplace_back(s.id, s.id, s.hp, s.atk, s.def, s.spd, s.family,
                            /*is_player_side=*/true, /*is_boss=*/false, /*knowledge_kills=*/0,
                            s.is_universal_compiler);
    for (const mira_sim::ActorSpec& s : setup.enemies)
        actors.emplace_back(s.id, s.id, s.hp, s.atk, s.def, s.spd, s.family,
                            /*is_player_side=*/false);

    std::vector<CombatActor*> actor_ptrs;
    actor_ptrs.reserve(actors.size());
    for (auto& a : actors) actor_ptrs.push_back(&a);

    // P4: heroes a 70% do HP (protocolo secao 2.2, herdado de L6 do estudo-pai). Mesmo
    // metodo publico take_damage(), sem Shield ativo no inicio = reducao de HP limpa.
    if (setup.party_hp_fraction_start < 1.0) {
        for (std::size_t i = 0; i < setup.party.size(); ++i) {
            CombatActor& a = actors[i];
            const int keep = static_cast<int>(
                std::lround(static_cast<double>(a.max_hp()) * setup.party_hp_fraction_start));
            const int to_remove = a.max_hp() - keep;
            if (to_remove > 0) a.take_damage(to_remove);
        }
    }

    AttractionTracker tr;
    PacingBattleTrace trace;
    std::optional<std::string> last_enemy_target;
    std::unordered_map<std::string, int> hits_per_enemy;
    bool first_kill_recorded = false;
    PropertyRandom rng(seed);
    const CombatStateMachine* sm_ptr = nullptr;

    // GAP DE ENGINE reaproveitado (ver cabecalho do arquivo, secao "GAP DE ENGINE"): o
    // gate abaixo generaliza o "1 acao/turno" do MIRA-SIM para "N acoes/turno" (X3, so
    // relevante pro daemon - protocolo secao 8 item 3, "variacao de teste, NAO mudanca do
    // motor de producao"; trash sempre 1).
    auto provider = [&](CombatActor& actor, const CombatState& state) -> CombatAction {
        const int round = state.round_index();
        tr.observe_round(round);

        if (actor.is_player_side()) {
            const PartyMember m = mira_sim::member_of(actor.id());
            return decide_party_action_pacing(m, roles[idx(m)], actor, state, *sm_ptr, tr,
                                              trace.base, round, axes.heal_amount, hits_per_enemy,
                                              first_kill_recorded, trace.hits_to_kill_first_enemy);
        }

        const int allowed = (actor.id() == "daemon") ? axes.elite_ap : 1;
        if ((actor.max_ap() - actor.ap()) >= allowed) return CombatAction::pass();
        return mira_sim::decide_enemy_action(arm, actor, state, *sm_ptr, tr, trace.base, rng,
                                             round, last_enemy_target);
    };

    CombatStateMachine sm(actor_ptrs, provider, /*card_registry=*/nullptr,
                         /*brain_registry=*/nullptr, &rng);
    sm_ptr = &sm;

    BoundedCombatResult result;
    try {
        result = run_bounded(sm, kRoundCap);
    } catch (const std::exception& e) {
        // Mesma defesa em profundidade do MIRA-SIM (cabecalho do arquivo, "CAP DE
        // RODADAS"): erro interno NUNCA e empate tecnico legitimo, contado A PARTE.
        result.capped = false;
        result.outcome = CombatOutcome::Ongoing;
        result.rounds_elapsed = kRoundCap;
        trace.base.internal_error = true;
        trace.base.internal_error_message = e.what();
    }

    trace.base.outcome = result.outcome;
    trace.base.rounds = result.rounds_elapsed;
    trace.base.capped = result.capped;

    for (std::size_t i = 0; i < setup.party.size(); ++i) {
        const CombatActor& a = actors[i];
        trace.base.final_hp_fraction[i] =
            a.max_hp() > 0 ? static_cast<double>(a.hp()) / a.max_hp() : 0.0;
        if (a.hp() <= 0) trace.base.fell[i] = true;
    }
    return trace;
}

// ============================================================================
// Pre-registro do lider (protocolo secao 4.1, aprovado 2026-08-01 ANTES de qualquer
// dado - mesma disciplina do MCID no MIRA-SIM: a regua nao pode ser escolhida depois de
// ver o resultado). Guarda-corpos mecanicos, sem julgamento de analise: cada um e um
// teto/piso fixo que QUALQUER ponto da grade tem que respeitar pra ser candidato.
// ============================================================================

// E1 (secao 4.1 item 1): taxa de vitoria alvo por tier.
inline constexpr double kTrashWinRateMinPct = 90.0;
inline constexpr double kTrashWinRateMaxPct = 97.0;
inline constexpr double kEliteWinRateMinPct = 55.0;
inline constexpr double kEliteWinRateMaxPct = 80.0;
// E4 (secao 4.1 item 2): teto de quedas do Gus por tier (limite SUPERIOR da faixa
// aprovada - "teto" e o pior caso tolerado, nao o centro da faixa).
//
// DECISAO DO LIDER, 2026-08-01 (achado do QA, resolvido apos pergunta explicita do
// team-lead): SO O TETO IMPORTA, DE PROPOSITO, SEM PISO. A faixa "8% a 12%" (trash) e
// "30% a 40%" (elite) do pre-registro (secao 4.1) e lida como TETO de 12%/40%, NAO
// como intervalo [8,12]/[30,40] com piso inferior obrigatorio. Um candidato em que o
// Gus quase nunca cai PASSA no guarda-corpo E4 - Gus cair pouco nunca e problema em si,
// e a trivialidade (luta oca, sem tensao nenhuma) ja e barrada por E9 (% de lutas sem
// dano nenhum), guarda-corpo SEPARADO. Este comentario existe pra nenhum QA futuro
// reabrir esta pergunta: a ausencia de piso e intencional, nao lacuna.
inline constexpr double kTrashGusFallCeilingPct = 12.0;
inline constexpr double kEliteGusFallCeilingPct = 40.0;
// E2 (secao 4, "a CAUDA importa"): guarda-corpo de cauda, igual pros dois tiers.
inline constexpr double kWindowP10MinRounds = 2.0;
inline constexpr double kWindowP90MaxRounds = 7.0;
// E9 (secao 4, proposto): trivialidade - trash nao pode ser oco.
inline constexpr double kTrashNoDamagePctCeiling = 40.0;
inline constexpr double kTrashFinalHpPctMin = 55.0;
inline constexpr double kTrashFinalHpPctMax = 90.0;
inline constexpr double kEliteFinalHpPctMax = 70.0;  // elite tem que doer
// E8 (secao 4, regra dura, so trash): "Nenhum candidato com mediana de 1a queda nas
// rodadas 1-2 no trash" - paulada injusta vs tensao construida, achado do QA 2026-08-01
// (o harness JA calculava median_first_fall_round mas nenhum guarda-corpo bloqueava).
inline constexpr double kFirstFallBadRoundMin = 1.0;
inline constexpr double kFirstFallBadRoundMax = 2.0;

// ============================================================================
// PacingPointReport: agregado de UM ponto da grade (1 cenario, 1 combinacao dos 4
// eixos) sobre N lutas. Cobre E1/E2/E3/E4/E6/E7/E8 (reaproveitados do vocabulario do
// MIRA-SIM, mesmas formulas de mean_metric/proportion_metric_pct/percentile - nao
// reinventadas) + E9/E10/E11 (novas deste estudo-irmao, protocolo secao 4). E5 (mira do
// estudo-pai) NAO consta da tabela de metricas deste protocolo - mira aqui e FIXA por
// tier, nao ha braco pra comparar previsibilidade entre.
// ============================================================================

struct PacingPointReport {
    std::string scenario;
    Tier tier = Tier::Trash;
    int n = 0;

    // E1: mesma divisao em 4 fracoes do MIRA-SIM (vitoria/derrota/empate tecnico/erro
    // interno, MESMO denominador n - "zero declarado vale mais que zero presumido").
    MetricWithCi win_rate_pct;
    MetricWithCi defeat_rate_pct;
    MetricWithCi pct_hit_round_cap;  // empate tecnico (E7 em P6, ver mais abaixo)
    MetricWithCi internal_error_pct;
    long internal_errors_count = 0;
    std::string first_internal_error_message;

    // E2 (primaria do estudo): duracao em rodadas + janela 3-5.
    double mean_rounds = 0.0;
    double median_rounds = 0.0;
    double p10_rounds = 0.0;
    double p90_rounds = 0.0;
    MetricWithCi pct_in_window_3_5;

    // E3 (sentinela de regressao, informativa aqui): concentracao de dano.
    MetricWithCi concentration_pct;
    MetricWithCi pct_saco_de_pancadas;

    // E4: quedas, com foco no Gus (unico com teto pre-registrado, secao 4.1 item 2).
    MetricWithCi pct_any_fall;
    std::array<MetricWithCi, kPartySize> pct_fall_by_member;

    // E6 (informativa, documenta H3b): absorcao do Shield.
    MetricWithCi shield_absorption_pct;

    // E7 (so tem sentido pleno em P6, protocolo secao 4: "cap de 30 rodadas batido"):
    // reusa pct_hit_round_cap acima, denominador valido (sem erro interno).
    MetricWithCi pct_hit_round_cap_valid;

    // E8: rodada da 1a queda (so entre lutas em que alguem caiu).
    double mean_first_fall_round = 0.0;
    double median_first_fall_round = 0.0;

    // E9 (nova): trivialidade - % de lutas SEM NENHUM HP perdido pela party, e HP final
    // medio da party (0-100%).
    MetricWithCi pct_no_damage_taken;
    double avg_final_hp_pct = 0.0;

    // E10 (nova): dano medio sofrido pela party POR LUTA, em HP absoluto e em % do pool
    // total (Gus+Caua+Jaci max_hp = 34+55+55=144, fixo - party e referencia canonica).
    MetricWithCi mean_damage_taken_hp;
    MetricWithCi mean_damage_taken_pct_pool;

    // E11 (nova): golpes da party ate o 1o inimigo cair, SO entre lutas em que algum
    // inimigo de fato caiu (hits_to_kill_first_enemy >= 0 - protocolo secao 4).
    double mean_hits_to_kill = 0.0;
    double median_hits_to_kill = 0.0;
    double p10_hits_to_kill = 0.0;
    double p90_hits_to_kill = 0.0;
    long battles_with_a_kill = 0;  // denominador de E11 (pode ser < n)
};

// Pool total de HP da party de referencia (Gus 34 + Caua 55 + Jaci 55 = 144, fixo -
// protocolo secao 2.1 "os statlines da PARTY inteiros ficam FIXOS").
inline constexpr int kPartyTotalHpPool =
    34 /*gus*/ + 55 /*caua*/ + 55 /*jaci*/;

[[nodiscard]] inline PacingPointReport aggregate_pacing(const std::string& scenario_name,
                                                        Tier tier,
                                                        const std::vector<PacingBattleTrace>& traces) {
    PacingPointReport r;
    r.scenario = scenario_name;
    r.tier = tier;
    r.n = static_cast<int>(traces.size());
    if (traces.empty()) return r;

    long victories = 0, defeats = 0, internal_errors = 0, hit_cap = 0;
    std::string first_internal_error_message;
    std::vector<double> rounds_d;
    long in_window = 0;
    std::vector<double> concentration_samples;
    long saco_de_pancadas = 0;
    long any_fall = 0;
    std::array<long, kPartySize> fall_by_member{0, 0, 0};
    long shield_raw = 0, shield_absorbed = 0;
    double final_hp_sum = 0.0;
    int final_hp_count = 0;
    std::vector<double> first_fall_rounds;
    long no_damage_battles = 0;
    std::vector<double> damage_taken_samples_hp;
    std::vector<double> hits_to_kill_samples;

    for (const PacingBattleTrace& pt : traces) {
        const mira_sim::BattleTrace& t = pt.base;
        if (t.internal_error) {
            // Mesma disciplina do MIRA-SIM: erro interno conta A PARTE e PULA o resto
            // do bookkeeping desta luta (rounds/dano/queda sao fallback, nao real).
            ++internal_errors;
            if (first_internal_error_message.empty())
                first_internal_error_message = t.internal_error_message;
            continue;
        }
        if (t.outcome == CombatOutcome::Victory) ++victories;
        else if (t.outcome == CombatOutcome::Defeat) ++defeats;
        if (t.capped) ++hit_cap;

        rounds_d.push_back(static_cast<double>(t.rounds));
        if (t.rounds >= 3 && t.rounds <= 5) ++in_window;

        const int total_dmg = t.damage_taken[0] + t.damage_taken[1] + t.damage_taken[2];
        if (total_dmg > 0) {
            const int max_dmg = std::max({t.damage_taken[0], t.damage_taken[1], t.damage_taken[2]});
            const double conc = 100.0 * static_cast<double>(max_dmg) / static_cast<double>(total_dmg);
            concentration_samples.push_back(conc);
            if (conc > 70.0) ++saco_de_pancadas;
        } else {
            ++no_damage_battles;  // E9: ninguem tomou dano nenhum nesta luta
        }
        damage_taken_samples_hp.push_back(static_cast<double>(total_dmg));

        if (t.any_fell()) ++any_fall;
        for (int i = 0; i < kPartySize; ++i)
            if (t.fell[i]) ++fall_by_member[i];

        shield_raw += t.shield_raw_total;
        shield_absorbed += t.shield_absorbed_total;

        for (int i = 0; i < kPartySize; ++i) {
            final_hp_sum += t.final_hp_fraction[i];
            ++final_hp_count;
        }
        if (t.first_fall_round >= 0) first_fall_rounds.push_back(static_cast<double>(t.first_fall_round));

        // E11: so entra na amostra se ALGUM inimigo de fato caiu nesta luta.
        if (pt.hits_to_kill_first_enemy >= 0)
            hits_to_kill_samples.push_back(static_cast<double>(pt.hits_to_kill_first_enemy));
    }

    const long valid_n = r.n - internal_errors;

    r.win_rate_pct = proportion_metric_pct(victories, r.n);
    r.defeat_rate_pct = proportion_metric_pct(defeats, r.n);
    r.pct_hit_round_cap = proportion_metric_pct(hit_cap, r.n);
    r.internal_error_pct = proportion_metric_pct(internal_errors, r.n);
    r.internal_errors_count = internal_errors;
    r.first_internal_error_message = first_internal_error_message;

    r.mean_rounds = mean_metric(rounds_d).value;
    r.median_rounds = percentile(rounds_d, 0.5);
    r.p10_rounds = percentile(rounds_d, 0.10);
    r.p90_rounds = percentile(rounds_d, 0.90);
    r.pct_in_window_3_5 = proportion_metric_pct(in_window, valid_n);

    r.concentration_pct = mean_metric(concentration_samples);
    r.pct_saco_de_pancadas = proportion_metric_pct(
        saco_de_pancadas, static_cast<long>(concentration_samples.size()));

    r.pct_any_fall = proportion_metric_pct(any_fall, valid_n);
    for (int i = 0; i < kPartySize; ++i)
        r.pct_fall_by_member[i] = proportion_metric_pct(fall_by_member[i], valid_n);

    r.shield_absorption_pct = proportion_metric_pct(shield_absorbed, shield_raw);
    r.pct_hit_round_cap_valid = proportion_metric_pct(hit_cap, valid_n);

    const MetricWithCi first_fall = mean_metric(first_fall_rounds);
    r.mean_first_fall_round = first_fall.value;
    r.median_first_fall_round = percentile(first_fall_rounds, 0.5);

    // E9: % de lutas sem dano nenhum (denominador = valid_n, mesma logica de "so lutas
    // que de fato rodaram" - erro interno nao tem dano confiavel pra contar).
    r.pct_no_damage_taken = proportion_metric_pct(no_damage_battles, valid_n);
    r.avg_final_hp_pct = final_hp_count > 0 ? 100.0 * final_hp_sum / final_hp_count : 0.0;

    // E10: dano medio sofrido por luta, em HP absoluto e em % do pool total (144).
    r.mean_damage_taken_hp = mean_metric(damage_taken_samples_hp);
    std::vector<double> damage_pct_samples;
    damage_pct_samples.reserve(damage_taken_samples_hp.size());
    for (double d : damage_taken_samples_hp)
        damage_pct_samples.push_back(100.0 * d / static_cast<double>(kPartyTotalHpPool));
    r.mean_damage_taken_pct_pool = mean_metric(damage_pct_samples);

    // E11: media/mediana/p10-p90 SO entre lutas com queda de inimigo (protocolo secao
    // 4: "quantos ataques... ate ele morrer" pressupoe que ele morreu).
    r.mean_hits_to_kill = mean_metric(hits_to_kill_samples).value;
    r.median_hits_to_kill = percentile(hits_to_kill_samples, 0.5);
    r.p10_hits_to_kill = percentile(hits_to_kill_samples, 0.10);
    r.p90_hits_to_kill = percentile(hits_to_kill_samples, 0.90);
    r.battles_with_a_kill = static_cast<long>(hits_to_kill_samples.size());

    return r;
}

// ============================================================================
// Guarda-corpos (protocolo secao 4: "um candidato reprova por qualquer guarda-corpo
// vermelho, mesmo com E2 perfeita"). Mecanico, sem julgamento de analise - so aplica os
// numeros do pre-registro (secao 4.1) ja fixados pelo lider ANTES de qualquer dado.
// ============================================================================

struct GuardrailVerdict {
    std::string label;
    bool green = false;
    std::string detail;
};

[[nodiscard]] inline std::vector<GuardrailVerdict> evaluate_guardrails(const PacingPointReport& r) {
    std::vector<GuardrailVerdict> out;
    const bool trash = r.tier == Tier::Trash;

    const double win_min = trash ? kTrashWinRateMinPct : kEliteWinRateMinPct;
    const double win_max = trash ? kTrashWinRateMaxPct : kEliteWinRateMaxPct;
    const bool win_ok = r.win_rate_pct.value >= win_min && r.win_rate_pct.value <= win_max;
    out.push_back({"E1 taxa de vitoria dentro da faixa pre-registrada", win_ok,
                   std::to_string(r.win_rate_pct.value) + "% (faixa " + std::to_string(win_min) +
                       "-" + std::to_string(win_max) + "%)"});

    // SO TETO, SEM PISO - decisao do lider 2026-08-01 (ver o comentario em
    // kTrashGusFallCeilingPct/kEliteGusFallCeilingPct acima): Gus cair pouco NUNCA
    // reprova este guarda-corpo. A trivialidade tem regua PROPRIA em E9.
    const double gus_ceiling = trash ? kTrashGusFallCeilingPct : kEliteGusFallCeilingPct;
    const double gus_fall = r.pct_fall_by_member[idx(PartyMember::Gus)].value;
    const bool gus_ok = gus_fall <= gus_ceiling;
    out.push_back({"E4 quedas do Gus dentro do teto pre-registrado (so teto, sem piso, decisao "
                   "do lider 2026-08-01)",
                   gus_ok,
                   std::to_string(gus_fall) + "% (teto " + std::to_string(gus_ceiling) + "%)"});

    const bool tail_ok = r.p10_rounds >= kWindowP10MinRounds && r.p90_rounds <= kWindowP90MaxRounds;
    out.push_back({"E2 cauda (p10>=2, p90<=7)", tail_ok,
                   "p10=" + std::to_string(r.p10_rounds) + " p90=" + std::to_string(r.p90_rounds)});

    if (trash) {
        const bool trivial_ok = r.pct_no_damage_taken.value <= kTrashNoDamagePctCeiling;
        out.push_back({"E9 trivialidade (trash, sem-dano <= 40%)", trivial_ok,
                       std::to_string(r.pct_no_damage_taken.value) + "%"});
        const bool hp_ok =
            r.avg_final_hp_pct >= kTrashFinalHpPctMin && r.avg_final_hp_pct <= kTrashFinalHpPctMax;
        out.push_back({"E9 HP final medio (trash, faixa 55-90%)", hp_ok,
                       std::to_string(r.avg_final_hp_pct) + "%"});

        // E8 (achado do QA 2026-08-01, regra dura do protocolo secao 4, so trash):
        // "Nenhum candidato com mediana de 1a queda nas rodadas 1-2". Vacuamente OK
        // quando ninguem cai (pct_any_fall==0) - sem isso, median_first_fall_round=0.0
        // (default de percentile() sobre amostra vazia) cairia fora de [1,2] por
        // coincidencia, e um mutante que mudasse esse default quebraria o guarda-corpo
        // em silencio.
        const bool no_falls = r.pct_any_fall.value <= 0.0;
        const bool e8_ok = no_falls || !(r.median_first_fall_round >= kFirstFallBadRoundMin &&
                                         r.median_first_fall_round <= kFirstFallBadRoundMax);
        out.push_back({"E8 mediana da 1a queda fora das rodadas 1-2 (trash)", e8_ok,
                       "mediana=" + std::to_string(r.median_first_fall_round) +
                           " (quedas=" + std::to_string(r.pct_any_fall.value) + "%)"});
    } else {
        const bool hp_ok = r.avg_final_hp_pct <= kEliteFinalHpPctMax;
        out.push_back({"E9 HP final medio (elite, teto 70% - tem que doer)", hp_ok,
                       std::to_string(r.avg_final_hp_pct) + "%"});
    }

    return out;
}

// ============================================================================
// Orquestracao de UM PONTO da grade (1 cenario, 1 combinacao dos 4 eixos): N lutas
// pareadas por seed, progresso no formato exato do protocolo (secao 8 item 6).
// ============================================================================

[[nodiscard]] inline std::vector<PacingBattleTrace> run_point_battles(
    Scenario scenario, const PacingAxes& axes, int n, std::uint32_t base_seed,
    std::ostream* progress_out, char phase, int point_idx_1based, int point_total) {
    std::vector<PacingBattleTrace> traces;
    traces.reserve(static_cast<std::size_t>(std::max(n, 0)));
    int last_pct_printed = -1;
    for (int i = 0; i < n; ++i) {
        traces.push_back(
            run_single_pacing_battle(scenario, axes, base_seed + static_cast<std::uint32_t>(i)));
        if (progress_out != nullptr && n > 0) {
            const int pct = ((i + 1) * 100) / n;
            if (pct != last_pct_printed) {
                last_pct_printed = pct;
                // .flush() explicito (mesmo achado do MIRA-SIM: sem isto, saida
                // redirecionada pra arquivo/pipe so mostra progresso quando o buffer do
                // SO decidir, nao a cada 1% de verdade).
                (*progress_out) << "fase [" << phase << "], ponto [" << point_idx_1based << "] de ["
                                << point_total << "], simulação [" << (i + 1) << "] de [" << n
                                << "]\n";
                progress_out->flush();
            }
        }
    }
    return traces;
}

// ============================================================================
// Relatorio de duas camadas (protocolo secao 8 item 7: "primeiro os dados... depois a
// analise"). Camada 1 = numeros (E1-E11); camada 2 = guarda-corpos aplicados (secao 4).
// A analise de C-level em linguagem simples (o resto do item 7) e trabalho POSTERIOR,
// fora desta fatia - o harness entrega o numero, nao a narrativa.
// ============================================================================

inline void print_point_report_layer1(std::ostream& out, const PacingPointReport& r) {
    out << "\n=== Ponto: " << r.scenario << " (" << (r.tier == Tier::Trash ? "trash" : "elite")
        << ") ===\n"
        << "n=" << r.n << "\n"
        << "  E1 (vitoria/derrota/empate tecnico/erro interno, 4 fracoes de n=" << r.n
        << "): vitoria=" << r.win_rate_pct.value << "%+-" << r.win_rate_pct.moe95
        << "  derrota=" << r.defeat_rate_pct.value << "%+-" << r.defeat_rate_pct.moe95
        << "  empate_tecnico=" << r.pct_hit_round_cap.value << "%+-" << r.pct_hit_round_cap.moe95
        << "  erro_interno=" << r.internal_error_pct.value << "%+-" << r.internal_error_pct.moe95
        << "\n"
        << "  E2 (duracao, primaria): rounds mean=" << r.mean_rounds
        << " median=" << r.median_rounds << " p10=" << r.p10_rounds << " p90=" << r.p90_rounds
        << "  janela-3-5=" << r.pct_in_window_3_5.value << "%+-" << r.pct_in_window_3_5.moe95
        << "\n"
        << "  E3 (concentracao, regressao): media=" << r.concentration_pct.value << "%+-"
        << r.concentration_pct.moe95 << "  saco-de-pancadas=" << r.pct_saco_de_pancadas.value
        << "%+-" << r.pct_saco_de_pancadas.moe95 << "\n"
        << "  E4 (quedas): qualquer=" << r.pct_any_fall.value << "%+-" << r.pct_any_fall.moe95
        << "  gus=" << r.pct_fall_by_member[idx(PartyMember::Gus)].value << "%"
        << "  caua=" << r.pct_fall_by_member[idx(PartyMember::Caua)].value << "%"
        << "  jaci=" << r.pct_fall_by_member[idx(PartyMember::Jaci)].value << "%\n"
        << "  E6 (absorcao do Shield, informativa/H3b): " << r.shield_absorption_pct.value
        << "%+-" << r.shield_absorption_pct.moe95 << "\n"
        << "  E7 (cap de 30 rodadas batido, so tem sentido pleno em P6): validas="
        << r.pct_hit_round_cap_valid.value << "%+-" << r.pct_hit_round_cap_valid.moe95 << "\n"
        << "  E8 (rodada da 1a queda): mean=" << r.mean_first_fall_round
        << " median=" << r.median_first_fall_round << "\n"
        << "  E9 (trivialidade, nova): sem-dano=" << r.pct_no_damage_taken.value << "%+-"
        << r.pct_no_damage_taken.moe95 << "  hp-final-medio=" << r.avg_final_hp_pct << "%\n"
        << "  E10 (dano sofrido, nova, insumo do economy-designer): media="
        << r.mean_damage_taken_hp.value << "HP+-" << r.mean_damage_taken_hp.moe95 << "  ("
        << r.mean_damage_taken_pct_pool.value << "%+-" << r.mean_damage_taken_pct_pool.moe95
        << " do pool de " << kPartyTotalHpPool << "HP)\n"
        << "  E11 (golpes ate o 1o inimigo cair, nova - NAO e a mesma pergunta que E2, 'golpes "
        << "para matar 1 inimigo' e 'rodadas pra vencer a luta inteira' coincidem em '3 a 5' "
        << "por acidente historico de terminologia, protocolo secao 1.1): n_validas="
        << r.battles_with_a_kill << " mean=" << r.mean_hits_to_kill
        << " median=" << r.median_hits_to_kill << " p10=" << r.p10_hits_to_kill
        << " p90=" << r.p90_hits_to_kill << "\n";
    if (r.scenario != scenario_label(Scenario::P1_TrashVanilla)) {
        out << "  NOTA E11: o protocolo (secao 4) define esta metrica para o cenario P1; "
            << "o numero acima e calculado e impresso aqui por uniformidade do relatorio, mas "
            << "quem analisar deve LER E11 SO da linha de P1 - nos demais cenarios e "
            << "informativo, nao a leitura oficial.\n";
    }
    out << "--- erros internos: " << r.internal_errors_count << " de " << r.n << " ---\n";
    if (!r.first_internal_error_message.empty())
        out << "  primeira mensagem: " << r.first_internal_error_message << "\n";
}

// Veredicto AGREGADO do ponto: [APROVADO] so se TODOS os guarda-corpos estiverem
// verdes (protocolo secao 4: "um candidato reprova por qualquer guarda-corpo vermelho,
// mesmo com E2 perfeita" - nao ha meio-termo).
[[nodiscard]] inline bool point_approved(const std::vector<GuardrailVerdict>& verdicts) {
    for (const GuardrailVerdict& v : verdicts)
        if (!v.green) return false;
    return true;
}

inline void print_point_guardrails_layer2(std::ostream& out, const PacingPointReport& r) {
    out << "--- guarda-corpos (pre-registro do lider, secao 4.1 - reprova por qualquer "
        << "vermelho mesmo com E2 perfeita) ---\n";
    const std::vector<GuardrailVerdict> verdicts = evaluate_guardrails(r);
    for (const GuardrailVerdict& v : verdicts)
        out << "  [" << (v.green ? "OK" : "REPROVADO") << "] " << v.label << ": " << v.detail << "\n";
    out << "VEREDICTO DO PONTO: [" << (point_approved(verdicts) ? "APROVADO" : "REPROVADO") << "]\n";
}

// ============================================================================
// Grade da Fase A (protocolo secao 2.1 "corte de dimensao" + secao 3 "quantas lutas"):
// fatorial FECHADO, todos os numeros ja fixados no protocolo - nao ha escolha de design
// pendente aqui, so encodificacao. N pleno (240.000) em TODO ponto, decisao do lider
// 2026-08-01 ("recusou a amostra escalonada... estatistica e ciencia de medir
// repeticoes"): a Fase A NAO usa o N reduzido (24.000) que o rascunho original do
// protocolo propunha pro screening.
//
// Trash: X1(4) x X2(4) x cenarios{P1, P2, P3 x X4(3)} = 16 x 5 = 80 pontos.
// Elite: X1(3) x X2(3) x X3(2) x cenarios{P4, P5} = 18 x 2 = 36 pontos.
// Total: 116 pontos (conferido com o team-lead, mesma conta dos dois lados).
// ============================================================================

// Um ponto da grade: cenario + eixos + rotulo legivel pro relatorio (identifica o ponto
// sem precisar reconstruir os eixos a partir do texto).
struct PacingGridPoint {
    Scenario scenario;
    PacingAxes axes;
    std::string label;
};

// X1 trash: percentuais do protocolo (secao 2.1) SAO exatos sobre a referencia de 55
// (22/33/44/55 = 55 x 0.40/0.60/0.80/1.00, sem arredondamento).
inline constexpr std::array<double, 4> kPhaseATrashHpMult{0.40, 0.60, 0.80, 1.00};
inline constexpr std::array<int, 4> kPhaseATrashAtk{8, 10, 12, 14};
inline constexpr std::array<int, 3> kPhaseATrashHealAmounts{6, 9, 12};  // X4, so P3

// X1 elite: o protocolo (secao 2.1) da o alvo em HP (90/115/144) e os percentuais
// "62%/80%/100%" sao ARREDONDADOS pra leitura (90/144=62.5%, 115/144=79.86%). Usar o
// multiplicador EXATO (HP alvo / referencia) evita que o arredondamento do texto vire
// off-by-one no lround() de sentinela_spec_x/daemon_spec_x.
inline constexpr std::array<double, 3> kPhaseAEliteHpMult{90.0 / 144.0, 115.0 / 144.0, 1.00};
inline constexpr std::array<int, 3> kPhaseAEliteAtk{14, 18, 22};
inline constexpr std::array<int, 2> kPhaseAEliteAp{1, 2};

[[nodiscard]] inline std::string phase_a_point_label(const std::string& tier_name,
                                                      Scenario scenario, const PacingAxes& axes) {
    std::string s = tier_name + " " + scenario_label(scenario) + " hp_mult=" +
                    std::to_string(axes.hp_mult) + " atk=" + std::to_string(axes.atk);
    if (tier_of(scenario) == Tier::Elite) s += " elite_ap=" + std::to_string(axes.elite_ap);
    if (scenario == Scenario::P3_TrashHealer) s += " heal=" + std::to_string(axes.heal_amount);
    return s;
}

[[nodiscard]] inline std::vector<PacingGridPoint> build_phase_a_trash_grid() {
    std::vector<PacingGridPoint> points;
    points.reserve(80);
    for (double hp_mult : kPhaseATrashHpMult) {
        for (int atk : kPhaseATrashAtk) {
            PacingAxes base_axes;
            base_axes.hp_mult = hp_mult;
            base_axes.atk = atk;

            for (Scenario s : {Scenario::P1_TrashVanilla, Scenario::P2_TrashParede}) {
                points.push_back({s, base_axes, phase_a_point_label("trash", s, base_axes)});
            }
            // P3 desdobra em X4 (cura, 3 valores) - so este cenario usa heal_amount.
            for (int heal : kPhaseATrashHealAmounts) {
                PacingAxes axes = base_axes;
                axes.heal_amount = heal;
                points.push_back({Scenario::P3_TrashHealer, axes,
                                 phase_a_point_label("trash", Scenario::P3_TrashHealer, axes)});
            }
        }
    }
    return points;
}

[[nodiscard]] inline std::vector<PacingGridPoint> build_phase_a_elite_grid() {
    std::vector<PacingGridPoint> points;
    points.reserve(36);
    for (double hp_mult : kPhaseAEliteHpMult) {
        for (int atk : kPhaseAEliteAtk) {
            for (int ap : kPhaseAEliteAp) {
                PacingAxes axes;
                axes.hp_mult = hp_mult;
                axes.atk = atk;
                axes.elite_ap = ap;
                for (Scenario s : {Scenario::P4_EliteEscolta, Scenario::P5_EliteSolo}) {
                    points.push_back({s, axes, phase_a_point_label("elite", s, axes)});
                }
            }
        }
    }
    return points;
}

[[nodiscard]] inline std::vector<PacingGridPoint> build_phase_a_grid() {
    std::vector<PacingGridPoint> points = build_phase_a_trash_grid();
    const std::vector<PacingGridPoint> elite = build_phase_a_elite_grid();
    points.insert(points.end(), elite.begin(), elite.end());
    return points;
}

// ============================================================================
// Resultado de UM ponto ja processado (agregado + guarda-corpos + veredicto), pronto
// pra impressao e pra montar a lista final de sobreviventes.
// ============================================================================

struct PacingPointResult {
    PacingGridPoint point;
    PacingPointReport report;
    std::vector<GuardrailVerdict> guardrails;
    bool approved = false;
};

// Roda UM ponto da grade (N lutas pareadas por seed) e retorna o resultado processado.
// Reusa run_point_battles (a mesma primitiva usada em qualquer chamada isolada) +
// aggregate_pacing + evaluate_guardrails - nada reimplementado aqui.
[[nodiscard]] inline PacingPointResult run_and_evaluate_point(const PacingGridPoint& gp, int n,
                                                              std::uint32_t base_seed,
                                                              std::ostream* progress_out,
                                                              char phase, int point_idx_1based,
                                                              int point_total) {
    const std::vector<PacingBattleTrace> traces =
        run_point_battles(gp.scenario, gp.axes, n, base_seed, progress_out, phase,
                          point_idx_1based, point_total);
    PacingPointResult result;
    result.point = gp;
    result.report = aggregate_pacing(gp.label, tier_of(gp.scenario), traces);
    result.guardrails = evaluate_guardrails(result.report);
    result.approved = point_approved(result.guardrails);
    return result;
}

// Roda a Fase A inteira (grade fatorial fechada, 116 pontos) e imprime o relatorio de 2
// camadas PONTO A PONTO (nao acumulado ate o fim - mesma disciplina do MIRA-SIM: o lider
// ve cada ponto assim que fecha). Retorna os resultados de TODOS os pontos (aprovados e
// reprovados), pra quem orquestrar Fase B/C decidir a partir dos aprovados.
[[nodiscard]] inline std::vector<PacingPointResult> run_phase_a(int n_per_point,
                                                                 std::uint32_t base_seed,
                                                                 std::ostream& out) {
    const std::vector<PacingGridPoint> grid = build_phase_a_grid();
    out << "=== PACING-SIM: Fase A (grade fatorial fechada, " << grid.size() << " pontos) ===\n"
        << "protocolo: docs/design/mecanicas/proposta-protocolo-simulacao-pacing.md secao 2.1/3\n"
        << "N por ponto: " << n_per_point << "  |  seed base: " << base_seed << "\n"
        << "====================================================================\n";

    std::vector<PacingPointResult> results;
    results.reserve(grid.size());
    int point_idx = 0;
    for (const PacingGridPoint& gp : grid) {
        ++point_idx;
        PacingPointResult r = run_and_evaluate_point(gp, n_per_point, base_seed, &out, 'A',
                                                     point_idx, static_cast<int>(grid.size()));
        out << "\n### ponto " << point_idx << " de " << grid.size() << ": " << gp.label << " ###\n";
        print_point_report_layer1(out, r.report);
        print_point_guardrails_layer2(out, r.report);
        out.flush();
        results.push_back(std::move(r));
    }

    const long approved_count =
        std::count_if(results.begin(), results.end(), [](const PacingPointResult& r) { return r.approved; });
    out << "\n=== Fase A concluida: " << approved_count << " de " << results.size()
        << " pontos aprovados em TODOS os guarda-corpos ===\n";
    if (approved_count == 0) {
        // Achado explicito do team-lead (protocolo secao 2.1, item 2 da lista de
        // mudanca de ideia): "se NENHUM ponto da grade entrar na janela... o estudo
        // devolve 'redesign necessario' em vez de tabela". A Fase B NAO tem vizinhanca
        // nenhuma pra refinar sem isto - declarar em vez de silenciar.
        out << "Fase B NAO RODA: nenhum ponto da Fase A aprovou em todos os guarda-corpos, "
            << "entao nao ha vizinhanca nenhuma pra refinar (protocolo secao 1.3 item 2: "
            << "isto pode significar que o problema e a FORMULA, nao os numeros).\n";
    }
    return results;
}

// ============================================================================
// Primitivas reusaveis pra Fase B/C, PURAS e MECANICAS - nao decidem o METODO (direcao
// da bissecao, quantos niveis, criterio de corte dos finalistas), so oferecem os blocos
// de construcao. Achado 2026-08-01: sinalizado ao team-lead antes de fechar a
// orquestracao completa das duas fases, porque o METODO tem ambiguidade real (ver
// mensagem no bus); estas funcoes NAO resolvem essa ambiguidade, so a preparam.
// ============================================================================

// Cenarios de um tier (protocolo secao 2.2): P6 (turtle total) e trash-only (roles_for
// so defende, enemies sao Sentinela-Bit - o mesmo elenco trash de P1-P3), nunca elite.
[[nodiscard]] inline std::vector<Scenario> scenarios_for_tier(Tier tier) {
    if (tier == Tier::Trash) {
        return {Scenario::P1_TrashVanilla, Scenario::P2_TrashParede, Scenario::P3_TrashHealer,
               Scenario::P6_TurtleTotal};
    }
    return {Scenario::P4_EliteEscolta, Scenario::P5_EliteSolo};
}

// "Bateria completa" (protocolo secao 2.2, Fase C): roda os MESMOS eixos contra TODOS
// os cenarios do tier correspondente (P6 incluso so pro trash, por construcao - nao ha
// P6 elite). NAO mistura eixo de tier errado: um candidato trash (eixos calibrados
// sobre a referencia de 55 HP) so faz sentido nos cenarios trash; idem elite.
[[nodiscard]] inline std::vector<PacingPointResult> run_full_battery(
    Tier tier, const PacingAxes& axes, int n, std::uint32_t base_seed, std::ostream* progress_out,
    int point_idx_1based, int point_total) {
    const std::vector<Scenario> scenarios = scenarios_for_tier(tier);
    std::vector<PacingPointResult> results;
    results.reserve(scenarios.size());
    const std::string tier_name = tier == Tier::Trash ? "trash" : "elite";
    for (Scenario s : scenarios) {
        PacingGridPoint gp{s, axes, phase_a_point_label(tier_name, s, axes)};
        results.push_back(run_and_evaluate_point(gp, n, base_seed, progress_out, 'C',
                                                  point_idx_1based, point_total));
    }
    return results;
}

// Vizinhanca SIMETRICA de um ponto (protocolo secao 2.1, Fase B: "passo de ~10% de HP e
// ±1 de Atk"). Gera o mini-grid combinatorio (todas as combinacoes de HP-mult x Atk num
// raio de `levels` passos), SEM decidir direcao (nao pressupoe que HP correlaciona com
// duracao - essa e a hipotese H1 do protocolo, secao 1.2, ainda nao confirmada pelos
// dados) nem quantos niveis usar de verdade (1 ou 2 - ambiguidade sinalizada ao
// team-lead). O proprio ponto base (offset 0,0) NAO entra - ja foi testado na Fase A.
[[nodiscard]] inline std::vector<PacingAxes> neighbor_axes(const PacingAxes& base, int levels) {
    std::vector<PacingAxes> out;
    for (int hp_level = -levels; hp_level <= levels; ++hp_level) {
        for (int atk_level = -levels; atk_level <= levels; ++atk_level) {
            if (hp_level == 0 && atk_level == 0) continue;
            PacingAxes a = base;
            a.hp_mult = base.hp_mult * (1.0 + 0.10 * hp_level);
            a.atk = base.atk + atk_level;
            out.push_back(a);
        }
    }
    return out;
}

}  // namespace gus::domain::tests::pacing_sim

#endif  // GUS_DOMAIN_TESTS_PACING_SIM_HARNESS_HPP

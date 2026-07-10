// balance_harness.hpp (dev tool, test-only)
//
// Harness de balanceamento OFFLINE (COMBATE-TEORIA-JOGOS item [1]). Orquestra N execucoes
// headless do motor JA EXISTENTE (CombatStateMachine::run_until_end, secao 3) pra medir
// win-rate, duracao (rounds) e dominancia de acao por cenario representativo, sem alterar
// FORMULA de dano, IA ou FSM. Ferramenta de MEDICAO, nao de balanceamento.
//
// O que e reaproveitado (NAO reimplementado):
//   - FSM completa via CombatStateMachine::run_until_end (ja existente, intacta).
//   - IA inimiga: ScriptedBrain (secao 13, ja existente em producao). Mesma classe usada
//     hoje pra Trash e Elite (UtilityBrain e jogo posterior, fora de escopo, item [3]).
//   - RNG seedado deterministico: PropertyRandom (property_gen.hpp, ja existente nos
//     testes de propriedade do motor, marco M5). Nenhum PRNG novo inventado aqui.
//
// Lado da PARTY: NAO existe ainda, em codigo de producao, uma classe "AutoResolveBrain"
// reutilizavel (a heuristica da secao 19.6 so aparece hoje como lambda inline de teste em
// combat_state_machine_test.cpp, "estilo AutoResolveBrain"). Productizar a classe completa
// (com selecao de carta da familia dominante) e escopo do item [19.9]/feature de
// auto-resolve, fora deste harness de medicao (evita decisao de API de producao nao
// aprovada). Este harness usa o mesmo ESTILO sub-otimo, restrito ao subconjunto mais
// conservador da secao 19.6: SO ataque basico, mira o 1o inimigo vivo, ZERO Gambito/Scan/
// combo/carta. E o pior-caso coerente com o principio da 19.6 ("o resultado pior EMERGE da
// falta de otimizacao, nao de um numero magico").
//
// Determinismo: cada batalha usa seed = base_seed + indice (reproduzivel bit-a-bit).
//
// Camada: domain/tests, POCO puro, ZERO I/O real de plataforma (o unico "I/O" e o
// relatorio texto impresso por quem chama print_report, responsabilidade do caller).
//
// Cross-ref: docs/design/mecanicas/combat.md secao 15 (janela-alvo 4-8 rounds), secao 17
// (stats de referencia), secao 19 (AutoResolveBrain); TODO.md item COMBATE-TEORIA-JOGOS [1].

#ifndef GUS_DOMAIN_TESTS_BALANCE_HARNESS_HPP
#define GUS_DOMAIN_TESTS_BALANCE_HARNESS_HPP

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <deque>
#include <numeric>
#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "gus/domain/combat/combat_actor.hpp"
#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/combat/combat_records.hpp"
#include "gus/domain/combat/combat_state.hpp"
#include "gus/domain/combat/combat_state_machine.hpp"
#include "gus/domain/combat/scripted_brain.hpp"
#include "property_gen.hpp"  // PropertyRandom (LCG seedado, ja existente, marco M5)

namespace gus::domain::tests {

// Especificacao de UM ator (party ou inimigo) pra montar um cenario representativo.
// Espelha os campos de CombatActor relevantes ao combate (secao 17, stats de referencia).
struct BalanceActorSpec {
    std::string id;
    int hp = 1;
    int atk = 1;
    int def = 0;
    int spd = 1;
    gus::domain::combat::CardFamily family = gus::domain::combat::CardFamily::Eletrico;
};

// Cenario representativo (party vs inimigos) a medir em N repeticoes com seeds diferentes.
struct BalanceScenario {
    std::string label;
    std::vector<BalanceActorSpec> party;
    std::vector<BalanceActorSpec> enemies;
};

// Resultado de UMA batalha, dado puro (sem logica de agregacao).
struct BattleOutcome {
    gus::domain::combat::CombatOutcome outcome = gus::domain::combat::CombatOutcome::Ongoing;
    int rounds = 0;
    // Contagem de acoes por rotulo "<lado>:<acao>" (ex.: "party:attack", "enemy:pass").
    std::unordered_map<std::string, int> action_counts;
};

// Relatorio agregado de um cenario (N batalhas). Consumido pelo lider/economy-designer.
struct BalanceReport {
    std::string label;
    int battles = 0;
    int victories = 0;
    int defeats = 0;
    int fled = 0;
    double win_rate_pct = 0.0;
    double mean_rounds = 0.0;
    double median_rounds = 0.0;
    double p95_rounds = 0.0;
    // true quando MEDIA e MEDIANA caem dentro da janela-alvo secao 15 (~4-8 rounds).
    bool window_4_8_ok = false;
    // rotulo "<lado>:<acao>" -> % do total de acoes do cenario (dominancia).
    std::unordered_map<std::string, double> action_share_pct;
};

namespace detail {

[[nodiscard]] inline std::string action_label(bool is_player_side,
                                              gus::domain::combat::CombatActionType type) {
    using gus::domain::combat::CombatActionType;
    const std::string side = is_player_side ? "party:" : "enemy:";
    switch (type) {
        case CombatActionType::Attack: return side + "attack";
        case CombatActionType::Defend: return side + "defend";
        case CombatActionType::UseCard: return side + "use_card";
        case CombatActionType::Flee: return side + "flee";
        case CombatActionType::Scan: return side + "scan";
        case CombatActionType::ScanEnvironment: return side + "scan_environment";
        case CombatActionType::GambitPredict: return side + "gambit_predict";
        case CombatActionType::GambitReorder: return side + "gambit_reorder";
        case CombatActionType::Pass: return side + "pass";
    }
    return side + "unknown";
}

// Percentil por interpolacao linear (rank continuo), p em [0,1]. Copia o vetor (N e
// pequeno por batalha: uma amostra por cenario, nao por turno).
[[nodiscard]] inline double percentile(std::vector<int> values, double p) {
    if (values.empty()) return 0.0;
    std::sort(values.begin(), values.end());
    const double rank = p * static_cast<double>(values.size() - 1);
    const auto lo = static_cast<std::size_t>(std::floor(rank));
    const auto hi = static_cast<std::size_t>(std::ceil(rank));
    if (lo == hi) return static_cast<double>(values[lo]);
    const double frac = rank - static_cast<double>(lo);
    return static_cast<double>(values[lo]) +
           frac * static_cast<double>(values[hi] - values[lo]);
}

}  // namespace detail

// Roda UMA batalha headless via o motor JA EXISTENTE (run_until_end, FSM intacta).
// Provider sub-otimo (secao 19.6) pra party; ScriptedBrain (secao 13, ja existente,
// producao) pros inimigos. NAO consome card_registry (sem cartas: subconjunto so-ataque).
[[nodiscard]] inline BattleOutcome run_single_battle(const BalanceScenario& scenario,
                                                     std::uint32_t seed) {
    using gus::domain::combat::CombatAction;
    using gus::domain::combat::CombatActor;
    using gus::domain::combat::CombatState;
    using gus::domain::combat::CombatStateMachine;
    using gus::domain::combat::ScriptedBrain;

    // deque preserva enderecos estaveis apos inserir: a FSM guarda CombatActor* nao-dono
    // (mesmo padrao ja usado nos testes existentes da FSM).
    std::deque<CombatActor> actors;
    for (const auto& spec : scenario.party)
        actors.emplace_back(spec.id, spec.id, spec.hp, spec.atk, spec.def, spec.spd,
                            spec.family, /*is_player_side=*/true);
    for (const auto& spec : scenario.enemies)
        actors.emplace_back(spec.id, spec.id, spec.hp, spec.atk, spec.def, spec.spd,
                            spec.family, /*is_player_side=*/false);

    std::vector<CombatActor*> actor_ptrs;
    actor_ptrs.reserve(actors.size());
    for (auto& a : actors) actor_ptrs.push_back(&a);

    BattleOutcome outcome;
    ScriptedBrain enemy_brain;

    auto provider = [&](CombatActor& actor, const CombatState& state) -> CombatAction {
        CombatAction action;
        if (actor.is_player_side()) {
            // Sub-otimo (secao 19.6): SO ataque basico, mira o 1o inimigo vivo.
            const auto enemies = state.alive_enemies();
            action = enemies.empty() ? CombatAction::pass()
                                     : CombatAction::attack(enemies.front()->id());
        } else {
            action = enemy_brain.decide_action(state, actor);
        }
        outcome.action_counts[detail::action_label(actor.is_player_side(), action.type)]++;
        return action;
    };

    PropertyRandom rng(seed);
    CombatStateMachine sm(actor_ptrs, provider, /*card_registry=*/nullptr,
                         /*brain_registry=*/nullptr, &rng);
    const auto result = sm.run_until_end();
    outcome.outcome = result.outcome;
    outcome.rounds = result.rounds_elapsed;
    return outcome;
}

// Agrega N resultados de batalha (PURA, sem I/O, sem rodar motor: testavel com dados
// sinteticos). win-rate, duracao (media/mediana/p95) e dominancia de acao (secao 15/19).
[[nodiscard]] inline BalanceReport aggregate(const std::string& label,
                                             const std::vector<BattleOutcome>& outcomes) {
    using gus::domain::combat::CombatOutcome;

    BalanceReport report;
    report.label = label;
    report.battles = static_cast<int>(outcomes.size());
    if (outcomes.empty()) return report;

    std::vector<int> rounds;
    rounds.reserve(outcomes.size());
    std::unordered_map<std::string, int> action_totals;
    int total_actions = 0;

    for (const auto& o : outcomes) {
        rounds.push_back(o.rounds);
        switch (o.outcome) {
            case CombatOutcome::Victory: ++report.victories; break;
            case CombatOutcome::Defeat: ++report.defeats; break;
            case CombatOutcome::Fled: ++report.fled; break;
            case CombatOutcome::Ongoing: break;  // nao deveria ocorrer (run_until_end)
        }
        for (const auto& [act_label, count] : o.action_counts) {
            action_totals[act_label] += count;
            total_actions += count;
        }
    }

    report.win_rate_pct = 100.0 * static_cast<double>(report.victories) / report.battles;
    report.mean_rounds =
        std::accumulate(rounds.begin(), rounds.end(), 0.0) / static_cast<double>(rounds.size());
    report.median_rounds = detail::percentile(rounds, 0.5);
    report.p95_rounds = detail::percentile(rounds, 0.95);
    report.window_4_8_ok = report.mean_rounds >= 4.0 && report.mean_rounds <= 8.0 &&
                           report.median_rounds >= 4.0 && report.median_rounds <= 8.0;

    if (total_actions > 0) {
        for (const auto& [act_label, count] : action_totals)
            report.action_share_pct[act_label] =
                100.0 * static_cast<double>(count) / static_cast<double>(total_actions);
    }
    return report;
}

// Orquestra N batalhas do MESMO cenario (seeds base_seed..base_seed+N-1) e agrega.
[[nodiscard]] inline BalanceReport run_scenario(const BalanceScenario& scenario, int n_battles,
                                                std::uint32_t base_seed) {
    std::vector<BattleOutcome> outcomes;
    outcomes.reserve(static_cast<std::size_t>(std::max(n_battles, 0)));
    for (int i = 0; i < n_battles; ++i)
        outcomes.push_back(
            run_single_battle(scenario, base_seed + static_cast<std::uint32_t>(i)));
    return aggregate(scenario.label, outcomes);
}

// Imprime o relatorio num formato legivel (linha de sumario + top-3 acoes dominantes).
inline void print_report(std::ostream& out, const BalanceReport& r) {
    out << "[" << r.label << "] N=" << r.battles << " win-rate=" << r.win_rate_pct << "%"
        << " (V=" << r.victories << " D=" << r.defeats << " F=" << r.fled << ")"
        << " rounds mean=" << r.mean_rounds << " median=" << r.median_rounds
        << " p95=" << r.p95_rounds
        << " janela-4-8=" << (r.window_4_8_ok ? "OK" : "FORA") << "\n";

    std::vector<std::pair<std::string, double>> shares(r.action_share_pct.begin(),
                                                        r.action_share_pct.end());
    std::sort(shares.begin(), shares.end(),
             [](const auto& a, const auto& b) { return a.second > b.second; });
    out << "  dominancia:";
    const std::size_t top_n = std::min<std::size_t>(shares.size(), 5);
    for (std::size_t i = 0; i < top_n; ++i)
        out << " " << shares[i].first << "=" << shares[i].second << "%";
    out << "\n";
}

// Cenarios representativos (party de referencia secao 17: Gus + Caua + Jaci) vs
// composicoes variadas de inimigo (secao 17: Sentinela-Bit Trash, Daemon-Guard Elite).
//
// ATENCAO (nao-canonico): Atk do Daemon-Guard e SPD de ambos inimigos sao TBD na secao 17
// ("Atk do Daemon-Guard e SPD dos inimigos = TBD, definir na implementacao/playtest"). Os
// valores abaixo sao PLACEHOLDERS DE MEDICAO (escalados a partir do Sentinela-Bit pela
// razao de HP), NAO calibracao canonica; a calibracao final e do economy-designer.
[[nodiscard]] inline std::vector<BalanceScenario> canonical_scenarios() {
    using gus::domain::combat::CardFamily;

    const BalanceActorSpec gus{"gus", 34, 8, 5, 9, CardFamily::Eletrico};
    const BalanceActorSpec caua{"caua", 55, 14, 8, 13, CardFamily::Eletrico};
    const BalanceActorSpec jaci{"jaci", 55, 9, 10, 7, CardFamily::Bioquimico};
    const std::vector<BalanceActorSpec> party{gus, caua, jaci};

    // Sentinela-Bit (Trash, secao 17): HP55/Atk10(provisorio)/Def8/Cinetico.
    const BalanceActorSpec sentinela1{"sentinela1", 55, 10, 8, 5, CardFamily::Cinetico};
    const BalanceActorSpec sentinela2{"sentinela2", 55, 10, 8, 5, CardFamily::Cinetico};
    const BalanceActorSpec sentinela3{"sentinela3", 55, 10, 8, 5, CardFamily::Cinetico};

    // Daemon-Guard (Elite, secao 17): HP144/Def14/Cinetico. Atk PLACEHOLDER (ver nota acima).
    const BalanceActorSpec daemon{"daemon", 144, 18, 14, 6, CardFamily::Cinetico};

    return {
        BalanceScenario{"party-vs-1trash", party, {sentinela1}},
        BalanceScenario{"party-vs-2trash", party, {sentinela1, sentinela2}},
        BalanceScenario{"party-vs-3trash", party, {sentinela1, sentinela2, sentinela3}},
        BalanceScenario{"party-vs-1elite", party, {daemon}},
        BalanceScenario{"party-vs-1elite+1trash", party, {daemon, sentinela1}},
    };
}

}  // namespace gus::domain::tests

#endif  // GUS_DOMAIN_TESTS_BALANCE_HARNESS_HPP

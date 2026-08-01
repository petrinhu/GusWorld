// SPDX-License-Identifier: Apache-2.0
// mira_sim_harness.hpp (dev tool, test-only)
//
// Harness de simulacao estatistica da mira inimiga (MIRA-SIM). Implementa o protocolo
// APROVADO pelo lider em docs/design/mecanicas/proposta-protocolo-simulacao-mira.md
// (secao 2: 6 bracos x 7 lotes; secao 3: N; secao 4: 8 metricas; secao 5: valores //SIM).
//
// PRINCIPIO (protocolo secao 0): roda o motor REAL (CombatStateMachine::run_until_end,
// POCO puro, mesmo caminho de balance_harness.hpp) headless. Nenhuma regra de combate e
// reimplementada aqui - dano/Shield/fila/RNG sao os do motor. As UNICAS pecas que este
// harness acrescenta sao TEST-ONLY: (a) as politicas de mira (bracos, injetadas no lugar
// do ScriptedBrain::decide_action pra inimigos), (b) as variantes de bot da party (nao
// existem em producao), (c) a telemetria de medicao.
//
// TELEMETRIA sem reimplementar formula: usa CombatStateMachine::preview_basic_attack_damage
// (metodo PURO e ja existente, projetado exatamente pra isto: "perda de HP PREVISTA de um
// ataque basico ... antes de confirmar") pra obter o dano REAL pos-Shield de cada ataque
// ANTES de devolver a CombatAction ao motor. O dano bruto (pre-Shield) e o MESMO
// max(kMinDamage, atk-def) ja usado por ScriptedBrain::preview_intent (scripted_brain.cpp) -
// nao e uma formula nova, e a formula canonica (secao 11/17) espelhada no MESMO padrao.
// Ataque basico e determinístico (secao 17: "sem variancia/canal/fraqueza"), entao o
// preview bate byte-a-byte com o que o motor de fato aplica.
//
// GAPS DE ENGINE encontrados (reportados, NAO contornados por reimplementacao):
//   1. Bonus de +10% Def do honeypot (F5, V6): nao ha primitiva PUBLICA pra um buff
//      TEMPORARIO de Def fora do StatusId fixo (Break so reduz; nao ha "Fortify"). Reusar
//      um StatusId existente pra outro fim seria gambiarra semantica. Este harness aplica
//      SO o peso de mira (x3.0) do honeypot; o bonus de Def NAO afeta o dano resolvido
//      pelo motor nesta fatia. Ver relatorio de entrega.
//   2. Cura da Jaci (L4, V10): nao existe CombatActionType de cura nem carta de cura
//      canonica. Usa CombatActor::heal() (metodo PUBLICO, documentado pra isto) diretamente,
//      fora do pipeline UseCard/resolve_use_card; a acao devolvida ao motor e Pass (Jaci
//      gasta o turno). Nao e reimplementacao de regra: heal() e o mesmo metodo que
//      qualquer carta de cura real usaria.
//
// CAP DE RODADAS (V12 //SIM = 30): o motor NAO tem cap de rodadas embutido (run_until_end
// tem so um teto de 10000 TURNOS que lanca excecao - nao um "empate" gracioso). Para nao
// reimplementar a FSM (os metodos de driving fino como expire_on_stunned_turn_end sao
// PRIVADOS), o cap e obtido via a PROPRIA mecanica publica de Fuga: ao alcancar
// round_index() >= kRoundCap, o bot da party forca CombatAction::flee() (resolve_flee e
// deterministico: sucesso se SPD top da party >= SPD top dos inimigos vivos - verdadeiro
// pra todos os cenarios deste estudo, ja que Caua SPD 13 > qualquer inimigo especificado).
// O resultado e classificado como "empate tecnico" (capped=true) quando o Fled ocorre
// exatamente no/apos o cap, distinguindo de uma fuga real (que nenhum bot deste harness
// escolhe antes do cap).
//
// Cross-ref: docs/design/mecanicas/proposta-protocolo-simulacao-mira.md;
//            docs/design/mecanicas/proposta-mira-inimiga.md;
//            docs/design/mecanicas/combat.md secao 11/15.1/17;
//            balance_harness.hpp (padrao de harness ja existente).

#ifndef GUS_DOMAIN_TESTS_MIRA_SIM_HARNESS_HPP
#define GUS_DOMAIN_TESTS_MIRA_SIM_HARNESS_HPP

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <deque>
#include <functional>
#include <numeric>
#include <optional>
#include <ostream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "gus/domain/combat/combat_actor.hpp"
#include "gus/domain/combat/combat_constants.hpp"
#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/combat/combat_records.hpp"
#include "gus/domain/combat/combat_state.hpp"
#include "gus/domain/combat/combat_state_machine.hpp"
#include "property_gen.hpp"  // PropertyRandom (LCG seedado, ja existente, marco M5)

namespace gus::domain::tests::mira_sim {

using gus::domain::combat::CardFamily;
using gus::domain::combat::CombatAction;
using gus::domain::combat::CombatActor;
using gus::domain::combat::CombatOutcome;
using gus::domain::combat::CombatResult;
using gus::domain::combat::CombatState;
using gus::domain::combat::CombatStateMachine;
using gus::domain::combat::StatusId;
namespace combat_constants = gus::domain::combat::combat_constants;

// ============================================================================
// Valores //SIM (protocolo secao 5). NENHUM entra em codigo de producao: sao chutes
// razoaveis para o estudo, revalidados pelo economy-designer/lider na onda de
// implementacao real.
// ============================================================================

// V1 //SIM: peso base de mira por heroi vivo.
inline constexpr double kBaseWeight = 100.0;
// V2 //SIM F1 (dano causado atrai): +1 de peso por ponto de dano nas ultimas 2 rodadas.
inline constexpr double kF1WeightPerDamagePoint = 1.0;
// V3 //SIM F2 (ferido atrai): +100 x (1 - hp/hpmax).
inline constexpr double kF2LowHpWeightScale = 100.0;
// V4 //SIM F3 (suporte atrai): +60 se curou/buffou aliado nas ultimas 2 rodadas.
inline constexpr double kF3SupportWeight = 60.0;
// V6 //SIM honeypot: peso x3.0 por 2 rodadas (SO o peso; bonus de Def NAO implementado
// nesta fatia, ver cabecalho).
inline constexpr double kHoneypotWeightMultiplier = 3.0;
inline constexpr int kHoneypotDurationRounds = 2;
// V7 firewall DROP: peso x0 por 2 rodadas; fallback uniforme se TODOS os vivos zerarem.
inline constexpr int kFirewallDurationRounds = 2;
// V10 //SIM cura da Jaci (L4): 12 HP no aliado mais ferido.
inline constexpr int kHealerHealAmount = 12;
// V12 //SIM cap de rodadas (empate tecnico via Flee forcado, ver cabecalho).
inline constexpr int kRoundCap = 30;
// Janela de memoria curta de F1/F3 ("ultimas 2 rodadas" = rodada corrente + anterior).
inline constexpr int kAttractionWindowRounds = 2;
// V8/V9 //SIM (proposta-protocolo secao 5): stats do Daemon-Guard (Atk/SPD sao TBD no
// canon, secao 17). Def/HP sao os do encontro de referencia (combat.md secao 17).
inline constexpr int kDaemonGuardAtk = 18;
inline constexpr int kDaemonGuardDef = 14;
inline constexpr int kDaemonGuardHp = 144;
inline constexpr int kDaemonGuardSpd = 10;
inline constexpr int kSentinelaSpd = 8;

// ============================================================================
// Bracos (politicas de mira inimiga). Protocolo secao 2.1.
// ============================================================================

enum class MiraArm : int {
    A_StatusQuo = 0,   // players.front() - o comportamento de hoje (scripted_brain.cpp)
    B_Uniform = 1,      // sorteio uniforme entre vivos
    C_F4Soft = 2,       // ponderada, F4 x0.5 ("evita a parede, mas nao sempre")
    D_F4Strong = 3,     // ponderada, F4 x0.1 ("quase nunca bate na parede")
    E_F4Inverted = 4,   // ponderada, F4 x2.0 ("quebra a parede cedo")
    F_NoF4 = 5,         // ponderada, sem F4 (x1.0) - isola o que F4 acrescenta
};

inline constexpr std::array<MiraArm, 6> kAllArms{
    MiraArm::A_StatusQuo, MiraArm::B_Uniform,  MiraArm::C_F4Soft,
    MiraArm::D_F4Strong,  MiraArm::E_F4Inverted, MiraArm::F_NoF4,
};

[[nodiscard]] inline std::string arm_label(MiraArm arm) {
    switch (arm) {
        case MiraArm::A_StatusQuo: return "A_status_quo";
        case MiraArm::B_Uniform: return "B_uniforme";
        case MiraArm::C_F4Soft: return "C_F4_suave";
        case MiraArm::D_F4Strong: return "D_F4_forte";
        case MiraArm::E_F4Inverted: return "E_F4_invertido";
        case MiraArm::F_NoF4: return "F_sem_F4";
    }
    throw std::logic_error("MiraArm desconhecido.");
}

// V5 //SIM: multiplicador de F4 (defendendo/Shield ativo) por braco. So usado pelos
// bracos ponderados (C/D/E/F); A/B nao consultam F1-F4 (ver decide_enemy_action).
[[nodiscard]] inline double f4_multiplier_for(MiraArm arm) {
    switch (arm) {
        case MiraArm::C_F4Soft: return 0.5;
        case MiraArm::D_F4Strong: return 0.1;
        case MiraArm::E_F4Inverted: return 2.0;
        case MiraArm::F_NoF4: return 1.0;
        default: return 1.0;  // A/B: nunca chamado (bracos nao-ponderados)
    }
}

[[nodiscard]] inline bool is_weighted_arm(MiraArm arm) {
    return arm != MiraArm::A_StatusQuo && arm != MiraArm::B_Uniform;
}

// ============================================================================
// Lotes (cenarios). Protocolo secao 2.2. Party fixa em todos: Gus/Caua/Jaci, indices
// 0/1/2 (PartyMember). So a ESTRATEGIA de party e a composicao/HP inicial de inimigos
// variam por lote.
// ============================================================================

enum class Lote : int {
    L1_Vanilla = 0,
    L2_ParedeDedicada = 1,
    L3_TurtleTotal = 2,
    L4_SuporteAtivo = 3,
    L5_CartasDeAgro = 4,
    L6_VantagemInimigos = 5,
    L7_AtacanteUnico = 6,
};

inline constexpr std::array<Lote, 7> kAllLotes{
    Lote::L1_Vanilla,          Lote::L2_ParedeDedicada, Lote::L3_TurtleTotal,
    Lote::L4_SuporteAtivo,     Lote::L5_CartasDeAgro,   Lote::L6_VantagemInimigos,
    Lote::L7_AtacanteUnico,
};

[[nodiscard]] inline std::string lote_label(Lote l) {
    switch (l) {
        case Lote::L1_Vanilla: return "L1_vanilla";
        case Lote::L2_ParedeDedicada: return "L2_parede_dedicada";
        case Lote::L3_TurtleTotal: return "L3_turtle_total";
        case Lote::L4_SuporteAtivo: return "L4_suporte_ativo";
        case Lote::L5_CartasDeAgro: return "L5_cartas_de_agro";
        case Lote::L6_VantagemInimigos: return "L6_vantagem_inimigos";
        case Lote::L7_AtacanteUnico: return "L7_atacante_unico";
    }
    throw std::logic_error("Lote desconhecido.");
}

// Party fixa (protocolo secao 2.2, "Fixo em todos os lotes"): indice = PartyMember.
enum class PartyMember : int { Gus = 0, Caua = 1, Jaci = 2 };
inline constexpr int kPartySize = 3;

[[nodiscard]] inline int idx(PartyMember m) { return static_cast<int>(m); }

[[nodiscard]] inline PartyMember member_of(const std::string& id) {
    if (id == "gus") return PartyMember::Gus;
    if (id == "caua") return PartyMember::Caua;
    if (id == "jaci") return PartyMember::Jaci;
    throw std::logic_error("id de party desconhecido no harness de mira: " + id);
}

// Papel de party por lote (protocolo secao 2.2). AttackFront = ataque basico no 1o
// inimigo vivo (mesmo estilo sub-otimo de balance_harness.hpp/AutoResolveBrain secao 19.6).
enum class PartyRole {
    AttackFront,
    DefendSelf,
    HealMostInjured,
    HoneypotEntaoAtaca,   // L5/Caua: joga honeypot na rodada 1, ataca depois
    FirewallEntaoAtaca,   // L5/Gus: recebe firewall pre-aplicado, ataca sempre
};

[[nodiscard]] inline std::array<PartyRole, kPartySize> roles_for(Lote l) {
    using R = PartyRole;
    switch (l) {
        case Lote::L1_Vanilla:
            return {R::AttackFront, R::AttackFront, R::AttackFront};
        case Lote::L2_ParedeDedicada:
            return {R::AttackFront, R::AttackFront, R::DefendSelf};  // Jaci = parede
        case Lote::L3_TurtleTotal:
            return {R::DefendSelf, R::DefendSelf, R::DefendSelf};
        case Lote::L4_SuporteAtivo:
            return {R::AttackFront, R::AttackFront, R::HealMostInjured};  // Jaci cura
        case Lote::L5_CartasDeAgro:
            // Suposicao explicita (protocolo nao fixa o turno 1 de Gus/Caua): Caua paga o
            // turno 1 pra jogar honeypot; Gus "recebe" firewall pre-aplicado (sem custo de
            // turno) e ataca desde a rodada 1; Jaci ataca sempre. Ver relatorio de entrega.
            return {R::FirewallEntaoAtaca, R::HoneypotEntaoAtaca, R::AttackFront};
        case Lote::L6_VantagemInimigos:
            return {R::AttackFront, R::AttackFront, R::DefendSelf};  // Jaci = parede sob pressao
        case Lote::L7_AtacanteUnico:
            return {R::AttackFront, R::AttackFront, R::DefendSelf};
    }
    throw std::logic_error("Lote desconhecido em roles_for.");
}

// Especificacao de UM ator (party ou inimigo), espelha BalanceActorSpec (balance_harness.hpp).
struct ActorSpec {
    std::string id;
    int hp = 1;
    int atk = 1;
    int def = 0;
    int spd = 1;
    CardFamily family = CardFamily::Eletrico;
    bool is_universal_compiler = false;
};

// Party de referencia (combat.md secao 17 / protocolo secao 2.2, "Fixo em todos os
// lotes"). is_universal_compiler=true no Gus (diferente de balance_harness.hpp, que nao
// seta essa flag): este harness quer o comportamento GUS-CENTRIC real de check_end()
// (Rei caiu = fim), mais fiel ao motor de producao.
[[nodiscard]] inline ActorSpec gus_spec() {
    return ActorSpec{"gus", 34, 8, 5, 9, CardFamily::Eletrico, /*is_universal_compiler=*/true};
}
[[nodiscard]] inline ActorSpec caua_spec() {
    return ActorSpec{"caua", 55, 14, 8, 13, CardFamily::Eletrico, false};
}
[[nodiscard]] inline ActorSpec jaci_spec() {
    return ActorSpec{"jaci", 55, 9, 10, 7, CardFamily::Bioquimico, false};
}
[[nodiscard]] inline std::vector<ActorSpec> party_reference() {
    return {gus_spec(), caua_spec(), jaci_spec()};
}

// Sentinela-Bit (Trash, combat.md secao 17). SPD = V9 //SIM (protocolo secao 5).
[[nodiscard]] inline ActorSpec sentinela_spec(std::string id) {
    return ActorSpec{std::move(id), 55, 10, 8, kSentinelaSpd, CardFamily::Cinetico, false};
}
// Daemon-Guard (Elite, combat.md secao 17). Atk/SPD = V8/V9 //SIM (protocolo secao 5).
[[nodiscard]] inline ActorSpec daemon_spec() {
    return ActorSpec{"daemon", kDaemonGuardHp, kDaemonGuardAtk, kDaemonGuardDef,
                     kDaemonGuardSpd, CardFamily::Cinetico, false};
}

// Cenario completo de um lote: party + inimigos + fracao de HP inicial da party
// (L6 = 0.70; demais = 1.0).
struct LoteScenario {
    Lote lote;
    std::vector<ActorSpec> party;
    std::vector<ActorSpec> enemies;
    double party_hp_fraction_start = 1.0;
};

[[nodiscard]] inline LoteScenario scenario_for(Lote l) {
    LoteScenario s;
    s.lote = l;
    s.party = party_reference();
    switch (l) {
        case Lote::L1_Vanilla:
        case Lote::L2_ParedeDedicada:
        case Lote::L3_TurtleTotal:
        case Lote::L4_SuporteAtivo:
        case Lote::L5_CartasDeAgro:
            s.enemies = {sentinela_spec("sentinela1"), sentinela_spec("sentinela2"),
                        sentinela_spec("sentinela3")};
            s.party_hp_fraction_start = 1.0;
            break;
        case Lote::L6_VantagemInimigos:
            s.enemies = {daemon_spec(), sentinela_spec("sentinela1"), sentinela_spec("sentinela2")};
            s.party_hp_fraction_start = 0.70;
            break;
        case Lote::L7_AtacanteUnico:
            s.enemies = {daemon_spec()};
            s.party_hp_fraction_start = 1.0;
            break;
    }
    return s;
}

// ============================================================================
// Rastreador de atracao (F1/F3 de janela + contadores F5/F6 de honeypot/firewall).
// Indexado por PartyMember (party fixa de 3, protocolo secao 2.2).
// ============================================================================

class AttractionTracker {
public:
    // Chamado no TOPO de toda invocacao do provider (party ou inimigo): decrementa os
    // contadores de honeypot/firewall pela quantidade de rodadas que passaram desde a
    // ultima observacao. Idempotente dentro da MESMA rodada (delta=0).
    void observe_round(int round) {
        if (last_seen_round_ < 0) {
            last_seen_round_ = round;
            return;
        }
        const int delta = round - last_seen_round_;
        if (delta <= 0) return;
        for (int i = 0; i < kPartySize; ++i) {
            honeypot_rounds_left[i] = std::max(0, honeypot_rounds_left[i] - delta);
            firewall_rounds_left[i] = std::max(0, firewall_rounds_left[i] - delta);
        }
        last_seen_round_ = round;
    }

    // F1 (V2): registra dano causado por `m` na rodada `round`.
    void record_damage(PartyMember m, int round, int amount) {
        damage_window_[idx(m)].emplace_back(round, amount);
        prune(damage_window_[idx(m)], round);
    }

    // F3 (V4): registra que `m` deu suporte (curou/buffou) na rodada `round`.
    void record_support(PartyMember m, int round) {
        support_window_[idx(m)].emplace_back(round, true);
        prune(support_window_[idx(m)], round);
    }

    // Soma de dano de `m` na janela de kAttractionWindowRounds rodadas (inclui `round`).
    [[nodiscard]] int damage_last_window(PartyMember m, int round) const {
        int total = 0;
        for (const auto& [r, amount] : damage_window_[idx(m)])
            if (r >= round - (kAttractionWindowRounds - 1)) total += amount;
        return total;
    }

    // true se `m` deu suporte dentro da janela de kAttractionWindowRounds rodadas.
    [[nodiscard]] bool supported_last_window(PartyMember m, int round) const {
        for (const auto& [r, flag] : support_window_[idx(m)])
            if (flag && r >= round - (kAttractionWindowRounds - 1)) return true;
        return false;
    }

    std::array<int, kPartySize> honeypot_rounds_left{0, 0, 0};  // F5 (V6)
    std::array<int, kPartySize> firewall_rounds_left{0, 0, 0};  // F6 (V7)

private:
    template <typename T>
    static void prune(std::deque<std::pair<int, T>>& window, int round) {
        const int cutoff = round - (kAttractionWindowRounds - 1);
        while (!window.empty() && window.front().first < cutoff) window.pop_front();
    }

    std::array<std::deque<std::pair<int, int>>, kPartySize> damage_window_;
    std::array<std::deque<std::pair<int, bool>>, kPartySize> support_window_;
    int last_seen_round_ = -1;
};

// Nota de atracao (V1 + F1 + F2 + F3), ANTES do multiplicador F4/F5/F6 (protocolo secao 5,
// proposta-mira-inimiga.md secao 1). So usado pelos bracos ponderados (C/D/E/F).
[[nodiscard]] inline double compute_target_weight(PartyMember m, const CombatActor& actor,
                                                   int round, const AttractionTracker& tr,
                                                   MiraArm arm) {
    double w = kBaseWeight;  // V1
    w += kF1WeightPerDamagePoint * static_cast<double>(tr.damage_last_window(m, round));  // F1/V2
    const double hp_frac = actor.max_hp() > 0
                               ? static_cast<double>(actor.hp()) / static_cast<double>(actor.max_hp())
                               : 0.0;
    w += kF2LowHpWeightScale * (1.0 - hp_frac);  // F2/V3
    if (tr.supported_last_window(m, round)) w += kF3SupportWeight;  // F3/V4

    if (actor.index_of_status(StatusId::Shield) >= 0) w *= f4_multiplier_for(arm);  // F4/V5
    if (tr.honeypot_rounds_left[idx(m)] > 0) w *= kHoneypotWeightMultiplier;  // F5/V6
    if (tr.firewall_rounds_left[idx(m)] > 0) w *= 0.0;  // F6/V7

    return std::max(0.0, w);
}

// Sorteio ponderado (fallback uniforme se total<=0, V7: "todos protegidos -> default").
[[nodiscard]] inline CombatActor* pick_weighted(const std::vector<CombatActor*>& alive,
                                                const std::vector<double>& weights,
                                                gus::domain::combat::IRandomSource& rng) {
    const double total = std::accumulate(weights.begin(), weights.end(), 0.0);
    if (total <= 0.0) {
        const int i = rng.next(static_cast<int>(alive.size()));
        return alive[static_cast<std::size_t>(i)];
    }
    const double roll = rng.next_double() * total;
    double cum = 0.0;
    for (std::size_t i = 0; i < alive.size(); ++i) {
        cum += weights[i];
        if (roll < cum) return alive[i];
    }
    return alive.back();  // fallback numerico (arredondamento de ponto flutuante)
}

// ============================================================================
// BattleTrace: dados brutos de UMA luta (protocolo secao 4, E1-E8). Pura, sem I/O.
// ============================================================================

struct BattleTrace {
    CombatOutcome outcome = CombatOutcome::Ongoing;
    bool capped = false;  // empate tecnico (V12): Fled forcado no/apos round_index>=cap
    int rounds = 0;

    // E3 (concentracao): dano REAL (pos-Shield) recebido por cada membro da party.
    std::array<int, kPartySize> damage_taken{0, 0, 0};
    // E4/E8 (quedas): flag + rodada da 1a queda (-1 = ninguem caiu).
    std::array<bool, kPartySize> fell{false, false, false};
    int first_fall_round = -1;

    // E5 (previsibilidade): repeticoes de alvo turno-inimigo a turno-inimigo.
    int enemy_target_repeats = 0;
    int enemy_target_turns = 0;

    // E6 (eficiencia da parede): dano bruto vs absorvido, so quando o alvo tinha Shield.
    long shield_raw_total = 0;
    long shield_absorbed_total = 0;

    // E7 (exploit de defesa): quantas vezes cada membro defendeu, e quantas dessas vezes
    // foi de fato atacado (usado pra derivar "pool que expirou cheio", protocolo E6).
    std::array<int, kPartySize> defend_count{0, 0, 0};
    std::array<int, kPartySize> hits_while_defending{0, 0, 0};

    // Estado final (HP fracao) por membro, pra E7 (HP final medio da party).
    std::array<double, kPartySize> final_hp_fraction{0.0, 0.0, 0.0};

    [[nodiscard]] bool any_fell() const {
        return fell[0] || fell[1] || fell[2];
    }
    // Dano bruto "desperdicado": rodadas em que o membro defendeu mas NAO foi atacado
    // (pool renovado e nunca tocado). Aproximacao documentada (ver cabecalho do arquivo):
    // usa def() do membro no MOMENTO da agregacao (chamador injeta), ja que o Shield
    // aplicado por resolve_defend tem magnitude == def() do defensor.
    [[nodiscard]] int wasted_defend_rounds(PartyMember m) const {
        return std::max(0, defend_count[idx(m)] - hits_while_defending[idx(m)]);
    }
};

// ============================================================================
// Decisao de acao (party e inimigo).
// ============================================================================

// Decide a acao de UM membro da party, dado o papel (protocolo secao 2.2). `sm` e usado
// SO pra estimate/preview (F1 bookkeeping); nunca muta o motor por fora do provider.
[[nodiscard]] inline CombatAction decide_party_action(PartyMember m, PartyRole role,
                                                       CombatActor& actor,
                                                       const CombatState& state,
                                                       const CombatStateMachine& sm,
                                                       AttractionTracker& tr, int round) {
    // V12 //SIM: cap de rodadas via Fuga forcada (ver cabecalho do arquivo). Testado
    // determinístico pra este estudo (Caua SPD 13 > qualquer inimigo especificado).
    if (round >= kRoundCap) return CombatAction::flee();

    switch (role) {
        case PartyRole::AttackFront:
        case PartyRole::FirewallEntaoAtaca: {
            if (role == PartyRole::FirewallEntaoAtaca && round == 0)
                tr.firewall_rounds_left[idx(m)] = kFirewallDurationRounds;  // V7, pre-aplicado
            const auto enemies = state.alive_enemies();
            if (enemies.empty()) return CombatAction::pass();
            CombatActor* target = enemies.front();
            const int predicted = sm.preview_basic_attack_damage(actor, *target);
            tr.record_damage(m, round, predicted);  // F1/V2 bookkeeping
            return CombatAction::attack(target->id());
        }
        case PartyRole::HoneypotEntaoAtaca: {
            if (round == 0) {
                tr.honeypot_rounds_left[idx(m)] = kHoneypotDurationRounds;  // V6
                // Bonus de +10% Def NAO aplicado (gap de engine, ver cabecalho do arquivo).
                return CombatAction::pass();
            }
            const auto enemies = state.alive_enemies();
            if (enemies.empty()) return CombatAction::pass();
            CombatActor* target = enemies.front();
            const int predicted = sm.preview_basic_attack_damage(actor, *target);
            tr.record_damage(m, round, predicted);
            return CombatAction::attack(target->id());
        }
        case PartyRole::DefendSelf:
            return CombatAction::defend();
        case PartyRole::HealMostInjured: {
            const auto players = state.alive_players();
            if (players.empty()) return CombatAction::pass();
            CombatActor* target = *std::min_element(
                players.begin(), players.end(), [](const CombatActor* a, const CombatActor* b) {
                    const double fa = a->max_hp() > 0
                                          ? static_cast<double>(a->hp()) / a->max_hp()
                                          : 0.0;
                    const double fb = b->max_hp() > 0
                                          ? static_cast<double>(b->hp()) / b->max_hp()
                                          : 0.0;
                    return fa < fb;
                });
            // V10 //SIM: heal() e o metodo PUBLICO real (nao reimplementa regra); gap de
            // engine e a ausencia de uma CombatAction de cura (ver cabecalho do arquivo).
            target->heal(kHealerHealAmount);
            tr.record_support(m, round);  // F3/V4 bookkeeping
            return CombatAction::pass();
        }
    }
    throw std::logic_error("PartyRole desconhecido.");
}

// Decide a acao de UM inimigo (braco de mira), com telemetria E3/E4/E5/E6/E8 embutida
// no MOMENTO da decisao (preview PURO, motor real - ver cabecalho do arquivo).
[[nodiscard]] inline CombatAction decide_enemy_action(
    MiraArm arm, CombatActor& self, const CombatState& state, const CombatStateMachine& sm,
    AttractionTracker& tr, BattleTrace& trace, gus::domain::combat::IRandomSource& rng,
    int round, std::optional<std::string>& last_enemy_target) {
    const auto players = state.alive_players();
    if (players.empty()) return CombatAction::pass();

    CombatActor* target = nullptr;
    if (arm == MiraArm::A_StatusQuo) {
        target = players.front();  // replica scripted_brain.cpp:26 EXATAMENTE
    } else if (arm == MiraArm::B_Uniform) {
        target = players[static_cast<std::size_t>(rng.next(static_cast<int>(players.size())))];
    } else {
        std::vector<double> weights;
        weights.reserve(players.size());
        for (CombatActor* p : players)
            weights.push_back(compute_target_weight(member_of(p->id()), *p, round, tr, arm));
        target = pick_weighted(players, weights, rng);
    }

    const int raw = std::max(combat_constants::kMinDamage, self.atk() - target->def());
    const int net = sm.preview_basic_attack_damage(self, *target);
    const bool defending = target->index_of_status(StatusId::Shield) >= 0;
    if (defending) {
        trace.shield_raw_total += raw;
        trace.shield_absorbed_total += std::max(0, raw - net);
        trace.hits_while_defending[idx(member_of(target->id()))]++;
    }

    const PartyMember tm = member_of(target->id());
    trace.damage_taken[idx(tm)] += net;
    if (!trace.fell[idx(tm)] && net >= target->hp()) {
        trace.fell[idx(tm)] = true;
        if (trace.first_fall_round < 0) trace.first_fall_round = round;
    }

    trace.enemy_target_turns++;
    if (last_enemy_target.has_value() && *last_enemy_target == target->id())
        trace.enemy_target_repeats++;
    last_enemy_target = target->id();

    return CombatAction::attack(target->id());
}

// ============================================================================
// Execucao de UMA luta (pareavel por seed: mesma seed sob os 6 bracos = mesma luta).
// ============================================================================

[[nodiscard]] inline BattleTrace run_single_mira_battle(Lote lote, MiraArm arm,
                                                        std::uint32_t seed) {
    const LoteScenario scenario = scenario_for(lote);
    const std::array<PartyRole, kPartySize> roles = roles_for(lote);

    std::deque<CombatActor> actors;
    for (const ActorSpec& s : scenario.party)
        actors.emplace_back(s.id, s.id, s.hp, s.atk, s.def, s.spd, s.family,
                           /*is_player_side=*/true, /*is_boss=*/false, /*knowledge_kills=*/0,
                           s.is_universal_compiler);
    for (const ActorSpec& s : scenario.enemies)
        actors.emplace_back(s.id, s.id, s.hp, s.atk, s.def, s.spd, s.family,
                           /*is_player_side=*/false);

    std::vector<CombatActor*> actor_ptrs;
    actor_ptrs.reserve(actors.size());
    for (auto& a : actors) actor_ptrs.push_back(&a);

    // L6: party entra com 70% do HP (protocolo secao 2.2). take_damage() e o metodo
    // publico real; sem Shield ativo no inicio, e uma reducao de HP limpa.
    if (scenario.party_hp_fraction_start < 1.0) {
        for (std::size_t i = 0; i < scenario.party.size(); ++i) {
            CombatActor& a = actors[i];
            const int keep = static_cast<int>(
                std::lround(static_cast<double>(a.max_hp()) * scenario.party_hp_fraction_start));
            const int to_remove = a.max_hp() - keep;
            if (to_remove > 0) a.take_damage(to_remove);
        }
    }

    AttractionTracker tr;
    BattleTrace trace;
    std::optional<std::string> last_enemy_target;
    PropertyRandom rng(seed);
    const CombatStateMachine* sm_ptr = nullptr;

    // GAP DE ENGINE (reportado, nao contornado por reimplementacao de regra): CombatActor
    // nao tem setter publico de max_ap - TODO ator recebe kBaseApPerTurn=3 (combat_actor.hpp),
    // sem diferenciacao trash/elite/party (combat.md secao 13.1/17 diz "1 AP" pro trash e
    // "no VS todos a 1 AP", mas isso NAO esta wireado no motor ainda). Sem isto, o loop de
    // ActionSelect<->ActionResolve chamaria o MESMO ator ate 3x por turno (3 AP), triplicando
    // dano/cura/defesa e distorcendo exatamente a variavel que o estudo mede. Este harness
    // contorna via "1 decisao real por turno, Pass nas chamadas extras do MESMO turno" -
    // decisao do BOT (party e inimigo), NAO do motor: Pass sempre encerra o turno (secao 3),
    // entao o efeito observado E o de "1 AP" sem mudar nenhuma formula de dano/Shield.
    std::unordered_map<std::string, int> last_decided_round;

    auto provider = [&](CombatActor& actor, const CombatState& state) -> CombatAction {
        const int round = state.round_index();
        tr.observe_round(round);

        const auto it = last_decided_round.find(actor.id());
        if (it != last_decided_round.end() && it->second == round) return CombatAction::pass();
        last_decided_round[actor.id()] = round;

        if (actor.is_player_side()) {
            const PartyMember m = member_of(actor.id());
            if (roles[idx(m)] == PartyRole::DefendSelf) trace.defend_count[idx(m)]++;
            return decide_party_action(m, roles[idx(m)], actor, state, *sm_ptr, tr, round);
        }
        return decide_enemy_action(arm, actor, state, *sm_ptr, tr, trace, rng, round,
                                   last_enemy_target);
    };

    CombatStateMachine sm(actor_ptrs, provider, /*card_registry=*/nullptr,
                         /*brain_registry=*/nullptr, &rng);
    sm_ptr = &sm;
    const CombatResult result = sm.run_until_end();

    trace.outcome = result.outcome;
    trace.rounds = result.rounds_elapsed;
    trace.capped = result.outcome == CombatOutcome::Fled && result.rounds_elapsed >= kRoundCap;

    for (std::size_t i = 0; i < scenario.party.size(); ++i) {
        const CombatActor& a = actors[i];
        trace.final_hp_fraction[i] =
            a.max_hp() > 0 ? static_cast<double>(a.hp()) / a.max_hp() : 0.0;
        if (a.hp() <= 0) trace.fell[i] = true;  // reconciliacao defensiva de borda
    }
    return trace;
}

// ============================================================================
// Agregacao (protocolo secao 4, E1-E8) + intervalo de 95%.
// ============================================================================

struct MetricWithCi {
    double value = 0.0;   // em % (proporcoes) ou na unidade nativa (rounds)
    double moe95 = 0.0;    // margem de erro de 95%, mesma unidade de `value`
};

// Proporcao binomial (E1/E2/E4/E5/E6-aprox/E7): p +- 1.96*sqrt(p(1-p)/n), em %.
[[nodiscard]] inline MetricWithCi proportion_metric_pct(long successes, long trials) {
    if (trials <= 0) return {};
    const double p = static_cast<double>(successes) / static_cast<double>(trials);
    const double moe =
        1.96 * std::sqrt(std::max(0.0, p * (1.0 - p)) / static_cast<double>(trials)) * 100.0;
    return {p * 100.0, moe};
}

// Media de uma amostra continua (E3 concentracao por luta; E8 rodada da 1a queda):
// media +- 1.96*desvio_padrao_amostral/sqrt(n).
[[nodiscard]] inline MetricWithCi mean_metric(const std::vector<double>& samples) {
    if (samples.empty()) return {};
    const double n = static_cast<double>(samples.size());
    const double mean = std::accumulate(samples.begin(), samples.end(), 0.0) / n;
    if (samples.size() < 2) return {mean, 0.0};
    double sq_sum = 0.0;
    for (double x : samples) sq_sum += (x - mean) * (x - mean);
    const double sd = std::sqrt(sq_sum / (n - 1.0));
    return {mean, 1.96 * sd / std::sqrt(n)};
}

[[nodiscard]] inline double percentile(std::vector<double> values, double p) {
    if (values.empty()) return 0.0;
    std::sort(values.begin(), values.end());
    const double rank = p * static_cast<double>(values.size() - 1);
    const auto lo = static_cast<std::size_t>(std::floor(rank));
    const auto hi = static_cast<std::size_t>(std::ceil(rank));
    if (lo == hi) return values[lo];
    const double frac = rank - static_cast<double>(lo);
    return values[lo] + frac * (values[hi] - values[lo]);
}

// Relatorio agregado de um (lote, braco) sobre N lutas. Os campos cobrem E1-E8; a
// INTERPRETACAO (qual metrica "e" qual campo, pra qual lote) segue a tabela da secao 4
// do protocolo - documentada em cada campo abaixo.
struct LoteArmReport {
    std::string lote;
    std::string arm;
    int n = 0;

    // E1: taxa de vitoria da party.
    MetricWithCi win_rate_pct;
    // E2: duracao em rodadas (media/mediana/p10/p90) + % dentro da janela 3-5
    // (combat.md secao 15.1, item 7 do brief: SUBSTITUI a janela 4-8 antiga).
    double mean_rounds = 0.0;
    double median_rounds = 0.0;
    double p10_rounds = 0.0;
    double p90_rounds = 0.0;
    MetricWithCi pct_in_window_3_5;
    // E3: concentracao de dano (media do % do dano total que caiu no mais atingido) +
    // % de lutas "saco de pancadas" (>70% num so).
    MetricWithCi concentration_pct;
    MetricWithCi pct_saco_de_pancadas;
    // E4: quedas (qualquer + por membro).
    MetricWithCi pct_any_fall;
    std::array<MetricWithCi, kPartySize> pct_fall_by_member;
    // E5: previsibilidade (repeticao de alvo turno-inimigo a turno-inimigo).
    MetricWithCi repeat_target_pct;
    // E6: eficiencia da parede (dano bruto absorvido) + dano desperdicado (rodadas
    // defendidas sem ser atacado x Def do defensor, aproximacao documentada).
    MetricWithCi shield_absorption_pct;
    long shield_wasted_total = 0;
    // E7 (so L2/L3, lido pelo chamador): reusa win_rate_pct/pct_in_window_3_5 acima +
    // HP final medio da party + % de lutas que bateram no cap de 30 rodadas.
    double avg_final_hp_pct = 0.0;
    MetricWithCi pct_hit_round_cap;
    // E8: rodada da 1a queda (so entre lutas em que alguem caiu).
    double mean_first_fall_round = 0.0;
    double median_first_fall_round = 0.0;
};

// Def canonica de referencia por PartyMember (Gus 5/Caua 8/Jaci 10, protocolo secao 2.2,
// "Fixo em todos os lotes"). Usada SO pra reconstruir o dano desperdicado do Shield em
// aggregate_mira (a BattleTrace nao carrega Def, so HP-fracao) - aproximacao documentada
// no comentario de BattleTrace::wasted_defend_rounds acima.
[[nodiscard]] inline int reference_def_of(PartyMember m) {
    switch (m) {
        case PartyMember::Gus: return gus_spec().def;
        case PartyMember::Caua: return caua_spec().def;
        case PartyMember::Jaci: return jaci_spec().def;
    }
    throw std::logic_error("PartyMember desconhecido em reference_def_of.");
}

[[nodiscard]] inline LoteArmReport aggregate_mira(const std::string& lote,
                                                  const std::string& arm,
                                                  const std::vector<BattleTrace>& traces) {
    LoteArmReport r;
    r.lote = lote;
    r.arm = arm;
    r.n = static_cast<int>(traces.size());
    if (traces.empty()) return r;

    long victories = 0;
    std::vector<double> rounds_d;
    long in_window = 0;
    std::vector<double> concentration_samples;
    long saco_de_pancadas = 0;
    long any_fall = 0;
    std::array<long, kPartySize> fall_by_member{0, 0, 0};
    long repeat_num = 0, repeat_den = 0;
    long shield_raw = 0, shield_absorbed = 0, shield_wasted = 0;
    long hit_cap = 0;
    double final_hp_sum = 0.0;
    int final_hp_count = 0;
    std::vector<double> first_fall_rounds;

    for (const BattleTrace& t : traces) {
        if (t.outcome == CombatOutcome::Victory) ++victories;
        rounds_d.push_back(static_cast<double>(t.rounds));
        if (t.rounds >= 3 && t.rounds <= 5) ++in_window;

        const int total_dmg = t.damage_taken[0] + t.damage_taken[1] + t.damage_taken[2];
        if (total_dmg > 0) {
            const int max_dmg = std::max({t.damage_taken[0], t.damage_taken[1], t.damage_taken[2]});
            const double conc = 100.0 * static_cast<double>(max_dmg) / static_cast<double>(total_dmg);
            concentration_samples.push_back(conc);
            if (conc > 70.0) ++saco_de_pancadas;
        }

        if (t.any_fell()) ++any_fall;
        for (int i = 0; i < kPartySize; ++i)
            if (t.fell[i]) ++fall_by_member[i];

        repeat_num += t.enemy_target_repeats;
        repeat_den += t.enemy_target_turns;

        shield_raw += t.shield_raw_total;
        shield_absorbed += t.shield_absorbed_total;
        for (int i = 0; i < kPartySize; ++i) {
            const auto m = static_cast<PartyMember>(i);
            shield_wasted += t.wasted_defend_rounds(m) * reference_def_of(m);
        }

        if (t.capped) ++hit_cap;
        for (int i = 0; i < kPartySize; ++i) {
            final_hp_sum += t.final_hp_fraction[i];
            ++final_hp_count;
        }
        if (t.first_fall_round >= 0) first_fall_rounds.push_back(static_cast<double>(t.first_fall_round));
    }

    r.win_rate_pct = proportion_metric_pct(victories, r.n);
    r.mean_rounds = mean_metric(rounds_d).value;
    r.median_rounds = percentile(rounds_d, 0.5);
    r.p10_rounds = percentile(rounds_d, 0.10);
    r.p90_rounds = percentile(rounds_d, 0.90);
    r.pct_in_window_3_5 = proportion_metric_pct(in_window, r.n);

    r.concentration_pct = mean_metric(concentration_samples);
    r.pct_saco_de_pancadas = proportion_metric_pct(
        saco_de_pancadas, static_cast<long>(concentration_samples.size()));

    r.pct_any_fall = proportion_metric_pct(any_fall, r.n);
    for (int i = 0; i < kPartySize; ++i)
        r.pct_fall_by_member[i] = proportion_metric_pct(fall_by_member[i], r.n);

    r.repeat_target_pct = proportion_metric_pct(repeat_num, repeat_den);

    r.shield_absorption_pct = proportion_metric_pct(shield_absorbed, shield_raw);
    r.shield_wasted_total = shield_wasted;

    r.avg_final_hp_pct = final_hp_count > 0 ? 100.0 * final_hp_sum / final_hp_count : 0.0;
    r.pct_hit_round_cap = proportion_metric_pct(hit_cap, r.n);

    const MetricWithCi first_fall = mean_metric(first_fall_rounds);
    r.mean_first_fall_round = first_fall.value;
    r.median_first_fall_round = percentile(first_fall_rounds, 0.5);

    return r;
}

// ============================================================================
// MCID (piso de relevancia, decisao do lider, protocolo secao 3): diferenca < 5pp entre
// bracos = EMPATE (metricas em %); < 0.5 rodada = EMPATE (metricas em rodadas). NAO
// opcional: consome quem gera o relatorio comparativo (fora do escopo desta fatia).
// ============================================================================

enum class McidVerdict { Empate, DiferencaRelevante };

struct McidComparison {
    double diff_abs = 0.0;
    McidVerdict verdict = McidVerdict::Empate;
};

[[nodiscard]] inline McidComparison mcid_compare_pct(double a, double b) {
    const double diff = std::abs(a - b);
    return {diff, diff < 5.0 ? McidVerdict::Empate : McidVerdict::DiferencaRelevante};
}

[[nodiscard]] inline McidComparison mcid_compare_rounds(double a, double b) {
    const double diff = std::abs(a - b);
    return {diff, diff < 0.5 ? McidVerdict::Empate : McidVerdict::DiferencaRelevante};
}

// ============================================================================
// Orquestracao: roda os 6 bracos de UM lote, pareados por seed (protocolo secao 2.1/7).
// Progresso impresso no formato exato pedido, a cada 1% do lote.
// ============================================================================

[[nodiscard]] inline std::vector<LoteArmReport> run_lote_all_arms(
    Lote lote, int n_per_arm, std::uint32_t base_seed, std::ostream* progress_out,
    int lote_idx_1based, int lote_total) {
    const int n_total_per_lote = n_per_arm * static_cast<int>(kAllArms.size());
    int n_done = 0;
    int last_pct_printed = -1;

    std::vector<LoteArmReport> reports;
    reports.reserve(kAllArms.size());
    for (MiraArm arm : kAllArms) {
        std::vector<BattleTrace> traces;
        traces.reserve(static_cast<std::size_t>(std::max(n_per_arm, 0)));
        for (int i = 0; i < n_per_arm; ++i) {
            // MESMA seed (base + indice da luta) sob os 6 bracos: comparacao pareada
            // (protocolo secao 2.1: "cada braco enfrenta exatamente as mesmas lutas").
            traces.push_back(run_single_mira_battle(lote, arm, base_seed + static_cast<std::uint32_t>(i)));
            ++n_done;
            if (progress_out != nullptr && n_total_per_lote > 0) {
                const int pct = (n_done * 100) / n_total_per_lote;
                if (pct != last_pct_printed) {
                    last_pct_printed = pct;
                    (*progress_out) << "lote [" << lote_idx_1based << "] de [" << lote_total
                                    << "], simulação [" << n_done << "] de [" << n_total_per_lote
                                    << "]\n";
                }
            }
        }
        reports.push_back(aggregate_mira(lote_label(lote), arm_label(arm), traces));
    }
    return reports;
}

}  // namespace gus::domain::tests::mira_sim

#endif  // GUS_DOMAIN_TESTS_MIRA_SIM_HARNESS_HPP

// gus/domain/src/combat/combat_state_machine.cpp
//
// Implementacao da FSM de combate (secao 3). Ver header para o contrato e o mapeamento
// C# -> C++. Espelha CombatStateMachine.cs com paridade 1:1 de comportamento e de
// aritmetica de ponto flutuante (float fiel ao C#: variancia ANTES do crit; divisiva
// secao 11). POCO puro, ZERO Qt, ZERO I/O.
//
// MAPEAMENTO de excecoes C# -> C++:
//   ArgumentException / ArgumentNullException -> std::invalid_argument
//   InvalidOperationException                 -> std::logic_error
//   KeyNotFoundException                      -> std::out_of_range
//   ArgumentOutOfRangeException               -> std::out_of_range
//
// Cross-ref: engine/foundation/turn_combat/CombatStateMachine.cs;
//            docs/design/mecanicas/combat.md secao 3..18; ADR-006.

#include "gus/domain/combat/combat_state_machine.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <string>
#include <utility>

#include "gus/domain/combat/combat_actor.hpp"
#include "gus/domain/combat/combat_constants.hpp"
#include "gus/domain/combat/combat_state.hpp"
#include "gus/domain/combat/combo_table.hpp"
#include "gus/domain/combat/enemy_brain.hpp"
#include "gus/domain/combat/environment_catalog.hpp"
#include "gus/domain/combat/techmagic.hpp"
#include "gus/domain/combat/weakness_wheel.hpp"

namespace gus::domain::combat {

namespace {

// Guarda contra loop infinito caso um provider mal-comportado nunca passe nem gaste AP.
constexpr int kMaxActionsPerTurn = 64;

// Default DETERMINISTICO da porta de RNG (ver header). Usado quando NENHUM rng e injetado,
// mantendo o dominio puro (o C# usaria Random.Shared global, impossivel num dominio
// determinístico). NAO e seeding. Sob a formula da secao 11 (sorteio de canal), "neutro"
// significa SEMPRE COMUM com variancia ZERO: next(100) retorna o topo da faixa (max-1, ex.
// 99) para cair fora de FALHA/CRIT, e next_double 0.5 zera a variancia (dano == danoBase).
class NeutralRandom final : public IRandomSource {
public:
    double next_double() override { return 0.5; }
    int next(int max_value) override { return max_value <= 0 ? 0 : max_value - 1; }
};

NeutralRandom& neutral_random() {
    static NeutralRandom kNeutral;
    return kNeutral;
}

// Nome de uma CardFamily para o log (espelha CardFamily.ToString() do C#: o log do Scan
// checa "familia {target.Family}", testes afere substring "Sonico" etc). So log/UI.
// CARD-FAMILY-UNIVERSAL: "Universal" e so o nome literal do ordinal 5 (mesmo padrao das
// outras 5 entradas, NAO e string de apresentacao nova/traduzida); nenhum ator de
// combate (personagem/inimigo) tem essa family hoje (so-cartas, PS-R1), entao este
// ramo fica de prontidao pro dia em que Scan citar uma carta Universal no log.
[[nodiscard]] const char* family_name(CardFamily f) {
    switch (f) {
        case CardFamily::Eletrico: return "Eletrico";
        case CardFamily::Bioquimico: return "Bioquimico";
        case CardFamily::Sonico: return "Sonico";
        case CardFamily::Cinetico: return "Cinetico";
        case CardFamily::Criptografico: return "Criptografico";
        case CardFamily::Universal: return "Universal";
    }
    return "Desconhecido";
}

// Nome de um EnvironmentId para o log (espelha EnvironmentId.ToString() do C#). O
// ScanEnvironment lista os ambientes ativos e a proxima fase ("Crepusculo"). So log/UI.
[[nodiscard]] const char* environment_name(EnvironmentId id) {
    switch (id) {
        case EnvironmentId::None: return "None";
        case EnvironmentId::Neblina: return "Neblina";
        case EnvironmentId::Chuva: return "Chuva";
        case EnvironmentId::Calor: return "Calor";
        case EnvironmentId::TempestadeEletrica: return "TempestadeEletrica";
        case EnvironmentId::Vento: return "Vento";
        case EnvironmentId::Estatica: return "Estatica";
        case EnvironmentId::Fumaca: return "Fumaca";
        case EnvironmentId::EscuridaoTotal: return "EscuridaoTotal";
        case EnvironmentId::Dia: return "Dia";
        case EnvironmentId::Crepusculo: return "Crepusculo";
        case EnvironmentId::Noite: return "Noite";
        case EnvironmentId::Aurora: return "Aurora";
        case EnvironmentId::Lamacento: return "Lamacento";
        case EnvironmentId::Seco: return "Seco";
        case EnvironmentId::Vinhas: return "Vinhas";
        case EnvironmentId::Gelo: return "Gelo";
        case EnvironmentId::AguaAlagado: return "AguaAlagado";
        case EnvironmentId::MetalCondutor: return "MetalCondutor";
        case EnvironmentId::Bioluminescencia: return "Bioluminescencia";
        case EnvironmentId::PavimentoTesselado: return "PavimentoTesselado";
        case EnvironmentId::TaludeInstavel: return "TaludeInstavel";
        case EnvironmentId::AshlarBruto: return "AshlarBruto";
        case EnvironmentId::SoloFertilRecursivo: return "SoloFertilRecursivo";
        case EnvironmentId::AnomaliaPerlin: return "AnomaliaPerlin";
        case EnvironmentId::EspelhoRessonante: return "EspelhoRessonante";
        case EnvironmentId::DutoCondutorPressurizado: return "DutoCondutorPressurizado";
        case EnvironmentId::ElevacaoDominante: return "ElevacaoDominante";
    }
    return "Desconhecido";
}

// Nome de um CardModifier para o slot da pipeline (espelha modifier.ToString() do C#:
// BuildPipeline usa mod.ToString() como Ref do slot Modifier => casa o combo). Contrato.
[[nodiscard]] const char* modifier_name(CardModifier m) {
    switch (m) {
        case CardModifier::Object: return "Object";
        case CardModifier::Stream: return "Stream";
        case CardModifier::Null: return "Null";
    }
    return "Desconhecido";
}

// Nome de um StatusId para o log (mesmo racional de family_name/modifier_name acima - so
// log/UI). Usado pelo Scan aprimorado (ADR-016 step 8, John Dee/Black-Mirror): lista os
// status ativos do alvo escaneado por nome, quando o scanner porta RevealIntent.
[[nodiscard]] const char* status_name(StatusId id) {
    switch (id) {
        case StatusId::Stun: return "Stun";
        case StatusId::Poison: return "Poison";
        case StatusId::Corrode: return "Corrode";
        case StatusId::Disrupt: return "Disrupt";
        case StatusId::Silence: return "Silence";
        case StatusId::Knockback: return "Knockback";
        case StatusId::Break: return "Break";
        case StatusId::Expose: return "Expose";
        case StatusId::Decrypt: return "Decrypt";
        case StatusId::Shield: return "Shield";
        case StatusId::Regen: return "Regen";
        case StatusId::Haste: return "Haste";
        case StatusId::Slow: return "Slow";
        case StatusId::SobrecargaTermica: return "SobrecargaTermica";
        case StatusId::Resfriamento: return "Resfriamento";
        case StatusId::Reflect: return "Reflect";
        case StatusId::BlindagemEM: return "BlindagemEM";
        case StatusId::NullProof: return "NullProof";
        case StatusId::Scrying: return "Scrying";
    }
    return "Desconhecido";
}

// Localiza um ator na ordem da fila por id (nullptr se ausente). Espelha
// Queue.Order.FirstOrDefault(a => a.Id == id) do C#.
[[nodiscard]] CombatActor* find_in_order(const InitiativeQueue& queue,
                                         const std::string& id) {
    for (CombatActor* a : queue.order())
        if (a->id() == id)
            return a;
    return nullptr;
}

// Dano do canal COMUM (secao 11): rolled = base * (1 + (v*2*r - v)), r em [0,1). Extraido
// pra ser COMPARTILHADO entre resolve_use_card (r = rng_->next_double() real) e
// estimate_card_damage (r = 0.0/1.0, faixa PURA sem RNG algum) - garante que o preview seja
// BIT-IDENTICO ao pior/melhor caso real, nunca uma formula paralela que possa divergir.
[[nodiscard]] float comum_channel_damage(float base_damage, float v, double r) {
    return base_damage * static_cast<float>(1.0 + (static_cast<double>(v) * 2.0 * r -
                                                    static_cast<double>(v)));
}

// Dano do canal CRIT (secao 11): round(base * (1+v) * 1.5). Compartilhado pelo mesmo motivo
// (resolve_use_card e estimate_card_damage chamam a MESMA formula).
[[nodiscard]] float crit_channel_damage(float base_damage, float v) {
    return base_damage * (1.0f + v) * 1.5f;
}

// Perda de HP apos absorcao de Shield (secao 9), SEM mutar nada: espelha CombatActor::
// absorb_with_shield (perda = raw - min(raw, magnitude_do_Shield_ativo), piso 0 quando ha
// absorcao). Compartilhado por preview_basic_attack_damage e estimate_card_damage (ambos
// previews PUROS pra UI, secao 11 e modo-mira).
[[nodiscard]] int shield_absorbed_loss(int raw_damage, const CombatActor& target) {
    int shield_magnitude = 0;
    for (const StatusEffect& s : target.status_effects()) {
        if (s.id == StatusId::Shield) {
            shield_magnitude = s.magnitude;
            break;
        }
    }
    const int absorbed = std::min(raw_damage, shield_magnitude);
    return absorbed <= 0 ? raw_damage : raw_damage - absorbed;
}

// Quantum-Lock (Planck, ADR-016 manifesto item 5): detecta a passiva DamageQuantize
// equipada em `actor`, varrendo equipped_special_ids() contra `registry` (mesmo padrao de
// deteccao de passiva ja usado pelo Reflect/Newton, mas SEM passar pelo dispatcher
// techMagic::execute - a quantizacao pluga DIRETO no canal COMUM do resolvedor/preview, ver
// doc-comment de EffectKind::DamageQuantize). Fail-SOFT (nullptr), ao contrario do
// fail-fast de execute_equipped: registry nullptr ou id nao encontrado so significa "sem
// quantizacao", nunca bloqueia as OUTRAS passivas equipadas do ator.
[[nodiscard]] const EffectSpec* quantize_spec_of(
    const CombatActor& actor, const std::unordered_map<std::string, Card>* registry) {
    if (registry == nullptr) return nullptr;
    for (const std::string& id : actor.equipped_special_ids()) {
        const auto it = registry->find(id);
        if (it == registry->end()) continue;
        for (const EffectSpec& spec : it->second.effects)
            if (spec.kind == EffectKind::DamageQuantize) return &spec;
    }
    return nullptr;
}

// Quantum-Lock: escolhe o degrau (r do comum_channel_damage) a partir de um roll 0..99.
// spec.percent = chance% de CADA extremo (piso E teto, simetrico); spec.magnitude =
// chance% do degrau central. So usado por resolve_use_card (roll REAL); o preview
// (estimate_card_damage) NAO sorteia - mostra os 3 degraus fixos direto.
[[nodiscard]] double quantize_step_r(const EffectSpec& spec, int roll) {
    if (roll < spec.percent) return 0.0;
    if (roll < spec.percent + spec.magnitude) return 0.5;
    return 1.0;
}

// Quantum-Lock: sufixo de log do hit quantizado (regra do lider: todo efeito loga), mesmo
// padrao dos sufixos existentes (" [CRITICO]", " FALHA DE COMPILACAO").
[[nodiscard]] std::string quantize_log_suffix(double r, const EffectSpec& spec) {
    if (r == 0.0)
        return " [Quantum-Lock: degrau baixo, " + std::to_string(spec.percent) + "%]";
    if (r == 1.0)
        return " [Quantum-Lock: degrau alto, " + std::to_string(spec.percent) + "%]";
    return " [Quantum-Lock: degrau medio, " + std::to_string(spec.magnitude) + "%]";
}

// Scan aprimorado (ADR-016 step 8, John Dee/Black-Mirror, decisao do lider D1-ii):
// detecta a carta RevealIntent EQUIPADA em `actor`, mesmo padrao de scan de
// quantize_spec_of acima (varre equipped_special_ids() contra `registry` procurando o
// EffectKind, NAO hardcoda id/nome de carta). Fail-SOFT (false): registry nullptr ou id
// nao encontrado so significa "Scan comum, sem bonus" - nunca bloqueia o Scan base.
[[nodiscard]] bool has_reveal_intent_equipped(
    const CombatActor& actor, const std::unordered_map<std::string, Card>* registry) {
    if (registry == nullptr) return false;
    for (const std::string& id : actor.equipped_special_ids()) {
        const auto it = registry->find(id);
        if (it == registry->end()) continue;
        for (const EffectSpec& spec : it->second.effects)
            if (spec.kind == EffectKind::RevealIntent) return true;
    }
    return false;
}

// ---- Free-Order (Hayek, CARD-ENGINE-MANIFESTO item 7, AMB-09, EffectKind::
// DiversityBonus) ----------------------------------------------------------------------
//
// Mesmo padrao "marker fora do dispatcher" de quantize_spec_of/quantize_step_r acima: a
// carta e um MARCADOR (techmagic.cpp::handle_diversity_bonus e no-op) e o bonus pluga
// DIRETO na cadeia divisiva/limiar de falha do resolvedor + preview, via os helpers abaixo.

// M2 (AMB-09): duas entradas do ledger tem a MESMA assinatura de acao quando o `type` bate
// E, se `type == UseCard`, a `family` da carta jogada tambem bate (2 cartas da MESMA
// familia = 1 assinatura; familias diferentes = assinaturas distintas). Nos demais tipos
// (Attack/Defend/Scan/etc.) so o `type` importa.
[[nodiscard]] bool same_action_signature(const techMagic::RoundActionEntry& a,
                                         const techMagic::RoundActionEntry& b) {
    if (a.type != b.type) return false;
    if (a.type == CombatActionType::UseCard) return a.family == b.family;
    return true;
}

// Conta as assinaturas de acao DISTINTAS ja registradas no ledger `actions` pelo lado
// `player_side` (todo o vetor recebido - o CALLER decide o corte "antes da acao corrente",
// ver CombatStateMachine::resolve_action: a acao corrente so entra no ledger DEPOIS do
// bonus dela ja ter sido lido, entao esta contagem NUNCA inclui a propria acao em
// resolucao - anti auto-inflacao). O(n^2) deliberado: n = acoes/rodada de 1 lado, tipicamente
// <= 8 (party 1-4 + eventuais AoE nao mudam n, 1 entrada/acao).
[[nodiscard]] int distinct_action_count(const std::vector<techMagic::RoundActionEntry>& actions,
                                        bool player_side) {
    std::vector<techMagic::RoundActionEntry> seen;
    for (const techMagic::RoundActionEntry& e : actions) {
        if (e.actor == nullptr || e.actor->is_player_side() != player_side) continue;
        const bool dup = std::any_of(seen.begin(), seen.end(), [&](const auto& s) {
            return same_action_signature(e, s);
        });
        if (!dup) seen.push_back(e);
    }
    return static_cast<int>(seen.size());
}

// true se QUALQUER VIVO do lado `player_side` porta a passiva DiversityBonus equipada
// (independente do limiar de distintas ter sido alcancado) - usado so pra decidir SE loga a
// linha (bonus OU no-op "sem diversidade ainda"; regra todo-efeito-loga). Fail-soft
// (registry nulo ou id nao encontrado => false), mesmo padrao de quantize_spec_of/
// has_reveal_intent_equipped.
[[nodiscard]] bool diversity_equipped_on_side(
    bool player_side, const std::vector<CombatActor*>& roster,
    const std::unordered_map<std::string, Card>* registry) {
    if (registry == nullptr) return false;
    for (const CombatActor* a : roster) {
        if (a == nullptr || !a->is_alive() || a->is_player_side() != player_side) continue;
        for (const std::string& id : a->equipped_special_ids()) {
            const auto it = registry->find(id);
            if (it == registry->end()) continue;
            for (const EffectSpec& spec : it->second.effects)
                if (spec.kind == EffectKind::DiversityBonus) return true;
        }
    }
    return false;
}

// Acha o EffectSpec de MAIOR limiar (spec.magnitude) que `distinct_count` ainda alcanca,
// entre TODAS as especiais DiversityBonus equipadas por QUALQUER VIVO do lado
// `player_side` (o beneficio e do LADO INTEIRO enquanto o dono estiver vivo/equipado, nao
// so do dono - decisao do lider). Cada EffectSpec da carta hayek e um DEGRAU: `magnitude` =
// limiar de assinaturas distintas pra este degrau valer, `percent` = bonus% de dano,
// `duration` = reducao em pontos-percentuais (pp) do limiar de falha (reuse deliberado do
// campo - mesmo padrao data-driven de outros EffectKind reinterpretando magnitude/percent/
// duration por-kind, ex. ChainDamage usa magnitude=saltos/percent=retencao). "4+" (o degrau
// mais alto) e automatico: nenhum spec tem magnitude > 4, entao distinct_count 4, 5, 6...
// sempre casam com o MESMO degrau mais alto (cap). Fail-soft (registry nulo ou nenhuma
// equipada/limiar nao alcancado => nullptr), mesmo padrao de quantize_spec_of.
[[nodiscard]] const EffectSpec* diversity_spec_of(
    bool player_side, const std::vector<CombatActor*>& roster,
    const std::unordered_map<std::string, Card>* registry, int distinct_count) {
    if (registry == nullptr) return nullptr;
    const EffectSpec* best = nullptr;
    for (const CombatActor* a : roster) {
        if (a == nullptr || !a->is_alive() || a->is_player_side() != player_side) continue;
        for (const std::string& id : a->equipped_special_ids()) {
            const auto it = registry->find(id);
            if (it == registry->end()) continue;
            for (const EffectSpec& spec : it->second.effects) {
                if (spec.kind != EffectKind::DiversityBonus) continue;
                if (spec.magnitude > distinct_count) continue;  // limiar nao alcancado.
                if (best == nullptr || spec.magnitude > best->magnitude) best = &spec;
            }
        }
    }
    return best;
}

// Resultado agregado do Free-Order pra UMA acao (calculado ANTES dela entrar no ledger -
// anti auto-inflacao). `equipped` decide SE loga algo (bonus ou no-op); `active` decide SE
// o bonus realmente aplica (equipada mas distinct_count<2 => equipped=true, active=false).
struct HayekBonus {
    bool equipped = false;
    bool active = false;
    int distinct_count = 0;
    int damage_percent = 0;       // spec->percent quando active (bonus% de dano).
    int fumble_reduction_pp = 0;  // spec->duration quando active (desconto pp no limiar).

    [[nodiscard]] float mult_damage() const noexcept {
        return active ? 1.0f + static_cast<float>(damage_percent) / 100.0f : 1.0f;
    }
};

// Monta o HayekBonus pro lado `player_side`, lendo `round_actions` (o ledger da rodada,
// ANTES da acao corrente - contrato do CALLER) + `roster`/`registry` (deteccao fail-soft).
[[nodiscard]] HayekBonus hayek_bonus_for(
    bool player_side, const std::vector<CombatActor*>& roster,
    const std::unordered_map<std::string, Card>* registry,
    const std::vector<techMagic::RoundActionEntry>& round_actions) {
    HayekBonus out;
    out.equipped = diversity_equipped_on_side(player_side, roster, registry);
    if (!out.equipped) return out;
    out.distinct_count = distinct_action_count(round_actions, player_side);
    const EffectSpec* spec = diversity_spec_of(player_side, roster, registry, out.distinct_count);
    if (spec == nullptr) return out;  // equipada, mas ainda sem diversidade suficiente.
    out.active = true;
    out.damage_percent = spec->percent;
    out.fumble_reduction_pp = spec->duration;
    return out;
}

// Sufixo de log do Free-Order (regra do lider: todo efeito loga - hit beneficiado E no-op),
// mesmo padrao dos sufixos existentes (quantize_log_suffix etc.). "" quando ninguem do lado
// porta a passiva (nada a logar).
[[nodiscard]] std::string hayek_log_suffix(const HayekBonus& hayek) {
    if (!hayek.equipped) return "";
    if (hayek.active)
        return " [Free-Order: " + std::to_string(hayek.distinct_count) + " abordagens, +" +
               std::to_string(hayek.damage_percent) + "%]";
    return " [Free-Order: sem diversidade ainda]";
}

}  // namespace

CombatStateMachine::CombatStateMachine(
    std::vector<CombatActor*> actors,
    CombatActionProvider action_provider,
    const std::unordered_map<std::string, Card>* card_registry,
    const std::unordered_map<std::string, IEnemyBrain*>* brain_registry,
    IRandomSource* rng)
    : action_provider_(std::move(action_provider)),
      card_registry_(card_registry),
      brain_registry_(brain_registry),
      rng_(rng != nullptr ? rng : &neutral_random()),
      // A fila valida vazio com std::invalid_argument (espelha o ArgumentException do C#
      // "Combate precisa de ao menos 1 ator."). Construida ja ordenada por SPD.
      queue_(std::move(actors)) {
    if (!action_provider_)
        throw std::invalid_argument("CombatStateMachine: action_provider nao pode ser nulo.");

    phase_ = CombatPhase::SetupPhase;
    outcome_ = CombatOutcome::Ongoing;

    // GUS-CENTRIC (BUG-4): cacheia o ator com is_universal_compiler()==true (o Gus), se
    // algum foi passado - AQUI, em SetupPhase, ANTES de qualquer prune_dead() (a fila
    // ainda tem TODOS os atores). check_end() consulta gus_actor_ diretamente (nao a
    // fila) pra sobreviver ao prune. nullptr se nenhum ator tiver a flag (comportamento
    // legado de wipe-total preservado).
    for (CombatActor* a : queue_.order()) {
        if (a->is_player_side() && a->is_universal_compiler()) {
            gus_actor_ = a;
            break;
        }
    }

    // Regroup por lado da RODADA 0 (§4.1): a fila ja veio ordenada por SPD; agrupa "quem abre
    // inteiro, depois o outro" (stable_partition, Gambito-safe). Fronteira da 1a rodada => o
    // cursor fica em 0 (primeiro ator do lado que abre). As rodadas 1+ reagrupam no wrap de
    // advance_to_next_actor.
    regroup_round_by_side();

    // SetupPhase -> TurnStart: a fila ja esta ordenada e agrupada; o primeiro ator entra.
    phase_ = CombatPhase::TurnStart;
}

// ---------------------------------------------------------------------------
// Ambiente de combate (secao 18)
// ---------------------------------------------------------------------------

void CombatStateMachine::set_environment(const std::vector<EnvironmentId>& ids) {
    active_environments_.clear();
    period_clock_.reset();

    for (const EnvironmentId id : ids) {
        if (id == EnvironmentId::None) continue;
        const EnvironmentModifier& env = EnvironmentCatalog::get(id);
        active_environments_.push_back(env);
        environment_changes_.push_back(id);

        if (env.layer == EnvironmentLayer::Periodo)
            period_clock_.emplace(id);
    }
}

float CombatStateMachine::mult_ambiente_for(CardFamily family) const {
    return EnvironmentCatalog::mult_ambiente(family, effective_environments());
}

std::vector<EnvironmentModifier> CombatStateMachine::effective_environments() const {
    std::vector<EnvironmentModifier> out;
    for (const auto& env : active_environments_)
        if (env.layer != EnvironmentLayer::Periodo)
            out.push_back(env);
    if (period_clock_.has_value())
        out.push_back(period_clock_->current());
    return out;
}

void CombatStateMachine::advance_period_clock() {
    if (!period_clock_.has_value()) return;
    if (period_clock_->advance())
        environment_changes_.push_back(period_clock_->current_phase());
}

// ---------------------------------------------------------------------------
// Janela de Comando da Party (comando livre sobre o CTB, modelo 1B, §4.1)
//
// Extensao ADITIVA: nenhuma destas funcoes consome RNG nem muda a FORMA da FSM. A selecao
// de ator e input do jogador/host; a resolucao de acao (§11) fica intacta. Sem chamada de
// select_party_actor, begin_turn opera no queue_.current() de sempre (motor pre-1B).
// ---------------------------------------------------------------------------

CombatSide CombatStateMachine::round_opening_side() const {
    // A SPD do lado (maior SPD entre os vivos) decide quem abre a rodada (§4.1). Empate
    // favorece a party: o inimigo so pega a party primeiro se for ESTRITAMENTE mais rapido.
    int party_max = -1;
    int enemy_max = -1;
    for (const CombatActor* a : queue_.order()) {
        if (!a->is_alive()) continue;
        if (a->is_player_side())
            party_max = std::max(party_max, a->spd());
        else
            enemy_max = std::max(enemy_max, a->spd());
    }
    return party_max >= enemy_max ? CombatSide::Party : CombatSide::Enemy;
}

std::vector<CombatActor*> CombatStateMachine::pending_party_actors() const {
    // Elegiveis = membros da party (player-side) vivos que ainda NAO agiram nesta rodada.
    // "Ainda nao agiu" = slot >= cursor (o cursor caminha a rodada; slots < cursor ja
    // agiram). Derivado da fila, sem estado extra. Ordenado por SPD desc (front =
    // pre-selecionado); stable_sort espelha o desempate por ordem de fila da §4.
    const std::vector<CombatActor*>& order = queue_.order();
    const int cursor = queue_.cursor();
    std::vector<CombatActor*> out;
    for (int i = cursor; i < static_cast<int>(order.size()); ++i) {
        CombatActor* a = order[static_cast<std::size_t>(i)];
        if (a->is_player_side() && a->is_alive())
            out.push_back(a);
    }
    std::stable_sort(out.begin(), out.end(), [](const CombatActor* x, const CombatActor* y) {
        return x->spd() > y->spd();
    });
    return out;
}

CombatActor* CombatStateMachine::preselected_party_actor() const {
    const std::vector<CombatActor*> pending = pending_party_actors();
    return pending.empty() ? nullptr : pending.front();
}

bool CombatStateMachine::is_pending_party_actor(const CombatActor* actor) const {
    if (actor == nullptr) return false;
    for (const CombatActor* a : pending_party_actors())
        if (a == actor) return true;
    return false;
}

void CombatStateMachine::select_party_actor(CombatActor* actor) {
    if (!is_pending_party_actor(actor))
        throw std::invalid_argument(
            "select_party_actor: '" + (actor != nullptr ? actor->id() : std::string{"<null>"}) +
            "' nao e um membro elegivel da party nesta rodada (vivo, player-side, "
            "ainda-nao-agiu). Consulte pending_party_actors().");
    selected_next_party_ = actor;
}

void CombatStateMachine::regroup_round_by_side() {
    // Quem ABRE (por SPD corrente entre os vivos; empate favorece a party). A fila nao conhece
    // "lado": passamos um predicado que marca o lado que abre. party_opens verdadeiro => os
    // atores player-side vao pra frente; falso => os enemy-side vao pra frente.
    const bool party_opens = round_opening_side() == CombatSide::Party;
    queue_.regroup_stable([party_opens](const CombatActor* a) {
        return a->is_player_side() == party_opens;
    });
}

// ---------------------------------------------------------------------------
// Conducao da FSM
// ---------------------------------------------------------------------------

bool CombatStateMachine::begin_turn() {
    phase_ = CombatPhase::TurnStart;

    // Janela de Comando da Party (1B, §4.1): se o host escolheu um membro elegivel via
    // select_party_actor, traz ele para o slot do cursor ANTES de operar (comando livre).
    // One-shot: consome e zera. Sem selecao (selected_next_party_ == nullptr), este bloco
    // e no-op e begin_turn opera no queue_.current() de sempre => default byte-identico ao
    // motor pre-1B (os testes de transicao existentes assumem isto). Re-valida a
    // elegibilidade no consumo: se o estado mudou (ator morreu, ja agiu), degrada pro
    // default em vez de forcar. NAO consome RNG.
    if (selected_next_party_ != nullptr) {
        CombatActor* chosen = selected_next_party_;
        selected_next_party_ = nullptr;
        if (is_pending_party_actor(chosen))
            queue_.bring_to_current(chosen);
    }

    // Knockback (K-B, decisao do lider 2026-07-15, A2/COMBATE-FILA-CURSOR-FIX): ANTES de
    // fixar `actor` = current(), resolve toda a cadeia de adiamentos pendentes no proprio
    // slot do cursor. Empurra o TURNO (nao um salto cru na fila): o vizinho que estava logo
    // apos passa a agir primeiro, e o ator com Knockback age em seguida - todos agem
    // EXATAMENTE 1x na rodada (substitui o bug antigo de reorder_actor(+1), que pulava o
    // vizinho ao cruzar o cursor, achado QA). One-shot: o status e CONSUMIDO (remove_status)
    // no instante em que dispara, sem depender do decremento de Duration de
    // apply_status_tick. Loop bounded por count() (defensivo; delay_current ja tem teto
    // proprio no fim da fila, nunca deveria estourar).
    {
        int guard = 0;
        while (queue_.current()->index_of_status(StatusId::Knockback) >= 0) {
            if (++guard > queue_.count())
                break;
            CombatActor* pushed = queue_.current();
            pushed->remove_status(StatusId::Knockback);
            if (queue_.delay_current(1)) {
                log_.push_back(CombatLogEntry{
                    pushed->id(), CombatActionType::StatusTick, queue_.current()->id(), 0,
                    pushed->id() + " e empurrado (Knockback): recua na fila, " +
                        queue_.current()->id() + " age primeiro."});
            } else {
                // Ultimo slot da fila: nao ha vizinho pra adiantar. Consome o status e age
                // agora mesmo (mesma regra de "sem acao futura" do Gambito/Einstein).
                log_.push_back(CombatLogEntry{
                    pushed->id(), CombatActionType::StatusTick, std::nullopt, 0,
                    pushed->id() + " e empurrado (Knockback), mas ja esta no ultimo slot da "
                                  "fila: age agora mesmo."});
            }
        }
    }

    CombatActor& actor = *queue_.current();

    actor.refresh_resources_for_turn(queue_.round_index());

    const bool stunned = apply_status_tick(actor);

    // Always (executor techMagic, ADR-016 secao 20 item 4/MVP): reforca (Refresh) o
    // ApplyStatus das especiais EQUIPADAS (Passiva/Hibrida) no dono, todo TurnStart,
    // enquanto a carta seguir equipada. So ApplyStatus neste step; OnAllyTurnEnd/
    // OnRoundEnd ficam DECLARADOS sem ponto de disparo (step 3+). No-op se `actor` nao
    // tiver especiais equipadas.
    {
        techMagic::TechMagicContext always_ctx{&actor, nullptr, /*damage=*/0, &log_};
        techMagic::execute_equipped(TriggerHook::Always, actor, card_registry_, always_ctx);
    }

    drain_status_changes();

    phase_ = CombatPhase::ActionSelect;
    return stunned;
}

void CombatStateMachine::drain_status_changes() {
    for (CombatActor* actor : queue_.order()) {
        auto changes = actor->drain_status_changes();
        status_changes_.insert(status_changes_.end(), changes.begin(), changes.end());
    }
}

void CombatStateMachine::run_active_turn_to_end() {
    if (phase_ != CombatPhase::ActionSelect)
        throw std::logic_error(
            "run_active_turn_to_end exige fase ActionSelect; chame begin_turn antes.");

    CombatActor& actor = *queue_.current();
    int guard = 0;

    while (actor.ap() > 0) {
        if (++guard > kMaxActionsPerTurn)
            throw std::logic_error(
                "Loop de ActionSelect excedeu o teto de acoes para '" + actor.id() +
                "' (provider nao passou nem gastou AP).");

        phase_ = CombatPhase::ActionSelect;
        const CombatState state(queue_, &actor, queue_.round_index(), card_registry_);
        const CombatAction action = action_provider_(actor, state);

        if (action.type == CombatActionType::Pass)
            break;  // 0 AP, encerra o turno

        if (action.ap_cost > actor.ap())
            break;  // acao inviavel por falta de AP: trata como pass (nao trava o turno)

        phase_ = CombatPhase::ActionResolve;
        resolve_action(actor, action, state);
    }

    phase_ = CombatPhase::TurnEnd;
    if (actor.expire_elapsed_statuses())
        queue_.recompute_by_speed();
    drain_status_changes();

    // OnAllyTurnEnd (ADR-016 secao 20 item 5, Ada/Re-Run): despacha nos OUTROS aliados
    // vivos do mesmo lado de `actor`, ao fechar este turno (2o dos 2 caminhos de
    // fim-de-turno; ver expire_on_stunned_turn_end abaixo).
    process_ally_turn_end_hooks(actor);
}

bool CombatStateMachine::check_end() {
    phase_ = CombatPhase::CheckEnd;

    prune_dead();

    bool any_player_alive = false;
    bool any_enemy_alive = false;
    for (const CombatActor* a : queue_.order()) {
        if (a->is_alive()) {
            if (a->is_player_side())
                any_player_alive = true;
            else
                any_enemy_alive = true;
        }
    }

    if (outcome_ == CombatOutcome::Fled)
        return true;  // fuga ja registrada na resolucao

    // GUS-CENTRIC (canon, decisao do lider, M7-COSTURA BUG-4): "o Rei caiu = fim". O Gus
    // (is_universal_compiler()==true) a HP 0 encerra o combate em Defeat IMEDIATO, MESMO
    // com companions ainda vivos - a party recua com ele. Companions caidos sozinhos NAO
    // encerram o combate (ficam incapacitados, canon Pillar 4); so o Gus domina esta
    // condicao - por isso o check vem ANTES (e independente) do wipe-total abaixo.
    // gus_actor_ (cacheado no construtor, sobrevive ao prune_dead() acima) e nullptr se
    // nenhum ator tiver a flag - nesse caso o comportamento legado de wipe-total segue
    // 100% intacto (nenhuma regressao pras FSMs headless/testes que nunca a setam).
    if (gus_actor_ != nullptr && !gus_actor_->is_alive()) {
        outcome_ = CombatOutcome::Defeat;
        return true;
    }

    if (!any_player_alive) {
        outcome_ = CombatOutcome::Defeat;
        return true;
    }
    if (!any_enemy_alive) {
        outcome_ = CombatOutcome::Victory;
        return true;
    }
    return false;
}

void CombatStateMachine::advance_to_next_actor() {
    prune_dead();  // defensivo

    if (queue_.count() == 0) return;

    const int round_before = queue_.round_index();
    queue_.advance();

    int safety = 0;
    while (!queue_.current()->is_alive() && safety++ < queue_.count())
        queue_.advance();

    if (queue_.round_index() > round_before) {
        // Fronteira da rodada (o wrap acabou de dar a volta na fila): despacha OnRoundEnd
        // (ADR-016 secao 20 item 4, HypotenuseCombo) sobre o ledger da rodada que ACABOU de
        // fechar, ANTES do regroup - a iteracao usa queue_.order() na ordem que fechou a
        // rodada (o regroup so reordena a PROXIMA). Depois reagrupa por lado a nova rodada
        // (§4.1) e avanca o relogio de periodo (§18.3). O regroup so acontece AQUI, na
        // fronteira (nunca no meio da rodada): a esta altura prune_dead ja rodou e o cursor
        // esta em 0, entao ele so pode reordenar atores pendentes desta nova rodada. Reagrupar
        // AQUI (e nao a cada advance) preserva o comando livre da Fase 1 dentro da rodada.
        process_round_end_hooks();
        regroup_round_by_side();
        process_scrying_hooks();
        advance_period_clock();
    }
}

void CombatStateMachine::process_round_end_hooks() {
    // Dedup 1x/alvo/rodada (ADR-016 secao 20 item 4): compartilhado entre TODOS os atores
    // iterados nesta chamada (= 1 rodada), entao >1 dono da passiva Hipotenusa fechando
    // combo no MESMO alvo nao dobra o bonus.
    std::unordered_set<CombatActor*> bonused_targets;

    for (CombatActor* actor : queue_.order()) {
        if (!actor->is_alive()) continue;  // cadaver: sem passiva pra rodar.
        techMagic::TechMagicContext ctx{
            /*caster=*/actor,       /*counterpart=*/nullptr, /*damage=*/0,
            /*log=*/&log_,          /*round_hits=*/&round_hits_,
            /*bonused_targets=*/&bonused_targets};
        techMagic::execute_equipped(TriggerHook::OnRoundEnd, *actor, card_registry_, ctx);
    }

    // Hits nao atravessam fronteira de rodada (limpa SEMPRE, mesmo sem nenhuma especial).
    round_hits_.clear();

    // RepeatLastAction (ADR-016 secao 20 item 5, Q4): a janela "ultima acao de dano de
    // QUALQUER aliado" tambem nao atravessa fronteira de rodada - zera junto do ledger.
    last_action_ = techMagic::LastActionRecord{};

    // Free-Order (Hayek, CARD-ENGINE-MANIFESTO item 7, AMB-09): o ledger de ACOES da
    // rodada tambem nao atravessa fronteira de rodada - zera na MESMA fronteira que
    // round_hits_/last_action_ acima.
    round_actions_.clear();
}

void CombatStateMachine::process_ally_turn_end_hooks(CombatActor& ended) {
    for (CombatActor* ally : queue_.order()) {
        if (ally == &ended) continue;         // a passiva reage a OUTRO aliado, nunca a si.
        if (!ally->is_alive()) continue;      // cadaver: sem passiva pra rodar.
        if (ally->is_player_side() != ended.is_player_side()) continue;  // so o MESMO lado.

        techMagic::TechMagicContext ctx{
            /*caster=*/ally,   /*counterpart=*/nullptr, /*damage=*/0,
            /*log=*/&log_,     /*round_hits=*/nullptr,  /*bonused_targets=*/nullptr,
            /*last_action=*/&last_action_, /*rng=*/rng_};
        techMagic::execute_equipped(TriggerHook::OnAllyTurnEnd, *ally, card_registry_, ctx);
    }
}

void CombatStateMachine::process_scrying_hooks() {
    for (CombatActor* actor : queue_.order()) {
        if (!actor->is_alive()) continue;             // cadaver: sem re-dump.
        if (actor->index_of_status(StatusId::Scrying) < 0) continue;  // nao porta o buff.

        techMagic::TechMagicContext ctx{
            /*caster=*/actor, /*counterpart=*/nullptr, /*damage=*/0, /*log=*/&log_};
        ctx.combatants = &queue_.order();
        ctx.queue = &queue_;
        ctx.brain_registry = brain_registry_;
        techMagic::dump_reveal_intent(ctx);
    }
}

CombatResult CombatStateMachine::run_until_end() {
    constexpr int kMaxTurns = 10000;
    int turns = 0;

    while (outcome_ == CombatOutcome::Ongoing) {
        if (turns++ > kMaxTurns)
            throw std::logic_error("Combate excedeu o teto de turnos (possivel stalemate).");

        const bool stunned = begin_turn();

        // BUG-1: ator morto pelo tick (Poison/Corrode letal no TurnStart) nao age. Pula
        // direto pro TurnEnd/CheckEnd (simetrico ao ramo stunned).
        if (stunned || !queue_.current()->is_alive())
            expire_on_stunned_turn_end(*queue_.current());
        else
            run_active_turn_to_end();

        if (check_end())
            break;

        advance_to_next_actor();

        if (queue_.count() == 0) {
            outcome_ = CombatOutcome::Defeat;
            break;
        }
    }

    phase_ = CombatPhase::CombatEnd;
    CombatResult result;
    result.outcome = outcome_;
    result.log = log_;
    result.rounds_elapsed = queue_.round_index();
    return result;
}

void CombatStateMachine::expire_on_stunned_turn_end(CombatActor& actor) {
    phase_ = CombatPhase::TurnEnd;
    if (actor.expire_elapsed_statuses())
        queue_.recompute_by_speed();
    drain_status_changes();

    // OnAllyTurnEnd (ADR-016 secao 20 item 5, Ada/Re-Run): gemeo do ramo ativo acima -
    // um turno perdido por Stun/morte-no-tick TAMBEM fecha o turno de `actor` (1o dos 2
    // caminhos de fim-de-turno; ver run_active_turn_to_end).
    process_ally_turn_end_hooks(actor);
}

// ---------------------------------------------------------------------------
// Resolucao de acoes
// ---------------------------------------------------------------------------

void CombatStateMachine::resolve_action(CombatActor& actor, const CombatAction& action,
                                        const CombatState& state) {
    switch (action.type) {
        case CombatActionType::Attack:
            actor.spend_ap(action.ap_cost);
            resolve_basic_attack(actor, action);
            break;
        case CombatActionType::Defend:
            actor.spend_ap(action.ap_cost);
            resolve_defend(actor);
            break;
        case CombatActionType::UseCard:
            actor.spend_ap(action.ap_cost);
            resolve_use_card(actor, action, state);
            break;
        case CombatActionType::Scan:
            actor.spend_ap(action.ap_cost);
            resolve_scan(actor, action);
            break;
        case CombatActionType::ScanEnvironment:
            actor.spend_ap(action.ap_cost);
            resolve_scan_environment(actor);
            break;
        case CombatActionType::Flee:
            actor.spend_ap(action.ap_cost);
            resolve_flee(actor);
            break;
        case CombatActionType::GambitPredict:
            actor.spend_ap(action.ap_cost);
            resolve_gambit_predict(actor, action);
            break;
        case CombatActionType::GambitReorder:
            actor.spend_ap(action.ap_cost);
            resolve_gambit_reorder(actor, action);
            break;
        case CombatActionType::Pass:
            break;  // tratado no loop; nao chega aqui
        default:
            throw std::out_of_range("Tipo de acao de combate desconhecido.");
    }

    // Free-Order (Hayek, CARD-ENGINE-MANIFESTO item 7, AMB-09): registra a assinatura da
    // acao corrente no ledger da rodada (round_actions_) DEPOIS de resolve_basic_attack/
    // resolve_use_card acima ja terem rodado e lido o ledger no estado PRE-acao (via
    // hayek_bonus_for) - sem este ordenamento a acao se auto-inflacionaria (contaria a si
    // propria como uma das distintas que ELA MESMA precisa pra escalar). Granularidade de
    // ACAO (nao de hit): 1 entrada por chamada de resolve_action, independente de quantos
    // alvos/hits a acao produza. `family` so e resolvido quando UseCard (M2) - a carta ja
    // foi validada por resolve_use_card acima (ou a excecao ja propagou e este ponto nunca
    // e alcancado), entao o find aqui e so releitura, nunca falha em caminho feliz.
    CardFamily action_family = CardFamily::Eletrico;
    if (action.type == CombatActionType::UseCard && action.card_id.has_value()) {
        const auto& registry = state.card_registry();
        const auto it = registry.find(*action.card_id);
        if (it != registry.end()) action_family = it->second.family;
    }
    round_actions_.push_back(techMagic::RoundActionEntry{&actor, action.type, action_family});
}

int CombatStateMachine::preview_basic_attack_damage(
    const CombatActor& attacker, const CombatActor& target) const noexcept {
    // Dano bruto IDENTICO a resolve_basic_attack (logo abaixo). Mantido em sincronia.
    const int raw = std::max(combat_constants::kMinDamage, attacker.atk() - target.def());
    // Free-Order (Hayek, item 7): MESMO gemeo preview<->real que o Quantum-Lock/Planck -
    // le o ledger CORRENTE (round_actions_), sem mutar nada (contrato PURO deste metodo).
    const HayekBonus hayek =
        hayek_bonus_for(attacker.is_player_side(), queue_.order(), card_registry_, round_actions_);
    const int boosted =
        static_cast<int>(std::lround(static_cast<float>(raw) * hayek.mult_damage()));
    // Absorcao de Shield espelhada de CombatActor::absorb_with_shield, SEM mutar (helper
    // compartilhado com estimate_card_damage, ver namespace anonimo acima).
    return shield_absorbed_loss(boosted, target);
}

CardDamageEstimate CombatStateMachine::estimate_card_damage(
    const CombatActor& attacker, const CombatActor& target, const Card& card,
    float mult_combo) const noexcept {
    CardDamageEstimate est;

    // Fraqueza (secao 6.1): alvo universal (compilador universal) = mult neutro, MESMA
    // regra de resolve_use_card.
    est.mult_fraqueza = target.is_universal_compiler()
                             ? 1.0f
                             : WeaknessWheel::multiplier(card.family, target.family());

    // Fura-defesa (Godel/Null-Proof, ADR-016 Balde B PR3): mantido em sincronia com
    // resolve_use_card, MESMA ordem e MESMAS duas vias. Diferenca deliberada: este metodo e
    // PURO (contrato do preview) - le o NullProof do atacante mas NUNCA chama remove_status
    // (sem isso a UI mostraria "IMUNE" enquanto o hit real fura, dessincronizando preview vs
    // resultado - achado de auditoria-dominó).
    if (card.ignores_weakness_wheel) {
        est.mult_fraqueza = 1.0f;
    } else if (est.mult_fraqueza < 1.0f) {
        const auto& attacker_effects = attacker.status_effects();
        const bool has_null_proof = std::any_of(
            attacker_effects.begin(), attacker_effects.end(),
            [](const StatusEffect& s) { return s.id == StatusId::NullProof; });
        if (has_null_proof) est.mult_fraqueza = 1.0f;
    }

    // Curto-circuito de imunidade (secao 11): dano 0 em tudo, ANTES de qualquer canal. O
    // motor tambem nao sorteia roll aqui (early-return em resolve_use_card) - o preview
    // reflete a MESMA regra sem nunca ter consumido RNG pra chegar ate este ponto.
    if (est.mult_fraqueza == 0.0f) {
        est.immune = true;
        return est;
    }

    constexpr float mult_mod = 1.0f;  // Stream distribui no jogo posterior; slice = 1.0

    // Expose (secao 9/11): LE o status do ALVO (nao muta, nao consome).
    float mult_expose = 1.0f;
    for (const StatusEffect& s : target.status_effects()) {
        if (s.id == StatusId::Expose) {
            mult_expose = 1.0f + static_cast<float>(s.magnitude) / 100.0f;
            break;
        }
    }

    // Disrupt (secao 9/11): LE o status do ATACANTE. resolve_use_card so CONSOME (remove) o
    // Disrupt na resolucao real de fato jogada; o preview apenas LE o multiplicador vigente,
    // sem remove_status - nao muta nada (contrato PURO deste metodo).
    float mult_disrupt = 1.0f;
    for (const StatusEffect& s : attacker.status_effects()) {
        if (s.id == StatusId::Disrupt) {
            mult_disrupt = std::max(0.0f, 1.0f - static_cast<float>(s.magnitude) / 100.0f);
            break;
        }
    }

    const float mult_ambiente = mult_ambiente_for(card.family);

    // Free-Order (Hayek, item 7): MESMO gemeo preview<->real que o Quantum-Lock/Planck - le
    // o ledger CORRENTE (round_actions_) do lado do ATACANTE, sem mutar nada.
    const HayekBonus hayek =
        hayek_bonus_for(attacker.is_player_side(), queue_.order(), card_registry_, round_actions_);

    // 1. Cadeia divisiva IDENTICA a resolve_use_card (secao 11). mult_hayek e o ULTIMO fator
    //    (Free-Order, item 7) - substitui mult_ambiente como ultimo da cadeia.
    const float base_damage =
        static_cast<float>(card.power + attacker.atk()) *
        (100.0f / (100.0f + static_cast<float>(target.def()))) * est.mult_fraqueza *
        mult_mod * mult_combo * mult_expose * mult_disrupt * mult_ambiente * hayek.mult_damage();

    // 2. Variancia Knowledge (secao 11), IDENTICA: v = max(0.05, 0.30*exp(-kills*0.10)).
    const int kills = target.knowledge_kills();
    const float v = std::max(0.05f, 0.30f * std::exp(-static_cast<float>(kills) * 0.10f));

    // 3. Chances dos canais (secao 11): formulas deterministicas sobre kills/CritChance -
    //    ZERO consumo de RNG (o sorteio de canal so acontece em resolve_use_card). Free-
    //    Order (item 7): desconta hayek.fumble_reduction_pp do LIMIAR (piso 0) - nao mexe
    //    na contagem de RNG (o motor continua sorteando 1 unico canal, so o limiar muda).
    est.fumble_chance_pct = std::max(
        0, static_cast<int>(std::lround(5.0 * std::exp(-static_cast<double>(kills) * 0.50))) -
               hayek.fumble_reduction_pp);
    est.crit_chance_pct = std::max(5, card.crit_chance);

    // 4. Faixa do canal COMUM: r=0 (piso) e r=1 (teto), MESMO helper comum_channel_damage de
    //    resolve_use_card - so troca o r real do rng_ por 0.0/1.0 fixos (sem sortear nada).
    //    CRIT: MESMO helper crit_channel_damage.
    const int comum_floor =
        static_cast<int>(std::lround(comum_channel_damage(base_damage, v, 0.0)));
    const int comum_ceil =
        static_cast<int>(std::lround(comum_channel_damage(base_damage, v, 1.0)));
    const int crit_raw =
        static_cast<int>(std::lround(crit_channel_damage(base_damage, v)));

    // 5. Perda de HP PREVISTA por canal, ja com a absorcao de Shield do alvo (MESMO helper
    //    de preview_basic_attack_damage).
    est.min_damage = shield_absorbed_loss(comum_floor, target);
    est.max_damage = shield_absorbed_loss(comum_ceil, target);
    est.crit_damage = shield_absorbed_loss(crit_raw, target);

    // 6. Quantum-Lock (Planck, ADR-016 manifesto item 5, A2: perfect information). SO
    //    quando o ATACANTE porta a passiva equipada. NAO sorteia (r fixo 0.0/0.5/1.0,
    //    MESMO helper comum_channel_damage do piso/teto acima) - bit-identico ao que
    //    resolve_use_card produziria em cada degrau forcado (gemeo preview<->real).
    if (const EffectSpec* quantize = quantize_spec_of(attacker, card_registry_);
        quantize != nullptr) {
        est.quantized = true;
        const int comum_mid =
            static_cast<int>(std::lround(comum_channel_damage(base_damage, v, 0.5)));
        est.mid_damage = shield_absorbed_loss(comum_mid, target);
        est.step_low_pct = quantize->percent;
        est.step_mid_pct = quantize->magnitude;
        est.step_high_pct = quantize->percent;
    }

    return est;
}

void CombatStateMachine::resolve_basic_attack(CombatActor& attacker,
                                              const CombatAction& action) {
    if (!action.target_id.has_value())
        throw std::logic_error("Attack exige TargetId.");

    CombatActor* target = find_in_order(queue_, *action.target_id);
    if (target == nullptr)
        throw std::logic_error("Alvo '" + *action.target_id + "' nao esta em combate.");

    const int raw = std::max(combat_constants::kMinDamage, attacker.atk() - target->def());

    // Free-Order (Hayek, CARD-ENGINE-MANIFESTO item 7, AMB-09): le o ledger da rodada NO
    // ESTADO PRE-acao (round_actions_ so ganha a entrada desta acao DEPOIS, em
    // resolve_action - anti auto-inflacao). Ataque basico: lround(raw * mult_hayek).
    const HayekBonus hayek = hayek_bonus_for(attacker.is_player_side(), queue_.order(),
                                             card_registry_, round_actions_);
    const int damage = static_cast<int>(std::lround(static_cast<float>(raw) * hayek.mult_damage()));

    // Ataque basico nao tem carta (source_card = nullptr); ainda assim dispara
    // OnDamageDealt das equipadas do atacante e OnDamageReceived das equipadas do alvo
    // (Reflect, secao 20 item 3 - mesmo helper compartilhado de resolve_use_card).
    apply_damage_with_hooks(attacker, *target, damage, /*source_card=*/nullptr);

    log_.push_back(CombatLogEntry{
        attacker.id(), CombatActionType::Attack, target->id(), damage,
        attacker.id() + " ataca " + target->id() + " por " + std::to_string(damage) + "." +
            hayek_log_suffix(hayek)});

    // RepeatLastAction (ADR-016 secao 20 item 5): grava ao FIM, so quando o hit causou
    // dano>0 (Q2). Uma acao sem dano preserva o registro anterior (ainda valido na rodada).
    if (damage > 0)
        last_action_ = techMagic::LastActionRecord{
            &attacker, CombatActionType::Attack, /*card_id=*/std::string{}, {{target, damage}}};
}

void CombatStateMachine::apply_damage_with_hooks(CombatActor& attacker, CombatActor& target,
                                                  int damage, const Card* source_card) {
    target.take_damage(damage);
    if (damage <= 0) return;  // canal FALHA/imunidade: hooks nao disparam (secao 20 item 2/3)

    // Ledger cross-ator (ADR-016 secao 20 item 4, HypotenuseCombo/OnRoundEnd): registra o
    // hit DEPOIS do guard acima (canal FALHA/imunidade nunca entra). Ticks de DoT (Poison/
    // Corrode, apply_status_tick) chamam CombatActor::take_damage direto - FORA deste
    // helper - entao ficam de fora do ledger por construcao (coerente com o escopo: o
    // combo e sobre GOLPES, nao sobre veneno tiquetaqueando).
    round_hits_.push_back(techMagic::RoundHitEntry{&attacker, &target, damage});

    // OnDamageDealt (Leech): fontes = a carta JOGADA (se houver, ataque basico nao tem) +
    // as especiais EQUIPADAS do atacante.
    techMagic::TechMagicContext dealt_ctx{&attacker, &target, damage, &log_};
    if (source_card != nullptr)
        techMagic::execute(TriggerHook::OnDamageDealt, *source_card, dealt_ctx);
    techMagic::execute_equipped(TriggerHook::OnDamageDealt, attacker, card_registry_, dealt_ctx);

    // OnDamageReceived (Reflect): fontes = as especiais EQUIPADAS do ALVO. O dano
    // refletido aplica via CombatActor::take_damage PURO dentro do handler (techmagic.cpp)
    // - nao reentra aqui, entao nunca redispara estes hooks (guarda anti-recursao).
    techMagic::TechMagicContext received_ctx{&target, &attacker, damage, &log_};
    techMagic::execute_equipped(TriggerHook::OnDamageReceived, target, card_registry_,
                                received_ctx);

    // Reflect-por-STATUS (Newton N-3/N-4, ADR-016 Balde B PR2): alem da passiva EQUIPADA
    // (handler acima, disparado via OnDamageReceived), um StatusId::Reflect NO ALVO
    // (concedido por Newton em modo-aliado, ver master_cards.cpp) TAMBEM reflete -
    // checagem SEPARADA, DEPOIS do guard damage<=0 acima (imune/FALHA nunca reflete).
    // take_damage PURO (anti-recursao, mesmo padrao do handler de Reflect equipado): NUNCA
    // reentra neste helper, entao nunca redispara OnDamageDealt/OnDamageReceived. N-4 (as
    // duas fontes somam) sai DE GRACA: a passiva equipada e o status disparam em pontos
    // DIFERENTES deste metodo, sem logica extra de soma. NAO entra em round_hits_/
    // last_action_ (mesmo racional do Reflect equipado - eco de dano, nao um "hit" novo
    // pro combo/RepeatLastAction).
    for (const StatusEffect& s : target.status_effects()) {
        if (s.id != StatusId::Reflect) continue;
        const int reflected = static_cast<int>(
            std::lround(static_cast<double>(damage) * static_cast<double>(s.magnitude) / 100.0));
        if (reflected <= 0) break;
        attacker.take_damage(reflected);
        log_.push_back(CombatLogEntry{
            target.id(), CombatActionType::UseCard, attacker.id(), reflected,
            target.id() + " reflete " + std::to_string(reflected) + " de volta em " +
                attacker.id() + " (status Reflect)."});
        break;  // 1 entrada por StatusId (insert_or_stack_status nao duplica).
    }
}

void CombatStateMachine::apply_offensive_status(CombatActor& actor, CombatActor& target,
                                                 const StatusEffect& status) {
    const StatusApplyResult result = target.try_add_status(status);
    if (result == StatusApplyResult::Applied) {
        log_.push_back(CombatLogEntry{
            actor.id(), CombatActionType::UseCard, target.id(), status.magnitude,
            actor.id() + " aplica status em " + target.id() + " (mag " +
                std::to_string(status.magnitude) + ", dur " + std::to_string(status.duration) +
                ")."});
    } else {
        log_.push_back(CombatLogEntry{
            actor.id(), CombatActionType::UseCard, target.id(), 0,
            actor.id() + " tenta aplicar status em " + target.id() +
                ": bloqueado pela blindagem EM."});
    }
}

void CombatStateMachine::resolve_defend(CombatActor& actor) {
    actor.add_status(StatusEffect{StatusId::Shield, /*magnitude=*/actor.def(),
                                  /*duration=*/1, StackRule::Replace, CardFamily::Eletrico});

    log_.push_back(CombatLogEntry{
        actor.id(), CombatActionType::Defend, actor.id(), actor.def(),
        actor.id() + " defende (Shield " + std::to_string(actor.def()) + ")."});
}

void CombatStateMachine::resolve_scan(CombatActor& actor, const CombatAction& action) {
    if (!action.target_id.has_value())
        throw std::logic_error("Scan exige TargetId.");

    CombatActor* target = find_in_order(queue_, *action.target_id);
    if (target == nullptr)
        throw std::logic_error("Alvo '" + *action.target_id + "' nao esta em combate.");

    target->set_scanned(true);

    log_.push_back(CombatLogEntry{
        actor.id(), CombatActionType::Scan, target->id(), target->hp(),
        actor.id() + " scaneia " + target->id() + ": HP " + std::to_string(target->hp()) +
            ", familia " + family_name(target->family()) + "."});

    // Scan aprimorado (ADR-016 step 8, John Dee/Black-Mirror, decisao do lider D1-ii): so
    // quando o SCANNER porta a carta RevealIntent equipada. Usa APENAS dados que ja existem
    // no modelo (status list, InitiativeQueue::index_of, IntentPreview) - nenhum atributo
    // oculto novo. 3 linhas extra, cada uma logada (regra todo-efeito-loga).
    if (has_reveal_intent_equipped(actor, card_registry_)) {
        std::string statuses;
        for (const StatusEffect& s : target->status_effects()) {
            if (!statuses.empty()) statuses += ", ";
            statuses += status_name(s.id);
        }
        if (statuses.empty()) statuses = "nenhum";
        log_.push_back(CombatLogEntry{
            actor.id(), CombatActionType::Scan, target->id(), 0,
            actor.id() + " (Black-Mirror) amplia o scan de " + target->id() +
                ": status ativos [" + statuses + "]."});

        const int index = queue_.index_of(target);
        log_.push_back(CombatLogEntry{
            actor.id(), CombatActionType::Scan, target->id(), index,
            actor.id() + " (Black-Mirror) amplia o scan de " + target->id() +
                ": posicao " + std::to_string(index) + " na fila de iniciativa."});

        techMagic::TechMagicContext ctx{&actor, nullptr, /*damage=*/0, &log_};
        ctx.queue = &queue_;
        ctx.brain_registry = brain_registry_;
        techMagic::log_intent_for(*target, ctx);
    }
}

void CombatStateMachine::resolve_scan_environment(CombatActor& actor) {
    environment_scanned_ = true;

    std::string ativos;
    if (active_environments_.empty()) {
        ativos = "nenhum";
    } else {
        for (std::size_t i = 0; i < active_environments_.size(); ++i) {
            if (i != 0) ativos += ", ";
            ativos += environment_name(active_environments_[i].id);
        }
    }

    std::string proxima_troca;
    if (period_clock_.has_value()) {
        proxima_troca = std::string(" Proximo periodo: ") +
                        environment_name(period_clock_->next_phase()) + " em " +
                        std::to_string(period_clock_->turns_remaining()) + " turno(s).";
    }

    log_.push_back(CombatLogEntry{
        actor.id(), CombatActionType::ScanEnvironment, std::nullopt, 0,
        actor.id() + " scaneia o ambiente: [" + ativos + "]." + proxima_troca});
}

void CombatStateMachine::resolve_gambit_predict(CombatActor& actor,
                                                const CombatAction& action) {
    if (!action.target_id.has_value())
        throw std::logic_error("Gambito Prever exige TargetId.");

    CombatActor* target = find_in_order(queue_, *action.target_id);
    if (target == nullptr)
        throw std::logic_error("Alvo '" + *action.target_id + "' nao esta em combate.");

    IEnemyBrain* brain = nullptr;
    if (brain_registry_ != nullptr) {
        const auto it = brain_registry_->find(target->id());
        if (it != brain_registry_->end())
            brain = it->second;
    }
    if (brain == nullptr)
        throw std::out_of_range(
            "Alvo '" + target->id() +
            "' nao tem IEnemyBrain no brain_registry; Gambito Prever exige brain registrado.");

    const CombatState state(queue_, &actor, queue_.round_index(), card_registry_);
    const IntentPreview intent = brain->preview_intent(state, *target);
    last_prediction_ = intent;

    const std::string legibility = intent.is_chaotic ? "[CAOTICO - ruido]" : "[LEGIVEL]";
    log_.push_back(CombatLogEntry{
        actor.id(), CombatActionType::GambitPredict, target->id(), intent.predicted_damage,
        actor.id() + " usa Gambito Prever em " + target->id() + ": preve " +
            intent.predicted_action_id + " -> " + intent.predicted_target_id + " (dano " +
            std::to_string(intent.predicted_damage) + "). " + legibility});
}

void CombatStateMachine::resolve_gambit_reorder(CombatActor& actor,
                                                const CombatAction& action) {
    if (!action.target_id.has_value())
        throw std::logic_error("Gambito Reordenar exige TargetId.");

    CombatActor* target = find_in_order(queue_, *action.target_id);
    if (target == nullptr)
        throw std::logic_error("Alvo '" + *action.target_id + "' nao esta em combate.");

    // Guard de cursor (decisao do lider 2026-07-15, A1/COMBATE-FILA-CURSOR-FIX): o alvo so
    // tem uma "acao futura" pra reordenar se AINDA nao agiu nesta rodada. O current() esta
    // EM RESOLUCAO agora (reordena-lo seria indefinido) e quem ja agiu (indice < cursor) nao
    // volta a jogar nesta rodada. Nos dois casos a carta DISSIPA: o AP ja foi debitado pelo
    // caller (janela de acao normal), so o efeito de reordenar que nao se aplica - estado
    // NORMAL, nao erro (mesma regra do handle_delay_action/Einstein).
    if (target == queue_.current() || queue_.index_of(target) < queue_.cursor()) {
        log_.push_back(CombatLogEntry{
            actor.id(), CombatActionType::GambitReorder, target->id(), 0,
            actor.id() + " usa Gambito Reordenar em " + target->id() +
                ": dissipa (alvo em acao agora ou ja agiu neste ciclo)."});
        return;
    }

    const int applied = queue_.reorder_pending(target, action.reorder_delta);

    const std::string suffix = applied == 0 ? " (no-op)" : "";
    log_.push_back(CombatLogEntry{
        actor.id(), CombatActionType::GambitReorder, target->id(), applied,
        actor.id() + " usa Gambito Reordenar: empurra " + target->id() + " " +
            std::to_string(applied) + " posicoes na fila." + suffix});
}

void CombatStateMachine::resolve_use_card(CombatActor& actor, const CombatAction& action,
                                          const CombatState& state) {
    if (!action.card_id.has_value())
        throw std::logic_error("UseCard exige CardId.");

    // Silence (secao 9 L252, secao 10): bloqueia JOGAR CARTAS (so ataque basico/defender/
    // flee sob Silence). Reusa o feedback de ERRO DE COMPILACAO (secao 10).
    for (const auto& s : actor.status_effects()) {
        if (s.id == StatusId::Silence)
            throw std::logic_error(
                "ERRO DE COMPILACAO: '" + actor.id() +
                "' esta silenciado (so ataque basico/defender/flee).");
    }

    const auto& registry = state.card_registry();
    const auto card_it = registry.find(*action.card_id);
    if (card_it == registry.end())
        throw std::out_of_range("Carta '" + *action.card_id + "' nao esta no registry de combate.");
    const Card& card = card_it->second;

    // Regra 1x/batalha das especiais ATIVA/HIBRIDA (executor techMagic, ADR-016 secao 20):
    // reusa a SEMANTICA "nao recarrega na batalha" da Analise Preditiva (secao 2.1) - NAO e
    // o mesmo flag (a Analise Preditiva ainda nao tem campo no dado da carta). Passiva/
    // ForaDeCombate e TODAS as Comuns sao isentas. Estetica de erro de compilacao, mesmo
    // padrao do bloqueio de Silence acima.
    if (card.tier != CardTier::Comum &&
        (card.category == CardCategory::Ativa || card.category == CardCategory::Hibrida)) {
        if (specials_cast_.count(card.id) > 0)
            throw std::logic_error(
                "ERRO DE COMPILACAO: '" + card.id +
                "' ja foi compilada nesta batalha (especiais Ativa/Hibrida sao 1x/batalha, "
                "secao 2.1).");
        specials_cast_.insert(card.id);
    }

    const std::optional<CardModifier>& modifier = action.modifier;

    // Pre-condicao Null (secao 8): exige Scan previo no alvo.
    CombatActor* primary_target = resolve_primary_target(actor, action, card);
    if (modifier.has_value() && *modifier == CardModifier::Null && primary_target != nullptr &&
        !primary_target->is_scanned())
        throw std::logic_error("Null requer Scan previo no alvo.");

    // Custo de mana (secao 5, 8): base + extra do modificador.
    int mana_extra = 0;
    if (modifier.has_value()) {
        switch (*modifier) {
            case CardModifier::Object: mana_extra = 1; break;
            case CardModifier::Stream: mana_extra = 2; break;
            case CardModifier::Null: mana_extra = 1; break;
        }
    }
    actor.spend_mana(card.mana_cost + mana_extra);

    // Pipeline de 1 slot (carta) + modificador -> casamento de combo (secao 10).
    std::vector<PipelineSlot> pipeline = {{PipelineSlotKind::Card, card.id}};
    if (modifier.has_value())
        pipeline.push_back(PipelineSlot{PipelineSlotKind::Modifier, modifier_name(*modifier)});

    const std::optional<ComboRecipe> combo = ComboTable::match(pipeline);
    const float mult_combo = combo.has_value() ? combo->mult_combo : 1.0f;

    if (combo.has_value()) {
        // display_name e a KEY de traducao do combo; o prefixo literal "COMPILADO:" e
        // tecnico/log (a UI resolve key->pt-br e formata, F2-G.5).
        log_.push_back(CombatLogEntry{
            actor.id(), CombatActionType::UseCard,
            primary_target != nullptr ? std::optional<std::string>(primary_target->id())
                                      : std::nullopt,
            0, "COMPILADO: " + combo->display_name});
    }

    // Disrupt (secao 9 L251): penalidade de Power na PROXIMA acao ofensiva do ATOR que
    // joga a carta. Multiplicativo (1 - Magnitude/100), consumido nesta carta.
    float mult_disrupt = 1.0f;
    {
        const auto& effects = actor.status_effects();
        const auto it = std::find_if(effects.begin(), effects.end(), [](const StatusEffect& s) {
            return s.id == StatusId::Disrupt;
        });
        if (it != effects.end()) {
            mult_disrupt = std::max(0.0f, 1.0f - static_cast<float>(it->magnitude) / 100.0f);
            actor.remove_status(StatusId::Disrupt);  // consumido nesta acao
        }
    }

    // Free-Order (Hayek, CARD-ENGINE-MANIFESTO item 7, AMB-09): le o ledger da rodada NO
    // ESTADO PRE-acao (round_actions_ so ganha a entrada desta acao DEPOIS, em
    // resolve_action - anti auto-inflacao). Calculado 1x pra acao inteira (nao por-alvo): o
    // lado do atacante nao muda entre alvos de um AoE.
    const HayekBonus hayek = hayek_bonus_for(actor.is_player_side(), queue_.order(),
                                             card_registry_, round_actions_);

    const std::vector<CombatActor*> targets = resolve_targets(actor, action, card);

    // RepeatLastAction (ADR-016 secao 20 item 5): agrega o dano>0 causado a CADA ALVO
    // desta carta (Q2), gravado ao FIM da funcao (depois do OnCast, ver abaixo).
    std::vector<std::pair<CombatActor*, int>> hits;

    for (CombatActor* target : targets) {
        // Fogo amigo DESLIGADO (regra geral, decisao do lider 2026-07-15, Balde B PR2):
        // carta NAO causa dano-base em alvo do PROPRIO time - so o efeito roda. Sem isso,
        // o dano-base = (card.power + actor.atk()) * fatores somava o ATK do conjurador
        // mesmo com power==0, machucando o proprio aliado nos modos-aliado (Einstein/
        // Faraday/Newton, que sao BENEFICIO). Dano 0 via o MESMO helper do ramo imune
        // abaixo (apply_damage_with_hooks com damage=0 - o guard damage<=0 la dentro ja
        // suprime os hooks OnDamageDealt/OnDamageReceived, mesmo padrao), SEM sortear
        // canal/variancia (0 consumo de RNG neste alvo) e SEM aplicar status OFENSIVO
        // (card.status_applied/combo->result_status sao debuffs de INIMIGO - pular pra
        // aliado). O loop CONTINUA pro proximo alvo; os EffectSpec OnCast da carta (ex.
        // Reflect-status AllyOnly do Newton) rodam DEPOIS deste loop, com seu proprio
        // side_filter, fora deste guard.
        const bool friendly = target->is_player_side() == actor.is_player_side();
        if (friendly) {
            apply_damage_with_hooks(actor, *target, 0, &card);
            log_.push_back(CombatLogEntry{
                actor.id(), CombatActionType::UseCard, target->id(), 0,
                actor.id() + " compila " + card.id + " em " + target->id() +
                    " (aliado, fogo amigo desligado): dano 0."});
            continue;
        }

        // Defesa neutra do compilador universal (secao 6.1): alvo universal => mult 1.0.
        float mult_fraqueza =
            target->is_universal_compiler()
                ? 1.0f
                : WeaknessWheel::multiplier(card.family, target->family());

        // Fura-defesa (Godel/Null-Proof, ADR-016 Balde B PR3, decisao do lider 2026-07-15):
        // DUAS vias independentes furam a roda de fraqueza. ORDEM CRITICA: isto roda ANTES
        // do curto-circuito de imunidade abaixo (senao o `continue` do imune agiria antes do
        // pierce) e antes do sorteio de canal (nem toca RNG, entao nao muda a ordem de
        // consumo dos casos SEM pierce, secao 11). Mantido em sincronia com
        // estimate_card_damage (preview PURO, mesma ordem, SEM consumir).
        //   (a) item 11 GENERICO: qualquer carta com ignores_weakness_wheel=true (Godel em
        //       si) sempre resolve mult 1.0 - independe do NullProof de quem quer que seja.
        //   (b) G-2/G-3: senao, se o ATACANTE porta NullProof E o mult calculado e < 1.0
        //       (Resistente 0.66 OU Imune 0.0 - fura OS DOIS), o trunfo fura (mult -> 1.0) e
        //       e CONSUMIDO. Contra Neutro/Fraco (mult >= 1.0) fica intacto - so consome
        //       quando ha algo a furar (G-2), nao e um consume-sempre.
        if (card.ignores_weakness_wheel) {
            mult_fraqueza = 1.0f;
        } else if (mult_fraqueza < 1.0f) {
            const auto& attacker_effects = actor.status_effects();
            const bool has_null_proof = std::any_of(
                attacker_effects.begin(), attacker_effects.end(),
                [](const StatusEffect& s) { return s.id == StatusId::NullProof; });
            if (has_null_proof) {
                mult_fraqueza = 1.0f;
                actor.remove_status(StatusId::NullProof);
                log_.push_back(CombatLogEntry{
                    actor.id(), CombatActionType::UseCard, target->id(), 0,
                    actor.id() + " fura a defesa de " + target->id() +
                        " com Null-Proof (sentenca indecidivel)."});
            }
        }

        constexpr float mult_mod = 1.0f;  // Stream distribui no jogo posterior; slice = 1.0

        // Expose (BUG-4, secao 11): alvo com Expose recebe dano * (1 + Mag/100). So UseCard.
        float mult_expose = 1.0f;
        {
            const auto& effects = target->status_effects();
            const auto it = std::find_if(effects.begin(), effects.end(),
                                         [](const StatusEffect& s) { return s.id == StatusId::Expose; });
            if (it != effects.end())
                mult_expose = 1.0f + static_cast<float>(it->magnitude) / 100.0f;
        }

        // mult_ambiente (secao 18, 11): penultimo fator da cadeia divisiva.
        const float mult_ambiente = mult_ambiente_for(card.family);

        // 1. Cadeia divisiva (secao 11). danoBase = "range da arma" base, ANTES da variancia.
        //    mult_hayek (Free-Order, item 7) e o ULTIMO fator da cadeia.
        const float base_damage =
            static_cast<float>(card.power + actor.atk()) *
            (100.0f / (100.0f + static_cast<float>(target->def()))) * mult_fraqueza * mult_mod *
            mult_combo * mult_expose * mult_disrupt * mult_ambiente * hayek.mult_damage();

        // 2. Curto-circuito de imunidade (secao 11): multFraqueza == 0 => dano 0 ANTES de
        //    qualquer sorteio. NAO consome RNG (determinismo: imune = 0 consumos).
        if (mult_fraqueza == 0.0f) {
            apply_damage_with_hooks(actor, *target, 0, &card);
            if (card.status_applied.has_value())
                apply_offensive_status(actor, *target, *card.status_applied);
            if (combo.has_value() && combo->result_status.has_value())
                apply_offensive_status(actor, *target, *combo->result_status);
            log_.push_back(CombatLogEntry{
                actor.id(), CombatActionType::UseCard, target->id(), 0,
                actor.id() + " compila " + card.id + " em " + target->id() + " por 0."});
            continue;
        }

        // 3. Variancia Knowledge (secao 11): "range da arma" deste encontro. Preservada.
        //    v = max(0.05, 0.30 * exp(-kills * 0.10)).
        const int kills = target->knowledge_kills();
        const float v = std::max(0.05f, 0.30f * std::exp(-static_cast<float>(kills) * 0.10f));

        // 4. Chances dos canais (secao 11).
        //    fumbleChance = round(5 * exp(-kills * 0.50)); 0 kills = 5%, 5+ kills = 0%.
        //    critChance   = max(5, card.CritChance); piso global de 5%, carta eleva.
        //    Free-Order (item 7): desconta hayek.fumble_reduction_pp do LIMIAR (piso 0) -
        //    NAO mexe na contagem de RNG (continua 1 UNICO sorteio de canal, so o limiar).
        const int fumble_chance = std::max(
            0, static_cast<int>(
                   std::lround(5.0 * std::exp(-static_cast<double>(kills) * 0.50))) -
                   hayek.fumble_reduction_pp);
        const int crit_chance = std::max(5, card.crit_chance);

        // 5. UM sorteio de canal (secao 11): consome rng.next(100) UMA vez.
        //    roll < fumble                -> FALHA
        //    roll < fumble + crit         -> CRIT  (faixa contigua a FALHA; ver secao 11)
        //    senao                        -> COMUM
        const int roll = rng_->next(100);

        int damage = 0;
        std::string channel_suffix;
        if (roll < fumble_chance) {
            // FALHA: dano 0. Log com estetica de erro de compilacao (secao 10). 1 consumo RNG.
            damage = 0;
            channel_suffix = " FALHA DE COMPILACAO";
        } else if (roll < fumble_chance + crit_chance) {
            // CRIT: round(maxArma * 1.5) = round(danoBase * (1 + v) * 1.5). 1 consumo RNG.
            // Helper compartilhado com estimate_card_damage (preview PURO) - mesma formula.
            const float crit_damage = crit_channel_damage(base_damage, v);
            damage = static_cast<int>(std::lround(crit_damage));
            channel_suffix = " [CRITICO]";
        } else {
            // COMUM: 2o consumo de RNG. SEM Quantum-Lock: variancia continua de sempre
            // (next_double). COM Quantum-Lock (Planck, ADR-016 manifesto item 5, A5): o 2o
            // consumo vira um sorteio de DEGRAU (next(100) em vez de next_double) - MESMA
            // contagem (1 consumo aqui de qualquer forma), so o TIPO muda. Helper
            // comum_channel_damage compartilhado com estimate_card_damage (preview PURO)
            // nos dois casos - so o `r` muda de continuo pra {0.0, 0.5, 1.0}.
            const EffectSpec* quantize = quantize_spec_of(actor, card_registry_);
            double r;
            if (quantize != nullptr) {
                const int roll2 = rng_->next(100);
                r = quantize_step_r(*quantize, roll2);
                channel_suffix = quantize_log_suffix(r, *quantize);
            } else {
                r = rng_->next_double();
            }
            const float rolled = comum_channel_damage(base_damage, v, r);
            damage = static_cast<int>(std::lround(rolled));
        }

        apply_damage_with_hooks(actor, *target, damage, &card);

        if (card.status_applied.has_value())
            apply_offensive_status(actor, *target, *card.status_applied);

        if (combo.has_value() && combo->result_status.has_value())
            apply_offensive_status(actor, *target, *combo->result_status);

        log_.push_back(CombatLogEntry{
            actor.id(), CombatActionType::UseCard, target->id(), damage,
            actor.id() + " compila " + card.id + " em " + target->id() + " por " +
                std::to_string(damage) + "." + channel_suffix + hayek_log_suffix(hayek)});

        // RepeatLastAction (ADR-016 secao 20 item 5, Q2): so agrega hits com dano>0
        // (FALHA/imune ja usaram `continue`/ficam em 0, nao chegam aqui com damage>0).
        if (damage > 0) hits.emplace_back(target, damage);
    }

    // OnCast (executor techMagic, ADR-016 secao 20 item 1): SO a carta JOGADA (nunca as
    // equipadas), aplicado POR CIMA da base, APOS o loop de dano/status por alvo acima, nos
    // MESMOS targets - inclusive alvos imunes (o curto-circuito de fraqueza so zera dano,
    // nao os efeitos declarativos da carta).
    if (!card.effects.empty()) {
        for (CombatActor* target : targets) {
            techMagic::TechMagicContext cast_ctx{&actor, target, /*damage=*/0, &log_};
            // RepeatLastAction (Mandelbrot/OnCast, ADR-016 secao 20 item 5): ctx.last_action
            // reflete o registro ANTES desta carta atualiza-lo (ver gravacao abaixo) - um
            // Mandelbrot ecoa a ULTIMA acao de dano de um aliado ja fechada nesta rodada,
            // nunca o proprio dano que ele acabou de causar no loop acima.
            cast_ctx.last_action = &last_action_;
            cast_ctx.rng = rng_;
            // ChainDamage (Tesla/OnCast, ADR-016 step 6): a cadeia salta pros proximos
            // inimigos vivos da fila -> injeta o roster completo (ctx.combatants) e o dano
            // REALMENTE causado a ESTE target no loop base (ctx.damage). `hits` ainda esta
            // intacto aqui (so e movido pro last_action_ DEPOIS deste loop); um alvo imune
            // (FALHA/fraqueza) nao tem entrada em `hits` -> dealt=0 -> cadeia no-op nesse
            // alvo. Nao afeta os outros handlers OnCast (ApplyStatus/Newton ignoram
            // ctx.damage/ctx.combatants).
            cast_ctx.combatants = &queue_.order();
            // DelayAction (Einstein/OnCast, ADR-016 step 7): a fila REAL (nao so o
            // snapshot ordenado de combatants) - handle_delay_action reordena via
            // InitiativeQueue::reorder_actor (mesma primitiva do Gambito).
            cast_ctx.queue = &queue_;
            // RevealIntent (John Dee/Black-Mirror, ADR-016 step 8): mesmo registry id->
            // IEnemyBrain* ja injetado na FSM, pro dump ler o intent de cada inimigo.
            cast_ctx.brain_registry = brain_registry_;
            int dealt = 0;
            for (const auto& [t, d] : hits)
                if (t == target) {
                    dealt = d;
                    break;
                }
            cast_ctx.damage = dealt;
            techMagic::execute(TriggerHook::OnCast, card, cast_ctx);
        }
    }

    // RepeatLastAction (ADR-016 secao 20 item 5, Q2): grava ao FIM da funcao (DEPOIS do
    // OnCast acima), so quando ao menos 1 alvo sofreu dano>0. Uma carta sem dano (status
    // puro, imune em todos os alvos) preserva o registro anterior (ainda valido na rodada).
    if (!hits.empty())
        last_action_ = techMagic::LastActionRecord{&actor, CombatActionType::UseCard, card.id,
                                                    std::move(hits)};
}

CombatActor* CombatStateMachine::resolve_primary_target(CombatActor& actor,
                                                        const CombatAction& action,
                                                        const Card& card) {
    if (card.target_shape == TargetShape::Self)
        return &actor;
    if (!action.target_id.has_value())
        return nullptr;
    return find_in_order(queue_, *action.target_id);
}

std::vector<CombatActor*> CombatStateMachine::resolve_targets(CombatActor& actor,
                                                              const CombatAction& action,
                                                              const Card& card) {
    switch (card.target_shape) {
        case TargetShape::Self:
            return {&actor};
        case TargetShape::Grupo:
        case TargetShape::Area3x3:
        case TargetShape::Linha: {
            // DOMINO consertado (Balde B PR2, achado de auditoria): a versao anterior
            // hardcodava "!is_player_side()" (sempre os inimigos) - se um INIMIGO castasse
            // uma carta Grupo, o alvo virava o proprio time dele. O lado-alvo correto e
            // sempre o OPOSTO ao `actor` que joga a carta (mesmo guard do ChainDamage,
            // handle_chain_damage, que ja acertava isto).
            //
            // Ramo assimetrico (Newton modo-aliado, N-3): se o alvo DECLARADO
            // (action.target_id) resolve pra um ator do MESMO lado de `actor`, o cast e um
            // BENEFICIO mirado - devolve so ESSE aliado (single), nao o grupo inteiro.
            // Ausente/nao encontrado/do lado oposto (o caso comum, ofensivo) => devolve
            // TODOS os vivos do lado OPOSTO ao actor.
            CombatActor* declared = action.target_id.has_value()
                                        ? find_in_order(queue_, *action.target_id)
                                        : nullptr;
            if (declared != nullptr && declared->is_player_side() == actor.is_player_side())
                return {declared};

            std::vector<CombatActor*> out;
            for (CombatActor* a : queue_.order())
                if (a->is_player_side() != actor.is_player_side() && a->is_alive())
                    out.push_back(a);
            return out;
        }
        case TargetShape::Single:
        default: {
            if (!action.target_id.has_value())
                throw std::logic_error("UseCard single-target exige TargetId.");
            CombatActor* target = find_in_order(queue_, *action.target_id);
            if (target == nullptr)
                throw std::logic_error("Alvo '" + *action.target_id + "' nao esta em combate.");
            return {target};
        }
    }
}

void CombatStateMachine::resolve_flee(CombatActor& actor) {
    const bool boss_presente = [&] {
        if (actor.is_boss()) return true;
        for (const CombatActor* a : queue_.order())
            if (!a->is_player_side() && a->is_alive() && a->is_boss())
                return true;
        return false;
    }();
    if (boss_presente)
        throw std::logic_error("Fuga impossivel de boss/mini-boss.");

    int party_top_spd = 0;
    int enemy_top_spd = 0;
    for (const CombatActor* a : queue_.order()) {
        if (!a->is_alive()) continue;
        if (a->is_player_side())
            party_top_spd = std::max(party_top_spd, a->spd());
        else
            enemy_top_spd = std::max(enemy_top_spd, a->spd());
    }

    const bool success = party_top_spd >= enemy_top_spd;
    if (success) {
        outcome_ = CombatOutcome::Fled;
        log_.push_back(CombatLogEntry{actor.id(), CombatActionType::Flee, std::nullopt, 0,
                                      actor.id() + " fugiu."});
    } else {
        log_.push_back(CombatLogEntry{actor.id(), CombatActionType::Flee, std::nullopt, 0,
                                      actor.id() + " falhou ao fugir."});
    }
}

void CombatStateMachine::prune_dead() {
    std::vector<CombatActor*> dead;
    for (CombatActor* a : queue_.order())
        if (!a->is_alive())
            dead.push_back(a);
    for (CombatActor* d : dead)
        queue_.remove(d);
}

// ---------------------------------------------------------------------------
// Status (secao 9): tick no TurnStart, expire no TurnEnd
// ---------------------------------------------------------------------------

bool CombatStateMachine::apply_status_tick(CombatActor& actor) {
    bool stunned = false;
    bool spd_changed = false;

    // Snapshot por VALOR (nao por indice): um tick de DoT pode esgotar um Shield e remove-
    // lo no meio do loop (BUG-3), invalidando indices. Iteramos um snapshot e re-localizamos
    // cada status pelo Id antes de decrementar a Duration.
    const std::vector<StatusEffect> snapshot = actor.status_effects();
    for (const StatusEffect& status : snapshot) {
        switch (status.id) {
            case StatusId::Poison:
                actor.take_damage(status.magnitude);
                break;
            case StatusId::Corrode:
                actor.take_damage(status.magnitude);
                actor.reduce_def(status.magnitude);  // reducao permanente (distinta do Break)
                break;
            case StatusId::Regen:
                actor.heal(status.magnitude);
                break;
            case StatusId::Stun:
                stunned = true;
                break;
            case StatusId::Break:
                actor.apply_stat_delta(StatusId::Break, /*def_delta=*/-status.magnitude,
                                       /*spd_delta=*/0);
                break;
            case StatusId::Haste:
                if (actor.apply_stat_delta(StatusId::Haste, 0, +status.magnitude))
                    spd_changed = true;
                break;
            case StatusId::Slow:
                if (actor.apply_stat_delta(StatusId::Slow, 0, -status.magnitude))
                    spd_changed = true;
                break;
            case StatusId::Decrypt:
                // Dispela TODOS os buffs do ator (secao 9 L256). RESET, NAO lockout.
                if (dispel_buffs(actor))
                    spd_changed = true;
                break;
            default:
                // Disrupt/Silence/Expose/Shield: sem tick de stat aqui. Knockback: resolvido
                // (e CONSUMIDO via remove_status) inteiramente em begin_turn, ANTES deste
                // metodo rodar (K-B, decisao do lider 2026-07-15, A2) - nunca aparece neste
                // snapshot pro current().
                break;
        }

        // Re-localiza: o indice pode ter mudado se outro status foi removido. Se este
        // proprio status sumiu, pula.
        const int idx = actor.index_of_status(status.id);
        if (idx >= 0) {
            StatusEffect updated = status;
            updated.duration = status.duration - 1;
            actor.replace_status_at(idx, updated);
        }
    }

    if (spd_changed)
        queue_.recompute_by_speed();

    return stunned;
}

bool CombatStateMachine::dispel_buffs(CombatActor& actor) {
    bool had_haste = false;
    std::vector<StatusId> to_remove;
    for (const StatusEffect& s : actor.status_effects()) {
        if (s.id == StatusId::Haste)
            had_haste = true;
        if (CombatActor::is_buff(s.id) &&
            std::find(to_remove.begin(), to_remove.end(), s.id) == to_remove.end())
            to_remove.push_back(s.id);
    }
    for (const StatusId id : to_remove)
        actor.remove_status(id);
    return had_haste;
}

}  // namespace gus::domain::combat

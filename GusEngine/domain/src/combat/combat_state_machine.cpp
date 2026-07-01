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
[[nodiscard]] const char* family_name(CardFamily f) {
    switch (f) {
        case CardFamily::Eletrico: return "Eletrico";
        case CardFamily::Bioquimico: return "Bioquimico";
        case CardFamily::Sonico: return "Sonico";
        case CardFamily::Cinetico: return "Cinetico";
        case CardFamily::Criptografico: return "Criptografico";
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

// Localiza um ator na ordem da fila por id (nullptr se ausente). Espelha
// Queue.Order.FirstOrDefault(a => a.Id == id) do C#.
[[nodiscard]] CombatActor* find_in_order(const InitiativeQueue& queue,
                                         const std::string& id) {
    for (CombatActor* a : queue.order())
        if (a->id() == id)
            return a;
    return nullptr;
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
    // SetupPhase -> TurnStart: a fila ja esta ordenada; o primeiro ator entra.
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
// Conducao da FSM
// ---------------------------------------------------------------------------

bool CombatStateMachine::begin_turn() {
    phase_ = CombatPhase::TurnStart;
    CombatActor& actor = *queue_.current();

    actor.refresh_resources_for_turn(queue_.round_index());

    const bool stunned = apply_status_tick(actor);
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

    if (queue_.round_index() > round_before)
        advance_period_clock();
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
}

int CombatStateMachine::preview_basic_attack_damage(
    const CombatActor& attacker, const CombatActor& target) const noexcept {
    // Dano bruto IDENTICO a resolve_basic_attack (logo abaixo). Mantido em sincronia.
    const int raw = std::max(combat_constants::kMinDamage, attacker.atk() - target.def());

    // Absorcao de Shield espelhada de CombatActor::absorb_with_shield, SEM mutar: pega a
    // magnitude do Shield ativo (0 se ausente) e devolve o remanescente que bateria no HP.
    // absorbed = min(raw, magnitude); com absorbed <= 0 (sem Shield ou pool vazio) a perda
    // e o dano bruto (espelha o early-return de absorb_with_shield). Piso 0 vem de raw -
    // min(raw, mag) = max(0, raw - mag) quando mag > 0.
    int shield_magnitude = 0;
    for (const StatusEffect& s : target.status_effects()) {
        if (s.id == StatusId::Shield) {
            shield_magnitude = s.magnitude;
            break;
        }
    }
    const int absorbed = std::min(raw, shield_magnitude);
    if (absorbed <= 0) {
        return raw;
    }
    return raw - absorbed;
}

void CombatStateMachine::resolve_basic_attack(CombatActor& attacker,
                                              const CombatAction& action) {
    if (!action.target_id.has_value())
        throw std::logic_error("Attack exige TargetId.");

    CombatActor* target = find_in_order(queue_, *action.target_id);
    if (target == nullptr)
        throw std::logic_error("Alvo '" + *action.target_id + "' nao esta em combate.");

    const int damage = std::max(combat_constants::kMinDamage, attacker.atk() - target->def());
    target->take_damage(damage);

    log_.push_back(CombatLogEntry{
        attacker.id(), CombatActionType::Attack, target->id(), damage,
        attacker.id() + " ataca " + target->id() + " por " + std::to_string(damage) + "."});
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

    queue_.reorder_actor(target, action.reorder_delta);

    const std::string suffix = action.reorder_delta == 0 ? " (no-op: delta 0)" : "";
    log_.push_back(CombatLogEntry{
        actor.id(), CombatActionType::GambitReorder, target->id(), action.reorder_delta,
        actor.id() + " usa Gambito Reordenar: empurra " + target->id() + " " +
            std::to_string(action.reorder_delta) + " posicoes na fila." + suffix});
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

    const std::vector<CombatActor*> targets = resolve_targets(actor, action, card);

    for (CombatActor* target : targets) {
        // Defesa neutra do compilador universal (secao 6.1): alvo universal => mult 1.0.
        const float mult_fraqueza =
            target->is_universal_compiler()
                ? 1.0f
                : WeaknessWheel::multiplier(card.family, target->family());
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

        // mult_ambiente (secao 18, 11): ULTIMO fator da cadeia divisiva.
        const float mult_ambiente = mult_ambiente_for(card.family);

        // 1. Cadeia divisiva (secao 11). danoBase = "range da arma" base, ANTES da variancia.
        const float base_damage =
            static_cast<float>(card.power + actor.atk()) *
            (100.0f / (100.0f + static_cast<float>(target->def()))) * mult_fraqueza * mult_mod *
            mult_combo * mult_expose * mult_disrupt * mult_ambiente;

        // 2. Curto-circuito de imunidade (secao 11): multFraqueza == 0 => dano 0 ANTES de
        //    qualquer sorteio. NAO consome RNG (determinismo: imune = 0 consumos).
        if (mult_fraqueza == 0.0f) {
            target->take_damage(0);
            if (card.status_applied.has_value())
                target->add_status(*card.status_applied);
            if (combo.has_value() && combo->result_status.has_value())
                target->add_status(*combo->result_status);
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
        const int fumble_chance = static_cast<int>(
            std::lround(5.0 * std::exp(-static_cast<double>(kills) * 0.50)));
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
            const float crit_damage = base_damage * (1.0f + v) * 1.5f;
            damage = static_cast<int>(std::lround(crit_damage));
            channel_suffix = " [CRITICO]";
        } else {
            // COMUM: 2o consumo de RNG (next_double); aplica a variancia normal.
            const double r = rng_->next_double();
            const float rolled = base_damage * static_cast<float>(
                                                   1.0 + (static_cast<double>(v) * 2.0 * r -
                                                          static_cast<double>(v)));
            damage = static_cast<int>(std::lround(rolled));
        }

        target->take_damage(damage);

        if (card.status_applied.has_value())
            target->add_status(*card.status_applied);

        if (combo.has_value() && combo->result_status.has_value())
            target->add_status(*combo->result_status);

        log_.push_back(CombatLogEntry{
            actor.id(), CombatActionType::UseCard, target->id(), damage,
            actor.id() + " compila " + card.id + " em " + target->id() + " por " +
                std::to_string(damage) + "." + channel_suffix});
    }
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
            // Simplificacao do slice: aplica a todos os inimigos vivos.
            std::vector<CombatActor*> out;
            for (CombatActor* a : queue_.order())
                if (!a->is_player_side() && a->is_alive())
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
            case StatusId::Knockback:
                // secao 9 L253 / secao 8 L105: empurra o ator afetado +1 na fila. One-shot:
                // consome a Duration toda neste tick. O ator NAO perde o turno corrente.
                queue_.reorder_actor(&actor, +1);
                queue_.sync_cursor_to(&actor);
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
                // Disrupt/Silence/Expose/Shield: sem tick de stat aqui.
                break;
        }

        // Re-localiza: o indice pode ter mudado se outro status foi removido. Se este
        // proprio status sumiu, pula.
        const int idx = actor.index_of_status(status.id);
        if (idx >= 0) {
            // Knockback one-shot: zera a Duration. Demais decrementam normalmente.
            const int new_duration =
                status.id == StatusId::Knockback ? 0 : status.duration - 1;
            StatusEffect updated = status;
            updated.duration = new_duration;
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

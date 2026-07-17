// gus/domain/combat/combat_records.hpp
//
// Records imutaveis de dados do combate, portados de
// engine/foundation/turn_combat/CombatRecords.cs. POCO puro, ZERO Qt (invariante de
// domain/, engine-design.md secao 2). Header-only: sao dados puros + factories triviais.
//
// readonly record struct (C#) -> struct C++ de campos publicos com igualdade por valor
// (operator== = default), mesmo padrao de CharacterTemplate/EnemyTemplate no M3.
//
// MAPEAMENTO de tipos C# -> C++:
//   string                 -> std::string
//   string?                -> std::optional<std::string>
//   StatusEffect?          -> std::optional<StatusEffect>
//   CardModifier?          -> std::optional<CardModifier>
//   IReadOnlyList<T>       -> std::vector<T> (dados imutaveis por convencao; sem const
//                             no campo para preservar igualdade/copia de record)
//
// IMPORTANTE (frontEIRA de porte): CombatActor (entidade MUTAVEL) e CombatState (view
// read-only sobre InitiativeQueue) NAO entram aqui. Sao os chunks 2-4. Este arquivo
// cobre so os records de DADOS imutaveis (secao 7/9/10/16/17).
//
// Cross-ref: engine/foundation/turn_combat/CombatRecords.cs;
//            docs/design/mecanicas/combat.md secao 3/5/7/8/9/10/16/17; ADR-006.

#ifndef GUS_DOMAIN_COMBAT_COMBAT_RECORDS_HPP
#define GUS_DOMAIN_COMBAT_COMBAT_RECORDS_HPP

#include <optional>
#include <string>
#include <vector>

#include "gus/domain/combat/combat_enums.hpp"

namespace gus::domain::combat {

// Status uniforme aplicado a qualquer ator. Tick processado no TurnStart. secao 9.
struct StatusEffect {
    StatusId id = StatusId::Stun;
    int magnitude = 0;
    int duration = 0;
    StackRule stack_rule = StackRule::Replace;
    CardFamily family_origin = CardFamily::Eletrico;

    [[nodiscard]] bool operator==(const StatusEffect&) const = default;
};

// Instrucao declarativa do executor techMagic (ADR-016): a carta e um programa (lista
// ordenada de EffectSpec) que o Tavus-Drive executa. Params tunaveis; significado por-kind.
struct EffectSpec {
    TriggerHook trigger = TriggerHook::OnCast;
    EffectKind kind = EffectKind::ApplyStatus;
    int magnitude = 0;
    int percent = 0;
    int duration = 0;
    StatusId status = StatusId::Stun;
    StackRule stack_rule = StackRule::Replace;
    // Filtro de lado do alvo (ADR-016 Balde B, Faraday/EM-Shield). Campo ADITIVO ao fim do
    // struct: default Any preserva TODAS as cartas/EffectSpec/testes existentes intactos
    // (nenhum filtro). Ver combat_enums.hpp::SideFilter + techmagic.cpp::handle_apply_status.
    SideFilter side_filter = SideFilter::Any;

    [[nodiscard]] bool operator==(const EffectSpec&) const = default;
};

// Carta-base imutavel (modelo B). Modificadores anexados em runtime via modifiers. secao 7.
struct Card {
    std::string id;
    // KEY de traducao (ex: CARD_PULSO_ELETRICO_NAME); resolvida via tr() no DISPLAY/UI,
    // nunca no runtime de combate.
    std::string display_name;
    CardFamily family = CardFamily::Eletrico;
    CardBaseType base_type = CardBaseType::Pulso;
    int mana_cost = 0;
    int ap_cost = 1;  // default 1 (espelha ApCost do C#)
    int power = 0;
    TargetShape target_shape = TargetShape::Single;
    std::optional<StatusEffect> status_applied;
    std::vector<CardModifier> modifiers;
    int mastery = 0;  // cresce por uso (Pillar 1)
    // 0..100, porcentagem de crit; 0 = sem crit (secao 7/11). Default 0.
    int crit_chance = 0;

    // ---- Executor techMagic (ADR-016, MVP step 1): record-only, sem mudanca de
    // comportamento no resolvedor neste step. Defaults preservam as 5 comuns intocadas
    // (tier Comum, effects vazio).
    CardTier tier = CardTier::Comum;
    CardCategory category = CardCategory::Ativa;  // semantica so p/ tier != Comum
    std::vector<EffectSpec> effects;               // vazio nas COMUNS (intocadas)
    bool ignores_weakness_wheel = false;  // trunfo fora-da-roda (Godel); resolvedor liga depois
    bool is_universal_compiler = false;   // flag de trunfo ADR-016; record-only neste step

    // ---- CARTAS-COMUNS-ENGINE (TODO.md, decisoes do lider 2026-07-16): 2 pecas de engine
    // pequenas pras COMUNS (NAO tocam o executor techMagic/EffectKind; ADR-016 continua so
    // ESPECIAL/SUPER). Campos ADITIVOS ao fim do struct, default neutro preserva TODA carta/
    // teste existente intacta.

    // SynergyStatus (Finalizador Opcao A): generaliza o multExpose (secao 9/11, que so cobre
    // Expose) pra QUALQUER StatusId. Vazio = sem sinergia (comportamento atual). Se >=1 dos
    // status listados aqui esta presente no ALVO, aplica o fator FIXO synergy_percent - NAO
    // stacka por-status-presente (2+ status da lista no alvo = MESMO fator, e binario
    // presente/ausente). Ver combat_state_machine.cpp::resolve_use_card/estimate_card_damage
    // (mult_synergy, mesmo padrao ordinal do mult_expose).
    std::vector<StatusId> synergy_statuses;
    // Percentual do bonus multiplicativo quando a sinergia dispara (40 = +40%, mesma forma
    // de StatusEffect::magnitude do Expose). 0 = sem bonus (default neutro). Data-driven
    // por-carta: a logica do resolvedor NUNCA hardcoda 40 (a carta e quem carrega o numero).
    int synergy_percent = 0;

    // Recarga de recurso (Eletrico-utilidade "Tavus-Overclock"): ao ser jogada, concede
    // grant_bonus_ap(restore_ap) + restore_mana(restore_mana) ao PROPRIO conjurador, apos o
    // custo de mana ja ter sido pago. Sujeito a trava 1x/turno (CombatActor::overclock_used_,
    // resetada no refresh de TurnStart). 0/0 = sem recarga (default neutro, toda carta
    // existente intocada). Ver resolve_use_card.
    int restore_ap = 0;
    int restore_mana = 0;

    [[nodiscard]] bool operator==(const Card&) const = default;
};

// Slot da pipeline de compilacao (Tavus-Drive). secao 10.
struct PipelineSlot {
    PipelineSlotKind kind = PipelineSlotKind::Card;  // Card | Modifier
    std::string ref;  // Card.Id ou CardModifier

    [[nodiscard]] bool operator==(const PipelineSlot&) const = default;
};

// Receita de combo deterministico (formato canonico; tabela completa e jogo posterior). secao 10.
struct ComboRecipe {
    std::string combo_id;
    // KEY de traducao (ex: COMBO_PULSO_STREAM_NAME); a UI resolve key->pt-br.
    std::string display_name;
    std::vector<PipelineSlot> signature;
    std::optional<StatusEffect> result_status;
    float mult_combo = 1.0f;
    bool discoverable = false;

    [[nodiscard]] bool operator==(const ComboRecipe&) const = default;
};

// Telegraph obrigatorio de intencao inimiga (contrato de IEnemyBrain). secao 13.
struct IntentPreview {
    std::string actor_id;
    std::string predicted_action_id;
    int predicted_damage = 0;
    TargetShape predicted_shape = TargetShape::Single;
    std::string predicted_target_id;
    bool is_chaotic = false;  // true => Gambito-Prever retorna ruido

    [[nodiscard]] bool operator==(const IntentPreview&) const = default;
};

// Acao escolhida por um ator em ActionSelect. Imutavel. secao 3/5.
// Use as factories estaticas pra garantir custo de AP canonico (tabela secao 5).
struct CombatAction {
    CombatActionType type = CombatActionType::Pass;
    int ap_cost = 0;
    std::optional<std::string> target_id;
    std::optional<std::string> card_id;
    std::optional<CardModifier> modifier;
    int reorder_delta = 0;

    [[nodiscard]] bool operator==(const CombatAction&) const = default;

    // Encerra o turno do ator. 0 AP.
    [[nodiscard]] static CombatAction pass() {
        return CombatAction{CombatActionType::Pass, /*ap_cost=*/0};
    }

    // Ataque basico. 1 AP, 0 mana.
    [[nodiscard]] static CombatAction attack(std::string target_id) {
        CombatAction a{CombatActionType::Attack, /*ap_cost=*/1};
        a.target_id = std::move(target_id);
        return a;
    }

    // Defender (stub incremento 2). 1 AP.
    [[nodiscard]] static CombatAction defend() {
        return CombatAction{CombatActionType::Defend, /*ap_cost=*/1};
    }

    // Fuga. 1 AP. Threshold de SPD deterministico no slice.
    [[nodiscard]] static CombatAction flee() {
        return CombatAction{CombatActionType::Flee, /*ap_cost=*/1};
    }

    // Jogar carta (stub incremento 2). 1 AP + custo da carta.
    [[nodiscard]] static CombatAction use_card(std::string card_id,
                                               std::string target_id) {
        CombatAction a{CombatActionType::UseCard, /*ap_cost=*/1};
        a.target_id = std::move(target_id);
        a.card_id = std::move(card_id);
        return a;
    }

    // Scan (stub incremento 2). 1 AP.
    [[nodiscard]] static CombatAction scan(std::string target_id) {
        CombatAction a{CombatActionType::Scan, /*ap_cost=*/1};
        a.target_id = std::move(target_id);
        return a;
    }

    // Scan-ambiente (secao 18.5): revela o ambiente ativo + a proxima troca. 1 AP, sem alvo.
    [[nodiscard]] static CombatAction scan_environment() {
        return CombatAction{CombatActionType::ScanEnvironment, /*ap_cost=*/1};
    }

    // Gambito Prever (stub incremento 3). 1 AP.
    [[nodiscard]] static CombatAction gambit_predict(std::string target_id) {
        CombatAction a{CombatActionType::GambitPredict, /*ap_cost=*/1};
        a.target_id = std::move(target_id);
        return a;
    }

    // Gambito Reordenar (stub incremento 3). 2 AP.
    [[nodiscard]] static CombatAction gambit_reorder(std::string target_id,
                                                     int delta) {
        CombatAction a{CombatActionType::GambitReorder, /*ap_cost=*/2};
        a.target_id = std::move(target_id);
        a.reorder_delta = delta;
        return a;
    }
};

// Entrada de log de combate (auditoria + UI feed). secao 16.
struct CombatLogEntry {
    std::string actor_id;
    CombatActionType action = CombatActionType::Pass;
    std::optional<std::string> target_id;
    int value = 0;  // dano/cura/etc; 0 quando nao aplica
    std::string message;

    [[nodiscard]] bool operator==(const CombatLogEntry&) const = default;
};

// Mudanca de status registrada pela FSM e drenada pos-turno pelo CombatManager (secao 16).
// Para Applied: valores do status aplicado; para Absorbed: magnitude = pool restante do
// Shield apos a absorcao; para Expired: ambos costumam ser 0.
struct StatusEffectChange {
    std::string actor_id;
    StatusId id = StatusId::Stun;
    StatusChangeKind kind = StatusChangeKind::Applied;
    int magnitude = 0;
    int duration = 0;

    [[nodiscard]] bool operator==(const StatusEffectChange&) const = default;
};

// Resultado final de combate, retornado por CombatEnd. secao 3/16.
struct CombatResult {
    CombatOutcome outcome = CombatOutcome::Ongoing;
    std::vector<CombatLogEntry> log;
    int rounds_elapsed = 0;

    [[nodiscard]] bool operator==(const CombatResult&) const = default;
};

}  // namespace gus::domain::combat

#endif  // GUS_DOMAIN_COMBAT_COMBAT_RECORDS_HPP

// gus/domain/combat/combat_records.hpp
//
// Records imutaveis de dados do combate, portados de
// engine/foundation/turn_combat/CombatRecords.cs. POCO puro, ZERO Qt (invariante de
// domain/, engine-design.md secao 2). Header-only: sao dados puros + factories triviais.
//
// ATOM-2 (decomposicao atomica ao nivel de modulo, generalizando ADR-019): os
// records de VOCABULARIO DE CARTA (StatusEffect, EffectSpec, Card - migram JUNTOS)
// foram EXTRAIDOS para gus/domain/cards/card_records.hpp (LAR CANONICO). Este header
// virou FACHADA para esse pedaco: inclui card_records.hpp e RE-EXPORTA os nomes em
// gus::domain::combat via using-declaration (MESMA identidade de tipo) - os ~90
// consumidores existentes continuam enxergando `gus::domain::combat::Card` etc.
// intocados. Os records de combate PROPRIAMENTE DITO (PipelineSlot, ComboRecipe,
// IntentPreview, CombatAction, CombatLogEntry, StatusEffectChange, CombatResult)
// continuam aqui - dependem de StatusEffect/CardModifier/TargetShape (agora vindos
// da fachada de card_enums.hpp via combat_enums.hpp) mas nao sao vocabulario de
// carta em si.
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
// Cross-ref: gus/domain/cards/card_records.hpp (LAR de StatusEffect/EffectSpec/Card);
//            engine/foundation/turn_combat/CombatRecords.cs;
//            docs/design/mecanicas/combat.md secao 3/5/7/8/9/10/16/17; ADR-006; ADR-019.

#ifndef GUS_DOMAIN_COMBAT_COMBAT_RECORDS_HPP
#define GUS_DOMAIN_COMBAT_COMBAT_RECORDS_HPP

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "gus/domain/cards/card_records.hpp"
#include "gus/domain/combat/combat_enums.hpp"

namespace gus::domain::combat {

// ---- Re-exporta o vocabulario de carta (LAR CANONICO: gus/domain/cards/card_records.hpp,
// ATOM-2). Using-declaration preserva a IDENTIDADE de tipo.
using cards::Card;
using cards::EffectSpec;
using cards::StatusEffect;

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

    // Ponte instancia->combate (CARDS-HW-2 fatia 1, VIRUS EM COMBATE; docs/design/mecanicas/
    // cartas-spec-logica.md secao 1/4): qual CardInstance (deck-mao-sistema.md secao 7) foi
    // jogada, pro motor de combate consultar o integrity_ledger (card_integrity_ledger.hpp) e
    // decidir se a carta esta infectada. Campo ADITIVO ao fim do struct: default nullopt
    // preserva TODO call site/teste existente intacto (mesmo padrao de EffectSpec::
    // side_filter). O preview/estimate (estimate_card_damage) NUNCA le isto (nao sorteia, nao
    // infecta - preserva o gemeo preview<->real). nullopt = comportamento IDENTICO ao motor
    // sem vírus (nenhum lookup no ledger acontece, ver CombatStateMachine::resolve_use_card).
    std::optional<std::uint64_t> card_instance_id;

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

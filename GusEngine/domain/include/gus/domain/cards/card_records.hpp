// gus/domain/cards/card_records.hpp
//
// Records imutaveis de VOCABULARIO DE CARTA (ATOM-2, extraidos de
// gus/domain/combat/combat_records.hpp): StatusEffect, EffectSpec, Card. POCO puro,
// ZERO Qt (invariante de domain/, engine-design.md secao 2). Header-only: sao dados
// puros + operator== por valor.
//
// FRONTEIRA DE CAMADA (arquitetura atomica, decisao do lider): este header e o LAR
// CANONICO destes 3 records (migram JUNTOS - Card contem StatusEffect/EffectSpec
// como membros). gus/domain/combat/combat_records.hpp virou FACHADA (include +
// using-declaration, MESMA identidade de tipo) - os records de combate PROPRIAMENTE
// DITO (PipelineSlot/ComboRecipe/IntentPreview/CombatAction/CombatLogEntry/
// StatusEffectChange/CombatResult) continuam la, e passam a incluir ESTE header pra
// resolver StatusEffect/Card. cards/ NAO pode incluir combat/ (gate de camadas).
//
// Cross-ref: gus/domain/cards/card_enums.hpp;
//            gus/domain/combat/combat_records.hpp (fachada);
//            engine/foundation/turn_combat/CombatRecords.cs;
//            docs/design/mecanicas/combat.md secao 7/8/9; ADR-006; ADR-016;
//            ADR-019 (decomposicao atomica, generalizada ao nivel de modulo aqui).

#ifndef GUS_DOMAIN_CARDS_CARD_RECORDS_HPP
#define GUS_DOMAIN_CARDS_CARD_RECORDS_HPP

#include <optional>
#include <string>
#include <vector>

#include "gus/domain/cards/card_enums.hpp"

namespace gus::domain::cards {

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
    // (nenhum filtro). Ver card_enums.hpp::SideFilter + techmagic.cpp::handle_apply_status.
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

    // ---- CARDS-HARDWARE-ENGINE incremento 1 (CARDS-HW-1, cartas-spec-dados.md
    // secao 3): clone-falso de especial (cartas-hardware-pirataria-energia.md
    // secao 2, "Clone-falso de especial"). Quando preenchido, esta entrada de
    // CATALOGO e uma imitacao pirata que IMPERSONA a carta especial cujo Card::id
    // esta aqui. A imitacao e sua PROPRIA entrada de catalogo (tier =
    // CardTier::Comum, id PROPRIO, ex. "cardExec-faraday-fake"), NUNCA o mesmo id
    // da especial real - preserva a unicidade (deck-mao-sistema.md secao 7 inv.9)
    // sem exigir NENHUM caso especial em CardCollection. std::nullopt = carta
    // normal (imensa maioria; default preserva todo catalogo existente intacto).
    std::optional<std::string> mimics_special_id;

    // ---- CARDS-HW-3C (TODO.md, docs/design/mecanicas/cartas-spec-logica.md secao 4.1/9):
    // flag de CATALOGO da carta Adware Sterling - opt-in CONSCIENTE do jogador (aceitou a
    // carta gratis sabendo do anuncio, cartas-hardware-pirataria-energia.md secao 10). NAO
    // e infeccao hostil (isso e gus::domain::infection::VirusKind::AdwareSterling, fora da
    // rolagem de contaminacao - ver contamination_service.hpp). Quando true, resolve_
    // use_card intercepta ANTES do debito de recurso via CombatStateMachine::
    // dispatch_adware_gate (gus/domain/combat/adware_sterling.hpp). false = imensa maioria
    // do catalogo, pipeline IDENTICO ao motor sem adware (default preserva toda carta/teste
    // existente intacta).
    bool has_adware = false;

    [[nodiscard]] bool operator==(const Card&) const = default;
};

}  // namespace gus::domain::cards

#endif  // GUS_DOMAIN_CARDS_CARD_RECORDS_HPP

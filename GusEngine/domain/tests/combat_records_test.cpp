// combat_records_test.cpp
//
// Spec executavel (Catch2 v3) dos records imutaveis de dados do combate, portados de
// engine/foundation/turn_combat/CombatRecords.cs (origem canonica) e dos casos
// portaveis de engine/tests/turn_combat/CombatDataRecordsTests.cs (xUnit = SPEC).
// POCO puro, ZERO Qt, headless. Marco M5 (chunk 1: fundacoes do combate).
//
// FRONTEIRA DE PORTE: os casos de CombatDataRecordsTests.cs que exercitam
// CombatActor / CombatStateMachine (UseCard/Scan/Gambito via FSM) NAO sao portados
// aqui: dependem de CombatActor (entidade mutavel) e da maquina de estados, que sao
// os chunks 2-4 (fora de escopo deste sub-porte). Portamos APENAS os casos de dados
// puros: value equality de Card, campos de StatusEffect, IntentPreview e as factories
// de CombatAction (tipo + custo de AP canonico, tabela secao 5). O comportamento das
// factories e identico ao C# 1:1.
//
// Records C# (readonly record struct) viram structs C++ de campos publicos com
// igualdade por valor (operator== = default), mesmo padrao ja usado em
// CharacterTemplate/EnemyTemplate no M3.
//
// Cross-ref: engine/foundation/turn_combat/CombatRecords.cs;
//            engine/tests/turn_combat/CombatDataRecordsTests.cs;
//            docs/design/mecanicas/combat.md secao 3/5/7/8/9/10/16/17; ADR-006.

#include <catch2/catch_test_macros.hpp>

#include <optional>
#include <string>
#include <vector>

#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/combat/combat_records.hpp"

using namespace gus::domain::combat;

// ---- Card: value record imutavel (CombatDataRecordsTests.Card_e_value_record) -----

TEST_CASE("combat_records: Card e value record (igualdade por valor)",
          "[domain][combat][records][card]") {
    const Card c1{
        /*id=*/"pulso_eletrico",
        /*display_name=*/"CARD_PULSO_ELETRICO",
        /*family=*/CardFamily::Eletrico,
        /*base_type=*/CardBaseType::Pulso,
        /*mana_cost=*/2,
        /*ap_cost=*/1,
        /*power=*/12,
        /*target_shape=*/TargetShape::Single,
        /*status_applied=*/std::nullopt,
        /*modifiers=*/{},
        /*mastery=*/0,
        /*crit_chance=*/0};

    Card c2 = c1;  // copia identica == "c1 with { Mastery = 0 }"
    REQUIRE(c1 == c2);

    Card c3 = c1;
    c3.mastery = 5;  // == "c1 with { Mastery = 5 }"
    REQUIRE(c1 != c3);
}

TEST_CASE("combat_records: Card ap_cost e crit_chance tem default canonico",
          "[domain][combat][records][card]") {
    // No C# ApCost nao tem default (sempre informado), CritChance default 0. Aqui o
    // crit_chance default 0 preserva os construtores que nao passam crit (spec secao 7).
    Card c{};
    REQUIRE(c.crit_chance == 0);
}

// ---- StatusEffect (CombatDataRecordsTests.StatusEffect_carrega_campos_da_spec) ----

TEST_CASE("combat_records: StatusEffect carrega campos da spec",
          "[domain][combat][records][status]") {
    const StatusEffect s{
        /*id=*/StatusId::Poison,
        /*magnitude=*/3,
        /*duration=*/2,
        /*stack_rule=*/StackRule::StackDuration,
        /*family_origin=*/CardFamily::Bioquimico};

    REQUIRE(s.id == StatusId::Poison);
    REQUIRE(s.magnitude == 3);
    REQUIRE(s.duration == 2);
    REQUIRE(s.stack_rule == StackRule::StackDuration);
    REQUIRE(s.family_origin == CardFamily::Bioquimico);
}

// ---- IntentPreview (CombatDataRecordsTests.IntentPreview_modela_telegraph) --------

TEST_CASE("combat_records: IntentPreview modela telegraph",
          "[domain][combat][records][intent]") {
    const IntentPreview i{
        /*actor_id=*/"enemy",
        /*predicted_action_id=*/"attack",
        /*predicted_damage=*/7,
        /*predicted_shape=*/TargetShape::Single,
        /*predicted_target_id=*/"gus",
        /*is_chaotic=*/false};

    REQUIRE(i.actor_id == "enemy");
    REQUIRE(i.is_chaotic == false);
}

// ---- CombatAction factories (custo de AP canonico, tabela secao 5) ----------------
// Porta de CombatDataRecordsTests.CombatAction_factories_definem_tipo_e_custo_ap.

TEST_CASE("combat_records: CombatAction::pass tipo Pass e 0 AP",
          "[domain][combat][records][action]") {
    REQUIRE(CombatAction::pass().type == CombatActionType::Pass);
    REQUIRE(CombatAction::pass().ap_cost == 0);
}

TEST_CASE("combat_records: CombatAction::attack tipo/custo/alvo",
          "[domain][combat][records][action]") {
    const auto atk = CombatAction::attack("alvo");
    REQUIRE(atk.type == CombatActionType::Attack);
    REQUIRE(atk.ap_cost == 1);
    REQUIRE(atk.target_id == "alvo");
}

TEST_CASE("combat_records: CombatAction::flee custa 1 AP",
          "[domain][combat][records][action]") {
    REQUIRE(CombatAction::flee().ap_cost == 1);
    REQUIRE(CombatAction::flee().type == CombatActionType::Flee);
}

TEST_CASE("combat_records: CombatAction::defend custa 1 AP",
          "[domain][combat][records][action]") {
    REQUIRE(CombatAction::defend().type == CombatActionType::Defend);
    REQUIRE(CombatAction::defend().ap_cost == 1);
}

TEST_CASE("combat_records: CombatAction::use_card 1 AP + alvo + carta",
          "[domain][combat][records][action]") {
    const auto a = CombatAction::use_card("pulso", "e");
    REQUIRE(a.type == CombatActionType::UseCard);
    REQUIRE(a.ap_cost == 1);
    REQUIRE(a.card_id == "pulso");
    REQUIRE(a.target_id == "e");
}

TEST_CASE("combat_records: CombatAction::scan 1 AP + alvo",
          "[domain][combat][records][action]") {
    const auto a = CombatAction::scan("e");
    REQUIRE(a.type == CombatActionType::Scan);
    REQUIRE(a.ap_cost == 1);
    REQUIRE(a.target_id == "e");
}

TEST_CASE("combat_records: CombatAction::scan_environment 1 AP sem alvo",
          "[domain][combat][records][action]") {
    const auto a = CombatAction::scan_environment();
    REQUIRE(a.type == CombatActionType::ScanEnvironment);
    REQUIRE(a.ap_cost == 1);
    REQUIRE(a.target_id == std::nullopt);
}

TEST_CASE("combat_records: CombatAction::gambit_predict 1 AP + alvo",
          "[domain][combat][records][action]") {
    const auto a = CombatAction::gambit_predict("e");
    REQUIRE(a.type == CombatActionType::GambitPredict);
    REQUIRE(a.ap_cost == 1);
    REQUIRE(a.target_id == "e");
}

TEST_CASE("combat_records: CombatAction::gambit_reorder 2 AP + alvo + delta",
          "[domain][combat][records][action]") {
    const auto a = CombatAction::gambit_reorder("e", 2);
    REQUIRE(a.type == CombatActionType::GambitReorder);
    REQUIRE(a.ap_cost == 2);
    REQUIRE(a.target_id == "e");
    REQUIRE(a.reorder_delta == 2);
}

// ---- PipelineSlot / ComboRecipe / CombatLogEntry / StatusEffectChange -------------
// Records de dados puros (secao 10/16). So checamos construcao + igualdade por valor.

TEST_CASE("combat_records: PipelineSlot guarda Kind + Ref",
          "[domain][combat][records]") {
    const PipelineSlot s{/*kind=*/PipelineSlotKind::Card, /*ref=*/"pulso"};
    REQUIRE(s.kind == PipelineSlotKind::Card);
    REQUIRE(s.ref == "pulso");
    REQUIRE(s == PipelineSlot{PipelineSlotKind::Card, "pulso"});
    REQUIRE(s != PipelineSlot{PipelineSlotKind::Modifier, "pulso"});
}

TEST_CASE("combat_records: ComboRecipe guarda assinatura e mult",
          "[domain][combat][records]") {
    const ComboRecipe r{
        /*combo_id=*/"pulso_stream",
        /*display_name=*/"COMBO_PULSO_STREAM",
        /*signature=*/{PipelineSlot{PipelineSlotKind::Card, "pulso"},
                       PipelineSlot{PipelineSlotKind::Modifier, "stream"}},
        /*result_status=*/std::nullopt,
        /*mult_combo=*/1.5f,
        /*discoverable=*/true};
    REQUIRE(r.combo_id == "pulso_stream");
    REQUIRE(r.signature.size() == 2);
    REQUIRE(r.mult_combo == 1.5f);
    REQUIRE(r.discoverable == true);
}

TEST_CASE("combat_records: CombatLogEntry guarda campos",
          "[domain][combat][records]") {
    const CombatLogEntry e{
        /*actor_id=*/"gus",
        /*action=*/CombatActionType::Attack,
        /*target_id=*/"e",
        /*value=*/12,
        /*message=*/"hit"};
    REQUIRE(e.actor_id == "gus");
    REQUIRE(e.action == CombatActionType::Attack);
    REQUIRE(e.target_id == "e");
    REQUIRE(e.value == 12);
    REQUIRE(e.message == "hit");
}

TEST_CASE("combat_records: StatusEffectChange guarda campos",
          "[domain][combat][records]") {
    const StatusEffectChange c{
        /*actor_id=*/"gus",
        /*id=*/StatusId::Shield,
        /*kind=*/StatusChangeKind::Absorbed,
        /*magnitude=*/4,
        /*duration=*/0};
    REQUIRE(c.actor_id == "gus");
    REQUIRE(c.id == StatusId::Shield);
    REQUIRE(c.kind == StatusChangeKind::Absorbed);
    REQUIRE(c.magnitude == 4);
}

// ---- CombatResult (secao 3/16): defaults canonicos --------------------------------

TEST_CASE("combat_records: CombatResult default e Ongoing, log vazio, 0 rounds",
          "[domain][combat][records]") {
    const CombatResult r{};
    REQUIRE(r.outcome == CombatOutcome::Ongoing);
    REQUIRE(r.log.empty());
    REQUIRE(r.rounds_elapsed == 0);
}

// ---- Executor techMagic (ADR-016, MVP step 1): defaults do Card + EffectSpec ------

TEST_CASE("combat_records: Card default tier/category/effects/flags do executor techMagic",
          "[domain][combat][records][techmagic]") {
    const Card c{};
    REQUIRE(c.tier == CardTier::Comum);
    REQUIRE(c.category == CardCategory::Ativa);
    REQUIRE(c.effects.empty());
    REQUIRE(c.ignores_weakness_wheel == false);
    REQUIRE(c.is_universal_compiler == false);
}

TEST_CASE("combat_records: EffectSpec e value record (igualdade por valor)",
          "[domain][combat][records][techmagic]") {
    const EffectSpec e1{
        /*trigger=*/TriggerHook::OnDamageDealt,
        /*kind=*/EffectKind::Leech,
        /*magnitude=*/5,
        /*percent=*/30,
        /*duration=*/3,
        /*status=*/StatusId::Poison,
        /*stack_rule=*/StackRule::StackMagnitude};

    EffectSpec e2 = e1;
    REQUIRE(e1 == e2);

    EffectSpec e3 = e1;
    e3.magnitude = 6;  // 1 campo difere
    REQUIRE(e1 != e3);
}

TEST_CASE("combat_records: EffectSpec default e OnCast/ApplyStatus com params zerados",
          "[domain][combat][records][techmagic]") {
    const EffectSpec e{};
    REQUIRE(e.trigger == TriggerHook::OnCast);
    REQUIRE(e.kind == EffectKind::ApplyStatus);
    REQUIRE(e.magnitude == 0);
    REQUIRE(e.percent == 0);
    REQUIRE(e.duration == 0);
    REQUIRE(e.status == StatusId::Stun);
    REQUIRE(e.stack_rule == StackRule::Replace);
}

TEST_CASE("combat_records: StatusEffect aceita os 3 StatusId novos do techMagic",
          "[domain][combat][records][techmagic]") {
    const StatusEffect s1{StatusId::SobrecargaTermica, 8, 3, StackRule::Refresh,
                           CardFamily::Eletrico};
    REQUIRE(s1.id == StatusId::SobrecargaTermica);

    const StatusEffect s2{StatusId::Resfriamento, 1, 3, StackRule::Refresh,
                           CardFamily::Universal};
    REQUIRE(s2.id == StatusId::Resfriamento);

    const StatusEffect s3{StatusId::Reflect, 0, 0, StackRule::Replace,
                           CardFamily::Criptografico};
    REQUIRE(s3.id == StatusId::Reflect);
}

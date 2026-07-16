// combat_enums_test.cpp
//
// Spec executavel (Catch2 v3) dos enums canonicos do motor de combate, portados de
// engine/foundation/turn_combat/CombatEnums.cs (origem canonica). POCO puro, ZERO Qt,
// headless. Marco M5 (chunk 1: fundacoes do combate).
//
// O C# nao expoe assert de ordinal (enums implicitos 0..N). Aqui fixamos o contrato
// de ORDINAL porque ele e o contrato binario do serializer futuro e o ponto de
// religacao com templates/card_family.hpp (CardFamily DEVE bater 1:1; a religacao em
// si e o item A1, chunk 4). Travar os ordinais agora torna a religacao futura trivial
// e detecta drift cedo.
//
// Cross-ref: engine/foundation/turn_combat/CombatEnums.cs;
//            docs/design/mecanicas/combat.md secao 3/6/7/8/9; ADR-006.

#include <catch2/catch_test_macros.hpp>

#include <cstdint>

#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/templates/card_family.hpp"

using namespace gus::domain::combat;

namespace {

template <typename E>
constexpr std::uint32_t ord(E e) {
    return static_cast<std::uint32_t>(e);
}

}  // namespace

// ---- CardFamily: fonte canonica + paridade com a copia local de templates --------

TEST_CASE("combat_enums: CardFamily ordinais espelham CombatEnums.cs",
          "[domain][combat][enums][card_family]") {
    REQUIRE(ord(CardFamily::Eletrico) == 0u);
    REQUIRE(ord(CardFamily::Bioquimico) == 1u);
    REQUIRE(ord(CardFamily::Sonico) == 2u);
    REQUIRE(ord(CardFamily::Cinetico) == 3u);
    REQUIRE(ord(CardFamily::Criptografico) == 4u);
}

// PS-R1 (decisao do criador 2026-07-14, docs/design/mecanicas/combat.md secao 20):
// Universal e append-only (ordinal 5, apos os 5 da roda). NAO reordena {0..4}: contrato
// binario do serializer .gdt intocado. So-cartas: templates (personagem/inimigo)
// continuam rejeitando este ordinal (ver character_template_test.cpp / kWheelFamilyCount).
TEST_CASE("combat_enums: CardFamily::Universal e ordinal 5 (append-only, PS-R1)",
          "[domain][combat][enums][card_family]") {
    REQUIRE(ord(CardFamily::Universal) == 5u);
}

TEST_CASE("combat_enums: CardFamily canonico bate 1:1 com templates::CardFamily",
          "[domain][combat][enums][card_family]") {
    // Contrato de religacao (A1, chunk 4): mesma ordem, mesmos ordinais. Se este
    // teste falhar, a copia em templates/ divergiu e a religacao NAO e trivial.
    REQUIRE(ord(CardFamily::Eletrico) ==
            ord(gus::domain::templates::CardFamily::Eletrico));
    REQUIRE(ord(CardFamily::Bioquimico) ==
            ord(gus::domain::templates::CardFamily::Bioquimico));
    REQUIRE(ord(CardFamily::Sonico) ==
            ord(gus::domain::templates::CardFamily::Sonico));
    REQUIRE(ord(CardFamily::Cinetico) ==
            ord(gus::domain::templates::CardFamily::Cinetico));
    REQUIRE(ord(CardFamily::Criptografico) ==
            ord(gus::domain::templates::CardFamily::Criptografico));
}

// ---- CardBaseType (secao 7) -------------------------------------------------------

TEST_CASE("combat_enums: CardBaseType ordinais", "[domain][combat][enums]") {
    REQUIRE(ord(CardBaseType::Pulso) == 0u);
    REQUIRE(ord(CardBaseType::Raiz) == 1u);
    REQUIRE(ord(CardBaseType::Eco) == 2u);
    REQUIRE(ord(CardBaseType::Fenda) == 3u);
    REQUIRE(ord(CardBaseType::Glifo) == 4u);
}

// ---- TargetShape (secao 7) --------------------------------------------------------

TEST_CASE("combat_enums: TargetShape ordinais", "[domain][combat][enums]") {
    REQUIRE(ord(TargetShape::Self) == 0u);
    REQUIRE(ord(TargetShape::Single) == 1u);
    REQUIRE(ord(TargetShape::Linha) == 2u);
    REQUIRE(ord(TargetShape::Area3x3) == 3u);
    REQUIRE(ord(TargetShape::Grupo) == 4u);
}

// ---- CardModifier (secao 8) -------------------------------------------------------

TEST_CASE("combat_enums: CardModifier ordinais", "[domain][combat][enums]") {
    REQUIRE(ord(CardModifier::Object) == 0u);
    REQUIRE(ord(CardModifier::Stream) == 1u);
    REQUIRE(ord(CardModifier::Null) == 2u);
}

// ---- StackRule (secao 9) ----------------------------------------------------------

TEST_CASE("combat_enums: StackRule ordinais", "[domain][combat][enums]") {
    REQUIRE(ord(StackRule::Replace) == 0u);
    REQUIRE(ord(StackRule::Refresh) == 1u);
    REQUIRE(ord(StackRule::StackMagnitude) == 2u);
    REQUIRE(ord(StackRule::StackDuration) == 3u);
}

// ---- StatusId (secao 9) -----------------------------------------------------------

TEST_CASE("combat_enums: StatusId ordinais", "[domain][combat][enums]") {
    REQUIRE(ord(StatusId::Stun) == 0u);
    REQUIRE(ord(StatusId::Poison) == 1u);
    REQUIRE(ord(StatusId::Corrode) == 2u);
    REQUIRE(ord(StatusId::Disrupt) == 3u);
    REQUIRE(ord(StatusId::Silence) == 4u);
    REQUIRE(ord(StatusId::Knockback) == 5u);
    REQUIRE(ord(StatusId::Break) == 6u);
    REQUIRE(ord(StatusId::Expose) == 7u);
    REQUIRE(ord(StatusId::Decrypt) == 8u);
    REQUIRE(ord(StatusId::Shield) == 9u);
    REQUIRE(ord(StatusId::Regen) == 10u);
    REQUIRE(ord(StatusId::Haste) == 11u);
    REQUIRE(ord(StatusId::Slow) == 12u);
}

// ---- WeaknessTier (secao 6) -------------------------------------------------------

TEST_CASE("combat_enums: WeaknessTier ordinais", "[domain][combat][enums]") {
    REQUIRE(ord(WeaknessTier::Fraco) == 0u);
    REQUIRE(ord(WeaknessTier::Neutro) == 1u);
    REQUIRE(ord(WeaknessTier::Resistente) == 2u);
    REQUIRE(ord(WeaknessTier::Imune) == 3u);
}

// ---- CombatActionType (secao 3/5) -------------------------------------------------

TEST_CASE("combat_enums: CombatActionType ordinais", "[domain][combat][enums]") {
    REQUIRE(ord(CombatActionType::Attack) == 0u);
    REQUIRE(ord(CombatActionType::Defend) == 1u);
    REQUIRE(ord(CombatActionType::UseCard) == 2u);
    REQUIRE(ord(CombatActionType::Flee) == 3u);
    REQUIRE(ord(CombatActionType::Scan) == 4u);
    REQUIRE(ord(CombatActionType::ScanEnvironment) == 5u);
    REQUIRE(ord(CombatActionType::GambitPredict) == 6u);
    REQUIRE(ord(CombatActionType::GambitReorder) == 7u);
    REQUIRE(ord(CombatActionType::Pass) == 8u);
}

// COMBATE-FILA-CURSOR-FIX (decisao do lider 2026-07-15): StatusTick e append-only (ordinal
// 9, apos Pass). NAO reordena {0..8}: contrato binario do serializer futuro intocado.
TEST_CASE("combat_enums: CombatActionType::StatusTick e ordinal 9 (append-only)",
          "[domain][combat][enums]") {
    REQUIRE(ord(CombatActionType::StatusTick) == 9u);
}

// ---- CombatPhase (secao 3) --------------------------------------------------------

TEST_CASE("combat_enums: CombatPhase ordinais", "[domain][combat][enums]") {
    REQUIRE(ord(CombatPhase::SetupPhase) == 0u);
    REQUIRE(ord(CombatPhase::TurnStart) == 1u);
    REQUIRE(ord(CombatPhase::ActionSelect) == 2u);
    REQUIRE(ord(CombatPhase::ActionResolve) == 3u);
    REQUIRE(ord(CombatPhase::TurnEnd) == 4u);
    REQUIRE(ord(CombatPhase::CheckEnd) == 5u);
    REQUIRE(ord(CombatPhase::CombatEnd) == 6u);
}

// ---- CombatOutcome (secao 3) ------------------------------------------------------

TEST_CASE("combat_enums: CombatOutcome ordinais", "[domain][combat][enums]") {
    REQUIRE(ord(CombatOutcome::Ongoing) == 0u);
    REQUIRE(ord(CombatOutcome::Victory) == 1u);
    REQUIRE(ord(CombatOutcome::Defeat) == 2u);
    REQUIRE(ord(CombatOutcome::Fled) == 3u);
}

// ---- PipelineSlotKind (secao 10) --------------------------------------------------

TEST_CASE("combat_enums: PipelineSlotKind ordinais", "[domain][combat][enums]") {
    REQUIRE(ord(PipelineSlotKind::Card) == 0u);
    REQUIRE(ord(PipelineSlotKind::Modifier) == 1u);
}

// ---- StatusChangeKind (secao 16) --------------------------------------------------

TEST_CASE("combat_enums: StatusChangeKind ordinais", "[domain][combat][enums]") {
    REQUIRE(ord(StatusChangeKind::Applied) == 0u);
    REQUIRE(ord(StatusChangeKind::Expired) == 1u);
    REQUIRE(ord(StatusChangeKind::Absorbed) == 2u);
}

// ---- Executor techMagic (ADR-016, MVP step 1) -------------------------------------

TEST_CASE("combat_enums: StatusId 0..12 intocados (guarda anti-reordenacao)",
          "[domain][combat][enums][techmagic]") {
    REQUIRE(ord(StatusId::Stun) == 0u);
    REQUIRE(ord(StatusId::Poison) == 1u);
    REQUIRE(ord(StatusId::Corrode) == 2u);
    REQUIRE(ord(StatusId::Disrupt) == 3u);
    REQUIRE(ord(StatusId::Silence) == 4u);
    REQUIRE(ord(StatusId::Knockback) == 5u);
    REQUIRE(ord(StatusId::Break) == 6u);
    REQUIRE(ord(StatusId::Expose) == 7u);
    REQUIRE(ord(StatusId::Decrypt) == 8u);
    REQUIRE(ord(StatusId::Shield) == 9u);
    REQUIRE(ord(StatusId::Regen) == 10u);
    REQUIRE(ord(StatusId::Haste) == 11u);
    REQUIRE(ord(StatusId::Slow) == 12u);
}

TEST_CASE("combat_enums: StatusId novos do executor techMagic (append-only 13..15)",
          "[domain][combat][enums][techmagic]") {
    REQUIRE(ord(StatusId::SobrecargaTermica) == 13u);
    REQUIRE(ord(StatusId::Resfriamento) == 14u);
    REQUIRE(ord(StatusId::Reflect) == 15u);
}

// ADR-016 Balde B (Faraday/EM-Shield, decisao do lider 2026-07-15): BlindagemEM e
// append-only (ordinal 16, apos Reflect). NAO reordena {0..15}.
TEST_CASE("combat_enums: StatusId::BlindagemEM e ordinal 16 (append-only)",
          "[domain][combat][enums][techmagic]") {
    REQUIRE(ord(StatusId::BlindagemEM) == 16u);
}

// CARD-ENGINE-MANIFESTO item 8 (CloneAlly, von Neumann/Fork + Giordano Bruno/Echo-Self,
// ultimo step do manifesto): Eco e append-only (ordinal 19, apos Scrying=18). NAO reordena
// {0..18}: contrato binario do serializer futuro intocado.
TEST_CASE("combat_enums: StatusId::Eco e ordinal 19 (append-only)",
          "[domain][combat][enums][techmagic]") {
    REQUIRE(ord(StatusId::NullProof) == 17u);
    REQUIRE(ord(StatusId::Scrying) == 18u);
    REQUIRE(ord(StatusId::Eco) == 19u);
}

TEST_CASE("combat_enums: SideFilter ordinais", "[domain][combat][enums][techmagic]") {
    REQUIRE(ord(SideFilter::Any) == 0u);
    REQUIRE(ord(SideFilter::EnemyOnly) == 1u);
    REQUIRE(ord(SideFilter::AllyOnly) == 2u);
}

TEST_CASE("combat_enums: CardTier ordinais", "[domain][combat][enums][techmagic]") {
    REQUIRE(ord(CardTier::Comum) == 0u);
    REQUIRE(ord(CardTier::Especial) == 1u);
    REQUIRE(ord(CardTier::Super) == 2u);
}

TEST_CASE("combat_enums: CardCategory ordinais", "[domain][combat][enums][techmagic]") {
    REQUIRE(ord(CardCategory::Ativa) == 0u);
    REQUIRE(ord(CardCategory::Passiva) == 1u);
    REQUIRE(ord(CardCategory::ForaDeCombate) == 2u);
    REQUIRE(ord(CardCategory::Hibrida) == 3u);
}

TEST_CASE("combat_enums: TriggerHook ordinais", "[domain][combat][enums][techmagic]") {
    REQUIRE(ord(TriggerHook::OnCast) == 0u);
    REQUIRE(ord(TriggerHook::OnDamageDealt) == 1u);
    REQUIRE(ord(TriggerHook::OnDamageReceived) == 2u);
    REQUIRE(ord(TriggerHook::OnAllyTurnEnd) == 3u);
    REQUIRE(ord(TriggerHook::OnRoundEnd) == 4u);
    REQUIRE(ord(TriggerHook::Always) == 5u);
}

TEST_CASE("combat_enums: EffectKind ordinais", "[domain][combat][enums][techmagic]") {
    REQUIRE(ord(EffectKind::ApplyStatus) == 0u);
    REQUIRE(ord(EffectKind::Leech) == 1u);
    REQUIRE(ord(EffectKind::Reflect) == 2u);
    REQUIRE(ord(EffectKind::HypotenuseCombo) == 3u);
    REQUIRE(ord(EffectKind::CloneAlly) == 4u);
}

// PS-Y-EXECUTOR-5 (decisao do lider 2026-07-14, ADR-016 secao 20 item 5): RepeatLastAction
// e append-only (ordinal 5, apos CloneAlly). NAO reordena {0..4}: contrato binario do
// serializer futuro intocado.
TEST_CASE("combat_enums: EffectKind::RepeatLastAction e ordinal 5 (append-only)",
          "[domain][combat][enums][techmagic]") {
    REQUIRE(ord(EffectKind::RepeatLastAction) == 5u);
}

// CARD-ENGINE-MANIFESTO item 9 (Mises/Calc-Edge, decisao do lider 2026-07-16):
// ApEfficiency e append-only (ordinal 11, apos DiversityBonus). NAO reordena {0..10}:
// contrato binario do serializer futuro intocado.
TEST_CASE("combat_enums: EffectKind::ApEfficiency e ordinal 11 (append-only)",
          "[domain][combat][enums][techmagic]") {
    REQUIRE(ord(EffectKind::ApEfficiency) == 11u);
}

// CARD-ENGINE-MANIFESTO item 8 (von Neumann/Fork, passiva "Construtor Universal", ultimo
// step do manifesto): TokenRefund e append-only (ordinal 12, apos ApEfficiency). NAO
// reordena {0..11}: contrato binario do serializer futuro intocado.
TEST_CASE("combat_enums: EffectKind::TokenRefund e ordinal 12 (append-only)",
          "[domain][combat][enums][techmagic]") {
    REQUIRE(ord(EffectKind::TokenRefund) == 12u);
}

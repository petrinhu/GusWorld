// gus/domain/combat/techmagic.cpp
//
// Implementacao do executor techMagic (ADR-016, MVP step 2). Handlers deste step:
// ApplyStatus, Leech, Reflect. EffectKind sem handler (HypotenuseCombo/CloneAlly) lanca
// std::logic_error - fail-fast, nunca no-op silencioso (ver techmagic.hpp).
//
// Cross-ref: gus/domain/combat/techmagic.hpp; combat_state_machine.cpp (pontos de hook);
//            docs/design/mecanicas/cartas-technomagik.md; ADR-016.

#include "gus/domain/combat/techmagic.hpp"

#include <cmath>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

namespace gus::domain::combat::techMagic {

namespace {

// Empurra uma CombatLogEntry pro log da FSM (secao 16), se `ctx.log` estiver setado.
// actor_id = ctx.caster (dono do programa); `target` opcional vira o TargetId da entrada.
void log_entry(TechMagicContext& ctx, CombatActor* target, int value, std::string message) {
    if (ctx.log == nullptr) return;
    ctx.log->push_back(CombatLogEntry{
        ctx.caster != nullptr ? ctx.caster->id() : std::string{}, CombatActionType::UseCard,
        target != nullptr ? std::optional<std::string>(target->id()) : std::nullopt, value,
        std::move(message)});
}

// ApplyStatus: monta um StatusEffect a partir do EffectSpec + a familia da CARTA (nao do
// caster - mesmo racional de card.status_applied no resolvedor base, secao 9) e aplica.
// Destinatario depende do hook: OnCast -> ctx.counterpart (alvo da carta); Always ->
// ctx.caster (o proprio dono, reforco no TurnStart enquanto a especial estiver equipada).
void handle_apply_status(const EffectSpec& spec, const Card& card, TriggerHook hook,
                         TechMagicContext& ctx) {
    if (ctx.caster == nullptr)
        throw std::logic_error("techMagic::ApplyStatus: ctx.caster nao pode ser nulo.");

    CombatActor* recipient = hook == TriggerHook::Always ? ctx.caster : ctx.counterpart;
    if (recipient == nullptr)
        throw std::logic_error(
            "techMagic::ApplyStatus: alvo ausente pro hook (ctx.counterpart nulo).");

    const StatusEffect status{spec.status, spec.magnitude, spec.duration, spec.stack_rule,
                              card.family};
    recipient->add_status(status);

    log_entry(ctx, recipient, spec.magnitude,
             (ctx.caster != nullptr ? ctx.caster->id() : std::string{}) + " tavus-executa " +
                 card.id + " em " + recipient->id() + ": status aplicado (mag " +
                 std::to_string(spec.magnitude) + ", dur " + std::to_string(spec.duration) +
                 ").");
}

// Leech (OnDamageDealt): caster recupera lround(ctx.damage * percent/100) em HP E mana
// (split igual = placeholder de engine; o split real do Volta e o dado da carta, pendente
// VOLTA-LEECH-%). Sem efeito se ctx.damage <= 0 (canal FALHA/imunidade nao dispara).
void handle_leech(const EffectSpec& spec, const Card& card, TechMagicContext& ctx) {
    if (ctx.caster == nullptr)
        throw std::logic_error("techMagic::Leech: ctx.caster nao pode ser nulo.");
    if (ctx.damage <= 0) return;

    const int amount = static_cast<int>(
        std::lround(static_cast<double>(ctx.damage) * static_cast<double>(spec.percent) / 100.0));
    if (amount <= 0) return;

    ctx.caster->heal(amount);          // clamp em max_hp (CombatActor::heal).
    ctx.caster->restore_mana(amount);  // clamp em max_mana (CombatActor::restore_mana).

    log_entry(ctx, ctx.counterpart, amount,
             ctx.caster->id() + " drena " + std::to_string(amount) + " HP/mana via " + card.id +
                 " (Leech).");
}

// Reflect (OnDamageReceived): ctx.counterpart (o atacante original) sofre
// lround(ctx.damage * percent/100) de volta. take_damage PURO - NAO reentra pelo helper
// compartilhado de dano da FSM, entao nunca redispara OnDamageDealt/OnDamageReceived
// (guarda anti-recursao, secao 20).
void handle_reflect(const EffectSpec& spec, const Card& card, TechMagicContext& ctx) {
    if (ctx.caster == nullptr)
        throw std::logic_error("techMagic::Reflect: ctx.caster nao pode ser nulo.");
    if (ctx.counterpart == nullptr)
        throw std::logic_error(
            "techMagic::Reflect: ctx.counterpart (atacante) nao pode ser nulo.");
    if (ctx.damage <= 0) return;

    const int amount = static_cast<int>(
        std::lround(static_cast<double>(ctx.damage) * static_cast<double>(spec.percent) / 100.0));
    if (amount <= 0) return;

    ctx.counterpart->take_damage(amount);

    log_entry(ctx, ctx.counterpart, amount,
             ctx.caster->id() + " reflete " + std::to_string(amount) + " de volta em " +
                 ctx.counterpart->id() + " via " + card.id + " (Reflect).");
}

}  // namespace

void execute(TriggerHook hook, const Card& card, TechMagicContext& ctx) {
    for (const EffectSpec& spec : card.effects) {
        if (spec.trigger != hook) continue;

        switch (spec.kind) {
            case EffectKind::ApplyStatus:
                handle_apply_status(spec, card, hook, ctx);
                break;
            case EffectKind::Leech:
                handle_leech(spec, card, ctx);
                break;
            case EffectKind::Reflect:
                handle_reflect(spec, card, ctx);
                break;
            case EffectKind::HypotenuseCombo:
            case EffectKind::CloneAlly:
            default:
                throw std::logic_error(
                    "techMagic: EffectKind sem handler implementado na carta '" + card.id +
                    "' (step 2 cobre so ApplyStatus/Leech/Reflect; HypotenuseCombo/CloneAlly "
                    "sao step 3+, ADR-016).");
        }
    }
}

void execute_equipped(TriggerHook hook, CombatActor& owner,
                      const std::unordered_map<std::string, Card>* registry,
                      TechMagicContext& ctx) {
    ctx.caster = &owner;
    for (const std::string& id : owner.equipped_special_ids()) {
        if (registry == nullptr)
            throw std::out_of_range("techMagic: '" + owner.id() + "' tem especial equipada '" +
                                    id +
                                    "' mas nenhum card_registry foi injetado na "
                                    "CombatStateMachine.");
        const auto it = registry->find(id);
        if (it == registry->end())
            throw std::out_of_range("techMagic: especial equipada '" + id + "' de '" +
                                    owner.id() + "' nao esta no registry de combate.");
        execute(hook, it->second, ctx);
    }
}

}  // namespace gus::domain::combat::techMagic

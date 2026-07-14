// gus/domain/combat/techmagic.hpp
//
// Executor techMagic (ADR-016, MVP step 2): a carta especial/super e um PROGRAMA (lista
// ordenada de EffectSpec, ver combat_records.hpp) que o Tavus-Drive executa. Este header
// declara o dispatcher `execute` (roda os EffectSpec de UMA carta cujo trigger == hook) e
// `execute_equipped` (roda o mesmo hook nas especiais EQUIPADAS de um ator, resolvidas
// contra o card_registry ja existente na FSM). POCO puro, ZERO Qt (invariante de domain/,
// engine-design.md secao 2).
//
// FAIL-FAST (regra do executor, secao 20/ADR-016): EffectKind sem handler implementado
// lanca std::logic_error. NUNCA no-op silencioso - um EffectSpec declarado numa carta que
// o executor nao sabe rodar e um BUG, nao um efeito vazio.
//
// ANTI-RECURSAO (Reflect): os handlers aplicam dano refletido via CombatActor::take_damage
// PURO (sem passar pelo helper compartilhado de dano da FSM), entao nunca redisparam
// OnDamageDealt/OnDamageReceived. Ver techmagic.cpp.
//
// ESCOPO deste step (NAO implementar aqui): CloneAlly, HypotenuseCombo/combo cross-ator,
// OnAllyTurnEnd/OnRoundEnd (so DECLARADOS no hook, sem ponto de disparo), ledger de hits,
// tick da Sobrecarga Termica, desconto do Resfriamento, query fora-de-combate. Step 3+.
//
// Cross-ref: gus/domain/combat/combat_records.hpp (EffectSpec/Card); combat_enums.hpp
//            (TriggerHook/EffectKind/CardTier/CardCategory); combat_state_machine.cpp
//            (pontos de hook OnCast/OnDamageDealt/OnDamageReceived/Always);
//            docs/design/mecanicas/cartas-technomagik.md; ADR-016.

#ifndef GUS_DOMAIN_COMBAT_TECHMAGIC_HPP
#define GUS_DOMAIN_COMBAT_TECHMAGIC_HPP

#include <string>
#include <unordered_map>
#include <vector>

#include "gus/domain/combat/combat_actor.hpp"
#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/combat/combat_records.hpp"

namespace gus::domain::combat::techMagic {

// Contexto de execucao de um EffectSpec. `caster` e o DONO do programa em execucao (quem
// jogou a carta, OU quem tem a especial equipada); `counterpart` e o outro lado do evento,
// com semantica POR HOOK:
//   OnCast            -> counterpart = alvo da carta (o ApplyStatus recai sobre ele).
//   OnDamageDealt      -> caster = quem CAUSOU o dano; counterpart = quem SOFREU (Leech
//                         cura/restaura mana do caster).
//   OnDamageReceived   -> caster = quem SOFREU o dano (dono da passiva, ex. Reflect);
//                         counterpart = quem CAUSOU (recebe o dano refletido).
//   Always             -> counterpart nao usado (nullptr); o efeito recai sobre o proprio
//                         caster (ex. ApplyStatus refresca no dono a cada TurnStart).
// `damage` = valor do evento de dano (Leech/Reflect leem daqui); 0 para OnCast/Always.
// `log` acumula as CombatLogEntry geradas (mesmo vetor log_ da CombatStateMachine, secao
// 16); nullptr desativa o log (ex. chamada isolada em teste unitario).
struct TechMagicContext {
    CombatActor* caster = nullptr;
    CombatActor* counterpart = nullptr;
    int damage = 0;
    std::vector<CombatLogEntry>* log = nullptr;
};

// Executa, NA ORDEM declarada, os EffectSpec de `card` cujo `trigger == hook`. `ctx.caster`
// deve estar setado ANTES da chamada (dono do programa); os handlers assumem isso e lancam
// std::logic_error se estiver nulo. EffectKind sem handler implementado (HypotenuseCombo,
// CloneAlly neste step) lanca std::logic_error - fail-fast, nunca no-op silencioso.
void execute(TriggerHook hook, const Card& card, TechMagicContext& ctx);

// Roda `hook` nas especiais EQUIPADAS (Passiva/Hibrida) de `owner`, resolvidas por id
// contra `registry` (mesmo card_registry injetado na CombatStateMachine). Seta
// `ctx.caster = &owner` antes de despachar cada carta equipada. Lanca std::out_of_range se
// `owner` tiver uma especial equipada e `registry` for nullptr, ou se o id equipado nao for
// encontrado em `registry` - mesmo padrao fail-fast do lookup de carta em resolve_use_card.
// No-op se `owner.equipped_special_ids()` estiver vazio (comportamento atual preservado).
void execute_equipped(TriggerHook hook, CombatActor& owner,
                      const std::unordered_map<std::string, Card>* registry,
                      TechMagicContext& ctx);

}  // namespace gus::domain::combat::techMagic

#endif  // GUS_DOMAIN_COMBAT_TECHMAGIC_HPP

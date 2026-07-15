// gus/domain/combat/techmagic.hpp
//
// Executor techMagic (ADR-016, MVP step 2-3): a carta especial/super e um PROGRAMA (lista
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
// ANTI-RECURSAO (Reflect/HypotenuseCombo): os handlers aplicam dano via
// CombatActor::take_damage PURO (sem passar pelo helper compartilhado de dano da FSM),
// entao nunca redisparam OnDamageDealt/OnDamageReceived. Ver techmagic.cpp.
//
// LEDGER CROSS-ATOR (step 3, HypotenuseCombo/OnRoundEnd): `apply_damage_with_hooks` da
// FSM registra cada hit (attacker/target/damage) num RoundHitEntry, DEPOIS do guard
// damage<=0 (FALHA/imunidade nunca entra no ledger). Ticks de DoT (Poison/Corrode, fora
// desse helper compartilhado) TAMBEM ficam fora do ledger - so hits via
// apply_damage_with_hooks contam pro combo. Na fronteira de rodada (wrap da fila), a FSM
// despacha OnRoundEnd nas especiais equipadas de cada ator VIVO com `ctx.round_hits`
// apontando pro ledger da rodada, e limpa o ledger em seguida (hits nao atravessam
// fronteira de rodada).
//
// STEP 5 (RepeatLastAction, Mandelbrot+Ada, ADR-016; decisoes Q1-Q4 do lider,
// 2026-07-14): OnAllyTurnEnd agora tem ponto de disparo real (CombatStateMachine::
// process_ally_turn_end_hooks, chamado nos dois caminhos de fim-de-turno). O handler
// reaplica o dano>0 da ULTIMA ACAO de dano de QUALQUER aliado NESTA RODADA
// (LastActionRecord, abaixo), escalado por EffectSpec.percent, via CombatActor::
// take_damage PURO (mesma anti-recursao de Reflect/HypotenuseCombo).
//
// ESCOPO ainda FORA deste step: CloneAlly, tick da Sobrecarga Termica, desconto do
// Resfriamento, query fora-de-combate. Step 6+.
//
// Cross-ref: gus/domain/combat/combat_records.hpp (EffectSpec/Card); combat_enums.hpp
//            (TriggerHook/EffectKind/CardTier/CardCategory); combat_state_machine.cpp
//            (pontos de hook OnCast/OnDamageDealt/OnDamageReceived/Always/OnRoundEnd);
//            docs/design/mecanicas/cartas-technomagik.md; ADR-016.

#ifndef GUS_DOMAIN_COMBAT_TECHMAGIC_HPP
#define GUS_DOMAIN_COMBAT_TECHMAGIC_HPP

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "gus/domain/combat/combat_actor.hpp"
#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/combat/combat_records.hpp"
#include "gus/domain/combat/random_source.hpp"

namespace gus::domain::combat::techMagic {

// Um hit de dano DESTA rodada (ledger cross-ator, ADR-016 secao 20 item 4). `attacker` e
// `target` sao ponteiros NAO-DONOS (mesmo padrao de CombatActor* na FSM/InitiativeQueue);
// `damage` e o valor efetivamente aplicado (sempre > 0 - o guard fica no call site,
// combat_state_machine.cpp). Consultado pelo handler de HypotenuseCombo em OnRoundEnd.
struct RoundHitEntry {
    CombatActor* attacker = nullptr;
    CombatActor* target = nullptr;
    int damage = 0;
};

// Registro da ULTIMA ACAO DE DANO de QUALQUER aliado NESTA RODADA (step 5,
// RepeatLastAction/Mandelbrot+Ada; decisoes Q1-Q4 do lider, 2026-07-14). Ponteiros
// NAO-DONOS (mesmo padrao de RoundHitEntry). `actor` = quem executou a acao (Attack ou
// UseCard); `card_id` vazio para ataque basico (Attack nao tem carta). `hits` agrega o
// dano>0 causado a CADA ALVO DISTINTO por essa acao (Q2: so ataque basico ou carta
// ofensiva que causou dano>0 grava aqui - Scan/Defend/status puro/canal FALHA/imunidade
// NUNCA gravam). A FSM (last_action_) so ATUALIZA este registro ao FIM de
// resolve_basic_attack/resolve_use_card, e so quando ha pelo menos 1 hit>0 (uma acao sem
// dano preserva o registro anterior, ainda valido na rodada). Zerado (actor=nullptr,
// hits vazio) na fronteira de rodada, junto do round_hits_ (Q4 - a janela nao atravessa
// rodada). `actor == nullptr` (ou hits vazio) e ESTADO NORMAL (nada a ecoar ainda nesta
// rodada), nao bug.
struct LastActionRecord {
    CombatActor* actor = nullptr;
    CombatActionType type = CombatActionType::Attack;
    std::string card_id;
    std::vector<std::pair<CombatActor*, int>> hits;
};

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
//   OnRoundEnd         -> counterpart nao usado (nullptr); o alvo do combo e derivado de
//                         `round_hits` (HypotenuseCombo), nao de counterpart.
// `damage` = valor do evento de dano (Leech/Reflect leem daqui); 0 para OnCast/Always/
// OnRoundEnd.
// `log` acumula as CombatLogEntry geradas (mesmo vetor log_ da CombatStateMachine, secao
// 16); nullptr desativa o log (ex. chamada isolada em teste unitario).
// `round_hits` (step 3): ledger de hits DESTA rodada, so usado por OnRoundEnd/
// HypotenuseCombo. Campo ADITIVO (default nullptr preserva os call sites/testes dos steps
// 1-2 intactos); handle_hypotenuse_combo lanca std::logic_error se rodar com round_hits ==
// nullptr (OnRoundEnd sem ledger injetado e bug de call site, nao efeito vazio).
// `bonused_targets` (step 3, opcional): set de alvos que JA receberam o bonus Hipotenusa
// NESTA rodada (dedup 1x/alvo/rodada mesmo que >1 dono da passiva feche o combo no mesmo
// alvo). Vive no escopo do CALLER (CombatStateMachine::process_round_end_hooks) - um set
// por chamada de OnRoundEnd (= por rodada), reaproveitado entre os atores iterados nessa
// chamada. nullptr desativa o dedup (ex. teste unitario isolado que so quer o valor bruto).
// `last_action` (step 5, RepeatLastAction/Mandelbrot+Ada): ponteiro pro LastActionRecord
// da FSM (last_action_), so usado por handle_repeat_last_action. Campo ADITIVO (default
// nullptr preserva os call sites/testes dos steps 1-4 intactos); handle_repeat_last_action
// lanca std::logic_error se rodar com last_action == nullptr (bug de call site - a FSM
// SEMPRE injeta &last_action_ nos hooks OnCast/OnAllyTurnEnd, mesmo padrao de round_hits
// acima). Um LastActionRecord com actor == nullptr (ou hits vazio, ou acao de lado OPOSTO
// ao ctx.caster) e ESTADO NORMAL (nada a ecoar ainda nesta rodada) - NAO lanca, so
// no-op+log.
// `rng` (step 5): porta de RNG injetada, so consumida por handle_repeat_last_action quando
// a carta declara EffectSpec.magnitude > 0 (chance do Re-Run, ex. Ada 34%). nullptr e bug
// de call site SE a carta em execucao tiver magnitude>0 (a FSM sempre injeta rng_); cartas
// com magnitude==0 (Mandelbrot, sempre-repete) NUNCA tocam este ponteiro - 0 consumo de
// RNG por construcao (determinismo, secao 11/ADR-006).
struct TechMagicContext {
    CombatActor* caster = nullptr;
    CombatActor* counterpart = nullptr;
    int damage = 0;
    std::vector<CombatLogEntry>* log = nullptr;
    const std::vector<RoundHitEntry>* round_hits = nullptr;
    std::unordered_set<CombatActor*>* bonused_targets = nullptr;
    const LastActionRecord* last_action = nullptr;
    IRandomSource* rng = nullptr;
};

// Executa, NA ORDEM declarada, os EffectSpec de `card` cujo `trigger == hook`. `ctx.caster`
// deve estar setado ANTES da chamada (dono do programa); os handlers assumem isso e lancam
// std::logic_error se estiver nulo. EffectKind sem handler implementado (CloneAlly neste
// step) lanca std::logic_error - fail-fast, nunca no-op silencioso.
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

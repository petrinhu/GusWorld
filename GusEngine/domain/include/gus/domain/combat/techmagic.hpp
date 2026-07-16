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
// MANIFESTO item 8 (CloneAlly, von Neumann/Fork + Giordano Bruno/Echo-Self, ultimo step do
// CARD-ENGINE-MANIFESTO): status-based (StatusId::Eco, ordinal 19), NAO entidade nova na
// fila (decisao do lider 2026-07-14 superada pela decisao 2026-07-16 - Party=3 fixo
// intacto; ver AMB-11 em docs/design/roster-analogos/_EFEITOS-ESCOLHIDOS.md). OnCast ->
// CloneAlly (handle_clone_ally) aplica o Eco (buff, add_status LEGADO) no ALIADO alvo
// (self incluso). Ao FIM de CADA turno PROPRIO do portador (2 sitios de fim-de-turno,
// mesmo par de RepeatLastAction/OnAllyTurnEnd - ver CombatStateMachine::
// process_eco_turn_end_hook), `replicate_eco` (funcao PUBLICA nova, abaixo, MESMO padrao
// de dump_reveal_intent) reaplica os hits>0 da ULTIMA ACAO DE DANO do PROPRIO portador
// nesta rodada (last_action_.actor == portador), escalado por StatusId::Eco.magnitude, via
// take_damage PURO. TokenRefund (ordinal 12, passiva "Construtor Universal" do von
// Neumann) e MARCADOR fora do dispatcher (handle_token_refund no-op) - o refund 1x/batalha
// pluga DIRETO no gate specials_cast_ de resolve_use_card.
//
// STEP 8 (RevealIntent, John Dee/Black-Mirror, manifesto item 6; decisoes D1-D4 do lider,
// 2026-07-15): status-based (StatusId::Scrying), NAO carta-equipada - `execute_equipped`
// NUNCA despacha isto (execute_equipped so roda hooks de cartas Passiva/Hibrida
// EQUIPADAS). O dump/re-dump vive em 2 funcoes PUBLICAS novas (abaixo, expostas fora do
// dispatcher `execute`): `log_intent_for` (1 alvo) e `dump_reveal_intent` (todos os
// inimigos vivos do lado oposto ao caster). handle_reveal_intent (OnCast, a carta jogada)
// chama dump_reveal_intent DEPOIS de aplicar o buff Scrying; CombatStateMachine::
// process_scrying_hooks chama dump_reveal_intent de novo na FRONTEIRA DE RODADA, 1x por
// aliado vivo que ainda porta Scrying; CombatStateMachine::resolve_scan chama
// log_intent_for (1 alvo) quando o scanner porta a carta (Scan aprimorado, D1-ii). FAIL-
// SOFT no brain ausente nos 3 sitios (assimetria deliberada vs o Gambito manual, que
// lanca) - ver doc-comment de log_intent_for.
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

namespace gus::domain::combat {
class InitiativeQueue;
class IEnemyBrain;
}  // namespace gus::domain::combat

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

// Um registro de ACAO (nao de HIT) desta rodada (CARD-ENGINE-MANIFESTO item 7, Hayek/
// Free-Order, EffectKind::DiversityBonus; AMB-09). Granularidade de ACAO
// (CombatStateMachine::resolve_action), NAO de golpe individual: diferente de
// RoundHitEntry (que nasce em apply_damage_with_hooks, 1 entrada por HIT>0 contra um alvo),
// este ledger ganha EXATAMENTE 1 entrada por ACAO despachada por resolve_action -
// independente de quantos alvos/hits ela produza (AoE conta 1x, nao N vezes) e mesmo pra
// acoes SEM dano (Defend/Scan/GambitPredict/etc. tambem contam como "assinaturas" pra fins
// de diversidade). Ecos de dano PURO que usam CombatActor::take_damage DIRETO (Mandelbrot/
// Ada/RepeatLastAction, HypotenuseCombo/OnRoundEnd) NUNCA passam por resolve_action -
// ficam FORA deste ledger por construcao (nao sao "acoes" no sentido de ActionSelect).
// `family` so e significativo quando `type == CombatActionType::UseCard` (M2, AMB-09): a
// assinatura de UseCard e REFINADA pela familia da CARTA jogada (2 cartas da MESMA familia
// = mesma assinatura; familias diferentes = assinaturas distintas); nos demais tipos de
// acao fica no default (Eletrico) e NAO participa da comparacao de assinatura (so o `type`
// importa - ver combat_state_machine.cpp namespace anonimo, helper de igualdade de
// assinatura). Ponteiro NAO-DONO (mesmo padrao de RoundHitEntry). Zerado na MESMA fronteira
// de rodada que round_hits_/last_action_ (CombatStateMachine::process_round_end_hooks).
struct RoundActionEntry {
    CombatActor* actor = nullptr;
    CombatActionType type = CombatActionType::Pass;
    CardFamily family = CardFamily::Eletrico;
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
// `combatants` (step 6, ChainDamage/Tesla): roster COMPLETO do combate (ordem da fila),
// so consumido por handle_chain_damage pra achar os alvos SECUNDARIOS dos saltos (proximos
// inimigos vivos do lado oposto, excluindo o alvo primario). Campo ADITIVO (default nullptr
// preserva os call sites/testes dos steps 1-5 intactos); handle_chain_damage lanca
// std::logic_error se rodar com combatants == nullptr (bug de call site - a FSM SEMPRE injeta
// &queue_.order() no OnCast, mesmo padrao de round_hits/last_action acima). Ponteiro
// NAO-DONO (mesmo padrao de CombatActor* na FSM/InitiativeQueue).
// `queue` (step 7, DelayAction/Einstein): ponteiro pra InitiativeQueue da FSM (fila de
// iniciativa real, nao so o snapshot de `combatants`), so consumido por
// handle_delay_action pra reordenar o alvo via InitiativeQueue::reorder_actor (mesma
// primitiva do Gambito-Reordenar). Campo ADITIVO (default nullptr preserva os call sites/
// testes dos steps 1-6 intactos); handle_delay_action lanca std::logic_error se rodar com
// queue == nullptr (bug de call site - a FSM SEMPRE injeta &queue_ no OnCast de uma carta
// com DelayAction). Ponteiro NAO-DONO. So forward-declarado aqui (nao inclui
// initiative_queue.hpp) pra nao empurrar essa dependencia pros consumidores do header que
// nao precisam dela.
// `brain_registry` (step 8, RevealIntent/John Dee-Black-Mirror): mesmo registry id->
// IEnemyBrain* ja injetado na CombatStateMachine (brain_registry_), so consumido por
// dump_reveal_intent/log_intent_for pra achar o IEnemyBrain de cada inimigo (mesmo lookup
// de resolve_gambit_predict). Campo ADITIVO (default nullptr preserva os call sites/testes
// dos steps 1-7 intactos). Ausencia de brain (registry nullptr OU id nao encontrado) e
// FAIL-SOFT aqui (loga "sem sinal" e segue) - ao contrario do Gambito manual
// (resolve_gambit_predict), que lanca std::out_of_range: o dump roda AUTOMATICO num hook de
// rodada/scan, nao pode derrubar o combate por 1 inimigo sem brain registrado. So
// forward-declarado (IEnemyBrain), nao inclui enemy_brain.hpp aqui.
struct TechMagicContext {
    CombatActor* caster = nullptr;
    CombatActor* counterpart = nullptr;
    int damage = 0;
    std::vector<CombatLogEntry>* log = nullptr;
    const std::vector<RoundHitEntry>* round_hits = nullptr;
    std::unordered_set<CombatActor*>* bonused_targets = nullptr;
    const LastActionRecord* last_action = nullptr;
    IRandomSource* rng = nullptr;
    const std::vector<CombatActor*>* combatants = nullptr;
    InitiativeQueue* queue = nullptr;
    const std::unordered_map<std::string, IEnemyBrain*>* brain_registry = nullptr;
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

// Scrying/Black-Mirror (John Dee, step 8, EffectKind::RevealIntent): loga 1 linha de
// IntentPreview de `target`, do ponto de vista de `ctx.caster` (o "vidente"). FAIL-SOFT no
// brain ausente (ctx.brain_registry nulo OU id nao encontrado): loga "sem sinal" e retorna,
// NUNCA lanca - assimetria DELIBERADA vs o Gambito manual (resolve_gambit_predict, que
// lanca std::out_of_range): este helper roda em contextos AUTOMATICOS (dump em massa,
// re-dump de rodada) ou como bonus OPCIONAL de outra acao (Scan aprimorado), nenhum dos
// dois pode derrubar o combate por causa de 1 inimigo sem brain registrado. Intent CAOTICO
// (D2, decisao do lider - Patch-Zero): loga ruido, nunca revela os campos previstos -
// preserva a one-way door do boss. Read-only: NAO muta nada (monta um CombatState local via
// ctx.queue quando ha brain a consultar, mesmo padrao de DelayAction), NAO consome RNG
// (preview_intent nao sorteia, contrato de IEnemyBrain). Lanca std::logic_error se
// ctx.caster for nulo (guard mais externo, sempre). ctx.queue so e EXIGIDO (lanca se nulo)
// quando um brain de fato foi ENCONTRADO pra `target` - sem brain (fail-soft acima), nao
// ha CombatState pra montar, entao queue ausente NAO e bug nesse ramo. Usado por
// dump_reveal_intent (abaixo) E por CombatStateMachine::resolve_scan (Scan aprimorado, 1
// alvo, quando o scanner porta a carta - ver combat_state_machine.cpp).
void log_intent_for(CombatActor& target, TechMagicContext& ctx);

// Scrying/Black-Mirror (John Dee, step 8): dump em massa - roda log_intent_for pra CADA
// inimigo VIVO do lado OPOSTO a ctx.caster, na ordem de ctx.combatants (mesmo guard de lado
// do ChainDamage/handle_chain_damage). Zero inimigos vivos: no-op logado (dissipacao), NAO
// silencioso (regra todo-efeito-loga). Lanca std::logic_error se ctx.caster ou
// ctx.combatants forem nulos (bug de call site). Chamado por handle_reveal_intent (OnCast,
// a carta jogada) E por CombatStateMachine::process_scrying_hooks (re-dump automatico na
// FRONTEIRA DE RODADA, 1x por aliado vivo que porta StatusId::Scrying - NAO via
// execute_equipped, Scrying e status, nao carta equipada).
void dump_reveal_intent(TechMagicContext& ctx);

// Eco/Molde-Fiel (CloneAlly, von Neumann/Fork + Giordano Bruno/Echo-Self; CARD-ENGINE-
// MANIFESTO item 8): reaplica os hits>0 da ULTIMA ACAO DE DANO do PROPRIO `ctx.caster`
// (o portador do status Eco) NESTA RODADA (ctx.last_action), escalado por `percent`% (o
// StatusEffect.magnitude do Eco, lido pelo CALLER - esta funcao NAO consulta
// status_effects() sozinha, mesma separacao de responsabilidade de replicate_eco vs.
// CombatStateMachine::process_eco_turn_end_hook, que localiza o status e extrai a
// magnitude ANTES de chamar), via CombatActor::take_damage PURO - MESMA anti-recursao/
// anti-inflacao de handle_repeat_last_action (NAO toca ctx.last_action/round_hits_, entao
// nao pode compor com Ada/HypotenuseCombo/RepeatLastAction pra dobrar dano; o TESTE-REI de
// composicao Ada+Eco fica em techmagic_clone_test.cpp). DIFERENTE de RepeatLastAction (que
// ecoa a ultima acao de QUALQUER aliado): exige `last_action->actor == ctx.caster`
// EXATAMENTE - durante o turno PROPRIO do portador ninguem mais age, entao o registro de 1
// slot ja identifica a acao DELE mesmo, sem estrutura nova. Sem acao-de-dano propria nesta
// rodada = "eco ocioso" (log, ESTADO NORMAL, nao erro). Zero consumo de RNG
// (determinismo). Lanca std::logic_error se ctx.caster ou ctx.last_action forem nulos (bug
// de call site - CombatStateMachine::process_eco_turn_end_hook SEMPRE injeta os dois antes
// de chamar, so quando `ended` de fato porta StatusId::Eco).
void replicate_eco(int percent, TechMagicContext& ctx);

}  // namespace gus::domain::combat::techMagic

#endif  // GUS_DOMAIN_COMBAT_TECHMAGIC_HPP

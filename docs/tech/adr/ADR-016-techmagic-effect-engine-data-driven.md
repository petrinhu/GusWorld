# ADR-016: Motor de efeitos das cartas especiais = executor DATA-DRIVEN `techMagic` (nao VM)

Status: Accepted (decisao do lider supremo via AskUserQuestion 2026-07-14; recomendacao do Caetano/CTO)
Data: 2026-07-14
Decisores: lider supremo (petrus), Caetano/CTO (analise + recomendacao), software-architect e backend-engineer/gameplay_engineer (execucao)

Cross-ref: `docs/design/mecanicas/cartas-technomagik.md` (taxonomia: COMUM/ESPECIAL/SUPER, 4 sub-categorias §2.3, status novos §5), `docs/design/roster-analogos/_EFEITOS-ESCOLHIDOS.md` (os 20 efeitos escolhidos), `docs/design/techmagic.md` (conceito techMagic, item TECHMAGIC-EXECUTOR que este ADR resolve), `GusEngine/domain/include/gus/domain/combat/combat_records.hpp` (struct `Card` atual), `GusEngine/domain/src/combat/combat_state_machine.cpp` (`resolve_use_card`), `GusEngine/domain/src/templates/canonical_templates.cpp` (padrao de catalogo data-driven), memorias `project_sistema_cartas_technomagik`, `project_techmagic_canon`, `feedback_glintfx_nao_mexer_so_pedir`, `project_rmlui_ui_stack`.

## Contexto

O `Card` atual so modela carta COMUM (family/base_type/mana/ap/power/target_shape + 1 status opcional + modifiers Object/Stream/Null). Falta a camada ESPECIAL (20 cartas dos mestres) e o motor de efeitos que ela exige: 4 sub-categorias (ATIVA 1x/batalha, PASSIVA equip-only sempre-ligada, FORA-DE-COMBATE overworld, HIBRIDA), efeitos diversos (leech %, reflect, combo cross-ator, clone-entidade-com-visual, revelar area, trunfo-ignora-roda), 2 status novos (Sobrecarga termica, Resfriamento) e flags de trunfo (`IgnoresWeaknessWheel`, `IsUniversalCompiler`). Os statlines finais das 20 estao DIFERIDOS pra playtest (os efeitos foram escolhidos, os numeros nao) - logo o motor precisa ser data-first.

O item TECHMAGIC-EXECUTOR (`techmagic.md`) nomeou um fork: (a) resolvedor data-driven vs (b) interpretador/VM de conjuro (bytecode). O contexto de valor e o easter-egg canonico "magia = software" (o executor techMagic ↔ Tavus-Drive do Gus).

## Decisao

**Motor de efeitos = executor DATA-DRIVEN chamado `techMagic`.** Cada carta carrega uma lista ordenada de instrucoes declarativas `EffectSpec = { gatilho (TriggerHook: OnCast/OnDamageDealt/OnDamageReceived/OnAllyTurnEnd/OnRoundEnd/Always), tipo (EffectKind: enum fechado ~20-25), parametros numericos tunaveis }`, resolvidas por um dispatcher `techMagic::execute(card, contexto)` na cadeia de `resolve_use_card`.

**A VM (opcao b) foi RECUSADA** (analise do Caetano, anti-OE pra G1): o vocabulario de efeitos e FECHADO e conhecido (20 cartas curadas, sem craft/modding/UGC em G1), entao nao existe o caso de uso que justifica interpretador ("conteudo novo sem recompilar"). E a VM NAO elimina nenhum trabalho: a dificuldade real esta nas PRIMITIVAS de engine (hook de dano, agregacao cross-ator, entidade-clone, queries de overworld), que sao C++ de qualquer jeito; a VM so adiciona indirecao (bytecode + compilador + debug de 2 camadas + serializacao/versionamento), custo estimado 3-5x antes da 1a carta, risco ALTO pra dev solo, zero ganho jogavel.

**O easter-egg fica LITERAL no codigo hoje, risco zero:** o executor chama-se `techMagic`, a carta E um programa declarativo (lista ordenada de instrucoes que o Tavus-Drive "executa"). **Porta aberta pra VM futura sem reescrita:** a lista `EffectSpec` e exatamente a IR que um compilador de conjuros emitiria; se v2 tiver consumidor real (editor de conjuros, modding), a VM nasce POR CIMA, sem tocar as cartas nem o resolvedor.

**Insight estrutural:** efeitos nao sao "coisas no cast", sao REACOES A EVENTOS (recebi dano -> reflete; 2 aliados batem no mesmo alvo -> hipotenusa) - por isso o modelo e (gatilho, tipo, params). As FORA-DE-COMBATE (Faraday/Euler/Menger) nao sao "efeito executavel": sao uma QUERY de posse ("a party tem cardExec-faraday?") que os sistemas de overworld consultam.

**Regra de VFX (ordem do lider 2026-07-14):** todo efeito VISUAL que o glintfx possa servir (screen-wave do Einstein, glow de carta, sprite-eco do clone) vai como PROMPT pro dev do glintfx, NUNCA hardcoded no nosso GL/render (`feedback_glintfx_nao_mexer_so_pedir`, `project_rmlui_ui_stack`). O dominio/gameplay dispara o efeito; a apresentacao e da lib.

## MVP (ordem minima pra destravar PS-Y1 hibrida + PS-Y2 primitivas)

1. **Records/enums** (backend-engineer): `CardTier` (Comum/Especial/Super) + sub-categoria; flags `ignores_weakness_wheel`/`is_universal_compiler`; `std::vector<EffectSpec>` no `Card` (comuns = vetor vazio, intocadas); `StatusId::{SobrecargaTermica, Resfriamento, Reflect}` (numeros de §5 ja fechados). `CardFamily::Universal` ja entregue (ADR/PS-R1, commit e594d07).
2. **Executor `techMagic` + 3 hooks** (gameplay_engineer): OnCast, OnDamageDealt (leech Volta), OnDamageReceived (reflect Newton) na cadeia de `resolve_use_card`; regra 1x/batalha reusando o flag da Analise Preditiva. -> uma ATIVA, uma PASSIVA e uma HIBRIDA de ponta a ponta = **PS-Y1 fecha**.
3. **Ledger cross-ator + hook OnRoundEnd** (Pythagoras): hits por alvo/round no estado de combate. -> **PS-Y2 fecha**.
4. **Catalogo das especiais** no padrao `canonical_templates` (dados tunaveis num arquivo so; playtest N=3 mexe num lugar).
5. **Por demanda do slice:** clone entidade-Objeto (slot visual = camada app + prompt glintfx), hooks de turno (Ada/Hayek/Mises), query de posse pras fora-de-combate (nao bloqueia o combate).

## Decisoes de design ainda pendentes do lider (nao bloqueiam o MVP)

1. Grafia canonica do simbolo: `techMagic` vs `TechnoMagik` (item TECHMAGIC-CANON) - o batismo do easter-egg e do lider.
2. VOLTA-LEECH-% (fracao de conversao) - bloqueia o statline do Volta, nao o motor.
3. Sequenciamento overworld: Faraday/Euler/Menger/Maxwell agem em sistemas ainda nao codados (mini-mapa/iluminacao/valuation) - recomendacao: stub por query agora, efeito real quando o sistema chegar.
4. Clone-visual (sprite-eco): qual visual + slot na battle-screen (prompt glintfx).
5. VFX Einstein (screen-wave): prompt pro glintfx antes, ou fallback barato.
6. Tag "comando central" (Mises) na taxonomia de `EnemyTemplate`: agora ou quando o bestiario fechar.

## Addendum (MVP step 5, 2026-07-14): RepeatLastAction (Mandelbrot/Fractal-Echo + Ada/Re-Run)

Sexto `EffectKind` entregue (append-only, ordinal 5): `RepeatLastAction`, o eco Mandelbrot
(OnCast, sempre dispara, 0 consumo de RNG) + Ada (OnAllyTurnEnd, chance 34%, percent 100%).
Decisoes do lider (Q1-Q4): eco do RESULTADO (reaplica o dano ja causado, na % da carta,
via `take_damage` puro, sem novo sorteio/critico/mana/status); so acao que causou dano>0
grava o registro (`LastActionRecord`); Ada ecoa a 100% (o freio dela e a chance); a janela
= ultima acao de dano de QUALQUER aliado NESTA RODADA, zerada na fronteira de rodada
junto do ledger `round_hits_` do HypotenuseCombo.

Isso exigiu ligar `OnAllyTurnEnd` a um ponto de disparo real (estava so DECLARADO desde o
step 2): `CombatStateMachine::process_ally_turn_end_hooks`, chamado nos dois caminhos de
fim-de-turno (`run_active_turn_to_end` e `expire_on_stunned_turn_end`), despachando pros
aliados VIVOS do mesmo lado, excluindo quem fechou o turno. `TechMagicContext` ganhou 2
campos aditivos (`last_action`, `rng`) com default `nullptr`, preservando os call sites
dos steps 1-4. Catalogo `MasterCards::build_registry()` passa de 8 para 10 cartas.

Cross-ref: `GusEngine/domain/include/gus/domain/combat/techmagic.hpp` (LastActionRecord);
`GusEngine/domain/src/combat/combat_state_machine.cpp` (gravacao + despacho);
`GusEngine/domain/src/combat/master_cards.cpp` (mandelbrot/ada);
`docs/design/mecanicas/cartas-technomagik.md` §9; `docs/design/roster-analogos/
_EFEITOS-ESCOLHIDOS.md` (AMB-01, "neste turno" -> "nesta rodada").

## Addendum (MVP step 6): ChainDamage (Tesla)

Setimo `EffectKind` entregue (append-only, ordinal 6): `ChainDamage`, a descarga em cadeia
da Tesla. `OnCast`: depois que o dano-base da carta atinge o alvo primario (no loop base do
`resolve_use_card`), a descarga SALTA pros proximos inimigos VIVOS na ordem da fila, do lado
OPOSTO ao caster, EXCLUINDO o primario. `EffectSpec.magnitude` = numero de saltos (Tesla = 2
-> 3 alvos no total); `EffectSpec.percent` = retencao por salto (62%). Decaimento
MULTIPLICATIVO: salto k (1-indexado) = `lround(danoPrimario * (percent/100)^k)`; para quando
faltam alvos vivos OU quando o salto arredonda pra `<=0`. O `danoPrimario` e o dano
REALMENTE causado ao primario (pos-fraqueza/crit/variancia); primario imune (dano 0) =>
no-op. Dano dos saltos vai por `CombatActor::take_damage` PURO (igual a Reflect/Leech/
RepeatLastAction): nao redispara `OnDamageDealt/OnDamageReceived`, nao entra no ledger
`round_hits_`. 0 consumo de RNG (a cadeia nao sorteia nada).

Wiring no `OnCast` de `resolve_use_card`: alem de `last_action`/`rng` (step 5), o contexto
agora injeta `combatants = &queue_.order()` (roster pra achar os alvos-salto) e `damage` = o
dano causado a ESTE `target` no loop base (varredura do vetor `hits` local, ainda intacto
antes de ser movido pro `last_action_`). `TechMagicContext` ganhou 1 campo aditivo
(`combatants`, default `nullptr`): `handle_chain_damage` lanca `std::logic_error` se rodar
com `combatants`/`counterpart`/`caster` nulos (fail-fast, bug de call site). EXCECAO de
catalogo: a Tesla e a unica especial que precisa de `power > 0` (a cadeia escala do dano-base
do primario, nao de um `EffectSpec`); `make_special` ganhou um parametro `power` opcional
(default 0 preserva as 10 anteriores). Catalogo passa de 10 para 11 cartas (familia Eletrico
ganha a Tesla).

Cross-ref: `GusEngine/domain/include/gus/domain/combat/techmagic.hpp`
(TechMagicContext.combatants); `GusEngine/domain/src/combat/techmagic.cpp`
(handle_chain_damage); `GusEngine/domain/src/combat/combat_state_machine.cpp` (wiring OnCast);
`GusEngine/domain/src/combat/master_cards.cpp` (tesla); `docs/design/mecanicas/
cartas-technomagik.md`.

## Addendum (MVP step 7): DelayAction (Einstein/Time-Dilate)

Oitavo `EffectKind` entregue (append-only, ordinal 7): `DelayAction`, a dilatacao temporal
da Einstein. `OnCast`: empurra a acao do alvo (`ctx.counterpart`) pro FIM da fila da rodada
corrente (`EffectSpec.magnitude == 0`, o caso da Einstein) ou N posicoes fixas
(`magnitude > 0`, generico), via a MESMA primitiva do Gambito-Reordenar
(`InitiativeQueue::reorder_actor`; o clamp nos limites da fila e dela). Sem dano - o
handler nunca toca `take_damage`/`round_hits`/`last_action`, 0 consumo de RNG.

**Regra de dissipacao (decisao do lider 2026-07-15, ver AMB-02 em
`_EFEITOS-ESCOLHIDOS.md`):** um alvo que JA AGIU nesta rodada (indice dele em
`queue->order()` menor que `queue->cursor()`) faz a carta DISSIPAR - no-op + log, NAO
banca pra proxima rodada. Outros estados NORMAIS que tambem viram no-op + log (nao lancam):
alvo morto, alvo fora da fila, ou alvo sendo o `current()` (em acao agora, sem turno futuro
nesta rodada pra adiar).

**Persistencia da reordenacao (contrato D3):** o empurrao e uma PERMUTACAO na fila, nao um
estado separado - fica valido ate a proxima `InitiativeQueue::recompute_by_speed()` (a
mesma primitiva que ja desfaz reordenacoes do Gambito quando um status muda SPD, ex.
Haste/Slow expira). Isso significa que o empurrao do Einstein pode "sumir" se algo
recomputar a fila por SPD antes do alvo chegar no fim - comportamento intencional, mesma
regra do Gambito, nao um bug.

Wiring no `OnCast` de `resolve_use_card`: `TechMagicContext` ganhou 1 campo aditivo
(`queue`, ponteiro `InitiativeQueue*`, default `nullptr`), injetado como `&queue_` (a fila
REAL da FSM, nao so o snapshot `combatants` de `ChainDamage`) - `handle_delay_action` lanca
`std::logic_error` se rodar com `queue`/`counterpart`/`caster` nulos (fail-fast, bug de call
site). Catalogo `MasterCards::build_registry()` passa de 11 para 12 cartas (a Einstein entra
na familia Cinetico, nao Universal - decisao do criador: a dilatacao temporal e a mesma
familia mecanica das cartas COMUNS de reordenar/knockback).

Cross-ref: `GusEngine/domain/include/gus/domain/combat/techmagic.hpp`
(TechMagicContext.queue); `GusEngine/domain/include/gus/domain/combat/initiative_queue.hpp`
(reorder_actor/cursor/current/recompute_by_speed); `GusEngine/domain/src/combat/techmagic.cpp`
(handle_delay_action); `GusEngine/domain/src/combat/combat_state_machine.cpp` (wiring OnCast);
`GusEngine/domain/src/combat/master_cards.cpp` (einstein); `docs/design/mecanicas/
cartas-technomagik.md`; `docs/design/roster-analogos/_EFEITOS-ESCOLHIDOS.md` (AMB-02).

## Consequencias

- **Positivas:** custo BAIXO-MEDIO, risco BAIXO; numeros 100% tunaveis pra playtest; testavel por unit test por EffectKind; easter-egg entregue hoje; nao fecha porta pra VM. Cada carta = 5-15 linhas de dado.
- **Negativas/custo irredutivel:** cada efeito exotico exige um EffectKind + handler C++ novo (mas esse custo existe em QUALQUER opcao - e das primitivas, nao do envelope). O enum EffectKind cresce a ~20-25 valores.
- **Impacto:** desbloqueia PS-Y1 (HIBRIDA) e PS-Y2 (Reflect/cross-ator/clone). Supersede a ambiguidade do fork em TECHMAGIC-EXECUTOR.

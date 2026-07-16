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

### Addendum ao addendum (modo-aliado assimetrico, decisao do lider 2026-07-15)

`handle_delay_action` ramifica por LADO do alvo (`ctx.counterpart->is_player_side() ==
ctx.caster->is_player_side()`), decisao do lider 2026-07-15: o comportamento pro alvo
INIMIGO (empurra pro fim da fila) fica INALTERADO; alvo ALIADO ganha o espelho beneficio -
avanca pra agir MAIS CEDO, indo pro PRIMEIRO SLOT AINDA-NAO-AGIDO logo apos o ator atual
(`cursor()+1`), em vez do fim da fila. `EffectSpec.magnitude` mantem a mesma semantica
generica dos dois lados (0 = "ao extremo" - fim da fila pro inimigo, `cursor()+1` pro
aliado; >0 = N posicoes fixas, atrasando pro inimigo / adiantando pro aliado). A carta
`einstein` (catalogo, `master_cards.cpp`) NAO muda - continua com `magnitude=0`; a
ramificacao por lado vive so no HANDLER.

**Invariante anti turno-duplo no ramo aliado:** o alvo NUNCA pode ser reordenado pra um
indice <= `cursor()` - isso desincronizaria `current()` do ator cuja resolucao esta
realmente em andamento (mesma classe de bug do guard preexistente "alvo e o current()").
Como o alvo aliado so chega no ramo de reordenacao apos passar pelo guard "indice <
cursor()" (ja agiu -> dissipa), ele sempre comeca em indice >= `cursor()+1`; o handler
clampa qualquer overshoot de N-posicoes (magnitude > 0 grande) pra nao ultrapassar
`cursor()+1` pra tras - `InitiativeQueue::reorder_actor` por si so so clampa nos limites
`[0, count-1]` da fila inteira, nao no cursor, entao esse clamp adicional e necessario pro
ramo aliado (o ramo inimigo nao precisa dele: empurrar pro fim so pode ir alem, nunca
aquem, do cursor).

Regras de dissipacao (alvo morto/fora da fila, alvo e o `current()`, alvo ja agiu nesta
rodada) sao IDENTICAS nos dois lados - o guard compartilhado roda ANTES da ramificacao por
lado. Sem dano, 0% de RNG nos dois ramos, `take_damage`/`round_hits`/`last_action`
intocados.

Cross-ref: `GusEngine/domain/include/gus/domain/combat/techmagic.hpp`
(TechMagicContext.queue); `GusEngine/domain/include/gus/domain/combat/initiative_queue.hpp`
(reorder_actor/cursor/current/recompute_by_speed); `GusEngine/domain/src/combat/techmagic.cpp`
(handle_delay_action); `GusEngine/domain/src/combat/combat_state_machine.cpp` (wiring OnCast);
`GusEngine/domain/src/combat/master_cards.cpp` (einstein);
`GusEngine/domain/tests/techmagic_delay_test.cpp` (casos 15-22, ramo aliado);
`docs/design/mecanicas/cartas-technomagik.md`;
`docs/design/roster-analogos/_EFEITOS-ESCOLHIDOS.md` (AMB-02).

## Addendum (Balde B, PR1, decisoes do lider 2026-07-15): Faraday/EM-Shield

Primeiro PR do "Balde B" (efeitos de imunidade/reflexo cross-status, distinto do Balde A de
dano/status ofensivo direto dos steps 5-7 acima). Entrega `StatusId::BlindagemEM` (ordinal
16, append-only apos Reflect) + DUAS primitivas COMPARTILHADAS reusadas pelo PR2 (Newton):

- **`SideFilter` (`combat_enums.hpp`):** `Any` / `EnemyOnly` / `AllyOnly`, campo novo
  `EffectSpec.side_filter` (ADITIVO ao fim do struct, default `Any` preserva toda carta
  existente). Filtro de lado DATA-DRIVEN checado em `handle_apply_status`
  (`techmagic.cpp`): quando o `recipient` resolvido esta do lado que o filtro EXCLUI, a
  carta DISSIPA (no-op + log, nao lanca - mesmo racional de `handle_delay_action`).
- **`CombatActor::try_add_status` (portao de imunidade, choke point unico):** substitui
  `add_status` como o caminho CANONICO de aplicar status OFENSIVO. Retorna
  `StatusApplyResult{Applied, BlockedByImmunity}`. Regras (decisoes do lider, F-1..F-4):
  - **F-1:** bloqueia SO quando o ator tem `BlindagemEM` ativa E o status entrante e um
    **debuff** (`!CombatActor::is_buff(id)`) de `family_origin == CardFamily::Eletrico`.
    Buffs eletricos e debuffs de outras familias passam direto - `try_add_status` cai no
    caminho normal de `insert_or_stack_status` (mesma logica de StackRule de `add_status`).
  - **Bloqueio nao registra `Applied`:** o guard roda ANTES de inserir/registrar no buffer
    de mudancas de status (secao 16) - um bloqueio NUNCA aparece como "status aplicado" no
    log/UI/replay.
  - **F-2 "previne + limpa":** quando o status INSERIDO E o proprio `BlindagemEM`, apos
    inserir, `try_add_status` chama `clear_electric_debuffs()` - remove (via
    `remove_status`, que ja reverte deltas de stat) TODOS os debuffs de `family_origin ==
    Eletrico` JA presentes no alvo (exceto a propria `BlindagemEM`). Nao so previne os
    futuros, tambem limpa o que ja estava ativo.
  - `add_status` (legado) vira WRAPPER de `try_add_status` que ignora o retorno -
    preserva os call sites de BUFF/defesa (`resolve_defend`/Shield etc) sem qualquer
    mudanca de comportamento.
- **"O dominó":** o portao SO protege quem passa por `try_add_status`. Os OUTROS 4 sitios
  de status OFENSIVO do motor - `handle_apply_status` (executor techMagic) + os 4 sitios
  de `card.status_applied`/`combo->result_status` em
  `combat_state_machine.cpp::resolve_use_card` (curto-circuito de imunidade E resolucao
  normal, x2 campos) - precisaram trocar `add_status` -> `try_add_status` explicitamente.
  Sem isso, o Faraday bloquearia so a carta ESPECIAL mas um Stun de carta COMUM eletrica
  ou um combo passaria batido. Os 4 sitios de `resolve_use_card` foram unificados num
  helper privado `CombatStateMachine::apply_offensive_status` (choke point + log honesto:
  `Applied` -> "status aplicado"; `BlockedByImmunity` -> "bloqueado pela blindagem EM" -
  nunca a mensagem generica de sucesso). Sitios de BUFF/defesa (Shield do Defend, Always
  das passivas equipadas) **NAO** foram tocados - continuam com `add_status` legado.
- **F-3, Faraday vira Hibrida:** `master_cards.cpp` - `category: ForaDeCombate ->
  Hibrida`, `mana_cost: 0 -> kActiveManaCost`, `effects = [{OnCast, ApplyStatus,
  duration=3, status=BlindagemEM, stack_rule=Refresh, side_filter=AllyOnly}]`. Sujeita ao
  MESMO gate 1x/batalha das outras Ativa/Hibrida (`specials_cast_`). A face
  fora-de-combate (anti-PEM, posse-only, `project_save_dungeon_pem_faraday`) **NAO** foi
  tocada - continua sem programa, feat futura.
- **F-4:** cast em alvo INIMIGO dissipa via `side_filter == AllyOnly` (o guard generico
  acima), nunca lanca.
- **Achado colateral (fix necessario, nao scope creep):** `StatusId::SobrecargaTermica`
  faltava na enumeracao `is_non_buff()` (`combat_actor.cpp`) - por exclusao, isso a
  classificava como BUFF, o que furaria F-1/F-2 (o proprio teste do lider, "Aplicar
  BlindagemEM em alvo COM SobrecargaTermica ativa -> limpa", so passa com o fix). Corrigido
  junto (`is_non_buff` ganhou o case `SobrecargaTermica`); `Resfriamento`/`BlindagemEM`
  continuam corretamente classificados como buff (nao entram na lista).

Cross-ref: `GusEngine/domain/include/gus/domain/combat/combat_enums.hpp` (SideFilter,
StatusId::BlindagemEM); `GusEngine/domain/include/gus/domain/combat/combat_records.hpp`
(EffectSpec.side_filter); `GusEngine/domain/include/gus/domain/combat/combat_actor.hpp`
(StatusApplyResult/try_add_status); `GusEngine/domain/src/combat/combat_actor.cpp`
(try_add_status/blocked_by_em_shield/clear_electric_debuffs/is_non_buff);
`GusEngine/domain/src/combat/techmagic.cpp` (handle_apply_status);
`GusEngine/domain/src/combat/combat_state_machine.cpp` (apply_offensive_status + os 4
sitios do dominó); `GusEngine/domain/src/combat/master_cards.cpp` (faraday);
`GusEngine/domain/tests/techmagic_faraday_test.cpp`;
`docs/design/mecanicas/cartas-technomagik.md` secao 5;
`docs/design/roster-analogos/_EFEITOS-ESCOLHIDOS.md` (AMB-03).

## Addendum (Balde B, PR2, decisoes do lider 2026-07-15): regra geral "fogo amigo desligado" + Newton/Force-Law

Segundo PR do "Balde B", reusa as DUAS primitivas do PR1 (SideFilter/try_add_status) sem
EffectKind nem StatusId novos.

- **Regra geral, fogo amigo DESLIGADO (achado da auditoria, corrigido como regra do motor,
  nao so do Newton):** `resolve_use_card` somava `actor.atk()` no dano-base MESMO quando o
  `target` era do PROPRIO time do `actor` (a formula `(card.power + actor.atk()) * fatores`
  nao checava lado) - isso ja quebrava silenciosamente os modos-aliado PRE-EXISTENTES
  (Einstein/Faraday, PR1 e step 7), que sao pensados como BENEFICIO. Fix: no INICIO do loop
  `for (CombatActor* target : targets)`, um guard `friendly = target->is_player_side() ==
  actor.is_player_side()` zera o dano-base (`apply_damage_with_hooks(actor, *target, 0,
  &card)`, MESMO padrao do ramo imune existente), SEM sortear canal/variancia (0 consumo de
  RNG nesse alvo) e SEM aplicar status OFENSIVO (`card.status_applied`/`combo->
  result_status` sao debuffs de INIMIGO). O loop `continue` pro proximo alvo; os
  `EffectSpec` `OnCast` da carta (que rodam DEPOIS deste loop, sobre os MESMOS `targets`)
  ficam intactos - o modo-aliado dos EffectSpec (Reflect-status `AllyOnly` do Newton, etc.)
  continua funcionando normalmente, so o dano-de-ATK bruto e que nao vaza mais pro aliado.
- **N-1, Poco Gravitacional vira AoE:** `master_cards.cpp::newton` ganha `target_shape =
  Grupo` (era `Single`). `OnCast -> ApplyStatus Stun` ganha `side_filter = EnemyOnly` -
  imobiliza (dur 1) TODOS os inimigos vivos do grupo; o dano-de-ATK acompanha o mesmo grupo
  (o `power` da carta segue 0, so o ATK do conjurador conta na formula-base).
- **`resolve_targets(TargetShape::Grupo)` consertado (dominó independente, achado durante o
  PR2):** a versao anterior hardcodava `!is_player_side()` (SEMPRE os atores nao-player,
  isto e, sempre "os inimigos da party") - se um INIMIGO castasse uma carta Grupo, o alvo
  virava o PROPRIO time dele (o mesmo bug de lado que `handle_chain_damage`/Tesla ja evitava
  corretamente com `a->is_player_side() == ctx.caster->is_player_side()`, mas que faltava
  aqui). Fix: alvos = todos os vivos do lado OPOSTO ao `actor` que joga a carta (nao mais
  hardcoded pro lado do jogador).
- **N-3, modo-aliado assimetrico (mesmo padrao ja usado em Einstein/AMB-02 e Faraday/
  AMB-03):** dentro de `resolve_targets(Grupo)`, se o alvo DECLARADO (`action.target_id`)
  resolve pra um ator do MESMO lado do `actor`, o retorno vira single (so ESSE 1 aliado),
  nao o grupo inteiro - a carta virou um cast MIRADO num beneficiario, nao um AoE. Newton
  ganha um segundo `EffectSpec` `OnCast -> ApplyStatus`, `status = StatusId::Reflect`
  (ordinal 15, ja existente - reusado como STATUS aplicavel, nao so como
  `EffectKind::Reflect`), `magnitude = 30` (percent), `duration = 3`, `stack_rule =
  Refresh`, `side_filter = AllyOnly`: concede Reflect 30%/3 turnos ao aliado alvo. Em
  inimigo, dissipa (`side_filter` cuida disso, mesmo racional de F-4).
- **N-4, Reflect-por-STATUS honrado em `apply_damage_with_hooks`:** alem da passiva-propria
  EQUIPADA (`OnDamageReceived -> Reflect`, ja existente, INTOCADA), `apply_damage_with_hooks`
  ganhou uma checagem SEPARADA, DEPOIS do guard `damage <= 0` e DEPOIS do despacho
  `OnDamageReceived`: se `target.status_effects()` tem `StatusId::Reflect`, o `attacker`
  sofre `lround(damage * status.magnitude / 100.0)` via `CombatActor::take_damage` PURO
  (MESMA anti-recursao do Reflect equipado - nunca reentra no helper, nunca redispara
  `OnDamageDealt`/`OnDamageReceived`). Como as duas fontes disparam em PONTOS DIFERENTES do
  metodo (sem dedup), se o mesmo ator tiver AMBAS (passiva equipada + Reflect-status), elas
  SOMAM de graca (30%+30% = 60% no caso de referencia) - decisao do criador, nao um efeito
  colateral acidental. Nenhuma das duas fontes entra em `round_hits_`/`last_action_` (eco de
  dano, nao um "hit" novo pro combo/RepeatLastAction - mesmo racional do Reflect equipado).

Cross-ref: `GusEngine/domain/src/combat/combat_state_machine.cpp` (guard de fogo amigo no
loop de `resolve_use_card`; `resolve_targets` Grupo; Reflect-por-status em
`apply_damage_with_hooks`); `GusEngine/domain/src/combat/master_cards.cpp` (newton, 3
`EffectSpec` + `target_shape = Grupo`); `GusEngine/domain/tests/techmagic_newton_test.cpp`;
`GusEngine/domain/tests/combat_state_machine_test.cpp` (regressao Einstein/Faraday-em-aliado,
tag `[friendlyfire]`); `docs/design/mecanicas/cartas-technomagik.md` secao 9;
`docs/design/roster-analogos/_EFEITOS-ESCOLHIDOS.md` (AMB-04).

## Addendum (Balde B, PR3, decisoes do lider 2026-07-15): Godel/Null-Proof

Terceiro e ULTIMO PR do "Balde B". Diferente de PR1/PR2 (que so ADICIONAM primitivas
compartilhadas), este PR3 muta o coracao do resolvedor de dano (`resolve_use_card`) - PR
separado por decisao do criador, capricho de paridade com o preview.

- **G-1, Godel vira castavel mana-0:** `master_cards.cpp::godel` passa de `Passiva`/
  `effects` vazio (so o trunfo `ignores_weakness_wheel=true`) pra `Ativa`, mana 0, com
  `effects = [OnCast -> ApplyStatus NullProof, side_filter AllyOnly]` - concede o status ao
  PORTADOR (proprio Godel ou um aliado, `AllyOnly` inclui self). 1x/batalha via
  `specials_cast_` (mesmo gate de Volta/Newton/Faraday, secao 2.1). `StatusId::NullProof`
  (ordinal 17, append-only apos `BlindagemEM`).
- **`StatusId::NullProof` = buff-trunfo GUARDADO**, nao um debuff-timer: duracao-sentinela
  ALTA (`dur=99`, `//PLAYTEST` - nao expira por turno na pratica do slice). A saida real e
  por CONSUMO (`remove_status` no hit que fura), nao por tick de duracao - sem isso o
  "guardado ate o proximo hit relevante" ficaria refem do relogio de turnos.
- **G-2/G-3, wiring do pierce em `resolve_use_card`:** no loop `for (CombatActor* target :
  targets)`, LOGO APOS calcular `mult_fraqueza` (defesa neutra do compilador universal) e
  ANTES do curto-circuito de imunidade (`if (mult_fraqueza == 0.0f) continue`), DUAS vias
  INDEPENDENTES furam a roda:
  1. **item 11 GENERICO** (o trunfo ORIGINAL de Godel, agora com wiring real): `if
     (card.ignores_weakness_wheel) mult_fraqueza = 1.0f;` - qualquer carta com a flag
     SEMPRE resolve mult 1.0, independente de NullProof de quem quer que seja.
  2. **G-2/G-3** (o trunfo NOVO): senao, `if (mult_fraqueza < 1.0f && actor tem
     StatusId::NullProof) { mult_fraqueza = 1.0f; actor.remove_status(NullProof); }` - o
     ATACANTE porta o status; furou (Resistente 0.66 OU Imune 0.0 - `< 1.0f` NAO distingue o
     tier, cobre os DOIS por construcao, G-3) e o status e CONSUMIDO. Contra Neutro/Fraco
     (`mult_fraqueza >= 1.0f`) o status fica INTACTO - so consome quando ha algo a furar
     (G-2), nao e um consume-sempre.
  ORDEM CRITICA: o pierce roda ANTES do `continue` do imune (senao o `continue` agiria antes
  dele) e ANTES do sorteio de canal - ele proprio NAO toca `rng_`, entao NAO muda a ordem de
  consumo de RNG dos casos SEM pierce (secao 11, a ordem e SAGRADA). O guard de fogo amigo
  (PR2) roda ANTES deste bloco inteiro (`continue` no INICIO do loop pra alvo aliado) - o
  pierce so se aplica a alvo INIMIGO, correto por construcao.
- **`estimate_card_damage` (preview PURO), o GEMEO obrigatorio:** MESMA logica, MESMA ordem,
  colada ANTES do curto-circuito de imunidade do preview. Diferenca deliberada: o preview
  NUNCA chama `remove_status` (contrato PURO do metodo) - sem isso a UI mostraria "IMUNE"
  enquanto o hit real fura, dessincronizando preview vs resultado (a classe de bug que a
  auditoria-dominó caca).
- **LIMITE DE COBERTURA HONESTO (achado durante o TDD, nao uma regressao):** o tier
  `WeaknessTier::Imune` (mult 0.0) NAO E ALCANCAVEL hoje via nenhuma combinacao real de
  `CardFamily` - `WeaknessWheel::tier_for` so devolve Fraco/Resistente/Neutro pros 5 pares
  nao-Universal (confirmado por inspecao de `weakness_wheel.hpp` e pela nota ja existente em
  `combat_formula_test.cpp`: "o tier Imune NAO e exposto pela API publica hoje... e flag de
  inimigo/lore, incremento futuro"). Os testes de `techmagic_godel_test.cpp` cobrem "vs
  RESISTENTE" (a UNICA combinacao real com `mult < 1.0` hoje) - o codigo do pierce
  (`mult_fraqueza < 1.0f`, sem distinguir o tier) trata os dois IDENTICAMENTE, entao a
  cobertura via Resistente exercita a MESMA branch que cobriria Imune no dia em que a flag
  de imunidade for plugada. Nao ha logica dedicada a 0.0 que fique sem teste.

Cross-ref: `GusEngine/domain/include/gus/domain/combat/combat_enums.hpp`
(`StatusId::NullProof`); `GusEngine/domain/src/combat/combat_state_machine.cpp`
(`resolve_use_card` - wiring do pierce; `estimate_card_damage` - gemeo PURO);
`GusEngine/domain/src/combat/master_cards.cpp` (godel); `GusEngine/domain/tests/
techmagic_godel_test.cpp`; `GusEngine/domain/tests/master_cards_test.cpp`;
`docs/design/mecanicas/cartas-technomagik.md` secao 6/9; `docs/design/roster-analogos/
_EFEITOS-ESCOLHIDOS.md` (AMB-05).

## Addendum (manifesto item 5, decisoes do lider 2026-07-15): Planck/Quantum-Lock

Diferente de todos os EffectKind anteriores (que resolvem POR DENTRO do dispatcher
`techMagic::execute`, contra `ctx.caster`/`ctx.counterpart`), o Quantum-Lock **pluga direto
no coracao da secao 11** (o resolvedor de dano `resolve_use_card` e o gemeo PURO
`estimate_card_damage`), nao no dispatcher de efeitos. PR PROPRIO, disjunto de qualquer
trabalho de fila (mexe na formula de dano, nao em ordenacao de turno).

- **`EffectKind::DamageQuantize` (ordinal 8, append-only)**: carta `planck` em
  `master_cards.cpp` - `Passiva`/`Universal`/mana 0, `effects = [OnCast ->
  DamageQuantize, magnitude=50 (chance% do centro), percent=25 (chance% de CADA
  extremo)]`. Carta HISTORICA (so o Gus equipa na progressao) - o MOTOR e agnostico
  por-ator, ZERO hardcode de id/nome de personagem no dominio (mesmo padrao do
  Reflect/Newton).
- **Handler no-op deliberado** (`techmagic.cpp::handle_damage_quantize`): satisfaz o
  invariante fail-fast "EffectKind sem handler = bug" sem lancar, mas nao faz nada - o
  trigger da carta e `OnCast`, que `execute_equipped` NUNCA despacha (Planck e Passiva,
  nunca jogada; equipadas so recebem `Always`/`OnDamageDealt`/`OnDamageReceived`/
  `OnRoundEnd`/`OnAllyTurnEnd`). Na pratica este handler nunca roda - documentado do mesmo
  jeito que o wiring fora-do-dispatcher do trunfo `ignores_weakness_wheel` de Godel.
- **`quantize_spec_of(actor, registry)`** (helper novo, anonimo em
  `combat_state_machine.cpp`): varre `actor.equipped_special_ids()` contra o
  `card_registry_` procurando um `EffectSpec.kind == DamageQuantize`. Fail-SOFT (nullptr),
  ao contrario do fail-fast de `execute_equipped` - registry nulo ou id ausente so
  significa "sem quantizacao", nunca bloqueia OUTRAS passivas equipadas do ator.
- **Wiring real, os DOIS gemeos:**
  - `resolve_use_card`, ramo COMUM: se `quantize_spec_of(actor, ...)` != nullptr, o 2o
    consumo de RNG (que seria `rng_->next_double()`, a variancia continua) vira
    `rng_->next(100)` (sorteio de degrau via `quantize_step_r`) - `roll2 < percent` = piso
    (`r=0.0`), `roll2 < percent+magnitude` = centro (`r=0.5`), senao teto (`r=1.0`). O `r`
    resultante alimenta o MESMO `comum_channel_damage(base_damage, v, r)` de sempre - zero
    formula paralela. Log ganha sufixo via `quantize_log_suffix` (degrau + chance%, regra
    do lider "todo efeito loga").
  - `estimate_card_damage` (preview PURO): campos ADITIVOS em `CardDamageEstimate`
    (`quantized`, `mid_damage`, `step_low_pct`/`step_mid_pct`/`step_high_pct`) calculados
    SEM sortear (`comum_channel_damage(base_damage, v, 0.5)` pro centro; piso/teto ja
    existiam via `r=0.0`/`r=1.0`) - bit-identicos ao que `resolve_use_card` produziria em
    cada degrau forcado (gemeo obrigatorio, auditoria-dominó).
- **Consumo de RNG (A5): MESMA contagem.** Canal COMUM com Planck = 2 consumos de
  `next(100)` (canal + degrau), 0 de `next_double`. Canal COMUM sem Planck = 1 de
  `next(100)` + 1 de `next_double` (caminho INTOCADO, byte-identico ao de sempre). Total =
  2 nos dois casos - so o TIPO do 2o consumo muda.
- **A6, critico/falha intactos:** a quantizacao so troca o `else` do canal COMUM; os ramos
  FALHA e CRIT (e o curto-circuito de imunidade, ANTES do sorteio) ficam byte-identicos com
  ou sem a passiva.
- **Colapso gracioso**: kills altos (`v` no piso ~0.05) + dano-base pequeno podem fazer os
  3 degraus arredondarem (`lround`) pro MESMO inteiro - comportamento esperado (o preview
  mostra os 3 iguais), coberto por teste dedicado.
- Testes: `GusEngine/domain/tests/techmagic_quantize_test.cpp` (10 casos: degraus corretos
  vs formula hand-calculada, paridade preview<->real com Shield, fronteiras do sorteio,
  determinismo/contagem de RNG com e sem a passiva, escopo por-ator, critico/falha
  intactos, ataque basico inalterado, colapso gracioso, log, fogo-amigo/imune 0 consumo
  extra) + `master_cards_test.cpp` (catalogo + i18n `CARD_EXEC_PLANCK_NAME`).

Cross-ref: `GusEngine/domain/include/gus/domain/combat/combat_enums.hpp`
(`EffectKind::DamageQuantize`); `GusEngine/domain/include/gus/domain/combat/
combat_state_machine.hpp` (`CardDamageEstimate` campos aditivos); `GusEngine/domain/src/
combat/combat_state_machine.cpp` (`quantize_spec_of`/`quantize_step_r`/
`quantize_log_suffix`, wiring em `resolve_use_card`/`estimate_card_damage`);
`GusEngine/domain/src/combat/techmagic.cpp` (`handle_damage_quantize`, marcador no-op);
`GusEngine/domain/src/combat/master_cards.cpp` (planck); `docs/design/mecanicas/combat.md`
secao 11 (Quantum-Lock); `docs/design/roster-analogos/_EFEITOS-ESCOLHIDOS.md` (AMB-06).

## Addendum (MVP step 8, manifesto item 6, decisoes do lider 2026-07-15, D1-D4): John Dee/Black-Mirror (RevealIntent/Scrying)

Diferente de HypotenuseCombo/RepeatLastAction (hooks despachados via `execute_equipped`
sobre cartas EQUIPADAS), o RevealIntent e **status-based**: a ativa aplica um BUFF
(`StatusId::Scrying`, ordinal 18) no proprio caster, e o re-dump de rodada e disparado
pela FSM checando o STATUS do ator (`index_of_status`), nao pelo mecanismo de cartas
equipadas. `execute_equipped` NUNCA despacha isto.

- **`EffectKind::RevealIntent` (ordinal 9, append-only)**: carta `dee` em
  `master_cards.cpp` - `Hibrida`/`Universal`/`TargetShape::Self`/mana `kActiveManaCost`
  (D4), `effects = [OnCast -> RevealIntent, duration=3, status=Scrying,
  stack_rule=Refresh]` (D3).
- **`StatusId::Scrying` (ordinal 18, append-only)**: buff auto-aplicado no CASTER via
  `add_status` LEGADO (NAO `try_add_status` - nao passa pelo portao de imunidade
  EM-Shield, e um buff auto-aplicado, nao um debuff ofensivo em outro ator). Classificado
  BUFF por exclusao (`is_non_buff` em `combat_actor.cpp` nao o lista).
- **`handle_reveal_intent`** (`techmagic.cpp`, dispatcher `OnCast`): aplica o buff
  Scrying, loga a ativacao, e chama `dump_reveal_intent` (funcao PUBLICA nova, ver
  abaixo) pra dumpar os intents dos inimigos vivos no MOMENTO do cast.
- **`log_intent_for(target, ctx)` e `dump_reveal_intent(ctx)`** (funcoes PUBLICAS novas,
  expostas em `techmagic.hpp` FORA do dispatcher `execute` - unica fonte de verdade
  reusada por 3 sitios: o dump inicial do cast, o re-dump de rodada, e o Scan
  aprimorado):
  - `log_intent_for`: loga 1 linha do `IntentPreview` de UM alvo, do ponto de vista do
    caster. Busca o `IEnemyBrain` em `ctx.brain_registry` (campo ADITIVO novo em
    `TechMagicContext`, mesmo registry id->`IEnemyBrain*` ja injetado na FSM).
    **FAIL-SOFT no brain ausente** (loga "sem sinal" e retorna, NUNCA lanca) -
    assimetria DELIBERADA vs. o Gambito-Prever manual (`resolve_gambit_predict`, que
    lanca `std::out_of_range`): a Scrying roda em contextos AUTOMATICOS (dump em
    massa/re-dump de rodada) ou como bonus OPCIONAL do Scan, nenhum dos dois pode
    derrubar o combate por 1 inimigo sem brain registrado; o Gambito e uma acao
    DELIBERADA do jogador sobre 1 alvo especifico, onde falhar alto e o certo. **D2
    (intent CAOTICO, Patch-Zero):** se `intent.is_chaotic == true`, loga RUIDO -
    NUNCA revela `predicted_action_id`/`predicted_target_id`/`predicted_damage` -
    preserva a one-way door do boss.
  - `dump_reveal_intent`: itera `ctx.combatants` (mesmo guard de lado do ChainDamage -
    inimigos VIVOS do lado OPOSTO ao caster), chamando `log_intent_for` pra cada um.
    Zero inimigos vivos: no-op logado (dissipacao), NAO silencioso.
  - Read-only sobre o combate: monta um `CombatState` LOCAL (via `ctx.queue`, so
    quando ha brain de fato pra consultar) pra alimentar `brain->preview_intent`, mas
    NAO muta NADA (0 dano, 0 status novo em terceiros, 0 RNG, fila intocada).
- **`CombatStateMachine::process_scrying_hooks`** (D3, re-dump de rodada): gemeo de
  `process_round_end_hooks`/`process_ally_turn_end_hooks`, mas orientado a STATUS -
  itera `queue_.order()`, e pra cada ator VIVO que porta `StatusId::Scrying`
  (`index_of_status >= 0`), chama `dump_reveal_intent` de novo. Chamado na FRONTEIRA
  DE RODADA (`advance_to_next_actor`, junto de `process_round_end_hooks`/
  `regroup_round_by_side`). Duracao tickada pelo motor de status DE SEMPRE
  (`apply_status_tick`, no `TurnStart` do proprio caster) - nenhum codigo dedicado de
  expiracao; quando `Scrying` some do ator, o proximo `process_scrying_hooks` so pula
  ele (`continue`).
- **Scan aprimorado (D1-ii):** `resolve_scan` ganha um bloco condicional -
  `has_reveal_intent_equipped(actor, registry)` (helper novo, anonimo em
  `combat_state_machine.cpp`, MESMO padrao de scan-por-EffectKind de
  `quantize_spec_of` - varre `equipped_special_ids()` procurando `EffectKind::
  RevealIntent`, Fail-SOFT `false` se ausente). Quando true, o Scan loga 3 linhas
  extra: status ativos do alvo (`status_name`, helper novo), posicao na fila
  (`InitiativeQueue::index_of`), e o intent previsto (`log_intent_for` direto). SO
  dados que JA EXISTEM no modelo - nenhum atributo oculto novo.
- **Stub posse-only de mundo (D1-i):** a face "revela baus/passagens ocultas na
  exploracao" do design original NAO tem sistema de ocultos no overworld ainda -
  ZERO codigo de combate aqui, so um comentario/ponto de extensao em
  `master_cards.cpp::dee`. Quando o sistema de ocultos existir, a query reusa o MESMO
  mecanismo do Scan aprimorado (`equipped_special_ids()` contem uma carta com
  `EffectKind::RevealIntent`?) - nenhum campo/EffectKind novo necessario. Mesmo padrao
  do stub anti-PEM do Faraday/Euler/Menger.
- **TESTE-REI (determinismo, mutation-alvo):** dois combates com a MESMA seed - um com
  Scrying ativo no caster, outro sem - produzem HP/dano/ordem-de-fila/contagem-de-RNG
  BYTE-IDENTICOS ao longo de N rodadas; so o log difere. Prova 0 consumo de RNG e que
  chamar `preview_intent` N vezes extras nao muta nenhum brain (telegraph honesto).
- Testes: `GusEngine/domain/tests/techmagic_reveal_test.cpp` (11 casos: dump inicial,
  no-op 0-inimigos, brain ausente fail-soft x2, D2 caotico->ruido sem vazamento, guards
  fail-fast, D3 re-dump de rodada + expiracao, Scan aprimorado, stub posse-only
  negativo, TESTE-REI) + `master_cards_test.cpp` (catalogo + i18n `CARD_EXEC_DEE_NAME`).

Cross-ref: `GusEngine/domain/include/gus/domain/combat/combat_enums.hpp`
(`EffectKind::RevealIntent`, `StatusId::Scrying`); `GusEngine/domain/include/gus/domain/
combat/techmagic.hpp` (`TechMagicContext.brain_registry`, `log_intent_for`,
`dump_reveal_intent`); `GusEngine/domain/src/combat/techmagic.cpp`
(`handle_reveal_intent`); `GusEngine/domain/src/combat/combat_state_machine.cpp`
(`process_scrying_hooks`, `resolve_scan`, `has_reveal_intent_equipped`, `status_name`);
`GusEngine/domain/src/combat/master_cards.cpp` (dee); `docs/design/mecanicas/
cartas-technomagik.md`; `docs/design/roster-analogos/_EFEITOS-ESCOLHIDOS.md` (AMB-07).

## Consequencias

- **Positivas:** custo BAIXO-MEDIO, risco BAIXO; numeros 100% tunaveis pra playtest; testavel por unit test por EffectKind; easter-egg entregue hoje; nao fecha porta pra VM. Cada carta = 5-15 linhas de dado.
- **Negativas/custo irredutivel:** cada efeito exotico exige um EffectKind + handler C++ novo (mas esse custo existe em QUALQUER opcao - e das primitivas, nao do envelope). O enum EffectKind cresce a ~20-25 valores.
- **Impacto:** desbloqueia PS-Y1 (HIBRIDA) e PS-Y2 (Reflect/cross-ator/clone). Supersede a ambiguidade do fork em TECHMAGIC-EXECUTOR.

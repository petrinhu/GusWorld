# ADR-017 — Combate unificado por Action-Clock (pedido do Gus + velocidade-por-carta)

**Status:** ACEITO (decisão do líder 2026-07-16). Constrói ISOLADO agora; cutover do motor DEPOIS do M7 fechar. One-way-door de feel de combate.
**Origem:** pedido do Gus Dragon (`docs/design/mecanicas/pedido-arceus-battle-engine.md`) + a velocidade-por-carta compilada/interpretada já canônica (`combat-flavor.md §1-2`). Decisão de UNIFICAR os dois num sistema só (são a mesma camada: timing de ação). Pareceres: `software-architect` (técnico) + `lead-game-designer` (feel), convergentes.

## Decisão

Substituir a `InitiativeQueue` (fila CTB por array+cursor) por um **`ActionClock`** (escalonador por ticks, min-heap por `next_action_at`), unificando três coisas:
- **Motor de ordem** = o action-clock por-ator (estilo Pokémon Legends: Arceus).
- **Velocidade-base da carta** = a linguagem (compilada = rápida, `speedMult < 1`; interpretada = lenta, `speedMult > 1`) — a decisão de `combat-flavor.md §2` pluga NATIVO como o `cardSpeedMult`.
- **Alavanca do jogador por-cast** = os 3 estilos Normal/Ágil/Forte (modificam action-time + dano).

### D1 — Ritmo: HÍBRIDO (clock por-ator + proteção do Gus) [decisão do líder]
Cada ator compete no relógio individualmente (intercalação real entre os dois lados — honra o pedido literal do Gus, "P2 pode agir várias vezes seguidas"), **MAS com uma trava anti-starvation que protege o Gus (o mais frágil, HP 34)**:
- **Starvation-guard (//PLAYTEST):** nenhum membro da party pode ficar mais que `MAX_STARVE_TICKS` (ou N ações inimigas) sem agir; ao estourar, o `next_action_at` dele recebe um catch-up bump pra agir logo. Mata a dor #3 do Arceus que o próprio Gus listou ("preso sem jogar em multi").
- Redes de segurança já-existentes que continuam valendo: Análise Preditiva 1×/batalha (absorve golpe fatal, qualquer estilo), Gambito-Reordenar 2 AP (já força mudança de alvo do inimigo, `combat.md §12`), escalonamento de letalidade por MODOS-MORTE.
- _(Rejeitado: Arceus puro sem proteção — reabre o "preso sem jogar" + ameaça o Gus. Rejeitado: conservador puro — não entrega o dinamismo que o Gus pediu.)_

### Comando da party (decisão do líder 2026-07-16): escolher entre os PRONTOS + segurar
O relógio decide QUANDO cada membro fica pronto (não é mais o free-order do 1B). Quando 1+ membro da party está pronto, o jogador **escolhe qual joga** (ação + alvo) e **pode SEGURAR** um membro pronto pra sequenciar (esperar outro ficar pronto e decidir a ordem dos dois). O jogador **sempre escolhe o ALVO** da ação (single/AoE) — isso é ortogonal ao motor e nunca muda. O que se perde vs 1B: forçar um membro a agir ANTES do relógio dele deixar. _(Rejeitado: "banco" total de turnos = afrouxa a pressão do relógio; mínimo sem-segurar = menos controle de ordem.)_

### Indicador de prontidão — barra de carga + LED sobre a cabeça (decisão do líder 2026-07-16, camada de UI/battle-screen)
Cada personagem exibe, **sobre a cabeça**, uma **barra de carga** (fração = quanto falta pro `next_action_at`, lida direto do action-clock — dado nativo, sem sistema novo) com um **LED na ponta direita da barra**:
- **PRONTO (pode ser escolhido):** LED **verde vivo**, aceso, leve glow.
- **CARREGANDO:** LED **verde escuro** (apagado) + um **pontinho branco** de brilho especular (parece LED físico desligado pegando luz ambiente).
Ancorado na posição do personagem (glintfx/RML ancorado OU host-drawn na cena). É a leitura "quem posso escolher AGORA" por-personagem; complementa a fila/clock visível (`battle-screen.md D4`). Entra no **cutover/battle-screen** (o build isolado do ActionClock é só lógica). Viável: confirmado.

## Arquitetura

```
struct ActionClockEntry { CombatActor* actor; int next_action_at; int last_acted_at; };
// escalonador = heap por next_action_at; pop() = próximo a agir.
// desempate: (last_acted_at asc → SPD desc → preferência do jogador)  [segue o pedido do Gus, D3]

// reset pós-ação:
next_action_at = global_tick + round( BASE_CLOCK / SPD_efetivo
                                       × cardSpeedMult(carta)
                                       × styleMult(estilo, cadeiaAgil) )
```
- **Ticks INTEIROS** (não float) — consistência com a §11 (tudo int/lround), preview determinístico trivial, zero risco de não-determinismo cross-compiler (CI Windows MSVC + Linux já validado). [D2 arch]
- **Cast interpretado ("resolve N casas à frente") = resolução AGENDADA num tick futuro** (`PendingResolution{efeito, alvo, resolve_at}`, evento one-shot no mesmo clock; cancelável por Stun/Disrupt/Silence/dano antes de `resolve_at` → RUNTIME ERROR, já canon). Nunca foi implementado em código (só doc) → nasce direto no modelo novo, sem clamp de array.
- **Estilo NÃO toca a fórmula de dano na ordem de RNG:** `multEstilo` entra na cadeia divisiva §11 como fator novo (padrão `multExpose`/`multAmbiente`), ANTES do sorteio de canal — zero mudança no consumo de RNG, zero mudança no Planck/average-preserving. **Nunca toca `multFraqueza`** (a roda de fraqueza continua o eixo dominante; o estilo AMPLIFICA a leitura certa: Forte na fraqueza 1.5×1.5=2.25×; Forte na resistência 0.66×1.5≈0.99× — reforça "vence por lógica").

## Números baseline (//PLAYTEST — parecer do lead-game-designer, N=3 afina)
- `BASE_CLOCK` = 1000 (constante de calibração).
- **cardSpeedMult:** compilada **0.85** · interpretada **1.35** · ataque básico 1.00.
- **styleMult (tempo / dano):** Normal 1.00/1.00 · **Ágil 0.55/0.50** (dano −50% FIXO) · **Forte 1.60/1.50**. [D2 game = "fiel ao Gus"]
- **Ágil consecutivo = decaimento suave** (não hard cap): 1ª ×0.55, 2ª ×0.75, 3ª+ ×0.90 (dano sempre ×0.50); loga o decaimento no terminal. Mata o double-turn infinito por anti-economia, sem trava de UI. [D3]
- **IA:** Trash (`ScriptedBrain`) sempre Normal (previsível, `combat.md §13.1`); estilo esperto só no `UtilityBrain` (Elite/boss, fase posterior), por heurística DETERMINÍSTICA, **nunca por sorteio** (fura o pillar). [D4]
- **Especiais/Super = FORA da alavanca de estilo** (Normal-only, no valor canônico fechado) — são classe curada (2 camadas, 2026-07-16); não triplicar a superfície de teste adversarial de cada EffectKind.

## Transparência (a dor #1 do Gus — já 90% resolvida)
O contrato `IEnemyBrain::PreviewIntent` já é OBRIGATÓRIO e legível (§13; só o Patch-Zero é caótico, one-way-door dele). Estender o record `IntentPreview` (`combat.md:814`) com **`EstiloProjetado` + `TicksProjetados`** (vira "ver o action-clock"). **REGRA DURA (gate de teste, igual ao gêmeo preview↔real do Planck/A2):** `PreviewIntent()` e `DecideAction()` do MESMO inimigo retornam o MESMO estilo pré-comprometido — senão reproduz o bug do Arceus (preview mente). Preview "rico" (variações se-inimigo-usar-X) = 2ª onda de UI, não bloqueia o motor.

## Migração (mapa do software-architect)
- **INTACTO (9 de 13 EffectKinds + §11 + CombatActor):** ApplyStatus, Leech, Reflect, HypotenuseCombo, CloneAlly, ChainDamage, RepeatLastAction, DamageQuantize/Planck, RevealIntent/Scrying, TokenRefund, ApEfficiency-face1. Fórmula de dano, roda de fraqueza, status, Shield — ortogonais, não mudam.
- **MIGRA (mais simples):** Einstein/DelayAction + Gambito-Reordenar (`reorder_pending`) + Knockback (`delay_current`) → viram `+N ticks` no `next_action_at` (já são "manipulação de timer" disfarçada de posição de array). Gambito-Reordenar fica MAIS legível ("atraso X ticks reais" vs "1 slot").
- **REWORK de fórmula (não de motor):** `turnoIndex`/`round_index` GLOBAL → contagem própria por-ator (`own_turn_count` p/ mana ramp `2+n`) + "rodada" = janela de N ticks (p/ ambientes T1/T2, Hayek/`round_actions_`, Mises-face2).
- **REWRITE:** `InitiativeQueue` → `ActionClock`; ~130 de ~465 testes de combate (os de ordem/cursor/round) reescrevem; ~335 (dano/status/roda) só recompilam. Ganho estrutural: "tempo só anda pra frente" no clock ELIMINA a classe de bug de cursor que o `COMBATE-FILA-CURSOR-FIX` consertou à mão.

## Plano [decisão do líder: isolado agora, cutover pós-M7]
1. **AGORA (zero risco pro M7):** construir o `ActionClock` ISOLADO (módulo próprio + harness), provando a fórmula de reset + os 3 estilos + o decaimento do Ágil + o desempate + o starvation-guard + a `PendingResolution` contra os **6 casos de exemplo do Gus** (Caso 1-6 do pedido = casos de teste prontos). NÃO tocar `CombatStateMachine`/`InitiativeQueue` de produção.
2. **Cutover (DEPOIS do M7):** onda dedicada `ARCEUS-CLOCK` — reescrever o turn-loop da FSM + os ~130 testes de ordem + o rework de "rodada", com auditoria-dominó (`qa-engineer`/`internal-auditor`). Sem flag de coexistência (abstração vazaria; ninguém pediu troca-de-motor em runtime).

## Pendências a confirmar retroativamente / vigiar no N=3
Números //PLAYTEST (multiplicadores, BASE_CLOCK, MAX_STARVE_TICKS); o "Golpe+status" das comuns piora com Forte (já no radar do N=3); a mecânica exata do starvation-guard (ticks vs contagem de ações). Cross-ref `CARTAS-CAST-TIME` (absorvido por este ADR), `COMBATE-TEORIA-JOGOS`, `MODOS-MORTE`, `battle-screen.md D4` (fila visível → clock visível).

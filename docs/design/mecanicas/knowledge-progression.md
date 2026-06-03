# Knowledge Progression — Spec de Design
**Status:** Canônico. Ratificado Sprint 2 W2 2026-06-03. Consolida F2-D.3 + F2-DSGN.KP.
**Cross-ref:** combat.md §11 (fórmula canônica), pillars.md.

> **Convenção:** pt-br, sem em-dash. Termos game-dev no original (variance, kill, threshold, crossover).

## 1. O que esta spec faz (e o que NÃO faz)

A fórmula em combat.md §11 JÁ está implementada e é canônica. Esta spec NÃO a altera: alimenta-a.
Ela define **os valores que o engine ainda não tem**: N-aparições por tier, o que conta como kill,
o threshold do Scan passivo, e o gating do Diário de Campo. Serve Pillar 1 (lógica vence força:
conhecer o inimigo remove a incerteza, anti-grind GDD §5) e Pillar 4 (Diário com voz analítica de 11 anos).

### Dois eixos independentes (reconciliação canônica)

Há DOIS contadores de "conhecer o inimigo" que NÃO se confundem:

| Eixo | Contador | O que faz | Vive em |
|---|---|---|---|
| **A. Variância de combate** | `CombatActor.KnowledgeKills` | decai ±30%→±5% (combat.md §11) | engine POCO + SaveSystem |
| **B. Bestiary do Diário** | progresso de páginas (1-4) | preenche entry narrativa + Knowledge categorial | diary/entries-fichas-bestiary.md |

Eixo A é **mecânico** (controle de dano). Eixo B é **narrativo** (compreensão + ending-gate, knowledge-gates.md).
Os triggers de página do bestiary (4/4/1 kills) JÁ são canônicos em entries-fichas-bestiary.md §5 e knowledge-gates.md §3;
esta spec NÃO os toca. O que falta e é definido aqui = a curva do Eixo A + o Scan passivo + a ponte entre os dois.

## 2. N-aparições por tier (Eixo A — proposta para ratificação)

`varianceFactor = max(0.05, 0.30 × e^(-knowledgeKills × 0.10))` (combat.md §11, canônica). Crossover natural ~18 kills.
N-aparições = quantos encontros designados daquele tipo existem no jogo inteiro (teto de farm orgânico, sem grind).

| Tier | N-aparições designadas | Kills p/ dominar (variância ±5%, "Vector mode") | Kills p/ Scan passivo | Brain (combat.md §13) |
|---|---|---|---|---|
| **Trash comum** | ~20 | 30 (piso ±5% só assintótico; ver §5) | **8** (canon DA-1) | ScriptedBrain |
| **Elite** | ~8 | atinge ±~8% no teto de 8 | **6** (canon DA-1) | UtilityBrain |
| **Mini-boss** | ~4 | atinge ±~17% no teto de 4 | **3** (canon DA-1) | UtilityBrain |
| **Boss** | 1 | sempre ±30% (kill único) | nunca (1 encontro) | UtilityBrain (+Perlin = Patch-Zero) |

Leitura de design: só o Trash comum (~20 aparições) chega perto do determinismo total. Elite/Mini-boss/Boss
mantêm tensão residual POR DESIGN: você nunca "resolve" um mini-boss à base de repetição, porque ele não
reaparece o suficiente. Isto protege Pillar 1 (a curva não vira power-creep) sem catch-up artificial.
A referência do TODO ("comum ~20 / mestre 4 / boss 1") casa com o crossover ~18 da fórmula: aos ~20 encontros
de Trash o jogador está em variância quase-determinística, o sinal narrativo de "dominei este tipo".

## 3. O que conta como "kill" para KnowledgeKills (proposta)

`KnowledgeKills` é "encontros resolvidos com aquele tipo via análise", não "corpos no chão". Proposta:

| Evento | Conta? | Racional (Pillar 1 / anti-grind) |
|---|---|---|
| Derrotar o inimigo em combate | **+1** | observação completa do padrão do início ao fim |
| Fugir do combate (Flee, combat.md §14) | **0** | não observou o ciclo completo; sem aprendizado |
| Scan do tipo (1 AP, combat.md §12) | **0** (DA-2 canon) | Scan revela dado; kill = ciclo completo em combate. KnowledgeKills continua int. |
| Matar o mesmo tipo após teto de N-aparições | **0 Knowledge categorial** (Eixo B) | knowledge-gates.md §8 (anti-grind: bestiary cap dá zero) |

Nota: a variância (Eixo A) continua a decair com kills mesmo além do bestiary cap, mas como o tipo
não reaparece (N-aparições esgotado) o ponto é teórico. Não há grind farmável porque não há spawn infinito.

## 4. XP diferencial por zona × Knowledge (dois sinais convergentes anti-grind)

`xp = base_xp × max(0, 1 - (player_zone - enemy_zone) × 0.15)` (combat.md §11, canônica; vive em game/, F2-G.8).

Os dois sistemas convergem para o MESMO sinal anti-grind, por vetores opostos:

- Farmar inimigo fraco (zona baixa) -> **XP despenca** (chega a 0 com 7 zonas de gap).
- Farmar o mesmo tipo -> **variância despenca** (combate fica trivial e sem surpresa) E **Knowledge categorial satura** (bestiary cap = 0 ganho).

Resultado: repetir não dá poder (XP), não dá tensão (variância mínima), não dá compreensão (Knowledge cap).
O jogo "não responde a esforço repetitivo" (knowledge-gates.md §8). Progressão é linear-narrativa (GDD §5).

## 5. Curva visual (Eixo A — valores calculados para ratificação)

`varianceFactor = max(0.05, 0.30 × e^(-kills × 0.10))`. "% incerteza" = ±varianceFactor sobre o dano.

| kills | varianceFactor | % incerteza | Estado do Diário (bestiary) | Scan passivo (Trash)? |
|---|---|---|---|---|
| 0 | 0.300 | ±30.0% | Stub (1ª aparição: silhueta, HP visto, 1 ataque) | não |
| 1 | 0.271 | ±27.1% | Stub | não |
| 3 | 0.222 | ±22.2% | Página 2 (padrão, 2-3 ataques, hipótese de fraqueza) | não |
| 4 | 0.201 | ±20.1% | Página 3 (Trash: fraqueza confirmada, loot, telegrafia) | não |
| 6 | 0.165 | ±16.5% | Página 3 | não |
| **8** | **0.135** | **±13.5%** | Página 3 -> 4 conforme Knowledge | **SIM (proposta Trash)** |
| 12 | 0.090 | ±9.0% | Página 4 (exploit, companion-counter, lore) | sim |
| 18 (crossover) | 0.050 (≈) | ±5.0% | Página 4 plena | sim |
| 30+ | 0.050 (piso) | ±5.0% | Página 4 plena | sim |

Observação numérica: o piso ±5% só é atingido formalmente por volta de kills≈18 (`0.30 × e^(-1.8) ≈ 0.0496` -> clamp 0.05).
Para Trash (~20 aparições) isto é alcançável; para Elite (~8) o jogador para em ~±8%, para Mini-boss (~4) em ~±17%.
A tabela acima é a fonte-de-verdade para os testes de balance (F2-D.1) e para o tuning do threshold de Scan passivo.

## 6. O Diário de Campo (Bestiário leve) — ponte Eixo A↔B

O Diário JÁ existe como sistema narrativo (diary/, 8 docs canônicos Bloco H) e como UI (diary/ui-spec.md:
caderno layer 1 + texto limpo layer 2). Esta spec só fixa a regra de **quando a entry aparece e cresce**,
alinhada à curva §5, e confirma o tom (Pillar 4).

- **Trigger de abertura:** stub aparece **após o 1º combate** com o tipo (não após Scan), conforme
  entries-fichas-bestiary.md §5.1. Scan isolado revela HP+fraqueza no HUD, mas não cria a entry narrativa.
- **Páginas por tier (DA-3 canon 2026-06-03):** Trash 1-2 / Elite 2-3 / Mini-boss 3 / Boss 4. Escala com
  importância; econômico de conteúdo. Casa com pillars.md §Knowledge: "max 1 pág comum / 4 pág mestre".
- **Tom (DA-4 canon 2026-06-03):** híbrido. Stats técnicos (HP/fraqueza/counter) SEMPRE; lore/origem só na
  última página em voz analítica de Gus (prodígio 11 anos). Serve Knowledge categorial sem virar lore-dump.
- **Conteúdo por página (tom Pillar 4, prodígio 11 anos):** stub = observação crua, hipótese ("hipótese:
  vulnerável a ultrassom?"); pág2-3 = confirmação técnica ("confirmado após 3 testes"); pág4 = exploit +
  companion-counter ("Cauã abre janela em 2 turnos; Linda finaliza em 1"). Voz analítica, "mostre não conte"
  (entries-fichas-bestiary.md §Princípio). Patch-Zero é exceção: a entry NUNCA estabiliza (glitch tipográfico).
- **O número não aparece no Diário:** `KnowledgeKills` é técnico (HUD); o Diário mostra ESTADO (borrado/parcial/
  cristalino), não o inteiro (knowledge-gates.md §6.1). Coerência de UX já canônica.

## 7. Anti-degeneração (como a curva protege os pillars)

| Risco de degeneração | Trava nesta spec |
|---|---|
| Grind por repetição | N-aparições FINITO por tier (§2): sem spawn infinito, sem farm. XP↓ + variância↓ + Knowledge-cap convergem (§4). |
| Hiper-especialista trivializa TODOS de um tipo cedo demais | Só Trash chega a ±5%; Elite/Mini-boss/Boss param em ±8/±17/±30% pelo teto de aparições. Boss SEMPRE ±30%. |
| Scan passivo vira "Scan grátis pra tudo" | Threshold POR TIPO (não global): cada novo tipo recomeça em "Scan custa 1 AP". Conhecimento não transfere. |
| Catch-up / pity artificial | NENHUM. A curva é monotônica e determinística; não há rubber-band. Pillar 1 (sem aleatoriedade punitiva, mas também sem muleta). |
| Fugir pra farmar variância sem risco | Flee = 0 kills (§3): não há atalho sem enfrentar o ciclo completo. |

## 8. Integração com o Vertical Slice (F2-E.5)

O VS de combat.md §17 entrega 2 inimigos: **Sentinela-Bit** (Trash, HP55) + **Daemon-Guard** (Elite, HP144),
ambos tipo Cinético / fraqueza Elétrica (Cauã é o DPS natural).

- **No slice, `KnowledgeKills` vem fixo** (combat.md §17 "Fora do slice": alimentação real pelo SaveSystem = jogo posterior).
  Para o VS, propõe-se demonstrar a curva com 2 valores plugados: `KnowledgeKills=0` (1º encontro, ±30%) e
  `KnowledgeKills=8` (encontro "dominado", ±13.5%, Scan passivo ativo) para validar o sinal em playtest.
- **Primeiras ~10 batalhas (manifestação esperada):** encontros 1-2 com Sentinela-Bit = surpresa (±30%, Scan custa AP,
  stub no Diário). Encontros 3-6 = padrão lido (variância caindo, pág2-3, jogador percebe que Elétrico é a chave).
  Encontros 7-10 = Scan passivo do Sentinela-Bit ativa (±13.5%), liberando o AP do Scan para ataque/Gambito.
  O Daemon-Guard (Elite, menos aparições) permanece com mais incerteza: ensina o contraste entre tiers.
- **Métrica de playtest (N=3 familiar, GDD §8):** observar se o jogador NOTA o Scan virar grátis (feedback claro
  de "dominei este tipo") e se a queda de variância é SENTIDA como controle, não como tédio. Hipótese falseável:
  "o jogador troca o AP de Scan por ofensiva assim que o Scan passivo ativa". Se não trocar, o threshold (8) está alto.

## 9. Notas de implementação (delegado Fase 2, não-bloqueante do VS)

- N-aparições e thresholds = data-driven (tabela por tipo, não hardcoded), para tuning em playtest sem recompilar.
- Scan passivo: quando `KnowledgeKills >= scanPassiveThreshold[tipo]`, o custo de AP do Scan daquele tipo = 0
  (combat.md §12 já prevê o passivo permanente; falta só o número, fixado aqui como proposta §2).
- A ponte para o Diário (Eixo B) é via `PlayerBus.CombatResultReceived` (combat.md §16); o engine POCO de combate
  não referencia o Diário (mantém-se reutilizável).

---

## Decisões Canonizadas (Sprint 2 W2 2026-06-03)

| ID | Decisão | Escolha |
|---|---|---|
| DA-1 | Threshold Scan passivo por tier | **Trash 8 / Elite 6 / Mini-boss 3** — ±13.5% Trash quando ativa; padrão lido mas incerteza residual |
| DA-2 | Scan sem derrota conta como kill? | **Não conta (0)** — kill = ciclo completo em combate; KnowledgeKills permanece int |
| DA-3 | Páginas do Diário por tier | **Trash 1-2 / Elite 2-3 / Mini-boss 3 / Boss 4** — escala com importância, econômico de conteúdo |
| DA-4 | Tom do Diário | **Híbrido** — stats técnicos sempre; lore/origem só na última página, voz analítica de Gus (Pillar 4) |

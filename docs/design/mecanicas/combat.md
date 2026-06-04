# Sistema de Combate Turn-Based (GusWorld)

**Status:** Canônico. Decisões ratificadas pelo criador supremo em 2026-05-26 (proposta de combate aprovada integralmente, sem reabertura). Spec de produção para implementação TDD da Fase 2 (vertical slice F2-E.5).

**Cross-ref pillars:** este sistema materializa GDD §6.1 (Sintonização Ortodôntica / Scan), §6.2 (Compilação de Deck Rúnico) e §6.3 (Vetores do Gambito), e serve diretamente Pillar 1 (Lógica vence força), Pillar 2 (Magia é sistema formal computável), Pillar 3 (Triângulo de hardware é a interface) e Pillar 4 (Prodígio de 11 anos, vulnerabilidade física + vitória por descoberta).

**Convenção de escrita:** pt-br. Termos técnicos de game-dev no original (turn-based, core loop, FSM, intent, telegraph, knockback etc). Sem em-dash; usa ponto, vírgula, parênteses, dois-pontos.

---

## 1. Visão e aesthetic alvo

O combate é a arena onde o verb dominante (decifrar) vira sistema jogável. O jogador não vence por dano bruto nem por reflexo: vence lendo o sistema (Scan), antecipando o adversário (Gambito) e compondo a resposta correta (Compilação de cartas). Aesthetic alvo, em ordem: **Challenge** (leitura de sistema), **Discovery** (descobrir fraquezas e receitas de combo), **Expression** (build de deck e estilo de jogo).

Princípio decisório: **toda regra abaixo existe para forçar decisão interessante** (Sid Meier), não complexidade decorativa. Toda mecânica responde risk vs reward de forma explícita.

### Verbs e o triângulo de hardware (Pillar 3)

| Verb | Vértice de hardware | Onde aparece |
|---|---|---|
| Scanear | Óculos Táticos + Matriz Ortodôntica | revela HP, fraqueza, intent, buffs |
| Prever / Reordenar | Óculos Táticos | Gambito: lê e manipula a fila de turnos |
| Compilar / Jogar carta | Tavus-Drive | executa cartas e pipeline de combo |

---

## 2. Parâmetros macro do combate

| Parâmetro | Valor canônico | Notas |
|---|---|---|
| `MaxPartySize` (engine) | 1 a 4 (configurável) | módulo de engine reutilizável aceita 1-4 |
| Party do jogo GusWorld | **3** | Gus + 2 companions ativos |
| Inimigos por encontro | 1 a 4 | encontros assimétricos são válidos (ex: 3 party vs 1 mini-boss) |
| Ordem de turno | **Por-ator** (CTB-style) | fila ordenada por SPD, não por-time |
| `AP` por turno | **3 fixo** | cresce via skill tree em jogo posterior; no slice é constante |
| `Mana` (Compilação) | ramp linear | `manaMax = 2 + turnoIndex`, cap 8 |
| Carry-over de mana | **nenhum** | recarrega ao máximo todo TurnStart; impossível bankar |
| Determinismo no slice | total (variância 0) | RNG visível plugado mas com variância zerada na entrega F2-E.5 |

### 2.1 Contrato de fragilidade do protagonista (Pillar 4, one-way door)

Decisão canônica N.2 R1, 2026-06-03:

- **HP de Gus é sempre o menor da party** (hard cap no statline; nenhum companion pode ter HP base igual ou menor que Gus).
- **Análise Preditiva (1× por batalha)**: se Gus tomaria dano fatal (HP → 0 ou abaixo), a mecânica absorve o golpe e Gus sobrevive com 1 HP. Não se acumula, não recarrega durante a batalha. Anunciado na UI: `ANÁLISE PREDITIVA: golpe fatal absorvido`.
- Reforça Pillar 4 mecanicamente: prodígio de 11 anos não é tank; vence por lógica (Scan/Gambito/Compilação), não por resistência bruta.

---

## 3. Máquina de estados do combate (FSM)

FSM por-ator. Cada ator (party ou inimigo) toma seu turno na ordem da fila de iniciativa. Multi-ação no turno acontece dentro de `ActionSelect` via loop interno consumindo AP.

```
        ┌──────────────┐
        │  SetupPhase  │  (instancia atores, monta fila SPD, dispara CombatBus.CombatStarted)
        └──────┬───────┘
               ▼
        ┌──────────────┐
   ┌───▶│  TurnStart   │  (recarrega mana do ator ativo: manaMax = 2 + turnoIndex, cap 8;
   │    └──────┬───────┘   aplica tick de status: Poison/Regen/Duration--; reseta AP = 3)
   │           ▼
   │    ┌──────────────┐
   │    │ ActionSelect │  ◀─┐  loop interno: jogador/AI escolhe ação enquanto AP > 0 e não "passar"
   │    └──────┬───────┘   │  (Scan / Gambito / atacar / defender / jogar carta / flee / passar)
   │           │           │
   │           ▼           │
   │    ┌──────────────┐   │
   │    │ ActionResolve│───┘  resolve a ação escolhida (dano, status, pipeline de combo,
   │    └──────┬───────┘      reordenar fila, etc). Volta a ActionSelect se AP > 0.
   │           ▼  (AP == 0 ou passou)
   │    ┌──────────────┐
   │    │   TurnEnd    │  (expira status com Duration==0 do ator; dispara CombatBus.TurnEnded)
   │    └──────┬───────┘
   │           ▼
   │    ┌──────────────┐
   │    │  CheckEnd    │  (party morta? inimigos mortos? flee bem-sucedido?)
   │    └──────┬───────┘
   │   não-fim │  fim
   └───────────┘  ▼
            ┌──────────────┐
            │  CombatEnd   │  (vitória/derrota/fuga; dispara CombatBus.CombatEnded + payload)
            └──────────────┘
```

### Notas de transição

- `SetupPhase → TurnStart`: a fila já está ordenada por SPD; o primeiro ator entra.
- `TurnStart → ActionSelect`: mana e AP do ator são preparados ANTES da seleção.
- `ActionSelect ⇄ ActionResolve`: laço interno. Cada ação custa AP. Quando `AP == 0` ou o ator escolhe "passar", sai do laço.
- `TurnEnd → CheckEnd → TurnStart`: avança para o próximo ator na fila. `turnoIndex` é por rodada completa de fila (afeta o ramp de mana de forma consistente entre todos).
- Qualquer ator que chega a HP 0 é resolvido conforme regra de morte/incapacitação (companions incapacitados, Pillar 4; inimigos removidos da fila).

---

## 4. Iniciativa e fila visível

A ordem de turnos é uma **fila ordenada por SPD, sempre visível** ao jogador (UI mostra próximos N atores). Isto é mecânica central, não cosmético: o Gambito opera sobre esta fila.

### Operação `ReorderActor(actor, deltaPosicao)`

Move um ator `deltaPosicao` casas na fila (negativo = adiantar, positivo = atrasar). É a primitiva que cartas Cinético/Sônico e o Gambito "reordenar/redirecionar vetor" usam para empurrar um inimigo para trás na ordem (ou puxar um aliado para frente, quando aplicável).

- Clamp nos limites da fila (não sai do range válido).
- Knockback (status, ver §9) tipicamente chama `ReorderActor(alvo, +1)`.
- Gambito-reordenar (2 AP) chama `ReorderActor(alvo, +deltaUpgradavel)`.
- A fila é recomputada por SPD apenas na entrada de novos atores ou em mudança de SPD (Haste/Slow); reordenações manuais persistem até a próxima recomputação natural.

---

## 5. Recursos: AP e Mana

Dois recursos, propósitos distintos. AP limita **quantas** ações; Mana limita **quão caras** as cartas.

### AP (Action Points)

- `AP = 3` fixo por turno no vertical slice.
- Reseta no `TurnStart`. Sem carry-over.
- Cresce via skill tree em jogo posterior (fora do escopo do slice; o campo é parametrizável).
- **Por-ator** (CTB, §3): AP pertence ao ator ativo no turno. Cada membro da party tem seus 3 AP independentes quando age na fila. Canonizado D.1 Sprint 1 W2, 2026-06-03.

### Mana / Compilação

- `manaMax = 2 + turnoIndex`, com `cap = 8`.
- Recarrega ao **máximo** a cada `TurnStart`. **Sem carry-over** (impossível bankar mana entre turnos: anti-degeneração, ver §13).
- Ramp linear garante que combos premium só ficam viáveis no mid-late do combate, preservando curva de tensão.
- **Por-ator** (CTB, §3): cada ator tem seu próprio pool de mana independente. Sem pool compartilhado entre personagens. Canonizado D.1 Sprint 1 W2, 2026-06-03.

### Tabela de custos canônica

| Ação | Custo AP | Custo Mana |
|---|---|---|
| Ataque básico | 1 | 0 |
| Defender | 1 | 0 |
| Scan | 1 | 0 |
| Gambito: Prever | 1 | 0 |
| Gambito: Reordenar / forçar-recuo | 2 | 0 |
| Jogar carta | 1 | custo da carta (`ManaCost`) |
| Flee | 1 | 0 |
| Passar (encerra turno) | 0 | 0 |

Modificadores anexados a cartas somam mana (Object +1, Stream +2, Null +1; ver §8).

---

## 6. Famílias de carta e roda de fraqueza

### As 5 famílias (identidade não-sobreposta)

Cada família tem identidade mecânica distinta: a sobreposição de papéis é proibida (anti feature-creep). Mapeamento de companion para família estabelece fantasia e cor de jogo.

| Família | Companion-âncora | Identidade mecânica | Status que aplica |
|---|---|---|---|
| **Elétrico** | Cauã "Volt" | burst de dano alto, single-target | Stun |
| **Bioquímico** | Jaci "Proxy" | dano-ao-longo-do-tempo (DoT), degradação | Poison / Corrode |
| **Sônico** | Linda "Siren" | controle de área (área-CC), interrupção | Disrupt, Silence |
| **Cinético** | Bento "Requiem" | impacto físico, deslocamento de fila | Knockback, Break |
| **Criptográfico** | Iara "Lumen" | utilidade, anti-buff, exposição | Expose, Decrypt |

Gus = **todas as famílias** (protagonista é o compilador universal; companions são especialistas).

> **Representação no engine (canon 2026-06-03, F2-E.10):** a universalidade do Gus NÃO é um valor da enum `CardFamily` (a roda de fraqueza mantém só os 5 elementais puros). É uma **flag `IsUniversalCompiler`** no `CharacterTemplate`: o deck do Gus pode conter qualquer família; companions ficam travados na sua. Defesa do Gus = neutra (sem fraqueza de família amplificada; a fragilidade dele vem do HP/contrato §2.1, não de tipagem). Implementação da flag + consumo em combate = F2-E.10b (CharacterRepository).

### Roda de fraqueza fechada e determinística

Ciclo fechado de 5 elos. Cada família é **forte contra** a seguinte no sentido da seta. Não há ambiguidade: a relação é determinística e revelada por Scan.

```
Elétrico → Cinético → Criptográfico → Sônico → Bioquímico → Elétrico
   (forte)    (forte)      (forte)       (forte)     (forte)    (fecha o ciclo)
```

Leitura: Elétrico é forte contra Cinético; Cinético forte contra Criptográfico; Criptográfico forte contra Sônico; Sônico forte contra Bioquímico; Bioquímico forte contra Elétrico.

- **Fraco** (atacante forte contra alvo): `multFraqueza = 1.5`.
- **Neutro** (sem relação na roda): `multFraqueza = 1.0`.
- **Resistente** (alvo forte contra atacante, sentido inverso da seta): `multFraqueza = 0.66`.
- **Imune** (caso especial de inimigo/lore): `multFraqueza = 0.0`.

A fraqueza de cada inimigo é **oculta até o 1º Scan** (Pillar 1: 1º encontro = surpresa; 2º+ = informação via hardware). Isto força o jogador a variar de família em vez de spammar uma só (anti-degeneração).

---

## 7. Modelo de carta (modelo B: carta-base + modificador em runtime)

Carta é um record imutável de dados. Modificadores são anexados em runtime (no momento de jogar ou montar pipeline), não pré-bakeados na carta.

### Sintaxe diegética

`tipo.família → modificador → alvo`

Exemplos legíveis: `pulso.elétrico → stream → linha-3`; `raiz.bioquímico → object → célula-tática`; `eco.sônico → null → grupo`.

A sintaxe é apresentada na UI exatamente nesse formato (Pillar 2: gramática internamente consistente e legível).

### Campos do record (conceitual)

| Campo | Tipo | Significado |
|---|---|---|
| `Id` | string/enum | identidade única estável (chave i18n, save) |
| `DisplayName` | string (via `tr()`) | nome diegético localizado |
| `Family` | enum `CardFamily` | uma das 5 famílias |
| `BaseType` | enum `CardBaseType` | tipo-base (pulso, raiz, eco, fenda, glifo...) |
| `ManaCost` | int | custo de mana base |
| `ApCost` | int | custo de AP (padrão 1) |
| `Power` | int | potência base entrando na fórmula de dano |
| `TargetShape` | enum `TargetShape` | single / linha / área-3x3 / grupo / self |
| `StatusApplied` | `StatusEffect?` | status que a carta aplica (pode ser nulo) |
| `Modifiers` | lista de `CardModifier` | modificadores anexados em runtime |
| `Mastery` | int (0..N) | nível de mestria por uso (Pillar 1; cresce por uso, não por kill) |
| `CritChance` | int (0..100) | porcentagem de crit (0 = sem crit). Visível na UI antes de confirmar a ação. |

---

## 8. Modificadores de carta (3)

Modificadores transformam a carta-base no momento de uso. Cada um soma mana.

| Modificador | Efeito | Custo extra de mana | Pré-condição |
|---|---|---|---|
| **Object** | cria uma entidade persistente no campo (armadilha, totem, célula-tática) que dura turnos e tem comportamento próprio | +1 | nenhuma |
| **Stream** | converte single-target em área OU em repetição (multi-hit / tick adicional) | +2 | nenhuma |
| **Null** | inverte ou cancela um efeito (buff inimigo, status, propriedade) | +1 | **requer Scan prévio** no alvo |

`Null` depende de Scan porque inverter/cancelar exige conhecer o que existe (Pillar 1: informação habilita ação). Sem Scan, Null fica indisponível na UI (botão desabilitado com tooltip "requer Scan").

---

## 9. Status framework

Status é record uniforme aplicado a qualquer ator. Tick processado no `TurnStart` do ator afetado.

### Campos do record (conceitual)

| Campo | Tipo | Significado |
|---|---|---|
| `Id` | enum `StatusId` | identidade do status |
| `Magnitude` | int | intensidade (dano por tick, % de redução etc) |
| `Duration` | int | turnos restantes (decrementa no TurnStart; expira em TurnEnd quando chega a 0) |
| `StackRule` | enum `StackRule` | como se acumula: `Replace` / `Refresh` / `StackMagnitude` / `StackDuration` |
| `FamilyOrigin` | enum `CardFamily` | família que originou (para Null/Decrypt e telemetria de balance) |

`Dispel` remove um status (via carta utilitária ou Null sobre status hostil).

### Lista base de status

| Status | Família origem | Efeito |
|---|---|---|
| **Stun** | Elétrico | ator perde o próximo turno |
| **Poison / Corrode** | Bioquímico | dano por tick no TurnStart (Corrode reduz Def além do dano) |
| **Disrupt** | Sônico | reduz eficácia da próxima ação (penalidade de Power) |
| **Silence** | Sônico | bloqueia jogar cartas (só ataque básico/defender/flee) |
| **Knockback** | Cinético | empurra o alvo na fila: `ReorderActor(alvo, +1)` |
| **Break** | Cinético | reduz Def do alvo por Duration turnos |
| **Expose** | Criptográfico | aumenta dano de carta recebido pelo alvo (ver detalhe abaixo) |
| **Decrypt** | Criptográfico | anula/remove TODOS os buffs do alvo (qualquer status benéfico); NÃO bloqueia reaplicação (reset, não lockout; alvo pode re-buffar via item/poção/carta). Canon revisado 2026-06-03 (F2-E.5b) |
| **Shield** | utilitário | pool de absorção: absorve TODO dano antes do HP (ver detalhe abaixo) |
| **Regen** | utilitário | cura por tick no TurnStart |
| **Haste / Slow** | utilitário | aumenta/reduz SPD (recomputa posição na fila) |

> **Forma dos efeitos (ratificada 2026-06-03, F2-E.5b — wiring real dos status).** Magnitudes vêm sempre da carta/combo que aplica (parametrizado), nunca hardcoded:
> - **Disrupt:** penalidade multiplicativa de Power `× (1 - Magnitude/100)`, consumida na 1ª carta ofensiva do alvo (literal "próxima ação").
> - **Break:** redução de Def aplicada de uma vez, mantida e restaurada ao expirar. Distinta de Corrode (que reduz Def por tick, acumulativo).
> - **Haste / Slow:** **aditivo** `SPD ±= Magnitude` (clamp 0), recomputa a fila na aplicação e no expire; restaura ao expirar. (NÃO multiplicativo: previsibilidade tática + equilibra o lento + casa com a notação "+1/-1/magnitude N" dos efeitos de ambiente §18.)
> - **Decrypt:** dispela qualquer buff benéfico; SEM lockout (ver linha acima).
> - **Knockback:** `ReorderActor(alvo, +1)`; NÃO custa o turno corrente do alvo (mantém-se distinto de Stun, que é quem tira o turno).
> - **Silence:** gate em `ResolveUseCard` com mensagem ERRO DE COMPILAÇÃO (§10); ataque básico / defender / flee passam.

### Shield (pool de absorção), canonizado 2026-05-26

`Magnitude` é um **pool de absorção** que protege o HP de QUALQUER fonte de dano (carta, ataque básico, tick de DoT). A cada dano:

```
absorvido  = min(dano, Magnitude)
Magnitude -= absorvido
Hp        -= (dano - absorvido)
```

- O Shield é **removido imediatamente** quando o pool zera (`Magnitude <= 0`), OU expira por `Duration` (o que vier primeiro).
- Implementado em `CombatActor.TakeDamage` (consulta o próprio Shield antes de tocar o HP). `Defend` deixa de ser no-op: aplica Shield com `Magnitude = Def` e passa a efetivamente reduzir o dano recebido.
- Registra mudança de status (§16): `Absorbed` a cada absorção (Magnitude = pool restante), `Expired` ao depletar o pool.

### Expose (amplificação de dano de carta), canonizado 2026-05-26

Alvo com Expose recebe dano de **UseCard** multiplicado por:

```
multExpose = alvoTemExpose ? (1 + Expose.Magnitude / 100) : 1.0
```

- Aplica **somente em UseCard**, como último fator da cadeia divisiva (§11). O ataque básico subtrativo `clamp(Atk - Def, 1)` **não** é afetado.
- Ex: `Expose.Magnitude = 30` → dano de carta ×1.3.

---

## 10. Stack pipeline de 3 slots e combos

O Tavus-Drive resolve uma **pipeline de até 3 slots** por sequência de compilação. Cada slot recebe uma carta OU um modificador. A pipeline resolve sequencialmente (slot 1, 2, 3). Quando a sequência casa uma **receita** registrada, dispara um **combo nomeado determinístico** com feedback de UI explícito: `COMPILADO: <NomeDoCombo>`.

### Estrutura da pipeline

- 3 slots, preenchidos em ordem.
- Cada slot: `{ kind: Card | Modifier, ref: Id }`.
- Resolução sequencial: aplica efeito de cada slot na ordem; modificadores no pipeline afetam a carta imediatamente anterior preenchida (ou a próxima, conforme receita).
- Casamento de receita: ao fechar a pipeline (1, 2 ou 3 slots), o resolvedor consulta a **tabela de receitas** procurando uma assinatura exata.
- Combos são **determinísticos** (Pillar 2: combos são receitas, não RNG). Nenhum combo é aleatório.

### Formato da tabela de receitas (estrutura canônica)

A tabela completa (~200 combos) é trabalho futuro. Esta spec define o **formato**. Cada receita:

| Campo | Significado |
|---|---|
| `ComboId` | identidade estável |
| `DisplayName` | nome diegético do combo (chave `tr()`) |
| `Signature` | sequência ordenada de slots que dispara (ex: `[Card:pulso.elétrico, Modifier:stream, Card:raiz.cinético]`) |
| `ResultEffect` | efeito resultante (dano, status, entidade, `multCombo`) |
| `multCombo` | multiplicador de dano aplicado na fórmula (§11) |
| `Discoverable` | bool (combo curado vs easter-egg descobrível por experimentação) |

Receita é casada por **assinatura exata** de família + base + modificador, não por instância de carta específica. Isto mantém ~200 receitas gerenciáveis e descobríveis (Pillar 2: ~200 combinações pré-planejadas + 5-10 easter-egg combos).

### Feedback de erro de compilação (Pillar 2, N.2 R2 2026-06-03)

Toda tentativa de jogar carta que falha em pré-condição DEVE exibir mensagem de erro explícita na UI (não só desabilitar o botão silenciosamente):

| Pré-condição falha | Mensagem |
|---|---|
| Mana insuficiente | `ERRO DE COMPILAÇÃO: mana insuficiente (custa X, tem Y)` |
| AP insuficiente | `ERRO DE COMPILAÇÃO: AP insuficiente` |
| Alvo inválido | `ERRO DE COMPILAÇÃO: alvo inválido para <família>` |
| Null sem Scan prévio | `ERRO DE COMPILAÇÃO: Null requer Scan prévio no alvo` |
| Pipeline cheia | `ERRO DE COMPILAÇÃO: pipeline já contém 3 slots` |

Feedback textual visível reforça a metáfora de compilação (Pillar 2: magia é sistema formal com gramática legível e erros detectáveis).

---

## 11. Fórmula de dano (canonizada 2026-05-26)

Duas fórmulas distintas: **UseCard é divisiva** (Def reduz por fração, escala bem sem zerar), **ataque básico é subtrativo** (recurso simples sempre-disponível, 0 mana).

### Fórmula UseCard (divisiva)

```
multExpose     = alvoTemExpose ? (1 + Expose.Magnitude / 100) : 1.0
danoBase       = (Power + Atk) × (100 / (100 + Def)) × multFraqueza × multMod × multCombo × multExpose × multAmbiente
varianceFactor = max(0.05, 0.30 × e^(-knowledgeKills × 0.10))
rolled         = danoBase × rand(1 - varianceFactor, 1 + varianceFactor)
danoFinal      = round( rolled × (isCrit ? 1.5 : 1.0) )
isCrit         = rand(0..99) < card.CritChance
```

- **Imune** (`multFraqueza == 0.0`): `danoFinal = 0`.
- **Sem clamp mínimo**: a divisiva nunca chega a 0 contra não-imunes (frações muito pequenas podem arredondar pra 0, telegrafando "elemento errado / Def alta demais").
- **`multExpose`** (§9, canon 2026-05-26): penúltimo fator da cadeia divisiva, ANTES da variância/crit. Só UseCard; o ataque básico não usa. Sem Expose no alvo = 1.0.
- **`multAmbiente`** (§18, canon 2026-05-26): último fator da cadeia divisiva. Produto dos multiplicadores das camadas ambientais ATIVAS (terreno + clima + período) que afetam a família da carta jogada. Default 1.0 (encontro sem ambiente marcado). Faixa por camada 0.66 a 1.5. **NUNCA toca `multFraqueza`** (a roda de fraqueza é fator independente; ambiente nunca altera a relação de fraqueza). Só UseCard; o ataque básico subtrativo não usa.
- **Ordem**: base divisiva (incl. `multExpose` e `multAmbiente`) → variância Knowledge → crit → um único `round` no final.
- RNG **injetável e seedável** (`CombatStateMachine(..., Random? rng = null)`; default `Random.Shared`, testes injetam semente fixa). Pillar 1/2: variância é transparente, não opaca.

**Stacking das 3 camadas ambientais** (terreno + clima + período, canonizado 2026-05-26): os multiplicadores das camadas ativas que afetam a mesma família **se multiplicam** entre si, com **cap final `multAmbiente ∈ [0.44, 2.25]`** (mesmo teto que o sistema já permite via 1.5 × 1.5 = 2.25 e piso via 0.66 × 0.66 ≈ 0.44). A curadoria de transições (§18.6) impede por design que 2 fontes ×1.5 da MESMA família coexistam (nunca atingir 2.25 organicamente fora de janela curta); o cap é a trava de segurança numérica.

| Fator | Origem | Valores |
|---|---|---|
| `Power` | carta | campo do record |
| `Atk` | atributo do atacante | stat |
| `multFraqueza` | roda de fraqueza (§6) | 1.5 / 1.0 / 0.66 / 0.0 |
| `multMod` | modificador aplicado | default 1.0; Stream pode distribuir; valores tabelados |
| `multCombo` | receita de combo (§10) | default 1.0; >1.0 em combo casado |
| `multExpose` | status Expose no alvo (§9) | default 1.0; `1 + Expose.Magnitude/100` se Expose presente; só UseCard |
| `multAmbiente` | camadas ambientais ativas (§18) | default 1.0; produto das camadas (terreno+clima+período) por família; só UseCard; nunca toca `multFraqueza`; cap [0.44, 2.25] (canonizado + implementado F2-E.11, ADR-004) |
| `Def` | atributo do defensor | divisor `100/(100+Def)`; reduzido por Break/Corrode |
| `knowledgeKills` | `CombatActor.KnowledgeKills` (SaveSystem) | kills do mesmo tipo de inimigo; alimenta o decaimento de variância |
| `card.CritChance` | carta | 0..100 (%) ; 0 = sem crit |

**Variância Knowledge Decay**: 1º encontro (`kills=0`) → ±30%; conforme o player farma o mesmo tipo, decai exponencialmente até o piso ±5% (`kills` altos). Contra inimigos muito farmados o dano fica quase determinístico (Knowledge Progression: conhecer o inimigo remove a incerteza).

### Fórmula ataque básico (subtrativa)

```
danoFinal = clamp(Atk - Def, 1)
```

Sem variância, sem crit, sem fraqueza de família. Recurso de 0 mana sempre disponível.

### XP pós-batalha (differential por zona)

```
xp = base_xp × max(0, 1 - (player_zone - enemy_zone) × 0.15)
```

Penaliza farmar inimigos de zonas muito abaixo da do player (anti-grind degenerado). Implementado em `game/` ao integrar `PlayerBus.CombatResultReceived` (F2-G.8) — **não** vive no POCO de combate do engine.

---

## 12. Sistemas de leitura e manipulação (Scan + Gambito)

### Scan (Óculos + Matriz)

- Custo: 1 AP.
- 1ª vez em um inimigo: revela **HP** e **fraqueza** (família).
- Upgrades (jogo posterior): revelam buffs/debuffs ativos e posição na fila do inimigo.
- Vira **passivo permanente** contra um tipo de inimigo após N kills (Knowledge Progression): deixa de custar AP contra inimigos farmados.
- Habilita **Null** e **Expose** (pré-condição informacional).

### Gambito (Óculos Táticos)

Dois modos, ambos operam sobre a fila/intent:

**Prever** (1 AP):
- Lê o `IntentPreview` do inimigo (alvo, dano, área do próximo turno).
- Upgrade (jogo posterior): 2-3 turnos à frente para um inimigo escolhido.
- **Falha contra intent caótico exclusivamente de Patch-Zero**: retorna ruído Perlin em vez de leitura limpa. Mini-bosses têm UtilityBrain sem ruído (intent complexo, mas legível). (N.2 R3, one-way door 2026-06-03.)

**Reordenar / forçar-recuo** (2 AP):
- Empurra um inimigo na fila via `ReorderActor`, OU força mudança de alvo do inimigo.
- **Sacrifica dano**: gastar AP em Gambito é AP não gasto em ataque (trade-off explícito de risk vs reward).

---

## 13. Inteligência artificial inimiga

Contrato de dados central: toda AI implementa `IEnemyBrain` e é **obrigada** a expor `IntentPreview`. Isto é o contrato de telegraph: Scan e Gambito leem `IntentPreview`; um inimigo sem intent legível quebraria o pillar de informação.

```
interface IEnemyBrain {
    IntentPreview PreviewIntent(CombatState state);   // contrato telegraph obrigatório
    EnemyAction   DecideAction(CombatState state);    // ação efetiva no turno
}
```

| Nível | Brain | Comportamento |
|---|---|---|
| **Trash** | `ScriptedBrain` | determinístico, roteiro fixo. Intent 100% legível. |
| **Elite** | `UtilityBrain` | pontuação de utilidade por ação (escolhe melhor jogada por heurística). Intent legível. |
| **Mini-boss** | `UtilityBrain` | utility sem ruído Perlin; intent legível porém complexo. Gambito-Prever funciona normalmente. |
| **Boss (Patch-Zero)** | `UtilityBrain` + **ruído Perlin exclusivo** | caos irredutível reservado EXCLUSIVAMENTE para Patch-Zero; Gambito-Prever retorna ruído. N.2 R3, one-way door 2026-06-03. |

**Vertical slice F2-E.5 entrega apenas `ScriptedBrain` + a interface `IEnemyBrain`** (com `IntentPreview`). `UtilityBrain` e a camada de ruído ficam para jogo posterior, mas a interface já contempla os três níveis.

---

## 14. Flee (fuga)

- Custo: 1 AP.
- Chance baseada em SPD relativo da party vs inimigos (fórmula determinística com componente RNG visível; no slice, sucesso/falha determinístico por threshold de SPD).
- **Bloqueado em mini-boss e boss** (botão desabilitado, tooltip "fuga impossível").

---

## 15. Anti-degeneração (consolidado)

Cada item abaixo é uma trava de design contra estratégia dominante ou jogo "resolvido". Validado contra anti-pillars do GDD/pillars.md.

| Trava | Como impede degeneração |
|---|---|
| **Sem win-button** | nenhuma carta resolve sozinha; pipeline e mana ramp limitam |
| **Mana ramp linear** | combos premium só no mid-late; sem alpha-strike no turno 1 |
| **Fraqueza força variar** | roda de fraqueza recompensa trocar de família; spam de uma família perde eficiência |
| **Sem grind (Knowledge)** | farmar reduz XP e aumenta conhecimento; não há power-creep por nível |
| **RNG visível** | porcentagem mostrada + seedável + Gambito re-roll/cancela; variância nunca pune skill às cegas |
| **Intent caótico** | Patch-Zero resiste a predição total (boss final exclusivo; mini-bosses têm intent legível porém complexo. N.2 R3.) |
| **Sem mana banking** | mana recarrega ao máximo e não acumula; impossível estocar pra combo gigante |
| **Roda fechada** | relação de fraqueza é determinística e completa; sem família "sempre melhor" |
| **Clamp dano mínimo 1** | impede build de Def infinita que zera dano (exceto imunidade telegrafa) |
| **AP escasso (3)** | toda ação compete por AP; Scan/Gambito custam o turno de ataque (trade-off real) |

---

## 16. Integração com event bus

O combate comunica via dois buses desacoplados (arquitetura de engine; ver `docs/tech/architecture.md`).

### `CombatBus` (eventos internos de combate)

- `CombatStarted(encounter)`
- `TurnStarted(actor, turnoIndex)`
- `ActionResolved(actor, action, result)`
- `StatusApplied(actorId, statusId, magnitude, duration)` / `StatusExpired(actorId, statusId)`
- `ActorDefeated(actor)` / `ActorIncapacitated(companion)`
- `TurnEnded(actor)`
- `CombatEnded(outcome, payload)` // vitória / derrota / fuga

#### Contrato de evento de status (canonizado 2026-05-26)

O POCO **não** emite signals diretamente (mantém-se sem dependência Godot). Em vez disso a FSM **acumula uma lista de `StatusEffectChange`** (mesmo padrão do `Log` de `CombatLogEntry`), drenada dos atores em cada transição de fase relevante (tick / resolução / expire). O `CombatManager` (ponte Node) lê essa lista pós-turno por índice e traduz em signals:

```
StatusEffectChange(ActorId, StatusId Id, StatusChangeKind Kind, int Magnitude, int Duration)
StatusChangeKind = { Applied, Expired, Absorbed }
```

- `Applied` (carta `StatusApplied`, combo `ResultStatus`, Defend/Shield) → `CombatBus.StatusApplied(actorId, statusId, magnitude, duration)`.
- `Expired` (Duration ≤ 0 no TurnEnd, OU depleção de pool de Shield) → `CombatBus.StatusExpired(actorId, statusId)`.
- `Absorbed` (Shield absorveu dano; Magnitude = pool restante) → emitido como `StatusApplied` (atualiza a barra de Shield na UI sem alargar o contrato do bus).

`StatusId` enum é mapeado pra string via `ToString()` (mesma convenção dos demais signals). Validado headless em `game/tools/TestCombatIntegration.cs`.

### `PlayerBus` (acoplamento com sistemas persistentes)

- consome `CombatEnded` para conceder XP, Knowledge, mestria de carta, loot narrativo.
- consome `ActorIncapacitated` para acionar fluxo hospital/economia (Pillar 4).
- fornece estado do deck em campo (15 cartas) e atributos da party para `SetupPhase`.

O combate **não** referencia diretamente sistemas de progressão; emite eventos. Isto mantém o módulo de combate reutilizável (engine vs game-specific).

---

## 17. Escopo do vertical slice F2-E.5

Subconjunto mínimo implementável via TDD. Tudo abaixo é entregável no slice; o resto da spec é jogo posterior mas a interface/dados já contemplam.

### Entrega (DEVE)

1. **FSM completa**: SetupPhase, TurnStart, ActionSelect (com loop interno por AP), ActionResolve, TurnEnd, CheckEnd, CombatEnd. Cobertura de testes por transição.
2. **Fila de iniciativa por SPD** + operação `ReorderActor(actor, deltaPosicao)` com clamp. Fila visível (dado exposto; UI mínima).
3. **AP = 3 fixo** por turno, reset no TurnStart.
4. **Mana ramp**: `manaMax = 2 + turnoIndex`, cap 8, recarrega ao máximo, sem carry-over.
5. **Records de dados**: `Card`, `CardFamily`, `StatusEffect`, mais enums de suporte (`CardBaseType`, `TargetShape`, `StackRule`, `StatusId`). Imutáveis.
6. **5 famílias como dados** + roda de fraqueza determinística como tabela consultável (`multFraqueza` por par atacante/alvo).
7. **3-4 status concretos**: `Stun`, `Poison`, `Shield`, `Expose` (cobrem aplicar / tick no TurnStart / expirar por Duration / StackRule / dispel).
8. **3 modificadores**: `Object`, `Stream`, `Null` (Null com pré-condição de Scan validada em teste).
9. **Pipeline de 3 slots** com casamento de receita e 1-2 combos mockup (assinatura + `multCombo` + nome `COMPILADO: X`).
10. **Scan**: 1 AP, revela HP + fraqueza; flag de "scaneado" habilita Null/Expose.
11. **Gambito 2 modos**: Prever (1 AP, lê `IntentPreview`) e Reordenar (2 AP, chama `ReorderActor`).
12. **`ScriptedBrain`** + interface `IEnemyBrain` com `IntentPreview` (telegraph determinístico).
13. **Flee stub**: 1 AP, threshold de SPD determinístico; bloqueado se flag boss/mini-boss.
14. **Integração `CombatBus` + `PlayerBus`**: emitir eventos da §16; PlayerBus mock recebe `CombatEnded`.
15. **Subsistema de ambientes de combate (§18)**: o slice inclui o **catálogo completo** (decisão do criador: "todos os ~15" terrenos + 8 climas + 4 períodos como dados). `multAmbiente` default 1.0 mantém **retrocompatibilidade** (encontros sem ambiente marcado funcionam exatamente igual). Implementação no slice: record `EnvironmentModifier` no engine POCO (§16); **aplicação estática por arena** (level-designer marca o terreno/clima/período inicial do encontro); verb **Scan-ambiente** (1 AP) para revelar tier Codex. **Cartas-clima e inimigos que invocam ambiente = fase posterior** (os dados já são preparados para isso).

### Fora do slice (jogo posterior, interface já plugada)

- Alimentação real de `KnowledgeKills` pelo SaveSystem (curva de decaimento de variância já implementada no engine; no slice o valor vem fixo até a integração de persistência).
- `UtilityBrain` para mini-bosses e Elite.
- Ruído Perlin exclusivo de Patch-Zero (boss final; N.2 R3).
- Tabela completa de ~200 combos (slice usa 1-2 mockups; formato §10 já definido).
- Upgrades de Scan/Gambito (multi-turno, buffs/posição).
- Skill tree que cresce AP e Scan-passivo (campos parametrizáveis já existem).
- Fluxo hospital/economia (evento `ActorIncapacitated` já emitido).

### Contratos de dados (records C# conceituais, para orientar TDD)

```csharp
// Imutáveis. Tipos ilustrativos; nomes finais conforme convenção do projeto.

public enum CardFamily { Eletrico, Bioquimico, Sonico, Cinetico, Criptografico }
public enum CardBaseType { Pulso, Raiz, Eco, Fenda, Glifo }
public enum TargetShape { Self, Single, Linha, Area3x3, Grupo }
public enum CardModifier { Object, Stream, Null }
public enum StackRule { Replace, Refresh, StackMagnitude, StackDuration }
public enum StatusId { Stun, Poison, Corrode, Disrupt, Silence, Knockback, Break, Expose, Decrypt, Shield, Regen, Haste, Slow }
public enum WeaknessTier { Fraco, Neutro, Resistente, Imune } // 1.5 / 1.0 / 0.66 / 0.0

public readonly record struct StatusEffect(
    StatusId Id,
    int Magnitude,
    int Duration,
    StackRule StackRule,
    CardFamily FamilyOrigin
);

public readonly record struct Card(
    string Id,
    string DisplayName,          // resolvido via tr() na UI
    CardFamily Family,
    CardBaseType BaseType,
    int ManaCost,
    int ApCost,                  // default 1
    int Power,
    TargetShape TargetShape,
    StatusEffect? StatusApplied,
    IReadOnlyList<CardModifier> Modifiers,
    int Mastery,                 // cresce por uso
    int CritChance = 0           // 0..100 (%); 0 = sem crit
);

public readonly record struct ComboRecipe(
    string ComboId,
    string DisplayName,          // "COMPILADO: X" via tr()
    IReadOnlyList<PipelineSlot> Signature,
    StatusEffect? ResultStatus,
    float MultCombo,
    bool Discoverable
);

public readonly record struct PipelineSlot(
    PipelineSlotKind Kind,       // Card | Modifier
    string Ref                   // Card.Id ou CardModifier
);

public enum PipelineSlotKind { Card, Modifier }

public readonly record struct IntentPreview(
    string ActorId,
    string PredictedActionId,
    int PredictedDamage,
    TargetShape PredictedShape,
    string PredictedTargetId,
    bool IsChaotic               // true => Gambito-Prever retorna ruído; EXCLUSIVO Patch-Zero (N.2 R3, one-way door 2026-06-03)
);

public interface IEnemyBrain {
    IntentPreview PreviewIntent(CombatState state);
    EnemyAction   DecideAction(CombatState state);
}

// Fórmula de dano UseCard (divisiva, canon 2026-05-26 — ver §11 pra detalhamento):
//   baseDamage = (Power + Atk) * (100f / (100f + Def)) * multFraqueza * multMod * multCombo;
//   v          = MathF.Max(0.05f, 0.30f * MathF.Exp(-knowledgeKills * 0.10f));
//   rolled     = baseDamage * (1 + (v * 2 * rng.NextDouble() - v));
//   if (CritChance > 0 && rng.Next(100) < CritChance) rolled *= 1.5f;
//   danoFinal  = multFraqueza == 0f ? 0 : (int)MathF.Round(rolled);  // sem clamp mínimo
// Ataque básico (subtrativo): danoFinal = Math.Max(1, Atk - Def);   // sem variância/crit/fraqueza
```

### Roda de fraqueza (tabela de dados para teste)

| Atacante \ Alvo | Elétrico | Cinético | Criptográfico | Sônico | Bioquímico |
|---|---|---|---|---|---|
| **Elétrico** | 1.0 | **1.5** | 1.0 | 1.0 | 0.66 |
| **Cinético** | 0.66 | 1.0 | **1.5** | 1.0 | 1.0 |
| **Criptográfico** | 1.0 | 0.66 | 1.0 | **1.5** | 1.0 |
| **Sônico** | 1.0 | 1.0 | 0.66 | 1.0 | **1.5** |
| **Bioquímico** | **1.5** | 1.0 | 1.0 | 0.66 | 1.0 |

(Linha = atacante, coluna = alvo. 1.5 = fraco/forte-contra; 0.66 = resistente; 1.0 = neutro. Imune 0.0 é caso especial de inimigo, não da roda base.)

### Stats de referência do encontro (canônico D.1 Sprint 1 W2, 2026-06-03)

Stats para TDD do encontro F2-E.5. HP pós-inflação +60% (Trash 34→55, Elite 89→144; sequência Fibonacci; TTK alvo 3-5 turnos).

**Party (Gus + 2 companions ativos):**

| Personagem | HP | Atk | Def | SPD | Família |
|---|---|---|---|---|---|
| Gus Vector Tavus Vance | **34** | 8 | 5 | 9 | Todas (compilador universal) |
| Cauã "Volt" Berenger | 55 | 14 | 8 | 13 | Elétrico |
| Jaci "Proxy" Vanderbist | 55 | 9 | 10 | 7 | Bioquímico |

Obs.: HP Gus (34) é o menor da party por hard cap canônico (§2.1).

**Inimigos do encontro de referência:**

| Inimigo | Tier | HP | Def | Tipo | Fraqueza (1.5×) | Brain (slice) |
|---|---|---|---|---|---|---|
| Sentinela-Bit | Trash | **55** | 8 | Cinético | Elétrico | ScriptedBrain |
| Daemon-Guard | Elite | **144** | 14 | Cinético | Elétrico | ScriptedBrain (placeholder; UtilityBrain = jogo posterior) |

Notas: Atk e SPD dos inimigos = TBD (definir na implementação F2-E.5). Cauã (Elétrico) é o DPS natural deste encontro pela roda de fraqueza (§6).

---

## 18. Sistema de Ambientes de Combate (terreno + clima + período)

Camada de ambiente que materializa o setting bipartido (Pillar 5: cidade ciber-gótica × Selve Sombria) e a natureza-matemática (Pillar 2) dentro da arena de combate. O ambiente nunca substitui a leitura de sistema (Scan/Gambito): ele **adiciona uma dimensão de decisão posicional e temporal** que o jogador lê via hardware e responde via escolha de família. Toda regra abaixo respeita os anti-pillars: sem RNG punitivo, sem hardware inútil, sem teto cognitivo estourado.

### 18.1 Arquitetura

Três **camadas simultâneas**, cada uma com sabor e ritmo próprios:

| Camada | Persistência | Quem define |
|---|---|---|
| **Terreno** | fixo por encontro | level-designer marca a arena |
| **Clima** | transitório, muda a cada N turnos | curadoria de transições (§18.6) ou cartas/inimigos (fase posterior) |
| **Período** | roda temporal de 4 fases (ciclo automático) | motor de ciclo (Selve natural / cidade = grade elétrica) |

Efeito mecânico de cada ambiente (qualquer camada) age por até quatro canais:

1. **`multAmbiente` por família** (faixa 0.66 a 1.5), entra na fórmula §11 como fator separado, **nunca na roda de fraqueza**.
2. **Facilita um status do enum existente** (§9: Stun / Poison / Corrode / Disrupt / Silence / Knockback / Break / Expose / Decrypt / Shield / Regen / Haste / Slow). **Nenhum status novo é criado.** "Root" mencionado neste documento = `Slow` de magnitude extrema (não é status novo).
3. **Efeitos em fila / SPD** (deltas via `ReorderActor` e Haste/Slow).
4. **Interação com hardware** (triângulo Pillar 3: Óculos/Scan, Matriz Ortodôntica, Tavus-Drive).

Princípios não negociáveis desta camada:

- **Dados no engine POCO**: record `EnvironmentModifier` (NÃO Resource Godot; mantém o engine puro, §16). Aplicação estática por arena no slice; cartas/inimigos que invocam ambiente = fase posterior (os dados já são preparados para isso).
- **Telegraph obrigatório (Pillar 4)**: toda mudança de clima ou período é avisada N turnos antes na fila, com ícone persistente; Scan revela o número exato de turnos restantes. Nunca RNG punitivo.
- **Cap anti-Scan (Pillar 3)**: degradações de custo de Scan **não empilham além de -2 AP total** num mesmo encontro. A curadoria (§18.6) impede 2 ambientes anti-Scan fortes simultâneos. Hardware nunca vira inútil.
- **Teto cognitivo**: no máximo ~12 terrenos visíveis. O tier **Codex** (efeitos sutis) só **ativa/revela após Scan-ambiente** (novo verb: Scan aplicado ao campo, custo 1 AP).
- **Separação da fórmula**: `multAmbiente` é fator próprio na cadeia divisiva (§11), nunca altera `multFraqueza`.
- **Mutabilidade = interações curadas**: tabela fechada determinística (§18.6). Sem RNG nas transições.

### 18.2 Camada CLIMA (8, transitória)

Muda a cada N turnos, sempre telegrafada. Cada clima favorece uma família e prejudica outra, facilita um status e pode mexer no hardware/fila.

| Clima | Família ↑ | Família ↓ | Status facilitado | Hardware / fila |
|---|---|---|---|---|
| **Neblina** | Sônico ×1.3 | Criptográfico ×0.66 | Disrupt +1 mag | Scan +1 AP; Prever só 1 turno à frente |
| **Chuva** | Elétrico ×1.5 | Bioquímico ×0.66 | Stun +1 dur | — |
| **Calor** | Bioquímico ×1.3 | Elétrico ×0.66 | Corrode +1 dur | overheat: 3+ cartas seguidas do mesmo ator → auto-Disrupt nele |
| **Tempestade Elétrica** | Elétrico ×1.5 | Criptográfico ×0.66 | Stun +1 dur | Scan +1 AP mas Prever +1 turno; raio a cada 3 turnos aplica Slow -1 ao ator mais lento da fila |
| **Vento** | Sônico ×1.3 | Bioquímico ×0.66 | Knockback +1 fila | Prever +1; Object Cinético ×0.85; party SPD +1 |
| **Estática / Interferência** | Criptográfico ×1.5 | Sônico ×0.66 | Decrypt / Expose / Silence +1 dur | Scan -1 dado de info; Null custa 0 AP |
| **Fumaça / Cinzas** | Bioquímico ×1.3 | Criptográfico ×0.66 | Poison / Corrode +1 dur; DoT +1 tick | Scan +1 AP; Prever -1 alcance |
| **Escuridão Total** | Sônico ×1.5 | Criptográfico ×0.85 | Disrupt +1 dur | Scan +2 AP (única fonte de info); Iara vê no escuro; inimigos sem visão Slow -1 |

### 18.3 Camada PERÍODO (roda de 4 fases)

Ciclo temporal automático: **Dia (5) → Crepúsculo (2) → Noite (5) → Aurora (2) → Dia**, em turnos. As fases curtas (Crepúsculo/Aurora) funcionam como telegraph de graça da transição que se aproxima.

> **Granularidade e telegraph (canon 2026-06-03, F2-E.11).** "Turnos" do ciclo = **rodadas completas de fila** (1 tique quando todos os atores agiram uma vez; o dia passa no mesmo ritmo independente do tamanho da party, casa com `RoundIndex`/ramp de mana). **Telegraph = 2 turnos** (as próprias fases Crepúsculo/Aurora de 2 turnos são a janela de aviso da fase forte seguinte). Parametrizável (`EnvironmentClock`).

| Fase | Família ↑ | Família ↓ | Status | Hardware / fila | Duração |
|---|---|---|---|---|---|
| **Dia** | Bioquímico ×1.3 | Criptográfico ×0.85 | Regen +1 dur (party) | Scan grátis | 5 |
| **Crepúsculo** | (transição, neutro) | — | Disrupt +1 mag | Scan revela +1 dado | 2 |
| **Noite** | Criptográfico ×1.5 + Sônico ×1.3 | Bioquímico ×0.85 | Decrypt / Expose +1 dur | Scan +1 AP; Iara: 1ª carta Cripto por turno ignora telegraph; inimigos diurnos Slow -1 | 5 |
| **Aurora** | Elétrico ×1.3 | — | Haste +1 dur (party) | Scan normaliza; Prever +1; party SPD +1 | 2 |

- **Selve Sombria**: ciclo natural automático (dia/noite reais).
- **Cidade ciber-gótica**: o eixo vira "ciclo da grade elétrica" (Surto ↔ Blackout) usando o **mesmo motor**, só muda o sabor (Pillar 5: contraste deliberado de setting com sistema unificado por baixo).
- Crepúsculo e Aurora curtos = janelas-ponte que telegrafam a próxima fase forte sem custo de Scan.

### 18.4 Camada TERRENO — Tier VISÍVEL (12, fixo por encontro)

Lido a olho nu (sem Scan), até ~12 visíveis simultâneos (teto cognitivo). Cada um declara família ↑ / ↓, status facilitado e efeitos de fila/hardware.

**Existentes (7):**

- **Lamacento**: Elétrico ×1.3 / Cinético ×0.66 (deslocamento dificultado); Knockback no alvo → +2 na fila (em vez de +1); Slow -2 todos (-3 para Cinético).
- **Seco**: Cinético ×1.3 / Elétrico ×0.66; DoT (Poison/Corrode) -1 dur; Break +1 dur.
- **Vinhas**: Bioquímico ×1.3 (Object +1 dur) / Cinético ×0.85; aplica Root (Slow de magnitude extrema); Knockback anulado em alvo enraizado; Scan revela crescimento Fibonacci.
- **Gelo**: Cinético ×1.3 / Bioquímico ×0.66; Break +1 dur; Slow -1; SPD -1 todos; Tavus-Drive +1 mana na 1ª carta do turno.
- **Água / Alagado**: Elétrico ×1.3 / Cinético ×0.85; Stun +1 dur; SPD -1 todos; alvo dentro da água que leva dano Elétrico → Disrupt grátis.
- **Metal-Condutor**: Elétrico ×1.3 / Sônico ×0.66; Corrode +1 dur; Knockback ricocheteia +1 fila adicional; Scan grátis + revela 1 dado extra (Matriz Ortodôntica amplificada).
- **Bioluminescência (SÓ SELVE)**: Sônico ×1.3 / Elétrico ×0.85; Regen / Haste +1 dur; Scan grátis + Prever +1; pulso luminoso em sequência 1-1-2-3-5.

**Novos (5):**

- **T1 Pavimento Tesselado** (cidade forte; variante na Selve): Criptográfico ×1.3 / Sônico ×0.66. **DUAL por rodada completa de fila**, sincronizado a `turnoIndex`: rodada **ÍMPAR = Branco (lapidado)** → +1 dado de Scan grátis a quem scaneia; rodada **PAR = Preto (bruto)** → a 1ª carta Cripto da rodada aplica Expose magnitude 13 grátis no alvo. Telegraph: o quadriculado pulsa branco/preto conforme a rodada. (Easter egg maçônico canon: ashlar bruto/polido; "13" Fibonacci.)
- **T2 Talude Instável**: Cinético ×1.5 / Criptográfico ×0.66. Pune o INATIVO: ator que não agiu ofensivamente na rodada anterior entra com Slow magnitude 2; ator que gastou ≥2 AP ofensivo na rodada anterior fica imune. Premia agressão, pune turtle. Telegraph: o chão racha sob o ator inativo 1 turno antes. Mexe a fila por comportamento (via Slow).
- **T4 Ashlar Bruto**: Cinético ×1.3 / Elétrico ×0.66. Usar **Defender** aqui → pool de Shield ×1.5 (Magnitude = Def × 1.5). Premia turtle (espelho do T2). Não dá Shield de graça (exige gastar AP em Defender). Transição de **PROGRESSÃO** (entre encontros): arena vencida "lapida" o Ashlar Bruto → vira **T1 Pavimento Tesselado** (bruto → polido = ofício maçônico canon).
- **T5 Solo Fértil Recursivo (SÓ SELVE)**: Bioquímico ×1.5 / Cinético ×0.66. Entidades Object plantadas aqui ganham +1 Duration e escalam o efeito por rodada na sequência 1,1,2,3,5 (ex: um totem de Poison aplica magnitude 1,1,2,3,5 ao longo das rodadas). Facilita Poison / Regen / Root. Scan revela o estágio do L-system. (Fibonacci canon.)
- **T6 Anomalia Perlin** (Selve profunda + cidade no ato 3): NENHUMA família ↑; Criptográfico ×0.66. Degrada hardware: Gambito-Prever sempre retorna ruído (`IsChaotic` global) + Scan retorna perfil parcial (revela HP, mas exige 2 scans para a fraqueza). **NÃO mexe no dano** (`multAmbiente` geral = 1.0). Telegraph forte: glitch visual no canal-4 (Perlin quebrado, gradiente cyan → vermelho a 21 lúmens). Vetor anti-padrão Patch-Zero canon.

### 18.5 Camada TERRENO — Tier CODEX (3, efeito ativa/revela só após Scan-ambiente 1 AP)

Efeitos sutis, deliberadamente fora do teto cognitivo "a olho nu". Só ativam e se revelam após o verb **Scan-ambiente** (1 AP) ser usado no campo.

- **T3 Espelho Ressonante**: Sônico ×1.5 / Bioquímico ×0.66. Cartas com `TargetShape` Grupo/Linha ricocheteiam um 2º tick a 0.5× no alvo de maior HP. **SIMÉTRICO**: AoE inimiga também ricocheteia neles (premia ler intent via Gambito antes de aglomerar a party). Facilita Disrupt.
- **T7 Duto Condutor Pressurizado (SÓ CIDADE)**: Elétrico ×1.3 / Bioquímico ×0.66. Se um ator usa Elétrico E outro usa Sônico na MESMA rodada → "Ressonância de Duto": +1 Disrupt em todos os inimigos (materializa a sinergia canon Cauã + Linda). Facilita Disrupt / Stun.
- **T8 Elevação Dominante**: Cinético ×1.3 / Sônico ×0.66. O 1º ator da party a agir na rodada (topo da fila) ganha Haste magnitude 1 até o fim da rodada + revela o intent de +1 inimigo grátis. Premia SPD / Gambito-reordenar. Efeito por **posição-de-FILA** (não por posição de grid).

### 18.6 Mutabilidade — interações curadas (tabela fechada determinística)

Transições de ambiente são **DETERMINÍSTICAS** (não RNG), só avançam/transformam de forma legível, nunca pulam estados. Tabela fechada de interações canonizadas:

| Gatilho | Resultado |
|---|---|
| Elétrico forte OU Calor sobre **Lamacento** | seca → vira **Seco** |
| Elétrico forte OU Calor sobre **Vinhas** | queima → vira **Seco** |
| Elétrico forte OU Calor sobre **Gelo** | derrete → vira **Água / Alagado** |
| Sônico forte sobre **Neblina / Fumaça / Estática** | dissipa → ar limpo |
| **Vento** sobre Neblina / Fumaça / Cinzas | dissipa → ar limpo |
| **Chuva + Calor** | gera **Vapor** (Neblina + calor leve) |
| **Chuva + Vinhas** | vinhas crescem mais rápido (avança estágio) |
| **Chuva + Elétrico ambiente** | escala para **Tempestade Elétrica** |
| **Calor** em **T5 Solo Fértil** | Object pula para o estágio 3 da sequência |
| **Gelo** em **T5 Solo Fértil** | congela o crescimento (trava estágio) |
| **Água** sobre **Metal-Condutor** | "condução total": Elétrico ×1.5 por 2 turnos |
| **Acaceiro saudável** próximo (estado de arena) | "Anomalia Contida": **T6** Scan volta a funcionar (vetor de purificação canon) |
| **T4 Ashlar Bruto** vencido (entre encontros) | lapida → vira **T1 Pavimento Tesselado** (progressão) |

Regras gerais: as transições são curadas e determinísticas; só avançam ou transformam de modo legível para o jogador; nunca pulam estados intermediários.

### 18.7 Mapa família → ambiente-casa (balance)

Cada família tem 2-3 casas (↑) e 2-3 hostis (↓): nenhuma família é sempre-ótima (anti-degeneração preservado, alinhado à roda fechada §6).

| Família | Casas (↑) | Hostis (↓) | Pico |
|---|---|---|---|
| **Elétrico** | Chuva, Tempestade, Água, Metal-Condutor, T7 Duto, Aurora | Seco, Calor, T4 Ashlar | 1.5 |
| **Cinético** | Seco, Gelo, T2 Talude, T4 Ashlar, T8 Elevação | Lamacento, Vinhas, Água, T5 Solo | 1.5 (T2) |
| **Criptográfico** | Noite, Estática, T1 Tesselado | Neblina, Tempestade, Fumaça, T2, T6 | 1.3 (sem pico de dano, por design: ganha informação, não dano bruto) |
| **Sônico** | Escuridão, Vento, Bioluminescência, T3 Espelho, Noite (2ª) | Estática, Metal-Condutor, T1, T8 | 1.5 |
| **Bioquímico** | Calor, Fumaça, Vinhas, T5 Solo, Dia | Chuva, Gelo, Água, Vento, T3, T7 | 1.5 (T5) |

Nota de balance: **Criptográfico é a única família deliberadamente sem pico de dano ×1.5**, compensada por ganhos informacionais (Scan grátis, dados extras, Expose grátis no T1). Reforça a identidade da família (utilidade/anti-buff, §6) e impede que ambiente vire mero amplificador de burst.

### 18.8 Eixos-espelho (legibilidade de opostos)

Pares opostos legíveis que ajudam o jogador a ler o sistema por simetria (Pillar 1: o sistema é decifrável porque é coerente):

| Eixo | Par |
|---|---|
| Umidade do solo | Lamacento ↔ Seco |
| Precipitação | Chuva ↔ Calor |
| Temperatura | Gelo ↔ Calor |
| Solo seco | Água ↔ Seco |
| Esconde ↔ revela info | Neblina ↔ Vento |
| Sinal ↔ limpa | Estática ↔ Vento |
| Luz | Dia ↔ Noite |
| Selve ↔ cidade | Bioluminescência ↔ Escuridão |
| Grade da cidade | Tempestade ↔ Blackout |
| Pune-parado ↔ premia-parado | T2 Talude ↔ T4 Ashlar |
| Maçônico (bruto ↔ polido) | T1 Tesselado polido ↔ T4 Ashlar bruto |

Vinhas / T5 Solo Fértil = eixo de **tempo / recursão** (Fibonacci), ainda sem espelho pleno declarado.

### 18.9 Easter eggs (densidade ~10-15%, sutil)

Aplicam os dois sistemas canon (sem siglas, sem ritual nomeado):

- **Fibonacci**: durações de período 5/2/5/2; crescimento de T5 em 1,1,2,3,5; pulso de Bioluminescência 1-1-2-3-5; Expose magnitude 13 no T1; raio de Tempestade a cada 3 turnos; T6 a 21 lúmens.
- **Maçonaria**: T1 Pavimento Tesselado (ashlar polido preto/branco), T4 Ashlar Bruto, transição bruto → polido. Sem siglas nem ritual nomeado.

### 18.10 Escopo de implementação (slice)

- Record **`EnvironmentModifier`** no engine POCO: família-mults, status facilitado, deltas de fila/SPD, hooks de hardware, tipo de camada (terreno/clima/período), tier (visível/codex).
- **`multAmbiente`** plugado na fórmula §11 (default 1.0 = retrocompatível).
- Evento **`CombatBus.EnvironmentSet(envId)`** e evento de mudança de camada (clima/período avançou).
- Verb **Scan-ambiente** (1 AP) para revelar o tier Codex.
- **Aplicação estática por arena**: level-designer marca o ambiente inicial do encontro. **Cartas-clima e inimigos que invocam ambiente = fase posterior** (dados já preparados).

A regra de STACKING das 3 camadas e seu cap final (`multAmbiente ∈ [0.44, 2.25]`) está canonizada em §11.

---

**Última revisão:** 2026-06-03 (D.1+N.2 Sprint 1 W2). Decisões canonizadas: HP +60% Trash 34→55 / Elite 89→144; AP e Mana por-ator CTB (§5); §2.1 contrato fragilidade Gus (N.2 R1, one-way door); §10 feedback ERRO DE COMPILAÇÃO (N.2 R2); §12/§13/§15 caos Perlin exclusivo Patch-Zero (N.2 R3, one-way door); §17 stats de referência pós-inflação + roda de fraqueza confirmada. Revisão anterior 2026-05-26: §18 ambientes de combate, §11 multAmbiente + stacking 3 camadas, escopo slice item 15. Status: canônico, pronto para implementação TDD F2-E.5.

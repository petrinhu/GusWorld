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

### Mana / Compilação

- `manaMax = 2 + turnoIndex`, com `cap = 8`.
- Recarrega ao **máximo** a cada `TurnStart`. **Sem carry-over** (impossível bankar mana entre turnos: anti-degeneração, ver §13).
- Ramp linear garante que combos premium só ficam viáveis no mid-late do combate, preservando curva de tensão.

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
| **Expose** | Criptográfico | aumenta dano recebido pelo alvo (revela ponto fraco) |
| **Decrypt** | Criptográfico | anula/remove buffs do alvo e bloqueia novos por Duration |
| **Shield** | utilitário | absorve dano (Magnitude = pontos absorvidos) antes do HP |
| **Regen** | utilitário | cura por tick no TurnStart |
| **Haste / Slow** | utilitário | aumenta/reduz SPD (recomputa posição na fila) |

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

---

## 11. Fórmula de dano (determinística)

```
danoFinal = round( (Power + Atk) × multFraqueza × multMod × multCombo − Def )
clamp: danoFinal >= 1   // nunca zera por subtração de Def (a não ser multFraqueza == 0.0 → imune → 0)
```

| Fator | Origem | Valores |
|---|---|---|
| `Power` | carta | campo do record |
| `Atk` | atributo do atacante | stat |
| `multFraqueza` | roda de fraqueza (§6) | 1.5 / 1.0 / 0.66 / 0.0 |
| `multMod` | modificador aplicado | default 1.0; Stream pode distribuir; valores tabelados |
| `multCombo` | receita de combo (§10) | default 1.0; >1.0 em combo casado |
| `Def` | atributo do defensor | stat (reduzido por Break/Corrode) |

Caso `multFraqueza == 0.0` (imune): `danoFinal = 0` (a regra de clamp mínimo 1 NÃO se aplica a imunidade; imunidade é zero genuíno e telegrafa "elemento errado" ao jogador).

### RNG visível decaindo (opção B)

- Hit / crit têm **porcentagem mostrada** ao jogador antes de confirmar a ação (transparência total, Pillar 1).
- RNG é **seedável** e usa **ruído coerente** (Perlin/Simplex), nunca opaco (Pillar 2: o jogador, via Óculos, "vê o noise underlying").
- Gambito pode **forçar re-roll** ou **cancelar** um resultado de RNG (Pillar 1).
- A variância **decai com Knowledge** (Knowledge Progression): contra inimigos farmados, variância tende a 0 (RNG zerado pra esse inimigo).
- **No vertical slice F2-E.5: entregar determinístico (variância 0)** com o gancho Knowledge plugado na interface, mas sem a curva de decaimento ativa. Hit/crit % aparece na UI sempre mostrando o valor determinístico (ex: 100% hit no slice).

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
- **Falha contra intent caótico** de mini-boss: retorna ruído Perlin em vez de leitura limpa (Pillar 2: limite do conhecimento é narrativo e mecânico).

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
| **Mini-boss** | `UtilityBrain` + ruído | utility com camada de intent caótico (ruído Perlin) que faz Gambito-Prever falhar parcialmente. |

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
| **Intent caótico** | mini-bosses resistem a predição total (Gambito não trivializa) |
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
- `StatusApplied(actor, statusEffect)` / `StatusExpired(actor, statusId)`
- `ActorDefeated(actor)` / `ActorIncapacitated(companion)`
- `TurnEnded(actor)`
- `CombatEnded(outcome, payload)` // vitória / derrota / fuga

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

### Fora do slice (jogo posterior, interface já plugada)

- Curva de decaimento de variância por Knowledge (no slice é variância 0 / determinístico, gancho plugado).
- `UtilityBrain` e camada de ruído de mini-boss.
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
    int Mastery                  // cresce por uso
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
    bool IsChaotic               // true => Gambito-Prever retorna ruído
);

public interface IEnemyBrain {
    IntentPreview PreviewIntent(CombatState state);
    EnemyAction   DecideAction(CombatState state);
}

// Fórmula de dano (referência de implementação):
// danoFinal = Math.Max(1, (int)Math.Round((Power + Atk) * multFraqueza * multMod * multCombo - Def));
// caso multFraqueza == 0.0f => danoFinal = 0 (imunidade ignora o clamp mínimo).
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

---

**Última revisão:** 2026-05-26. Status: canônico, pronto para implementação TDD F2-E.5. Próxima revisão prevista: após primeiro playtest do encontro do vertical slice (validar tensão do mana ramp, legibilidade da fila, sensação do Gambito).

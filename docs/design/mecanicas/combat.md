# Sistema de Combate Turn-Based (GusWorld)

**Status:** Canônico. Decisões ratificadas pelo criador supremo em 2026-05-26 (proposta de combate aprovada integralmente, sem reabertura). Spec de produção para implementação TDD da Fase 2 (vertical slice F2-E.5). **Evolução 2026-06-22 (M5-DMG):** §11 ganhou o sorteio de canal FALHA/CRIT/COMUM sobre a variância Knowledge; a §11 deixa de ser paridade com o C# (que morre no M8) e passa a ser o contrato do motor C++.

**Cross-ref pillars:** este sistema materializa GDD §6.1 (Sintonização Ortodôntica / Scan), §6.2 (Compilação do Codex) e §6.3 (Vetores do Gambito), e serve diretamente Pillar 1 (Lógica vence força), Pillar 2 (Magia é sistema formal computável), Pillar 3 (Triângulo de hardware é a interface) e Pillar 4 (Prodígio de 11 anos, vulnerabilidade física + vitória por descoberta).

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

- `SetupPhase → TurnStart`: a fila já está ordenada por SPD; o primeiro ator entra. A SPD também decide qual LADO abre a rodada (party-block ou enemy-block), ver §4.1 (Janela de Comando da Party, modelo 1B).
- `TurnStart → ActionSelect`: mana e AP do ator são preparados ANTES da seleção.
- `ActionSelect ⇄ ActionResolve`: laço interno. Cada ação custa AP. Quando `AP == 0` ou o ator escolhe "passar", sai do laço.
- `TurnEnd → CheckEnd → TurnStart`: avança para o próximo ator. Quando o ator era da party, o "próximo" é escolhido pelo jogador dentro do bloco da party (comando livre, §4.1); quando era inimigo, segue a ordem de SPD entre inimigos. `turnoIndex` é por rodada completa de fila (afeta o ramp de mana de forma consistente entre todos).
- Qualquer ator que chega a HP 0 é resolvido conforme regra de morte/incapacitação (companions incapacitados, Pillar 4; inimigos removidos da fila).

> **Modelo de seleção de ator (canon 2026-06-25, decisão do criador via AskUserQuestion, modelo 1B).** A FORMA da FSM acima NÃO muda. O que muda é a regra de QUEM entra em `ActionSelect` quando é a vez da party: em vez de forçar o topo da fila, o motor expõe os membros-da-party-elegíveis-na-rodada e o jogador escolhe qual age, em que ordem, com qual ação e qual alvo (comando livre estilo Final Fantasy clássico). A SPD deixa de microgerenciar a ordem DENTRO da party, mas continua decidindo qual lado abre a rodada e segue valendo para Haste/Slow, Gambito-Reordenar, ambientes-SPD (§18) e o item de iniciativa. Detalhe completo + nota técnica em §4.1.

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

## 4.1 Janela de Comando da Party (comando livre sobre o CTB, modelo 1B)

**Status:** canonizado pelo criador supremo em 2026-06-25 (decisão D1/D2 via AskUserQuestion). Substitui o "agir só com o ator do topo da fila" pelo comando LIVRE da party dentro do seu bloco, mantendo o CTB por SPD como esqueleto. O criador ACEITOU o contra-argumento do lead-game-designer: SPD NÃO vira pura sugestão; continua sendo um stat de build que decide o que importa.

### Conceito

A ordem de turno continua sendo o CTB por SPD (§4). A mudança é a fantasia de comando: quando chega a vez da party, o jogador comanda o BLOCO da party na ordem que quiser (escolhe qual membro age, em que ordem, qual ação e qual alvo de cada um), em vez de ser forçado a agir só com o ator do topo. É o feel Final Fantasy clássico / Dragon Quest (comando de bloco por lado).

- **SPD = pré-seleção sugerida** dentro do bloco: ao abrir a Janela, o membro com maior SPD entre os ainda-não-agiram fica pré-selecionado, mas o jogador pode escolher qualquer um.
- **SPD = quem ABRE a rodada:** a SPD do lado decide se o party-block ou o enemy-block joga primeiro na rodada. É aqui que o stat SPD preserva peso real (o inimigo rápido ainda te pega primeiro).

### Fluxo de rodada (modelo 1B)

```
RODADA = uma volta completa da fila CTB (turnoIndex / RoundIndex, igual antes).

1. SetupPhase / início de rodada: a fila é ordenada por SPD e os atores são
   agrupados por LADO. A SPD comparada entre os lados decide quem abre:
   party-block primeiro OU enemy-block primeiro.

2. JANELA DE COMANDO DA PARTY (quando é a vez do bloco da party):
   - Membros vivos da party que ainda não agiram nesta rodada ficam elegíveis.
   - Pré-selecionado = o de maior SPD entre os elegíveis (sugestão, não trava).
   - O jogador escolhe QUAL membro age -> entra no loop de AP normal (ActionSelect,
     3 AP, §5): Scan / Gambito / Atacar / Defender / COMPILAR / Flee / passar, com
     ALVO escolhido (modo-mira, battle-screen §3.5).
   - Resolve a ação imediatamente (feedback na hora, pacing battle-screen §5.2).
   - Repete até todos os membros da party terem agido nesta rodada.

3. BLOCO DOS INIMIGOS: agem na ordem de SPD entre eles (ScriptedBrain / UtilityBrain,
   §13; AP por tier §13.1). O PacingDirector itera por-ação (battle-screen §5.2 D8-D12).

4. CheckEnd -> próxima rodada (recomputa quem abre por SPD).
```

A rodada é, portanto, "um lado age todo, depois o outro", com a SPD decidindo a ordem dos lados. Dentro do bloco da party o comando é 100% livre; dentro do bloco inimigo a ordem segue SPD (IA).

### Impacto nos sistemas canônicos (todos PRESERVADOS em 1B)

| Sistema | Efeito em 1B |
|---|---|
| **Fila CTB (§4)** | Preservada. Continua a estrutura, reagrupada por lado/rodada via SPD. A faixa visível (battle-screen §2) segue mecânica, não cosmética. |
| **Gambito-Reordenar (§12, `ReorderActor`)** | Preservado e mais valioso. Empurrar um inimigo o atrasa para a rodada seguinte ou adia a abertura do enemy-block; puxar um aliado para frente afeta a ordem entre os lados. |
| **Gambito-Prever (§12, `IntentPreview`)** | Preservado e mais valioso. O jogador lê o intent e ordena a party para responder (proteger o Gus, focar quem vai bater). |
| **AP por ator (§5)** | Intacto. Cada membro mantém seus 3 AP independentes; o loop interno de `ActionSelect` não muda. Só muda QUEM o jogador seleciona para entrar nele. |
| **Iniciativa / SetupPhase (§3)** | Estendida: passa a calcular qual lado abre (comparação de SPD entre os lados). |
| **Cast-time / cartas lentas (CARTAS-CAST-TIME, INBOX)** | Preservado (D4). Como 1B mantém a fila, a carta lenta segue resolvendo posições à frente na fila/rodada; o Gambito ainda a vê e pode proteger/reordenar. |
| **Haste / Slow (§9/§18)** | Preservados: mexem na SPD, logo em quem abre a rodada e na posição na fila. Continuam decisões táticas reais. |
| **Ambientes que mexem SPD/fila (§18: T8 Elevação "1º a agir", T2 Talude, Aurora, Gelo)** | Preservados: o "1º da fila/rodada" continua definido por SPD. |
| **ITEM-SPD-INICIATIVA (INBOX)** | Preservado e justificado: usar o item aumenta SPD e faz o party-block abrir a rodada. O item ganha razão de existir exatamente por causa de 1B. |
| **IA / AP por tier (§13.1)** | Intacto. Inimigos agem em bloco por SPD; o PacingDirector já itera por-ação (2 beats por inimigo). |
| **Auto-resolve / AutoResolveBrain (§19)** | Intacto. Roda a FSM headless; comando livre é seleção de ator na apresentação, não muda o headless (o AutoResolveBrain já decide ações sozinho). |
| **Análise Preditiva / fragilidade do Gus (§2.1)** | Intacta. Bônus: comando livre deixa o jogador proteger melhor o Gus frágil (coerente com Pillar 4). |
| **Fórmula de dano (§11)** | NÃO muda. Comando livre não toca a resolução de ação, só a seleção de ator. |

### Por que SPD continua sendo um stat com peso (preservação do equilíbrio)

O comando livre tira do SPD apenas o microgerenciamento da ordem DENTRO da party (a parte que o criador quis libertar). O SPD continua decidindo o que importa: (a) qual lado abre a rodada (o inimigo rápido te pega primeiro); (b) o membro pré-selecionado ao abrir a Janela; (c) o alvo/efeito de Gambito-Reordenar, Haste/Slow e ambientes; (d) o valor do item de iniciativa. Um stat que não decide nada seria decoração (viola "tensão > complexidade"); 1B impede isso.

### Bônus de iniciativa (PENDENTE de playtest, D5)

Opcional, decidido depois com dados: o lado que abre a rodada por SPD poderia ganhar um pequeno bônus tático (ex.: na linha do T8 Elevação, que já dá Haste ao 1º da fila), reforçando que correr na frente vale sem ser dominante. Registrado como pendência; não entra agora. Mede-se em playtest.

### Nota técnica para o backend-engineer (mudança mínima, FSM e §11 NÃO mudam)

A implementação é uma extensão ADITIVA ao `CombatStateMachine`. A auditoria do motor permanece válida porque a FORMA da FSM e a fórmula de dano (§11) não mudam; muda a seleção de ator, não a resolução de ação. Os testes de transição existentes continuam passando (forçar o topo é o caso particular de o conjunto elegível ter 1 elemento).

- **Campo novo no estado:** `PendingPartyActors` (membros vivos da party que ainda não agiram NESTA rodada). Análogo do lado inimigo já é coberto pela fila por SPD.
- **`ActionSelect`:** quando o lado ativo é a party, consulta `PendingPartyActors` (membros elegíveis) em vez de só `fila.Peek()`. O jogador escolhe qual entra no loop de AP; o pré-selecionado é o de maior SPD entre os elegíveis.
- **`SetupPhase` / início de rodada:** agrupa a fila por lado e compara SPD para decidir quem abre (party-block ou enemy-block). É reordenação da fila já existente, não reescrita do loop.
- **`CheckEnd` / avanço:** marca o ator escolhido como "já agiu nesta rodada" (em vez de `Dequeue` cego) e, quando o party-block esvazia, passa para o enemy-block (ou inicia nova rodada). Recomputa quem abre por SPD na nova rodada.
- **Determinismo preservado:** a ordem de consumo do RNG (§11) não muda; a seleção de ator é input de jogador (ou do AutoResolveBrain headless), não consome RNG.
- **Escopo (D6):** o target selection (modo-mira, zero motor, battle-screen §3.5) entra JÁ; a Janela de Comando da Party (extensão do motor acima) entra logo após, briefada ao backend-engineer + gameplay_engineer.

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
| `CritChance` | int (0..100) | **piso de crit configurável por carta** (canon 2026-06-22, M5-DMG): chance efetiva de crit = `max(5, CritChance)`. Existe um **piso global de 5%** de crit; `CritChance = 0` significa "usa o piso 5%", e um valor maior ELEVA a chance acima de 5%. Visível na UI antes de confirmar a ação. NÃO é mais "0 = sem crit". |

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
| `Discoverable` | bool (combo curado vs combo secreto descobrível por experimentação) |

Receita é casada por **assinatura exata** de família + base + modificador, não por instância de carta específica. Isto mantém ~200 receitas gerenciáveis e descobríveis (Pillar 2: ~200 combinações pré-planejadas + 5-10 combos secretos).

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

## 11. Fórmula de dano (canonizada 2026-05-26; evoluída 2026-06-22, item M5-DMG)

Duas fórmulas distintas: **UseCard é divisiva** (Def reduz por fração, escala bem sem zerar), **ataque básico é subtrativo** (recurso simples sempre-disponível, 0 mana).

> **Nota de divergência canônica (2026-06-22, M5-DMG).** A partir desta revisão, a §11 NÃO é mais paridade 1:1 com o C# de referência. O protótipo C# morre no M8 (port para C++ concluído); **o motor C++ passa a seguir esta §11**, não a antiga implementação C#. A evolução abaixo (sorteio de canal falha/crit/comum) é a fórmula canônica de produção. O snippet ilustrativo do §17 reflete a forma C# antiga e fica como referência histórica do protótipo, não como contrato.

### Fórmula UseCard (divisiva)

A variância Knowledge atual É o "range da arma": preservada intacta. Sobre ela roda **um único sorteio de canal** que decide se o golpe é FALHA, CRIT ou COMUM (mutuamente exclusivos).

```
// 1. Cadeia divisiva (INALTERADA — nenhum fator novo aqui)
multExpose  = alvoTemExpose ? (1 + Expose.Magnitude / 100) : 1.0
danoBase    = (Power + Atk) × (100 / (100 + Def)) × multFraqueza × multMod × multCombo × multExpose × multAmbiente

// 2. Curto-circuito de imunidade (ANTES de qualquer sorteio)
se multFraqueza == 0.0  →  danoFinal = 0  (FIM; nenhum RNG consumido)

// 3. Range da arma neste encontro (variância Knowledge, preservada)
v       = max(0.05, 0.30 × e^(-kills × 0.10))      // kills = target.KnowledgeKills
maxArma = danoBase × (1 + v)                        // topo do range deste encontro

// 4. Chances dos canais
fumbleChance = round(5 × e^(-kills × 0.50))         // 5% no 1º encontro, decai a ~0
critChance   = max(5, card.CritChance)              // piso global 5%; carta eleva acima

// 5. UM sorteio de canal (consome RNG UMA vez; ver ordem abaixo)
roll = rng.Next(0..99)                              // inteiro 0..99
se roll < fumbleChance                 → canal = FALHA
senão se roll < fumbleChance + critChance → canal = CRIT
senão                                  → canal = COMUM

// 6. Resolução por canal
FALHA:  danoFinal = 0                               // log com estética de erro de compilação (§10)
CRIT:   danoFinal = round(maxArma × 1.5)            // = round(danoBase × (1+v) × 1.5); sufixo [CRITICO]
COMUM:  r = rng.NextDouble()                        // 2º consumo de RNG, SÓ no canal COMUM
        danoFinal = round(danoBase × (1 + (v × 2 × r - v)))   // aplica a variância normal
```

#### Ordem de consumo do RNG (determinismo dos testes)

A cadeia consome o RNG injetado nesta ordem fixa (importa para reproduzir testes com semente fixa):

1. **Sorteio de canal** (`rng.Next(100)`): sempre consumido, exceto no curto-circuito de imunidade (`multFraqueza == 0.0`), que retorna antes de tocar o RNG.
2. **Variância COMUM** (`rng.NextDouble()`): consumido **somente** quando o canal resolvido é COMUM. FALHA e CRIT não consomem o 2º roll (dano deles é fechado).

Implicação para testes: um inimigo imune não move o RNG; um golpe que cai em FALHA ou CRIT move o RNG exatamente 1 vez; um golpe COMUM move 2 vezes. Os testes de paridade devem assertar essa contagem.

#### Canais (detalhe)

- **FALHA (dano 0):** chance NÃO fixa. Começa em 5% no 1º encontro (`kills=0`) e DECAI a ~0 conforme a maestria (Knowledge) sobe, MESMA família exponencial da variância (`e^(-kills × k)`). Constante `k = 0.50` (justificativa abaixo). Telegrafada no log com estética de "erro de compilação" (§10), ex. `FALHA DE COMPILACAO`. A falha é o "tropeço de quem ainda não conhece o inimigo": some com a maestria, reforçando a Knowledge Progression (Pillar 1).
- **CRIT:** chance = piso global de 5%, que `card.CritChance` pode ELEVAR (piso configurável por carta; ver §7). Dano = "máximo da arma" × 1.5, onde "máximo da arma" = topo do range do encontro = `danoBase × (1 + v)`. Logo `critCrit = round(danoBase × (1+v) × 1.5)`. Sufixo `[CRITICO]` no log.
- **COMUM:** o resto da probabilidade. Aplica a variância Knowledge normal sobre `danoBase` (forma idêntica à antiga: `danoBase × (1 + (v × 2 × r − v))`).

#### Curva de decaimento da FALHA

```
fumbleChance(%) = round(5 × e^(-kills × 0.50))
```

| kills | 5 × e^(-kills×0.50) | round → % |
|---|---|---|
| 0 | 5.00 | **5** |
| 1 | 3.03 | 3 |
| 2 | 1.84 | 2 |
| 3 | 1.12 | 1 |
| 4 | 0.68 | 1 |
| 5 | 0.41 | 0 |
| 6+ | < 0.30 | **0** |

A falha chega a **0% a partir de 5 kills** do mesmo tipo de inimigo.

**Justificativa da constante `k = 0.50`:** a falha é um "risco de iniciante" que deve sumir RÁPIDO (cinco mortes do mesmo inimigo já é familiaridade plena), enquanto a variância de dano persiste mais tempo (a dispersão só atinge o piso ±5% bem mais tarde). Por isso a falha usa um decaimento **mais agressivo** que a variância (`k = 0.50` vs `k = 0.10`), mantendo a **mesma família funcional** `e^(-kills × k)` pedida pelo criador. Racional de design: o jogador não deve ser punido com "dano zero" depois que já domina o inimigo (anti-frustração; Pillar 4), mas a variância pode continuar comunicando incerteza por mais encontros sem frustrar (errar 0 vs. errar a magnitude são pesos psicológicos diferentes). A constante 0.50 também produz uma escada limpa de números pequenos (5→3→2→1→1→0), legível na UI.

#### Regras gerais

- **Imune** (`multFraqueza == 0.0`): `danoFinal = 0`, curto-circuito ANTES do sorteio (não consome RNG). Tem prioridade sobre FALHA/CRIT/COMUM.
- **Sem clamp mínimo** no canal COMUM: a divisiva nunca chega a 0 contra não-imunes (frações muito pequenas podem arredondar pra 0, telegrafando "elemento errado / Def alta demais"). Isto é distinto de FALHA (dano 0 deliberado por sorteio, com log de erro de compilação).
- **`multExpose`** (§9, canon 2026-05-26): último fator da cadeia divisiva, ANTES do sorteio de canal. Só UseCard; o ataque básico não usa. Sem Expose no alvo = 1.0.
- **`multAmbiente`** (§18, canon 2026-05-26): também na cadeia divisiva, ANTES do sorteio. Produto dos multiplicadores das camadas ambientais ATIVAS (terreno + clima + período) que afetam a família da carta jogada. Default 1.0 (encontro sem ambiente marcado). Faixa por camada 0.66 a 1.5. **NUNCA toca `multFraqueza`** (a roda de fraqueza é fator independente; ambiente nunca altera a relação de fraqueza). Só UseCard; o ataque básico subtrativo não usa.
- **Ordem**: cadeia divisiva completa (incl. `multExpose` e `multAmbiente`) → curto-circuito de imunidade → sorteio de canal → resolução por canal → um único `round` no final.
- **RNG injetável e seedável** (porta `IRandomSource`, ADR-006): o domínio é PURO; a semente real (data+hora+ms) é injetada na fronteira `app/`, nunca dentro do domínio. O canal (falha/crit/comum) é decidido por UM sorteio do RNG injetado; o 2º roll (variância) só ocorre no canal COMUM. Pillar 1/2: a incerteza é transparente (chances visíveis na UI), não opaca.

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
| `kills` (`knowledgeKills`) | `CombatActor.KnowledgeKills` (SaveSystem) | kills do mesmo tipo de inimigo; alimenta o decaimento de variância E de falha |
| `v` (varianceFactor) | derivado de `kills` | `max(0.05, 0.30 × e^(-kills × 0.10))`; range da arma neste encontro |
| `fumbleChance` | derivado de `kills` | `round(5 × e^(-kills × 0.50))`; chance de FALHA (dano 0), decai a 0 em 5 kills |
| `critChance` | piso global + carta | `max(5, card.CritChance)`; piso 5%, carta eleva acima |
| `card.CritChance` | carta | **bônus acima do piso global de 5%** (§7); 0 = usa o piso 5% |

**Variância Knowledge Decay**: 1º encontro (`kills=0`) → ±30%; conforme o player farma o mesmo tipo, decai exponencialmente até o piso ±5% (`kills` altos). Contra inimigos muito farmados o dano comum fica quase determinístico (Knowledge Progression: conhecer o inimigo remove a incerteza).

**Falha Knowledge Decay**: 1º encontro → 5% de FALHA (dano 0); decai exponencialmente (`k = 0.50`, mais agressivo que a variância) até **0% a partir de 5 kills**. O "tropeço de iniciante" some quando o inimigo já é familiar (anti-frustração, Pillar 4).

### Exemplo numérico (3 canais)

Dado: carta `Power = 20`, atacante `Atk = 10`, alvo `Def = 0`, fraqueza ativa (`multFraqueza = 1.5`), demais mults = 1.0.

`danoBase = (20 + 10) × (100/100) × 1.5 = 45`.

**Caso A — 1º encontro (`kills = 0`):** `v = 0.30`, `maxArma = 45 × 1.30 = 58.5`. `fumbleChance = 5%`, `critChance = max(5, CritChance)`.

| Canal | Condição (roll 0..99, CritChance=0 → crit 5%) | Dano |
|---|---|---|
| FALHA | roll ∈ [0, 4] (5%) | **0** (`FALHA DE COMPILACAO`) |
| CRIT | roll ∈ [5, 9] (5%) | `round(58.5 × 1.5)` = **88** `[CRITICO]` |
| COMUM | roll ∈ [10, 99] (90%) | `round(45 × (1 + (0.30 × 2r − 0.30)))`, r∈[0,1) → faixa **32 a 58** |

**Caso B — inimigo farmado (`kills = 6`):** `v = max(0.05, 0.30 × e^(-0.6)) = max(0.05, 0.165) = 0.165`, `maxArma = 45 × 1.165 ≈ 52.4`. `fumbleChance = round(5 × e^(-3.0)) = round(0.25) = 0` → **sem falha**.

| Canal | Condição (CritChance=0 → crit 5%) | Dano |
|---|---|---|
| FALHA | — (0%) | impossível neste estágio de maestria |
| CRIT | roll ∈ [0, 4] (5%) | `round(52.4 × 1.5)` = **79** `[CRITICO]` |
| COMUM | roll ∈ [5, 99] (95%) | `round(45 × (1 + (0.165 × 2r − 0.165)))`, r∈[0,1) → faixa **38 a 52** |

Leitura de design: conforme o jogador domina o inimigo, a FALHA desaparece e o range do COMUM aperta (variância cai), tornando o dano mais previsível e a maestria perceptível (Knowledge Progression).

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

### 13.1 AP gasto por turno por tier (canon, decisão do criador 2026-06-25)

A FSM permite multi-ação no turno (loop interno de `ActionSelect` consumindo AP, §3). Quanto AP cada inimigo gasta por turno escala por tier:

| Tier | AP/turno (regra de design) | AP/turno no VS (entrega) |
|---|---|---|
| **Trash** | **1** | 1 (correto, já implementado) |
| **Elite** | **2** | 1 (placeholder honesto) |
| **Mini-boss** | **3** | 1 (placeholder honesto) |

- **Racional:** AP por turno escala "quantas decisões o inimigo força" o jogador a antecipar (1 intent a ler no trash, até 3 no mini-boss), aprofundando Scan/Gambito-Prever onde o aesthetic Challenge importa, sem inflar o trash. O trash bate com o ataque básico subtrativo `max(1, Atk - Def)` (§11), determinístico: N AP = N golpes = N× dano, linear e sem teto suavizante. Por isso 1 AP no trash.
- **Por que NÃO 3 AP uniforme:** um único trash a 3 AP derrubaria o Gus (HP 34, menor da party §2.1) em ~3 turnos e tornaria o encontro net-negativo na economia (cura ~15 a 20 cr no Hospital a 1 cr / 3 HP vs ~8 cr ganhos por encontro, economia.md §2 a §3), criando o death-loop econômico que o §3.3 da economia existe para evitar. Trash deve ser fácil (Pillar 1: vitória por leitura, não por aguentar burst).
- **Interação com o auto-kill:** o trash que o jogador ENFRENTA é o NÃO-dominado (o dominado é morto no overworld), logo tem `KnowledgeKills` baixo, o que já embute do lado do jogador variância ±30% + 5% de FALHA (§11). Essa imprecisão já é a tensão da abertura; somar 3 golpes/turno do inimigo puniria o onboarding. 1 AP no trash compensa essa variância e mantém a 1ª batalha vencível sem otimizar (onboarding-vs.md).
- **Análise Preditiva (§2.1) reservada ao elite/boss:** com trash a 1 AP o Gus quase nunca chega a golpe fatal contra trash, preservando o colchão de 1 absorção por batalha (§2.1) para os momentos elite/mini-boss/boss. Trash a 3 AP queimaria a Análise Preditiva em todo encontro trivial.
- **Multi-ação do inimigo (2 a 3 AP) entra apenas com o `UtilityBrain`** (tabela §13): o `ScriptedBrain` resolve uma ação determinística, então 2 a 3 AP só viram decisão interessante quando o UtilityBrain escolhe QUAIS ações (bater + status, ou focar o Gus). A apresentação precisa que o `PacingDirector` (battle-screen.md §5.2) itere o loop de AP do inimigo emitindo um step pausado POR-AÇÃO (N floaters + N linhas de log no tempo certo), em vez do atual 1 ação/turno. Até lá, elite/mini-boss ficam em 1 AP.
- **Entrega do VS:** o código do slice já entrega trash = 1 AP (correto). Elite e mini-boss ficam em 1 AP como placeholder honesto até o `UtilityBrain` + o `PacingDirector` por-ação existirem; a curva 1 / 2 / 3 é a regra de design canônica.

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

O combate comunica via dois buses desacoplados (arquitetura de engine; ver `docs/tech/pivot/engine-design.md`).

> **Nota de stack (2026-06-23, pós-ADR-008).** Esta seção descreve a integração no vocabulário do protótipo **C# + Godot** (signals, `CombatManager` como "ponte Node", `game/tools/TestCombatIntegration.cs`), que morre no M8. No motor **C++20 + SDL3** o equivalente é: o domínio puro (`domain/combat/`) não emite eventos diretamente, apenas acumula a lista de `StatusEffectChange`; a casca da aplicação (`app/`) drena essa lista e a publica no barramento de eventos interno (`core/events/`), sem `Node` nem `signals` de framework. O CONTRATO de dados (acumular mudanças de status numa lista drenável, manter o domínio livre de framework) permanece idêntico; só o nome dos mecanismos muda.

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

public enum CardFamily { Eletrico, Bioquimico, Sonico, Cinetico, Criptografico, Universal }
// Universal (decisão do criador 2026-07-14, achado PS-R1): família das cartas que NÃO
// competem na roda de fraqueza (multFraqueza SEMPRE 1.0). Cobre as ~13 cartas ESPECIAIS
// não-elementais dos mestres (matemáticos/computação/economistas/ocultistas) + o
// "utilitário" já usado em §9 (Shield/Regen/Haste/Slow). NÃO tem Fraco/Resistente/Imune.
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
    int CritChance = 0           // piso de crit por carta (M5-DMG): chance = max(5, CritChance); 0 = usa piso global 5%
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

// NOTA (2026-06-22, M5-DMG): o snippet abaixo é a forma C# ANTIGA do protótipo (morre no M8).
// A fórmula canônica de produção (motor C++) é a §11 evoluída: sorteio de canal FALHA/CRIT/COMUM.
// Mantido aqui apenas como referência histórica do protótipo C#; NÃO é o contrato vigente.
//
//   baseDamage = (Power + Atk) * (100f / (100f + Def)) * multFraqueza * multMod * multCombo;
//   v          = MathF.Max(0.05f, 0.30f * MathF.Exp(-knowledgeKills * 0.10f));
//   rolled     = baseDamage * (1 + (v * 2 * rng.NextDouble() - v));
//   if (CritChance > 0 && rng.Next(100) < CritChance) rolled *= 1.5f;
//   danoFinal  = multFraqueza == 0f ? 0 : (int)MathF.Round(rolled);  // sem clamp mínimo
//
// Forma canônica vigente (§11, motor C++, RNG via porta IRandomSource ADR-006):
//   se multFraqueza == 0 -> 0 (curto-circuito, sem RNG)
//   v            = max(0.05, 0.30 * exp(-kills * 0.10))
//   maxArma      = baseDamage * (1 + v)
//   fumbleChance = round(5 * exp(-kills * 0.50))     // 0% a partir de 5 kills
//   critChance   = max(5, CritChance)                // piso global 5%
//   roll         = rng.Next(100)                     // 1 sorteio de canal
//   FALHA  (roll < fumbleChance)                  -> 0
//   CRIT   (roll < fumbleChance + critChance)     -> round(maxArma * 1.5)
//   COMUM  (resto)                                -> round(baseDamage * (1 + (v*2*rng.NextDouble() - v)))  // 2º roll só aqui
// Ataque básico (subtrativo): danoFinal = Math.Max(1, Atk - Def);   // sem variância/canal/fraqueza
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

Stats para TDD do encontro F2-E.5. HP pós-inflação +60% (Trash 34→55, Elite 89→144; sequência recorrente; TTK alvo 3-5 turnos).

**Party (Gus + 2 companions ativos):**

| Personagem | HP | Atk | Def | SPD | Família |
|---|---|---|---|---|---|
| Gus Vector Tavus Vance | **34** | 8 | 5 | 9 | Todas (compilador universal) |
| Cauã "Volt" Berenger | 55 | 14 | 8 | 13 | Elétrico |
| Jaci "Proxy" Vanderbist | 55 | 9 | 10 | 7 | Bioquímico |

Obs.: HP Gus (34) é o menor da party por hard cap canônico (§2.1).

**Inimigos do encontro de referência:**

| Inimigo | Tier | HP | Atk | Def | Tipo | Fraqueza (1.5×) | Brain (slice) | AP/turno |
|---|---|---|---|---|---|---|---|---|
| Sentinela-Bit | Trash | **55** | **10** (provisório) | 8 | Cinético | Elétrico | ScriptedBrain | 1 (§13.1) |
| Daemon-Guard | Elite | **144** | TBD | 14 | Cinético | Elétrico | ScriptedBrain (placeholder; UtilityBrain = jogo posterior) | 1 no VS, 2 na regra de design (§13.1) |

Notas:
- **Atk do Sentinela-Bit = 10 é PROVISÓRIO** (decisão do criador 2026-06-25): dano no Gus (Def 5) = `max(1, 10-5) = 5`/golpe, ~7 turnos de sobrevida do Gus sob foco a 1 AP (TTK saudável, Pillar 4). A fechar com testes/playtest; não é valor final.
- Atk do Daemon-Guard e SPD dos inimigos = TBD (definir na implementação / playtest).
- AP/turno por tier segue §13.1 (Trash 1 / Elite 2 / Mini-boss 3; no VS todos a 1 AP até o UtilityBrain).
- Cauã (Elétrico) é o DPS natural deste encontro pela roda de fraqueza (§6).

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
- **Vinhas**: Bioquímico ×1.3 (Object +1 dur) / Cinético ×0.85; aplica Root (Slow de magnitude extrema); Knockback anulado em alvo enraizado; Scan revela crescimento em sequência recorrente.
- **Gelo**: Cinético ×1.3 / Bioquímico ×0.66; Break +1 dur; Slow -1; SPD -1 todos; Tavus-Drive +1 mana na 1ª carta do turno.
- **Água / Alagado**: Elétrico ×1.3 / Cinético ×0.85; Stun +1 dur; SPD -1 todos; alvo dentro da água que leva dano Elétrico → Disrupt grátis.
- **Metal-Condutor**: Elétrico ×1.3 / Sônico ×0.66; Corrode +1 dur; Knockback ricocheteia +1 fila adicional; Scan grátis + revela 1 dado extra (Matriz Ortodôntica amplificada).
- **Bioluminescência (SÓ SELVE)**: Sônico ×1.3 / Elétrico ×0.85; Regen / Haste +1 dur; Scan grátis + Prever +1; pulso luminoso em sequência 1-1-2-3-5.

**Novos (5):**

- **T1 Pavimento Tesselado** (cidade forte; variante na Selve): Criptográfico ×1.3 / Sônico ×0.66. **DUAL por rodada completa de fila**, sincronizado a `turnoIndex`: rodada **ÍMPAR = Branco (lapidado)** → +1 dado de Scan grátis a quem scaneia; rodada **PAR = Preto (bruto)** → a 1ª carta Cripto da rodada aplica Expose magnitude 13 grátis no alvo. Telegraph: o quadriculado pulsa branco/preto conforme a rodada.
- **T2 Talude Instável**: Cinético ×1.5 / Criptográfico ×0.66. Pune o INATIVO: ator que não agiu ofensivamente na rodada anterior entra com Slow magnitude 2; ator que gastou ≥2 AP ofensivo na rodada anterior fica imune. Premia agressão, pune turtle. Telegraph: o chão racha sob o ator inativo 1 turno antes. Mexe a fila por comportamento (via Slow).
- **T4 Ashlar Bruto**: Cinético ×1.3 / Elétrico ×0.66. Usar **Defender** aqui → pool de Shield ×1.5 (Magnitude = Def × 1.5). Premia turtle (espelho do T2). Não dá Shield de graça (exige gastar AP em Defender). Transição de **PROGRESSÃO** (entre encontros): arena vencida "lapida" o Ashlar Bruto → vira **T1 Pavimento Tesselado** (bruto → polido).
- **T5 Solo Fértil Recursivo (SÓ SELVE)**: Bioquímico ×1.5 / Cinético ×0.66. Entidades Object plantadas aqui ganham +1 Duration e escalam o efeito por rodada na sequência 1,1,2,3,5 (ex: um totem de Poison aplica magnitude 1,1,2,3,5 ao longo das rodadas). Facilita Poison / Regen / Root. Scan revela o estágio do L-system.
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
| Ofício (bruto ↔ polido) | T1 Tesselado polido ↔ T4 Ashlar bruto |

Vinhas / T5 Solo Fértil = eixo de **tempo / recursão** (sequência recorrente), ainda sem espelho pleno declarado.

### 18.9 Detalhes recorrentes de design

Dois conjuntos de motivos atravessam os terrenos:

- **Numéricos**: durações de período 5/2/5/2; crescimento de T5 em 1,1,2,3,5; pulso de Bioluminescência 1-1-2-3-5; Expose magnitude 13 no T1; raio de Tempestade a cada 3 turnos; T6 a 21 lúmens.
- **Ofício de pedra**: T1 Pavimento Tesselado (ashlar polido preto/branco), T4 Ashlar Bruto, transição bruto → polido.

### 18.10 Escopo de implementação (slice)

- Record **`EnvironmentModifier`** no engine POCO: família-mults, status facilitado, deltas de fila/SPD, hooks de hardware, tipo de camada (terreno/clima/período), tier (visível/codex).
- **`multAmbiente`** plugado na fórmula §11 (default 1.0 = retrocompatível).
- Evento **`CombatBus.EnvironmentSet(envId)`** e evento de mudança de camada (clima/período avançou).
- Verb **Scan-ambiente** (1 AP) para revelar o tier Codex.
- **Aplicação estática por arena**: level-designer marca o ambiente inicial do encontro. **Cartas-clima e inimigos que invocam ambiente = fase posterior** (dados já preparados).

A regra de STACKING das 3 camadas e seu cap final (`multAmbiente ∈ [0.44, 2.25]`) está canonizada em §11.

---

## 19. Resolver sem encarar (auto-resolve opt-in) + Auto-kill (eixo de domínio)

**Status:** canonizado pelo criador supremo em 2026-06-25 (brainstorm colaborativo, 6 decisões via AskUserQuestion + gate de onboarding + princípio ratificado). Materializa a INBOX `COMBATE-AUTOKILL` (ideia 2026-06-23) e a mecânica nova `Resolver sem encarar` (2026-06-25) como um único eixo coerente: **quanto o jogador domina um inimigo determina quanto o sistema "roda sozinho" por ele**.

### 19.1 Conceito e ludonarrativa (Pillar 1 + Pillar 2)

Encarar a batalha = o jogador **compila à mão, otimizando** (escolhe cartas, lê intent via Scan, monta combo no pipeline, usa Gambito). É o caminho ótimo e é onde o aesthetic Challenge/Discovery vive.

`Resolver sem encarar` = o jogador manda o sistema **rodar o build sem otimização (`-O0`, sem as flags táticas dele)**. O build roda, mas é lento e sujo: a party joga no automático mínimo, toma mais dano, colhe menos loot, e pode crashar (party wipe → Hospital, §3.1 da battle-screen). Frase-conceito canônica:

> **Encarar = compilar à mão, otimizando (`-O2`). Não encarar = rodar o build sem otimização (`-O0`): roda, mas lento, sujo, e pode dar core dump.**

Isto reforça Pillar 1 (a lógica é literalmente o que dá o melhor resultado; abrir mão dela custa) e Pillar 2 (magia = software; o registro terminal da batalha trata a luta como uma compilação). NÃO é "force seu caminho": é "se você não usar a cabeça, o sistema resolve pior".

**Princípio ratificado (one-way-ish de design, criador 2026-06-25):** **pular NUNCA pode ser a jogada ótima contra trash não-dominado.** A penalidade (loot reduzido, dano majorado, risco real de Hospital) tem que doer o bastante para que ENCARAR seja o default de quem joga bem. Se o playtest mostrar que pular vira a jogada racional contra trash não-dominado, a penalidade sobe ou a mecânica é revista. Mede-se em playtest (§19.8).

### 19.2 O eixo de domínio (auto-kill × auto-resolve × encarar)

As duas mecânicas vivem em pontos diferentes do espectro de Knowledge do inimigo. O selo de domínio no bestiário (Bronze/Prata/Ouro, alimentado por `KnowledgeKills`) define o comportamento:

| Domínio do inimigo (selo) | O que acontece ao esbarrar | O jogo pergunta? |
|---|---|---|
| **Ouro** (dominado) | **Auto-kill silencioso** no overworld: NÃO monta a arena; mata com micro-animação no próprio mapa; concede loot + Knowledge BÁSICO. | Nunca (silencioso e automático) |
| **Ouro, 8% (sequência numérica recorrente)** | "O bug resistiu / mutou" → cai DIRETO na batalha (monta a arena). | Não (cai direto) |
| **Bronze ou Prata** | Monta a arena; abertura PARA e espera input; **[Resolver sem encarar] disponível** como verbo opt-in. | Só se o jogador apertar o verbo |
| **Sem selo** (1º contato / quase nenhum kill) | Monta a arena; abertura PARA e espera input; **[Resolver sem encarar] NÃO aparece** (gate de onboarding). | Não (sem opção de pular; encara) |

Leitura de design: conforme o jogador domina o inimigo, o "rodar sozinho" evolui de IMPOSSÍVEL (sem selo, encara obrigatório) → OPÇÃO custosa (Bronze/Prata, auto-resolve com penalidade) → AUTOMÁTICO grátis (Ouro, auto-kill com loot básico). É o mesmo verbo conceitual ("o sistema roda sozinho") em três níveis de maestria, casando perfeitamente com a Knowledge Progression (Pillar 1).

**Gate de onboarding (canon 2026-06-25):** `Resolver sem encarar` só é oferecido a partir do selo **Bronze** naquele tipo de inimigo. Sem selo (jogador ainda não conhece o inimigo), cai direto na luta. Razão: pular às cegas um inimigo desconhecido fere Pillar 1 (informação habilita ação) e o onboarding; o jogador precisa ter lutado o suficiente para o jogo saber estimar o risco (§19.6).

### 19.3 Escopo: SÓ TRASH (canon 2026-06-25)

`Resolver sem encarar` e o auto-kill aplicam **somente a inimigos Trash**. Boss, mini-boss, elite e qualquer luta scriptada de história **sempre encaram**, sem opção de pular. Mesma fronteira do auto-kill original (só-trash) e do Flee (§14: bloqueado em mini-boss/boss). Em lutas não-trash o verbo simplesmente não existe na abertura.

- **Por que:** elite usa `UtilityBrain` (intent complexo, multi-AP §13.1) e boss é onde o sistema brilha e a narrativa pesa; deixar pular esvaziaria o tier de maior Challenge e seria absurdo narrativo. Coerente com 3 sistemas já canônicos.

### 19.4 Fluxo de UX (Opção 1B + abertura-espera-input, canon 2026-06-25)

**Default é ENCARAR. Não há splash S/N em toda luta** (anti-pillar de fricção; o auto-kill canon já manda "o jogo NUNCA pergunta por inimigo"). A abertura da batalha PARA e espera o input do jogador, o que de quebra resolve o ritmo (a luta só começa quando o jogador manda).

Sequência ao esbarrar num Trash não-dominado (Bronze+):

1. **Transição de entrada** (battle-screen §3.3): boot/scanline ~0.5s, a arena monta.
2. **Estado de HOLD na abertura** (battle-screen §5.2 D10, agora com espera de input): a arena monta PARADA, exibe **"BATALHA!"** + a fila CTB; NINGUÉM agiu ainda. A tela espera o jogador:
   - **[Encarar] (Enter):** inicia o combate normal; os turnos passam a animar um a um com o `PacingDirector` (D8-D12). Custo ZERO de atrito para quem encara (99% das lutas).
   - **[Resolver sem encarar] (tecla dedicada):** SÓ aparece para Trash com selo Bronze+. Ao apertar, abre o **aviso de consequências** (§19.6) com o rótulo de risco; o jogador confirma ou volta para encarar.
3. Quem não aperta o verbo simplesmente encara (default). O verbo é um gesto deliberado, opt-in, nunca imposto.

Notas:
- A abertura-espera-input substitui a abertura que "começava parada e animava sozinha"; agora **o 1º turno só dispara com o input do jogador** (resolve o feedback de playtest "a tela aparece com o ataque já feito").
- NÃO se memoriza a escolha por inimigo (comportamento implícito seria confuso). O atalho global para pular em massa é o toggle de 3 estados (§19.5).
- Em lutas sem selo / não-trash, o verbo não aparece; a abertura ainda PARA e espera [Encarar] (Enter) para iniciar (o hold é universal; o verbo de pular é condicional).

### 19.5 Toggle "Auto-resolver" do HUD: 3 estados (Opção 4B, canon 2026-06-25)

O toggle do HUD previsto na INBOX do auto-kill ganha **3 estados** (em vez de liga/desliga), expressando "quanto eu deixo o sistema rodar por mim":

| Estado | Comportamento | Para quem |
|---|---|---|
| **Encarar tudo** | Luta tudo na mão, INCLUSIVE o dominado (Ouro não é auto-killado). | Modo treino / bônus (lutar de verdade dá o bônus de eficiência, §3.1). |
| **Auto só dominado** (DEFAULT) | Auto-kill silencioso no Ouro; abaixo de Ouro monta a arena e o jogador encara (com [Resolver sem encarar] à mão como override pontual em Bronze+). | Andar sem atrito numa tela cheia, lutando o que ainda vale. |
| **Auto máximo / "modo pressa"** | Auto-kill no Ouro + auto-resolve AUTOMÁTICO no Bronze+ também (assume as penalidades sem perguntar caso a caso). | Atravessar uma área já grindada rápido, aceitando o custo. |

- O verbo per-luta [Resolver sem encarar] (§19.4) é o override pontual no estado DEFAULT (não precisa mexer no toggle para pular UMA luta).
- O auto-kill no Ouro é silencioso e automático em "Auto só dominado" e "Auto máximo" (nunca pergunta por inimigo, INBOX canon).
- Trash sem selo NUNCA é pulado por nenhum estado do toggle (gate de onboarding, §19.2): só Bronze+ é elegível ao auto-resolve.

### 19.6 Cálculo do auto-resolve (Opção 2C: FSM headless + IA sub-ótima, canon 2026-06-25)

O auto-resolve NÃO usa fórmula fechada paralela. Roda o **próprio motor** (`CombatStateMachine`, POCO puro, já roda headless sem janela: é como os testes rodam) com uma **IA de party deliberadamente sub-ótima**. O resultado pior EMERGE da falta de otimização, não de um número mágico colado por cima.

**`AutoResolveBrain` (IA de party sub-ótima, POCO testável):**
- Só **ataque básico** (subtrativo, §11) + no MÁXIMO **1 carta single-target da família dominante** da party naquele encontro.
- NÃO usa Gambito (sem Prever, sem Reordenar).
- NÃO monta combos (pipeline nunca passa de 1 carta; zero `multCombo`).
- NÃO usa Scan (logo NÃO habilita Null/Expose; perde `multExpose`).
- NÃO lê intent (toma os golpes que viriam).

Como a IA não otimiza, a simulação naturalmente: tarda mais turnos (compile time alto, §3.1), toma mais dano (sem mitigação tática), e pode terminar em party wipe contra um Trash de risco mais alto. As penalidades adicionais de loot/dano por selo ficam parametrizadas em `economia.md` (§19.7).

**Determinismo / RNG:** a FSM consome o RNG injetado (`IRandomSource`, ADR-006) com a variância Knowledge real (§11). O auto-resolve usa a MESMA seed/fonte da estimativa de risco (§19.6) para que **classificação de risco e resultado nunca se contradigam** (um inimigo marcado "risco baixo" não pode acabar em wipe por divergência de cálculo: seria mentira do sistema e frustração legítima). Roda em <1ms (sem render).

### 19.7 Sinalização de risco + parâmetros econômicos (Opção 6A, canon 2026-06-25)

Antes de confirmar o pulo, o aviso de consequências (§19.4) mostra a **chance de derrota como rótulo qualitativo: baixa / média / alta**, derivada do Knowledge e do poder relativo party × inimigo (mesma simulação do §19.6). Tematizado no registro terminal: `risk assessment: LOW / MEDIUM / HIGH`.

- **Por que rótulo e não porcentagem exata:** legível para o público-alvo (prodígio de 11 anos, Pillar 4), sem convidar o min-max frio; e uma % exata de uma simulação com variância sugere uma precisão que não existe.
- **Risco ALTO = aviso explícito de Hospital** (canon 2026-06-25): se a chance for alta, o aviso adverte que pular pode mandar a party para o Hospital (`HIGH risk: pode mandar a party pro Hospital`) e exige confirmação consciente.
- **Hospital ao pular** usa o MESMO fluxo do §3.1 da battle-screen (safe mode grátis OU cura a crédito); nada novo na economia. A tensão é legítima porque o jogador escolheu pular um inimigo de risco alto e confirmou ciente; o que NUNCA pode acontecer é o auto-resolve mandar ao Hospital um inimigo classificado "risco baixo" (garantido pelo cálculo único, §19.6).

**Parâmetros econômicos (em `economia.md`, escritos pelo economy-designer em paralelo):**
- `x` = redução de loot por pulo, por selo (Bronze/Prata; Ouro abaixo entra no auto-kill com loot básico).
- `y` = dano majorado / penalidade do auto-resolve por selo.
- Faixas de `P(derrota)` que mapeiam para os rótulos baixo/médio/alto.
- O bônus de "lutar de verdade" (eficiência de build, §3.1) e o loot básico do auto-kill ficam calibrados lá, sob o princípio §19.1 (pular nunca é a jogada ótima contra trash não-dominado).

Este documento define a MECÂNICA e o PRINCÍPIO; a calibração numérica é de `economia.md`.

### 19.8 Tematização no terminal de resultado

O fim de um combate pulado imprime no terminal de resultado (battle-screen §3.1) um **build NÃO-otimizado**, distinto do build à mão:

- **Pulou e sobreviveu:** `building (no optimizations, -O0)...` → `warning: combat resolved unattended` → `BUILD SUCCEEDED` com loot reduzido (sem rótulo de elogio de eficiência; o `-O0` já comunica "lento/sujo").
- **Pulou e deu wipe:** `building (no optimizations, -O0)...` → `BUILD FAILED` / `core dumped` → Hospital (fluxo §3.1).
- **Encarou e venceu rápido:** o canon §3.1 já cobre (`clean build, -O2, blazing fast` + bônus de eficiência).

O contraste `-O0` (pulou) vs `-O2` (encarou e otimizou) é o feedback diegético da escolha (Pillar 2).

### 19.9 Escopo de implementação

- **`AutoResolveBrain`** (POCO testável) em `domain/combat/`: heurística mínima descrita em §19.6. Isolável, testável sem janela.
- **Função de auto-resolve**: instancia o encontro, injeta `AutoResolveBrain` na party + o mesmo brain inimigo do encontro, roda a `CombatStateMachine` headless até `CombatEnd`, devolve o payload (vitória/wipe + dano + loot bruto). PURA, sem framework.
- **Estimativa de risco**: a mesma simulação rodada 1x (ou amostrada) classifica baixo/médio/alto antes do aviso (§19.7); compartilha a fonte de RNG com o auto-resolve real.
- **Selo de domínio**: lê o `KnowledgeKills` do save (já portado, M3) e mapeia para Bronze/Prata/Ouro (thresholds = `economia.md` / bestiário; gate Bronze para o verbo).
- **Apresentação** (battle-screen, `app/`): estado de HOLD na abertura com [Encarar]/[Resolver sem encarar] (§19.4 / battle-screen §5.2), aviso de consequências com rótulo de risco, toggle de 3 estados no HUD, terminal de resultado `-O0` (§19.8). O auto-kill silencioso no overworld é apresentação de overworld (micro-animação por arquétipo + falas-balão, INBOX canon), fora da BattleScreen.
- O motor (`domain/`, `core/`) permanece POCO puro; o auto-resolve é uma chamada à FSM existente com um brain diferente, não um novo sistema de regras.

---

**Última revisão:** 2026-06-25 (§4.1 Janela de Comando da Party, modelo 1B: comando livre da party dentro do bloco com SPD decidindo qual lado abre a rodada; decisão D1/D2 do criador via AskUserQuestion; criador aceitou o contra-argumento de preservar o SPD como stat; FSM e §11 não mudam, extensão aditiva `PendingPartyActors`; sistemas fila/Gambito/AP/Haste-Slow/ambientes-SPD/cast-time/auto-resolve/ITEM-SPD todos preservados; bônus de iniciativa pendente de playtest D5; target selection entra já, comando-livre-sobre-o-CTB logo após D6). Revisão anterior nesta data: §19 Resolver sem encarar + auto-kill, eixo de domínio. Nova seção canoniza a mecânica opt-in de pular combate de Trash (Opção 1B + abertura-espera-input), o cálculo via FSM headless com `AutoResolveBrain` sub-ótima (Opção 2C), a tematização `-O0`/`-O2` no terminal (D-C), o toggle de HUD de 3 estados (Opção 4B), escopo só-Trash (Opção 5A), rótulo de risco baixo/médio/alto com aviso de Hospital no alto (Opção 6A), o gate de onboarding em Bronze, e o princípio "pular nunca é a jogada ótima contra trash não-dominado". Parâmetros econômicos (x/y por selo, faixas de P-derrota) em `economia.md` (economy-designer, paralelo). Revisão anterior 2026-06-22 (M5-DMG). §11 evoluída: sorteio único de canal FALHA/CRIT/COMUM sobre a variância Knowledge preservada; FALHA decai com a maestria (`round(5 × e^(-kills × 0.50))`, 0% a partir de 5 kills); CRIT com piso global 5% elevável por `card.CritChance`, dano = `round(danoBase × (1+v) × 1.5)`; imunidade `multFraqueza==0` em curto-circuito antes do RNG; ordem de consumo do RNG documentada (1 sorteio de canal, +1 só no COMUM); RNG via porta `IRandomSource` (ADR-006, domínio puro); §7 `CritChance` redefinido como piso por carta. A §11 deixa de ser paridade com o C# (que morre no M8); o motor C++ segue esta §11. Revisão anterior 2026-06-03 (D.1+N.2 Sprint 1 W2): HP +60% Trash 34→55 / Elite 89→144; AP e Mana por-ator CTB (§5); §2.1 contrato fragilidade Gus (N.2 R1, one-way door); §10 feedback ERRO DE COMPILAÇÃO (N.2 R2); §12/§13/§15 caos Perlin exclusivo Patch-Zero (N.2 R3, one-way door); §17 stats de referência pós-inflação + roda de fraqueza confirmada. Revisão 2026-05-26: §18 ambientes de combate, §11 multAmbiente + stacking 3 camadas, escopo slice item 15. Status: canônico, pronto para implementação TDD F2-E.5.

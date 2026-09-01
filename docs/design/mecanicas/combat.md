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
| Prever / Reordenar | Óculos Táticos | Gambito: lê o intent e ajusta o relógio de ação (`ActionClock`, `ADR-017`) |
| Compilar / Jogar carta | Tavus-Drive | executa cartas e pipeline de combo |

---

## 2. Parâmetros macro do combate

| Parâmetro | Valor canônico | Notas |
|---|---|---|
| `MaxPartySize` (engine) | 1 a 4 (configurável) | módulo de engine reutilizável aceita 1-4 |
| Party do jogo GusWorld | **3** | Gus + 2 companions ativos |
| Inimigos por encontro | 1 a 4 | encontros assimétricos são válidos (ex: 3 party vs 1 mini-boss) |
| Ordem de turno | **Relógio de ação por ator** (`ActionClock`, `ADR-017`) | escalonador por instante-de-próxima-ação (inteiro); não é mais fila nem rodada global. Reconciliado com este canon em 25/08/2026 (decisão do líder); ver §3/§4 |
| `AP` por turno | **3 fixo** | cresce via skill tree em jogo posterior; no slice é constante. ⚠️ Lacuna de spec aberta sobre ativação (uma ação por vez vs multi-ação): ver nota em §3 |
| `Mana` (Compilação) | vazão: ramp linear | `manaMax = 2 + contagemPropriaDeTurnos`, cap 8, é a VAZÃO — taxa máxima de saque por turno da bateria ativa, não um pool do ator (§5, "Estoque e vazão"). Contagem por-ator, substitui o antigo `turnoIndex` global (ver §5) |
| Carry-over de vazão | **nenhum** | a vazão reseta e cresce todo TurnStart; impossível bankar. O ESTOQUE da bateria (a carga real) não recarrega sozinho e persiste entre turnos e batalhas (`cartas-hardware-pirataria-energia.md` §5) |
| Determinismo no slice | total (variância 0) | RNG visível plugado mas com variância zerada na entrega F2-E.5 |

### 2.1 Contrato de fragilidade do protagonista (Pillar 4, one-way door)

Decisão canônica N.2 R1, 2026-06-03:

- **HP de Gus é sempre o menor da party** (hard cap no statline; nenhum companion pode ter HP base igual ou menor que Gus).
- **Análise Preditiva (1× por batalha)**: se Gus tomaria dano fatal (HP → 0 ou abaixo), a mecânica absorve o golpe e Gus sobrevive com 1 HP. Não se acumula, não recarrega durante a batalha. Anunciado na UI: `ANÁLISE PREDITIVA: golpe fatal absorvido`.
- Reforça Pillar 4 mecanicamente: prodígio de 11 anos não é tank; vence por lógica (Scan/Gambito/Compilação), não por resistência bruta.

---

## 3. Máquina de estados do combate (FSM)

FSM por-ator. Cada ator (party ou inimigo) toma seu turno quando o **relógio de ação** o elege (`ActionClock`, `ADR-017`, reconciliado com este canon em 25/08/2026 por decisão do líder; ver §4). Não existe mais fila ordenada por SPD nem rodada com `turnoIndex` global. Multi-ação no turno acontece dentro de `ActionSelect` via loop interno consumindo AP.

```
        ┌──────────────┐
        │  SetupPhase  │  (instancia atores, inicializa o relogio de acao: instante
        └──────┬───────┘   corrente = 0, next_action_at inicial por ator; dispara
               ▼            CombatBus.CombatStarted)
        ┌──────────────┐
   ┌───▶│  TurnStart   │  (o relogio elege o ator de menor next_action_at, §4;
   │    └──────┬───────┘   recalcula a vazao (mana) do ator eleito: manaMax = 2 +
   │           ▼            contagemPropriaDeTurnos, cap 8; aplica tick de status:
   │                        Poison/Regen/Duration--; reseta AP = 3, ver nota abaixo)
   │    ┌──────────────┐
   │    │ ActionSelect │  ◀─┐  loop interno: jogador/AI escolhe ação enquanto AP > 0 e não "passar"
   │    └──────┬───────┘   │  (Scan / Gambito / atacar / defender / jogar carta / flee / passar)
   │           │           │
   │           ▼           │
   │    ┌──────────────┐   │
   │    │ ActionResolve│───┘  resolve a ação escolhida (dano, status, pipeline de combo,
   │    └──────┬───────┘      ajustar o relogio de outro ator, etc). Volta a ActionSelect se AP > 0.
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

- `SetupPhase → TurnStart`: o relógio elege o ator de menor instante-de-próxima-ação, pela ordem total de desempate (§4). Não existe mais "lado que abre a rodada": qualquer ator, de qualquer lado, pode ser o primeiro a agir.
- `TurnStart → ActionSelect`: mana e AP do ator são preparados ANTES da seleção. ⚠️ **Lacuna de spec aberta (parecer do CTO, 25/08/2026; L-11, decisão pendente do líder).** O `ADR-017` reseta o relógio "pós-ação" com a fórmula tratando uma carta por ativação (estilo Pokémon Legends: Arceus); não diz o que acontece com os 3 AP fixos e o loop de multi-ação deste laço, que é o modelo deste documento. Os dois modelos são incompatíveis nesse ponto, e o D5 não decide isso sozinho. Este documento mantém o laço de `AP = 3` porque é a única regra escrita até a decisão; ela fica **para a onda de combate**.
- `ActionSelect ⇄ ActionResolve`: laço interno. Cada ação custa AP. Quando `AP == 0` ou o ator escolhe "passar", sai do laço.
- `TurnEnd → CheckEnd → TurnStart`: avança para o ator seguinte. O relógio reseta o `next_action_at` do ator que acabou de agir (fórmula em §4); o próximo a agir, de qualquer lado, é sempre o de menor instante-de-próxima-ação no relógio único (§4.1 descreve como o jogador comanda os membros da party quando mais de um fica pronto ao mesmo tempo). A contagem própria de turnos de cada ator, que alimenta o ramp de mana, substitui o antigo `turnoIndex` por rodada completa de fila.
- Qualquer ator que chega a HP 0 é resolvido conforme regra de morte/incapacitação (companions incapacitados, Pillar 4; inimigos removidos do relógio).

---

## 4. Relógio de ação e prontidão

Não existe mais fila ordenada por SPD. A ordem de ação é resolvida por um **relógio de ação por ator** (`ActionClock`, `ADR-017`, reconciliado com este canon em 25/08/2026 por decisão do líder). Cada ator vivo tem um instante-de-próxima-ação (inteiro); o relógio elege sempre o ator de menor instante-de-próxima-ação, por uma ordem total de desempate (abaixo). O instante corrente do relógio é monotônico: só anda para a frente, nunca recua, e nenhum ator recebe instante-de-próxima-ação menor que o instante corrente. Isto é mecânica central, não cosmético: o Gambito opera sobre este relógio.

### Ordem total de desempate

Ao empatar em instante-de-próxima-ação, a ordem segue, nesta sequência, até resolver:

1. Menor instante-de-próxima-ação.
2. Em empate, menor instante-em-que-agiu-pela-última-vez.
3. Em empate, maior velocidade efetiva (SPD).
4. Em empate, preferência do jogador (quando existir escolha do lado do jogador).
5. Em empate ainda, **identificador único do ator**. Chave final obrigatória: sem ela, dois inimigos idênticos empatam e a ordem cai na ordem do contêiner, a armadilha 4 da L-17 (achado do parecer do CTO, 25/08/2026; o `ADR-017` termina a cadeia em "preferência do jogador", que nem sempre existe).

### Reset pós-ação (aritmética inteira)

```
next_action_at(ator) = instante_corrente + resultado_inteiro_de(
    BASE_CLOCK, velocidade_efetiva(ator), fator_da_acao(carta), fator_do_estilo(estilo, cadeia_de_agil)
)
```

- `BASE_CLOCK` e os fatores de ação/estilo são parâmetros nomeados, marcados `//PLAYTEST`, carregados como dado, nunca literais no corpo da regra.
- **A cadeia inteira é aritmética inteira, com uma única regra de arredondamento declarada.** ⚠️ Ponto de risco marcado pelo parecer do CTO (25/08/2026): a redação original do `ADR-017` escreve a fórmula com ponto flutuante intermediário (`BASE_CLOCK / SPD × 0.85 × 0.55`); com a matriz de cinco plataformas de CI (L-20, MSVC incluído), contração de operações e ordem de avaliação podem divergir um bit e mudar o arredondamento. Este documento fecha essa porta: os fatores de ação e de estilo são expressos como razões inteiras (por exemplo 85/100, 55/100), e a divisão por velocidade e a aplicação dos fatores seguem sempre a mesma ordem de operações e a mesma regra de arredondamento, qualquer que seja a plataforma. A regra exata de arredondamento é detalhe de implementação; o que este canon fixa é que ela é ÚNICA, e o detector permanente é o replay byte-exato do D13.

### Ação agendada (cast interpretado)

Cartas com resolução em atraso (`combat-flavor.md §1`, "interpretada") não saem do relógio: agendam um evento de resolução no mesmo relógio, com um instante-de-resolução. O evento é cancelável pelos gatilhos canônicos (Stun/Disrupt/Silence/dano) antes do instante de resolução. A fronteira exata (um instante antes de resolver, versus o próprio instante) é caso de teste obrigatório.

### Trava anti-inanição

Nenhum ator da party fica sem agir além de um limite parametrizado (`//PLAYTEST`); ao estourar, o motor aplica um catch-up determinístico ao instante-de-próxima-ação do ator. A trava é função do estado, nunca de sorteio.

### O relógio não consome sorteio

A seleção de quem age (posição no relógio, desempate, catch-up) nunca consome RNG. A ordem de consumo do sorteio (§11) continua exclusiva da fórmula de dano: o relógio decide QUEM age, nunca QUANTO.

### Operações que ajustam o relógio (substituem `ReorderActor`)

As primitivas que empurravam um ator numa fila viram ajustes diretos ao instante-de-próxima-ação do alvo:

- **Gambito-Reordenar** (2 AP): soma ticks ao `next_action_at` do alvo, empurrando-o para mais longe do próprio próximo turno.
- **Knockback** (status, §9): soma ticks ao `next_action_at` do alvo afetado, one-shot.
- **Haste / Slow** (status, §9): mexem na velocidade efetiva do ator, o que muda o RESULTADO da próxima fórmula de reset acima; não existe mais fila pra recomputar.

Como o instante corrente só anda para a frente, empurrar um ator não pode fazê-lo "repetir o turno" nem "pular" um vizinho: essas duas classes de bug (achado QA do modelo antigo, corrigidas à mão duas vezes) somem por construção no relógio.

---

## 4.1 Comando da party sobre o relógio (escolher entre os prontos + segurar)

**Status:** canonizado pelo `ADR-017` (16/07/2026, decisão do líder), reconciliado com este canon em 25/08/2026 (parecer do CTO, decisão do líder). Substitui o modelo 1B (§3, nota superada) de "comando livre dentro do bloco da party sobre uma fila CTB por SPD". Não existe mais "bloco da party" nem "bloco de inimigos" tomando a rodada inteira: cada ator, de qualquer lado, fica pronto no seu próprio instante (§4), e os dois lados se intercalam de verdade no relógio.

### Conceito

Quando um ou mais membros da party ficam prontos (elegíveis pelo relógio, §4), o jogador:

- **Escolhe qual membro pronto age** (ação + alvo), entre os prontos no momento; ou
- **Segura** um membro pronto, esperando outro ficar pronto também, para decidir a ordem dos dois.

O jogador sempre escolhe o alvo da ação (single/AoE), isso é ortogonal ao relógio e não muda (modo-mira, `battle-screen.md §3.5`).

**O que se perde em relação ao modelo 1B:** forçar um membro a agir ANTES do relógio dele deixar. O "comando livre dentro do bloco" caía nisso; a party deixa de ser um bloco.

### Por que SPD continua sendo um stat com peso

O SPD não decide mais "qual lado abre a rodada": não existe mais rodada nem lado. No relógio (§4), o SPD entra direto na fórmula de reset: quanto maior a velocidade efetiva, menor o `next_action_at`, ou seja, o ator volta a ficar pronto mais cedo. É um efeito estrutural, não um bônus de abertura. SPD também segue decidindo o desempate (posição 3 da ordem total, §4), o alvo/efeito de Gambito-Reordenar, Haste/Slow e ambientes que mexem SPD (§18), e o valor do item de iniciativa (INBOX).

### Impacto nos sistemas canônicos

| Sistema | Efeito no relógio |
|---|---|
| **Relógio de ação (§4)** | É o motor. Cada ator compete individualmente pelo próprio `next_action_at`; os dois lados se intercalam de verdade (honra o pedido do Gus Dragon, "P2 pode agir várias vezes seguidas"). |
| **Gambito-Reordenar (§12)** | Preservado e mais legível: soma ticks ao `next_action_at` do alvo (§4), em vez de mover posição numa fila. |
| **Gambito-Prever (§12, `IntentPreview`)** | Preservado. O `IntentPreview` ganha os campos `EstiloProjetado` + `TicksProjetados` (`ADR-017`, "Transparência"): o jogador lê o estilo e o tempo projetado do inimigo, não só a ação. Gate de teste permanente: `PreviewIntent()` e a decisão real do MESMO inimigo têm de devolver o MESMO estilo pré-comprometido, senão o preview mente. |
| **AP por ator (§5)** | ⚠️ Ver a lacuna de ativação registrada em §3: o `ADR-017` não fecha o que acontece com os 3 AP fixos e o loop de multi-ação. Não decidida aqui. |
| **SetupPhase (§3)** | Inicializa o relógio (instante corrente = 0, `next_action_at` inicial de cada ator); não calcula mais "qual lado abre". |
| **Cast-time / cartas lentas (CARTAS-CAST-TIME, INBOX)** | Preservado como resolução agendada no mesmo relógio (§4), não mais como "posição à frente na fila". |
| **Haste / Slow (§9/§18)** | Preservados: mexem na velocidade efetiva, o que muda o resultado da fórmula de reset (§4). |
| **Ambientes que mexem SPD (§18)** | O efeito de SPD sobre o relógio (reset mais cedo) substitui o efeito antigo sobre "posição na fila/rodada". Os efeitos específicos do §18 que ainda citam "posição-de-fila" ficam fora do escopo desta reconciliação (relatório do agente que fez esta edição, item 6). |
| **ITEM-SPD-INICIATIVA (INBOX)** | O item aumenta SPD; no relógio isso significa resets mais rápidos, não mais "abrir a rodada". Precisa de re-derivação; fora do escopo desta reconciliação. |
| **Análise Preditiva / fragilidade do Gus (§2.1)** | Intacta. |
| **Fórmula de dano (§11)** | NÃO muda. O relógio decide seleção de ator, não resolução de ação. |

### Bônus de iniciativa (PENDENTE de playtest, D5)

Opcional, decidido depois com dados: o ator que fica pronto primeiro por SPD poderia ganhar um pequeno bônus tático (ex.: na linha do que o T8 Elevação já dá de Haste), reforçando que correr na frente vale sem ser dominante. Registrado como pendência; não entra agora. Mede-se em playtest.

### Nota técnica (mudança de contrato, não de camada)

A implementação troca o escalonador: `InitiativeQueue` (array + cursor) morre; `ActionClock` (heap por `next_action_at`) nasce. Isso é REWRITE, não extensão (`ADR-017`, "Migração"): a fila por-lado (`PendingPartyActors`) do modelo 1B deixa de existir; o estado passa a ser "conjunto de atores prontos neste instante", de qualquer lado. Os testes de transição de FSM que não dependiam de ORDEM (dano, status, roda de fraqueza) continuam válidos; os de ordem/cursor/rodada são re-derivados sobre o relógio (L-19, TDD estrito: o contrato é testável antes de existir código).

- **Campo novo no estado, por ator:** `next_action_at` (inteiro), `last_acted_at` (inteiro), contagem de ágeis consecutivos, contagem própria de turnos (substitui `turnoIndex` global, §2).
- **Elegibilidade:** qualquer ator vivo com `next_action_at <= instante_corrente` está pronto; entre os prontos do lado do jogador, o jogador escolhe quem age ou segura.
- **Avanço do relógio:** o instante corrente avança para o menor `next_action_at` pendente sempre que ninguém está pronto.
- **Determinismo preservado:** a ordem de consumo do RNG (§11) não muda; a seleção de ator nunca consome sorteio (§4).
- **Escopo (D6):** o target selection (modo-mira, zero motor, battle-screen §3.5) entra JÁ; o comando sobre o relógio (extensão do motor acima) entra logo após, briefado ao backend-engineer + gameplay_engineer.

---

## 5. Recursos: AP e Mana

Dois recursos, propósitos distintos. AP limita **quantas** ações; Mana limita **quão caras** as cartas.

### AP (Action Points)

- `AP = 3` fixo por turno no vertical slice.
- Reseta no `TurnStart`. Sem carry-over.
- Cresce via skill tree em jogo posterior (fora do escopo do slice; o campo é parametrizável).
- **Por-ator** (relógio de ação, §4): AP pertence ao ator eleito pelo relógio. Cada membro da party tem seus 3 AP independentes quando age. Canonizado D.1 Sprint 1 W2, 2026-06-03. ⚠️ Ver a lacuna de ativação registrada em §3 (AP=3/multi-ação vs uma carta por ativação).

### Mana / Compilação (bateria: estoque × vazão, decisão do líder 31/08/2026)

**Mana e a carga de bateria são o MESMO recurso, com duas propriedades físicas de uma só bateria**
(`cartas-hardware-pirataria-energia.md` §5, "Mana e bateria são o mesmo recurso" e "Múltiplas
baterias e a barra da tela"): **estoque** (a carga disponível na bateria ativa — persiste entre
batalhas, degrada, exige recarga real) e **vazão** (a taxa máxima de descarga por turno).

- `manaMax = 2 + contagemPropriaDeTurnos`, com `cap = 8`, é a **vazão**: o quanto o ator PODE sacar
  da bateria ativa naquele turno — não um orçamento próprio que nasce do nada a cada turno. O
  número não muda; muda só a natureza do que ele mede. Contagem por-ator, substitui o antigo
  `turnoIndex` global (reconciliado com `ADR-017` em 25/08/2026; ver §2).
- **O limite real de cada turno é o MENOR dos dois: a vazão do turno, ou o que ainda resta de
  estoque na bateria ativa.** Nunca se saca mais do que a bateria tem, mesmo que a vazão do turno
  permitisse mais.
- A **vazão** cresce e reseta a cada `TurnStart`, como antes (sem mudança de número). **O estoque
  NÃO recarrega no `TurnStart`** — é a carga real da bateria, e só sobe por recarga de verdade
  (troca de bateria, estação de recarga, cidade; `cartas-hardware-pirataria-energia.md` §5, "Troca
  e recarga"). "Sem carry-over" segue valendo, mas agora só para a VAZÃO não usada no turno (não dá
  para acumular capacidade de saque de um turno para o outro); o estoque, ao contrário, PERSISTE
  entre turnos e entre batalhas, porque é a bateria — ficar sem estoque no meio de uma batalha passa
  a ser possível, e o mecanismo já está especificado: a carta vira `DEPLETED` (inerte) e o jogador
  troca por outra bateria do inventário via `SwapBattery`, ação de emergência de **2 AP fixo**
  (`cartas-hardware-pirataria-energia.md` §5, "Estoque e vazão"; `cartas-spec-logica.md` §3.2-3.4).
- Ramp linear garante que combos premium só ficam viáveis no mid-late do combate, preservando curva
  de tensão — **contanto que a bateria ativa tenha estoque suficiente**; se não tiver, o teto real é
  o estoque, não a vazão.
- **Por-ator** (relógio de ação, §4): a vazão é calculada por ator, como antes. **A bateria (o
  estoque) é PRÓPRIA DE CADA CARTA, não uma bateria única do ator nem um cinto comum do jogador**
  (decisão do líder, 31/08/2026, `cartas-hardware-pirataria-energia.md` §5): cada carta consome a
  bateria dela; baterias avulsas do inventário são peça de troca (encaixadas numa carta descarregada
  para voltar a usá-la), não um pool do qual as cartas puxam. A frase "cada ator tem seu próprio pool
  de mana independente, sem pool compartilhado entre personagens" (canonizada D.1 Sprint 1 W2,
  2026-06-03) descrevia o modelo antigo de recurso-próprio-do-ator, e é **apagada** por L-24 deste
  projeto: no modelo atual, a mana/carga pertence à CARTA que o ator está usando, não ao ator em si.

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

### Recarga de recurso via carta comum (CARTAS-COMUNS-ENGINE, "Tavus-Overclock", canonizado 2026-07-16)

Elétrico-utilidade (versão forte escolhida pelo líder, não o fallback Haste): carta comum que devolve AP e/ou mana ao próprio conjurador, campos `Card.RestoreAp`/`Card.RestoreMana` (record-base §7, `//PLAYTEST` a carta canônica usa +1 AP / +2 mana). Custo de mana da carta é **sempre pago primeiro** (`spend_mana`), a recarga entra depois — anti-exploit de "a carta paga a si mesma". `RestoreAp` usa a MESMA semântica temporária do bônus de Calc-Edge/Mises (§20 do executor techMagic): não muta `MaxAp`, some no próximo `TurnStart`. `RestoreMana` clampa em `manaMax` (sem overflow). ⚠️ **Sinalizado pelo modelo estoque×vazão (acima), não resolvido aqui:** com mana = carga de bateria, falta dizer se este clamp devolve carga real ao ESTOQUE da bateria (capado pela capacidade física dela) ou se continua capado pela VAZÃO do turno (`manaMax`) — os dois números não são o mesmo sob o novo modelo, e a redação atual não distingue.

**Trava 1×/TURNO (decisão do líder):** sem ela, a carta vira loop infinito de AP/mana dentro do mesmo turno + farm degenerado de Mastery (confirmado pelo economy-designer). Flag por-ator (`CombatActor.OverclockUsed`), **resetada no MESMO `TurnStart`** que zera AP/mana (não é 1×/batalha como a Análise Preditiva §2.1 nem as especiais Ativa/Hibrida — é 1× por turno, todo turno). 2ª tentativa no mesmo turno paga o custo normalmente mas NÃO recarrega; loga o bloqueio (regra "todo efeito loga", inclusive quando não faz nada). Silence bloqueia o cast inteiro (gate no topo de `ResolveUseCard`, mesmo padrão de qualquer carta).

---

## 6. Famílias de carta e roda de fraqueza

### As 5 famílias (identidade não-sobreposta)

Cada família tem identidade mecânica distinta: a sobreposição de papéis é proibida (anti feature-creep). Mapeamento de companion para família estabelece fantasia e cor de jogo.

| Família | Companion-âncora | Identidade mecânica | Status que aplica |
|---|---|---|---|
| **Elétrico** | Cauã "Volt" | burst de dano alto, single-target | Stun |
| **Bioquímico** | Jaci "Proxy" | dano-ao-longo-do-tempo (DoT), degradação | Poison / Corrode |
| **Sônico** | Linda "Siren" | controle de área (área-CC), interrupção | Disrupt, Silence |
| **Cinético** | Bento "Requiem" | impacto físico, atraso no relógio de ação | Knockback, Break |
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
| **Knockback** | Cinético | adia o próximo turno do alvo (one-shot): soma ticks ao `next_action_at` do relógio de ação (§4) |
| **Break** | Cinético | reduz Def do alvo por Duration turnos |
| **Expose** | Criptográfico | aumenta dano de carta recebido pelo alvo (ver detalhe abaixo) |
| **Decrypt** | Criptográfico | anula/remove TODOS os buffs do alvo (qualquer status benéfico); NÃO bloqueia reaplicação (reset, não lockout; alvo pode re-buffar via item/poção/carta). Canon revisado 2026-06-03 (F2-E.5b) |
| **Shield** | utilitário | pool de absorção: absorve TODO dano antes do HP (ver detalhe abaixo) |
| **Regen** | utilitário | cura por tick no TurnStart |
| **Haste / Slow** | utilitário | aumenta/reduz SPD (muda o resultado da próxima fórmula de reset do relógio de ação, §4) |

> **Forma dos efeitos (ratificada 2026-06-03, F2-E.5b — wiring real dos status).** Magnitudes vêm sempre da carta/combo que aplica (parametrizado), nunca hardcoded:
> - **Disrupt:** penalidade multiplicativa de Power `× (1 - Magnitude/100)`, consumida na 1ª carta ofensiva do alvo (literal "próxima ação").
> - **Break:** redução de Def aplicada de uma vez, mantida e restaurada ao expirar. Distinta de Corrode (que reduz Def por tick, acumulativo).
> - **Haste / Slow:** **aditivo** `SPD ±= Magnitude` (clamp 0), aplicado ao entrar e restaurado ao expirar. (NÃO multiplicativo: previsibilidade tática + equilibra o lento + casa com a notação "+1/-1/magnitude N" dos efeitos de ambiente §18.) No relógio de ação (§4, reconciliado 25/08/2026), a mudança de SPD não recomputa nada na hora: ela muda o RESULTADO da próxima fórmula de reset do próprio ator afetado (a velocidade efetiva entra direto na fórmula), o que faz o `next_action_at` dele vir mais cedo (Haste) ou mais tarde (Slow) na próxima vez que a fórmula rodar. Não existe mais fila, cursor nem partição pra preservar.
> - **Decrypt:** dispela qualquer buff benéfico; SEM lockout (ver linha acima).
> - **Knockback (status consumido one-shot ao disparar, `RemoveStatus`):** no relógio de ação (§4, reconciliado 25/08/2026), soma ticks ao `next_action_at` do alvo afetado — adia quando ele volta a ficar pronto, sem tocar em mais ninguém. Como o instante corrente só anda para a frente (§4), o ator afetado não pode nem "repetir o turno" nem "pular" um vizinho: essas duas classes de bug (achado QA do modelo antigo, corrigidas à mão duas vezes) somem por construção no relógio. Continua distinto de Stun (que tira o próximo turno do ator por completo, não apenas o adia).
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

### SynergyStatus (Finalizador-sinérgico, CARTAS-COMUNS-ENGINE, canonizado 2026-07-16)

Generalização de `multExpose` (acima) pra **qualquer** `StatusId`, usada pelas cartas COMUNS finalizadoras (`combat.md` §7, record-base — **não** é `EffectKind` do executor techMagic, ADR-016 continua exclusivo de ESPECIAL/SUPER). Campo `Card.SynergyStatuses` (lista de `StatusId`) + `Card.SynergyPercent` (int, `//PLAYTEST`, a carta canônica usa 40):

```
multSynergy = (alvo tem >= 1 status de SynergyStatuses) ? (1 + SynergyPercent / 100) : 1.0
```

- **Fator FIXO, NÃO stacka por-status-presente**: 2 ou mais status da lista presentes no alvo continuam valendo o MESMO `multSynergy` (é binário presente/ausente, não soma por status).
- Aplica **somente em UseCard**, na MESMA posição ordinal do `multExpose` na cadeia divisiva (§11) — antes do curto-circuito de imunidade e do sorteio de canal. `multFraqueza == 0.0` (imune) tem prioridade absoluta: zera o dano antes de qualquer sorteio, independente de `multSynergy`.
- Sem RNG consumido pelo próprio fator (é determinístico, igual ao Expose).
- Log diegético só quando dispara (regra "todo efeito loga"): sufixo `[SINERGIA: alvo vulnerável, dano +N%]`.
- Exemplo canônico: Tavusa-Fulminante (Elétrico) tem `SynergyStatuses = [Stun]`, `SynergyPercent = 40` → dano ×1.4 contra alvo já Stunado ("execute em alvo travado", `cartas-comuns-statlines.md`).

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
// 1. Cadeia divisiva (CARTAS-COMUNS-ENGINE 2026-07-16 acrescenta multSynergy, mesma posição
//    ordinal do multExpose — nenhum OUTRO fator novo aqui)
multExpose  = alvoTemExpose ? (1 + Expose.Magnitude / 100) : 1.0
multSynergy = (>=1 status de Card.SynergyStatuses presente no alvo) ? (1 + Card.SynergyPercent / 100) : 1.0
danoBase    = (Power + Atk) × (100 / (100 + Def)) × multFraqueza × multMod × multCombo × multExpose × multSynergy × multAmbiente

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

#### Quantum-Lock (Planck) — quantização do canal COMUM (ADR-016 manifesto item 5, canonizado 2026-07-15)

Passiva mana-0 (carta HISTÓRICA — só o Gus equipa; o **motor é agnóstico por-ator**, zero hardcode de personagem no domínio, mesmo padrão do Reflect/Newton). Quando o **atacante** porta a passiva equipada, o canal COMUM deixa de sortear um `r` contínuo e passa a sortear um **degrau discreto** dentro da MESMA faixa da variância Knowledge — elimina os "quase-acertos", sem mexer na largura do range nem na média.

```
// SEM Quantum-Lock (de sempre):
COMUM:  r = rng.NextDouble()                                    // [0, 1) continuo
        danoFinal = round(danoBase × (1 + (v × 2 × r − v)))

// COM Quantum-Lock (atacante porta a passiva equipada):
COMUM:  roll2 = rng.Next(0..99)                                  // SUBSTITUI o NextDouble
        se roll2 < percent               → r = 0.0   // degrau PISO   = danoBase × (1 − v)
        senão se roll2 < percent + magnitude → r = 0.5   // degrau CENTRO = danoBase
        senão                             → r = 1.0   // degrau TETO   = danoBase × (1 + v)
        danoFinal = round(danoBase × (1 + (v × 2 × r − v)))       // MESMA fórmula, r discreto
```

- **3 degraus, chances FIXAS 25% / 50% / 25%** (`percent` = chance de CADA extremo, `magnitude` = chance do centro no `EffectSpec` da carta — `//PLAYTEST`, número fechado pelo criador). Média ponderada = `danoBase` (**zero mudança de balance** vs. o canal COMUM de sempre).
- **Knowledge continua mandando na LARGURA**: `v` (variância) segue a mesma curva de decaimento por `kills` (§11 acima) — os degraus encolhem conforme o jogador farma o inimigo, exatamente como o range contínuo encolhia. Só as CHANCES (25/50/25) não evoluem com Knowledge.
- **Crítico e FALHA intactos** (regra A6): a quantização só troca o sorteio DENTRO do canal COMUM. O 1º sorteio (canal FALHA/CRIT/COMUM) é idêntico com ou sem a passiva.
- **Consumo de RNG**: o canal COMUM com Quantum-Lock consome `rng.Next(100)` **duas vezes** (canal + degrau) e `rng.NextDouble()` **zero** vezes — MESMA contagem total de consumos do canal COMUM de sempre (1 canal + 1 variância = 2), só muda o TIPO do 2º consumo. Sem a passiva equipada, o caminho fica **byte-idêntico** ao de hoje (0 mudança).
- **Preview (perfect information, regra A2):** a UI mostra os **3 valores** (piso/centro/teto) **+ as 3 chances**, calculados PUROS (sem sortear) pelo mesmo helper do canal COMUM — bit-idênticos ao que o motor produziria em cada degrau forçado (gêmeo preview↔real obrigatório).
- **Colapso gracioso**: quando `danoBase` é pequeno e `v` está no piso (~0.05, inimigo muito farmado), os 3 degraus podem arredondar pro MESMO inteiro — comportamento esperado, não um bug (o preview mostra os 3 iguais).
- **Escopo por-ator**: a detecção varre as especiais EQUIPADAS do próprio ATACANTE da ação corrente — outro membro da party sem a passiva segue com a variância contínua normal; um inimigo nunca porta a carta na prática (não é bloqueio estrutural do motor, é o fato de a carta ser exclusiva do Gus na progressão).

**Stacking das 3 camadas ambientais** (terreno + clima + período, canonizado 2026-05-26): os multiplicadores das camadas ativas que afetam a mesma família **se multiplicam** entre si, com **cap final `multAmbiente ∈ [0.44, 2.25]`** (mesmo teto que o sistema já permite via 1.5 × 1.5 = 2.25 e piso via 0.66 × 0.66 ≈ 0.44). A curadoria de transições (§18.6) impede por design que 2 fontes ×1.5 da MESMA família coexistam (nunca atingir 2.25 organicamente fora de janela curta); o cap é a trava de segurança numérica.

| Fator | Origem | Valores |
|---|---|---|
| `Power` | carta | campo do record |
| `Atk` | atributo do atacante | stat |
| `multFraqueza` | roda de fraqueza (§6) | 1.5 / 1.0 / 0.66 / 0.0 |
| `multMod` | modificador aplicado | default 1.0; Stream pode distribuir; valores tabelados |
| `multCombo` | receita de combo (§10) | default 1.0; >1.0 em combo casado |
| `multExpose` | status Expose no alvo (§9) | default 1.0; `1 + Expose.Magnitude/100` se Expose presente; só UseCard |
| `multSynergy` | `Card.SynergyStatuses`/`SynergyPercent` no alvo (§9, CARTAS-COMUNS-ENGINE) | default 1.0; `1 + SynergyPercent/100` se >=1 status da lista presente no alvo; NÃO stacka por-status; só UseCard; mesma posição ordinal do `multExpose` |
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
- Upgrades (jogo posterior): revelam buffs/debuffs ativos e o instante-de-próxima-ação do inimigo no relógio (`combat.md §4`).
- Vira **passivo permanente** contra um tipo de inimigo após N kills (Knowledge Progression): deixa de custar AP contra inimigos farmados.
- Habilita **Null** e **Expose** (pré-condição informacional).

### Gambito (Óculos Táticos)

Dois modos, ambos operam sobre o relógio de ação/intent:

**Prever** (1 AP):
- Lê o `IntentPreview` do inimigo (alvo, dano, área do próximo turno).
- Upgrade (jogo posterior): 2-3 turnos à frente para um inimigo escolhido.
- **Falha contra intent caótico exclusivamente de Patch-Zero**: retorna ruído Perlin em vez de leitura limpa. Mini-bosses têm UtilityBrain sem ruído (intent complexo, mas legível). (N.2 R3, one-way door 2026-06-03.)

**Reordenar / forçar-recuo** (2 AP):
- Soma ticks ao instante-de-próxima-ação do inimigo alvo, empurrando-o para mais longe do próprio próximo turno (Gambito-Reordenar, `combat.md §4`), OU força mudança de alvo do inimigo.
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
| **Sem combo gigante via estoque** | a VAZÃO do turno (`manaMax`) não acumula e reseta a cada `TurnStart` (isso não mudou); o ESTOQUE da bateria, ao contrário, persiste e pode crescer — mas o teto de vazão por turno segue capando quanto se pode sacar numa tacada só, não importa quanto estoque a bateria tenha acumulado (`cartas-hardware-pirataria-energia.md` §5, "Estoque e vazão") |
| **Roda fechada** | relação de fraqueza é determinística e completa; sem família "sempre melhor" |
| **Clamp dano mínimo 1** | impede build de Def infinita que zera dano (exceto imunidade telegrafa) |
| **AP escasso (3)** | toda ação compete por AP; Scan/Gambito custam o turno de ataque (trade-off real) |

### 15.1 Duração-alvo por tier de encontro (decisão líder + Gus, 2026-07-19)

Alvo de sensação por tier (rodadas; conversão informal ~25-30s/rodada, cai com a maestria). O harness de balanceamento (`balance_harness.hpp`, cross-ref §15) valida média+mediana dentro da janela. **Nota de reconciliação 25/08/2026:** a conversão informal era referenciada "no modelo 1B"; o número em si (segundos por rodada, medido em playtest) não dependia do mecanismo de ordem de turno e sobrevive à troca para o relógio de ação (`ADR-017`, §4). Não é número de balanceamento novo, só a citação ao modelo superado foi removida.

| Tier | Rodadas-alvo | ~Minutos | Perfil |
|---|---|---|---|
| **Inimigo normal (trash/elite)** | **3-5** | ~1,5-2,5 min | **Ágil clássico** (Pokémon/SNES): trash rápido, não cansa o público 11+ |
| **Boss** | **12-18** | ~6-9 min | **Tático/pesado** — respira. **Bosses são estilo Zelda** (mecânica/puzzle, não esponja de HP): aguardam BRAINSTORM DEDICADO (ver INBOX `BOSSES-ZELDA-BRAINSTORM`); os números aqui são o envelope de duração, o DESIGN da luta vem depois |
| **Boss final (Sterling, 2 fases / Dragon Victory)** | **~30** (somando fases) | ~15-18 min | **Tático/pesado — TAMBÉM estilo Zelda** (mecânica/puzzle no clímax, não só troca de dano): entra no mesmo `BOSSES-ZELDA-BRAINSTORM`. Evento clímax |

Nota de harness: a janela histórica do normal era 4-8 rodadas; a decisão aperta o trash pra **3-5** (ágil). **Retune concluído em 2026-08-01** (item MIRA-SIM, autorizado pelo líder): `balance_harness.hpp`/`balance_harness_test.cpp` (campo `window_3_5_ok`) e o novo `mira_sim_harness.hpp` (métrica E2, `pct_in_window_3_5`) já validam contra 3-5, não mais 4-8. One-way-door de sensação: fica registrado, revisável só via nova decisão do líder.

---

## 16. Integração com event bus

O combate comunica via dois buses desacoplados (arquitetura de engine).

O domínio puro (`domain/combat/`) não emite eventos diretamente: apenas acumula a lista de `StatusEffectChange`. A casca da aplicação (`app/`) drena essa lista e a publica no barramento de eventos interno (`core/events/`), sem `Node` nem `signals` de framework, mantendo o domínio livre de framework.

### `CombatBus` (eventos internos de combate)

- `CombatStarted(encounter)`
- `TurnStarted(actor, contagemPropriaDeTurnos)` — contagem própria de turnos do ator eleito (reconciliado 25/08/2026; substitui o antigo `turnoIndex` global, §2/§4)
- `ActionResolved(actor, action, result)`
- `StatusApplied(actorId, statusId, magnitude, duration)` / `StatusExpired(actorId, statusId)`
- `ActorDefeated(actor)` / `ActorIncapacitated(companion)`
- `TurnEnded(actor)`
- `CombatEnded(outcome, payload)` // vitória / derrota / fuga

#### Contrato de evento de status (canonizado 2026-05-26)

O POCO **não** emite signals diretamente (mantém-se sem dependência de framework). Em vez disso a FSM **acumula uma lista de `StatusEffectChange`** (mesmo padrão do `Log` de `CombatLogEntry`), drenada dos atores em cada transição de fase relevante (tick / resolução / expire). O `CombatManager` (ponte Node) lê essa lista pós-turno por índice e traduz em signals:

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

> ⚠️ **Nota de L-01 (parecer do CTO, `PARECER-ADR-017.md` §1b; reconciliação 25/08/2026).** Esta seção descreve o escopo de um vertical slice **redigido contra a árvore de código do projeto anterior**, que a L-01 desqualifica como base (nenhum arquivo daquele projeto existe neste repositório). O que sobrevive é a **intenção de escopo** (quais sistemas entram no primeiro corte jogável), não um plano de execução contra um binário existente. Os itens 2 e 4 abaixo, que citavam o modelo de fila e o `turnoIndex` global, foram corrigidos para a linguagem do relógio de ação (`ADR-017`, §4). O resto da lista — os records C# conceituais (§"Contratos de dados", já com sua própria nota de L-01/M5-DMG desde 2026-06-22), o mapa de migração e qualquer referência a evidência de CI do protótipo — permanece descrição de intenção histórica, não spec executável hoje; não foi re-verificado item a item nesta reconciliação.

### Entrega (DEVE)

1. **FSM completa**: SetupPhase, TurnStart, ActionSelect (com loop interno por AP), ActionResolve, TurnEnd, CheckEnd, CombatEnd. Cobertura de testes por transição.
2. **Relógio de ação por ator** (`ActionClock`, `ADR-017`, §4), com ordem total de desempate e reset pós-ação em aritmética inteira. Substitui a antiga fila de iniciativa por SPD e a operação `ReorderActor(actor, deltaPosicao)` com clamp; as primitivas que ajustam o relógio (Gambito-Reordenar, Knockback) somam ticks ao `next_action_at` do alvo (§4). Relógio visível (dado exposto; UI mínima, `battle-screen.md` D4).
3. **AP = 3 fixo** por turno, reset no TurnStart.
4. **Mana ramp (vazão da bateria)**: `manaMax = 2 + contagemPropriaDeTurnos`, cap 8, é a VAZÃO — taxa máxima de saque por turno, que cresce e reseta a cada `TurnStart`, sem carry-over. O ESTOQUE da bateria (a carga real) não recarrega sozinho, persiste entre turnos e batalhas (`cartas-hardware-pirataria-energia.md` §5, "Estoque e vazão"). Contagem por-ator, substitui o antigo `turnoIndex` global (§2).
5. **Records de dados**: `Card`, `CardFamily`, `StatusEffect`, mais enums de suporte (`CardBaseType`, `TargetShape`, `StackRule`, `StatusId`). Imutáveis.
6. **5 famílias como dados** + roda de fraqueza determinística como tabela consultável (`multFraqueza` por par atacante/alvo).
7. **3-4 status concretos**: `Stun`, `Poison`, `Shield`, `Expose` (cobrem aplicar / tick no TurnStart / expirar por Duration / StackRule / dispel).
8. **3 modificadores**: `Object`, `Stream`, `Null` (Null com pré-condição de Scan validada em teste).
9. **Pipeline de 3 slots** com casamento de receita e 1-2 combos mockup (assinatura + `multCombo` + nome `COMPILADO: X`).
10. **Scan**: 1 AP, revela HP + fraqueza; flag de "scaneado" habilita Null/Expose.
11. **Gambito 2 modos**: Prever (1 AP, lê `IntentPreview`) e Reordenar (2 AP, soma ticks ao `next_action_at` do alvo no relógio de ação, `combat.md §4`).
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

### Stats de referência do encontro (canônico D.1 Sprint 1 W2, 2026-06-03; Trash revisado 2026-08-07, item BAL-STATLINES-APLICAR)

Stats para TDD do encontro F2-E.5. HP pós-inflação +60% (Trash 34→55, Elite 89→144; sequência recorrente; TTK alvo 3-5 turnos). **A inflação do Trash foi desfeita em 2026-08-07** (ver tabela e notas abaixo); o Elite permanece em 144 (`ELITE-MEIO-TERMO`, congelado à parte).

**Party (Gus + 2 companions ativos; o terceiro slot é ocupado pelo companion adequado ao encontro):**

| Personagem | HP | Atk | Def | SPD | Família |
|---|---|---|---|---|---|
| Gus Vector Tavus Vance | **34** | 8 | 5 | 9 | Todas (compilador universal) |
| Cauã "Volt" Berenger | 55 | 14 | 8 | 13 | Elétrico |
| Jaci "Proxy" Vanderbist | 55 | 9 | 10 | 7 | Bioquímico |
| Bento "Requiem" Chevalier | 55 | 10 | **13** | 5 | Cinético |

Obs.: HP Gus (34) é o menor da party por hard cap canônico (§2.1).

**Inimigos do encontro de referência:**

| Inimigo | Tier | HP | Atk | Def | Tipo | Fraqueza (1.5×) | Brain (slice) | AP/turno |
|---|---|---|---|---|---|---|---|---|
| Sentinela-Bit | Trash | **33** | **12** | 8 | Cinético | Elétrico | ScriptedBrain | 1 (§13.1) |
| Daemon-Guard | Elite | **144** | TBD | 14 | Cinético | Elétrico | ScriptedBrain (placeholder; UtilityBrain = jogo posterior) | 1 no VS, 2 na regra de design (§13.1) |

Notas:
- **HP do Sentinela-Bit = 33 e Atk = 12, MEDIDOS (não mais provisório)** — aplicado 2026-08-07, item `BAL-STATLINES-APLICAR`, achado da Fase A do estudo de pacing PACING-SIM (`docs/design/mecanicas/analise-pacing-fase-a.md` §4). **HP** desfaz a inflação de +60% de 2026-06-03 (volta ao patamar pré-inflação: 34 histórico → **33** = 0,60×55, o ponto vencedor medido). **Atk** sai do provisório de 2026-06-25 (10) e sobe 2 pontos (**12**): desfazer a inflação de HP sozinha NÃO aprova — com HP 33 e Atk 10 a luta mede "segura demais" (vitória 99,88%, quedas do Gus 0,12%), reprovada pela própria faixa de aprovação do líder; o pacote vencedor exige os DOIS movimentos juntos. Dano no Gus (Def 5) = `max(1, 12-5) = 7`/golpe. Medido no ponto vencedor (ponto 31 da grade): vitória 92,81%, duração média 3,89 rodadas (janela-alvo 3-5 em 96,92% das lutas), Gus caindo em 7,19% das lutas — dentro do teto de 12% fixado pelo líder. Ressalva do próprio estudo: o TTK em GOLPES (não rodadas) continua ~11, não 3-5 — os dois alvos "3 a 5" (rodadas vs golpes) nunca foram a mesma pergunta, mesmo antes da inflação.
- **Statline do Bento "Requiem" Chevalier — HP 55 / Atk 10 / Def 13 / SPD 5, família Cinético** — proposto pelo `lead-game-designer` e **aprovado pelo líder em 2026-08-11** (item `BENTO-STATLINE-COMBATE`). Ele existe porque o Bento é o tanque canônico do grupo e, até aqui, toda a onda de balanceamento (Fases A/A-bis/B/C do PACING-SIM e a spec de Provocar) mediu a defesa do grupo usando a **Jaci como proxy** — o gate G4 da Fase C-bis (o provocador tem de cair MAIS provocando do que numa luta normal) empatava 0%×0% porque a Jaci-proxy não caía em nenhum dos dois cenários. Número a número:
  - **Def 13 é a maior da party, por desenho.** É o papel dele; fica **1 ponto abaixo da Def 14 do Daemon-Guard (Elite)** — "armadura de elite em corpo de aliado" —, sem empatar nem ultrapassar o inimigo de referência.
  - **Atk 10 é contido de propósito** (1 acima da Jaci, 4 abaixo do Cauã): o Bento é parede, não DPS. ⚠️ **Consequência registrada, porque medida e não estimada:** contra a Def 8 do Sentinela-Bit o golpe básico dele sai `max(1, 10-8) = 2`, o **dobro** do `max(1, 9-8) = 1` da Jaci-proxy — o terceiro membro passa a contribuir 2 de dano por golpe onde as fases já medidas contribuíam 1, então os pontos que trocam o ocupante do slot não são comparáveis golpe-a-golpe com a série histórica. Contra a Def 14 do Daemon-Guard os dois empatam no piso (`max(1, ·) = 1`). Preservar exatamente o raw 1 contra Def 8 exigiria Atk ≤ 9; o valor aprovado é 10, e este parágrafo existe para que a diferença não passe por acaso.
  - **SPD 5 é o mais lento da party** (canon: cálculo lento, armadura pesada). Fica abaixo do Sentinela-Bit (SPD 8, `//SIM`), então o Bento fica pronto depois do trash no relógio de ação (menor velocidade efetiva, `combat.md §4`).
  - **HP 55 é par com os outros companheiros, não maior** — decisão explícita do líder de **não** dar um HP muito maior (tipo 89): a resistência do Bento vem da **Def**, não de pool extra. HP inflado anularia o preço da ação **Provocar** (`spec-provocar-soft-enrage-criterio-cap.md` §2), porque ele nunca cairia sob pressão. Com Def 13 o preço aparece de fato: provocado, a Def efetiva cai a `floor(13 × 0,5) = 6` (`provoke::provoked_def_from_base`) e o golpe do Sentinela-Bit passa de `max(1, 12-13) = 1` para `max(1, 12-6) = 6` por golpe.
- Atk do Daemon-Guard e SPD dos inimigos (Trash e Elite) seguem TBD (definir na implementação / playtest); fora do escopo desta revisão.
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
3. **Efeitos no relógio de ação / SPD** (deltas ao instante-de-próxima-ação via Gambito-Reordenar, Knockback e Haste/Slow, `combat.md §4`).
4. **Interação com hardware** (triângulo Pillar 3: Óculos/Scan, Matriz Ortodôntica, Tavus-Drive).

Princípios não negociáveis desta camada:

- **Dados no engine POCO**: record `EnvironmentModifier` (mantém o engine puro, §16). Aplicação estática por arena no slice; cartas/inimigos que invocam ambiente = fase posterior (os dados já são preparados para isso).
- **Telegraph obrigatório (Pillar 4)**: toda mudança de clima ou período é avisada N turnos antes, com ícone persistente; Scan revela o número exato de turnos restantes. Nunca RNG punitivo.
- **Cap anti-Scan (Pillar 3)**: degradações de custo de Scan **não empilham além de -2 AP total** num mesmo encontro. A curadoria (§18.6) impede 2 ambientes anti-Scan fortes simultâneos. Hardware nunca vira inútil.
- **Teto cognitivo**: no máximo ~12 terrenos visíveis. O tier **Codex** (efeitos sutis) só **ativa/revela após Scan-ambiente** (novo verb: Scan aplicado ao campo, custo 1 AP).
- **Separação da fórmula**: `multAmbiente` é fator próprio na cadeia divisiva (§11), nunca altera `multFraqueza`.
- **Mutabilidade = interações curadas**: tabela fechada determinística (§18.6). Sem RNG nas transições.

### 18.2 Camada CLIMA (8, transitória)

Muda a cada N turnos, sempre telegrafada. Cada clima favorece uma família e prejudica outra, facilita um status e pode mexer no hardware/relógio de ação.

| Clima | Família ↑ | Família ↓ | Status facilitado | Hardware / relógio |
|---|---|---|---|---|
| **Neblina** | Sônico ×1.3 | Criptográfico ×0.66 | Disrupt +1 mag | Scan +1 AP; Prever só 1 turno à frente |
| **Chuva** | Elétrico ×1.5 | Bioquímico ×0.66 | Stun +1 dur | — |
| **Calor** | Bioquímico ×1.3 | Elétrico ×0.66 | Corrode +1 dur | overheat: 3+ cartas seguidas do mesmo ator → auto-Disrupt nele |
| **Tempestade Elétrica** | Elétrico ×1.5 | Criptográfico ×0.66 | Stun +1 dur | Scan +1 AP mas Prever +1 turno; raio a cada 3 turnos aplica Slow -1 ao ator mais lento |
| **Vento** | Sônico ×1.3 | Bioquímico ×0.66 | Knockback +1 mag | Prever +1; Object Cinético ×0.85; party SPD +1 |
| **Estática / Interferência** | Criptográfico ×1.5 | Sônico ×0.66 | Decrypt / Expose / Silence +1 dur | Scan -1 dado de info; Null custa 0 AP |
| **Fumaça / Cinzas** | Bioquímico ×1.3 | Criptográfico ×0.66 | Poison / Corrode +1 dur; DoT +1 tick | Scan +1 AP; Prever -1 alcance |
| **Escuridão Total** | Sônico ×1.5 | Criptográfico ×0.85 | Disrupt +1 dur | Scan +2 AP (única fonte de info); Iara vê no escuro; inimigos sem visão Slow -1 |

### 18.3 Camada PERÍODO (roda de 4 fases)

Ciclo temporal automático: **Dia (5) → Crepúsculo (2) → Noite (5) → Aurora (2) → Dia**, em turnos. As fases curtas (Crepúsculo/Aurora) funcionam como telegraph de graça da transição que se aproxima.

> **Granularidade e telegraph (canon 2026-06-03, F2-E.11).** "Turnos" do ciclo = **rodadas completas de fila** (1 tique quando todos os atores agiram uma vez; o dia passa no mesmo ritmo independente do tamanho da party, casa com `RoundIndex`/ramp de mana). **Telegraph = 2 turnos** (as próprias fases Crepúsculo/Aurora de 2 turnos são a janela de aviso da fase forte seguinte). Parametrizável (`EnvironmentClock`).
>
> ⚠️ **Lacuna de spec aberta (achado da auditoria de B12, mesma classe do T1/T8 em §18.4/§18.5).** A definição acima amarra "turno do ciclo" a uma rodada completa da fila por SPD, que não existe mais no relógio de ação (`ADR-017`, §4): não há "todos os atores agiram uma vez" como fronteira, só o instante corrente do relógio e a contagem própria de turnos por ator. Sem uma definição canônica de "rodada" no relógio (mesma pendência do T1/T8), este documento não pode dizer quantos instantes do relógio equivalem a um "turno do ciclo" de clima/período. A intenção de design sobrevive (o ciclo avança em passos regulares, telegrafados 2 passos antes); a unidade exata de avanço é decisão do líder.

| Fase | Família ↑ | Família ↓ | Status | Hardware / relógio | Duração |
|---|---|---|---|---|---|
| **Dia** | Bioquímico ×1.3 | Criptográfico ×0.85 | Regen +1 dur (party) | Scan grátis | 5 |
| **Crepúsculo** | (transição, neutro) | — | Disrupt +1 mag | Scan revela +1 dado | 2 |
| **Noite** | Criptográfico ×1.5 + Sônico ×1.3 | Bioquímico ×0.85 | Decrypt / Expose +1 dur | Scan +1 AP; Iara: 1ª carta Cripto por turno ignora telegraph; inimigos diurnos Slow -1 | 5 |
| **Aurora** | Elétrico ×1.3 | — | Haste +1 dur (party) | Scan normaliza; Prever +1; party SPD +1 | 2 |

- **Selve Sombria**: ciclo natural automático (dia/noite reais).
- **Cidade ciber-gótica**: o eixo vira "ciclo da grade elétrica" (Surto ↔ Blackout) usando o **mesmo motor**, só muda o sabor (Pillar 5: contraste deliberado de setting com sistema unificado por baixo).
- Crepúsculo e Aurora curtos = janelas-ponte que telegrafam a próxima fase forte sem custo de Scan.

### 18.4 Camada TERRENO — Tier VISÍVEL (12, fixo por encontro)

Lido a olho nu (sem Scan), até ~12 visíveis simultâneos (teto cognitivo). Cada um declara família ↑ / ↓, status facilitado e efeitos no relógio/hardware.

**Existentes (7):**

- **Lamacento**: Elétrico ×1.3 / Cinético ×0.66 (deslocamento dificultado); Knockback no alvo → mag 2 (em vez do padrão mag 1); Slow -2 todos (-3 para Cinético).
- **Seco**: Cinético ×1.3 / Elétrico ×0.66; DoT (Poison/Corrode) -1 dur; Break +1 dur.
- **Vinhas**: Bioquímico ×1.3 (Object +1 dur) / Cinético ×0.85; aplica Root (Slow de magnitude extrema); Knockback anulado em alvo enraizado; Scan revela crescimento em sequência recorrente.
- **Gelo**: Cinético ×1.3 / Bioquímico ×0.66; Break +1 dur; Slow -1; SPD -1 todos; Tavus-Drive +1 mana na 1ª carta do turno.
- **Água / Alagado**: Elétrico ×1.3 / Cinético ×0.85; Stun +1 dur; SPD -1 todos; alvo dentro da água que leva dano Elétrico → Disrupt grátis.
- **Metal-Condutor**: Elétrico ×1.3 / Sônico ×0.66; Corrode +1 dur; Knockback ricocheteia com +1 mag adicional; Scan grátis + revela 1 dado extra (Matriz Ortodôntica amplificada).
- **Bioluminescência (SÓ SELVE)**: Sônico ×1.3 / Elétrico ×0.85; Regen / Haste +1 dur; Scan grátis + Prever +1; pulso luminoso em sequência 1-1-2-3-5.

**Novos (5):**

- **T1 Pavimento Tesselado** (cidade forte; variante na Selve): Criptográfico ×1.3 / Sônico ×0.66. **DUAL, alternando por rodada**: rodada **ÍMPAR = Branco (lapidado)** → +1 dado de Scan grátis a quem scaneia; rodada **PAR = Preto (bruto)** → a 1ª carta Cripto da rodada aplica Expose magnitude 13 grátis no alvo. Telegraph: o quadriculado pulsa branco/preto conforme a rodada. ⚠️ **Lacuna de spec aberta (parecer do CTO, `PARECER-ADR-017.md` §3; L-11, decisão pendente do líder; achado desta reconciliação 25/08/2026).** O efeito original sincronizava a alternância a `turnoIndex`, um contador GLOBAL de rodada. O relógio de ação (`ADR-017`, §4) não tem essa peça: só existem o instante corrente (que avança em quantidades variáveis por ação resolvida, não 1 por rodada) e a contagem própria de turnos POR ATOR. Nenhuma das seções já reconciliadas deste canon (§3/§4/§4.1) define "rodada" como fronteira global; o `PARECER-ADR-017.md` §5 levanta "rodada = janela de N ticks" como candidato, mas isso não foi adotado como canon, é sugestão de auditoria. Sem uma definição de rodada, este documento não pode dizer O QUE alterna entre Branco/Preto. A intenção de design (o piso pulsa e alterna, premiando ora Scan ora Cripto-ofensivo) fica registrada; a mecânica de gatilho exata é decisão do líder, mesma onda da lacuna de ativação (§3).
- **T2 Talude Instável**: Cinético ×1.5 / Criptográfico ×0.66. Pune o INATIVO: ator que não agiu ofensivamente na rodada anterior entra com Slow magnitude 2; ator que gastou ≥2 AP ofensivo na rodada anterior fica imune. Premia agressão, pune turtle. Telegraph: o chão racha sob o ator inativo 1 turno antes. Mexe o relógio de ação por comportamento (via Slow, `combat.md §4`).
- **T4 Ashlar Bruto**: Cinético ×1.3 / Elétrico ×0.66. Usar **Defender** aqui → pool de Shield ×1.5 (Magnitude = Def × 1.5). Premia turtle (espelho do T2). Não dá Shield de graça (exige gastar AP em Defender). Transição de **PROGRESSÃO** (entre encontros): arena vencida "lapida" o Ashlar Bruto → vira **T1 Pavimento Tesselado** (bruto → polido).
- **T5 Solo Fértil Recursivo (SÓ SELVE)**: Bioquímico ×1.5 / Cinético ×0.66. Entidades Object plantadas aqui ganham +1 Duration e escalam o efeito por rodada na sequência 1,1,2,3,5 (ex: um totem de Poison aplica magnitude 1,1,2,3,5 ao longo das rodadas). Facilita Poison / Regen / Root. Scan revela o estágio do L-system.
- **T6 Anomalia Perlin** (Selve profunda + cidade no ato 3): NENHUMA família ↑; Criptográfico ×0.66. Degrada hardware: Gambito-Prever sempre retorna ruído (`IsChaotic` global) + Scan retorna perfil parcial (revela HP, mas exige 2 scans para a fraqueza). **NÃO mexe no dano** (`multAmbiente` geral = 1.0). Telegraph forte: glitch visual no canal-4 (Perlin quebrado, gradiente cyan → vermelho a 21 lúmens). Vetor anti-padrão Patch-Zero canon.

### 18.5 Camada TERRENO — Tier CODEX (3, efeito ativa/revela só após Scan-ambiente 1 AP)

Efeitos sutis, deliberadamente fora do teto cognitivo "a olho nu". Só ativam e se revelam após o verb **Scan-ambiente** (1 AP) ser usado no campo.

- **T3 Espelho Ressonante**: Sônico ×1.5 / Bioquímico ×0.66. Cartas com `TargetShape` Grupo/Linha ricocheteiam um 2º tick a 0.5× no alvo de maior HP. **SIMÉTRICO**: AoE inimiga também ricocheteia neles (premia ler intent via Gambito antes de aglomerar a party). Facilita Disrupt.
- **T7 Duto Condutor Pressurizado (SÓ CIDADE)**: Elétrico ×1.3 / Bioquímico ×0.66. Se um ator usa Elétrico E outro usa Sônico na MESMA rodada → "Ressonância de Duto": +1 Disrupt em todos os inimigos (materializa a sinergia canon Cauã + Linda). Facilita Disrupt / Stun.
- **T8 Elevação Dominante**: Cinético ×1.3 / Sônico ×0.66. O 1º ator da party a agir na rodada ganha Haste magnitude 1 até o fim da rodada + revela o intent de +1 inimigo grátis. Premia SPD / Gambito-reordenar. ⚠️ **Lacuna de spec aberta (parecer do CTO, `PARECER-ADR-017.md` §3; L-11, decisão pendente do líder; achado desta reconciliação 25/08/2026).** O efeito original era definido por **posição-de-fila** ("topo da fila" = 1º da party a agir na rodada). O relógio de ação (§4) não tem fila nem posição: cada ator compete pelo próprio `next_action_at`. É a MESMA lacuna do T1 (acima): sem uma definição canônica de "rodada" no relógio, não há como dizer quando o bônus reaplica (uma vez por combate? a cada janela de N ticks? ao 1º ator da party que fica pronto depois de cada evento?). A intenção de design sobrevive (recompensar o ator mais rápido a agir), a regra de gatilho e de janela exata não é decidida aqui.

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

- Record **`EnvironmentModifier`** no engine POCO: família-mults, status facilitado, deltas no relógio de ação/SPD, hooks de hardware, tipo de camada (terreno/clima/período), tier (visível/codex).
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
2. **Estado de HOLD na abertura** (battle-screen §5.2 D10, agora com espera de input): a arena monta PARADA, exibe **"BATALHA!"** + a faixa dos próximos a agir no relógio de ação (§4, reconciliado 25/08/2026; ver `battle-screen.md` D4); NINGUÉM agiu ainda. A tela espera o jogador:
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

## 20. Origem do combate: encontros aleatórios (cross-ref)

**Doc canônico próprio:** [`encontros-aleatorios.md`](encontros-aleatorios.md) (item `ENCONTRO-ALEATORIO-SISTEMA`, 4 decisões do líder via AskUserQuestion em 2026-08-12).

Este doc (`combat.md`) define o que acontece DENTRO da batalha; o de encontros define QUANDO uma batalha genérica nasce no overworld. Resumo do contrato, detalhes lá:

- **Rampa com graça:** 8 tiles de graça a chance zero após qualquer desfecho de batalha (inclusive auto-kill do Ouro, §19.2), entrada em área e load; depois a chance por tile cruzado cresce linear de 1% (tile 9) a 8% (tile 42, rampa de 34 tiles) e trava no platô de 8% (nunca há encontro forçado). Forma inteira exata de base 6600 para implementação e testes.
- **Multiplicadores:** `multDificuldade` (slot reservado, tabela de dado do `ENCONTRO-FREQ-DIFICULDADE`, Médio = 1,0) × `multFaraday` (0,5× com a carta Gaiola de Faraday ativa; reduz, não suprime; PEM segue afetando só o autosave).
- **Composição:** 1 inimigo por encontro nesta onda (`group_size` existe no schema como dado para calibração futura; o motor de combate já aceita 1 a 4, §2).
- **Área é dado:** `EncounterProfile` por `AreaDescriptor` (ADR-020); cidade é 100% segura por ausência de perfil; o sistema estreia jogável na 1ª dungeon.
- **Costura com o combate:** o spawn-path `EnemyTemplate → CombatActor` do director é o primeiro call site de produção do `difficulty_multiplier_for` (`DIFICULDADE-TABELA-DADO`), e o desfecho do encontro segue o eixo de domínio da §19 (auto-kill / auto-resolve / encarar).

---

**Última revisão:** 2026-08-28 (item `B12`, auditoria-dominó dos resíduos de vocabulário CTB deixados fora do escopo da reconciliação de 25/08: §1 tabela de verbs, "Gambito: lê e manipula a fila de turnos" vira "lê o intent e ajusta o relógio de ação"; §6 identidade do Cinético, "deslocamento de fila" vira "atraso no relógio de ação"; §12 Scan/Gambito, três frases que ainda citavam `ReorderActor`/"fila" reescritas com a linguagem já fixada em §4 (soma ticks ao `next_action_at`); §17 item 11 do escopo do slice, "Reordenar (2 AP, chama `ReorderActor`)" alinhado ao item 2 (que já estava correto); §17 nota do Bento, "ordem bruta de iniciativa" vira "relógio de ação (menor velocidade efetiva)"; §18.1 item 3, cabeçalhos "Hardware / fila" das três tabelas (Clima/Período/Terreno) e a frase de abertura do Terreno Visível renomeados pra "relógio"; §18.2 Tempestade Elétrica e Vento, "ao ator mais lento da fila" e "Knockback +1 fila" corrigidos ("+1 mag", mesmo sufixo já usado por Disrupt/Corrode/Stun na mesma tabela); §18.4 Lamacento e Metal-Condutor, os deltas numéricos de Knockback ("+2 na fila", "+1 fila adicional") traduzidos pra Magnitude ("mag 2", "+1 mag adicional") reaproveitando o mesmo campo que Haste/Slow já usam (§9), sem inventar unidade nova; §18.4 T2 Talude, "mexe a fila" vira "mexe o relógio"; §18.10 escopo, "deltas de fila/SPD" vira "deltas no relógio de ação/SPD". **Achado além da lista (não decidido, só sinalizado):** §18.3 Período (granularidade e telegraph, canon 2026-06-03) definia "turno do ciclo" como "rodada completa de fila", a mesma lacuna estrutural do T1/T8 (§18.4/§18.5) — marcado com a mesma tag "Lacuna de spec aberta", decisão de unidade fica com o líder. `battle-screen.md`: status header (linha 3) e decisão macro #2 (linha 16) com "fila"/"CTB"; mockup ASCII "FILA CTB:" vira "RELOGIO:"; bullet do cockpit (o que aparece "na abertura") corrigido pro modelo de HOLD (D10/D13, não mais "1º da fila por SPD"); bullet da faixa de iniciativa CTB (zona da tela, §2) alinhado ao D4 já reconciliado; fallback de mira sem Scan (§3.5) trocado de "mais à frente na fila" pra "mais próximo de agir no relógio"; escopo do M5 (§4) e nota de cast-time (§4) com "fila CTB"/"marcador na fila"; **D10** (§5.2), gêmeo exato do §19.4 já corrigido em `combat.md`, reescrito na mesma linguagem (faixa do relógio, sem "se o inimigo for o 1º por maior SPD"). Nenhum número de balanceamento foi alterado além da tradução 1:1 posição-de-fila→Magnitude de Knockback descrita acima; nenhuma seção já reconciliada em 25/08 (§2/§3/§4/§4.1/§9/§15.1/§16/§19.4) foi tocada.) Revisão anterior 2026-08-25 (fechamento das sobras de linguagem de fila, item de trabalho posterior à reconciliação principal com o `ADR-017`: §9 status framework, Knockback e Haste/Slow reescritos pro relógio de ação com nota SUPERADO citando o texto antigo verbatim; §15.1 remove a citação ao "modelo 1B" da conversão informal de duração; §16 `TurnStarted` troca `turnoIndex` por `contagemPropriaDeTurnos`; §17 itens 2 e 4 do escopo do vertical slice reescritos pro relógio, com nota de L-01 no topo da seção; §18 T1 Pavimento Tesselado e T8 Elevação Dominante marcados como lacuna de spec aberta — dependiam de `turnoIndex`/posição-de-fila que não têm equivalente no relógio, e a definição de "rodada" no relógio não é canon, decisão fica com o líder; §19.4 troca "a fila CTB" pela faixa do relógio, título "Opção 1B" preservado por ser rótulo de outra decisão, sem relação com o modelo de turno. Nenhum número de balanceamento foi alterado; nenhuma seção já reconciliada (§2/§3/§4/§4.1) foi tocada.) Revisão anterior 2026-08-12 (§20 nova, cross-ref do sistema de encontros aleatórios, item `ENCONTRO-ALEATORIO-SISTEMA`: doc canônico próprio `encontros-aleatorios.md` com as 4 decisões do líder de 2026-08-12: rampa 8/34/1%→8% com forma inteira base 6600, cidade segura por ausência de dado com estreia na 1ª dungeon, Faraday 0,5× multiplicativo sem supressão, 1 inimigo por encontro com `group_size` no schema. Nenhuma seção de regra de combate foi alterada.) Revisão anterior 2026-08-11 (§17 Stats de referência do encontro, item `BENTO-STATLINE-COMBATE`: entra o statline do **Bento "Requiem" Chevalier — HP 55 / Atk 10 / Def 13 / SPD 5, Cinético**, proposto pelo `lead-game-designer` e aprovado pelo líder. Def 13 = a maior da party, 1 abaixo da Def 14 do Daemon-Guard; Atk 10 contido (parede, não DPS — com a consequência medida de sair `raw 2` contra Def 8, o dobro do `raw 1` da Jaci-proxy, registrada na nota); SPD 5 = o mais lento; HP 55 par com os companheiros, POR DECISÃO — HP inflado anularia o preço da ação Provocar. Nenhum outro statline foi tocado. Consumido pelo harness da Fase C-bis do estudo de pacing, que passa a rodar os cenários de parede/provocação com o Bento real em vez da Jaci-proxy.) Revisão anterior 2026-08-07 (§17 Stats de referência do encontro, item `BAL-STATLINES-APLICAR`: Sentinela-Bit/Trash HP 55→33 e Atk 10-provisório→12-medido, aplicando o "ponto vencedor" da Fase A do estudo de pacing PACING-SIM, `docs/design/mecanicas/analise-pacing-fase-a.md` §4 — HP desfaz a inflação de +60% de 2026-06-03 e volta ao patamar pré-inflação (34→33, 0,60×55); Atk sai do provisório de 2026-06-25 e sobe 2 pontos, porque HP sozinho reprova a faixa do líder (luta "segura demais"); medido: vitória 92,81%, duração média 3,89 rodadas, janela 3-5 em 96,92%, quedas do Gus 7,19%. Elite (Daemon-Guard) intocado, congelado à parte em `ELITE-MEIO-TERMO`. Mira do trash (`MIRA-PONDERADA-PROD`) também intocada, sequenciada depois por decisão de processo — ver TODO.md.) Revisão anterior 2026-06-25 (§4.1 Janela de Comando da Party, modelo 1B: comando livre da party dentro do bloco com SPD decidindo qual lado abre a rodada; decisão D1/D2 do criador via AskUserQuestion; criador aceitou o contra-argumento de preservar o SPD como stat; FSM e §11 não mudam, extensão aditiva `PendingPartyActors`; sistemas fila/Gambito/AP/Haste-Slow/ambientes-SPD/cast-time/auto-resolve/ITEM-SPD todos preservados; bônus de iniciativa pendente de playtest D5; target selection entra já, comando-livre-sobre-o-CTB logo após D6). Revisão anterior nesta data: §19 Resolver sem encarar + auto-kill, eixo de domínio. Nova seção canoniza a mecânica opt-in de pular combate de Trash (Opção 1B + abertura-espera-input), o cálculo via FSM headless com `AutoResolveBrain` sub-ótima (Opção 2C), a tematização `-O0`/`-O2` no terminal (D-C), o toggle de HUD de 3 estados (Opção 4B), escopo só-Trash (Opção 5A), rótulo de risco baixo/médio/alto com aviso de Hospital no alto (Opção 6A), o gate de onboarding em Bronze, e o princípio "pular nunca é a jogada ótima contra trash não-dominado". Parâmetros econômicos (x/y por selo, faixas de P-derrota) em `economia.md` (economy-designer, paralelo). Revisão anterior 2026-06-22 (M5-DMG). §11 evoluída: sorteio único de canal FALHA/CRIT/COMUM sobre a variância Knowledge preservada; FALHA decai com a maestria (`round(5 × e^(-kills × 0.50))`, 0% a partir de 5 kills); CRIT com piso global 5% elevável por `card.CritChance`, dano = `round(danoBase × (1+v) × 1.5)`; imunidade `multFraqueza==0` em curto-circuito antes do RNG; ordem de consumo do RNG documentada (1 sorteio de canal, +1 só no COMUM); RNG via porta `IRandomSource` (ADR-006, domínio puro); §7 `CritChance` redefinido como piso por carta. A §11 deixa de ser paridade com o C# (que morre no M8); o motor C++ segue esta §11. Revisão anterior 2026-06-03 (D.1+N.2 Sprint 1 W2): HP +60% Trash 34→55 / Elite 89→144; AP e Mana por-ator CTB (§5); §2.1 contrato fragilidade Gus (N.2 R1, one-way door); §10 feedback ERRO DE COMPILAÇÃO (N.2 R2); §12/§13/§15 caos Perlin exclusivo Patch-Zero (N.2 R3, one-way door); §17 stats de referência pós-inflação + roda de fraqueza confirmada. Revisão 2026-05-26: §18 ambientes de combate, §11 multAmbiente + stacking 3 camadas, escopo slice item 15. Status: canônico, pronto para implementação TDD F2-E.5.

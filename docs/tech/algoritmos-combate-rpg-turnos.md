<!--
SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
SPDX-License-Identifier: AGPL-3.0-or-later
-->

> ⚠️ **DOCUMENTO HISTÓRICO, resgatado em 25/08/2026 do `gusworld_legacy` por decisão do líder via `AskUserQuestion`.**
>
> **Origem e autoria:** pesquisa consolidada em **11/08/2026**, no projeto anterior. **Foi o próprio líder quem trouxe o documento inteiro**, não um agente — o corpo abaixo é cópia íntegra do original (4.478 linhas), sem corte, resumo, reordenação ou correção de uma linha sequer, inclusive onde algo aqui diverge do canon vigente ou precisa de reconferência.
>
> **NÃO foi reconferido contra as leis vigentes deste projeto.** É pesquisa e insumo trazido para consulta, não spec ratificada nem trabalho em andamento. Antes de qualquer decisão de engine sobre IA de combate, o texto precisa passar pelo crivo de `GODS_LAWS.md` (entre outras: L-17, espinha de cinco camadas com `aplica(estado, comando)` determinístico; L-25, nenhuma primitiva de criptografia ou aleatoriedade escrita em casa — vem do GlintFx; L-19, TDD estrito e os cinco portões de qualidade) e do canon vivo de `docs/design/mecanicas/combat.md`.
>
> **O que já não existe (LEI ZERO, L-01):** o texto cita, ao longo de várias seções, arquivos e módulos de código do projeto anterior — caminhos de classe, pseudocódigo em C++/C#, e documentos internos dele como `analise-mira-resultados.md` e `analise-pacing-fase-c-20260811.md`. **Nenhum desses arquivos existe neste projeto.** O GusWorld está sendo escrito do zero, sempre assentado sobre o GlintFx: código anterior não é base de reaproveitamento. Essas referências ficam como estão no corpo do documento — descrevem a intenção de quem pesquisou, não um arquivo consultável aqui — e não foram consertadas, reescritas nem apagadas.
>
> **Sobre a restrição de topo do próprio documento** ("Gus é um ator crítico: quando Gus é derrotado, a party perde a batalha"): ela **bate, no essencial, com o canon vigente**. `docs/design/mecanicas/combat.md` §2.1 já trava o HP de Gus como o menor da party e reserva a Análise Preditiva como colchão de uma queda fatal por batalha; `docs/design/pillars.md` ("Game over — 4 modos escalonados por dificuldade") confirma que é o HP de Gus chegando a zero, e não o de qualquer companion, que dispara o fim de jogo. A única divergência é de granularidade: o documento fala em "a party perde a batalha" como desfecho único, enquanto o canon de hoje escalona a consequência em quatro modos por dificuldade (`docs/design/mecanicas/modos-morte.md`, ainda em status de proposta aguardando canonização do líder segundo o próprio arquivo, embora `pillars.md` já o trate como canônico) — reload, whiteout no Hospital, respawn deslocado ou a chance única de permadeath do Hardcore, não um "perde a batalha" uniforme.

---
# Algoritmos de combate para RPG por turnos híbrido
## IA, atributos, itens, targeting, anti-stall, balanceamento, chefes e validação estatística

**Versão:** 1.0 — pesquisa consolidada em 11/08/2026  
**Escopo-alvo:** RPG híbrido inspirado em *Chrono Trigger*, *Pokémon* e JRPGs/táticos modernos; party de **3–5** personagens; **1–6** inimigos; inimigos, aliados automáticos, IA×IA/autobattle e princípios aplicáveis a PvP.  
**Contexto de integração:** `petrinhu/GusWorld` + `petrinhu/glintfx`, mantendo os algoritmos **agnósticos de engine**.  
**Restrição crítica:** **Gus é um ator crítico**: quando Gus é derrotado, a party perde a batalha. Isso altera profundamente a teoria de targeting e deve ser tratado explicitamente, e não como um detalhe de statline.

---

# Sumário

1. [Conclusão executiva](#1-conclusão-executiva)
2. [O que já existe no GusWorld](#2-o-que-já-existe-no-gusworld)
3. [Objetivos formais do sistema](#3-objetivos-formais-do-sistema)
4. [Arquitetura recomendada](#4-arquitetura-recomendada)
5. [Relógio híbrido de combate](#5-relógio-híbrido-de-combate)
6. [Modelo de dados de combate](#6-modelo-de-dados-de-combate)
7. [Arquitetura de atributos](#7-arquitetura-de-atributos)
8. [Fórmulas de dano, cura, acerto, crítico e status](#8-fórmulas-de-dano-cura-acerto-crítico-e-status)
9. [Economia de ações](#9-economia-de-ações)
10. [Utility AI como núcleo](#10-utility-ai-como-núcleo)
11. [Escolha conjunta ação + alvo](#11-escolha-conjunta-ação--alvo)
12. [Targeting: foco tático vs. tunnel vision](#12-targeting-foco-tático-vs-tunnel-vision)
13. [Coordenador de pressão ofensiva](#13-coordenador-de-pressão-ofensiva)
14. [Gus como ator crítico](#14-gus-como-ator-crítico)
15. [Conhecimento e inteligência do NPC](#15-conhecimento-e-inteligência-do-npc)
16. [Easy / Normal / Hard](#16-easy--normal--hard)
17. [IA de aliados](#17-ia-de-aliados)
18. [IA × IA e autobattle](#18-ia--ia-e-autobattle)
19. [Comparação dos algoritmos de IA](#19-comparação-dos-algoritmos-de-ia)
20. [Itens e inventário real](#20-itens-e-inventário-real)
21. [Cura e prevenção de luta infinita](#21-cura-e-prevenção-de-luta-infinita)
22. [Detector geral de stall](#22-detector-geral-de-stall)
23. [Anti-turtle](#23-anti-turtle)
24. [Stun-lock e controle de grupo](#24-stun-lock-e-controle-de-grupo)
25. [Revive loops](#25-revive-loops)
26. [Reflect loops, proc loops e recursão de efeitos](#26-reflect-loops-proc-loops-e-recursão-de-efeitos)
27. [Evasão e precisão degeneradas](#27-evasão-e-precisão-degeneradas)
28. [One-shot e burst excessivo](#28-one-shot-e-burst-excessivo)
29. [Domínio de velocidade](#29-domínio-de-velocidade)
30. [Buff/debuff stacking](#30-buffdebuff-stacking)
31. [Fraquezas, resistências e imunidades](#31-fraquezas-resistências-e-imunidades)
32. [Aleatoriedade e legibilidade](#32-aleatoriedade-e-legibilidade)
33. [Telegraph e intenção](#33-telegraph-e-intenção)
34. [Chefes](#34-chefes)
35. [Duração-alvo das lutas](#35-duração-alvo-das-lutas)
36. [Escalonamento para 3–5 × 1–6](#36-escalonamento-para-35--16)
37. [PvP](#37-pvp)
38. [DDA e adaptação](#38-dda-e-adaptação)
39. [Simulação e harness](#39-simulação-e-harness)
40. [Métricas de balanceamento](#40-métricas-de-balanceamento)
41. [Estatística e critérios de aprovação](#41-estatística-e-critérios-de-aprovação)
42. [Personas sintéticas de jogador](#42-personas-sintéticas-de-jogador)
43. [Casos adversariais obrigatórios](#43-casos-adversariais-obrigatórios)
44. [Instrumentação e explicabilidade](#44-instrumentação-e-explicabilidade)
45. [Configuração data-driven](#45-configuração-data-driven)
46. [Pseudocódigo do sistema completo](#46-pseudocódigo-do-sistema-completo)
47. [Integração sugerida ao GusWorld](#47-integração-sugerida-ao-gusworld)
48. [Plano incremental](#48-plano-incremental)
49. [Decisões que devem continuar em playtest](#49-decisões-que-devem-continuar-em-playtest)
50. [Referências](#50-referências)
51. [Apêndice A — tabela de falhas e contramedidas](#apêndice-a--tabela-de-falhas-e-contramedidas)
52. [Apêndice B — conjunto inicial de parâmetros](#apêndice-b--conjunto-inicial-de-parâmetros)
53. [Apêndice C — testes de propriedade](#apêndice-c--testes-de-propriedade)
54. [Apêndice D — glossário](#apêndice-d--glossário)
55. [Apêndice E — mapa comparativo de padrões de jogos](#apêndice-e--mapa-comparativo-de-padrões-de-jogos)
56. [Apêndice F — falhas de implementação e invariantes adicionais](#apêndice-f--falhas-de-implementação-e-invariantes-adicionais)

---

# 1. Conclusão executiva

A melhor solução para este jogo **não é um algoritmo único**. O desenho mais robusto é uma arquitetura em camadas:

```text
BattleRules / invariantes de segurança
        ↓
EncounterDirector / coordenação coletiva
        ↓
KnowledgeModel / o que cada NPC sabe
        ↓
Brain (Scripted | Utility | Utility+Lookahead | Search)
        ↓
CandidateGenerator
        ↓
ActionTargetEvaluator
        ↓
TargetPressureCoordinator
        ↓
StochasticSelector
        ↓
IntentPreview / Telegraph
        ↓
ActionClock / execução
        ↓
EffectEngine + LoopGuards
        ↓
CombatMetrics / BattleLog / Harness
```

A recomendação central é:

- **Utility AI** como política padrão para elites, aliados automáticos e a maior parte dos inimigos inteligentes.
- **Script/FSM** como camada externa de identidade, tutorial, fases de boss e sequências narrativas.
- **Behavior Tree** principalmente para execução, reatividade e organização hierárquica, não como única função de decisão tática.
- **Expectimax raso** ou rollout curto para Hard, chefes e ações com consequência futura clara.
- **MCTS/ISMCTS** como ferramenta seletiva, especialmente offline, autobattle avançado, análise de balanceamento ou poucos inimigos excepcionais; não como cérebro universal.
- **RL/dynamic scripting** como camada experimental ou adaptativa, nunca como fundamento invisível da IA central sem limites de autor.
- **Targeting ponderado com memória + orçamento de pressão por alvo**, em vez de “maior score sempre” ou sorteio puro.
- **Foco tático permitido quando existe motivo objetivo**, mas com penalidade de repetição/pressão quando o ganho marginal de continuar focando não justifica o dogpile.
- **Defesa deve dar agência ao jogador de forma mecânica**, por taunt/guard-redirect/intercept/firewall/cover, e não depender apenas de o inimigo voluntariamente atacar o tanque.
- **Itens de cura devem ser finitos e ter custo de oportunidade real**. Cooldown, cargas, bateria, custo temporal e custo de inventário são preferíveis a um simples “proibir cura após N usos”.
- **Um StallGuard geral** deve existir como invariável de segurança, mesmo que as regras normais tornem sua ativação rara.
- **Loops de eventos devem ser impossíveis por construção**, com provenance, profundidade de reflexão e orçamento de triggers.
- **Dificuldade deve mudar qualidade de decisão e conhecimento**, não apenas HP/Atk.
- **Inteligência do NPC e dificuldade escolhida pelo jogador devem ser eixos separados**.
- **Gus não pode ser tratado como alvo comum**. Se todos os NPCs souberem que matá-lo encerra o combate, a estratégia dominante racional é focá-lo. Portanto, “saber que Gus é condição de derrota” precisa ser informação diegética restrita a certos inimigos.
- **O harness existente do GusWorld deve se tornar o árbitro**. Nenhum número de HP/Atk, targeting, cura ou defesa deve ser canonizado sem simulação multi-persona e teste adversarial.

A literatura de produção apoia essa combinação. *Dragon Age: Inquisition* usou uma arquitetura de utility scoring que avalia ações e seus alvos, incluindo custos de recursos e desperdício de consumíveis [R1]. *Kingdoms of Amalur: Reckoning* usou capacidade centralizada para limitar quantos e quais ataques podem convergir ao jogador [R2]. *Gears Tactics* separou planejamento e execução e explicitamente tratou “clareza tática” como requisito de coordenação de múltiplos inimigos [R3]. A literatura acadêmica fornece MCTS/UCT [R6], ISMCTS para informação oculta [R7], dynamic scripting [R8], DDA [R9–R11] e personas sintéticas para playtesting [R12–R14].

---

# 2. O que já existe no GusWorld

## 2.1 Premissas canônicas relevantes

A documentação atual do repositório já define uma base forte:

- combate em fila/CTB influenciada por `SPD`;
- `AP` por turno;
- mana com rampa linear e sem banking;
- Scan/Gambito como mecanismos de informação;
- fraquezas, status, combos e ambiente;
- RNG injetável e seedável;
- `IntentPreview` no contrato de brain;
- `ScriptedBrain` no slice e `UtilityBrain` previsto;
- Knowledge reduzindo variância/falha;
- harness headless;
- alvo normal de combate comum em **3–5 rodadas**;
- Gus com **menor HP da party**;
- Análise Preditiva de Gus como uma absorção de golpe fatal por batalha.

Fonte interna principal:  
`docs/design/mecanicas/combat.md`

## 2.2 Party atual e arquétipos

Os documentos de personagem estabelecem papéis úteis para o desenho de IA:

| Personagem | Papel mecânico aproximado | Consequência para IA |
|---|---|---|
| **Gus** | Utility / Control / análise | alto valor estratégico, baixo STR/HP; ator crítico |
| **Cauã “Volt”** | Striker | gera ameaça por burst |
| **Iara “Lumen”** | Infiltrator / Decoy | deve manipular percepção e targeting |
| **Bento “Requiem”** | Tank / Area Defender | precisa de proteção determinística ou aggro explícito |
| **Linda “Siren”** | Crowd Control | alta ameaça situacional por controle |
| **Dante “Grid”** | Support / Fortification | utilidade indireta e conhecimento narrativo |
| **Jaci “Proxy”** | Healer / Buffer | alvo de prioridade racional, mas cura precisa de limites |

Fontes internas: `CHARS.md` e `docs/narrative/characters/*.md`.

## 2.3 Statline atual de referência

Em `combat.md`, a referência atual inclui, entre outros:

```text
Gus:   HP 34, Atk 8, Def 5, SPD 9
Cauã:  HP 55, Atk 14, Def 8, SPD 13
Jaci:  HP 55, Atk 9, Def 10, SPD 7
```

O `Sentinela-Bit` foi ajustado para aproximadamente:

```text
HP 33, Atk 12, Def 8
```

A análise recente registrou em determinados cenários aproximadamente 3,89 rodadas médias, ~92,8% de vitória e ~7,2% de queda do Gus, sob o braço específico de mira usado no estudo. Esses números **não devem ser generalizados para toda composição e toda política de targeting**.

## 2.4 O achado interno mais importante: defesa e targeting estão acoplados

O estudo `analise-mira-resultados.md` executou **10.080.000** lutas e encontrou:

1. mira fixa no primeiro ator cria tunnel vision;
2. sorteio por si só espalha dano, mas não produz inteligência legível;
3. Defender/Shield atual pode absorver praticamente tudo em alguns cenários;
4. se a IA aprende a evitar a “parede”, os alvos frágeis recebem muito mais pressão;
5. Gus foi o único a cair em muitos braços do estudo;
6. defesa total pôde produzir empate técnico no desenho anterior.

A Fase C de pacing (`analise-pacing-fase-c-20260811.md`) executou **11,52 milhões** de lutas e encontrou que os quatro candidatos de statline falhavam a um teste de robustez quando a política de mira mudava. A conclusão mais útil é metodológica:

> **Não balancear statline isoladamente de targeting, defesa e action economy.**

Outro achado importante da Fase C: exigir literalmente `0%` de stalls observados numa amostra grande é um critério estatístico ruim. Zero eventos observados não implica probabilidade real zero; ver [R15].

## 2.5 Expansão pedida nesta pesquisa

O jogo atual usa números menores em algumas especificações. Esta proposta amplia o framework para:

- party **3–5**;
- inimigos **1–6**;
- aliados automáticos;
- IA×IA/autobattle;
- perfis Easy/Normal/Hard;
- diferentes graus de conhecimento de NPC;
- cast time e ActionClock;
- itens reais limitados;
- mecanismos globais contra stall e loops.

O motor deve continuar agnóstico: nada deve hardcodar “Gus” no domínio. A regra deve ser expressa por dados como `CriticalActor`, `LossCondition`, `KnowledgeFact`, `ProtectivePassive`, etc.

---

# 3. Objetivos formais do sistema

O combate deve satisfazer simultaneamente:

## 3.1 Correção

- sempre termina;
- nunca entra em recursão infinita de efeitos;
- não consome item duas vezes;
- não executa ator morto/incapacitado;
- não escolhe alvo ilegal;
- RNG é reproduzível por seed;
- preview e resolução usam a mesma função matemática.

## 3.2 Legibilidade

O jogador deve conseguir responder:

- por que este inimigo atacou este alvo?
- por que a cura foi usada?
- por que uma habilidade foi preferida?
- por que um status não funcionou?
- quem agirá a seguir?
- o que um boss está preparando?
- quando uma regra de anti-stall começou?

“IA forte” sem legibilidade pode parecer aleatória ou trapaceira. *Gears Tactics* documenta diretamente que um comportamento objetivamente bom pode parecer ruim se o jogador não percebe a causalidade [R3].

## 3.3 Não degeneração

Nenhuma estratégia simples deve resolver o sistema inteiro:

- atacar sempre;
- defender sempre;
- curar sempre;
- focar sempre o mesmo alvo;
- stun em loop;
- revive em loop;
- reflect em loop;
- empilhar evasão;
- maximizar SPD e impedir o adversário de agir;
- stackar uma única família;
- acumular consumíveis sem custo;
- explorar uma fraqueza sem contrajogo.

## 3.4 Agência

A correção de uma estratégia dominante não deve simplesmente remover a escolha do jogador. É preferível criar **trade-offs**.

Exemplo: contra focus fire, a melhor solução não é “IA é proibida de atacar duas vezes o mesmo alvo”. É fornecer:

- taunt/honeypot;
- guard redirect;
- intercept;
- firewall;
- clone/decoy;
- cover;
- mudança de fila;
- controle do agressor.

## 3.5 Identidade dos NPCs

Dois inimigos com os mesmos stats não precisam jogar igual. A identidade pode estar em:

- conhecimento;
- pesos de utility;
- temperatura de seleção;
- aversão a risco;
- persistência de alvo;
- preferências de família;
- disciplina de itens;
- coordenação;
- capacidade de antecipar ações.

---

# 4. Arquitetura recomendada

## 4.1 Camadas

### A. `CombatRules`

Responsável por invariantes:

```text
legalidade
custos
cooldowns
recursos
cálculo de dano
status
fila/relógio
condições de vitória
loops de efeito
limites duros de segurança
```

A IA **não** pode contornar essas regras.

### B. `EncounterDirector`

Responsável por coordenação coletiva:

```text
pressure budget por alvo
reservation de ações
combos entre NPCs
não duplicar debuff inútil
não desperdiçar 4 ultimates no mesmo inimigo com 2 HP
limitar ações simultâneas/encadeadas
controle de fase de boss
```

### C. `KnowledgeModel`

Define o que o ator acredita ser verdade.

### D. `Brain`

Escolhe a política:

```text
ScriptedBrain
UtilityBrain
UtilityLookaheadBrain
ExpectimaxBrain
MctsBrain
AutoBattleBrain
BossBrain
```

### E. `ActionTargetEvaluator`

Avalia **par ação-alvo**, não ação e alvo em módulos desconectados.

### F. `IntentPreview`

Converte decisão interna em informação legível ao jogador.

### G. `CombatTelemetry`

Registra o porquê de cada score.

---

# 5. Relógio híbrido de combate

O usuário pediu um híbrido entre ATB/CTB e turnos discretos. O modelo mais flexível é um **ActionClock contínuo com janelas discretas de decisão**.

## 5.1 Estado

Cada ator possui:

```text
next_ready_time
speed
current_cast
recovery
priority
interrupt_state
```

Existe uma fila de eventos:

```text
Ready(actor)
ResolveCast(actor, action)
StatusTick(actor)
EnvironmentTick
RoundBoundary
```

## 5.2 Delay por velocidade

Defina:

- `SPD` = velocidade do ator;
- `S_ref` = velocidade de referência;
- `K_s` = constante de suavização;
- `baseDelay(action)` = custo temporal da ação.

Use:

```text
speedFactor = (K_s + S_ref) / (K_s + SPD)
actionDelay = baseDelay(action) * speedFactor
```

Com clamp:

```text
speedFactor ∈ [0.65, 1.55]
```

Isso reduz o risco de SPD gerar crescimento hiperbólico de número de turnos.

## 5.3 Ações rápidas e lentas

```text
Rápida:
  decide → resolve → recovery

Lenta:
  decide → paga custo → agenda ResolveCast no futuro
  pode ser interrompida
  pode ler parte do estado no momento da resolução
```

Isso encaixa diretamente no `CARTAS-CAST-TIME` já planejado no GusWorld.

## 5.4 Janela de party

Quando dois ou mais aliados entram numa janela pequena:

```text
abs(next_ready_time_i - next_ready_time_j) <= epsilon_party
```

o jogador pode ordenar esses aliados livremente dentro do bloco.

Vantagens:

- mantém sensação tática;
- SPD continua importando;
- permite combo;
- evita microespera;
- lembra CTB/ATB sem copiar um jogo específico.

## 5.5 Round lógico

Mesmo com ActionClock contínuo, mantenha `RoundIndex` lógico para:

- clima;
- efeitos “por rodada”;
- anti-stall;
- métricas;
- cooldowns que não devem depender do número de atores.

Uma rodada pode terminar quando todos os atores vivos no snapshot inicial da rodada tiveram ao menos uma oportunidade de agir.

---

# 6. Modelo de dados de combate

```text
CombatActor
  id
  team
  tags
  role
  coreStats
  derivedStats
  hp
  resources
  inventory
  statuses
  cooldowns
  actionClock
  knowledgeProfile
  brainProfile
  criticalFlags
  targetingMemory
  actionMemory

ActionSpec
  id
  tags
  resourceCost
  itemCost
  baseDelay
  castDelay
  recovery
  targetShape
  scaling
  hitModel
  damageModel
  statusPayload
  utilityHints
  telegraphClass

ItemStack
  itemId
  quantity
  charges
  cooldownGroup
  scarcityClass
  reservePolicy

CombatState
  actors
  timeline
  environment
  round
  encounterObjectives
  pressureLedger
  effectLedger
  rngState
  progressLedger
```

## 6.1 Tags são preferíveis a ifs por personagem

Exemplo:

```text
CriticalActor
UniversalCompiler
Healer
Tank
Decoy
Boss
Hunter
KnowsCriticalActor
CannotFlee
ReviveRestricted
```

Assim, “Gus perde → party perde” fica em:

```text
LossCondition:
  if any actor with CriticalActor is Defeated:
      PartyLoss
```

e não:

```cpp
if (actor.name == "Gus") ...
```

---

# 7. Arquitetura de atributos

Recomendo separar **stats de fantasia do personagem** e **valores táticos derivados**.

## 7.1 Stats primários

| Stat | Função |
|---|---|
| `HP` | reserva de vida |
| `STR` | força física |
| `INT` | capacidade técnica/mágica/cognitiva |
| `DEF` | defesa física |
| `RES` | resistência técnica/mágica/status |
| `SPD` | relógio/iniciativa |
| `ACC` | precisão |
| `LUK` | crítico, resistência a crítico e pequenos efeitos probabilísticos |

Opcionalmente, se o sistema ficar muito dependente de CC:

| Stat | Função |
|---|---|
| `CTL` | potência de controle/status |
| `RSL` | resolve/tenacidade |

Mas é possível derivar `CTL` de `INT` e `RSL` de `RES` para reduzir carga cognitiva.

## 7.2 Stats derivados

```text
PhysicalPower
TechPower
HealingPower
Armor
TechDefense
HitChance
CritChance
CritResist
ControlPower
Resolve
TimelineSpeed
ThreatGeneration
```

## 7.3 Gus

Gus pode ter:

```text
STR baixo
HP baixo
INT muito alto
ACC alto
SPD médio
DEF baixa-média
RES média-alta
```

O ponto decisivo é: **baixo STR não deve significar inutilidade ofensiva**. Cartas “software/magia” devem escalar principalmente com `INT`.

Exemplo:

```text
Impacto físico:
  scaling = 0.8 STR + 0.1 INT

Compilado:
  scaling = 0.1 STR + 0.9 INT

Controle:
  scaling = 0.7 INT + 0.3 ACC

Cura bio:
  scaling = 0.6 INT + coefficient próprio do healer
```

## 7.4 Não criar um stat visível “Threat” como regra geral

Threat deve ser principalmente **derivado do estado recente**, porque:

- dano recente muda;
- cura recente muda;
- vulnerabilidade muda;
- vendetta muda;
- taunt muda;
- objetivo muda.

Um `ThreatModifier` de equipamento pode existir, mas a “ameaça” final é uma variável dinâmica.

---

# 8. Fórmulas de dano, cura, acerto, crítico e status

## 8.1 Dano base divisivo

O modelo já presente no GusWorld é bom:

```text
mitigation = K / (K + EffectiveDefense)
```

Com `K = 100` como ponto inicial.

Para uma carta:

```text
offense =
    Power
  + a_STR * STR
  + a_INT * INT

baseDamage =
    offense
  * mitigation
  * weaknessMultiplier
  * environmentMultiplier
  * comboMultiplier
  * exposeMultiplier
  * synergyMultiplier
```

Depois:

```text
finalDamage = channel(baseDamage, crit, variance, fumble)
```

## 8.2 Por que evitar dano puramente subtrativo como fórmula principal

```text
damage = max(1, Atk - Def)
```

é legível, mas cria degraus:

- abaixo de determinado Atk, todo mundo causa 1;
- +1 Atk pode ser irrelevante;
- +1 Def pode mudar muitos ataques de uma vez;
- bots de ataque básico podem produzir artefatos de balanceamento.

O ataque básico pode continuar subtrativo como fallback temático, mas o balanceamento principal deve ocorrer em fórmulas suaves.

## 8.3 Precisão

Uma função logística evita extremos fáceis:

```text
rawHit =
    1 / (1 + exp(-(2.20 + (ACC - EVA) / 15)))

pHit = clamp(rawHit, 0.05, 0.99)
```

Em igualdade, `pHit ≈ 0.90`.

Os números são **seed de playtest**, não canon.

## 8.4 Crítico

```text
pCrit =
    clamp(
        baseCrit
      + cardCritBonus
      + (LUK_attacker - LUK_defender) * critSlope,
      critMin,
      critMax
    )
```

Sugestão inicial:

```text
critMin  = 0.02
critMax  = 0.40
critMult = 1.5
```

O GusWorld hoje usa piso global de crítico em torno de 5%. Isso pode ser preservado se já estiver alinhado à fantasia de sistema.

## 8.5 Variância

O Knowledge Decay atual é particularmente coerente com o jogo:

```text
v = max(0.05, 0.30 * exp(-kills * 0.10))
```

A ideia é excelente porque converte conhecimento em previsibilidade.

Recomendação: **não aumentar a variância para criar dificuldade**. Hard deve jogar melhor, não apenas rolar dados piores para o jogador.

## 8.6 Cura

```text
rawHeal =
    BaseHeal
  + h_INT * INT
  + h_role * HealingAffinity

heal =
    rawHeal
  * healingReceivedMultiplier
  * fatigueMultiplier
```

Recomendações:

- sem crítico de cura por padrão;
- overheal não gera recurso;
- cura não remove automaticamente status;
- cura forte consome recurso de oportunidade;
- heal recebido pode sofrer anti-stall/fatigue de forma telegráfica.

## 8.7 Status

```text
effectiveControl = BaseStatusPower + CTL
effectiveResolve = RSL + targetResistanceBonus

pStatus =
    clamp(
      baseChance * (100 + effectiveControl) /
                   (100 + effectiveResolve),
      pMinStatus,
      pMaxStatus
    )
```

Para bosses, preferir barra de `Resolve` a imunidade binária universal.

---

# 9. Economia de ações

Em party-based RPG, **action economy** costuma ser mais determinante do que HP.

Uma ação deve ter pelo menos quatro preços possíveis:

```text
AP
mana/bateria
tempo no ActionClock
consumível/carga
```

A maioria das ações não precisa pagar os quatro; mas ações muito poderosas devem ter custo em mais de um eixo.

## 9.1 Valor esperado de uma ação

```text
ActionValue =
    expectedHPDelta
  + expectedControlValue
  + expectedTimelineValue
  + objectiveValue
  + futureSetupValue
  - resourceOpportunityCost
  - exposureRisk
```

## 9.2 “Dano por AP” não basta

Uma habilidade de stun de 0 dano pode ser superior a 50 de dano se evitar duas ações inimigas.

Use `EquivalentActionValue`:

```text
1 ação inimiga negada ≈ valor médio de uma ação daquele inimigo
```

Isso permite comparar:

```text
damage
heal
shield
stun
haste
slow
reorder
scan
```

na mesma escala de utility.

---

# 10. Utility AI como núcleo

Utility AI é a opção padrão mais adequada porque o estado tem:

- muitas ações;
- múltiplos alvos;
- itens;
- cooldowns;
- informação parcial;
- status;
- fraquezas;
- objetivos;
- custos futuros.

A arquitetura de *Dragon Age: Inquisition* descreve exatamente um problema semelhante: enumerar comportamentos legais, pontuar utilidade e selecionar ação/alvo [R1]. David Graham descreve normalização e “maximum expected utility” como forma prática de comparar escolhas heterogêneas [R4]. Mike Lewis mostra considerações utilitárias aplicadas a seleção de skills [R5].

## 10.1 Score normalizado

Cada consideration retorna `[0,1]`.

Exemplo para ataque:

```text
C_damage
C_kill
C_weakness
C_resource
C_setup
C_risk
C_repeat
C_pressure
```

Score:

```text
U = Σ(w_i * C_i)
```

ou, quando um fator deve ser veto:

```text
U = baseUtility * Π gate_i * compensation(...)
```

## 10.2 Não deixar “muitos fatores medíocres” vencerem por soma

Se 8 considerações de 0,55 derrotam uma escolha com 2 considerações de 0,95, pode ocorrer comportamento contraintuitivo.

Uma solução:

```text
U = weightedMean(considerations) * confidenceFactor
```

ou usar o método de compensação descrito em arquiteturas utilitárias.

## 10.3 Score deve registrar decomposição

Exemplo de log:

```text
ACTION FireBolt -> Gus
  base             +0.20
  expected_damage  +0.24
  weakness         +0.18
  kill_probability +0.16
  critical_target  +0.00  (fact unknown)
  repeat_penalty   -0.08
  pressure_penalty -0.10
  charge_cost        -0.04
TOTAL               0.56
```

Isso é indispensável para depurar IA.

---

# 11. Escolha conjunta ação + alvo

Evite:

```text
1. escolher ação
2. escolher alvo por outra heurística
```

Isso cria incoerências.

Exemplo:

```text
AI decide Fire
target module decide alvo imune a Fire
```

O correto:

```text
candidates = {
  (Attack, Gus),
  (Attack, Cauã),
  (Fire, Gus),
  (Fire, Cauã),
  (Heal, Self),
  ...
}
```

Cada par recebe utility.

Pseudo:

```text
for action in legalActions:
    for targetSet in legalTargets(action):
        score = evaluate(action, targetSet, beliefState)
        candidates.add(action, targetSet, score)
```

Depois, o EncounterDirector aplica coordenação coletiva.

---

# 12. Targeting: foco tático vs. tunnel vision

Este é o ponto central da pesquisa.

## 12.1 Fatores de targeting

Use:

```text
ThreatRecentDamage
ThreatRecentHealing
ExecuteLowHP
SupportPriority
WeaknessExploit
ObjectivePriority
Vendetta
Taunt
Decoy
Firewall
KnownCriticality
ControlThreat
CastInterruptValue
DefenseEfficiency
RecentTargetPenalty
PressurePenalty
Visibility
KnowledgeConfidence
```

## 12.2 Score de alvo

```text
S_target(t) =
    wDamage      * RecentDamage(t)
  + wHeal        * RecentHealing(t)
  + wExecute     * ExecuteValue(t)
  + wSupport     * SupportValue(t)
  + wWeakness    * WeaknessValue(t)
  + wControl     * ControlThreat(t)
  + wObjective   * ObjectiveValue(t)
  + wVendetta    * Vendetta(t)
  + wTaunt       * Taunt(t)
  - wFirewall    * Firewall(t)
  - wRepeat      * RepeatPenalty(t)
  - wPressure    * PressurePenalty(t)
  + wCritical    * KnownCriticality(t)
```

## 12.3 Foco tático justificável

Focus deve ser explicitamente permitido se houver uma razão forte.

Defina:

```text
JustifiedFocus =
    KillProbability >= K_kill
 OR TargetHasDangerousCast
 OR TauntActive
 OR VendettaActive
 OR KnownCriticalObjective
 OR UtilityGap >= K_gap
```

Quando `JustifiedFocus == true`:

```text
RepeatPenalty *= focusExceptionFactor
PressurePenalty *= focusExceptionFactor
```

Não zere necessariamente as penalidades.

## 12.4 Tunnel vision

Tunnel vision é:

> repetir alvo porque a política ficou presa nele, sem ganho marginal que justifique a persistência.

Detecte:

```text
sameTargetRun >= N
AND utilityGap < threshold
AND no explicit tactical exception
```

Então aplique uma penalidade progressiva.

Exemplo:

```text
repeatPenalty =
    1 - exp(-lambda_repeat * consecutiveHitsOnTarget)
```

## 12.5 Não usar “cada alvo deve receber 25% dos ataques”

Isso seria artificial. Um healer exposto deve receber mais pressão que um tanque blindado.

A métrica correta compara:

```text
observedTargetShare
vs.
expectedTargetShareGivenTacticalState
```

---

# 13. Coordenador de pressão ofensiva

A literatura de jogos de ação mostra um padrão reutilizável: um coordenador central pode controlar a **capacidade ofensiva** que converge sobre o jogador [R2].

Para um RPG por turnos, não precisamos copiar o sistema espacial. Podemos converter a ideia em `TargetPressureBudget`.

## 13.1 Peso de pressão de uma ação

```text
pressureCost(action, target) =
    expectedDamage / targetMaxHP
  + hardCCEquivalent
  + lethalThreat
  + irreversibleDebuff
```

Exemplo:

```text
golpe de 20% HP = 0.20
stun de uma ação = 0.25
stun + 20% HP    = 0.45
ultimate letal   = 0.80+
```

## 13.2 Ledger

Por rodada:

```text
TargetPressureLedger[target] += pressureCost
```

## 13.3 Penalidade suave

```text
excess = max(0, pressureUsed - pressureBudget)

coordinationMultiplier =
    exp(-lambda_pressure * excess)
```

Não é proibição.

## 13.4 Exceções

A IA pode ultrapassar o orçamento se:

```text
boss scripted mechanic
vendetta
target can be killed with high confidence
player explicitly taunted
story hunter
hard difficulty + master tactician
```

## 13.5 Por que isso é melhor que “não atacar o mesmo membro duas vezes”

Porque:

- permite execução racional;
- impede dogpile acidental;
- escala com potência do ataque;
- funciona com AoE;
- funciona com 1–6 inimigos;
- pode variar por dificuldade;
- é legível;
- é testável.

---

# 14. Gus como ator crítico

A regra “Gus caiu → a party perdeu” transforma Gus em algo matematicamente parecido com:

```text
King / VIP / protected objective
```

## 14.1 Problema de teoria dos jogos

Se um inimigo sabe perfeitamente que:

```text
Kill(Gus) => WinBattle
```

e não há custo maior para atacá-lo, então `Attack Gus` tende a dominar.

Portanto, há três soluções possíveis:

1. tornar Gus difícil de alcançar;
2. impedir a IA de saber a condição;
3. dar ao jogador mecanismos de redirecionamento/proteção.

A melhor solução é combinar as três **em graus diferentes**.

## 14.2 Criticality não deve ser conhecimento global

Dados:

```text
Fact: GusIsCritical
Visibility:
  CommonTrash      = Unknown
  SmartElite       = Suspected
  Hunter           = Known
  Sterling         = Known
  BetrayerDante    = Known
```

## 14.3 Níveis

### NPC burro

Não sabe.

Gus só ganha threat pelo que faz.

### NPC mediano

Percebe que Gus usa Scan/controle e pode classificá-lo como suporte perigoso.

### Elite

Pode inferir:

```text
Gus protegido demais
Gus altera fila
Gus controla informação
```

mas não necessariamente sabe que a batalha termina com sua queda.

### Caçador / boss informado

Sabe explicitamente.

Nesse caso, foco é **tático e narrativamente justificável** e deve ser telegrafado.

## 14.4 Proteções necessárias

O jogador precisa ter pelo menos três famílias de resposta:

```text
1. redirect:
   Bento intercept / guard

2. targeting manipulation:
   Iara decoy
   honeypot / taunt
   firewall

3. timeline control:
   Slow, Stun, reorder, interrupt
```

A Análise Preditiva de Gus já é uma quarta camada.

## 14.5 Não compensar apenas com HP

Aumentar muito o HP de Gus destruiria sua identidade de glass utility.

O problema é sistêmico; a resposta deve ser sistêmica.

---

# 15. Conhecimento e inteligência do NPC

Dificuldade e inteligência narrativa são eixos independentes.

## 15.1 `KnowledgeProfile`

```text
Perception
Memory
Inference
PriorKnowledge
HiddenInfoAccess
LearningRate
ForgetRate
DeceptionResistance
```

## 15.2 Classes

### Ignorante

```text
observa apenas efeitos explícitos
não conhece resistências
não conhece inventário
não sabe cooldowns
memória curta
```

### Comum

```text
lembra o que viu na batalha
aprende fraquezas após tentativa
estima papel da party
```

### Treinado

```text
conhece arquétipos
estima dano
rastreia cooldowns visíveis
infere recurso provável
```

### Mestre

```text
memória longa
inferência precisa
conhece kits documentados
rastreia item usage
antecipa combo
```

### Omnisciente permitido

Só para entidade cujo lore justifica.

## 15.3 `BeliefState`

A IA não deve consultar `CombatState` cru quando não tem direito.

```text
Belief<T>:
  estimate
  confidence
  lastObservedRound
  provenance
```

Exemplo:

```text
Gus.RES:
  estimate = 12
  confidence = 0.35
  source = inferred_from_damage
```

## 15.4 Atualização Bayesiana simples sem exigir Bayes completo

Pode-se usar atualização exponencial:

```text
estimate_new =
    alpha * observation
  + (1-alpha) * estimate_old
```

onde `alpha` depende da inteligência.

---

# 16. Easy / Normal / Hard

Não mude todas as dimensões ao mesmo tempo.

## 16.1 Matriz

| Eixo | Easy | Normal | Hard |
|---|---|---|---|
| knowledge | limitado | coerente com NPC | mais memória/inferência |
| lookahead | 0 | 0–1 ply | 1–2 ply seletivo |
| utility noise | maior | moderado | pequeno |
| top-k | amplo | médio | estreito |
| focus persistence | baixa | tática | maior quando justificado |
| item conservation | ruim | normal | boa |
| combo coordenação | rara | comum elite | forte |
| telegraph | exato | exato | exato, exceto inimigo explicitamente caótico |
| hidden cheating | não | não | não, salvo lore |
| stat multiplier | pequeno | 1.0 | pequeno/moderado |

## 16.2 Temperatura de seleção

Em vez de `argmax` sempre:

```text
P(a) = exp(U(a)/T) / Σ exp(U(i)/T)
```

- Easy: `T` maior;
- Normal: médio;
- Hard: menor.

Mesmo Hard pode manter pequeno `epsilon` de diversidade para evitar comportamento robótico.

## 16.3 Hard não deve “ler o input”

A IA deve decidir a partir do estado que legitimamente conhece, não da escolha ainda não resolvida do jogador, salvo mecânica explicitamente declarada.

---

# 17. IA de aliados

A IA de aliado tem uma função diferente da IA inimiga:

> maximizar valor sem roubar agência do jogador.

## 17.1 Prioridades

```text
1. evitar morte absurda
2. cumprir comando explícito
3. preservar ator crítico
4. cumprir papel
5. aproveitar fraqueza
6. economizar recursos raros
```

## 17.2 Perfis

### Conservative

```text
cura cedo
não usa item raro
evita cast arriscado
```

### Balanced

```text
utility normal
```

### Aggressive

```text
prioriza kill/setup
cura mais tarde
```

### RoleStrict

```text
Bento protege
Jaci cura/buffa
Cauã burst
```

## 17.3 Command overrides

Comandos do jogador podem adicionar utility:

```text
FOCUS target     +X
PROTECT Gus      +X
SAVE ITEMS       itemShadowCost ×2
BURST            resourceShadowCost ×0.5
CONTROL          CC utility ×1.5
```

Isso é superior a scriptar toda ação.

---

# 18. IA × IA e autobattle

## 18.1 Objetivos

Autobattle precisa ser:

- determinístico com seed;
- rápido;
- reproduzível;
- capaz de usar todo sistema;
- não necessariamente “divertido de assistir” no harness.

## 18.2 Dois brains

Tenha:

```text
FastAutoResolveBrain
RepresentativePlayerBrain
```

O primeiro otimiza throughput.

O segundo tenta modelar decisão humana.

Não use apenas um bot “bom” para balancear: ele mede uma única política.

## 18.3 Limite duro

Toda simulação deve ter:

```text
maxRounds
maxActions
maxEffectEvents
maxWallClock
```

Se estourar:

```text
SimulationAborted(Stall)
```

e o cenário reprova.

---

# 19. Comparação dos algoritmos de IA

## 19.1 Script / FSM

**Bom para:**

- tutorial;
- trash simples;
- boss phase;
- sequências narrativas;
- identidade forte.

**Ruim para:**

- muitos itens;
- múltiplos alvos;
- combinações emergentes.

**Uso recomendado:** camada externa.

## 19.2 Behavior Tree

**Bom para:**

- reatividade;
- execução;
- hierarquia;
- fallback;
- estados de cast e movimentação.

**Ruim como única escolha tática:** árvores ficam grandes quando cada ação precisa comparar utility com muitas outras.

**Uso recomendado:** execução e controle de alto nível.

## 19.3 Utility AI

**Bom para:**

- ações heterogêneas;
- itens;
- targeting;
- cura;
- status;
- personalidade;
- dados.

**Custo:** tuning de funções e pesos.

**Uso recomendado:** núcleo.

## 19.4 Minimax

Adequado a:

```text
jogo determinístico
adversarial
informação perfeita
branching pequeno
```

Seu combate tem RNG, informação parcial e muitos atores; minimax puro não é o melhor padrão.

## 19.5 Expectimax

Adiciona nós de chance.

Útil para:

```text
lookahead de 1–2 ply
ações de grande impacto
Hard/boss
```

## 19.6 MCTS / UCT

UCT foi proposto por Kocsis & Szepesvári para planejamento Monte Carlo em espaços grandes [R6].

**Vantagens:**

- não precisa expandir tudo;
- trabalha bem com simulador;
- escala com orçamento de rollout.

**Desvantagens:**

- custo;
- variância;
- difícil explicar;
- pode descobrir comportamento anti-diversão;
- precisa de rollout policy boa.

**Recomendação:** seletivo e offline.

## 19.7 ISMCTS

Quando existe informação oculta, MCTS sobre estado verdadeiro trapaceia.

ISMCTS pesquisa sobre conjuntos de informação [R7].

Use se:

- inventário oculto importa muito;
- ações secretas;
- PvP com informação escondida;
- bosses inferem mas não sabem tudo.

## 19.8 Dynamic scripting / RL

Dynamic scripting adapta pesos/regras ao oponente [R8].

Bom para pesquisa e inimigos adaptativos.

Risco:

- imprevisibilidade;
- QA;
- quebra de telegraph;
- “counter-picking” automático;
- jogador sentir que o jogo invalida sua build.

**Recomendação:** se usado, atualizar **entre encontros** ou em bosses explicitamente adaptativos; não mudar silenciosamente a função de dano durante a luta.

## 19.9 DDA

Há evidência experimental de que balanceamento adaptativo pode melhorar satisfação em certos contextos [R9], e modelos temporais podem estimar dificuldade/skill em combate RPG [R10]. Entretanto, DDA mal escondido pode comprometer confiança; Hunicke já tratava a necessidade de preservar a experiência central [R11].

**Recomendação:** dificuldade explícita primeiro; DDA apenas com limites.

---

# 20. Itens e inventário real

## 20.1 Mesmas regras para jogador e IA

Se o inimigo possui:

```text
2 potions
1 revive
1 buff
```

a IA deve realmente consumir.

## 20.2 Utility de item

```text
U_item =
    ImmediateCombatGain
  - ActionOpportunityCost
  - ItemScarcityCost
  - FutureEncounterShadowCost
  - OverhealWaste
```

## 20.3 Shadow price

```text
scarcityCost =
    itemBaseValue
  * reserveFactor
  * campaignImportance
```

NPC descartável pode ter `reserveFactor = 0`.

Aliado persistente pode ter alto reserve.

## 20.4 Cure threshold errado

Evite:

```text
if HP < 30%: potion
```

Use:

```text
DeathRiskBeforeNextAction
ExpectedIncomingDamage
HealAmount
Overheal
ItemScarcity
```

Exemplo:

```text
pDeathWithoutHeal = 0.65
pDeathWithHeal    = 0.12
```

isso dá valor alto à cura.

---

# 21. Cura e prevenção de luta infinita

Nenhum mecanismo isolado é suficiente em todos os casos.

## 21.1 Limite de quantidade

**Prós:** simples, diegético, forte.  
**Contras:** pode ser hoard-friendly.

É a primeira linha.

## 21.2 Cooldown

**Prós:** impede spam imediato.  
**Contras:** ainda permite ciclo infinito se nenhum recurso acaba.

## 21.3 Custo crescente

Exemplo:

```text
healCost(n) = baseCost * (1 + 0.25*n)
```

Bom como pressão, mas pode parecer artificial.

## 21.4 Redução progressiva de eficiência

```text
healMultiplier =
  max(healFloor, 1 - decay * repeatedHeals)
```

Use apenas se tematizado como:

```text
heal fatigue
toxicity
battery degradation
```

## 21.5 Bateria/cargas

Muito boa para o GusWorld porque já existe conceito de bateria.

A bateria atual proposta para Jaci deve ser testada em **sequência de encontros**, porque a Fase B já encontrou que o limite pode nem ser alcançado numa luta de ~4 rodadas.

## 21.6 Soft enrage

Após duração-alvo:

```text
BattlePressure += 1
damageDealt    *= 1 + 0.05 * pressure
healingReceived*= 1 - 0.05 * pressure
```

com caps.

É uma rede útil, não deve ser a solução primária.

## 21.7 Recomendação combinada

Para cura comum:

```text
finite quantity / battery
+ action cost
+ cooldown category
+ AI shadow price
+ StallGuard global
```

Para boss:

```text
boss heal limitado por fase
+ interrupt/telegraph
+ anti-stall pressure
```

---

# 22. Detector geral de stall

Crie uma camada que não dependa de “cura”.

## 22.1 O que é progresso

Progresso irreversível inclui:

```text
consumível gasto
carga/bateria gasta
ator derrotado
objetivo avançado
phase transition
max resource reduzido
revive charge gasto
```

Dano que é curado integralmente não é progresso líquido.

## 22.2 Janela

```text
window = últimas K rodadas
```

Calcule:

```text
NetProgress =
    irreversibleResourceLoss
  + weightedDefeats
  + objectiveDelta
  + unrecoveredHPDelta
```

Se:

```text
NetProgress < epsilon
for K rounds
```

marque `StallSuspected`.

## 22.3 Escada de resposta

### Nível 0

Nada.

### Nível 1 — IA

Penaliza ações que repetem ciclo sem ganho.

### Nível 2 — Warning

Telegraph:

```text
“instabilidade crescente”
```

### Nível 3 — Battle Pressure

Aumenta pressão gradualmente.

### Nível 4 — Hard safety

No cap extremo:

- PvE: evento diegético de colapso/enrage/escape/derrota conforme encontro;
- AI×AI: adjudicação por score;
- PvP: regra de tempo/turn cap explícita.

---

# 23. Anti-turtle

Defender deve:

- comprar tempo;
- reduzir dano;
- habilitar contra-ataque;
- permitir proteger outro.

Não deve:

- regenerar shield infinito sem custo;
- impedir todo dano indefinidamente;
- vencer sem progresso.

## 23.1 Opções

### Shield com pool persistente

Pool não renasce totalmente todo turno.

### Shield regenerável com custo

```text
Defend consumes AP
and/or battery
```

### Guard redirect

```text
Bento intercepts x% of targeted actions
```

### Chip

Alguns ataques ignoram parte de shield.

### Break

Defesa repetida aumenta vulnerabilidade a `Break`.

### Pressure

Turtle aumenta Battle Pressure.

## 23.2 Melhor solução para Bento

Separar:

```text
Defend = proteção própria
Guard = redirecionamento explícito
Taunt = targeting manipulation
```

Isso remove a dependência de “IA ser burra o suficiente para bater no tanque”.

---

# 24. Stun-lock e controle de grupo

## 24.1 Problema

Se:

```text
p(stun) alto
duration >= tempo de recarga
```

o alvo pode nunca agir.

## 24.2 Resolve meter

Cada hard CC aumenta:

```text
ResolveMeter += controlWeight
```

Próximo CC:

```text
effectiveDuration *= 1 / (1 + ResolveMeter)
```

e o meter decai quando o alvo age.

## 24.3 Immunity window

Após CC forte:

```text
HardCCImmunity = 1 opportunity
```

## 24.4 Boss

Boss não precisa ser simplesmente “IMMUNE”.

Melhor:

```text
BreakBar
```

CC reduz bar; quando zera, boss sofre stagger real.

## 24.5 Invariante

Em encontro normal, salvo mecânica explícita:

```text
nenhum ator pode perder > N oportunidades consecutivas
por hard CC reaplicável comum
```

---

# 25. Revive loops

## 25.1 Causas

```text
revive barato
revive sem cooldown
revive com HP alto
revive permite agir imediatamente
recursos renováveis
```

## 25.2 Contramedidas

Escolha 2–3:

```text
revive charges
HP baixo após revive
revive sickness
timeline delay
custo crescente
cooldown de equipe
revive não pode ressuscitar CriticalActor em certos modos
```

## 25.3 Sugestão

```text
revive:
  25–35% HP
  perde próxima prioridade / grande delay
  aplica Fragile por 1 rodada
  consome carga finita
```

Evite aplicar todas as punições simultaneamente.

---

# 26. Reflect loops, proc loops e recursão de efeitos

Este problema deve ser resolvido por arquitetura.

## 26.1 Provenance

Todo efeito:

```text
EffectEvent
  eventId
  originActionId
  sourceActor
  currentActor
  target
  effectType
  parentEventId
  reflectionDepth
  triggerDepth
  tags
```

## 26.2 Reflect

Regra padrão:

```text
if reflectionDepth >= 1:
    cannotReflectAgain
```

ou:

```text
Reflected tag → non-reflectable
```

## 26.3 Cycle signature

```text
signature =
  (originActionId, effectType, source, target, hook)
```

Se a mesma assinatura reaparece na mesma cadeia:

```text
CycleDetected
```

## 26.4 Event budget

```text
maxEffectEventsPerAction = 128
maxTriggerDepth = 16
```

Números altos para nunca afetar jogo legítimo, mas matar bugs.

## 26.5 Nunca “deixar o stack estourar”

O domínio deve retornar erro controlado e logar a cadeia.

---

# 27. Evasão e precisão degeneradas

## 27.1 Floors/caps

```text
pHit ∈ [5%, 99%]
```

ou faixas diferentes por categoria.

## 27.2 True-hit

Algumas habilidades:

```text
CannotMiss
```

mas com custo.

## 27.3 Anti-evasion status

```text
Expose / Decrypt
```

pode reduzir EVA.

## 27.4 DR de evasão

Evite somar percentuais diretamente:

```text
EVA_effective = K * EVA_raw / (K + EVA_raw)
```

Isso dá retornos decrescentes.

---

# 28. One-shot e burst excessivo

One-shot não é sempre ruim. É ruim quando:

```text
não telegrafado
não prevenível
turno 1
atinge ator crítico
não existe counterplay
```

## 28.1 Guardrail

Meça:

```text
FirstRoundLethalProbability
p05 TimeToGusLoss
UnpreventableBurstRate
```

## 28.2 Gus

A Análise Preditiva já funciona como buffer de uma fatalidade.

Mantenha-a como:

```text
safety valve
```

e não como desculpa para statline que mata Gus todo encontro.

## 28.3 Boss lethal

Pode existir se:

```text
telegraph forte
interruptível
guardável
exige falha anterior do jogador
```

---

# 29. Domínio de velocidade

## 29.1 Risco

Se SPD reduz delay sem limite:

```text
SPD ↑ → mais ações → mais Haste → mais ações → lock
```

## 29.2 Travas

- fórmula de delay com assíntota/clamp;
- `minActionDelay`;
- Haste com retornos decrescentes;
- limite de ações consecutivas sem oportunidade adversária, salvo habilidade explícita;
- Slow também clampado.

## 29.3 Haste

Em vez de:

```text
SPD += 100%
```

use:

```text
actionDelay *= 0.85
```

e stacking multiplicativo com cap.

---

# 30. Buff/debuff stacking

Defina para todo efeito:

```text
StackRule:
  Replace
  RefreshDuration
  AddMagnitude
  AddDuration
  MaxMagnitude
  IndependentInstances
```

## 30.1 Cap

Todo stat temporário deve ter cap de combate.

## 30.2 DR

Para buffs percentuais:

```text
effectiveBonus = 1 - Π(1 - bonus_i)
```

evita adição explosiva.

## 30.3 Debuff

Defesa negativa precisa de piso:

```text
EffectiveDefense >= defenseFloor
```

salvo mecânica específica.

---

# 31. Fraquezas, resistências e imunidades

O modelo atual 1.5 / 1.0 / 0.66 / 0 é claro.

## 31.1 Regras

- imunidade deve ser rara e legível;
- não esconder todas as resistências de trash;
- Scan deve reduzir incerteza;
- master NPC pode conhecer fraquezas da party;
- NPC burro deve aprender por tentativa.

## 31.2 Não criar “família universal melhor”

Faça auditoria de payoff:

```text
expectedDamagePerResource(family)
```

ao longo de todo bestiário.

---

# 32. Aleatoriedade e legibilidade

RNG deve produzir variedade, não arbitrariedade.

## 32.1 Tipos

### Outcome RNG

```text
hit/miss
crit
status
```

### Decision RNG

```text
softmax
top-k weighted
```

Não misture os dois no debug.

## 32.2 Filtered randomness

Para seleção de alvo, pode-se reduzir repetição sem proibir:

```text
recentlyChosen → temporary weight reduction
```

## 32.3 Seed

Todo combate:

```text
BattleSeed
```

e toda simulação guarda:

```text
seed
decisionIndex
rng stream
```

---

# 33. Telegraph e intenção

O contrato atual `IntentPreview` é uma vantagem arquitetural.

## 33.1 O que mostrar

Para inimigo comum:

```text
tipo de ação
alvo provável/exato
dano estimado
status
cast time
```

## 33.2 Precisão depende do inimigo

Telegraph não precisa significar “o inimigo é previsível”.

Pode existir:

```text
IntentConfidence
```

### trash

100%.

### elite

100% da intenção atual; pode reagir se estado mudar antes de agir.

### boss caótico

mostra classe de risco, não alvo exato.

## 33.3 Replanejamento

Se a ação planejada se tornar ilegal:

```text
target morreu
status removeu cast
alvo desapareceu
```

recalcule e atualize telegraph.

---

# 34. Chefes

Boss não deve ser apenas UtilityBrain com HP enorme.

## 34.1 Arquitetura

```text
BossPhaseFSM
  ↓
PhasePolicy
  ↓
Utility/Planner
  ↓
TargetCoordinator
```

## 34.2 Fases

Mudança por:

```text
HP threshold
turn threshold
objective
player solved mechanic
environment
specific status
```

## 34.3 Anti-cheese

Boss pode detectar repetição:

```text
sameFamilyShare
sameCC
sameTargetingExploit
sameDefendPattern
```

Mas deve responder com **contra-jogo amplo**, não “nega sua build”.

Exemplo:

```text
player usa muito stun
→ boss ativa Resolve mode
→ ainda pode ser quebrado por bar
```

## 34.4 Enrage

Soft enrage é preferível a hard enrage abrupto.

## 34.5 Puzzle boss

Como o próprio `combat.md` pretende “boss estilo Zelda”, o boss deve ter:

```text
Observe
Hypothesize
Exploit opening
Punishment window
Reset
```

Isso combina perfeitamente com Scan/Gambito.

---

# 35. Duração-alvo das lutas

Não existe um número universalmente “ótimo”. A variável mais útil é:

> **quantas decisões significativas o jogador toma antes de o estado ficar resolvido?**

## 35.1 Baseline recomendado

### Trash

```text
2–4 rodadas
```

### Normal/elite

```text
3–6
```

### Elite complexo

```text
5–8
```

### Boss

```text
10–16
```

### Final multi-phase

```text
18–28
```

O alvo atual de **3–5** para combate comum no GusWorld é razoável e já possui harness. Eu **não o mudaria agora**; ampliaria os testes para party 3–5 e inimigos 1–6 antes de alterar.

## 35.2 Medir distribuição, não média

Exija:

```text
median
p10
p90
% dentro da janela
```

Uma média de 4 pode esconder:

```text
50% em 1 rodada
50% em 7 rodadas
```

---

# 36. Escalonamento para 3–5 × 1–6

A maior armadilha é escalar somente HP.

## 36.1 Threat budget por encontro

Defina:

```text
EncounterThreatBudget
```

Cada inimigo consome:

```text
enemyThreatCost =
    expectedActions
  * expectedActionValue
  * survivabilityFactor
  * controlFactor
```

## 36.2 Muitos inimigos

Com 6 inimigos, evite:

```text
6 × kit completo × AP completo
```

Opções:

- swarm com ações fracas;
- shared attack capacity;
- enemy AP pool;
- cooldown global de AoE;
- pressão por alvo;
- alguns inimigos em setup enquanto outros atacam.

## 36.3 Crescimento sublinear

O poder efetivo de 6 unidades já cresce por:

```text
target coverage
CC overlap
action count
synergy
```

Logo, statline individual deve cair ou o action budget deve limitar.

---

# 37. PvP

As regras anti-loop permanecem.

Mas retire “fairness artificial” de targeting porque humanos escolhem alvo.

## 37.1 Necessário

```text
turn timer
battle timer
hard effect budget
item restrictions
revive rules
stall adjudication
deterministic seed policy
```

## 37.2 Não usar DDA

PvP competitivo não deve alterar poder em favor de quem está perdendo.

---

# 38. DDA e adaptação

## 38.1 Recomendação

Prioridade:

```text
1. Easy/Normal/Hard explícito
2. encounter composition
3. AI sophistication
4. só depois DDA
```

## 38.2 DDA seguro

Pode ajustar entre encontros:

```text
enemy mix
frequency
optional challenge
hint frequency
resource drop
```

## 38.3 DDA perigoso

Evite silenciosamente:

```text
reduzir dano porque jogador está perdendo
aumentar miss do inimigo
spawnar potion invisivelmente
mudar fraqueza
```

Isso destrói capacidade de aprender o sistema.

---

# 39. Simulação e harness

O GusWorld já tem uma base rara e valiosa: harness headless.

Transforme-o em um “laboratório de combate”.

## 39.1 Execução

Para cada cenário:

```text
N = 10k para exploração
N = 100k+ para confirmação
seeds controladas
múltiplas personas
```

Não use sempre N gigantesco durante busca de parâmetros.

## 39.2 Duas fases

### Screening

Barato.

### Confirmation

Grande N e pré-registro.

## 39.3 Reproduzir produção

Toda função crítica deve ser compartilhada:

```text
damage
targeting
item use
status
timeline
```

O harness não deve reimplementar fórmula.

---

# 40. Métricas de balanceamento

## 40.1 Resultado

```text
win rate
loss rate
escape rate
draw/stall rate
```

## 40.2 Pacing

```text
round median
p10/p50/p90
% in target window
actions per combat
decision count
```

## 40.3 Gus

```text
Gus defeat rate
Gus first-target rate
Gus damage share
Gus focus-run distribution
predictive-analysis consumption rate
unpreventable Gus loss rate
```

## 40.4 Targeting

```text
target share
target entropy
same-target transition rate
longest focus run
pressure per target
justified-focus ratio
unjustified-repeat ratio
```

### Entropia

```text
H = -Σ p_i log(p_i)
```

Normalize:

```text
H_norm = H / log(numberOfValidTargets)
```

Não exija H alto sempre; compare por arquétipo.

## 40.5 Ação

```text
action entropy
card dominance
item use
overheal
overkill
wasted debuff
CC chain length
```

## 40.6 Recursos

```text
mana remaining
battery remaining
items consumed
revives
resource waste
```

## 40.7 Anti-stall

```text
noProgress windows
pressure escalation activation
hardCap hits
max action count
```

---

# 41. Estatística e critérios de aprovação

## 41.1 Nunca exigir “0% verdadeiro”

Se 0 eventos foram observados em `N`, a probabilidade real não foi provada como zero.

A “regra de três” fornece aproximadamente:

```text
upper95 ≈ 3/N
```

para evento raro com zero observações [R15].

## 41.2 Critério correto

Em vez de:

```text
stall_count == 0
```

use:

```text
upperConfidenceBound(stallRate) < allowedRate
```

## 41.3 Win rate

Use intervalo de Wilson para proporções.

## 41.4 Pré-registro

Antes do run oficial, fixe:

```text
primary metrics
thresholds
MCID
N
seeds policy
exclusion rules
```

O repositório já está adotando uma disciplina próxima disso; continue.

---

# 42. Personas sintéticas de jogador

Playtesting automático com uma única IA “ótima” é insuficiente.

Holmgård et al. demonstram personas sintéticas com utilidades diferentes e MCTS para representar estilos [R12, R13].

## 42.1 Personas mínimas

### P1 — Basic Attacker

```text
ataque básico
```

Detecta dead zones.

### P2 — Rational DPS

```text
max expected damage
```

### P3 — Healer Conservative

```text
cura cedo
```

### P4 — Healer Greedy

```text
cura sempre que possível
```

### P5 — Turtle

```text
defende
```

### P6 — Control Lock

```text
maximiza CC
```

### P7 — Speed Abuse

```text
maximiza timeline
```

### P8 — Item Hoarder

```text
não usa item
```

### P9 — Item Spammer

```text
usa recurso cedo
```

### P10 — Gus Protector

```text
prioriza proteção de Gus
```

### P11 — Greedy Burst

```text
busca one-shot
```

### P12 — Random Legal

Controle negativo.

---

# 43. Casos adversariais obrigatórios

Cada release de combate deve testar:

```text
A. 1v1 de cura
B. healer vs healer
C. turtle total
D. reflect vs reflect
E. revive bilateral
F. stun chain
G. max EVA
H. max SPD
I. 6 inimigos focando Gus
J. 6 inimigos com AoE
K. 5 aliados + 6 inimigos
L. todos com Shield
M. item de cura no último HP
N. alvo morre durante cast
O. todos os alvos ficam inválidos
P. boss troca fase no mesmo tick em que morre
Q. reflect mata o originador
R. revive de CriticalActor
S. taunt + firewall simultâneos
T. decoy + vendetta + known criticality
```

---

# 44. Instrumentação e explicabilidade

## 44.1 Decision trace

```text
Decision #1842
Actor: DaemonGuard
KnowledgeProfile: Trained
Difficulty: Hard

Candidates:
  Slash->Gus     U=.61
  Slash->Caua    U=.59
  Break->Bento   U=.72
  Potion->Self   U=.34

Selected:
  Break->Bento

Reasons:
  shield_break .26
  target_role .10
  pressure .00
  resource -.03
  setup .39
```

## 44.2 Target trace

```text
Gus:
  threat .25
  lowHP .18
  critical 0.00 (UNKNOWN)
  repeat -.09
  pressure -.12
```

## 44.3 glintfx

`glintfx` pode ser usado para:

- debug overlay;
- timeline;
- heatmap de utility;
- intent icons;
- target weights;
- live harness telemetry.

Isso fica na camada UI; o domínio continua sem dependência.

---

# 45. Configuração data-driven

Exemplo ilustrativo:

```yaml
brain_profiles:
  trash_naive:
    selector: softmax
    temperature: 0.35
    lookahead: 0
    knowledge: ignorant
    target_policy:
      recent_damage: 0.30
      low_hp: 0.20
      support: 0.10
      weakness: 0.00
      repeat_penalty: 0.20
      pressure_penalty: 0.20

  elite_trained:
    selector: softmax
    temperature: 0.15
    lookahead: 1
    knowledge: trained
    target_policy:
      recent_damage: 0.25
      low_hp: 0.25
      support: 0.15
      weakness: 0.15
      repeat_penalty: 0.10
      pressure_penalty: 0.10

  gus_hunter:
    selector: top_k_softmax
    temperature: 0.08
    lookahead: 2
    knowledge: master
    facts:
      - GusIsCritical
    target_policy:
      critical_actor: 0.40
      low_hp: 0.20
      recent_damage: 0.10
      pressure_penalty: 0.05
```

Os números são apenas seeds de teste.

---

# 46. Pseudocódigo do sistema completo

## 46.1 Loop

```text
while not combatEnded:

    event = timeline.pop()

    applyScheduledEvent(event)

    if lossOrWinCondition():
        endCombat()
        break

    if event.type == READY:

        actor = event.actor

        if cannotAct(actor):
            scheduleNextOpportunity(actor)
            continue

        belief = knowledgeSystem.buildBelief(actor, state)

        legalActions = rules.enumerateLegalActions(actor, belief)

        candidates = []

        for action in legalActions:
            for targetSet in rules.enumerateLegalTargets(action, belief):

                baseUtility =
                    brain.evaluate(actor, action, targetSet, belief)

                coordinatedUtility =
                    director.applyCoordination(
                        actor,
                        action,
                        targetSet,
                        baseUtility,
                        pressureLedger,
                        reservations
                    )

                candidates.add(
                    action,
                    targetSet,
                    coordinatedUtility
                )

        selected =
            selector.choose(candidates, brain.temperature, rng)

        intentBus.publish(
            buildIntentPreview(selected, actor, belief)
        )

        resolution =
            rules.commitAction(selected)

        director.reserve(resolution)

        timeline.schedule(resolution)

    if roundBoundaryReached():
        stallGuard.update(state)
        environment.tick()
        pressureLedger.decayOrReset()
        knowledgeSystem.consolidate()
```

## 46.2 Utility

```text
function evaluate(actor, action, target, belief):

    if not legal:
        return -INF

    features = extractFeatures(actor, action, target, belief)

    utility = 0

    utility += w_damage    * curveDamage(features.expectedDamage)
    utility += w_kill      * curveKill(features.killProbability)
    utility += w_control   * curveControl(features.deniedActions)
    utility += w_support   * curveSupport(features.supportValue)
    utility += w_setup     * curveSetup(features.futureSynergy)
    utility += w_objective * curveObjective(features.objectiveProgress)

    utility -= w_resource  * curveResource(features.shadowCost)
    utility -= w_risk      * curveRisk(features.selfRisk)
    utility -= w_repeat    * curveRepeat(features.repeatState)
    utility -= w_stall     * curveStall(features.noProgressRisk)

    utility *= knowledgeConfidenceFactor(features)

    return clamp01(utility)
```

## 46.3 Target pressure

```text
function applyCoordination(action, target, utility):

    pressure = ledger[target]

    cost = estimatePressure(action, target)

    excess = max(0, pressure + cost - budget[target])

    penalty = exp(-lambda * excess)

    if isJustifiedFocus(action, target):
        penalty = lerp(penalty, 1.0, focusOverrideStrength)

    return utility * penalty
```

## 46.4 Justified focus

```text
function isJustifiedFocus(action, target):

    return (
        killProbability(action, target) >= KILL_THRESHOLD
        or interruptsDangerousCast(target)
        or target.has(TAUNT)
        or vendettaAgainst(target)
        or knowsCriticalAndCanExploit(target)
        or utilityGapToSecondBest >= GAP_THRESHOLD
    )
```

## 46.5 Heal

```text
function healUtility(actor, healAction, target):

    hpMissing = target.maxHP - target.hp

    effectiveHeal = min(
        expectedHeal(healAction, actor, target),
        hpMissing
    )

    overheal = expectedHeal(...) - effectiveHeal

    deathBeforeNext = estimateDeathRisk(target, withoutHeal=true)
    deathAfterHeal   = estimateDeathRisk(target, withoutHeal=false)

    survivalGain = deathBeforeNext - deathAfterHeal

    return (
        survivalGain * W_SURVIVAL
      + effectiveHeal / target.maxHP * W_HEAL
      - overheal / target.maxHP * W_OVERHEAL
      - itemShadowCost(healAction) * W_SCARCITY
      - stallRisk(healAction) * W_STALL
    )
```

## 46.6 Effect recursion

```text
function dispatchEffect(event, chainContext):

    if chainContext.totalEvents >= MAX_EFFECT_EVENTS:
        return EffectGuardAbort

    if event.triggerDepth > MAX_TRIGGER_DEPTH:
        return EffectGuardAbort

    signature = makeSignature(event)

    if chainContext.contains(signature):
        return CycleSuppressed

    chainContext.push(signature)

    result = resolve(event)

    for child in result.triggeredEvents:
        dispatchEffect(child, chainContext.child())

    chainContext.pop(signature)
```

---

# 47. Integração sugerida ao GusWorld

## 47.1 Preservar

Eu preservaria:

- FSM de combate;
- RNG seedável;
- `IntentPreview`;
- Knowledge decay;
- pipeline de efeitos;
- ambiente;
- status;
- Scan/Gambito;
- mana ramp;
- harness.

## 47.2 Alterar primeiro

### 1. Targeting de produção

O bug `players.front()` precisa desaparecer.

A implementação ponderada já estudada deve chegar ao call site real antes de nova canonização de statline.

### 2. Separar defesa e aggro

Criar mecanismos explícitos:

```text
Defend
GuardRedirect
Taunt/Honeypot
Firewall
Decoy
```

### 3. StallGuard

Antes de adicionar dezenas de cartas.

### 4. LoopGuard

Antes de Reflect e triggers complexos crescerem.

### 5. Healing multi-encounter harness

O próprio estudo recente mostrou que uma bateria que limita 6 usos não é testada por uma luta que termina em 4 rodadas.

## 47.3 Depois

- ActionClock;
- cast time;
- UtilityBrain;
- lookahead Hard;
- boss planner;
- personas avançadas.

---

# 48. Plano incremental

## Fase 0 — invariantes

```text
EffectLoopGuard
Stall hard cap
seed/replay
telemetry
```

## Fase 1 — targeting

```text
weighted target
repeat memory
pressure ledger
taunt
firewall
guard redirect
```

## Fase 2 — utility

```text
ActionTargetEvaluator
item utility
heal utility
status utility
```

## Fase 3 — knowledge

```text
belief state
NPC intelligence tiers
```

## Fase 4 — difficulty

```text
Easy/Normal/Hard brains
```

## Fase 5 — ActionClock

```text
ready queue
cast
recovery
interrupt
```

## Fase 6 — bosses

```text
phase FSM
mechanic-specific utility
selective search
```

## Fase 7 — automated balance

```text
personas
parameter sweeps
confidence intervals
regression gates
```

---

# 49. Decisões que devem continuar em playtest

Não congelaria agora:

```text
HP/Atk finais do trash
peso de F4 defender
budget de pressão exato
temperaturas Easy/Normal/Hard
limites de CC
duração de revive sickness
heal fatigue
soft-enrage slope
boss round windows
SPD clamp
crit cap
```

São parâmetros de sensação.

Eu congelaria **a arquitetura**, não os números:

```text
utility + action-target joint scoring
knowledge separado de difficulty
target pressure
explicit aggro tools
stall guard
event cycle guard
seeded simulation
statistical confidence thresholds
```

---

# 50. Referências

## Fontes de produção / engenharia de Game AI

**[R1]** Hanlon, S.; Watts, C. *Behavior Decision System: Dragon Age Inquisition’s Utility Scoring Architecture*. Game AI Pro 3, 2017.  
https://www.gameaipro.com/GameAIPro3/GameAIPro3_Chapter31_Behavior_Decision_System_Dragon_Age_Inquisition%E2%80%99s_Utility_Scoring_Architecture.pdf

**[R2]** Dawe, M. *Beyond the Kung-Fu Circle: A Flexible System for Managing NPC Attacks*. Game AI Pro, 2013.  
https://www.gameaipro.com/GameAIPro/GameAIPro_Chapter28_Beyond_the_Kung-Fu_Circle_A_Flexible_System_for_Managing_NPC_Attacks.pdf

**[R3]** Siemonsmeier, M. *Gearing the Tactics Genre: Simultaneous AI Actions in Gears Tactics*. Game AI Pro Online Edition, 2021.  
https://www.gameaipro.com/GameAIProOnlineEdition2021/GameAIProOnlineEdition2021_Chapter03_Gearing_the_Tactics_Genre_Simultaneous_AI_Actions_in_Gears_Tactics.pdf

**[R4]** Graham, D. “Rez”. *An Introduction to Utility Theory*. Game AI Pro, 2013.  
https://www.gameaipro.com/GameAIPro/GameAIPro_Chapter09_An_Introduction_to_Utility_Theory.pdf

**[R5]** Lewis, M. *Choosing Effective Utility-Based Considerations*. Game AI Pro 3, 2017.  
https://www.gameaipro.com/GameAIPro3/GameAIPro3_Chapter13_Choosing_Effective_Utility-Based_Considerations.pdf

Game AI Pro index, incluindo Behavior Trees, FSMs, Utility, Final Fantasy XV, MCTS, combat coordination, autoplay e outros capítulos:  
https://www.gameaipro.com/

## Busca, planejamento e informação parcial

**[R6]** Kocsis, L.; Szepesvári, C. *Bandit Based Monte-Carlo Planning*. ECML 2006. DOI: 10.1007/11871842_29.  
https://doi.org/10.1007/11871842_29

**[R7]** Cowling, P. I.; Powley, E. J.; Whitehouse, D. *Information Set Monte Carlo Tree Search*. IEEE TCIAIG 4(2), 2012. DOI: 10.1109/TCIAIG.2012.2200894.  
https://doi.org/10.1109/TCIAIG.2012.2200894

## IA adaptativa e dificuldade

**[R8]** Spronck, P. et al. *Adaptive Game AI with Dynamic Scripting*. Machine Learning 63, 217–248, 2006. DOI: 10.1007/s10994-006-6205-6.  
https://doi.org/10.1007/s10994-006-6205-6

**[R9]** Andrade, G.; Ramalho, G.; Gomes, A.; Corruble, V. *Dynamic Game Balancing: an Evaluation of User Satisfaction*. AIIDE 2006. DOI: 10.1609/aiide.v2i1.18739.  
https://doi.org/10.1609/aiide.v2i1.18739

**[R10]** Zook, A.; Riedl, M. *A Temporal Data-Driven Player Model for Dynamic Difficulty Adjustment*. AIIDE 2012. DOI: 10.1609/aiide.v8i1.12504.  
https://doi.org/10.1609/aiide.v8i1.12504

**[R11]** Hunicke, R. *The Case for Dynamic Difficulty Adjustment in Games*. ACE 2005. DOI: 10.1145/1178477.1178573.  
https://doi.org/10.1145/1178477.1178573

## Automated playtesting / personas

**[R12]** Holmgård, C.; Liapis, A.; Togelius, J.; Yannakakis, G. *Monte-Carlo Tree Search for Persona Based Player Modeling*. AIIDE 2015. DOI: 10.1609/aiide.v11i5.12849.  
https://doi.org/10.1609/aiide.v11i5.12849

**[R13]** Holmgård, C.; Green, M. C.; Liapis, A.; Togelius, J. *Automated Playtesting with Procedural Personas through MCTS with Evolved Heuristics*. IEEE Transactions on Games 11(4), 352–362. DOI: 10.1109/TG.2018.2808198.  
https://doi.org/10.1109/TG.2018.2808198

**[R14]** Horn, B.; Miller, J.; Smith, G.; Cooper, S. *A Monte Carlo Approach to Skill-Based Automated Playtesting*. AIIDE 2018. DOI: 10.1609/aiide.v14i1.13036.  
https://doi.org/10.1609/aiide.v14i1.13036

## Estatística

**[R15]** Hanley, J. A.; Lippman-Hand, A. *If Nothing Goes Wrong, Is Everything All Right? Interpreting Zero Numerators*. JAMA 249(13), 1743–1745, 1983. DOI: 10.1001/jama.1983.03330370053031.  
https://doi.org/10.1001/jama.1983.03330370053031

## Arquitetura/adaptação adicional

Bergsma, M.; Spronck, P. *Adaptive Spatial Reasoning for Turn-Based Strategy Games*. AIIDE 2008. DOI: 10.1609/aiide.v4i1.18690.  
https://doi.org/10.1609/aiide.v4i1.18690

Timuri, T.; Spronck, P.; van den Herik, H. *Automatic Rule Ordering for Dynamic Scripting*. AIIDE 2007. DOI: 10.1609/aiide.v3i1.18782.  
https://doi.org/10.1609/aiide.v3i1.18782

Zook, A.; Fruchter, E.; Riedl, M. *Automatic Playtesting for Game Parameter Tuning via Active Learning*, 2019.  
https://arxiv.org/abs/1908.01417

---

# Apêndice A — tabela de falhas e contramedidas

| Falha | Sintoma | Causa típica | Contramedida primária | Rede de segurança |
|---|---|---|---|---|
| Tunnel vision | mesmo herói sempre | target argmax/fixo | weighted targeting + memória | pressure budget |
| Foco irracional | insiste num tanque | sem avaliação marginal | action-target utility | repeat penalty |
| Foco racional frustrante | Gus morre sempre | IA conhece criticalidade | knowledge gating + proteção | Predictive Analysis |
| Cura infinita | HP oscila | recurso renovável | cargas/bateria | StallGuard |
| Healer trivializa | win rate explode | heal > incoming DPS | action/resource cost | multi-encounter tuning |
| Turtle infinito | nenhum lado progride | shield renovável | guard resource / break | BattlePressure |
| Stun-lock | alvo não age | CC sem DR | Resolve | immunity window |
| Revive-loop | morto volta sempre | revive renovável | charges | revive sickness |
| Reflect-loop | stack infinito | recursive hooks | reflectionDepth | event budget |
| Proc-loop | triggers recursivos | ciclo de eventos | provenance signature | maxTriggerDepth |
| EVA infinita | quase todo ataque erra | stacking linear | diminishing return | hit floor |
| SPD dominance | um lado joga sozinho | delay → 0 | asymptotic speed | min delay |
| One-shot | derrota sem decisão | burst alto | telegraph/counterplay | first-round guardrail |
| AoE spam | party inteira derrete | coordenação isolada | global cooldown/pressure | attack capacity |
| Item waste | IA cura full HP | threshold simples | expected utility | overheal penalty |
| Item hoard | nunca usa | scarcity exagerada | reserve policy | story-specific override |
| Hidden cheating | IA sabe tudo | usa true state | BeliefState | audit log |
| DDA percebido | jogo “rouba” | stat rubber band | explicit difficulty | bounded DDA |
| Boss sponge | 15 min repetitivo | HP sem mecânica | phases/puzzle | duration guardrail |
| Random AI | ações sem motivo | sorteio puro | weighted utility | telegraph |
| Deterministic AI | fácil explorar | argmax puro | softmax/top-k | filtered random |
| Duplicate debuff | 3 NPCs usam Silence | sem coordenação | reservation ledger | global tag cooldown |
| Overkill | 4 ultimates em 1 HP | plano simultâneo cego | simulated reservations | coordinator |
| Sim bug | harness ≠ jogo | fórmula duplicada | same domain code | parity tests |
| “0 stalls” falso | reprova por 1/240k | critério estatístico | CI threshold | rule-of-three |

---

# Apêndice B — conjunto inicial de parâmetros

**IMPORTANTE:** valores abaixo são *playtest seeds*, não recomendações canônicas.

```yaml
combat:
  max_rounds_safety: 40
  max_actions_safety: 500
  max_effect_events_per_action: 128
  max_trigger_depth: 16

timeline:
  speed_reference: 10
  speed_smoothing_k: 20
  min_speed_factor: 0.65
  max_speed_factor: 1.55
  party_ready_epsilon: 0.10

accuracy:
  equal_stats_hit: 0.90
  min_hit: 0.05
  max_hit: 0.99
  logistic_scale: 15

crit:
  min: 0.02
  max: 0.40
  multiplier: 1.50

targeting:
  kill_focus_threshold: 0.65
  utility_gap_focus_threshold: 0.20
  repeat_lambda: 0.35
  pressure_lambda: 1.25

stall:
  observe_window_rounds: 4
  warning_after_windows: 1
  pressure_after_windows: 2
  damage_step_per_round: 0.05
  heal_reduction_step_per_round: 0.05
  damage_cap_bonus: 0.50
  heal_reduction_cap: 0.50

cc:
  resolve_decay_per_action: 0.35
  hard_cc_immunity_opportunities: 1

revive:
  hp_fraction: 0.30
  timeline_delay_multiplier: 1.50
```

O harness deve varrer intervalos, não apenas estes pontos.

---

# Apêndice C — testes de propriedade

Além de exemplos, use property-based tests.

## C.1 Terminação

```text
Para todo estado legal e seed:
  battle termina antes de MAX_ACTIONS
  OU produz StallAbort controlado
```

## C.2 Itens

```text
quantity_after >= 0
consumption exactly once
illegal action cannot consume
```

## C.3 Timeline

```text
next_ready_time never decreases below current_time
dead actor never receives Ready
```

## C.4 Effect engine

```text
triggerDepth <= max
reflectionDepth <= max
event count <= max
```

## C.5 Target

```text
selected target ∈ legal targets
Firewall cannot make all targets permanently illegal
```

## C.6 CriticalActor

```text
if CriticalActor defeated:
    outcome == PartyLoss
```

## C.7 Preview

```text
preview math == resolution math
given forced RNG channel
```

## C.8 Knowledge

```text
ignorant NPC cannot score using hidden fact
```

Teste essencial:

```text
change hidden inventory
if NPC profile cannot know inventory:
    decision distribution must remain invariant
```

## C.9 Difficulty

```text
Hard may search deeper
but cannot access a fact denied by KnowledgeProfile
```

## C.10 Statistical regression

Guardar seeds que reproduzem bugs:

```text
stall_seed
gus_dogpile_seed
reflect_cycle_seed
revive_loop_seed
```

---

# Apêndice D — glossário

**ActionClock** — relógio contínuo/discreto que determina quando um ator fica pronto e quando cast resolve.

**Action economy** — quantidade e valor relativo das oportunidades de agir.

**BeliefState** — estado que contém o que a IA acredita, não o estado verdadeiro.

**CriticalActor** — ator cuja derrota satisfaz condição de derrota da equipe.

**DDA** — Dynamic Difficulty Adjustment.

**Focus fire** — concentração deliberada de ações em um alvo.

**Justified focus** — foco com benefício marginal tático alto.

**Tunnel vision** — repetição de alvo sem justificativa marginal.

**Pressure budget** — capacidade suave de dano/controle que pode convergir ao mesmo alvo em uma janela.

**Utility AI** — sistema que atribui score contextual a ações e seleciona entre elas.

**Expectimax** — árvore de decisão adversarial com nós de chance.

**MCTS** — Monte Carlo Tree Search.

**ISMCTS** — MCTS em conjuntos de informação.

**Softmax** — transformação de scores em distribuição probabilística.

**Stall** — estado de baixa ou nenhuma progressão por janela prolongada.

**Soft enrage** — pressão crescente gradual para terminar uma luta.

**Resolve** — mecanismo de resistência crescente a hard CC.

**Provenance** — linhagem de origem de um evento/efeito.

**Shadow cost** — custo de oportunidade estimado de consumir um recurso agora.

---

# Fechamento

Para o GusWorld, a prioridade não deveria ser criar uma IA “mais inteligente” em abstrato. O objetivo deve ser criar uma IA:

```text
racional o suficiente para ser tática,
limitada o suficiente para ser justa,
variável o suficiente para não ser resolvida,
legível o suficiente para ser aprendida,
e instrumentada o suficiente para ser balanceada.
```

O ponto arquitetural mais importante é que **targeting, defesa, cura, itens, action economy e condição de derrota do Gus formam um único sistema**. Se qualquer um deles for calibrado isoladamente, o harness pode aprovar números que desmoronam quando outra política muda. Os estudos internos de agosto de 2026 já demonstraram exatamente isso.



---

# Apêndice E — mapa comparativo de padrões de jogos

Este apêndice não afirma conhecer o código-fonte proprietário de jogos comerciais. Ele usa
**mecânicas observáveis como referência de design** e, quando fala de implementação de IA,
se apoia nas fontes técnicas da seção 50. A finalidade é responder à pergunta útil:
“que problema cada família de jogo resolveu bem, e que princípio vale transportar para um
sistema novo?”

| Referência | Padrão de interesse | O que transportar | O que não transportar automaticamente |
|---|---|---|---|
| *Chrono Trigger* | ritmo híbrido, iniciativa temporal, técnicas combinadas | relógio que cria janelas táticas e cooperação entre atores | copiar timings/valores sem considerar party 3–5 e AI moderna |
| *Pokémon* | prioridade, velocidade, troca, informação parcial, tipos | counters legíveis, custo de oportunidade e inferência do kit adversário | assumir que 1×1 é equivalente a combate de grupo |
| *Final Fantasy* clássico | ATB/CTB e papéis complementares | pressão temporal + identidade funcional | deixar SPD dominar a ponto de um lado monopolizar ações |
| *Final Fantasy XII* | automação configurável de aliados | regras do jogador como **bias de utility**, não IA autônoma opaca | gambits rígidos como único cérebro em situações combinatórias |
| *Shin Megami Tensei / Persona* | fraqueza ligada à economia de turnos | recompensar leitura correta do inimigo com ação/tempo, não só +dano | criar snowball de turno que não possua freios |
| *Octopath Traveler* | janela de quebra e preparação de burst | alternância setup → payoff; defesa/status como recurso temporal | transformar todo inimigo em puzzle de uma única solução |
| *Darkest Dungeon* | posição, proteção e papéis de controle | proteção como ação explícita, com efeito verificável, não “torcer para a IA atacar o tanque” | excesso de punição cumulativa em RPG com outra fantasia de produto |
| *Slay the Spire* | intenção inimiga altamente legível | telegraph como parte do jogo de informação | previsibilidade total para bosses cuja identidade exige incerteza |
| *XCOM / Gears Tactics* | coordenação de múltiplos agentes | reservar alvos/ações e controlar concentração de ameaça | permitir que coordenação perfeita pareça telepatia injusta |
| *Dragon Age: Inquisition* | Utility AI ação+alvo | avaliar ação e alvo em conjunto e registrar por que uma opção venceu | um score global sem normalização/curvas por contexto |
| *Kingdoms of Amalur* | coordenador de ataques | `attack capacity`/pressure budget para impedir dogpile sem “proibir foco” | hard cap invisível que invalida uma vendeta ou execução tática legítima |
| *Final Fantasy XV* | combinação de BT, posicionamento e decisões contextuais | separar decisão estratégica de execução | misturar animação/locomoção com avaliação tática no mesmo algoritmo |

## E.1 Chrono Trigger: o princípio útil não é “ATB porque é clássico”

O ponto relevante para um sistema híbrido é que **tempo pode ser recurso tático sem exigir
ação em tempo real**. Para o GusWorld, isso sugere:

```text
ready_time
cast_time
recovery_time
interrupt_window
reorder_delta
```

A UI pode pausar na escolha humana e continuar simulando a ordem discretamente. Assim, o
jogador raciocina sobre uma linha temporal legível, mas SPD, Haste, Slow, Knockback,
cast lento e ações duplas continuam tendo significado.

A implicação algorítmica é importante: o cérebro não pergunta apenas “qual ação dá mais
dano?”, mas:

```text
qual ação produz maior valor
considerando quando ela começa,
quando resolve,
quem age antes,
quem pode interromper,
e qual estado provavelmente existirá na resolução?
```

Isso é uma forma de utilidade temporal. Não exige MCTS para todos os inimigos. Um elite
pode estimar apenas o próximo evento relevante.

## E.2 Pokémon: a principal lição é informação parcial + counters

O formato clássico 1×1 é muito diferente de 3–5×1–6, mas três princípios generalizam:

1. **tipos e resistências são legíveis**;
2. **velocidade/prioridade alteram a ordem e o risco**;
3. o adversário frequentemente precisa agir sem saber todo o kit futuro.

No GusWorld, a terceira lição é especialmente valiosa. Um NPC comum não deve consultar:

```text
player.inventory
player.hidden_cards
future_player_action
exact_unknown_resistance
```

Ele consulta `BeliefState`. Um mestre pode conhecer um catálogo inteiro e ainda assim não
saber qual carta está fisicamente disponível se não houver justificativa narrativa.

Esse detalhe impede um erro frequente em RPGs com IA “boa”: a inteligência deixa de parecer
boa e passa a parecer **leitura de memória**.

## E.3 Final Fantasy XII: automação de aliado deve ser controlável

O melhor uso da ideia de gambits aqui não é reproduzir uma lista de `if`s. É deixar o jogador
definir **preferências de política** que entram no UtilityBrain.

Exemplo:

```text
PROTECT_GUS:
    U(GuardRedirect Gus) += 40
    U(Heal Gus)          += 25

SAVE_ITEMS:
    ItemShadowCost *= 2.0

FINISH_WEAK:
    ExecuteUtility *= 1.4
```

Com isso, o aliado continua sabendo lidar com casos que o jogador não antecipou, mas a sua
“personalidade tática” é dirigida pelo jogador.

## E.4 Persona/SMT e Octopath: counters devem alterar economia de ações

Uma fraqueza que apenas transforma `100` em `150` tende a ser uma equação de DPS. Uma
fraqueza que também produz:

```text
delay
interrupt
break window
mana efficiency
setup token
```

altera a decisão.

Para GusWorld, não é necessário copiar sistemas de “turno extra”. O princípio geral é:

> informação correta deve gerar **vantagem tática qualitativa**, não apenas um multiplicador.

Scan, Gambito, Expose e timeline já oferecem caminhos para isso.

## E.5 Darkest Dungeon e a defesa do Bento

O achado interno mais útil da pesquisa do GusWorld é que “Defender” e “ser tanque” não podem
depender da mesma variável.

Separar:

```text
Mitigation:
    Shield / Defend / armor

Aggro manipulation:
    Honeypot / Taunt / vendetta weight

Protection:
    GuardRedirect / intercept

Avoidance:
    Firewall / Decoy
```

faz o papel de Bento funcionar mesmo quando a IA é racional e percebe que bater no escudo é
ruim.

Isso também torna o balanceamento mais auditável:

```text
Bento mitigou X
Bento redirecionou Y
Honeypot alterou probabilidade Z
```

em vez de inferir “tankiness” a partir de quem por acaso recebeu golpes.

## E.6 Slay the Spire: telegraph é parte do algoritmo

`IntentPreview` não deve ser um texto gerado depois que a IA escolheu. Deve ser um **contrato**
entre decisão e apresentação:

```text
DecisionToken {
    action_id
    target_id
    expected_range
    intent_class
    confidence
    hidden_components
}
```

A UI mostra o subconjunto permitido.

Se a IA recalcular silenciosamente depois que o jogador reagiu ao intent, o telegraph é falso.
Logo:

```text
preview -> reservation/commit -> player response -> resolve
```

ou, se o inimigo puder mudar:

```text
preview says "adaptive"
and the rule explaining adaptation is visible.
```

## E.7 Dragon Age: Inquisition: ação e alvo são uma única decisão

A literatura de produção do BDS é particularmente pertinente porque o problema “qual habilidade?”
e “em quem?” explode combinatoriamente em party combat.

Em vez de:

```text
bestAction = choose_action()
target = choose_target(bestAction)
```

use:

```text
bestCandidate = argmax Utility(action, target, state, beliefs)
```

Exemplo:

```text
Heal Jaci     = 28
Heal Gus      = 81
Potion Jaci   = 11
Potion Gus    = 57
Attack A      = 36
Attack B      = 43
Silence MageB = 76
```

A ação “Heal” não tem valor independente de alvo. O mesmo vale para Taunt, Firewall,
revive, dispel e execução.

## E.8 Kingdoms of Amalur + Gears Tactics: coordenação coletiva

Quando seis inimigos calculam utilidade isoladamente, todos podem chegar à mesma conclusão
correta:

```text
"Gus é o melhor alvo."
```

Se cada um estiver certo localmente, o resultado global ainda pode ser ruim para o produto.

Por isso existe uma camada acima do brain:

```text
EncounterDirector
```

Ela não precisa mandar “ataque Cauã”. Ela mantém:

```text
reservations
pressure ledger
AoE overlap
CC reservations
overkill estimate
global cooldown tags
```

A distinção é:

```text
Brain:
    "o que EU gostaria de fazer?"

Director:
    "quanto dessa intenção o encontro comporta agora?"
```

O foco continua possível. A coordenação só impede que seis decisões localmente ótimas produzam
um dogpile involuntário.

## E.9 Quando quebrar deliberadamente o pressure budget

Há situações em que o jogo **deve** concentrar fogo:

```text
1. Gus está em estado de execução e o inimigo sabe a condição crítica;
2. vendeta pré-declarada;
3. boss tem fase "hunt";
4. alvo canaliza ação que encerra a luta;
5. healer está prestes a reviver dois aliados;
6. oportunidade de lethal tem alta confiança;
7. taunt/honeypot do jogador pede concentração.
```

Nesse caso:

```text
JustifiedFocus = true
```

e o custo de pressão é reduzido ou ignorado.

O que se evita não é foco. É foco **sem justificativa marginal**.

---

# Apêndice F — falhas de implementação e invariantes adicionais

Esta lista complementa o Apêndice A. São bugs que aparecem quando o design conceitual está
correto, mas a implementação temporal, o RNG ou a coordenação não estão.

## F.1 Target invalidation

Problema:

```text
NPC A reserva ataque em Gus
NPC B age antes e incapacita Gus
NPC A tenta atacar ator inválido
```

Política obrigatória por ação:

```text
RetargetPolicy =
    Fail
    RetargetSameClass
    ReevaluateUtility
    ConvertToFallback
```

Não deixe cada habilidade improvisar.

Para ataques comuns:

```text
ReevaluateUtility
```

é geralmente seguro.

Para ultimate telegrafado:

```text
Fail or hit reserved location
```

pode preservar counterplay.

## F.2 Overkill por decisões simultâneas

Sem reserva:

```text
enemy1 predicts target HP 10 -> attacks for 15
enemy2 predicts target HP 10 -> attacks for 20
enemy3 predicts target HP 10 -> attacks for 18
```

Todos escolhem corretamente no snapshot.

Use:

```text
ReservedDamage[target] += ExpectedDamage(candidate)
effectiveProjectedHp =
    currentHp - ReservedDamage[target]
```

e penalize:

```text
OverkillWaste =
    max(0, ExpectedDamage - effectiveProjectedHp)
```

A exceção é quando overkill produz benefício secundário explícito.

## F.3 Duplo dispel / duplo Silence

O mesmo problema existe com status:

```text
ReservedTags:
    SILENCE(target)
    DISPEL(target)
    BREAK(target)
```

Segundo NPC ainda pode usar o mesmo status se:

```text
refresh valuable
stack rule permits
first cast may fail
different duration matters
```

mas precisa justificar.

## F.4 RNG consumido por preview

Um bug grave:

```text
previewDamage() -> chama RNG
```

Então abrir tooltip altera o futuro da luta.

Invariante:

```text
Preview functions are PURE.
```

Teste:

```text
seed S
state hash H
call preview N times
resolve
result must equal resolve without previews
```

O repositório já segue esse princípio em partes; deve virar invariável global.

## F.5 RNG correlacionado entre atores

Evite um único stream opaco quando debugging ficar difícil.

Alternativas:

```text
BattleSeed
    -> ActionRng(actionSequenceId)
    -> EffectRng(effectInstanceId)
```

ou manter stream único, mas logar:

```text
rng_index
roll_kind
roll_value
```

O requisito é **replay exato**, não necessariamente múltiplos PRNGs.

## F.6 Tie-breaking não determinístico

Se dois candidatos têm utility igual e a iteração vem de hash container, builds/plataformas
podem divergir.

Defina:

```text
stableCandidateId
stable target order
stable action order
```

antes de aplicar ruído seedado.

## F.7 Estado morto atuando

Toda transição deve provar:

```text
dead actor cannot:
    gain ready time
    act
    receive normal heal unless revivable
    trigger living-only passive
```

Mas efeitos `on_death` precisam ser processados exatamente uma vez.

Use estado explícito:

```text
Alive
Incapacitated
Dead
Removed
```

não apenas `hp <= 0`.

## F.8 Spawn infinito

Summons + revive + on-death summon podem criar crescimento sem limite.

Invariantes:

```text
max_active_entities_per_side
max_summons_per_source
summon_budget_per_encounter
event_budget_per_action
```

Chefes podem ultrapassar limites normais por phase spec explícita.

## F.9 DoT e HoT com ordem ambígua

Defina uma ordem única:

```text
TurnStart:
    expire pre-start locks?
    DoT
    HoT
    resource regen
    cooldown tick
    ready effects
```

ou outra, mas documente.

Caso crítico:

```text
HP = 3
Poison = 5
Regen = 5
```

O resultado não pode depender de ordem de `std::vector`.

## F.10 Snapshot vs. dynamic resolution

Cast lento iniciado contra alvo com Def 10. Antes de resolver, Def vira 30.

Escolha por propriedade da ação:

```text
SnapshotAtCast:
    accuracy?
    base power?
    source stats?

DynamicAtResolve:
    target defense?
    current weakness?
    current shield?
```

Não deixe cada efeito decidir sem contrato.

Para o GusWorld, uma regra geralmente legível é:

```text
source commitment snapshot at cast
target defenses/status evaluated at resolve
```

porque cria counterplay durante a janela de cast.

## F.11 Cancelamento e reembolso

Ao interromper:

```text
AP?
mana?
item?
battery?
cooldown?
```

precisam de política independente.

Modelo:

```text
CostTiming =
    OnDeclare
    OnCommit
    OnResolve

RefundPolicy =
    None
    Partial
    Full
```

Item físico usado/aberto pode ser `OnCommit`; mana pode variar por família.

## F.12 Hard CC sobre ator crítico

Stun-lock de Gus equivale quase a derrota mesmo sem HP zero.

Além de `Resolve`, medir:

```text
CriticalActorActionDenialRate
```

por luta.

Guardrail inicial para playtest:

```text
p95 consecutive denied actions <= threshold
```

O threshold é de sensação, não deve ser canonizado sem teste.

## F.13 Cura gera threat e cria ciclo perverso

Se F3 “healer” e healing recente aumentam threat, pode surgir:

```text
Jaci cura Gus
-> Jaci vira alvo
-> Gus precisa protegê-la
-> Jaci cura a si
-> threat cresce
```

Isso pode ser bom, mas deve saturar.

Exemplo:

```text
healThreat =
    W * saturate(recentEffectiveHeal / referenceHeal)
```

não:

```text
W * totalHealingEver
```

## F.14 Focus penalty que protege o jogador artificialmente

Erro oposto ao tunnel vision:

```text
repeatPenalty muito forte
```

faz IA recusar lethal óbvio só porque já atacou o mesmo alvo.

Regra:

```text
if lethalConfidence >= lethalThreshold:
    repeatPenalty *= lethalRepeatScale  # perto de zero
```

ou `JustifiedFocus`.

## F.15 “Fairness” que o jogador aprende a explorar

Se o jogador descobre que:

```text
"o jogo nunca deixa 3 inimigos mirarem Gus"
```

pode deixar Gus exposto deliberadamente.

Por isso `TargetPressureBudget` deve ser:

- suave;
- contextual;
- quebrável por justificativa;
- não anunciado como lei rígida.

## F.16 Difficulty alterando conhecimento indevidamente

Hard não ganha acesso automático a campos ocultos.

Teste metamórfico:

```text
same NPC KnowledgeProfile
same observed history
Easy vs Hard

Hard may rank legal known actions better,
but both must expose the same known facts.
```

## F.17 DDA contaminando experimento

Se DDA roda durante harness de balanceamento, você mede um controlador que muda os parâmetros
enquanto mede.

Harness canônico:

```text
DDA = OFF
```

Depois existe um braço separado:

```text
DDA = ON
```

## F.18 Save/load muda a batalha

Se o jogador salvar durante combate:

```text
save
load
```

deve preservar:

```text
RNG state
action clock
reservations
knowledge beliefs
cooldowns
effect provenance
boss phase
pressure ledger
pending intents
```

ou a feature precisa declarar que não existe save mid-battle.

## F.19 Preview e intent falsos por replanejamento

Se um inimigo anuncia:

```text
ATTACK GUS
```

e recalcula após qualquer clique do jogador, o intent vira desinformação.

Use:

```text
CommitStrength:
    HardCommit
    ConditionalCommit(predicate)
    Adaptive
```

Exemplo:

```text
ConditionalCommit:
    attack Gus unless Gus becomes untargetable
```

A UI pode mostrar o gatilho.

## F.20 Informação perfeita em autobattle escondendo problemas humanos

Bot ótimo pode concluir que um encontro está “balanceado” porque explora:

```text
fraqueza exata
timing perfeito
zero desperdício
memória total
```

Mantenha múltiplas personas, inclusive uma que:

- não conhece fraqueza até Scan;
- usa item com atraso;
- erra prioridade;
- protege Gus excessivamente;
- joga conservador;
- joga aleatório limitado.

A aprovação é uma **região de comportamento**, não a win-rate de um bot.

## F.21 Propriedades globais recomendadas

As seguintes propriedades merecem testes automatizados permanentes:

```text
P1  batalha sempre termina ou aciona StallGuard
P2  nenhum efeito gera recursão ilimitada
P3  nenhuma entidade age duas vezes pelo mesmo ticket temporal
P4  ator removido nunca é alvo legal
P5  preview não consome RNG
P6  decisão não usa informação negada pelo KnowledgeProfile
P7  Hard não lê ação futura do jogador
P8  quantidade de itens nunca fica negativa
P9  revive não duplica ator
P10 dano reservado nunca altera HP antes da resolução
P11 IntentPreview corresponde à política de commit
P12 seed + input sequence reproduzem outcome bit-a-bit
P13 pressure budget nunca proíbe JustifiedFocus quando override explícito está ativo
P14 utility NaN/Inf invalida candidato e gera diagnóstico
P15 todo candidato possui razão auditável de inclusão/exclusão
```

## F.22 Ordem prática para implementação segura

Antes de sofisticar a IA:

```text
1. invariantes + replay determinístico
2. loop guards
3. targeting ponderado real no call site
4. reservations / pressure ledger
5. guard redirect / taunt / firewall
6. item + healing utility
7. UtilityBrain completo
8. KnowledgeModel
9. difficulty policies
10. ActionClock/cast-time
11. bosses/search seletivo
12. DDA opcional
```

Esta ordem reduz um risco comum: adicionar “inteligência” em cima de um simulador que ainda
permite loops, invalid targets ou divergência entre preview e resolução.

<!--
SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
SPDX-License-Identifier: AGPL-3.0-or-later
-->

# Especificação do formato `.gw.card`

**Tipo Diátaxis:** Reference. **Audiência:** quem vai escrever o leitor/parser deste formato (`backend-engineer`, `content/` layer, L-17), e quem vai escrever ou revisar um arquivo `.gw.card` novo. **Versão do produto:** nenhuma ainda — formato nasce nesta tarefa, sem implementação de leitor. **Last-reviewed:** 25/08/2026. **Owner:** `technical-writer`, a pedido do orquestrador; qualquer mudança de forma passa pelo mesmo canal que aprovou esta.

> **Por que este documento existe:** o `.gw.text` (`resources/dialogues/*.gw.text`) foi ao ar sem especificação escrita, e o próprio arquivo-fonte registra isso como falha ("A ESPECIFICAÇÃO DESTE FORMATO AINDA NÃO EXISTE ESCRITA... escrevê-la é item próprio da tabela"). Este documento nasce **junto** com o primeiro uso do `.gw.card`, para não repetir o buraco.

## 1. O que é

`.gw.card` é o formato-fonte, próprio da casa (`docs/tech/convencao-formatos-gw.md`), para especificar os dados mecânicos de uma carta do GusWorld. **Fonte de build, não formato de runtime**: existe em texto dentro do repositório para ser lido, revisado e traduzido por humano; o jogo nunca lê `.gw.card` diretamente. Um gerador (ainda não escrito) compila estes arquivos no catálogo binário selado que a L-18/L-25 exigem. Nenhum código de leitura ou de gravação deste formato existe hoje (L-01): este documento descreve o **contrato**, não uma implementação.

**Desenhado para servir a mais que carta.** O líder já avisou que o formato vai reger outros tipos de conteúdo ("isso será estendido depois para todos os itens"). Por isso a seção 3 (blocos `@effect`) não assume nada específico de "carta" — um item de inventário com um efeito ao usar cabe na mesma gramática. A seção 4 é que é hoje específica de carta (família, forma de alvo); um tipo de conteúdo futuro pode precisar de uma seção 4 diferente, com o mesmo esqueleto de comentário + `#meta` + blocos.

## 2. Gramática geral

O formato tem quatro elementos de sintaxe, e só quatro:

| Elemento | Sintaxe | Onde vale |
|---|---|---|
| Comentário | `//` até o fim da linha | em qualquer linha, sozinho ou depois de conteúdo |
| Metadado de topo | `#meta <chave> <valor>` | antes do primeiro bloco `@` |
| Bloco | `@<nome>` inicia um bloco; vai até o próximo `@` ou o fim do arquivo | corpo do arquivo |
| Campo de bloco | `<chave>: <valor>` (dois-pontos) | dentro de um bloco |

Regra de espaço: `#meta` separa chave e valor por **espaço** (sem dois-pontos); campo de bloco separa por **dois-pontos**. A distinção não é estética — é o que permite ao leitor saber, sem olhar contexto, se uma linha é metadado de topo ou campo de bloco. Esta gramática espelha, propositalmente, a do `.gw.text` já em uso (`#meta dialogue_id x`, `speaker: bertoldo`), para não introduzir uma segunda convenção de sintaxe na casa.

Linha em branco: ignorada. Linha só com comentário: ignorada. Espaço em branco no fim de linha: ignorado.

## 3. Comentário

**A exigência que define o formato** (decisão do líder, 25/08/2026): todo número tunável carrega, na mesma linha ou na linha imediatamente acima, o comentário que diz de onde ele veio. Onde o código-legado marcava `//PLAYTEST` (número testado em jogo, mas não fechado por afinação formal), o comentário aqui carrega a mesma marca, textualmente, para não perder a distinção entre "chutado" e "jogado".

Três formas válidas, todas aceitas pelo leitor:

```
mana_cost: 6   // kActiveManaCost, //PLAYTEST: mana das ativas/hibridas
```

```
// kNewtonReflectDuration, //PLAYTEST: 3 turnos
duration: 3
```

```
// origem não documentada
duration: 2
```

**Quando não há razão escrita no legado**, o comentário diz isso com todas as letras: `origem não documentada`. Nunca se inventa uma razão para preencher a lacuna (ordem da tarefa que criou este formato).

## 4. `#meta`: identificação e números-base da carta

Bloco implícito no topo do arquivo, antes de qualquer `@`. Uma chave por linha.

| Chave | Obrigatório | Valor | Nota |
|---|---|---|---|
| `id` | sim | string, minúscula, o identificador único da carta (ex.: `newton`, `pulso_eletrico`) | é o nome do arquivo sem a extensão |
| `display_name` | sim | chave de tradução (ex.: `CARD_EXEC_NEWTON_NAME`) | resolvida por `tr()` na UI, nunca hardcoded; se a chave de tradução não existe hoje no catálogo, o valor é a string literal `SEM_CHAVE_DE_TRADUCAO` e um comentário explica |
| `tier` | sim | `comum` \| `especial` \| `super` | vocabulário em `_vocabulario.md` |
| `category` | obrigatório quando `tier` ≠ `comum` | `ativa` \| `passiva` \| `hibrida` \| `fora_de_combate` | sem sentido para `comum`; se presente numa `comum`, o leitor **rejeita o arquivo** (campo não pertence a este tier) |
| `family` | sim | `eletrico` \| `bioquimico` \| `sonico` \| `cinetico` \| `criptografico` \| `universal` | `universal` só é válido para carta, nunca para personagem/inimigo (regra herdada da fonte, não deste formato) |
| `base_type` | sim | `pulso` \| `raiz` \| `eco` \| `fenda` \| `glifo` | tipo diegético; nas 20 especiais sempre `glifo`, marcado `//PLAYTEST` na fonte por não haver convenção fechada |
| `target_shape` | sim | `self` \| `single` \| `linha` \| `area3x3` \| `grupo` | |
| `mana_cost` | sim | inteiro ≥ 0 | |
| `ap_cost` | sim | inteiro ≥ 0 | |
| `power` | sim | inteiro ≥ 0 | `0` quando a carta não tem dano-base próprio (a maioria das especiais); comentário explica a exceção quando `power` > 0 |
| `ignores_weakness_wheel` | não (default `false`) | `true` \| `false` | trunfo fora-da-roda; hoje só a Gödel usa `true` |

## 5. `@effect`: o programa da carta (zero ou mais)

Um bloco `@effect` por instrução do programa da carta. Zero blocos é válido (carta `fora_de_combate` posse-only, ou uma `hibrida` cujo dano vem só de `power`, como a Maxwell). A **ordem dos blocos no arquivo é a ordem de registro no programa**; um leitor não reordena.

| Campo | Obrigatório | Valor | Nota |
|---|---|---|---|
| `trigger` | sim | `OnCast` \| `OnDamageDealt` \| `OnDamageReceived` \| `OnAllyTurnEnd` \| `OnRoundEnd` \| `Always` | quando o efeito dispara |
| `kind` | sim | um dos 14 tipos em `_vocabulario.md` §4 | o que o efeito faz |
| `magnitude` | não (default `0`) | inteiro | significado depende de `kind`; o comentário ao lado diz qual |
| `percent` | não (default `0`) | inteiro, 0-100 salvo nota em contrário | idem |
| `duration` | não (default `0`) | inteiro (turnos) | idem |
| `status` | não | um `StatusId` de `_vocabulario.md` §5, ou ausente | só relevante quando `kind` aplica status |
| `stack_rule` | não (default `Replace`) | `Replace` \| `Refresh` \| `StackMagnitude` \| `StackDuration` | |
| `side_filter` | não (default `Any`) | `Any` \| `EnemyOnly` \| `AllyOnly` | |

## 6. `@status_applied`: a carta COMUM (no máximo um)

As 5 cartas COMUNS não passam pelo executor de programa (`@effect`); elas aplicam, no máximo, **um** status fixo. Bloco distinto, para não fingir que uma COMUM roda o mesmo mecanismo de uma ESPECIAL — são caminhos de código diferentes na fonte, e o formato preserva essa diferença em vez de escondê-la atrás de uma sintaxe única.

| Campo | Obrigatório | Valor |
|---|---|---|
| `status` | sim | um `StatusId` |
| `magnitude` | sim | inteiro |
| `duration` | sim | inteiro (turnos) |
| `stack_rule` | sim | `Replace` \| `Refresh` \| `StackMagnitude` \| `StackDuration` |
| `family_origin` | sim | a família de `_vocabulario.md` §2 |

Uma carta `comum` tem **no máximo um** bloco `@status_applied` (zero é válido — nenhuma das 5 do legado tem zero, mas o campo é opcional no record de origem). Duplicidade é erro de leitura.

## 7. Campo desconhecido, chave repetida, chave ausente: **falha, nunca silêncio**

Decisão de desenho desta tarefa, não fato herdado do legado (que não tinha este formato): o leitor deste formato segue a mesma disciplina que o resto do projeto já pratica em outros gates (L-19, lição 3: *"gate que não consegue medir tem de FALHAR, nunca aprovar em silêncio"*).

- **Chave de `#meta` ou de bloco não listada nas tabelas acima:** o leitor **rejeita o arquivo inteiro**, com o nome do arquivo, a linha e a chave desconhecida na mensagem. Nunca ignora e segue.
- **Chave obrigatória ausente:** rejeita, nomeando a chave que falta.
- **Chave repetida dentro do mesmo bloco (ou no `#meta` de topo):** rejeita — é sinal de edição manual inconsistente, não de intenção.
- **Bloco desconhecido (`@algo` fora de `@effect`/`@status_applied`):** rejeita.
- **Valor fora do vocabulário** (ex.: `family: Fogo`, que não existe): rejeita, citando o vocabulário aceito.

Nenhuma dessas falhas é ambígua o bastante para "melhor esforço": um catálogo de conteúdo errado que passa calado é exatamente o cenário que a L-18 (proteção contra fraude/edição) e a L-19 (gate que aprova em silêncio) existem para impedir.

## 8. Exemplo completo, anotado

```
// newton.gw.card
//
// Force-Law (Newton): Hibrida, Universal, Grupo. Duas faces, roteadas por
// side_filter. Ver docs/design/mecanicas/cartas/newton.md para o "porque"
// completo (o comentario aqui so ancora o numero ao lado dele).

#meta id newton
#meta display_name CARD_EXEC_NEWTON_NAME
#meta tier especial
#meta category hibrida
#meta family universal
#meta base_type glifo
#meta target_shape grupo
#meta mana_cost 6   // kActiveManaCost, //PLAYTEST: mana das ativas/hibridas
#meta ap_cost 1
#meta power 0
#meta ignores_weakness_wheel false

// Face ofensiva: imobiliza o grupo inimigo. Dissipa se o alvo for aliado.
@effect
trigger: OnCast
kind: ApplyStatus
duration: 1   // //PLAYTEST: 1 turno de Stun
status: Stun
stack_rule: Replace
side_filter: EnemyOnly

// Face de beneficio: concede Reflect a um aliado. Dissipa se o alvo for inimigo.
@effect
trigger: OnCast
kind: ApplyStatus
magnitude: 30   // kNewtonReflectPercent, //PLAYTEST: fracao do Reflect
duration: 3     // kNewtonReflectDuration, //PLAYTEST: 3 turnos
status: Reflect
stack_rule: Refresh
side_filter: AllyOnly

// Passiva propria do dono: reflete dano recebido. Se o dono TAMBEM tiver o
// status Reflect acima, as duas fontes SOMAM (nao ha dedup no motor).
@effect
trigger: OnDamageReceived
kind: Reflect
percent: 30   // kNewtonReflectPercent, //PLAYTEST: mesma fracao da face 2
```

## 9. O que este documento decidiu, e contra qual critério

Onde o número fica (`.gw.card`, curto, ao lado do valor) contra onde a prosa de design fica (`.md` por carta, longo, explicando o mecanismo inteiro): o critério usado foi **tamanho + reusabilidade**. Uma frase de uma linha que ancora um número (`kActiveManaCost, //PLAYTEST`) cabe e pertence ao dado, porque é parte do próprio dado (sem ela, o número é órfão). Um parágrafo que explica por que duas faces de uma carta existem, o que cada uma faz, e que decisão do líder ou achado de auditoria motivou a forma atual — isso é ensaio, não anotação, e enche o arquivo de dado de texto que um leitor de máquina jamais precisa. Cada `.gw.card` termina o comentário de cabeçalho apontando para o `.md` irmão (L-30); nenhum arquivo de dado repete o ensaio nem faz o leitor humano caçá-lo.

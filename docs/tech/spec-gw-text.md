<!--
SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
SPDX-License-Identifier: AGPL-3.0-or-later
-->

# Especificação do formato-fonte `.gw.text` (1 de 2)

> **Status:** canônico, item `B9` da tabela de pendências (metade 1 de 2 — a outra metade, o pacote
> binário em que este formato é compilado, é `spec-catalogo-dialogo-binario.md`, e as duas se
> definem em conjunto por decisão do líder de 24/08/2026).
>
> **Derivado de:** os quatro arquivos reais em `resources/dialogues/` (`cena15_m1_pergunta.gw.text`,
> `cena15_m2_intervalo.gw.text`, `cena15_m3_coda.gw.text`, `npc_intro_bertoldo.gw.text`) e do
> exemplo comentado em `docs/narrative/propostas/cena15-dlgtxt-PILOTO-PROPOSTA.md` (proposta, não
> canon, mas fonte da convenção de chave já em uso). Nada aqui foi inventado além do que os quatro
> arquivos praticam; o que os arquivos não cobrem está nomeado no §10, não preenchido por adivinhação.
>
> **Endossado por:** `docs/tech/contrato-i18n.md` (item `D20`, convenção de chave §4, `B9` depende
> dele) e `docs/tech/convencao-formatos-gw.md` (item `G2`, convenção `.gw.<tipo>`).
>
> **O achado que resolve a aparente contradição com a L-18, dito aqui antes de qualquer outra
> coisa, para não se perder:** um arquivo `.gw.text` **nunca contém prosa**. Todo campo visível ao
> jogador — a fala de um nó, o rótulo de uma escolha — é sempre uma **chave i18n**, nunca um valor
> literal. A prosa vive só nos catálogos de tradução (`pt_br.md`, `en_intl.md`), fora deste
> formato. É isso, e só isso, que faz "formato de texto" no nome do item conviver com a lei que
> proíbe texto chegando ao jogador (§2 detalha a prova, medida nos quatro arquivos reais).
>
> **Por que este documento não buscou fora do projeto:** o padrão "motor fixo + conteúdo como dado
> atômico, grafo de nós com id, transição e flag" já está canonizado internamente em
> `docs/tech/adr/ADR-019-arquitetura-conteudo-atomica-data-driven.md`, que cita este EXATO desenho
> como já em produção (*"Diálogo runtime POCO: grafo `DialogueGraph`... interpretado por um
> parser+runtime fixo"*). Buscar um padrão de terceiro arriscaria reintroduzir o interpretador/VM
> genérico que o `ADR-016`/`ADR-019` já rejeitaram explicitamente para este projeto. A gramática
> abaixo é derivação de canon interno já aprovado, não pesquisa externa — para o próximo agente não
> refazer a busca nem achar que ela foi esquecida.

---

## 1. O que este documento é, e o que não é

**É:** a gramática do arquivo-fonte `.gw.text` — o grafo de diálogo ramificado, em texto, mantido
dentro do repositório para revisão, controle de versão e tradução.

**Não é:**

- o pacote binário selado que chega à máquina do jogador (`spec-catalogo-dialogo-binario.md`);
- o contrato de i18n em si — convenção de chave, fallback, plural/gênero (`contrato-i18n.md`, `D20`);
  este documento só **usa** essas regras, não as redefine;
- decisão de interface — nenhuma tela de diálogo existe ainda (`present/` bloqueada, L-06).

---

## 2. Onde este formato vive, e por que o nome do item não contradiz a L-18

`.gw.text` fica em `resources/dialogues/*.gw.text`: **fonte de build**, nunca artefato de
distribuição (L-25: *"Fonte do conteúdo: texto estruturado dentro do repositório, lido apenas pelo
gerador em tempo de build. Nada em texto na distribuição"*). O item `B5` já fechou, em 24/08/2026,
que não há tensão real entre "formato de texto" no nome do item e a L-18: a proibição é sobre o
que **chega ao jogador**, não sobre a fonte mantida em texto no repositório.

**O ponto central que resolve a aparente contradição, e que se mede nos quatro arquivos reais, não
se presume:** `.gw.text` **nunca contém prosa**. Todo campo visível ao jogador — a fala, o rótulo
de uma escolha — é uma **chave i18n**, nunca um valor literal. Nas quatro fontes existentes, nenhuma
linha carrega a fala em si; carrega só o identificador da chave (`text: DIALOGUE_CENA15_M1_N07_LEITURA`,
nunca `text: "Cauã lê por cima do ombro dele."`). A prosa mora exclusivamente nos catálogos de
tradução (`resources/translations/pt_br.md`, `en_intl.md`), fora deste formato.

**Consequência que atravessa o resto deste documento:** `.gw.text` já nasce locale-agnóstico.
Acrescentar um idioma nunca toca um arquivo `.gw.text` — cumpre, de graça, o princípio central do
`contrato-i18n.md` §1 (*"acrescentar idioma é acrescentar dado, nunca mudar formato"*), porque o
locale nem aparece aqui.

---

## 3. Estrutura léxica geral

- Codificação UTF-8. Uma instrução por linha, salvo o bloco de linhas de escolha (§7.2).
- Comentário: `// texto livre até o fim da linha`. Sem comentário de bloco.
- Linhas em branco são ignoradas e servem só de espaçamento visual entre nós.
- Nenhuma linha de comentário é verificada por máquina hoje — o cabeçalho do §4 é convenção de
  revisão humana, não sintaxe validada pelo gerador (o gerador valida o que o §10 lista).

---

## 4. Cabeçalho de comentário (convenção obrigatória de revisão)

Os quatro arquivos existentes seguem, sem exceção, o mesmo conjunto de campos no bloco de
comentário que abre o arquivo. Esta especificação os torna **obrigatórios por convenção de
revisão** — um `.gw.text` sem eles reprova revisão, ainda que o gerador (que não lê prosa de
comentário) não recuse compilar:

| Campo | Conteúdo |
|---|---|
| Identificação | nome do arquivo, cena/movimento, uma linha de contexto |
| Fonte canônica | documento de origem da fala, se a cena vier de prosa aprovada em outro lugar |
| `REGISTRO` | qual registro é o default e onde um nó diverge dele |
| `GATILHO` | condição de disparo, em prosa — **deliberadamente fora da gramática**, ver §10 |
| `FLAG(S) ESCRITA(S)` | lista resumida do que o grafo grava, para quem lê sem parsear |
| Regra de ferro / nota de encenação | qualquer restrição narrativa que a cena exige (ex.: "ninguém comenta") |

---

## 5. Diretivas `#meta`

Três diretivas, obrigatórias, uma vez cada por arquivo, em qualquer ordem (por convenção, logo
após o cabeçalho de comentário):

```
#meta dialogue_id <id>
#meta default_register <id>
#meta entry <node_id>
```

- **`dialogue_id`**: snake_case, único no projeto inteiro (§10 do gerador exige isso). Raiz da
  convenção de chave i18n (`DIALOGUE_<DIALOGUE_ID>_<NODE_ID>`, `contrato-i18n.md` §4.1) e do
  namespace de flag (§8).
- **`default_register`**: registro aplicado a todo nó que não sobrescrever com `register:`.
  **Lista fechada, decisão do líder:** `{warm, terminal}` — `warm` para personagem humano (caixa
  quente + retrato), `terminal` para narração e telas de sistema. Um terceiro registro só nasce por
  pedido novo ao líder, nunca por um `.gw.text` inventando um id ad-hoc. Um `register`/
  `default_register` fora desta lista é **erro de geração** (§10 do gerador, `spec-catalogo-dialogo-binario.md`
  §4), nunca aviso silencioso — mesma disciplina de vocabulário fechado que o `ADR-019` já aplica a
  todo enum de conteúdo do projeto.
- **`entry`**: aponta para o `node_id` de entrada. Único ponto de partida do grafo — não há entrada
  condicional (§10).

---

## 6. Nós (`@node`)

```
@node <node_id>
speaker: <speaker_id>
text: <CHAVE_I18N>
register: <id>            // opcional
on_enter: <flag_expr>     // opcional
-> <node_id ou @exit>
```

- **`node_id`**: snake_case, com prefixo numérico de dois dígitos por convenção (`n01`, `n02`, ...
  nunca `n1`) — existe só para ordenação alfabética do arquivo fonte; a ordem de jogo real vem dos
  links `->`, nunca da numeração (`contrato-i18n.md` §4.3).
- **`speaker`** (obrigatório): referencia um personagem (`CHARS.md`) ou um id de sistema (hoje só
  `terminal`, usado para narração/log). **Decisão do líder: o gerador confere `speaker` contra o
  inventário de personagens.** `speaker` que não é nem uma entrada de `CHARS.md` nem um id de
  sistema da allowlist (hoje só `terminal`) é **erro de geração**, não erro de runtime — nome
  errado passa a quebrar a build, nunca chega ao jogo. **Consequência de fluxo de trabalho que esta
  decisão implica, e que fica escrita para não ficar subentendida: todo personagem novo precisa
  estar cadastrado em `CHARS.md` ANTES de qualquer `.gw.text` poder referenciá-lo como `speaker`** —
  escrever o diálogo primeiro e cadastrar o personagem depois inverte a ordem e reprova na geração.
- **`text`** (obrigatório): sempre uma chave maiúscula no padrão
  `DIALOGUE_<DIALOGUE_ID>_<NODE_ID>` (`contrato-i18n.md` §4.1, `DLG-03`). Nunca um valor literal
  (§2).
- **`register`** (opcional): sobrescreve `default_register` só para este nó.
- **`on_enter`** (opcional): grava uma flag ao entrar no nó, sintaxe `<namespace>.<nome>=<valor>`
  (§8). Observado sempre no último nó do grafo, mas nada na gramática restringe isso a nós finais.
- Terminação do bloco: implícita — o próximo `@node`, a próxima linha `- [...]` de escolha (§7.2),
  ou o fim do arquivo.

---

## 7. Transições

### 7.1 Linear

```
-> <node_id>
```
ou
```
-> @exit
```

Exatamente uma por nó sem escolha. `@exit` é **sentinela**, não um `node_id` real — não existe
`@node @exit` em lugar nenhum do corpus.

### 7.2 Escolha (ramificação por decisão do jogador)

Substitui a linha `->` única por uma ou mais linhas:

```
- [<CHAVE_I18N_ROTULO>] -> <node_id> flag:<flag_expr>
```

`flag:<flag_expr>` é opcional por escolha (grava uma flag específica daquele ramo).

**Convenção de chave do rótulo, decidida pelo líder:** `DIALOGUE_<DIALOGUE_ID>_<NODE_ID>_CHOICE_<NOME>`
— o `node_id` do nó que oferece a escolha entra na chave, junto com o `dialogue_id`. Isto evita
colisão entre duas escolhas de mesmo nome ofertadas em nós diferentes do mesmo `dialogue_id` (o
risco que motivou a decisão — ver §12). Exemplo real, em `n1_hook` de `npc_intro_bertoldo.gw.text`:
`DIALOGUE_NPC_INTRO_N1_HOOK_CHOICE_CURIOSO`.

### 7.3 Regra de exclusividade

Todo nó tem **exatamente uma** forma de saída: ou uma `->` linear, ou uma ou mais linhas de
escolha — nunca as duas juntas. Não observado combinando nos quatro arquivos, e a L-04 do projeto
(um átomo, uma responsabilidade) trata a mistura como erro de forma, não como recurso.

### 7.4 Convergência é permitida

O grafo é um DAG orientado (`entry` → ... → `@exit`), mas mais de uma escolha pode levar ao mesmo
`node_id` — é o caso real de `n3_reconverge` em `npc_intro_bertoldo.gw.text`, que recebe de três
ramos distintos. Isto não é lacuna, é o desenho pretendido (reconvergência de escolha).

---

## 8. Namespace de flag

Toda flag observada segue `<namespace>.<nome>`, por exemplo `cena15.m1_done`,
`npc_intro.choice_curioso`. O namespace **não é necessariamente igual a `dialogue_id`** — em
`cena15_m1`/`m2`/`m3` (três `dialogue_id` distintos, um por movimento) o namespace compartilhado é
`cena15`, a cena inteira, porque as flags precisam ser lidas por fora do próprio arquivo que as
grava (o M2 é disparado só com `cena15.m1_done` já escrita pelo M1). O namespace é, portanto,
**decisão narrativa de quem autora**, não derivado mecanicamente do `dialogue_id` — este documento
não impõe a igualdade dos dois.

---

## 9. Exemplo mínimo anotado

Trecho real de `resources/dialogues/npc_intro_bertoldo.gw.text`, com comentário de cada linha:

```
#meta dialogue_id npc_intro       // raiz da chave i18n e, aqui, também do namespace de flag
#meta default_register warm       // Bertoldo é humano; toda fala usa caixa quente + retrato
#meta entry n0_greet               // grafo entra por aqui, sempre

@node n0_greet
speaker: bertoldo                  // personagem de CHARS.md
text: DIALOGUE_NPC_INTRO_N0_GREET  // chave, nunca a fala
on_enter: npc_intro.met=true       // grava flag ao entrar (1a vez que a conversa acontece)
-> n1_hook                         // transição linear, um só destino

@node n1_hook
speaker: bertoldo
text: DIALOGUE_NPC_INTRO_N1_HOOK
- [DIALOGUE_NPC_INTRO_N1_HOOK_CHOICE_CURIOSO] -> n2a_curioso flag:npc_intro.choice_curioso=true
- [DIALOGUE_NPC_INTRO_N1_HOOK_CHOICE_PRAGMATICO] -> n2b_pragmatico flag:npc_intro.choice_pragmatico=true
- [DIALOGUE_NPC_INTRO_N1_HOOK_CHOICE_SECO] -> n2c_seco flag:npc_intro.choice_seco=true
// três ramos, cada um grava sua própria flag de escolha; os três reconvergem depois (§7.4).
// Chave de escolha carrega o node_id (N1_HOOK) por decisão do líder — ver §7.2 e §12.
```

---

## 10. Fora do escopo desta gramática, por decisão explícita — nomeado, não inventado

- **Sem espera, retomada nem entrada condicional.** Um arquivo corre inteiro do `entry` ao `@exit`
  numa sessão só. Isto é conhecimento herdado da capacidade do parser do projeto anterior (citado
  na proposta piloto como `dialogue_text.hpp`) — citado aqui só como restrição de capacidade
  conhecida, **não como arquivo existente hoje** (o projeto é do zero, L-01; nenhum código existe
  ainda no repositório).
- **Sem leitura condicional de flag para desviar o grafo por estado prévio** (distinto de
  ramificar por escolha do jogador dentro da própria sessão, que §7.2 já cobre). Os quatro arquivos
  só **escrevem** flag, nunca **leem** uma para decidir caminho. É lacuna real e nomeada: o próprio
  `npc_intro_bertoldo.gw.text` cita `n7_revisit_hub` como fora de escopo "até existir combate/puzzle
  [que ainda não escrevem essas flags]". Não se inventa sintaxe de condicional aqui — ver
  pendência **P-3**.
- **Sem gatilho estruturado.** O campo `GATILHO` do cabeçalho (§4) é só prosa. Quem decide iniciar
  um `dialogue_id` — quest, NPC, zona de mapa — é responsabilidade de outro sistema (`.gw.quest`,
  domínio de NPC), fora do escopo de `B9`.
- **Sem interpolação, plural ou gênero dentro do arquivo.** Esses mecanismos (subconjunto ICU
  MessageFormat, `contrato-i18n.md` §6) vivem no **valor** da chave, dentro do catálogo de
  tradução — nunca em `.gw.text`, que só carrega a chave.

---

## 11. Camadas (L-17)

| Onde | O que mora |
|---|---|
| **fora da espinha**, `tools/` | o `.gw.text` em si (fonte, lido só pelo gerador em build) e o próprio gerador — ferramenta que só constrói e nunca embarca (LEI ZERO, reforma de 28/08/2026, L-05) |
| **`content/`** | o grafo estático pós-compilação (tabela de nós/transições/flags) — dado, não código de instância |
| **`domain/`** | o motor que percorre o grafo em tempo de execução: estado de qual nó está ativo, aplica `on_enter`, expõe escolhas |
| **`app/`** | decide quando disparar um `dialogue_id` e alimenta a escolha do jogador de volta ao motor |
| **`present/`** | desenha a caixa de diálogo — bloqueada até o GlintFx existir (L-06) |

Detalhe de payload e a fronteira jurídica AGPL × ativo reservado: `spec-catalogo-dialogo-binario.md` §5.

---

## 12. Decisões do líder e a única pendência que continua aberta, de propósito

**P-1 — `register` é lista fechada, decidido.** `{warm, terminal}`, sem mais nenhum valor até
pedido novo ao líder. Aplicado em §5.

**P-2 — o gerador valida `speaker` contra `CHARS.md`, decidido: sim.** `speaker` desconhecido
(nem `CHARS.md`, nem allowlist de sistema) é erro de geração. Aplicado em §6, com a consequência de
fluxo de trabalho explícita ali: personagem novo entra em `CHARS.md` antes de qualquer `.gw.text`
poder citá-lo.

**P-3 — leitura condicional de flag para desviar o grafo por estado prévio (diálogo de revisita):
decidido ficar pendente, e a pendência é decisão, não esquecimento.** O corpus de hoje só escreve
flag, nunca lê uma para decidir caminho (§10), e não existe ainda um consumidor real de revisita
(o próprio `npc_intro_bertoldo.gw.text` cita `n7_revisit_hub` como fora de escopo até existir
combate/puzzle). Construir um portão antes de saber o que ele precisa barrar é o erro que a L-36
global existe para evitar — um portão sem uso real nunca é testado contra o caso que deveria
recusar, e vira falso sentimento de cobertura. **O próximo agente que topar com este ponto espera
o primeiro consumidor de revisita nascer e desenha a sintaxe contra ele, em vez de preencher esta
lacuna por conta própria agora.**

**P-4 — convenção de chave do rótulo de escolha: decidido, por nó.**
`DIALOGUE_<DIALOGUE_ID>_<NODE_ID>_CHOICE_<NOME>` — verbatim do líder, *"Nome por nó, mais seguro"*.
Duas escolhas de mesmo nome em nós diferentes do mesmo `dialogue_id` deixam de colidir. Aplicado em
§7.2 e no exemplo do §9; a única chave de escolha já publicada
(`npc_intro_bertoldo.gw.text`, nó `n1_hook`) foi atualizada para
`DIALOGUE_NPC_INTRO_N1_HOOK_CHOICE_{CURIOSO,PRAGMATICO,SECO}` no próprio arquivo-fonte e nos dois
catálogos de tradução que a continham (`pt_br.md`, `en_intl.md`) — nenhum outro lugar do corpus
referenciava a forma anterior.

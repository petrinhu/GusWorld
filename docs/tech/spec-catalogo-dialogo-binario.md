<!--
SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
SPDX-License-Identifier: AGPL-3.0-or-later
-->

# Especificação do payload compilado de diálogo: `E8a` + `E8b` (2 de 2)

> **Status:** canônico, item `B9` da tabela de pendências (metade 2 de 2 — a outra metade é
> `spec-gw-text.md`, a gramática do arquivo-fonte). As duas se definem em conjunto por decisão do
> líder de 24/08/2026.
>
> **O que este documento NÃO redesenha:** o envelope binário comum do projeto (`E1`: magia, versão
> de formato, tipo, nonce, dado, selo — servindo save, configuração, mapa e catálogo, L-25), que
> ainda **não existe** (`⏳ Pendente` na tabela, bloqueado pela criptografia do GlintFx via `E7`,
> sem dublê, L-05). Este documento fixa **o que a compilação de `.gw.text` produz** — o `payload`
> que um dia entra dentro de `E1` —, não como `E1` embrulha esse payload. Também não redesenha o
> contrato de i18n (`D20`, `contrato-i18n.md`), só usa suas regras de fallback e convenção de chave.

---

## 1. A fronteira jurídica, e por que ela corta o diálogo ao meio

A L-25 do projeto fixa, para carta: *"Nenhum número ou regra de carta fica fora do executável: são
código sob a licença do código e vivem dentro dele. Nenhum texto de sabor ou prosa de carta entra
no executável: ficam no pacote binário selado como asset."*

Aplicado a diálogo, a mesma régua corta o dado em dois, e a fronteira não é "de que arquivo veio",
é **a natureza do conteúdo**:

- **A estrutura do grafo** (quais nós existem, para onde cada um leva, quem fala, que flag grava,
  qual chave i18n cada nó referencia) é **regra/mecânica** — não carrega expressão autoral por si
  só; é puro grafo, do mesmo jeito que os números de uma carta são puro balanceamento. Vai para
  **`E8a`**: tabela embutida **dentro do próprio executável**, sob a licença do código
  (AGPL-3.0-or-later).
- **A prosa** — o valor de cada chave i18n, em cada locale — é **texto de sabor**: ficção, voz de
  personagem, com direito autoral reservado (L-08, `ASSETS-LICENSE.md`). Vai para **`E8b`**: pacote
  binário selado à parte, no mesmo regime de todos-os-direitos-reservados que já vale hoje para
  `pt_br.md`/`en_intl.md`.

**Consequência direta de `spec-gw-text.md` §2:** como um arquivo `.gw.text` nunca contém prosa (só
chaves), **todo `.gw.text` compila inteiro para `E8a`** — nenhuma linha dele vai para `E8b`. O que
alimenta `E8b` vem de outro lugar: os catálogos de tradução (`pt_br.md`, `en_intl.md`, e o que vier
depois), não do `.gw.text`.

**Isto é a fronteira entre autoria e binário selado que a ordem de serviço pediu para escrever com
precisão: são DOIS pacotes de destino, nunca um só, e a divisão nasce da natureza do dado — nunca
do arquivo de onde ele veio.**

---

## 2. Payload `E8a`: tabela de diálogo embutida no executável

**Por grafo** (uma entrada por `dialogue_id`):

| Campo | Tipo | Observação |
|---|---|---|
| `dialogue_id` | string estável | nunca reordenável nem repurposeado — mesma disciplina de `enum` append-only do `ADR-019`, aplicada a um catálogo de string-id em vez de `enum` numérico |
| `default_register` | enum fechado `{warm, terminal}` | decisão do líder — `spec-gw-text.md` §5, §12 (P-1) |
| `entry_node` | id de nó | referencia uma entrada da tabela de nós abaixo |

**Por nó** (uma entrada por `@node`, indexada por `dialogue_id` + `node_id`):

| Campo | Tipo | Observação |
|---|---|---|
| `node_id` | string estável, único dentro do grafo | |
| `speaker_id` | string, **validado contra `CHARS.md` + allowlist de sistema** | decisão do líder — ver `spec-gw-text.md` §6, §12 (P-2); id fora das duas listas é erro de geração |
| `text_key` | string (chave i18n) | nunca o valor — o valor mora em `E8b` |
| `register_override` | enum fechado `{warm, terminal}`, opcional | ausente = usa `default_register` do grafo |
| `on_enter_flag` | par (nome, valor), opcional | grava no save ao entrar no nó |
| `transition` | **um** de: nó linear, `@exit`, ou lista de escolhas | ver abaixo |

**Escolha**, quando `transition` é uma lista: cada item é `(chave i18n do rótulo, node_id destino,
flag opcional)` — o mesmo shape de `on_enter_flag`, reaproveitado.

**Serialização:** binária, versionada, escrita em casa, por struct explícito com round-trip
byte-exato — o mesmo padrão que `ADR-006`/o item `E2` já fixam para os POCOs de domínio. Nunca
JSON, nunca texto, nunca biblioteca externa de serialização (o projeto é zero-dependência fora do
GlintFx, LEI ZERO).

**Origem:** o gerador (`E8a`) varre todo `resources/dialogues/*.gw.text`, valida (§4) e emite esta
tabela.

---

## 3. Payload `E8b`: entradas de string seladas, por locale

**Não é exclusivo de diálogo** — é o mesmo mecanismo que qualquer domínio de chave i18n usa (menu,
HUD, erro, etc.), então esta seção descreve só a **fatia** que as chaves `DIALOGUE_*` ocupam dentro
dele, não o catálogo inteiro (fora do escopo de `B9`; ver `contrato-i18n.md`).

**Por chave, por locale:**

| Campo | Tipo | Observação |
|---|---|---|
| `key` | string (chave i18n) | mesma chave referenciada por `E8a.text_key` ou por um rótulo de escolha |
| `locale` | id de locale | lido do registro de locale, dimensão aberta (`contrato-i18n.md` §2) |
| `value` | string UTF-8, presente ou ausente | pode carregar sintaxe ICU de plural/seleção (`contrato-i18n.md` §6); **ausência é estado válido** (tradução pendente, §3.2 do contrato) — o payload precisa distinguir "vazio" de "ausente", nunca colapsar os dois num só bit |

O locale de referência (`pt_br`) é a única entrada cuja **completude** o CI exige
(`contrato-i18n.md` §10) — isto é regra de aceite de build, não uma restrição do formato do
payload em si.

---

## 4. O gerador: o que ele deve recusar antes de emitir

Não é código — é o contrato de validação que qualquer implementação de `E8a` precisa cumprir:

- Todo `node_id` referenciado por uma transição (linear ou de escolha) existe como `@node` no
  mesmo arquivo, ou é `@exit`.
- `entry` aponta para um `node_id` que existe.
- Toda chave (`text:` ou rótulo de escolha) segue o padrão `DIALOGUE_<DIALOGUE_ID>_...` e existe —
  mesmo que vazia — no catálogo de referência (`pt_br`). Chave ausente da referência é **erro de
  build**, nunca silêncio: o comportamento de "chave crua na tela" do `contrato-i18n.md` §3.3 é
  para erro que **escapou** até o runtime; em tempo de build o gerador pode e deve recusar antes.
- `dialogue_id` é único entre todos os arquivos compilados — o namespace de chave e o de flag
  dependem disso.
- Nenhum nó fica sem transição de saída — todo caminho do grafo termina em `@exit`.
- **`speaker` de todo nó é uma entrada de `CHARS.md` ou um id de sistema da allowlist (hoje só
  `terminal`)** — decisão do líder (`spec-gw-text.md` §12, P-2). `speaker` fora das duas listas é
  erro de build, nunca aviso: personagem novo precisa existir em `CHARS.md` antes de qualquer
  `.gw.text` poder citá-lo.
- **`register`/`default_register` de todo nó ou grafo pertence ao enum fechado `{warm, terminal}`**
  — decisão do líder (`spec-gw-text.md` §12, P-1). Um valor fora da lista é erro de build.

---

## 5. Camadas (L-17)

| Camada | O que mora aqui, para diálogo |
|---|---|
| **fora da espinha**, `tools/` | o gerador: lê `.gw.text` + catálogos de tradução, valida (§4), emite `E8a` + `E8b`. Nunca embarca no binário do jogador (LEI ZERO, reforma de 28/08/2026, L-05) |
| **`content/`** | o catálogo estático pós-compilação — a tabela de grafos/nós/transições (`E8a`) e a tabela de strings por locale (`E8b`), carregadas em memória no boot, imutáveis durante a partida |
| **`domain/`** | o motor de travessia: estado "qual nó está ativo neste `dialogue_id` agora" (parte do save), aplica `on_enter` (grava flag), calcula as escolhas disponíveis, avança por comando — transição determinística que recebe estado e comando e devolve estado novo e eventos (L-17) |
| **`app/`** | decide quando invocar um `dialogue_id` (reage a evento de quest/NPC/zona de mapa) e alimenta a escolha do jogador de volta ao motor de `domain/` |
| **`present/`** | desenha a caixa de diálogo, retrato, texto (chave → string do locale ativo, com a cadeia de fallback do `contrato-i18n.md` §3). Bloqueada até o GlintFx existir (L-06) |

---

## 6. Relação com `E1` (envelope comum) — o que este documento não decide

`E1` ainda não existe (`⏳ Pendente`, bloqueado por `E7`/GlintFx). Este documento não presume o
layout de `magia`/`versão de formato`/`tipo`/`nonce`/`selo` de `E1` — ele só define o **payload**
(§2, §3) que um dia entra no campo `dado` de `E1`, sem opinar em como `E1` embrulha esse dado.

Quando `E1` nascer, pode ser preciso um apêndice aqui dizendo qual `tipo` (discriminador) `E1`
reserva para o catálogo de diálogo — isso fica pendência do próprio `E1`, não decidida neste
documento.

---

## 7. Decisões do líder aplicadas ao schema, e a única pendência que resta

Três das quatro perguntas de `spec-gw-text.md` §12 já vieram decididas e já estão refletidas neste
schema: `register_override`/`default_register` são o enum fechado `{warm, terminal}` (P-1, §2);
`speaker_id` é validado contra `CHARS.md` + allowlist de sistema, com a checagem exigida no
contrato do gerador (P-2, §4); a chave de rótulo de escolha em `E8b` carrega o `node_id`
(`DIALOGUE_<ID>_<NODE_ID>_CHOICE_<NOME>`, P-4) — nenhum campo novo de schema decorre disso, é só a
forma da própria string-chave.

**A única pendência que resta é P-3 (leitura condicional de flag), e ela fica pendente por
decisão, não por esquecimento** (`spec-gw-text.md` §12): sem consumidor real de revisita hoje, não
existe caso contra o qual desenhar ou testar um campo de condição em `transition`. Se e quando P-3
for decidida, o campo entra em `transition` (§2) como uma condição opcional de leitura de flag
antes de oferecer um destino ou uma escolha — não decidido aqui, para não construir portão sem o
que ele deve barrar (mesma razão registrada em `spec-gw-text.md` §12).

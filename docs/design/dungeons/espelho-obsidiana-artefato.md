<!--
SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
SPDX-License-Identifier: NONE — regime reservado (L-08 de GODS_LAWS.md; docs/design/ fica fora do
código, sob o mesmo regime de `narrative/`, `specs/`, `CHARS.md`, `PLACES.md`, `sinopse.md`).
-->

# Especificação do artefato Espelho de Obsidiana (`D28`, metade física/pública)

> **Metade PÚBLICA desta especificação.** A metade que revela o EFEITO do artefato (o ganho de
> combate, a camada de tradução/kernel que ele liga, e o que isso significa para o mundo) é a
> própria revelação central do fim da dungeon Kola-SG3-12262 — fica cifrada, em
> `docs/_secret/dungeons/espelho-obsidiana-mecanica.md`, e este documento aponta pra lá pelo
> caminho, sem citar conteúdo. **Critério da fronteira, o mesmo já em vigor para o hub da dungeon**
> (`docs/design/dungeons/kola-sg3-12262-espelho-obsidiana.md`, verbatim do briefing do líder que
> abriu aquele split): um leitor do repositório público pode saber que o artefato existe, a
> aparência dele, como se chega a ele e como ele se pareia com a carta; não pode saber o que ele
> FAZ quando equipado, nem o que essa revelação conta sobre a natureza do mundo.
>
> Tudo que este documento afirma já é público em `docs/design/dungeons/kola-sg3-12262-espelho-obsidiana.md`
> (beat 8, secão 3) e em `docs/_secret/dungeons/pesquisa-espelho-obsidiana.md` §5 (cifrado só por
> estar no mesmo diretório de pesquisa, não por conteúdo secreto — a pesquisa em si é meramente
> factual sobre RFID real). Este documento não introduz fato novo sobre a física do objeto; ele
> consolida, num átomo próprio (L-04/L-33), a especificação de hardware que o item `D28` da tabela
> de pendências pede.
>
> **Fonte:** `docs/_secret/dungeons/CAPTURA-BRUTA-kola-espelho-obsidiana.md` item 13 (dimensão) e
> texto do líder verbatim (pareamento por RF); `docs/_secret/dungeons/pesquisa-espelho-obsidiana.md`
> §1.1 (dimensões do objeto histórico) e §5 (plausibilidade técnica de RFID passivo por indução).
>
> **Cross-refs:** `docs/design/dungeons/kola-sg3-12262-espelho-obsidiana.md` (hub da dungeon, beat
> 8) · `docs/design/roster-analogos/15-john-dee.md` (a carta Espelho Negro, `OCU-02-R_v1.0`,
> pública) · `docs/design/mecanicas/cartas/dee.md` (implementação da carta) ·
> `docs/design/mecanicas/cartas-hardware-pirataria-energia.md` §5 (o vocabulário de bateria/carga
> que a carta pareada usa) · `docs/_secret/dungeons/espelho-obsidiana-mecanica.md` (metade cifrada
> desta mesma especificação, o efeito) · `docs/design/technomagik.md` (metafísica pública, alvo da
> propagação do `D30`, que a metade cifrada deste documento estende).

---

## 1. O que este documento cobre, e o que não cobre

Este documento fixa a **forma física** do artefato e o **mecanismo de pareamento** com a carta —
tudo o que a party descobre e usa antes de o efeito do espelho ser revelado (beats 1-8 do hub
público). Ele não cobre, porque pertence à metade cifrada:

| O que fica de fora daqui | Onde vive |
|---|---|
| O que a camada de tradução (o "kernel") realmente é e faz | `docs/_secret/dungeons/espelho-obsidiana-mecanica.md` |
| O número de ganho de eficiência em combate e a mudança de mundo (democratização) | `docs/_secret/dungeons/espelho-obsidiana-mecanica.md` |
| A revelação sobre a Era Lendária e a natureza do mundo que o espelho carrega na memória ROM | `docs/_secret/dungeons/kola-sg3-12262-desfecho.md` secão 3 |

## 2. Identidade do objeto

- **Nome do artefato:** Espelho de Obsidiana. Item de **hardware**, não carta — não entra no
  catálogo de cartas (`_IDS-CARTAS.md`), não tem entrada em `.gw.card`, e não é craftável nem
  obtido por drop: é **singleton de missão**, existe uma única unidade no jogo, encontrada no fim
  da cadeia de missão da Kola-SG3-12262 (hub público, secão 3, beats 7-8).
- **Dimensões:** **22 × 18,4 cm** (decisão do líder, arbitrando entre as duas medições divergentes
  do objeto histórico real — 18,5 cm de diâmetro numa fonte, 22 × 18,4 cm noutra —, a favor da
  segunda para o objeto do jogo; a arte da carta pareada não precisa respeitar essa escala).
  Formato: disco/placa de obsidiana polida, análogo direto ao espelho histórico de John Dee
  (British Museum, registro `Am1966,01.1`).
- **Localização:** sala final da Kola-SG3-12262, atrás da porta com fechadura em forma de disco de
  vinil (hub público, beat 7). Não se move do lugar antes de equipado (não é item de inventário
  carregável antes desse ponto da missão — o "equipar" acontece na própria sala, como parte da
  cena de revelação, cuja mecânica exata é cifrada).

## 3. Mecanismo de pareamento com a carta Espelho Negro

- **Carta pareada:** `OCU-02-R_v1.0`, "Espelho Negro" (mestre John Dee), já pública e já
  implementada em `docs/design/mecanicas/cartas/dee.md` e `resources/cards/dee.gw.card`. O
  pareamento é **fixo e exclusivo**: nenhuma outra carta do roster aciona o artefato.
- **Base técnica (plausibilidade real, não invenção de jogo):** o pareamento é descrito, em
  diegese, como percepção mútua por **indução/radiofrequência de curto alcance**, análoga a RFID
  passivo por acoplamento indutivo real (`pesquisa-espelho-obsidiana.md` §5.1-5.2). Isso ancora
  fisicamente **duas** propriedades já fixadas na cadeia de missão:
  1. **Alcance curto:** poucos centímetros até no máximo ~1 metro (faixa real de RFID indutivo
     LF/HF; a pergunta do líder, "percebem-se via RFID", cabe sem esticar a física — ver
     `pesquisa-espelho-obsidiana.md` §5.4 sobre o que **estica** a régua, que não é o caso aqui).
     Nenhum número exato de centímetros é fixado além dessa faixa: não é dado de balanceamento, é
     plausibilidade de lore, e não precisa de medição de combate para existir.
  2. **A carta precisa estar com bateria carregada** para o espelho reagir (regra já canônica da
     cadeia de missão, hub público beat 8): sem carga, a carta é hardware inerte, e RFID passivo
     não tem nada para "responder" com um par igualmente inerte do outro lado.
- **Pista jogável:** a arte da carta e a face do espelho físico são **idênticas** — é essa
  identidade visual, não um texto explicativo, que leva o jogador a deduzir a ligação (hub
  público, beat 8: "a pista visual é a face idêntica entre a arte da carta e o espelho físico").
- **Estado antes da revelação (o que este documento cobre):**
  - **Dormente:** antes de a party se aproximar com a carta carregada, o espelho não reage.
  - **Brilhando:** ao se aproximar com a carta (bateria carregada), o espelho brilha (hub público,
    beat 8). Este é o gatilho de puzzle que leva Gus a investigar com o aplicativo de escuta de
    frequências.
  - **Ativo:** o que acontece a partir daqui (o que o espelho revela, o que equipá-lo muda) é a
    metade cifrada (secão 1 acima).

## 4. O que este item NÃO é

- **Não é carta.** Não tem POCO de carta, não entra em mão, não é craftável, não tem raridade, não
  aparece em `_IDS-CARTAS.md`.
- **Não introduz sistema de altura, verticalidade de câmera ou coordenada Z (L-26).** O espelho é
  um objeto de cena, num ponto fixo da grade 2D da sala final da Kola-SG3-12262, igual a qualquer
  outro objeto interativo do jogo. Ele não é, em si, o mecanismo que resolve a descida física até
  12.262 m — isso é papel do item `D25` (traje) somado à geografia (rocha pastosa que fecha o
  fundo, hub público secão 2). O espelho só existe DEPOIS que a party já chegou ao fundo da
  caverna pela travessia de mapa normal.
- **Não tem bateria própria.** Quem tem bateria é a carta pareada (`OCU-02-R_v1.0`). O artefato em
  si, na física estabelecida (RFID passivo), não precisa de fonte de energia própria para a etapa
  descrita neste documento — ele é energizado pelo campo da carta, não o contrário
  (`pesquisa-espelho-obsidiana.md` §5.1: "o leitor" energiza a etiqueta passiva; aqui os papéis
  entre carta e espelho, e qual dos dois é o "leitor" ativo, são detalhe da metade cifrada).

## 5. Cortes da L-29 conferidos (nenhum atravessado)

- **C-02** (sem mundo aberto/persistente, gating não é trava dura): o artefato é objeto de missão
  linear dentro de uma dungeon já orçada; não cria mundo novo nem trava permanente de área. **Cabe.**
- **C-06** (sem dificuldade dinâmica adaptativa): o efeito do espelho (cifrado) é um evento fixo de
  progresso de missão, disparado por completar a cadeia, não um ajuste que reage à performance do
  jogador. **Não é DDA. Cabe.**
- **C-15** (escopo fechado, mecânica nova só se couber num jogo com fim): o artefato não é
  mecânica nova de sistema — é item singleton de missão dentro de uma dungeon já orçada e já
  aprovada (`D21`). **Cabe**, sem tensionar além do que o hub público já registrou para a dungeon
  como um todo.
- Nenhum outro corte é tocado por este documento.

## 6. Handoff

- `D30` (propagação a `CHARS.md`, `sinopse.md`, `docs/design/technomagik.md`,
  `docs/narrative/arco-principal.md`) segue apontando para a metade cifrada
  (`docs/_secret/dungeons/espelho-obsidiana-mecanica.md`) para o conteúdo do efeito, e para este
  documento para a forma física, quando precisar da segunda.
- Redação em prosa final da cena de revelação: `narrative-writer`, só depois de a metade cifrada
  existir, seguindo o mesmo template de 5 beats já usado para o Tavus-Eco/Morlhin
  (`docs/design/technomagik.md` §"Cena de encontro").
- Nenhum número de balanceamento pendente nesta metade: os únicos números aqui (dimensão física,
  faixa de alcance de RFID) são flavor/plausibilidade, não dado de combate, e não exigem medição
  por simulação.

---

**Última revisão:** 06/09/2026.

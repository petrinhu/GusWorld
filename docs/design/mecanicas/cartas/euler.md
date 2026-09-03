<!--
SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
SPDX-License-Identifier: AGPL-3.0-or-later
-->

# Bridge-Walk (`euler`)

**Tipo Diátaxis:** Explanation. **Audiência:** design/engenharia deste projeto. **Last-reviewed:** 25/08/2026. **Owner:** `technical-writer`, extração do projeto anterior (L-01).

Dado completo em [`resources/cards/euler.gw.card`](../../../../resources/cards/euler.gw.card).

## Ficha rápida

Família **Elétrico** · categoria **Fora de combate** · especial · mana 0 · posse-only, sem programa de combate.

## Como funciona

Não tem nenhum efeito de combate. É uma carta de posse: dá um atalho de exploração e revela o grafo (o mapa de conexões) de uma dungeon.

## Por que é assim

O nome vem do problema clássico: "As 7 pontes de Königsberg: dava pra cruzar todas sem repetir? Euler resolveu e criou a teoria dos grafos. Eis o seu mapa" (`_EFEITOS-ESCOLHIDOS.md`). O efeito escolhido junta duas peças (`_EFEITOS-ESCOLHIDOS.md`, Lote 3): a "Ponte de Euler" (passiva de atalho) e "Traçar Rota" (ativa que revela o grafo de uma dungeon) — a mesma lógica de teoria dos grafos, aplicada como mecânica de exploração. No código-fonte, isso aparece só como registro posse-only, sem `EffectSpec`: o sistema de "revelar grafo" em si não tem implementação, é design de exploração ainda a construir.

## Pontas soltas

- Nenhum número numérico afinável — a carta não tem `@effect` nenhum no legado.
- A chave `CARD_EXEC_EULER_NAME` resolve para **"Bridge-Walk"**.
- **`mana 0` é decisão registrada do líder, 03/09/2026, por `AskUserQuestion`** (`cartas-hardware-pirataria-energia.md` §5), não herança sem revisão: as cartas Fora de combate são posse-only e não ficam ligadas em nada, então não ganham os dois gastos que a face passiva das Híbridas e das passivas puras carregam.

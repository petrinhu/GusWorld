<!--
SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
SPDX-License-Identifier: AGPL-3.0-or-later
-->

# Quantum-Lock (`planck`)

**Tipo Diátaxis:** Explanation. **Audiência:** design/engenharia deste projeto. **Last-reviewed:** 25/08/2026. **Owner:** `technical-writer`, extração do projeto anterior (L-01).

Dado completo em [`resources/cards/planck.gw.card`](../../../../resources/cards/planck.gw.card).

## Ficha rápida

Família **Universal** · categoria **Passiva** · especial · dois gastos de mana/bateria (standby + disparo, números pendentes, ver `_vocabulario.md` §9) · equip-only, nunca jogada.

## Como funciona

Substitui a variação contínua de dano do portador por **3 degraus fixos**: piso, centro e teto da própria faixa de variância que o jogo já usa (não uma faixa nova). As chances são **fixas**: 25% para cada extremo (piso e teto), 50% para o centro — e essas chances **não mudam** com progressão do jogador; só a **largura** dos degraus estreita com o avanço. A média resultante é igual à base, ou seja, **zero mudança de balance** — a carta elimina os quase-acertos, não infla nem reduz o dano esperado.

## Por que é assim

O nome vem direto da física: "A energia não vem em qualquer quantidade, só em pacotes fixos. Planck descobriu isso, e por isso seu dano agora cai em degraus certos" (`_EFEITOS-ESCOLHIDOS.md`). Ao contrário da maioria das outras especiais, os números desta carta (25/50/25) **não são `//PLAYTEST`-livres**: o código-fonte marca explicitamente que são "números FECHADOS pelo líder" — decisão de balance definitiva, não um valor provisório à espera de afinação.

`_EFEITOS-ESCOLHIDOS.md` (AMB-06) registra as cinco decisões que fecharam esta carta: 3 degraus da própria faixa de variância (não uma faixa nova); o preview da UI mostra os 3 valores e as 3 chances (informação perfeita, nunca opaca); o mecanismo troca o sorteio contínuo por um sorteio de degrau sem consumir sorteio extra; crítico e falha ficam intactos (a quantização só mexe no canal de dano comum). É também a única carta do catálogo marcada como **histórica**: só o Gus a equipa, por decisão de progressão narrativa — não uma restrição do motor, que continua agnóstico a qual personagem porta qual carta.

## Pontas soltas

- Nenhum número aqui é `//PLAYTEST` — todos são "fechados pelo líder".
- A chave `CARD_EXEC_PLANCK_NAME` resolve para **"Quantum-Lock"**.

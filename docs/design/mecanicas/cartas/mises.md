<!--
SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
SPDX-License-Identifier: AGPL-3.0-or-later
-->

# Calc-Edge (`mises`)

**Tipo Diátaxis:** Explanation. **Audiência:** design/engenharia deste projeto. **Last-reviewed:** 25/08/2026. **Owner:** `technical-writer`, extração do projeto anterior (L-01).

Dado completo em [`resources/cards/mises.gw.card`](../../../../resources/cards/mises.gw.card).

## Ficha rápida

Família **Universal** · categoria **Passiva** · especial · dois gastos de mana/bateria (standby + disparo, números pendentes, ver `_vocabulario.md` §9) · equip-only, nunca jogada. Passiva **dupla**, num único registro de efeito.

## Como funciona

Duas faces que não competem entre si:

1. **"A party aloca melhor":** concede +1 ponto de ação ao próprio dono, a cada turno dele — não ao time inteiro, e o bônus não persiste (reseta no turno seguinte).
2. **"Comando central":** inimigos marcados com uma tag específica ("comando central") no lado oposto ao portador sofrem atraso na fila (empurrados para o fim do bloco do lado deles) e um desconto fixo de 13% no dano ofensivo que causam, enquanto a Mises estiver equipada por alguém vivo do lado que a porta.

## Por que é assim

O nome vem da crítica de Mises ao cálculo econômico centralizado: "Sem preço real, ninguém calcula o que vale a pena (Mises). Quem obedece um comando central erra a conta. Você, não" (`_EFEITOS-ESCOLHIDOS.md`). A mecânica é literalmente essa ideia traduzida em duas metades: quem **calcula bem** (o dono da Mises) ganha eficiência (+1 AP); quem **obedece um comando central** (os inimigos taggeados) erra a conta (atraso + desconto de dano).

A "tag de comando central" **não é um efeito de carta** — é uma propriedade do próprio combatente (o mesmo padrão já usado para marcar um inimigo como chefe), e a curadoria de **quais** inimigos do bestiário levam essa tag é trabalho de design/lore, fora do escopo desta carta: o motor só sabe reagir à tag, não decidir quem a porta.

## Pontas soltas

- O `+1` de AP e o desconto de 13% estão marcados `//PLAYTEST`, com o mesmo aviso do Hayek contra rotular como "easter-egg" em código ou commit.
- Nenhum inimigo do bestiário está confirmado como portador da tag "comando central" no material lido para esta extração — é curadoria de design pendente, não uma lacuna desta carta.
- A chave `CARD_EXEC_MISES_NAME` resolve para **"Calc-Edge"**.

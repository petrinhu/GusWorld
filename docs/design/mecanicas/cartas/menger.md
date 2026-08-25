<!--
SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
SPDX-License-Identifier: AGPL-3.0-or-later
-->

# Barter (`menger`)

**Tipo Diátaxis:** Explanation. **Audiência:** design/engenharia deste projeto. **Last-reviewed:** 25/08/2026. **Owner:** `technical-writer`, extração do projeto anterior (L-01).

Dado completo em [`resources/cards/menger.gw.card`](../../../../resources/cards/menger.gw.card).

## Ficha rápida

Família **Universal** · categoria **Fora de combate** · especial · mana 0 · posse-only, sem programa de combate.

## Como funciona

Não tem nenhum efeito de combate. Mostra o valor real de cada consumível ("Valor Marginal") e permite trocar loot excedente por Crédito na hora, sem precisar de loja ("Escambo Espontâneo", 1×/encontro).

## Por que é assim

O nome vem do economista que explicou a origem do dinheiro pela troca direta: "Menger explicou como o dinheiro nasceu da troca direta. Você trocou o que sobrava por Crédito na hora, sem loja" (`_EFEITOS-ESCOLHIDOS.md`). O efeito (Lote 5) junta duas peças com a mesma raiz teórica: revelar o valor marginal de um item (quanto ele realmente vale para você, não um preço fixo de tabela) e converter esse valor em Crédito sem intermediário. O próprio design já registra a amarração com outro sistema do jogo (o "Mercado da Sucata Honesta"), que está fora do escopo desta extração de cartas.

## Pontas soltas

- Nenhum número numérico afinável — a carta não tem `@effect` nenhum no legado; a lógica de "valor marginal" e de conversão vive em sistemas de economia fora do registro da carta.
- A chave `CARD_EXEC_MENGER_NAME` resolve para **"Barter"**.

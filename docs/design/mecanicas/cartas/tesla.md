<!--
SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
SPDX-License-Identifier: AGPL-3.0-or-later
-->

# `tesla`

**Tipo Diátaxis:** Explanation. **Audiência:** design/engenharia deste projeto. **Last-reviewed:** 25/08/2026. **Owner:** `technical-writer`, extração do projeto anterior (L-01).

Dado completo em [`resources/cards/tesla.gw.card`](../../../../resources/cards/tesla.gw.card).

## Ficha rápida

Família **Elétrico** · categoria **Ativa** · especial · mana 6 · poder-base 8 · 1×/batalha.

## Como funciona

Depois do dano-base atingir o alvo primário, a descarga **salta** para os próximos inimigos vivos na ordem da fila (2 saltos = 3 alvos no total), retendo 62% do dano a cada salto (decaimento multiplicativo: 10 → 6 → 4, por exemplo). A cadeia para quando faltam alvos vivos ou quando o salto arredonda para zero.

Esta é a **única** das 20 especiais com poder-base próprio (`power = 8`): todas as outras têm `power = 0` porque o dano/efeito delas vem inteiramente do programa (`EffectSpec`), não de um valor de poder direto na carta. A cadeia da Tesla precisa desse poder-base porque é dele que ela escala.

## Por que é assim

O nome do efeito, "Descarga em Cadeia", vem literal do experimento de Tesla: "O arco de Tesla salta de inimigo em inimigo, mais fraco a cada salto. Ele sonhava em enviar energia sem fios" (`_EFEITOS-ESCOLHIDOS.md`). O decaimento multiplicativo (62% a cada salto, não uma perda fixa) é o que faz a cadeia "morrer" naturalmente em vez de continuar para sempre — é o mesmo tipo de decaimento físico real de um arco elétrico perdendo energia a cada salto.

O comentário-fonte registra que o dano-base é "turbinável por item futuro" — um capacitor com trade-off série/paralelo (mais tensão de pico por menos saltos, ou o inverso), já desenhado em `capacitor-item.md` deste projeto, mas como item separado, ainda não implementado.

## Pontas soltas

- **A chave de tradução em uso (`CARD_EXEC_TESLA_NAME`) resolve literalmente para "Tesla"** em `resources/translations/pt_br.md` — não para a alcunha de efeito "Coil-Arc" que `_EFEITOS-ESCOLHIDOS.md` §"Nomes dos efeitos" registra como o nome escolhido. É uma inconsistência real entre os dois documentos-fonte, não uma escolha desta extração: o padrão declarado em `cartas-technomagik.md` §8.1 é `cardExec-[figura]` usando o nome real do mestre, então "Tesla" bate com esse padrão — mas quebra o padrão paralelo da alcunha de efeito que todas as outras 19 especiais seguem (`CARD_EXEC_NEWTON_NAME` → "Force-Law", não "Newton"). Ver também `einstein.md`, mesma situação.
- O poder-base (8) e a retenção por salto (62%) estão marcados `//PLAYTEST`.

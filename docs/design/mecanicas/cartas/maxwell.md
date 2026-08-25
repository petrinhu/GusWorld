<!--
SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
SPDX-License-Identifier: AGPL-3.0-or-later
-->

# Spectra-Wave (`maxwell`)

**Tipo Diátaxis:** Explanation. **Audiência:** design/engenharia deste projeto. **Last-reviewed:** 25/08/2026. **Owner:** `technical-writer`, extração do projeto anterior (L-01).

Dado completo em [`resources/cards/maxwell.gw.card`](../../../../resources/cards/maxwell.gw.card).

## Ficha rápida

Família **Elétrico** · categoria **Híbrida** · especial · mana 6 · alvo **Grupo** · poder-base 5.

## Como funciona

Causa dano a **todos** os inimigos vivos do lado oposto — dano-base (5) mais o ataque do conjurador, exatamente a mesma fórmula de qualquer ataque comum, sem nenhuma instrução declarativa extra: o dano-base **é** o efeito inteiro. Não existe nenhum bloco de efeito nesta carta porque não faz falta nenhum. Mirar um aliado é um no-op completo (a regra "fogo amigo desligado" já zera o dano, e sem instrução declarativa não sobra nada mais para acontecer).

## Por que é assim

O nome vem da unificação teórica de Maxwell: "Maxwell mostrou que luz, eletricidade e magnetismo são a MESMA onda. Por isso a luz também ataca" (`_EFEITOS-ESCOLHIDOS.md`). O achado de engenharia mais interessante desta carta é que ela **não precisou de nada novo**: o comentário-fonte registra que ela reusa, puro, o mesmo caminho de área que a face ofensiva do Newton já usa — "zero `EffectKind` novo, zero wiring novo no resolvedor... é a carta mais simples do catálogo até hoje". A decisão que exigiu julgamento não foi de mecanismo, foi de **número**: por que o poder-base é 5, e não 8 como a Tesla. O próprio comentário-fonte explica: um poder cheio de 8 "eclipsaria o nicho" da Tesla — a diferença de design entre as duas cartas de área elétrica é que a Tesla é burst concentrado que decai a cada salto, e a Maxwell é dano uniforme mais fraco espalhado por todos os inimigos ao mesmo tempo. Sem essa diferença de poder, as duas cartas competiriam pelo mesmo papel.

## Pontas soltas

- A decisão do poder-base (5) foi tomada de forma **autônoma pelo orquestrador da implementação**, com o líder ausente naquele dia (2026-07-16) — o próprio comentário-fonte marca isso, "decisão AUTÔNOMA a confirmar". Não há registro, no material lido para esta extração, de confirmação posterior do líder.
- A face fora-de-combate ("iluminar áreas escuras") não tem sistema para se apoiar (o overworld não tem esse conceito ainda) — mesma situação de outras faces fora-de-combate do catálogo.
- A chave `CARD_EXEC_MAXWELL_NAME` resolve para **"Spectra-Wave"**.

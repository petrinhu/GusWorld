<!--
SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
SPDX-License-Identifier: AGPL-3.0-or-later
-->

# Fractal-Echo (`mandelbrot`)

**Tipo Diátaxis:** Explanation. **Audiência:** design/engenharia deste projeto. **Last-reviewed:** 25/08/2026. **Owner:** `technical-writer`, extração do projeto anterior (L-01).

Dado completo em [`resources/cards/mandelbrot.gw.card`](../../../../resources/cards/mandelbrot.gw.card).

## Ficha rápida

Família **Universal** · categoria **Ativa** · especial · mana 6 · 1×/batalha.

## Como funciona

Ecoa a última ação de dano de **qualquer** aliado nesta rodada, a 50% do valor original (`kMandelbrotEchoPercent`). Ao contrário da Ada (que também ecoa ações de aliados), o Mandelbrot **sempre dispara** — não sorteia chance nenhuma (`magnitude = 0`, zero consumo de sorteio).

## Por que é assim

O nome vem direto da geometria: "um fractal repete a mesma forma cada vez menor, sem fim" (`_EFEITOS-ESCOLHIDOS.md`, frase pedagógica de "Fractal-Echo") — o eco a 50% é a "forma cada vez menor" do fractal, uma repetição diminuída da ação original.

A janela de memória ("nesta rodada", não "neste turno") não foi óbvia de definir: `_EFEITOS-ESCOLHIDOS.md` AMB-01 registra que a redação original dizia "repete a última ação da party **neste turno**", ambígua num combate por-turno-de-ator (cada ator tem seu próprio turno dentro de uma rodada). O líder decidiu por "nesta rodada" — uma janela do tamanho de "1 turno de 1 ator" tornaria o Mandelbrot quase sempre um no-op, já que ele mesmo precisa gastar seu turno para conjurar, e "o turno anterior" seria quase sempre de outro ator. A memória usada (a última ação de dano de qualquer aliado, zerada na fronteira da rodada) é a mesma primitiva que o Pitágoras já usa para o combo cross-ator.

## Pontas soltas

- O percentual do eco (50%) está marcado `//PLAYTEST`, número não afinado por playtest formal.
- A chave `CARD_EXEC_MANDELBROT_NAME` resolve para **"Fractal-Echo"**.

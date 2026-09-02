<!--
SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
SPDX-License-Identifier: AGPL-3.0-or-later
-->

# Re-Run (`ada`)

**Tipo Diátaxis:** Explanation. **Audiência:** design/engenharia deste projeto. **Last-reviewed:** 25/08/2026. **Owner:** `technical-writer`, extração do projeto anterior (L-01).

Dado completo em [`resources/cards/ada.gw.card`](../../../../resources/cards/ada.gw.card).

## Ficha rápida

Família **Universal** · categoria **Passiva** · especial · dois gastos de mana/bateria (standby + disparo, números pendentes, ver `_vocabulario.md` §9) · equip-only, nunca jogada.

## Como funciona

No fim do turno de **outro** aliado (não o próprio dono da carta), há uma chance de repetir a última ação de dano dele por completo — a 100% do valor original (`kAdaEchoPercent`), não uma fração reduzida. A chance de disparar é baixa: 34% (`kAdaEchoChance`).

## Por que é assim

O nome remete ao primeiro programa da história: "Ada Lovelace escreveu o primeiro programa da história: uma sequência que se repete sozinha" (`_EFEITOS-ESCOLHIDOS.md`). A distinção mecânica com o Mandelbrot é deliberada e nomeada no código-fonte: **o freio da Ada é a chance, não a escala** (decisão Q3 do líder, 2026-07-14) — por isso ela ecoa a 100% quando dispara (sem desconto), mas só dispara 1 em cada 3 vezes, aproximadamente. O Mandelbrot é o inverso: sempre dispara, mas só ecoa metade.

O código-fonte chama o número 34% de "easter-egg velado" — uma pista de que o valor não é aleatório-sem-sentido, mas o texto não diz qual é a referência por trás dele, e este documento não inventa uma.

## Pontas soltas

- **O "easter-egg" por trás do 34% não está explicado em lugar nenhum do legado.** O comentário-fonte só afirma que existe um, sem dizer qual — registrado aqui como está, sem completar a lacuna.
- A chave `CARD_EXEC_ADA_NAME` resolve para **"Re-Run"**.

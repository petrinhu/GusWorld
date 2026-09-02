<!--
SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
SPDX-License-Identifier: AGPL-3.0-or-later
-->

# Hypotenuse (`pythagoras`)

**Tipo Diátaxis:** Explanation. **Audiência:** design/engenharia deste projeto. **Last-reviewed:** 25/08/2026. **Owner:** `technical-writer`, extração do projeto anterior (L-01).

Dado completo em [`resources/cards/pythagoras.gw.card`](../../../../resources/cards/pythagoras.gw.card).

## Ficha rápida

Família **Universal** · categoria **Passiva** · especial · dois gastos de mana/bateria (standby + disparo, números pendentes, ver `_vocabulario.md` §9) · equip-only, nunca jogada.

## Como funciona

Quando dois ou mais aliados batem no **mesmo alvo** na **mesma rodada**, a carta soma os dois golpes por um golpe-bônus cuja potência é a **raiz quadrada da soma dos quadrados** dos dois — o teorema de Pitágoras, literal: `c = √(a² + b²)`. O resultado é sempre maior que cada golpe isolado e sempre menor que a soma simples dos dois, porque é assim que a hipotenusa se comporta.

## Por que é assim

`_EFEITOS-ESCOLHIDOS.md` registra o efeito ESCOLHIDO como "quando 2 aliados atacam o mesmo alvo, golpe-bônus de potência = √(a²+b²) dos dois. **É o teorema literal**." Não é uma metáfora frouxa: a fórmula do jogo **é** a fórmula geométrica. O mesmo documento registra a direção de VFX dada pelo próprio criador, fora do escopo desta extração de números (traça uma linha vertical e uma horizontal até o inimigo, e o poder sai pela diagonal) — citado aqui só para mostrar que a coerência "nome ↔ mecânica ↔ visual" foi pensada nos três eixos ao mesmo tempo, não só no código.

Este é também o precedente mecânico que outras cartas do catálogo reusam: o "ledger por rodada" que o Mandelbrot e a Ada compartilham (a memória do que aconteceu "nesta rodada", zerada na fronteira dela) foi justificado, em `_EFEITOS-ESCOLHIDOS.md` AMB-01, citando o Pitágoras como o precedente já implementado.

## Pontas soltas

- Nenhum parâmetro numérico desta carta é `//PLAYTEST`: a fórmula em si não é um valor afinável, é o teorema.
- A chave `CARD_EXEC_PYTHAGORAS_NAME` resolve para **"Hypotenuse"**.

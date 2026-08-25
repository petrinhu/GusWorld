<!--
SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
SPDX-License-Identifier: AGPL-3.0-or-later
-->

# Raiz Curativa (`raiz_cura`)

**Tipo Diátaxis:** Explanation. **Audiência:** design/engenharia deste projeto. **Last-reviewed:** 25/08/2026. **Owner:** `technical-writer`, extração do projeto anterior (L-01).

Dado completo em [`resources/cards/raiz_cura.gw.card`](../../../../resources/cards/raiz_cura.gw.card).

## Ficha rápida

Família **Bioquímico** · tipo-base **Raiz** · comum · mana 2 · poder 0 (sem dano) · alvo o próprio conjurador.

## Como funciona

Não causa dano. Aplica `Regen` (magnitude 8, 2 turnos, `Refresh`) no próprio conjurador — cura ao longo do tempo, não instantânea. É a única das 5 comuns do slice cujo alvo é `Self`.

## Por que é assim

O comentário-fonte marca só a classificação: "Bioquímico - Raiz: sem dano, aplica Regen no próprio caster" (`combat.md` §6/§9). O tipo-base **Raiz** (crescimento, cultivo) casa com a semântica de cura ao longo do tempo em vez de cura instantânea — a mesma leitura diegética que `cartas-technomagik.md` usa para a família Bioquímica em geral (nanobots que "religam o processo travado"), embora aquele documento descreva um sistema de Ampolas separado, não esta carta.

## Pontas soltas

- Mana 2, Regen magnitude 8/2 turnos: `origem não documentada` no legado.
- A chave `CARD_RAIZ_CURA_NAME` existe em `resources/translations/pt_br.md` e resolve para **"Raiz Curativa"**.
- Por que o alvo é só o próprio conjurador (`Self`), e não também aliado, não está explicado no legado — é uma limitação, não uma escolha registrada.

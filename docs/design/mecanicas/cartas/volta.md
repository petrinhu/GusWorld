<!--
SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
SPDX-License-Identifier: AGPL-3.0-or-later
-->

# Volt-Leech (`volta`)

**Tipo Diátaxis:** Explanation. **Audiência:** design/engenharia deste projeto. **Last-reviewed:** 25/08/2026. **Owner:** `technical-writer`, extração do projeto anterior (L-01).

Dado completo em [`resources/cards/volta.gw.card`](../../../../resources/cards/volta.gw.card).

## Ficha rápida

Família **Elétrico** · categoria **Ativa** · especial · mana 6 · alvo único · 1×/batalha.

## Como funciona

Ao causar dano, drena uma fração dele (50%, `kVoltaLeechPercent`) e devolve ao conjurador como recurso (mana e HP, segundo o design). O código-fonte não implementa mais nada além do gatilho `OnDamageDealt → Leech`: toda a nuance de "quanto vira mana, quanto vira HP, e o que se perde" é design ainda em aberto.

## Por que é assim

`cartas-technomagik.md` §9 (o item `VOLTA-LEECH-%`) já registra a decisão de design que motivou este número: o Volta virou um **leech termodinâmico** — dreno que absorve energia do alvo e devolve à party como mana **e** HP, mas só uma fração do absoluto drenado, "o resto perdido como calor" (2ª lei da termodinâmica). Essa é a leitura diegética completa; **não repetida aqui em detalhe** (ver a fonte). O que o código acrescenta é o número que ele efetivamente usava em runtime: `kVoltaLeechPercent = 50`, marcado `//PLAYTEST` — um valor provisório, não a resposta final ao "quanto exatamente converte" que o design ainda considera pendente.

Vale notar a garantia que o comentário-fonte faz questão de registrar: este leech **não fura a regra de "sem carry-over de mana"** do jogo — é dreno do inimigo, não retenção da própria mana não-gasta (um design anterior, descartado, propunha "mana não-gasta vira Shield"; esse caminho está morto).

## Pontas soltas

- O `x%` exato de conversão (o quanto realmente vira mana vs. HP, e a proporção entre os dois) **não está fechado** — é o item de brainstorm `VOLTA-LEECH-%` citado em `cartas-technomagik.md` §9, ainda pendente no momento desta extração.
- A chave `CARD_EXEC_VOLTA_NAME` resolve para **"Volt-Leech"** em `resources/translations/pt_br.md`.

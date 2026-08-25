<!--
SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
SPDX-License-Identifier: AGPL-3.0-or-later
-->

# Pulso Elétrico (`pulso_eletrico`)

**Tipo Diátaxis:** Explanation. **Audiência:** design/engenharia deste projeto, ao retomar a carta comum de Elétrico. **Last-reviewed:** 25/08/2026. **Owner:** `technical-writer`, extração do projeto anterior (L-01).

Dado completo em [`resources/cards/pulso_eletrico.gw.card`](../../../../resources/cards/pulso_eletrico.gw.card).

## Ficha rápida

Família **Elétrico** · tipo-base **Pulso** · comum · mana 1 · poder 6 · alvo único.

## Como funciona

Dano direto contra um único alvo, com uma aplicação curta de `Stun` (magnitude 2, 1 turno). É a carta mais barata de custo (mana 1) do conjunto de 5 comuns extraídas, e a segunda mais forte em poder bruto (6, atrás só do Impacto Cinético).

## Por que é assim

O código-fonte do projeto anterior não trazia comentário de design para os números desta carta — só a classificação estrutural ("dano direto + Stun (controle curto)", com referência às seções 6 e 9 de `combat.md`, que neste projeto correspondem a "Famílias de carta e roda de fraqueza" e "Status framework"). O padrão geral — Elétrico combinando dano com uma trava curta de Stun — é coerente com a intenção editorial que este projeto já registra sobre o Elétrico em `cartas-comuns-statlines.md` (uma proposta distinta e mais recente, de 30 cartas comuns, não confundir com esta carta de 5 do vertical slice), mas nenhuma linha liga formalmente esta carta específica àquela proposta.

## Pontas soltas

- **Nenhum número desta carta tem justificativa escrita no legado.** Mana 1, Power 6, Stun magnitude 2/1 turno: todos marcados `origem não documentada` no `.gw.card` irmão.
- A chave de tradução `CARD_PULSO_ELETRICO_NAME` existe em `resources/translations/pt_br.md` deste projeto e resolve para **"Pulso Elétrico"**.

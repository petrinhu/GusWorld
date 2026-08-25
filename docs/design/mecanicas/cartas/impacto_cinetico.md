<!--
SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
SPDX-License-Identifier: AGPL-3.0-or-later
-->

# `impacto_cinetico`

**Tipo Diátaxis:** Explanation. **Audiência:** design/engenharia deste projeto. **Last-reviewed:** 25/08/2026. **Owner:** `technical-writer`, extração do projeto anterior (L-01).

Dado completo em [`resources/cards/impacto_cinetico.gw.card`](../../../../resources/cards/impacto_cinetico.gw.card).

## Ficha rápida

Família **Cinético** · tipo-base **Pulso** · comum · mana 1 · poder 7 (o mais alto das 5 comuns) · alvo único.

## Como funciona

O maior dano direto das 5 cartas comuns, ao menor custo de mana (1), com aplicação de `Knockback` (magnitude 1, 1 turno, `Replace`) — reposiciona o alvo, mexendo na fila de iniciativa.

## Por que é assim

O comentário-fonte é explícito sobre a intenção comparativa: "Cinético - Pulso: **maior** dano direto + Knockback (reposiciona)". É a única das 5 cujo comentário de origem se define **por comparação com as outras** (não só descreve a própria carta) — o poder 7 é deliberadamente o teto do grupo. O resto (por que 7 e não 8, por que Knockback magnitude 1) não tem justificativa escrita além disso.

## Pontas soltas

- Mana 1, Power 7, Knockback magnitude 1/1 turno: sem justificativa numérica além da comparação "maior dano direto" já citada.
- A chave `CARD_IMPACTO_CINETICO_NAME` **não tem** entrada em `resources/translations/pt_br.md` hoje (mesma situação de `eco_sonico`/`fenda_criptica`).

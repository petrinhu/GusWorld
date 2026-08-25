<!--
SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
SPDX-License-Identifier: AGPL-3.0-or-later
-->

# `fenda_criptica`

**Tipo Diátaxis:** Explanation. **Audiência:** design/engenharia deste projeto. **Last-reviewed:** 25/08/2026. **Owner:** `technical-writer`, extração do projeto anterior (L-01).

Dado completo em [`resources/cards/fenda_criptica.gw.card`](../../../../resources/cards/fenda_criptica.gw.card).

## Ficha rápida

Família **Criptográfico** · tipo-base **Fenda** · comum · mana 2 · poder 5 · alvo único.

## Como funciona

Dano contra um único alvo, com aplicação de `Expose` (magnitude 30, 2 turnos, `Refresh`) — um debuff que, em `combat.md` §9 deste projeto, aumenta o dano de carta que o alvo recebe (multiplicador que outras cartas exploram).

## Por que é assim

O comentário-fonte descreve só a classificação: "Criptográfico - Fenda: dano + Expose (abre o alvo a mais dano)". O nome do tipo-base, **Fenda**, é literal: a carta abre uma brecha (o `Expose`) que outras cartas/aliados exploram depois — a mesma lógica de sinergia que a proposta mais recente de 30 comuns (`cartas-comuns-statlines.md`, não a mesma carta, ver nota abaixo) generaliza como `SynergyStatus`.

## Pontas soltas

- **Nome de exibição não documentado**, mesma situação de `eco_sonico`: a chave `CARD_FENDA_CRIPTICA_NAME` não tem entrada em `resources/translations/pt_br.md` hoje.
- Mana 2, Power 5, Expose magnitude 30/2 turnos: `origem não documentada`.
- **Não confundir com a proposta de 30 cartas comuns** (`cartas-comuns-statlines.md`, família Criptográfica de Iara "Lumen"): aquele documento é uma proposta de design mais recente e maior (2026-07-16), com números e nomes próprios (`Rimin-Brecha`, etc.), sem vínculo declarado com esta carta de 5 do vertical slice.

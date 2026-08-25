<!--
SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
SPDX-License-Identifier: AGPL-3.0-or-later
-->

# `eco_sonico`

**Tipo Diátaxis:** Explanation. **Audiência:** design/engenharia deste projeto. **Last-reviewed:** 25/08/2026. **Owner:** `technical-writer`, extração do projeto anterior (L-01).

Dado completo em [`resources/cards/eco_sonico.gw.card`](../../../../resources/cards/eco_sonico.gw.card).

## Ficha rápida

Família **Sônico** · tipo-base **Eco** · comum · mana 1 · poder 4 (o mais leve das 5) · alvo único.

## Como funciona

Dano leve contra um único alvo, com aplicação de `Disrupt` (magnitude 20, 1 turno, `Replace`) — um debuff de magnitude, que em `combat.md` §9 (Status framework deste projeto) reduz o poder da próxima ação do alvo.

## Por que é assim

O comentário-fonte descreve só a classificação: "Sônico - Eco: dano leve + Disrupt (debuff de magnitude)". Não há explicação registrada de por que o tipo-base é **Eco** (ressonância) nem por que o poder é o mais baixo das 5 — plausivelmente porque o Disrupt já carrega o peso ofensivo (reduzir a próxima ação do alvo vale como dano indireto), mas isso é leitura minha, não fato do legado, e por isso não entra como afirmação no corpo do documento.

## Pontas soltas

- **Nome de exibição não documentado.** `resources/translations/pt_br.md` deste projeto **não tem** entrada para a chave `CARD_ECO_SONICO_NAME` — ao contrário de `pulso_eletrico` e `raiz_cura`, esta carta (e as duas seguintes) não têm texto em português cadastrado hoje. A chave existe (é o valor literal do campo `display_name` no registro de origem), só o texto que ela resolveria está ausente.
- Mana 1, Power 4, Disrupt magnitude 20/1 turno: `origem não documentada`.

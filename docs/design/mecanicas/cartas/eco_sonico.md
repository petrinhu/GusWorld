<!--
SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
SPDX-License-Identifier: AGPL-3.0-or-later
-->

# `eco_sonico`

**Tipo Diátaxis:** Explanation. **Audiência:** design/engenharia deste projeto. **Last-reviewed:** 25/08/2026. **Owner:** `technical-writer`, extração do projeto anterior (L-01).

Dado completo em [`resources/cards/eco_sonico.gw.card`](../../../../resources/cards/eco_sonico.gw.card).

## Ficha rápida

Família **Sônico** · tipo-base **Eco** · comum · mana 2 · poder 4 (o mais leve das 5) · alvo único.

## Como funciona

Dano leve contra um único alvo, com aplicação de `Disrupt` (magnitude 20, 1 turno, `Replace`) — um debuff de magnitude, que em `combat.md` §9 (Status framework deste projeto) reduz o poder da próxima ação do alvo.

## Por que é assim

O comentário-fonte descreve só a classificação: "Sônico - Eco: dano leve + Disrupt (debuff de magnitude)". Não há explicação registrada de por que o tipo-base é **Eco** (ressonância) nem por que o poder é o mais baixo das 5 — plausivelmente porque o Disrupt já carrega o peso ofensivo (reduzir a próxima ação do alvo vale como dano indireto), mas isso é leitura minha, não fato do legado, e por isso não entra como afirmação no corpo do documento.

**Ajuste de custo, decisão do líder em 02/09/2026:** o mana subiu de 1 para 2 (poder 4 mantido). Esta comum batia tão forte quanto a contraparte dela na proposta de 30 cartas comuns pelo mesmo custo, o que a tornava estritamente melhor; o ajuste faz bater mais forte custar mais bateria. A mesma decisão baixou o poder da Lhinin-Estalo (a contraparte de família Sônico na proposta de 30 comuns) de 3 para 2, em `cartas-comuns-statlines.md`.

## Pontas soltas

- **Nome de exibição não documentado.** `resources/translations/pt_br.md` deste projeto **não tem** entrada para a chave `CARD_ECO_SONICO_NAME` — ao contrário de `pulso_eletrico` e `raiz_cura`, esta carta (e as duas seguintes) não têm texto em português cadastrado hoje. A chave existe (é o valor literal do campo `display_name` no registro de origem), só o texto que ela resolveria está ausente.
- **Nenhum número original desta carta tem justificativa escrita no legado.** Power 4, Disrupt magnitude 20/1 turno: `origem não documentada`. O mana (hoje 2) é ajuste de balanceamento do líder, não herança do legado.

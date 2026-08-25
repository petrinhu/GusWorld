<!--
SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
SPDX-License-Identifier: AGPL-3.0-or-later
-->

# Free-Order (`hayek`)

**Tipo Diátaxis:** Explanation. **Audiência:** design/engenharia deste projeto. **Last-reviewed:** 25/08/2026. **Owner:** `technical-writer`, extração do projeto anterior (L-01).

Dado completo em [`resources/cards/hayek.gw.card`](../../../../resources/cards/hayek.gw.card).

## Ficha rápida

Família **Universal** · categoria **Passiva** · especial · mana 0 · equip-only, nunca jogada. Beneficia o **lado inteiro**, não só o dono, enquanto ele estiver vivo e a carta equipada.

## Como funciona

Conta quantas ações **diferentes** os membros do lado do portador já fizeram na rodada corrente antes da ação atual (dois ataques básicos contam como iguais; duas cartas de famílias diferentes contam como diferentes). Em três degraus crescentes de diversidade (2, 3 e 4-ou-mais assinaturas distintas), o lado inteiro ganha um bônus de dano cada vez maior e uma redução no limiar de falha. **Nunca pune** — não existe degrau negativo, e o mínimo é sempre "sem bônus", nunca "com penalidade".

## Por que é assim

O nome e a mecânica vêm da "ordem espontânea" de Hayek: "Hayek mostrou que a melhor ordem nasce sozinha, sem um chefe mandando. Sua party rendeu mais porque cada um fez algo diferente" (`_EFEITOS-ESCOLHIDOS.md`). A definição de "ação diferente" não foi trivial — `_EFEITOS-ESCOLHIDOS.md` AMB-09 registra que a redação original não dizia se dois ataques básicos contam como iguais, se duas cartas da mesma família contam como iguais, quem colhe o benefício (só o dono ou o time todo) e quantos degraus existem. A métrica escolhida (assinatura de `CombatActionType`, refinada pela família da carta quando a ação é jogar carta) e o "beneficia o lado inteiro" vieram dessas decisões.

Um detalhe de engenharia vale registrar porque é um padrão que se repete em outras passivas do catálogo (Planck, Mises): esta carta **não passa pelo dispatcher genérico de efeitos** — o bônus pluga direto na fórmula de dano e no limiar de falha, porque ela nunca é "jogada" no sentido de OnCast disparar de verdade (é equip-only). O `EffectSpec` existe só como dado de catálogo, marcado como "sem handler real" no comentário-fonte.

## Pontas soltas

- Os três degraus (2/3/8+ assinaturas → +5%/+8%/+13% de dano, -1/-2/-3pp de falha) estão marcados `//PLAYTEST`, com um aviso explícito no código-fonte para não rotulá-los como "easter-egg" em comentário ou commit — o repositório é público.
- A chave `CARD_EXEC_HAYEK_NAME` resolve para **"Free-Order"**.

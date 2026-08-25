<!--
SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
SPDX-License-Identifier: AGPL-3.0-or-later
-->

# `urandom`

**Tipo Diátaxis:** Explanation. **Audiência:** design/engenharia deste projeto. **Last-reviewed:** 25/08/2026. **Owner:** `technical-writer`, extração do projeto anterior (L-01).

Dado completo em [`resources/cards/urandom.gw.card`](../../../../resources/cards/urandom.gw.card).

## Ficha rápida

Família **Universal** · categoria **Ativa** · especial · mana **0** · alvo próprio conjurador (o alvo real é sorteado à parte).

## Como funciona

Ao contrário das outras 19, o "programa" real desta carta não vive dentro do registro de efeitos — vive em **duas tabelas de peso**, fora do escopo deste registro: uma para a carta em versão **original**, outra para a versão **pirata**. Ao ser jogada, sorteia por peso uma faixa (fraco/médio/forte/especial, e para a pirata também um resultado de "backfire") e redireciona a execução para uma carta **já existente** no catálogo — nunca inventa um efeito novo. O `EffectSpec` desta carta existe só para que esse redirecionamento seja disparado por um gatilho de vocabulário compartilhado, em vez de um `if` amarrado ao id literal desta única carta (um achado de auditoria do próprio projeto anterior).

Os pesos, extraídos de `cartas-numeros-proposta.md` §4 deste projeto (já existente, não duplicado aqui em detalhe): a versão **original** sorteia 21/34/21/8 (fraco/médio/forte/especial, de um total de 84); a versão **pirata** sorteia 7/2/1/0/5 de um total de 15 (fraco/médio/forte/jackpot/backfire) — um **backfire exato de 1/3** (33,3%), fechado pelo líder especificamente para bater essa fração, com crédito de autoria explícito ao Gus Dragon (colaborador humano, filho do líder, cross-ref L-07/L-16 de `GODS_LAWS.md`).

## Por que é assim

Esta não é uma carta de mestre histórico — é "a carta-caos do Gus" (o protagonista), e o comentário-fonte marca isso: `urandom` é o nome de uma fonte de números aleatórios de sistema, não o nome de uma pessoa. A ideia e o número exato do backfire pirata (1/3) vieram do próprio Gus Dragon como playtester, segundo o comentário-fonte (`CARDS-HW-2B`).

## Pontas soltas

- **`urandom` não é um dos "20 mestres" no sentido narrativo do roster** (`_EFEITOS-ESCOLHIDOS.md` lista 20 mestres terminando em "Bastiat", que **não** aparece no catálogo de cartas implementado — e o catálogo implementado inclui `urandom`, que não é um mestre). As duas listas de "20" não são a mesma lista: uma é o roster narrativo de figuras históricas: Bastiat tem efeito e nome de carta desenhados ("Hidden-Cost") mas **nunca foi implementado** no registro de cartas; a outra é o catálogo de 20 cartas que o código de fato registra, que troca Bastiat por `urandom`. Este achado vale para o `_INDEX.md` desta pasta, não só para este arquivo.
- A chave `CARD_EXEC_URANDOM_NAME` resolve para **"urandom"**, literal, minúsculo — o único nome de carta que não segue nem o padrão de figura (`cardExec-[nome real]`) nem o de alcunha técnica capitalizada das outras 19.
- O mecanismo exato de escolha de tier dentro do redirecionamento (record-base vs. execução pelo motor de efeitos, conforme o tier sorteado) não está detalhado no material lido — o comentário-fonte descreve a existência do branch dedicado, não o algoritmo interno dele.

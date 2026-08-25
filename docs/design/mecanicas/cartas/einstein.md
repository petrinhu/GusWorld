<!--
SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
SPDX-License-Identifier: AGPL-3.0-or-later
-->

# `einstein`

**Tipo Diátaxis:** Explanation. **Audiência:** design/engenharia deste projeto. **Last-reviewed:** 25/08/2026. **Owner:** `technical-writer`, extração do projeto anterior (L-01).

Dado completo em [`resources/cards/einstein.gw.card`](../../../../resources/cards/einstein.gw.card).

## Ficha rápida

Família **Cinético** · categoria **Ativa** · especial · mana 6 · 1×/batalha.

## Como funciona

Empurra a ação de um inimigo para o **fim da fila** da rodada corrente — ele age por último, depois de toda a party e dos outros inimigos. Se o alvo **já agiu** nesta rodada quando a carta é jogada, ela simplesmente dissipa (não fica "guardada" para empurrar o alvo na rodada seguinte). Mirar um aliado é o espelho benéfico: em vez de atrasar, o aliado **avança**, agindo logo depois do ator atual.

## Por que é assim

A família **Cinético** é uma escolha deliberada, e não a família elétrica que se poderia esperar do nome: o comentário-fonte explica que "a dilatação temporal empurra a fila, mesma família das cartas COMUNS de reordenar/knockback" — a carta compartilha identidade mecânica com reposicionamento, não com física de partícula.

A tradução de "empurra a próxima ação dele pra depois do próximo turno da party" (a redação de design original) para "fim da fila da rodada" (a implementação real) não foi óbvia — `_EFEITOS-ESCOLHIDOS.md` AMB-02 registra a ambiguidade: o motor de combate deste jogo é por-turno-de-ator, numa fila de iniciativa única, sem um "turno da party" como bloco separado. A decisão do líder escolheu "fim da fila da rodada" por três motivos: é a leitura mais fiel à intenção ("a party toda age antes dele"); não depende de um número mágico de posições que precisaria escalar com o tamanho da fila; e reusa a mesma primitiva de reordenação que o Gambito-Reordenar já usa. O modo-aliado (espelho benéfico) foi decisão complementar do mesmo dia, registrada no addendum de AMB-02.

## Pontas soltas

- **A chave de tradução em uso (`CARD_EXEC_EINSTEIN_NAME`) resolve literalmente para "Einstein"**, não para a alcunha de efeito "Time-Dilate" que `_EFEITOS-ESCOLHIDOS.md` registra — mesma situação da carta `tesla` (ver `tesla.md`, seção Pontas soltas, para a leitura completa da inconsistência).
- A frase pedagógica canônica do Time-Dilate narra só o caso inimigo; falta a variante para o caso aliado (`_EFEITOS-ESCOLHIDOS.md`, addendum AMB-02) — pendência de conteúdo, não de código.

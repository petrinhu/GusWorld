<!--
SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
SPDX-License-Identifier: AGPL-3.0-or-later
-->

# Null-Proof (`godel`)

**Tipo Diátaxis:** Explanation. **Audiência:** design/engenharia deste projeto. **Last-reviewed:** 25/08/2026. **Owner:** `technical-writer`, extração do projeto anterior (L-01).

Dado completo em [`resources/cards/godel.gw.card`](../../../../resources/cards/godel.gw.card).

## Ficha rápida

Família **Universal** · categoria **Ativa** · especial · mana **0** · 1×/batalha.

## Como funciona

Duas vias fura-defesa **independentes**, que não se tocam:

1. **A flag `ignores_weakness_wheel`**, sempre ligada enquanto a carta está no catálogo do portador: qualquer carta marcada com esta flag sempre resolve como se o alvo fosse neutro à roda de fraqueza (mult 1.0), sem depender de nenhum status.
2. **O status `NullProof`**, concedido ao ser jogada (ao próprio Gödel ou a um aliado). No **próximo** golpe do portador contra um alvo Resistente **ou** Imune, o motor força o multiplicador a 1.0 e **consome** (remove) o status. Contra um alvo Neutro ou Fraco, o status **não é gasto** — só se consome quando há algo de fato a furar.

## Por que é assim

O nome e a mecânica vêm direto do teorema da incompletude: "Gödel mostrou que toda lógica tem verdades que ela mesma não alcança. A defesa do inimigo é uma lógica, e sua jogada está fora dela" (`_EFEITOS-ESCOLHIDOS.md`). A carta nasceu como um trunfo pessoal do mestre — a flag `ignores_weakness_wheel` já existia no registro antes de ter qualquer forma jogável (`_EFEITOS-ESCOLHIDOS.md` AMB-05: "a flag nunca tinha WIRING real no resolvedor... decisão adiada"). A segunda via (o status castável) resolveu quatro perguntas em aberto de uma vez (decisões G-1 a G-3 do líder, 2026-07-15): se Gödel vira carta jogável de verdade (sim, mana 0), se o benefício é permanente ou um recurso limitado (limitado — um "tiro guardado"), contra o quê ele fura (os dois tiers abaixo de neutro, Resistente **e** Imune, sem distinção — "o código não precisa checar qual tier: basta mult < 1.0"), e se consome sempre ou só quando há algo a furar (só quando há algo a furar).

A duração alta (99) não é um número de balance — é uma sentinela deliberada: a saída real do status é por **consumo** no golpe certo, não por expiração de turnos, então uma duração "normal" (3, 5 turnos) seria enganosa.

## Pontas soltas

- O tier "Imune" (mult 0.0) não é alcançável hoje por nenhuma combinação real de família de carta no legado — um limite de cobertura honesto registrado em `_EFEITOS-ESCOLHIDOS.md` AMB-05, não uma lacuna desta extração.
- A chave `CARD_EXEC_GODEL_NAME` resolve para **"Null-Proof"**.

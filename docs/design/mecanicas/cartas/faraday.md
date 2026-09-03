<!--
SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
SPDX-License-Identifier: AGPL-3.0-or-later
-->

# EM-Shield (`faraday`)

**Tipo Diátaxis:** Explanation. **Audiência:** design/engenharia deste projeto. **Last-reviewed:** 25/08/2026. **Owner:** `technical-writer`, extração do projeto anterior (L-01).

Dado completo em [`resources/cards/faraday.gw.card`](../../../../resources/cards/faraday.gw.card).

## Ficha rápida

Família **Elétrico** · categoria **Híbrida** · especial · mana 6 · 1×/batalha (face de combate) · face passiva com dois gastos de mana/bateria (standby + disparo, números pendentes; decisão do líder, 03/09/2026, ver `cartas-hardware-pirataria-energia.md` §5).

## Como funciona

Ao ser jogada num aliado (incluindo o próprio conjurador), aplica `BlindagemEM` por 3 turnos: **previne** a aplicação de qualquer novo debuff de origem Elétrica no portador **e limpa imediatamente** os debuffs elétricos que já estivessem ativos nele. Mirar um inimigo faz a carta dissipar — o benefício não "vaza" para o lado oposto.

## Por que é assim

A carta nasceu **fora de combate**: o efeito original escolhido (`_EFEITOS-ESCOLHIDOS.md`, Lote 1) era só a "passiva anti-PEM" — anula o pulso eletromagnético das dungeons, habilitando save onde há PEM. Essa face fora-de-combate **continua existindo** no design, mas **não tem programa implementado** ("feat futura", sem wiring) — não é um corte, é trabalho ainda não feito.

A face de combate nasceu depois, por decisão do líder em 2026-07-15 (`_EFEITOS-ESCOLHIDOS.md` AMB-03, decisões F-1 a F-4), respondendo quatro perguntas que a redação original deixava em aberto: o filtro de imunidade cobre só debuff (não buff) de origem elétrica (F-1); a blindagem previne **e** limpa o que já estava ativo (F-2); a carta vira Híbrida, ganhando a face castável sem tocar a original (F-3); e mirar um inimigo dissipa, para o benefício nunca vazar pro lado oposto (F-4). O filtro de lado que resolveu a pergunta F-4 (`SideFilter`) virou depois a primitiva genérica que o Newton herdou pronta.

## Pontas soltas

- A duração de 3 turnos está marcada `//PLAYTEST`.
- A face fora-de-combate (anti-PEM) segue sem nome mecânico de wiring definido — cross-ref `project_save_dungeon_pem_faraday` no legado, um documento que não foi trazido para este projeto (fora do escopo desta extração de cartas).
- A chave `CARD_EXEC_FARADAY_NAME` resolve para **"EM-Shield"**.

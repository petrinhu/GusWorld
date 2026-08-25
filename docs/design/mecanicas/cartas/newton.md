<!--
SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
SPDX-License-Identifier: AGPL-3.0-or-later
-->

# Force-Law (`newton`)

**Tipo Diátaxis:** Explanation. **Audiência:** design/engenharia deste projeto. **Last-reviewed:** 25/08/2026. **Owner:** `technical-writer`, extração do projeto anterior (L-01).

Dado completo em [`resources/cards/newton.gw.card`](../../../../resources/cards/newton.gw.card).

## Ficha rápida

Família **Universal** · categoria **Híbrida** · especial · mana 6 · alvo **Grupo** · 1×/batalha (face ativa).

## Como funciona

A carta tem **duas leis reais do Newton**, cada uma virando uma face mecânica distinta, roteada pelo lado do alvo:

- **Contra um inimigo** ("Poço Gravitacional"): imobiliza (`Stun`, 1 turno) **todos** os inimigos vivos do grupo adversário. O dano que acompanha é só o ATK do conjurador — a carta em si não carrega poder próprio (`power = 0`).
- **A favor de um aliado** ("Ação e Reação", modo-benefício): não ataca ninguém, concede `Reflect` (30%, 3 turnos) ao aliado alvo.
- **Passiva própria, sempre ligada** (`OnDamageReceived → Reflect`): o dono da carta reflete uma fração do dano que recebe, independente das duas faces acima. Se o dono também estiver com o status `Reflect` da segunda face (por exemplo, um auto-cast), **as duas fontes somam** — não há dedup, cada uma dispara no seu próprio ponto do motor.

Mirar o lado errado em qualquer uma das duas primeiras faces faz a carta **dissipar** (não lança, não erra, simplesmente não acontece — com registro do porquê).

## Por que é assim

Este é o exemplo mais completo, no catálogo, do princípio "toda carta com duas faces roteia por filtro de lado, e dissipa no lado errado sem exceção e sem crash" — o comentário-fonte é explícito: "Duas faces, roteadas por `side_filter` (ambas dissipam-no-lado-errado, sem erro e sem exceção)". A forma AoE da face ofensiva não era a implementação original: o design (`_EFEITOS-ESCOLHIDOS.md`, AMB-04) registra que a versão anterior tratava o Poço como alvo único, "herdado do template genérico de carta especial", e a decisão do líder em 2026-07-15 (N-1 a N-4) generalizou para o grupo inteiro — mais fiel à física real (um poço gravitacional não escolhe 1 vítima).

**Achado colateral que veio junto, e vale registrar porque não é exclusivo desta carta:** ao implementar o Newton, a auditoria descobriu que `resolve_use_card` somava o ataque do conjurador ao dano-base **mesmo quando o alvo era do próprio time** — um bug pré-existente que já quebrava silenciosamente os modos-benefício de outras cartas (Einstein, Faraday). A correção virou regra geral do motor ("fogo amigo desligado": nenhuma carta causa dano-base num alvo do próprio time), não um remendo local do Newton.

## Pontas soltas

- A fração exata do Reflect (30%) e a duração (3 turnos) estão marcadas `//PLAYTEST` — não é número fechado pelo líder, é o valor que o código usava.
- A chave `CARD_EXEC_NEWTON_NAME` resolve para **"Force-Law"**.
- Detalhe completo da decisão N-1..N-4 e do achado do fogo amigo: `docs/design/roster-analogos/_EFEITOS-ESCOLHIDOS.md`, AMB-04 (não duplicado aqui).

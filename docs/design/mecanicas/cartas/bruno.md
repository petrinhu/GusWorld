<!--
SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
SPDX-License-Identifier: AGPL-3.0-or-later
-->

# Echo-Self (`bruno`)

**Tipo Diátaxis:** Explanation. **Audiência:** design/engenharia deste projeto. **Last-reviewed:** 25/08/2026. **Owner:** `technical-writer`, extração do projeto anterior (L-01).

Dado completo em [`resources/cards/bruno.gw.card`](../../../../resources/cards/bruno.gw.card).

## Ficha rápida

Família **Universal** · categoria **Ativa** · especial · mana 6 · alvo **próprio conjurador**, sempre · 1×/batalha.

## Como funciona

Aplica ao próprio conjurador o mesmo status `Eco` que o von Neumann concede a um aliado à escolha — mas Giordano Bruno é **self-only** por desenho (alvo fixo em si mesmo, não há opção de mirar outro aliado), e os números são diferentes: 62% de reaplicação por 2 turnos (contra 50%/3 turnos do von Neumann).

## Por que é assim

O nome vem do filósofo queimado por defender infinitos mundos: "Giordano Bruno foi queimado por dizer que existem infinitos mundos. Em um deles vive outro você, e ele veio lutar ao seu lado" (`_EFEITOS-ESCOLHIDOS.md`) — "a imagem de dois Gus". A escolha de números diferentes do von Neumann (62%/2 turnos, contra 50%/3) não é arbitrária: o comentário-fonte explica que ela existe deliberadamente **"pra evitar colisão"** com o clone de qualquer-aliado do von Neumann — as duas cartas usam a mesma mecânica base, mas precisavam de identidades numéricas distintas para não parecerem a mesma carta com nome trocado. `_EFEITOS-ESCOLHIDOS.md` (Lote 4) registra o mesmo racional na escolha original do efeito.

## Pontas soltas

- A percentagem de eco (62%) e a duração (2 turnos) estão marcadas `//PLAYTEST`.
- Mesmo requisito de visual obrigatório do von Neumann (sprite-eco translúcido), mesma dependência da camada de apresentação ainda não existente.
- A chave `CARD_EXEC_BRUNO_NAME` resolve para **"Echo-Self"**.

<!--
SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
SPDX-License-Identifier: AGPL-3.0-or-later
-->

# Fork (`vonneumann`)

**Tipo Diátaxis:** Explanation. **Audiência:** design/engenharia deste projeto. **Last-reviewed:** 25/08/2026. **Owner:** `technical-writer`, extração do projeto anterior (L-01).

Dado completo em [`resources/cards/vonneumann.gw.card`](../../../../resources/cards/vonneumann.gw.card).

## Ficha rápida

Família **Universal** · categoria **Híbrida** · especial · mana 6 · 1×/batalha.

## Como funciona

Duas faces no mesmo cast:

1. **"Molde Fiel":** aplica o status `Eco` num aliado à escolha (self incluído), por 3 turnos próprios do portador. Ao fim de cada um desses turnos, o eco reaplica **50%** do último dano que aquele aliado causou. Mirar um inimigo dissipa.
2. **"Construtor Universal":** enquanto a carta está equipada, a **primeira** carta especial ativa/híbrida jogada na batalha (inclusive esta própria) não gasta o uso único de 1×/batalha — "se reconstrói", e o refund em si só acontece uma vez por batalha.

## Por que é assim

O nome vem da autorreplicação: "von Neumann imaginou uma máquina que se copia sozinha. Sua jogada virou dado, e pôde ser lida outra vez" (`_EFEITOS-ESCOLHIDOS.md`). O efeito escolhido (Lote 4) já nascia com duas descobertas reais do mestre numa carta só: o "Construtor Universal" (a máquina autorreplicante) e o "Molde Fiel" (clonar um aliado por N turnos, com stats reduzidos na concepção original — na implementação real, virou reaplicação parcial de dano via status, não uma unidade extra na fila).

O mecanismo de clone **não** adiciona um ator novo à fila de combate — o comentário-fonte de `bruno.gw.card` (a carta irmã) registra explicitamente que essa era a decisão original ("entidade-Objeto", um 4º ator), superada por uma versão mais simples orientada a status: reaplicar o último golpe do próprio portador, sem entidade nova nenhuma. A vantagem prática dessa mudança é não mexer no tamanho fixo da party (3, canon deste projeto).

## Pontas soltas

- A percentagem de eco (50%) e a duração (3 turnos) estão marcadas `//PLAYTEST`.
- O visual do clone (um "sprite-eco translúcido" num slot de batalha) é **obrigatório** por decisão de design, mas é responsabilidade da camada de apresentação — não existe hoje, porque a camada de apresentação deste projeto (L-06) só nasce quando o GlintFx tiver janela e desenho.
- A chave `CARD_EXEC_VONNEUMANN_NAME` resolve para **"Fork"**.

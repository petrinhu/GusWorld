<!--
SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
SPDX-License-Identifier: AGPL-3.0-or-later
-->

# Black-Mirror (`dee`)

**Tipo Diátaxis:** Explanation. **Audiência:** design/engenharia deste projeto. **Last-reviewed:** 25/08/2026. **Owner:** `technical-writer`, extração do projeto anterior (L-01).

Dado completo em [`resources/cards/dee.gw.card`](../../../../resources/cards/dee.gw.card).

## Ficha rápida

Família **Universal** · categoria **Híbrida** · especial · mana 6 · alvo **próprio conjurador** · 1×/batalha (face ativa).

## Como funciona

A carta tem **três** faces, mas só uma tem programa de combate implementado:

1. **Scrying (ativa, implementada):** aplica o buff `Scrying` no próprio conjurador (3 turnos) e revela, sem gastar sorteio, a intenção prevista de cada inimigo vivo. Enquanto **qualquer** aliado vivo portar o buff, a intenção é revelada de novo a cada fronteira de rodada — não é preciso jogar a carta de novo.
2. **Scan aprimorado (passiva, implementada):** enquanto a carta está equipada, a ação de Scan do jogo passa a revelar também status ativos e posição na fila do alvo escaneado, além do que já revelava.
3. **Espelho Negro (fora de combate, NÃO implementada):** a face de mundo original — revelar baús e passagens ocultas na exploração — fica como registro posse-only, porque o sistema de "ocultos no overworld" ainda não existe em nenhum projeto.

Contra um inimigo de comportamento caótico (um "boss" cujo padrão é deliberadamente imprevisível), a Scrying **não fura essa proteção**: devolve ruído em vez do dado real, tanto no dump quanto no Scan aprimorado.

## Por que é assim

O nome vem do ocultista histórico: "John Dee consultava um espelho negro para ver o que ainda não tinha acontecido. Agora você vê o próximo golpe do inimigo" (`_EFEITOS-ESCOLHIDOS.md`). A decisão de manter o comportamento caótico opaco (`_EFEITOS-ESCOLHIDOS.md` AMB-07, D2) não é tratada como uma limitação da carta: é telegraph **honesto** de um sistema que é, por definição, imprevisível — "a Scrying não é um trunfo genérico, é telegraph honesto, e telegraph honesto de um sistema caótico é ruído por definição". A escolha de revelar só dados que **já existem** no modelo (status, posição na fila, intenção prevista) e nunca inventar um atributo oculto novo foi restrição explícita do criador.

## Pontas soltas

- A face de mundo "Espelho Negro" está sem sistema para se apoiar — não é corte de escopo, é dependência de um sistema (ocultos no overworld) que ainda não existe em lugar nenhum.
- A duração do Scrying (3 turnos) é um número **fechado pelo líder**, não `//PLAYTEST`-livre — mesma categoria dos números do Planck.
- A chave `CARD_EXEC_DEE_NAME` resolve para **"Black-Mirror"**.

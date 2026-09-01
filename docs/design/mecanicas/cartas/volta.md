<!--
SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
SPDX-License-Identifier: AGPL-3.0-or-later
-->

# Volt-Leech (`volta`)

**Tipo Diátaxis:** Explanation. **Audiência:** design/engenharia deste projeto. **Last-reviewed:** 31/08/2026. **Owner:** `technical-writer`; mecânica de recurso fechada por decisão do líder (dado herdado do projeto anterior via L-01 está revogado, ver abaixo).

Dado completo em [`resources/cards/volta.gw.card`](../../../../resources/cards/volta.gw.card). ⚠️ **Esse arquivo ainda carrega o número e o gatilho revogados** (`percent: 50`, `trigger: OnDamageDealt`) — atualizá-lo é fatia própria, fora deste documento (dado de carta, não canon de design).

## Ficha rápida

Família **Elétrico** · categoria **Ativa** · especial · mana 6 · alvo único · 1×/batalha.

## Como funciona

Ao ser usada, drena **21% da energia do ALVO** por uso (porcentagem do alvo, não valor absoluto). **55% do drenado converte e chega à party**; os outros **45% se perdem como calor** (2ª lei da termodinâmica). No momento do uso, **o jogador escolhe** se o retorno vira mana ou vida — a escolha é **cronometrada por dificuldade**: Fácil 13s, Médio 8s, Difícil 5s, Hardcore 3s. Esgotado o tempo sem escolha, a energia dissipa e o jogador **não recebe nada** (nem mana, nem vida).

## Por que é assim

Especificação fechada pelo líder em 31/08/2026, por `AskUserQuestion`, verbatim: *"o volta fica como decidi agora, achei melhor"*. Fonte canônica: `docs/design/roster-analogos/_EFEITOS-ESCOLHIDOS.md` linha 14 (o item `VOLTA-LEECH-%` de `cartas-technomagik.md` §9, agora RESOLVIDO). Racional do líder: percentual em vez de absoluto, para a carta seguir relevante do início ao fim da campanha sem escalar número, e casar com a física (quanto mais energia há no sistema, mais se pode extrair); 21% e 55% são Fibonacci, coerentes com o resto da economia; a escolha cronometrada nasce de ordem direta dele — *"ele tem x segundos para escolher ou a energia dissipa e ele perde. x depende da dificuldade"*.

Vale notar a garantia que já valia antes e continua valendo: este leech **não fura a regra de "sem carry-over de mana"** do jogo — é dreno do inimigo, não retenção da própria mana não-gasta (um design anterior, descartado, propunha "mana não-gasta vira Shield"; esse caminho segue morto).

**Revogado em 31/08/2026, por decisão do líder:** a leitura anterior descrita aqui — gatilho `OnDamageDealt → Leech`, drenando uma fração do **DANO CAUSADO**, e a constante `kVoltaLeechPercent = 50` — está morta. O dreno é sobre a **energia do alvo**, em porcentagem, nunca sobre o dano.

Duas consequências desta decisão, registradas e **não resolvidas aqui**: o combate por turnos ganha um elemento em tempo real, e a escolha cronometrada é barreira de acessibilidade conhecida. Detalhe completo em `_EFEITOS-ESCOLHIDOS.md` linha 14.

## Pontas soltas

- ⚠️ **MARCADO PARA O LÍDER, NÃO RESOLVIDO AQUI:** a Ficha Rápida (acima) registra a carta como **1×/batalha**. Se essa restrição continuar valendo tal como está, os 21% de dreno acontecem **uma única vez por combate**, e a carta deixa de ser a sustentação REPETÍVEL que foi descrita a ele no momento em que escolheu o número (`_EFEITOS-ESCOLHIDOS.md` linha 14 já registra: "a carta é sustentação, não execução — precisa de vários usos para esvaziar um alvo"). A interação entre "21% por uso" e "1×/batalha" precisa da revisão dele. Já foi avisado pelo orquestrador. Não alterada aqui.
- `resources/cards/volta.gw.card` segue com o número e o gatilho revogados (ver aviso no topo deste documento) — atualização do dado é tarefa separada, não feita aqui.
- A chave `CARD_EXEC_VOLTA_NAME` resolve para **"Volt-Leech"** em `resources/translations/pt_br.md` — sem mudança.

<!--
SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
SPDX-License-Identifier: AGPL-3.0-or-later
-->

# Volt-Leech (`volta`)

**Tipo Diátaxis:** Explanation. **Audiência:** design/engenharia deste projeto. **Last-reviewed:** 31/08/2026. **Owner:** `technical-writer`; mecânica de recurso fechada por decisão do líder (dado herdado do projeto anterior via L-01 está revogado, ver abaixo).

Dado completo em [`resources/cards/volta.gw.card`](../../../../resources/cards/volta.gw.card). ⚠️ **Esse arquivo ainda carrega o número e o gatilho revogados** (`percent: 50`, `trigger: OnDamageDealt`) — atualizá-lo é fatia própria, fora deste documento (dado de carta, não canon de design).

## Ficha rápida

Família **Elétrico** · categoria **Ativa** · especial · mana 6 · alvo único · consome **1 bateria inteira por uso** — repetível na mesma batalha enquanto houver bateria disponível (não é limite de 1×/batalha; corrigido em 31/08/2026, ver "Por que é assim").

## Como funciona

Ao ser usada, drena **21% da energia do ALVO** por uso (porcentagem do alvo, não valor absoluto). **55% do drenado converte e chega à party**; os outros **45% se perdem como calor** (2ª lei da termodinâmica). No momento do uso, **o jogador escolhe** se o retorno vira mana ou vida — a escolha é **cronometrada por dificuldade**: Fácil 13s, Médio 8s, Difícil 5s, Hardcore 3s. Esgotado o tempo sem escolha, a energia dissipa e o jogador **não recebe nada** (nem mana, nem vida).

Usar a carta consome **uma bateria inteira** (não uma fração dela). Com outra bateria disponível no inventário, a carta pode ser usada de novo na mesma batalha — não há teto de usos por combate; o teto é a quantidade de baterias que o jogador tem.

## Por que é assim

Especificação fechada pelo líder em 31/08/2026, por `AskUserQuestion`, verbatim: *"o volta fica como decidi agora, achei melhor"*. Fonte canônica: `docs/design/roster-analogos/_EFEITOS-ESCOLHIDOS.md` linha 14 (o item `VOLTA-LEECH-%` de `cartas-technomagik.md` §9, agora RESOLVIDO). Racional do líder: percentual em vez de absoluto, para a carta seguir relevante do início ao fim da campanha sem escalar número, e casar com a física (quanto mais energia há no sistema, mais se pode extrair); 21% e 55% são Fibonacci, coerentes com o resto da economia; a escolha cronometrada nasce de ordem direta dele — *"ele tem x segundos para escolher ou a energia dissipa e ele perde. x depende da dificuldade"*.

Vale notar a garantia que já valia antes e continua valendo: este leech **não fura a regra de "sem carry-over de mana"** do jogo — é dreno do inimigo, não retenção da própria mana não-gasta (um design anterior, descartado, propunha "mana não-gasta vira Shield"; esse caminho segue morto).

**Revogado em 31/08/2026, por decisão do líder:** a leitura anterior descrita aqui — gatilho `OnDamageDealt → Leech`, drenando uma fração do **DANO CAUSADO**, e a constante `kVoltaLeechPercent = 50` — está morta. O dreno é sobre a **energia do alvo**, em porcentagem, nunca sobre o dano.

**Segunda correção do líder, 31/08/2026, verbatim:** *"A carta de volta é uma vez por bateria. Ela drena a bateria toda. Se tiver outra bateria, pode usar novamente."* Isto substitui a leitura de **1×/batalha** que a Ficha Rápida registrava até aqui (herdada do código do projeto anterior como regra geral de anti-abuso das especiais, `cartas-technomagik.md` §2.1) — o Volta é uma **exceção deliberada** a essa regra geral. **Isto encerra o ponto que antes estava marcado para revisão do líder:** não existe mais tensão entre "21% por uso" e um teto de usos por combate — a carta é repetível, limitada por recurso (bateria), não por contagem de batalha.

**Leitura de desenho, registrada porque explica o conjunto:** a bateria é o **combustível** e o dreno é o **efeito**. O jogador gasta energia armazenada (a bateria) para extrair energia do inimigo, e perde 45% como calor no processo — a 2ª lei da termodinâmica aparece nas **duas pontas** da carta (o custo de acionar e o custo de converter), não só numa.

**Decisão consciente, não descuido — registrado para ninguém "corrigir" depois achando que foi engano:** o canon geral de energia (`docs/design/mecanicas/cartas-hardware-pirataria-energia.md` §5, "Energia: baterias CR2032") descreve o DEFAULT como degradação gradual da bateria com o uso, não consumo total de uma vez. O Volta desvia desse default por ordem explícita do líder — consome a bateria inteira por uso. Não é pedido de confirmação: ele foi explícito. (Precedente mecânico já existente no corpus para "consumir a bateria de uma vez": o payload de vírus Zip-bomb, mesmo documento §8 — lá é malware que estoura a bateria; aqui é desenho legítimo da carta, mesma primitiva, uso oposto.)

**Mana e bateria coexistem por padrão — canon já fechado, não é pergunta nova:** `cartas-hardware-pirataria-energia.md` §3 fixa o princípio (líder, 08/08/2026, ampliado no mesmo dia): *"TODAS as cartas devem passar pelo mecanismo de originalidade da carta (§3) E degradação da bateria (§5) — as duas variáveis JUNTAS, não uma ou outra. Isso é o DEFAULT; exceções existem, mas cada uma se discute e se aprova caso a caso com o líder, nunca por omissão."* Logo mana 6 (Ficha Rápida) e o custo de bateria coexistem nesta carta por padrão; não há exceção registrada para o Volta quanto a isso. Ponteiro (L-30): `cartas-hardware-pirataria-energia.md` §3.

Duas consequências da especificação numérica, registradas e **não resolvidas aqui**: o combate por turnos ganha um elemento em tempo real, e a escolha cronometrada é barreira de acessibilidade conhecida. Detalhe completo em `_EFEITOS-ESCOLHIDOS.md` linha 14.

## Pontas soltas

- ⚠️ **MARCADO PARA O LÍDER, NÃO RESOLVIDO AQUI:** o princípio canônico de `cartas-hardware-pirataria-energia.md` §3 exige responder, ao planejar cada carta, **"o que muda se ela for original × pirata × homebrew?"** — essa resposta **não existe em lugar nenhum** para o Volta. Não inventada aqui.
- ⚠️ **ACHADO ADICIONAL, MESMA CATEGORIA, TAMBÉM NÃO RESOLVIDO:** o mesmo §3 exige responder, para toda carta, uma segunda pergunta — **"o que muda se a bateria estiver nova × degradada?"** — e essa resposta também não foi encontrada em lugar nenhum para o Volta. Marcado junto, sem inventar resposta.
- `resources/cards/volta.gw.card` segue com o número e o gatilho revogados (ver aviso no topo deste documento) — atualização do dado é tarefa separada, não feita aqui.
- A chave `CARD_EXEC_VOLTA_NAME` resolve para **"Volt-Leech"** em `resources/translations/pt_br.md` — sem mudança.

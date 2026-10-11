<!--
SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
SPDX-License-Identifier: AGPL-3.0-or-later
-->

# Volt-Leech (`volta`)

**Tipo Diátaxis:** Explanation. **Audiência:** design/engenharia deste projeto. **Last-reviewed:** 31/08/2026. **Owner:** `technical-writer`; mecânica de recurso fechada por decisão do líder (dado herdado do projeto anterior via L-01 está revogado, ver abaixo).

Dado completo em [`resources/cards/volta.gw.card`](../../../../resources/cards/volta.gw.card): `trigger: OnCast`, `percent: 21` (energia do alvo). O split 55/45 de conversão e a escolha cronometrada por dificuldade ainda não têm campo próprio no schema — ver comentário do arquivo.

## Ficha rápida

Família **Elétrico** · categoria **Ativa** · especial · **piso de ativação 6** (só jogável com 6 ou mais de carga na bateria) · alvo único · consome **toda a carga disponível por uso**, repetível na mesma batalha enquanto houver carga disponível (ver "Por que é assim").

## Como funciona

Ao ser usada, drena **21% da energia do ALVO** por uso (porcentagem do alvo, não valor absoluto — este número não muda com nada abaixo). **55% do drenado converte e chega à party**; os outros **45% se perdem como calor** (2ª lei da termodinâmica). No momento do uso, **o jogador escolhe** se o retorno vira mana ou vida — a escolha é **cronometrada por dificuldade**: Fácil 13s, Médio 8s, Difícil 5s, Hardcore 3s. Esgotado o tempo sem escolha, a energia dissipa e o jogador **não recebe nada** (nem mana, nem vida).

Isso é o dreno sobre o ALVO. Separado disso, usar a carta tem um custo para o PRÓPRIO conjurador: consome **toda a carga disponível** (não uma fração dela), e só é jogável com **6 ou mais de carga** na bateria em uso. Abaixo de 6, a carta aparece indisponível: não pode ser escolhida, e nenhuma carga é gasta, nem parcialmente (ver "Por que é assim"). Com carga disponível de novo (outra bateria), a carta pode ser usada de novo na mesma batalha: não há teto de usos por combate; o teto é a carga que o jogador tem.

## Por que é assim

Especificação fechada pelo líder em 31/08/2026, por `AskUserQuestion`, verbatim: *"o volta fica como decidi agora, achei melhor"*. Fonte canônica: `docs/design/roster-analogos/_EFEITOS-ESCOLHIDOS.md` linha 14 (o item `VOLTA-LEECH-%` de `cartas-technomagik.md` §9, agora RESOLVIDO). Racional do líder: percentual em vez de absoluto, para a carta seguir relevante do início ao fim da campanha sem escalar número, e casar com a física (quanto mais energia há no sistema, mais se pode extrair); 21% e 55% são Fibonacci, coerentes com o resto da economia; a escolha cronometrada nasce de ordem direta dele — *"ele tem x segundos para escolher ou a energia dissipa e ele perde. x depende da dificuldade"*.

Vale notar a garantia que já valia antes e continua valendo: este leech **não fura a regra de "sem carry-over de mana"** do jogo — é dreno do inimigo, não retenção da própria mana não-gasta (um design anterior, descartado, propunha "mana não-gasta vira Shield"; esse caminho segue morto).

O dreno é sobre a **energia do alvo**, em porcentagem, nunca sobre o dano.

**Decisão do líder, 31/08/2026, verbatim:** *"A carta de volta é uma vez por bateria. Ela drena a bateria toda. Se tiver outra bateria, pode usar novamente."* O Volta é uma **exceção deliberada** à regra geral de anti-abuso das especiais herdada do código do projeto anterior (`cartas-technomagik.md` §2.1): não há tensão entre "21% por uso" e um teto de usos por combate — a carta é repetível, limitada por recurso (bateria), não por contagem de batalha.

**Leitura de desenho, registrada porque explica o conjunto:** a bateria é o **combustível** e o dreno é o **efeito**. O jogador gasta energia armazenada (a bateria) para extrair energia do inimigo, e perde 45% como calor no processo — a 2ª lei da termodinâmica aparece nas **duas pontas** da carta (o custo de acionar e o custo de converter), não só numa.

**Decisão consciente, não descuido — registrado para ninguém "corrigir" depois achando que foi engano:** o canon geral de energia (`docs/design/mecanicas/cartas-hardware-pirataria-energia.md` §5, "Energia: baterias CR2032") descreve o DEFAULT como degradação gradual da bateria com o uso, não consumo total de uma vez. O Volta desvia desse default por ordem explícita do líder — consome a bateria inteira por uso. Não é pedido de confirmação: ele foi explícito. (Precedente mecânico já existente no corpus para "consumir a bateria de uma vez": o payload de vírus Zip-bomb, mesmo documento §8 — lá é malware que estoura a bateria; aqui é desenho legítimo da carta, mesma primitiva, uso oposto.)

**Mana e carga de bateria são o MESMO recurso, com dois nomes (decisão do líder, 31/08/2026 — canon de PROJETO INTEIRO, não desta carta só).** Verbatim: *"como magia é tecnologia, então mana e carga de bateria se confundem e são a mesma coisa. Pode ser citado mana ou bateria... O jogador no início vai se confundir, mas é proposital e depois vai passar a entender quando alguém disser por exemplo que a bateria está com pouco mana."* A confusão inicial do jogador é deliberada, e se desfaz por fala de personagem, não por tutorial. **O registro deste princípio como canon do projeto inteiro é feito por outro agente, em outro documento** — não redigido aqui, para não abrir uma segunda fonte da mesma verdade. Candidato natural a abrigá-lo, não confirmado: `cartas-hardware-pirataria-energia.md` §4-§5 (terminologia e energia).

**Piso de ativação e consumo total, decisão do líder de 11/10/2026, verbatim:** *"gasta tudo. Mas se for menos de 6, ela nao funciona"*. O 6 deixa de ser custo somado: não se paga 6 para jogar a carta, precisa-se TER 6 ou mais na bateria dela para jogá-la. Usada, a carta consome a carga inteira, qualquer que seja o valor. O piso impede que a carta seja jogada com um resto inútil de carga, e consumir tudo mantém a escolha de usar agora ou guardar para depois.

Duas consequências da especificação numérica, registradas e **não resolvidas aqui**: o combate por turnos ganha um elemento em tempo real, e a escolha cronometrada é barreira de acessibilidade conhecida. Detalhe completo em `_EFEITOS-ESCOLHIDOS.md` linha 14.

## Pontas soltas

- ⚠️ **MARCADO PARA O LÍDER, NÃO RESOLVIDO AQUI:** o princípio canônico de `cartas-hardware-pirataria-energia.md` §3 exige responder, ao planejar cada carta, **"o que muda se ela for original × pirata × homebrew?"** — essa resposta **não existe em lugar nenhum** para o Volta. Não inventada aqui.
- ⚠️ **ACHADO ADICIONAL, MESMA CATEGORIA, TAMBÉM NÃO RESOLVIDO:** o mesmo §3 exige responder, para toda carta, uma segunda pergunta — **"o que muda se a bateria estiver nova × degradada?"** — e essa resposta também não foi encontrada em lugar nenhum para o Volta. Marcado junto, sem inventar resposta.
- `resources/cards/volta.gw.card` usa `trigger: OnCast`, `percent: 21` (ver topo deste documento). O split 55/45 (conversão à party × perda em calor) e a escolha cronometrada por dificuldade seguem sem campo próprio no schema.
- A chave `CARD_EXEC_VOLTA_NAME` resolve para **"Volt-Leech"** em `resources/translations/pt_br.md` — sem mudança.

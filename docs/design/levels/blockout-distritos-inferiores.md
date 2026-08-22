# Blockout: Distritos Inferiores (Vertical Slice)

**Status:** Canônico (design). Ratificado Sprint 5 W3 2026-06-03; **traçado reescrito em 90x60 em 2026-08-04** (`DEMO-CIDADE-VESTIDA` fatia A); **adensado com 16 fachadas em 2026-08-04** (fatia H, ver §5.2). O grafo de nós, as decisões DA-1/DA-2/DA-3 e a pedagogia da versão 30x20 seguem valendo; o que mudou foi a escala, a densidade urbana em volta e a passagem de "5 blocos soltos" para "bairro construído".

**Fonte da verdade do traçado:** `distritos_inferiores.csv` (90x60, `tile_size` 2.0 m). Este doc explica o porquê; o CSV manda no o quê.

**Cross-ref:** core-loop-exploracao.md §2/§3/§4/§5; onboarding-vs.md §2/§7; puzzle-gambito.md §8; dialogue-tree-npc-intro.md §1 (Bertoldo na Praça da Compilação); combat.md §17; gdd.md §6/§7; pillars.md; PLACES.md (Praça da Compilação e Distritos Inferiores canon); `docs/narrative/environments/01-cidade-cyber-gotica.md` §2/§3 (fonte da Engrenagem-Mestra, pedestal Era 3 vazio, holograma Sterling, pichação "Compila com a gente").

**Convenção:** pt-br. Termos game-dev no original (blockout, graybox, gold path, sightline, choke point, landmark, vantage point). Sem em-dash.

---

## 1. O que mudou e por quê

O mapa anterior tinha 30x20 células, ou seja **1,4 tela de largura por 1,6 de altura**: a cidade cabia quase inteira em um enquadramento, e os 5 nós de design flutuavam em chão liso. O novo tem **90x60 células = 180x120 m = cerca de 4 telas de largura por 4,8 de altura**, um fator de 9 em área.

O tamanho foi escolhido **contra** o pedido inicial de 480x320. A conta que sustentou a recusa está no `TODO.md` do item e vale repetir porque restringe este traçado: com as 9 peças de cenário existentes, um mapa de 153.600 células daria **uma peça a cada 17 mil células**, isto é, minutos de caminhada sem nada. Mapa grande sem conteúdo não parece grande, parece abandonado.

A consequência de projeto é a regra que guiou cada célula deste traçado: **densidade antes de extensão**. Na prática:

- **Cerca de metade do mapa é massa construída** (tile Parede), não vão navegável. Quarteirão fechado não é espaço desperdiçado, é silhueta de cidade e é o que faz uma rua parecer rua em vez de campo aberto.
- **Nenhuma reta longa sobrou de graça.** Onde uma avenida atravessaria o mapa inteiro, entram canteiros, quarteirões deslocados e muros. A única linha originalmente contínua de ponta a ponta (a rua leste-oeste em y=21/22) foi quebrada de propósito.
- **Toda região navegável tem função declarada** na tabela da §3. Nenhuma existe só para ligar duas outras.

## 2. Convenção de âncora (contrato com a fatia C)

O formato do mapa tem 5 tipos de tile e nenhum deles é "prop". Para não inventar tipo novo (decisão que não é minha), este traçado usa o tile **Marco (2)** como **âncora de peça de cenário**:

- **Marco não bloqueia** (só Parede bloqueia, ver `gus/domain/map/tile_map.hpp`). A âncora é onde o sprite deve ser plantado, não onde o jogador é impedido de andar.
- **Peça sólida** (casa, fonte, portão, cover box): o volume é feito de **Parede**, e a âncora fica na **célula de rua em frente à fachada**, isto é, onde o jogador para para interagir ou de onde vê a peça de frente. Isso casa com a hitbox de ativação nos pés já usada no projeto.
- **Cover box não tem âncora própria:** a peça é exatamente uma célula de Parede isolada, e vale **uma caixa por célula** (bloco de 1x2 recebe duas caixas). A fatia C pinta o sprite sobre a célula de Parede solta, e o render deixa de pintar a cor de graybox dessa célula na leitura de produção: quem faz o papel de parede ali é a arte. Um bloco de 1x2 com uma caixa só deixava meio tile de parede nua acima dela, que é o que o laudo visual da fatia D mediu (achado A3).
- **Peça sem volume** (board do puzzle, holograma projetado, poste): a âncora é o canto superior-esquerdo do sprite.

São **50 âncoras** no mapa, listadas na §5. (Eram 29 na primeira versão deste traçado, 34 depois dos
postes de cobertura de tela, e 50 desde a fatia H, que adensou a cidade com 16 fachadas novas. Este
número já ficou desatualizado uma vez, e a divergência só apareceu quando a fatia C cruzou doc contra
CSV. O CSV é a fonte da verdade: 50, reconferidos célula a célula na fatia H.)

**A cor da âncora só existe para quem traça o nível.** Marco (âmbar), Entrada (verde) e Saída (azul)
eram a legenda de graybox, útil enquanto não havia arte nenhuma. Com a cidade vestida elas viraram
defeito medido (achado A2 do laudo visual da fatia D): o âmbar vaza por baixo e ao redor de cada peça,
e as âncoras sem sprite viram quadrados chapados no meio da rua. Desde a fatia E o jogo tem **duas
leituras do mesmo mapa**: em **produção** (o default) esses três tipos saem na cor do Chão, e quem
marca o lugar é a arte; a legenda inteira volta com `GUSWORLD_TILE_PALETTE=blockout`, que é a
ferramenta para conferir traçado. Chão e Parede são iguais nas duas. Ou seja: **continue usando Marco
como âncora exatamente como antes** e confira o traçado com a env var ligada.

## 3. Regiões (retângulos exatos, conferíveis contra o CSV)

Eixo X para a direita (0..89), Y para baixo (0..59). Bordas do mapa (x=0, x=89, y=0, y=59) são Parede, salvo a entrada e a saída.

| # | Região | Faixa | Função (por que existe) |
|---|---|---|---|
| R1 | Portal Norte + Alameda | x40..49, y0..8 | Chegada. Entrada em (44,0)/(45,0), spawn em (44,3). Corredor de 10 de largura com sightline reta para o sul: o jogador vê o holograma à esquerda e o vão da praça ao fundo. |
| R2 | Terraço de Chegada | x30..57, y1..6 | Átrio antes da praça. Holograma Sterling e primeiro poste. Dois canteiros quebram a largura. |
| R3 | Quarteirões Norte | x3..15 e x19..29 e x58..70 e x74..88, y1..7 | Massa construída. Definem a alameda por contraste, não por muro de arena. |
| R4 | Becos Norte | x16..18 e x71..73, y1..8 | Duas descidas laterais paralelas à alameda. A leste é o começo da rota do explorador (§7). |
| R5 | Rua do Terraço | x1..88, y8 | Rua leste-oeste, quebrada por 6 canteiros para não virar linha reta. |
| R6 | Anel Norte | x1..19 e x25..65 e x71..88, y9..11 | Distribuição. Dois quarteirões atravessam (x20..24 e x66..70) e forçam desvio. |
| R7 | Muro Norte do Hub | x25..39 e x50..63, y12 | Fecha a praça deixando só a boca da alameda (x40..49). Choke de entrada com propósito: enquadra a primeira visão da fonte. |
| **R8** | **Praça da Compilação (hub)** | **x26..63, y13..29** | **Coração social.** Fonte da Engrenagem-Mestra ao centro, pedestal Era 3 vazio do lado oposto, banco do Bertoldo, placa Era 2. 4 canteiros e 4 postes quebram os 38x17 de vão. |
| R9 | Aberturas do hub | x25 em y20..22; x64 em y16..18 | Únicas passagens laterais. A oeste desce ao bairro residencial, a leste à praceta do terminal. |
| R10 | Bairro Oeste | x1..24, y13..33 | Três ruas norte-sul (x1..3, x12..14, x22..24), duas fileiras de quarteirões, uma rua horizontal em y21..22. Duas casas cibergóticas. |
| R11 | Beco morto oeste | x22..24, y23..33 | Branch curto com retorno: âncora de loot no fundo (23,33). Beco com recompensa, não beco vazio. |
| R12 | Bairro Leste | x65..88, y13..38 | Espelho assimétrico do oeste. Praceta do terminal de hack (x68..75, y13..17), duas casas, e **três ruas norte-sul** (x65..67, x76..78, x86..88) que descem até a travessa. |
| R13 | Choke da escadaria | x42..47, y30..33 | Descida à arena. Estreita para 4 células em y32 (o "degrau" central) e reabre. Único acesso frontal ao combate. |
| R14 | Pátio de Sucata | x1..14, y34..43 | Branch oeste com retorno: 2 cover boxes e espaço de manobra. Sem saída para a arena, de propósito (senão o choke perderia a função). |
| **R15** | **Arena rebaixada** | **x34..56, y34..45** | **Encontro.** 23x12 células. Entrada norte (choke), flanco leste (rota do explorador), saída sul única. 4 cover boxes, 1 pilar central 2x2, 2 postes. |
| R16 | Travessa Leste e flanco | x53..88, y36..38 | Coleta as **três** ruas do bairro leste e as despeja no flanco da arena. Entrada alternativa no combate, com ângulo de aproximação diferente. Canteiro em (72,37)/(73,37) para não virar reta limpa. **Toda descida pelo leste termina aqui**, ou seja, dentro da arena: não existe rota que pule o combate. |
| R17 | Corredor-Puzzle | x42..48, y47..51 | 7x5 células exatas do board do Gambito. Bloqueio lógico da progressão. |
| R18 | Beco da Pichação | x4..20, y47..51 | Branch sudoeste: lore de parede ("Compila com a gente", canon). |
| R19 | Pátio da FIR | x65..85, y47..51 | Branch sudeste: espaço de patrulha em rota fixa, 2 cover boxes, 1 poste. |
| R20 | Antepraça do Portão | x30..60, y52..54 | Respiro antes da saída. |
| R20b | Rua de Ligação | y52..53, x4..85 | Costura R18, R20 e R19. **A linha y53 é livre de x4 a x85 sem uma única interrupção**, e essa continuidade é o que mantém os dois branches vivos: é invariante de traçado, não detalhe de decoração. A quebra visual fica na fileira de cima (y52), com duas ilhas isoladas em (23,52) e (62,52), que não podem vedar nada porque y53 passa por baixo. |
| R21 | Portão Sul | x44..45, y55 | Passagem única para a Periferia. |
| R22 | Vestíbulo | x40..49, y56..58 | Save point antes de sair (DA-2). Afunila para (44,59)/(45,59). |

## 4. Esquema do traçado

Esquema **rotulado**, não escala 1:1 (a fonte exata é o CSV e a tabela da §3). Serve para conferir de relance a topologia e o sentido de percurso.

```
        x0                    x30        x44       x57                    x89
   y0   ##########################  E E  ##########################
        |  R3 quarteirões   |R4|   R2 terraço   |R4|  R3 quarteirões  |
   y8   ---------------- R5 rua do terraço (6 canteiros) -------------
   y9   |   R6 anel   |###|        R6 anel          |###|   R6 anel   |
   y12  ##########  R7 muro  ##########[ boca x40..49 ]##########  R7  ##
        |            |                                 |             |
        |   R10      |                                 |     R12     |
        |  bairro    | (25)      R8 PRAÇA DA           |(64)  bairro |
        |  oeste     |  ->        COMPILAÇÃO           | <-   leste  |
        |  x1..24    | y20..22    fonte (43,21)        |y16..18 x65..88
        |            |            pedestal (52,21)     |             |
        | R11 beco   |            placa (36,24)        | praceta do  |
        | morto      |            Bertoldo (32,19)     | terminal    |
   y29  |            |#################################|             |
   y30  |            |####### R13 choke escada ########|             |
        |            |        x42..47 (estreita)       |             |
   y34  | R14 pátio  |#####|                     |#####| 3 ruas do
        | de sucata  |#####|    R15 ARENA        |#####| bairro leste
   y36  | x1..14     |#####|   x34..56, y34..45  |<==== R16 travessa =====
        | 2 covers   |#####|  4 covers + pilar   |#####| (x53..88, y36..38)
   y39  |            |#####|                     |#############(fechado)##
   y45  |############|#####|_____ saída sul _____|#######################
   y46  ####################[ x42..47 ]#################################
   y47  | R18 beco   |#####| R17 corredor-puzzle |#####| R19 pátio da |
        | pichação   |#####|  board 7x5 (42,47)  |#####| FIR (patrulha)
   y51  |____________|#####|_____________________|#####|______________|
   y52  |        R20 antepraça x30..60          |
   y53  ----------- rua de ligação y53 (costura os três) -----------
   y55  ############################[ PORTÃO 44..45 ]############################
   y56  ############################| R22 vestíbulo |############################
   y59  ############################     S S        ############################
```

Percurso pretendido: **norte para sul, sempre**. O jogador nunca precisa voltar por onde veio, e os dois branches laterais (R14, R18/R19) ficam do lado do caminho, não atrás dele.

## 5. Mapa de peças de cenário (insumo direto da fatia C)

Todas as peças vivem em `resources/sprites/world/distritos_inferiores/`. Coordenadas em célula (x,y). "Volume" = as células de Parede que a peça ocupa fisicamente, quando ela bloqueia.

| Peça (arquivo) | Âncora | Volume | Por que ali |
|---|---|---|---|
| `holo_sterling.png` | (35,4) | nenhum | Primeira coisa que o jogador vê ao entrar, à esquerda da alameda. Vira o landmark "de onde eu vim" quando ele olhar para trás. Canon: holograma em cada esquina dizendo "continue". |
| `fonte_latao.png` | (43,21) | x42..45, y18..20 | Centro geométrico e social da praça, no eixo da entrada: da boca da alameda (x40..49) a sightline cai direto nela. Landmark âncora do hub. Canon: fonte da Engrenagem-Mestra, mecanismo Era 1 girando há 700 anos. |
| `placa_lore.png` | (36,24) | nenhum | Logo abaixo e à esquerda da fonte, no caminho óbvio de quem falou com o Bertoldo e segue para o sul. Primeiro nó de lore, ensina o Scan. |
| `terminal_hack.png` | (69,15) | nenhum | Praceta do bairro leste, fora do gold path. Recompensa quem explorou a abertura leste do hub; abre a leitura do flanco (R16). |
| `casa_cibergotica_a.png` | (8,21) | x4..11, y13..20 | Fachada sul do quarteirão oeste, de frente para a rua horizontal. |
| `casa_cibergotica_b.png` | (17,21) | x15..21, y13..20 | Vizinha da anterior, mesma rua, silhueta diferente (torre). Duas silhuetas distintas lado a lado dão leitura de bairro. |
| `casa_cibergotica_a.png` (2ª) | (82,21) | x79..85, y13..20 | Espelho no bairro leste, para o leste não parecer só corredor de serviço. |
| `casa_cibergotica_b.png` (2ª) | (73,17) | x68..75, y18..33 | Fachada norte, fecha a praceta do terminal ao sul. |
| **16 fachadas do adensamento (fatia H)** | ver §5.2 | quarteirões existentes | Repetição dos dois modelos para a cidade ler como cidade. Nenhuma parede nova: toda âncora se apoia em quarteirão que já existia. |
| `poste_neon_ciano.png` x18 | (17,4), (40,4), (52,4), (72,4), (50,11), (33,15), (56,15), (33,27), (56,27), (13,28), (66,28), (36,34), (54,34), (75,48), (10,53), (33,53), (78,53), (52,54) | nenhum | Ritmo de iluminação e breadcrumb noturno. Distribuídos para marcar cada mudança de região, nunca em fileira regular. Os cinco de (17,4), (72,4), (13,28), (66,28) e (33,53) entraram depois da medição de densidade por tela (ver §10): eram becos e ruas com espaço andável e nenhuma peça. **Dois mudaram de lugar na fatia G** (playtest do líder, "tem um poste no meio da rua"): **(40,7) → (40,4)** e **(48,9) → (50,11)** — ver a nota abaixo. |
| `cover_box.png` x14 | sem âncora | (6,36),(6,37),(10,36),(10,37) no pátio; (38,36),(38,37),(38,42),(38,43),(52,36),(52,37),(52,42),(52,43) na arena; (70,49),(78,49) no pátio da FIR | Cobertura real (célula de Parede), **uma caixa por célula**. Na arena formam 4 grupos verticais 1x2 simétricos, com o pilar central 2x2 (x44..45, y39..40) fechando a leitura tática. |
| `board_puzzle.png` | (42,47) | nenhum | Canto superior-esquerdo do tabuleiro 7x5 (x42..48, y47..51), alinhado célula a célula com o grid do puzzle-gambito §8. |
| `portao_sul_fechado.png` | (44,55) e (45,55) | x40..43 e x46..49 em y55 | Único vão do muro sul. Os batentes já são Parede; o vão está aberto por ora (ver §8, decisão em aberto). |

Âncoras que ainda não têm sprite e são marcação de design: **(52,21)** pedestal Era 3 vazio "reservado para futuro monumento Sterling Corp" (canon de `01-cidade-cyber-gotica.md` §2; peça a desenhar, do lado oposto à fonte), **(32,19)** banco do Bertoldo (spawn do NPC), **(23,33)** fundo do beco morto (loot/lore da fatia D), **(44,57)** save point do vestíbulo (DA-2), **(12,49)** pichação do beco sudoeste.

Conferência (cruzada célula a célula contra o CSV na fatia H): **50 âncoras**, sendo **18 postes**, **24 de arquitetura** (holograma, fonte, placa, terminal e **20 casas**), **3 de instalação** (board do puzzle e as 2 células do portão) e **5 de marcação sem sprite** (pedestal, banco, beco, save, pichação). As 14 barricadas continuam **sem âncora** por definição (a peça É a célula de Parede), então a tabela de vestimenta tem **57 linhas** para 50 âncoras.

### 5.1 Os dois postes que saíram do meio da rua (fatia G)

O líder viu a cidade vestida e reclamou: *"tem um poste no meio da rua"*. Enumerando as 18
âncoras de poste e medindo a parede mais próxima de cada uma, **dois** estavam em chão liso
absoluto, sem uma única célula de Parede num raio de nove por cinco células, e os dois dentro da
**Alameda de Chegada (R1)** — o corredor por onde o jogador entra no jogo. Poste solto num
corredor de passagem não lê como iluminação, lê como obstáculo sem motivo.

| Antes | Depois | Em que encosta | Por que ali |
|---|---|---|---|
| (40,7) | **(40,4)** | face **leste** do canteiro de (38..39, 3..4) | Espelho exato do poste que já existia em **(52,4)**, encostado na face **oeste** do canteiro de (53..54, 3..4). Os dois canteiros que a §3 R2 diz existirem "para quebrar a largura" do terraço passam a ter, cada um, a sua luminária, e a alameda continua com âncora em vez de virar a tela vazia que a §10 existe para evitar. |
| (48,9) | **(50,11)** | batente **leste** da boca da praça, sobre a célula de Parede (50,12) do muro R7 | Ilumina o único choke de entrada do hub, que é a função declarada daquele muro na §3 ("enquadra a primeira visão da fonte"). É também o menor deslocamento que tira o poste do meio do Anel Norte. |

**Os quatro postes soltos da praça — (33,15), (56,15), (33,27), (56,27) — NÃO foram tocados, de
propósito.** Eles também estão em chão liso, mas por decisão de traçado: a §3 R8 os põe ali para
quebrar o vão de 38x17 da Praça da Compilação. Poste no meio de uma praça é iluminação de praça;
poste no meio de um corredor de passagem é obstáculo. A régua que separa os dois casos é a
FUNÇÃO da região, não a distância até a parede mais próxima.

Movimento de âncora toca o mapa, então o CSV foi reeditado, o `.gmap` recompilado e a
conectividade da cidade vestida reconferida célula a célula (§9).

### 5.2 As 16 fachadas do adensamento (fatia H)

Decisão do líder, verbatim: *"como é um demo, deixe esses prédios assim como placeholders. Pode
repetir os prédios, como em uma cidade de verdade, no jogo vamos fazer certo as imagens."* Duas
ordens numa frase. A primeira: **a arte fica como está** (a perspectiva isométrica sobre mundo
ortogonal, achado A7 do laudo visual, é trabalho do jogo, não do demo). A segunda: **repetir**. Havia
4 fachadas num mapa de 90x60 com quase metade da área construída, ou seja quarteirão atrás de
quarteirão de retângulo preto liso.

**A régua que escolheu cada célula. Nenhuma das três é gosto, e é por elas que a fatia não quebrou o
mapa:**

1. **A âncora fica na célula de rua imediatamente abaixo da face sul de um quarteirão.** A caixa
   sólida de uma casa mede **5,49 x 1,83 células** na escala 1,83, e ela se apoia na base do desenho,
   crescendo para cima: sela a fileira da âncora **mais a de cima**. Ancorando logo abaixo de uma
   parede, a fileira de cima já era parede e a peça custa **uma só** fileira de rua. É o placement das
   casas que já estavam de pé em (8,21), (17,21) e (82,21), agora enunciado como regra.
2. **O desenho tem que cair inteiro sobre massa construída.** A térrea tem 5,49 células de altura e a
   torre 7,32, desenhadas para cima a partir da base. Se houver rua andável nesse retângulo, o Y-sort
   põe o jogador **atrás do prédio** e ele some por vários passos. É o defeito conhecido de (73,17), e
   nenhuma peça nova o repete: cada uma foi conferida contra as fileiras de parede acima dela.
3. **Duas fachadas nunca ficam a menos de 5,5 células em x na mesma fileira.** Mesma base = empate de
   Y-sort, e os desenhos se cortam.

| # | Âncora | Peça | Onde e por quê |
|---|---|---|---|
| 1 | (27,7) | B | Torre isolada no canto sudeste do quarteirão x19..29, na boca da alameda. Primeira silhueta alta que o jogador vê. |
| 2 | (77,7) | A | Quarteirão NE, face sul, de frente para a Rua do Terraço. |
| 3 | (84,7) | A | Vizinha da anterior. Duas térreas seguidas a leste. |
| 4 | (6,8) | A | Quarteirão NO (x3..15), face sul. |
| 5 | (13,8) | A | Mesmo quarteirão, segunda térrea. Duas seguidas a oeste. |
| 6 | (61,8) | A | Quarteirão x58..70, face sul. Fecha a leitura da Rua do Terraço. |
| 7 | (8,34) | A | Face norte do Pátio de Sucata (R14), que era muro liso com duas barricadas. |
| 8 | (71,36) | A | Travessa Leste (R16), bloco x68..75. A rota do explorador descia entre dois paredões de 18 fileiras. |
| 9 | (82,36) | B | Travessa Leste, bloco x79..85. Torre isolada entre térreas. |
| 10 | (18,47) | A | Topo leste do Beco da Pichação (R18). A pichação de (12,49) continua livre a oeste. |
| 11 | (68,47) | A | Pátio da FIR (R19). |
| 12 | (74,47) | A | Pátio da FIR. |
| 13 | (80,47) | A | Pátio da FIR. As três formam fileira de sobrados de conjunto habitacional. |
| 14 | (35,52) | A | Antepraça (R20), isolada a oeste. |
| 15 | (52,52) | A | Antepraça, par encostado. |
| 16 | (58,52) | B | Antepraça. Última silhueta alta antes do portão sul. |

**Distribuição:** 13 térreas e 3 torres novas, o que deixa a cidade em **15 A para 5 B**. Torre é
exceção numa cidade, e as cinco ficam isoladas entre térreas. A alternância é **irregular de
propósito**: dois pares colados, uma fileira de três, quatro peças solitárias. A-B-A-B lê mais
artificial que repetição.

**Onde NÃO entrou fachada, e por quê:**

- **Praça da Compilação (R8).** É o coração social, e a geometria concorda com o design: as únicas
  faces disponíveis são o muro R7, de **uma** fileira de espessura, com o Anel Norte andável logo
  acima. Qualquer prédio ali violaria a regra 2 e esconderia quem anda no anel. Zero fachadas.
- **Arena (R15) e corredor-puzzle (R17).** Espaço de jogo, não de cenário.
- **Ruas norte-sul de 3 células** (x1..3, x12..14, x22..24, x65..67, x76..78, x86..88) e os becos
  (R4). A caixa da casa tem 5,49 de largura: uma fachada ali **fecha a rua inteira**.
- **Faixa x21..29 da antepraça.** Ali a fileira y54 já é parede, e uma casa deixaria a rua de ligação
  y53 com 2 sub-fileiras úteis. y53 é **invariante de traçado** (§3 R20b), e não se estreita uma
  invariante para ganhar decoração.

**O bug que a fatia produziu e o teste pegou (registro, para não renascer).** O primeiro arranjo do
Pátio da FIR foi 68/74/**82**, com folgas irregulares de 6 e 8 células. O teste *"nenhuma célula
andável fica ilhada"* reprovou: a folga entre a casa de 74 e a de 82 caía **exatamente sobre a
barricada de (78,49)**, e sobravam 2 sub-fileiras de rua em y47 cercadas por casa a oeste, casa a
leste, o muro y46 ao norte e a barricada ao sul. Um quarto sem porta. Com 68/74/**80** as caixas se
encostam e não existe folga onde o bolsão possa se formar. **Regra que fica: entre duas fachadas
vizinhas, a folga nunca pode cair sobre uma peça sólida da fileira de baixo.** A leitura de nível não
pegaria isso; o teste pegou na primeira rodada.

## 6. Gold path

1. **Chegada (y0..8).** Spawn em (44,3). Alameda larga, hazard zero, holograma à vista. Espaço seguro para aprender a andar.
2. **Anel Norte (y9..12).** Primeira escolha barata: seguir reto (gold path) ou ir para os becos laterais. A boca da praça em x40..49 é a abertura mais larga do mapa e puxa o olho.
3. **Praça (y13..29).** Bertoldo em (32,19) logo à esquerda de quem entra; a fonte em frente; a placa em (36,24) no caminho de saída ao sul. NPC, lore e landmark na ordem pedagógica sem forçar corredor.
4. **Choke (y30..33).** Vão único de 6 células que estreita para 4. Momento de leitura: daqui se vê a arena antes de entrar nela.
5. **Arena (y34..45).** Combate. Cover em quatro pontos, pilar central, duas rotas de fuga (norte de onde veio, leste pelo flanco).
6. **Corredor-puzzle (y47..51).** Board 7x5. Tensão cognitiva, sem dano.
7. **Antepraça e portão (y52..59).** Respiro, save em (44,57), saída em (44,59).

Tempo estimado do gold path: **6 a 8 minutos** de travessia sem exploração (era 5 no mapa antigo). Com os três branches: **12 a 15 minutos**.

## 7. Rotas alternativas (todas com retorno)

- **Rota do explorador (leste).** Beco norte (x71..73) desce sem passar pela praça, chega à praceta do terminal (x68..75, y13..17) e desce por qualquer das três ruas do bairro até a **travessa leste (R16)**, entrando na arena pelo flanco. Recompensa: terminal de hack, uma casa a mais e posição tática. Custo: pula o Bertoldo e a placa, ou seja quem corre perde a lore, não o caminho.

  **Regra de fecho que sustenta o caminho único (canon D4):** todas as descidas laterais terminam **dentro** de uma região obrigatória. O leste desemboca na arena; o Pátio de Sucata a oeste é fundo de saco proposital; o Beco da Pichação e o Pátio da FIR só existem depois do puzzle, alcançáveis pela rua y53. Em nenhum ponto o jogador contorna a arena ou o corredor-puzzle.
- **Bairro oeste e Pátio de Sucata (R10/R14).** Pela abertura oeste do hub. Recompensa: 2 casas, cover boxes e o beco morto com loot em (23,33). **Não conecta à arena de propósito**: se conectasse, o choke perderia a função de enquadrar o combate.
- **Beco da Pichação e Pátio da FIR (R18/R19).** Saem da rua de ligação y53, já depois do puzzle. Recompensa: lore de parede a oeste, encontro opcional de patrulha em rota fixa a leste (decisão (b) do líder: inimigo anda ida e volta, sem reagir).

## 8. Riscos e decisões em aberto

- **O portão sul está aberto.** O formato não tem tipo "porta" e eu não invento tipo. As duas células do vão (44,55) e (45,55) estão como Marco (livre), com os batentes em Parede. Isso mantém o demo jogável hoje. **Decisão para o líder:** (a) manter livre e fazer o bloqueio pela lógica do jogo quando o puzzle for resolvido, ou (b) deixar as células como Parede e o loader trocá-las na conclusão do puzzle. Prefiro (a): o bloqueio é regra de jogo, não geometria.
- **Ids de tile 5 e acima já são válidos no formato** e são tratados como chão que não bloqueia (`kMaxReservado = 5` em `tile_map.hpp`). É a porta pronta para "tile decorativo" na fatia C, sem mudar o binário. Registro como observação; a decisão de usar é do líder.
- **Onde o jogador pode se perder:** o Anel Norte (y9..11) é o ponto com mais saídas simultâneas. Mitigação embutida: a boca da praça é a abertura mais larga do mapa (10 células contra 3 dos becos) e a fonte fica na sightline. Se o playtest mostrar hesitação ali, a correção é iluminação (postes), não geometria.
- **Onde a dificuldade pode picar:** entrar na arena pelo flanco leste chega mais perto dos cover boxes do lado leste. Se ficar fácil demais, a correção é posição de spawn do inimigo, não o mapa.

## 9. Conectividade: como este traçado é verificado

Traçado de nível **não se confere por leitura**. Este aqui foi lido por três pessoas e a leitura não pegou que dois branches inteiros estavam ilhados: 266 células, 10% de todo o espaço andável, com quatro âncoras de peça dentro, isoladas por dois tampões de duas células cada em y53. O gold path estava íntegro o tempo todo, então o mapa parecia jogável.

A verificação obrigatória, sempre que este CSV mudar, é **busca em largura a partir do spawn**, tratando só Parede como bloqueio (mesma regra de `tile_map.hpp`), exigindo:

1. **zero** células passáveis inalcançáveis;
2. **todas** as âncoras alcançáveis (nenhuma peça de cenário em lugar onde não se chega);
3. entrada, saída e todo `#portal` alcançáveis.

O que a busca provou e a leitura não prova: uma ligação pode estar **descrita no doc e ausente no CSV**. Foi exatamente o caso, e por isso a §3 agora enuncia a continuidade de y53 como **invariante**, não como descrição.

### 9.1 A partir da fatia G, a busca roda sobre a cidade VESTIDA

O CSV deixou de ser a única coisa que barra o jogador. Cada peça de cenário carrega uma caixa
sólida (`scene_prop_solid`), e a escala global das peças **multiplica essa caixa junto com o
desenho**. Quando o líder subiu a escala para 1,83 (variante A2), o custo medido no mapa real foi
**de 62 para 154 células andáveis tocadas por peça** — 92 células de rua a mais barradas. O
número foi aceito com conhecimento de causa; o que ninguém tinha verificado era se alguma dessas
células era um **gargalo que parte a cidade em duas**.

Por isso a verificação obrigatória passou a rodar sobre **paredes do CSV mais as caixas sólidas
das peças na escala vigente**, e com a régua do motor, não a da célula: o jogo colide em AABB
contínua, então a busca varre uma sub-grade fina e aceita uma posição quando a **hitbox real do
jogador** (0,6 tile) cabe ali. Uma busca por célula aprovaria fresta de meio tile por onde o Gus
não passa.

Os três chokes declarados neste doc — o degrau da escadaria (R13, 4 células em y32) e as duas
aberturas laterais do hub (R9, x25 em y20..22 e x64 em y16..18) — são conferidos **um a um**,
exigindo que **toda** célula que o traçado deixou livre continue servindo. Choke que perde metade
da largura ainda "passa" numa busca, mas vira funil de colisão que o jogador sente como
travamento.

Isto vive como teste, não como procedimento manual: `app/tests/city_props_test.cpp`, seção
"cidade vestida". Quem mexer no CSV, na tabela de vestimenta ou na escala das peças vai ver o
teste falhar antes de ver o playtest travar.

**A fatia H foi a primeira prova de fogo desses três testes, e eles morderam.** O adensamento levou a
cidade de 41 para 57 peças, das quais 16 são prédios com caixa de 5,49 x 1,83 células, e o teste de
células ilhadas reprovou um dos arranjos (o bolsão do Pátio da FIR, §5.2). O arranjo foi corrigido e
os três voltaram ao verde. Registro do que isso significa na prática: **a densidade desta cidade não é
mais escolhida no olho**, é escolhida contra o teste, e cada fachada nova a partir daqui paga o mesmo
pedágio.

Observação medida e aceita na escala 1,83: a âncora **(73,17)**, do prédio alto do bairro leste,
fica **cercada pela base do próprio prédio** (a caixa dele passou a cobrir a fileira de calçada
ao norte, e ao sul já era o quarteirão). Não é quebra de conectividade — zero células ficam
ilhadas e o jogador contorna pelo norte, por y14/y15 —, e não há interação presa àquela peça. É
consequência esperada de um prédio com 5,5 células de base, e está registrado aqui para não ser
"descoberto" de novo como se fosse defeito novo.

## 10. Densidade por tela

Segunda métrica, medida em grade fixa de 22 por 12,5 células (campo de visão aproximado): contar espaço andável e âncoras por tela. Telas com espaço andável relevante e **zero** âncoras são candidatas a chão liso, que é o defeito que este traçado existe para evitar. A medição apontou seis; cinco eram becos e ruas de fato sem nada e ganharam poste. A sexta (arena sul) foi mantida sem âncora nova de propósito: ela já tem o pilar central e dois cover boxes, ou seja tem leitura visual sem precisar de peça.

Limite conhecido da métrica: a grade é fixa e a câmera real segue o jogador, então a fronteira das telas é arbitrária; e silhueta de quarteirão preenche o enquadramento sem produzir âncora. Serve para apontar onde olhar, não para reprovar sozinha.

## 11. Métricas de playtest

- Tempo até a saída pelo gold path: alvo 6 a 8 min.
- Achou pelo menos um branch: alvo maior ou igual a 60% dos playtesters.
- "Soube para onde ir sem ajuda": alvo maior ou igual a 80%.
- Mortes na arena: alvo menor que 2 na primeira passagem.

---

## Decisões Canonizadas (Sprint 5 W3 2026-06-03, seguem válidas)

| # | Decisão | Escolha |
|---|---|---|
| **DA-1** | Topônimo "Distritos Inferiores" | **Canonizado como sub-local novo** em PLACES.md (borda baixa do Núcleo descendo à charneira da Periferia, engloba a Praça da Compilação e a descida sul). |
| **DA-2** | Save points | **Entrada (S0) + portão sul.** No traçado novo: spawn (44,3) e vestíbulo (44,57). |
| **DA-3** | Daemon-Guard (HP144) | **Spawn na arena, ativado pós-Sentinela.** A arena de 23x12 comporta o segundo encontro sem inflar o tutorial. |


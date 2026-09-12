# Puzzle das Sete Pontes de Königsberg (Periferia, labirinto de vielas)

> **Status:** CONCEITO (mesmo grão que as demais dungeons registradas em
> `docs/design/mundo-topologia.md` §4: tema, gimmick e forma; layout sala a sala fica para
> produção, quando a engine de mapa existir e o GlintFx tiver `present/`, L-06/L-27). Nada aqui é
> planta nem disposição fina de encontro.
>
> **Decisão do líder, 12/09/2026.** Escolha do problema (as Sete Pontes de Königsberg, Euler,
> 1736) e escolha do local (Periferia residencial, item 4 de `mundo-topologia.md` §4) por
> `AskUserQuestion`, sobre o gimmick de becos/portões/escadas que já existia registrado ali.
>
> **Cross-refs:** `docs/design/mundo-topologia.md` §4 item 4 (Periferia residencial, labirinto de
> vielas; gimmick de becos/portões/escadas que reconfiguram a rota, refugiados Tesla/Einstein nos
> cantos, Praça do Compilador) e §9 (a carta Ponte de Euler como sistema de atalhos central do
> mundo). `docs/design/dungeons/areas-secretas/04-periferia-vielas.md` (as 5 áreas secretas desta
> mesma dungeon, inclusive a Área E, que já usa a Praça do Compilador como ponto central do
> labirinto). `docs/design/roster-analogos/09-euler.md` e
> `docs/narrative/deep/characters/mestre-mat-02-euler.md` (a versão pessoal de Euler deste mesmo
> problema, num distrito alagado diferente, ver §10 abaixo). `docs/design/mecanicas/cartas/euler.md`
> (a carta Ponte de Euler / Bridge-Walk). L-26 (perspectiva 3/4 top-down fixa, sem eixo de altura).
> L-27 (nenhuma marcação de interface antes de o GlintFx traduzir marcação). Mesma trava de
> "o jogador resolve" da espiral de Ulam (`mundo-topologia.md` §4 item 13) e das cordas emaranhadas
> (`docs/design/dungeons/catedrais-cordas-emaranhadas.md`).
>
> **A resposta do enigma e a matemática por trás da malha são área cifrada:**
> `docs/_secret/dungeons/periferia-pontes-de-konigsberg-solucao.md`.

---

## 1. O padrão, em termos de jogo

O bairro apertado da Praça do Compilador já é, por desenho, um grafo: quarteirões fechados
(cruzamentos) ligados por vielas estreitas, com becos e portões que a comunidade foi fechando e
reabrindo por conta própria ao longo dos anos (o gimmick que já existia em `mundo-topologia.md` §4
item 4, antes desta nota). Este puzzle não substitui esse gimmick: nomeia o mecanismo que o resolve.

**A malha (pequena, legível de uma olhada, grade quadrada):**

- **5 cruzamentos:** a **Praça do Compilador** (o cruzamento central, já canônico como ponto
  focal do labirinto, `areas-secretas/04-periferia-vielas.md` Área E) e quatro quarteirões vizinhos
  em volta dela: **Quadra Alta**, **Quadra Baixa**, **Beco do Cotovelo** e **Beco Sem Saída**.
- **8 vielas fixas**, sempre abertas, sem portão: duas entre a Praça e a Quadra Alta, duas entre a
  Praça e a Quadra Baixa, uma entre a Praça e o Beco do Cotovelo, uma entre a Quadra Alta e o Beco
  do Cotovelo, uma entre a Quadra Baixa e o Beco do Cotovelo, e uma entre a Praça e o Beco Sem
  Saída.
- **2 portões que o jogador controla**, sem contar os da antecâmara (§3): o **Portão do Quintal**,
  que começa **fechado** e liga a Quadra Alta direto à Quadra Baixa por trás dos quintais; e o
  **Portão da Praça**, que começa **aberto** e é uma segunda passagem redundante ligando a Praça à
  Quadra Alta.

Com a malha como está, a travessia pedida (cruzar cada viela aberta exatamente uma vez e sair do
outro lado) é impossível. Um dos dois portões, alternado de estado, destrava a travessia; o outro,
alternado no lugar dele, não resolve nada. A resposta e a razão matemática vivem na área cifrada
(topo deste documento).

## 2. Por que esta dungeon, e não outra

O gimmick da Periferia residencial já era "becos/portões/escadas que reconfiguram a rota" antes
desta nota (`mundo-topologia.md` §4 item 4): um bairro remendado por quem vive nele, cheio de
passagens que mudam de estado com o tempo. Não existe outro lugar no jogo onde reconfigurar uma
passagem já seja, por si, o vocabulário nativo do espaço. As Sete Pontes de Königsberg pedem
exatamente isso: um grafo que já existe, e um jogador que percebe que pode mudar UMA aresta dele
para destravar a travessia inteira. O puzzle não é enxertado; é o nome do que o bairro já fazia.

## 3. Como o jogo ensina antes de cobrar

Uma antecâmara pequena, sem risco de falha, ensina a lição em escala mínima antes da malha cheia
cobrar. Ela é um pátio de ensaio com três becos sem saída ao redor: cada beco tem uma única
viela ligando-o ao pátio, e um quarto portão, entre dois dos becos, começa **fechado**. Como está,
a travessia dos quatro pontos também é impossível (o pátio e os três becos sobram saída avulsa
cada um). O jogador que testa o único portão que existe ali vê, no mesmo instante, o efeito da
mudança sobre a travessia. A antecâmara ensina a mesma lógica que a malha cheia vai cobrar, em
escala pequena o bastante para ver de uma olhada, sem punição alguma se o jogador só ficar curioso
(não há outro portão para errar aqui: só existe este, e ele é o único jeito de interagir com a
antecâmara).

## 4. Grade e câmera

Toda a malha, os cinco cruzamentos e as dez vielas possíveis (8 fixas + 2 portões), vive num único
plano, em grade quadrada 2D, sem eixo de altura e sem câmera que gire ou suba (L-26). As escadas já
citadas no gimmick original mudam **rota**, nunca **altura**: são leitura de cenário (uma escada
visível na Área secreta E de `areas-secretas/04-periferia-vielas.md`, por exemplo, é ponto de
orientação, não trecho escalável) e cabem direto na perspectiva 3/4 top-down fixa do jogo.

## 5. O que perde quem desiste

Esta dungeon não é uma Área faraday contornável (`mundo-topologia.md` §9); é a própria dungeon da
Periferia residencial, tipo labirinto, e a party precisa atravessá-la de algum jeito para seguir
adiante. Quem não lê a malha como grafo não fica trancado: continua podendo vagar pelo labirinto do
jeito comum, testando vielas, batendo em becos sem saída, gastando mais tempo e cruzando de novo
trechos já andados, exatamente como qualquer labirinto sem este puzzle. O que se perde é a
travessia limpa, cruzando cada viela uma única vez até o outro lado. Resolver premia com a rota
mais curta pelo bairro (o mesmo tipo de recompensa que a carta Ponte de Euler formaliza depois, em
escala de mundo, `mundo-topologia.md` §9); não resolver custa tempo e repetição, nunca bloqueio.

## 6. A trava obrigatória: o jogador resolve

Mesma exigência da espiral de Ulam e das cordas emaranhadas (decisão do líder, 12/09/2026, verbatim:
"o jogador resolve, nao é pra ser automatico"). Três coisas nunca acontecem nesta dungeon:

1. **Nenhum personagem anuncia o padrão.** O Gus não nomeia grafo, grau, paridade nem Euler em
   fala ou aparte, e não resolve o puzzle por diálogo. Se um aparte entrega a resposta, o jogador
   assistiu, não descobriu.
2. **Nenhuma marcação de interface aponta o caminho.** Sem numerar saídas, sem destacar o
   cruzamento com saída avulsa, sem desenhar a rota no HUD, sem contorno de destaque em portão
   nenhum. O jogador precisa perceber, olhando a malha, que certos cruzamentos têm saída sobrando.
3. **A passagem não cede por insistência nem por tentativa exaustiva confortável.** Testar as duas
   opções de portão é possível, mas cada tentativa errada custa a punição inteira (§8); não existe
   "tentar até cansar" sem custo.

**Frase-guarda:** qualquer solução em que o jogo aponte a resposta contradiz esta decisão.

## 7. O que o jogo confere

Só o acerto: a party cruzou cada viela aberta da malha exatamente uma vez e chegou ao outro lado.
Nunca o vocabulário. Quem resolve sem conseguir dizer "passeio euleriano", "grafo" ou "paridade"
resolve exatamente igual a quem sabe os nomes.

## 8. Ao errar

Pisar duas vezes na mesma viela (ou alternar o portão errado e tentar seguir mesmo assim, o que
deixa a malha impossível do mesmo jeito) não mata, não tira item e não escala castigo. A party é
devolvida à entrada do labirinto de vielas, tem que refazer o trajeto andado até aquele ponto, e a
malha continua exatamente como estava: os mesmos dois portões, nos mesmos estados. O preço é tempo
e caminho percorrido de novo, nunca um puzzle mais difícil na tentativa seguinte, e cada tentativa
deixa o jogador ver mais um pouco de onde sobra saída, porque é ele quem está olhando de novo, não
o jogo cedendo pista.

## 9. Frase-guarda

Qualquer solução em que o jogo aponte a resposta contradiz esta decisão. Ela existe para quem
implementar esta malha daqui a meses, não para quem já sabe o que ela quer dizer.

## 10. Âncora de lore: a mesma malha que Euler já resolveu em vida

Resolver a travessia desta malha é o que concede ao jogador a carta **Ponte de Euler**: esta
dungeon é a missão de descoberta dela. Euler está no roster dos mestres do Codex
(`docs/design/roster-analogos/09-euler.md`, canônico em
`docs/narrative/deep/characters/mestre-mat-02-euler.md`), e o problema real das sete passarelas e
quatro ilhotas que ele mesmo resolveu em vida, num distrito alagado (`mestre-mat-02-euler.md` §2),
é a mesma matemática de paridade desta malha. Este puzzle **não é aquele encontro, nem o
substitui, nem cria um segundo**: é a Periferia residencial, décadas depois e num bairro
diferente, tropeçando de novo na mesma estrutura que o mestre já formalizou em vida, sem que
ninguém no bairro tenha lido o tratado dele para perceber isso. Nenhum NPC da Periferia nomeia
Euler ao lado da malha (a trava do §6 vale igual aqui); o eco é só para quem já tem a carta
**Ponte de Euler** (`docs/design/mecanicas/cartas/euler.md`) e liga os pontos por conta própria.

**A fronteira da carta:** a face ativa, "Traçar Rota", mostra o MAPA de uma dungeon (nós e
conexões nos óculos táticos), nunca qual caminho satisfaz a condição de um enigma; nenhum puzzle
do jogo, inclusive este, se resolve equipando-a. Fronteira completa em
`docs/design/mecanicas/cartas/euler.md`.

## 11. O que não está decidido

- Redação em prosa da cena (o que a party vê e diz ao entrar no labirinto) fica para
  `narrative-writer`, só depois de aprovação deste conceito.
- Posição exata da malha dentro do grafo interno da dungeon (antes ou depois de qual outra área) e
  layout fino sala a sala ficam para produção, junto do layout fino desta e das outras 12 dungeons
  (`mundo-topologia.md` §10 item 1).
- Conteúdo exato do que a travessia limpa recompensa (item, atalho interno, ou os dois) fica para
  produção, no mesmo padrão já usado nas áreas secretas desta dungeon
  (`docs/design/dungeons/areas-secretas/04-periferia-vielas.md`).

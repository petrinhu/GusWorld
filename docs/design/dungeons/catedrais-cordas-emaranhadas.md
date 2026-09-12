# Puzzle das Cordas Emaranhadas (Catedrais #1)

> **Status:** CONCEITO (mesmo grão que as demais dungeons registradas em
> `docs/design/mundo-topologia.md` §4: tema, gimmick e forma; layout sala a sala fica para
> produção, quando a engine de mapa existir e o GlintFx tiver `present/`, L-06/L-27). Nada aqui é
> planta nem disposição fina de encontro.
>
> **Decisão do líder, 12/09/2026.** Escolha de local por `AskUserQuestion` (Catedrais #1, sobre
> outras opções apresentadas); escolha de 13 cordas por `AskUserQuestion` (sobre 11 e sobre 10, com
> a razão aceita registrada na área cifrada acima: 13 é Fibonacci, e sobram cordas depois da
> resposta).
>
> **Cross-refs:** `docs/design/mundo-topologia.md` §4 item 10 (Catedrais #1: vertical, nave e
> campanário; gimmick de engrenagens/carrilhões que já existe e que este puzzle não substitui) e a
> nota da espiral de Ulam no item 13 do mesmo arquivo (mesma trava de "o jogador resolve", adaptada
> aqui). `docs/design/dungeons/areas-secretas/13-area-faraday.md` (segundo exemplar da mesma trava).
> `docs/design/dungeons/areas-secretas/10-catedrais-liturgico.md` (as 3 áreas secretas da mesma
> dungeon, sincronização de carrilhões). `docs/narrative/environments/03-catedrais-neo-sylvania.md`
> §2.1 e §2.3 (Nave Principal e Capela do Sino Mecânico Asmódico, vocabulário nativo de cordas e
> sino) e §7 (ganchos mecânicos já registrados, que este documento acrescenta). `PLACES.md` e
> `CHARS.md` (Catedral do Cronômetro-Hilário, Bento "Requiem", Ordem Recursiva). L-26 (perspectiva
> 3/4 top-down fixa, sem eixo de altura: a verticalidade "nave + campanário" é leitura de cenário,
> nunca eixo jogável). L-27 (nenhuma marcação de interface antes de o GlintFx traduzir marcação).
>
> **A resposta do enigma e a matemática por trás da pedra são área cifrada:**
> `docs/_secret/dungeons/catedrais-cordas-emaranhadas-solucao.md`.

---

## 1. O obstáculo

Cordas de sino, grossas, velhas, emaranhadas num nó só, bloqueiam a passagem entre a nave e a base
do campanário. Não são cortáveis: material narrativo (fibra tratada por processo Era 1, a mesma
tradição que fez o cordão litúrgico de 89 nós resistir a séculos), e a única forma de abrir caminho
é desatar o nó pela corda certa. Puxar a corda errada não corta nada e não força o nó: dispara a
punição descrita na seção 4.

Este obstáculo é uma peça nova dentro da dungeon Catedrais #1, e convive com o gimmick que já existe
ali (sincronização de engrenagens/carrilhões que abre a cripta, `mundo-topologia.md` §4 item 10).
As cordas emaranhadas não substituem esse gimmick nem o antecipam: elas guardam a subida ao
campanário, onde o mecanismo de carrilhão mora. Quem resolve as cordas ganha acesso ao campanário;
quem sincroniza os carrilhões lá dentro é quem, depois, abre a cripta. Dois obstáculos distintos,
em sequência, na mesma dungeon.

**Grade e câmera:** o nó de cordas vive todo numa única parede/vão da câmara, em grade quadrada 2D,
sem eixo de altura e sem câmera que gire ou suba (L-26). A "subida ao campanário" é troca de mapa,
não geometria empilhada, no mesmo sentido já fixado para "nível" em
`docs/design/dungeons/subsolo-vance-disco-de-ouro.md` §3.

## 2. A pedra

Embutida na parede ao lado do nó, uma pedra grava quatro marcas: dois desenhos, uma letra, e uma
conta.

- **A lua.** Desenho do luar.
- **O monstro.** Desenho de uma criatura, sem nome dito em lugar nenhum da câmara.
- **A letra J**, gravada sozinha, sem explicação ao lado.
- **A conta:** "196883 + ? = 196884", gravada por extenso, com a incógnita literal. O jogador lê a
  conta pronta; não precisa deduzir que ela existe.

**O brilho da pedra** é sombrio e especial, e serve para uma coisa só: chamar a atenção do jogador
para a pedra, entre as demais superfícies da câmara. Não é luz de luar entrando por vitral, não é
alinhamento astronômico, não é céu aberto (a câmara não tem janela para isso). A pedra brilha por
si, como recurso ambiental fixo. O brilho não pisca, não muda de cor quando o jogador acerta ou
erra, e não indica qual corda puxar: ele só existe para que o jogador pare diante da pedra e leia o
que está gravado nela, antes de tocar em qualquer corda.

## 3. As 13 cordas

Treze cordas pendem do nó, cada uma com uma letra gravada no talo, de A a M, sem repetição e sem
ordem alfabética visível na disposição física (o emaranhado embaralha a posição; a letra é a única
informação confiável em cada corda). Uma delas desfaz o nó por completo; as demais não fazem nada.

Puxar a corda certa desfaz o nó: as treze cordas se soltam de uma vez, e o caminho para o campanário
abre.

**Nota sobre força bruta (a única que este desenho admite):** como as treze cordas estão à vista e
rotuladas, é sempre possível puxá-las uma a uma até acertar, sem qualquer leitura da pedra. Mas cada
erro custa a punição inteira da seção 4 (volta ao início da dungeon), o que torna essa busca cara em
tempo e caminho percorrido, nunca impossível.

## 4. A punição por erro

Puxar qualquer corda que não seja a certa teleporta a party de volta ao início da dungeon
Catedrais #1.

- **Sem perda de item.** O inventário não é tocado.
- **Sem morte.** Não há dano nem tela de derrota.
- **Sem aumento de dificuldade.** O puzzle está exatamente como estava: as mesmas treze cordas, a
  mesma letra certa, a mesma pedra. Nada muda entre uma tentativa e a próxima.
- **A punição é de tempo e caminho percorrido**, nunca de dificuldade: o preço de errar é
  percorrer de novo o trajeto até aqui, não enfrentar um puzzle mais difícil.

## 5. A trava obrigatória: o jogador resolve

Mesma exigência da espiral de Ulam (`mundo-topologia.md` §4 item 13, decisão do líder, 12/09/2026,
verbatim: "o jogador resolve, nao é pra ser automatico"). Três coisas nunca acontecem nesta câmara:

1. **Nenhum personagem anuncia o padrão.** O Gus não nomeia o que a conta e a letra da pedra
   significam, não diz o resultado da conta, e não resolve por diálogo. Se um aparte entrega a
   resposta, o jogador assistiu, não descobriu.
2. **Nenhuma marcação de interface aponta a corda certa.** Sem contorno de destaque, sem brilho
   reativo na corda certa, sem scan, sem prompt de botão. O brilho descrito na seção 2 é ambiente
   fixo da pedra, jamais indicador de resposta, e vale só para a pedra, nunca para as cordas.
3. **A fechadura não cede por insistência confortável.** Errar custa o caminho de volta (seção 4);
   puxar as treze uma a uma é possível, mas caro (seção 3), e essa é a única forma de força bruta
   que o desenho admite.

**Frase-guarda:** qualquer solução em que o jogo aponte a resposta contradiz esta decisão. Ela
existe para quem implementar esta câmara daqui a meses, não para quem já sabe o que ela quer dizer.

**O que o jogo confere:** só o acerto, a corda certa puxada, nunca o vocabulário. Quem resolve sem
conseguir explicar por que aquela corda é a certa resolve exatamente igual a quem sabe explicar. A
pedra grava a conta e os desenhos; o jogo nunca pergunta ao jogador o que eles significam.

## 6. Por que esta pedra existe (leitura em jogo, sem explicar a matemática)

A pedra grava lua, monstro, letra e conta juntos, de propósito: um jogador que nunca ouviu falar de
teoria de grupos vê, ainda assim, uma equação incompleta faltando "mais um", e uma letra isolada,
sem explicação ao lado. A ponte entre o que a conta resolve e qual corda carrega aquele resultado é
a única leitura que a câmara pede. A resposta exata está na área cifrada (topo deste documento).

## 7. O que não está decidido

- Redação em prosa da cena (o que a party vê e diz ao entrar na câmara, se algo é dito) fica para
  `narrative-writer`, só depois de aprovação deste conceito (mesmo fluxo do `subsolo-vance-disco-de-ouro.md` §6).
- Posição exata da câmara dentro do grafo interno de Catedrais #1 (antes ou depois de qual outro
  espaço) fica para produção, junto do layout fino da dungeon (`mundo-topologia.md` §10 item 1).
- Se este puzzle guarda algum item ou carta além de liberar o caminho ao campanário, o conteúdo
  exato fica para produção, no mesmo padrão já usado nas áreas secretas desta dungeon
  (`docs/design/dungeons/areas-secretas/10-catedrais-liturgico.md`).

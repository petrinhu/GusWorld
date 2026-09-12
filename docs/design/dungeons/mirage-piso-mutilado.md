# O Piso Mutilado (Setor Mirage: o Festival)

> **Status:** CONCEITO (mesmo grão que as demais dungeons registradas em
> `docs/design/mundo-topologia.md` §4: tema, gimmick e forma; layout sala a sala fica para
> produção, quando a engine de mapa existir e o GlintFx tiver `present/`, L-06/L-27). Nada aqui é
> planta nem disposição fina de encontro.
>
> **Decisão do líder, 12/09/2026.** Local escolhido por `AskUserQuestion` (Setor Mirage, Festival,
> sobre outras duas opções apresentadas), pela razão declarada na escolha: a dungeon inteira já é
> sobre distinguir o que parece do que é, e o piso mutilado encarna esse tema. A câmara é a ilusão
> do Festival, em forma de quebra-cabeça.
>
> **Cross-refs:** `docs/design/mundo-topologia.md` §4 item 3 (Setor Mirage: gimmick de scan
> real×holográfico, não tocado por esta câmara) e a nota de resolução logo abaixo do item 3 (este
> mesmo puzzle, versão condensada de canon). `docs/design/dungeons/areas-secretas/03-mirage-festival.md`
> (as 3 áreas secretas desta mesma dungeon, mesmo verbo de scan em variações distintas desta
> câmara). `docs/narrative/environments/05-setor-mirage.md` §2.4 (Catacumbas do Cult, nível -1) e §7
> (ganchos mecânicos já registrados, que este documento acrescenta). `docs/narrative/characters/iara-lumen.md`
> (Wound, Belief, memórias formativas). `CHARS.md`, `PLACES.md` (Iara "Lumen", Caleidoscópio,
> Catacumbas Cult). L-26 (perspectiva 3/4 top-down fixa, sem eixo de altura: o salão vive todo num
> único piso, nunca em geometria empilhada). L-27 (nenhuma marcação de interface antes de o GlintFx
> traduzir marcação).
>
> **O resultado de tentar cobrir o piso, onde fica a saída real, e a matemática por trás dos dois
> são área cifrada:** `docs/_secret/dungeons/mirage-piso-mutilado-solucao.md`.

---

## 1. O obstáculo: o Salão do Tapete Perfeito

Nas Catacumbas do Cult, nível -1 (`environments/05-setor-mirage.md` §2.4, hoje descrito como
"depósito convencional, equipamento holográfico"), um salão lateral guarda uma passagem que a
party precisa atravessar para seguir decifrando o padrão do Festival. O Cult Mirage o chama de
Salão do Tapete Perfeito e o apresenta às "promessas artísticas" (crianças adotadas como Iara foi,
`iara-lumen.md` Wound) como prova de fé: cobrir o piso por completo, dizem os hierofantes,
"liberta o peregrino paciente". Ninguém jamais cobriu.

O que os hierofantes não dizem, porque não sabem ou porque escolheram não saber, é que o salão é
instrumentado com os mesmos biossensores do Teatro Holográfico Maior (`environments/05-setor-mirage.md`
§2.2): cada tentativa de cobertura, cada minuto de frustração, é dado colhido e encaminhado a
Sterling Corp pelo mesmo cabo Óxido que atravessa o distrito inteiro. O Salão do Tapete Perfeito não
é um teste de fé. É mais um posto de extração sensorial, disfarçado de ritual, o mesmo tipo de
mentira que a missão de Iara nesta dungeon existe para desligar (`mundo-topologia.md` §4 item 3).

## 2. O tabuleiro principal

O piso do salão é um tabuleiro quadriculado **oito por oito**, tesselado em dois tons por desenho
de arte (roxo e ciano, a paleta já canônica do Mirage, `mundo-topologia.md` §7 "ilusão
holográfica, neon-glitch, ofuscamento"), sem nenhuma sobreposição de HUD. A cor de cada casa é
sempre a mesma pintura física do piso; o jogador lê a cor olhando, exatamente como lê a cor de
qualquer ladrilho do jogo.

**Faltam duas casas: os dois cantos diagonalmente opostos.** As peças rituais do Cult ("tapetes",
placas de pedra polida que a party empurra) cobrem sempre duas casas adjacentes. A party pode
tentar cobrir o salão por completo, em qualquer ordem, com qualquer disposição de peças; o
resultado de cada tentativa, e a razão por trás dele, é área cifrada (topo deste documento).

**Fibonacci:** oito É Fibonacci (1, 1, 2, 3, 5, **8**, 13, 21, 34, 55...), então o tamanho clássico
do tabuleiro de oito por oito não muda por acaso, mas também não muda por regra: ele já cumpria o
idioma numérico do jogo antes de qualquer decisão nova, e a escolha aqui foi mantê-lo, não trocá-lo
por outro número.

## 3. A antecâmara: onde a peça se aprende

Antes de chegar ao salão de oito por oito, a party passa por uma sala pequena e sem risco de
falha: um tabuleiro **quatro por quatro**, na mesma tesselação roxo/ciano, faltando dois cantos.
Este tabuleiro pequeno se cobre por completo. A antecâmara não é contada contra o idioma numérico
Fibonacci do jogo (o número que importa é o do salão principal, seção 2): ela existe só para
ensinar, em escala mínima e sem custo, a regra da peça (cobre duas casas adjacentes) e para que o
jogador saia dali tendo **conseguido** cobrir um tabuleiro mutilado por completo. A relação entre a
antecâmara e o salão principal, e por que uma se cobre e o outro não, é área cifrada (topo deste
documento).

## 4. Ao insistir

Testar ordens diferentes de peças no salão principal não é o jogo enganando o jogador: as cores
estão pintadas, visíveis, honestas, e o piso nunca trapaceia. A mentira, se existe, é só a promessa
do Cult ("cobrir liberta"). Quem for implementar esta câmara precisa preservar essa distinção: o
piso não trapaceia o jogador, o Cult é quem trapaceia as próprias promessas.

## 5. A saída

O salão guarda uma passagem para fora, cujo mecanismo exato é área cifrada (topo deste documento).
Ela nunca depende de quantas tentativas a party já gastou no piso, e está sempre à vista de quem
olha para o salão de frente, nunca escondida num canto morto. Ninguém pode ficar preso neste salão
para sempre sem que a saída esteja, literalmente, à sua frente o tempo todo.

## 6. Ao errar (aqui, "errar" é continuar tentando)

Não existe punição por tentativa: mover uma peça para uma posição que não fecha o quebra-cabeça
simplesmente falha em encaixar (a peça recua para o lugar de onde saiu), sem dano, sem perda de
item, sem teleporte, sem qualquer escalada de castigo entre uma tentativa e a próxima. O único
custo de insistir é o tempo do próprio jogador, e é um custo que o jogo nunca aumenta de propósito:
a milésima tentativa custa exatamente o mesmo que a primeira. Isto é distinto da punição por erro
das cordas emaranhadas (que teleporta a party de volta ao início da dungeon): aqui não há "resposta
errada" que puna. A única coisa que pode ser chamada de erro é o jogador continuar procurando onde
não há o que achar, e o próprio desenho garante (seção 5) que esse erro nunca é sem saída.

## 7. A trava obrigatória: o jogador resolve

Mesma exigência das cordas emaranhadas, da espiral de Ulam, do jardim de Conway e das Sete Pontes
de Königsberg (decisão do líder, 12/09/2026, verbatim: "o jogador resolve, nao é pra ser
automatico"). Três coisas nunca acontecem neste salão:

1. **Nenhum personagem anuncia o padrão.** O Gus não conta as casas de cada cor, e não resolve por
   diálogo o que o jogador precisa perceber sozinho. Se um aparte entrega a conclusão, o jogador
   assistiu, não descobriu.
2. **O scan do Gus não emite veredito sobre o piso.** O scan distingue real de holográfico, mas o
   piso mutilado É real, de pedra de verdade: não há nada ali para o scan "descobrir" sobre ele, e
   nenhuma versão desta câmara pode fazer o scan emitir um veredito sobre a cobertura.
3. **Nenhuma marcação de interface conta casas, colore por fora, recusa a peça errada antes do
   encaixe, ou aponta a saída.** Sem contorno de destaque na casa que sobra, sem contador de cor em
   HUD, sem prompt que avisa "esta peça não vai caber" antes do jogador tentar encaixar, sem seta ou
   brilho reativo apontando a saída da seção 5. Qualquer efeito ambiental fixo do salão é o mesmo em
   qualquer superfície equivalente do Mirage, nunca um indicador desta câmara em particular.
4. **Nada cede por insistência.** Nem tempo parado diante do piso, nem número de tentativas, nem
   item equipado abre um atalho na cobertura. Aqui insistir sem nunca reconsiderar é literalmente a
   armadilha (seção 4); ceder por insistência desfaria a própria razão de o salão existir.

**Frase-guarda:** qualquer solução em que o jogo aponte a resposta contradiz esta decisão. Ela
existe para quem implementar esta câmara daqui a meses, não para quem já sabe o que ela quer dizer.

## 8. O que o jogo confere

Só o acerto: a party do outro lado, na passagem real do salão. O jogo nunca verifica se o jogador
sabe explicar o que viu, e nunca distingue, na sua condição de sucesso, entre quem entendeu a
lógica do piso e quem simplesmente sentiu que "isso nunca vai fechar" e resolveu olhar em volta. Os
dois chegam à mesma passagem, pelo mesmo caminho, e o jogo os trata de forma idêntica. O
vocabulário nunca é testado; só o resultado.

## 9. Âncora de lore: Iara e o Festival

Iara é, no Codex de personagens, a única figura do distrito que "gosta de ver as três camadas ao
mesmo tempo" (`environments/05-setor-mirage.md` §1) e cuja Belief é explícita: "tudo é encenação.
quem não percebe, é encenado" (`iara-lumen.md`, Interno). O Salão do Tapete Perfeito é essa frase
construída em pedra: uma encenação de prova espiritual que na verdade é colheita de dado
psicológico, do mesmo tipo que a Wound dela nomeia (Adila colhendo dados de adeptos via espetáculo,
`iara-lumen.md`). Iara mesma, adotada aos 6 anos como "promessa artística" (`iara-lumen.md`
memórias formativas, aos 6 anos), é do tipo de criança para quem este salão foi desenhado: alguém
que a Cult queria manter tentando, paciente, medida, sem nunca libertar de verdade.

Este documento não cria personagem novo, não acrescenta hierofante nomeado ao elenco já fechado em
`environments/05-setor-mirage.md` §6, e não toca o molde dos 20 mestres do Codex (`mundo-topologia.md`
§4.2): o eco é só de tema e de Wound, não um encontro novo. Se Iara acompanhar a party neste salão,
cabe a `narrative-writer`, em fase própria, decidir se ela reconhece o lugar (a fita e a memória são
dela, mas este documento não abre esse gancho, só deixa o espaço para ele existir).

## 10. O que não está decidido

- Redação em prosa da cena (o que a party vê e diz ao entrar no salão, se algum NPC do Cult
  administra a "prova" no momento em que a party chega) fica para `narrative-writer`, só depois de
  aprovação deste conceito, mesmo fluxo dos documentos-irmãos desta onda.
- Posição exata do salão dentro do grafo interno da dungeon do Festival (antes ou depois de qual
  outro espaço, se bloqueia a rota principal ou é desvio opcional) fica para produção, junto do
  layout fino da dungeon (`mundo-topologia.md` §10 item 1).
- Se este salão guarda algum item ou carta além de liberar a passagem, o conteúdo exato fica para
  produção, no mesmo padrão já usado nas áreas secretas desta dungeon
  (`docs/design/dungeons/areas-secretas/03-mirage-festival.md`).
- Textura exata do efeito ambiental que sinaliza a saída (frequência, duração) fica para produção,
  com o `level-designer` e a engine de mapa; a única exigência fixada aqui é que ele já seja
  perceptível desde a entrada do salão, sem gatilho de tentativa prévia no piso.

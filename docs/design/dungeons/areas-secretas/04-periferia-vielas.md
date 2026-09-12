# Áreas secretas · Periferia (residencial): labirinto de vielas

> Tipo: **labirinto** · Áreas secretas: **5** · Nível: conceito (ver `_INDEX.md`).
>
> Cross-refs: `docs/design/mundo-topologia.md` §4 item 4 (gimmick: becos/portões/escadas que
> reconfiguram a rota; refugiados Tesla/Einstein nos cantos, Praça do Compilador; nota de resolução
> acrescentada em 12/09/2026 nomeia o mecanismo do labirinto inteiro, decisão do líder: **as Sete
> Pontes de Königsberg**, especificação completa em
> `docs/design/dungeons/periferia-pontes-de-konigsberg.md`); §9 (cartas Tesla/Newton como chave de
> mundo, tema compatível). `docs/design/mecanicas/save-por-local.md` §1.2: intensidade **total**.

## Por que 5, e por que todas pedem orientação espacial

O verbo desta dungeon é explorar um espaço apertado que se reconfigura. Labirinto ganha o maior
número (5) porque explorar É o verbo dele: cada área secreta abaixo recompensa uma forma
diferente de orientação espacial (padrão, altura, marca, som, retrospecto), nunca "encoste em
toda parede".

## Área secreta A: O ralo desalinhado

**Pista:** todas as grades de bueiro da viela têm a mesma orientação de barras; uma, num beco sem
saída aparente, está girada 90° em relação a todas as outras.

**Como se entra:** notar a diferença de orientação (não a existência da grade, que é comum) e
levantá-la.

**O que guarda:** categoria: atalho interno da própria dungeon, ligando dois ramos distantes do
labirinto (não um atalho de mundo, só desta dungeon) + material de crafting. Conteúdo exato:
produção.

**O que conta:** a Periferia é remendada por quem vive nela; um ralo fora do padrão é sinal de
mão humana recente, não de acidente.

## Área secreta B: O varal que aguenta peso

**Pista:** um varal de roupas entre dois prédios está esticado ao ponto de balançar quase reto,
mais tenso do que qualquer outro varal do bairro, afordância honesta de que aguenta peso.

**Como se entra:** atravessar o varal andando; mecanicamente é um trecho de piso comum, percorrido
com a locomoção de sempre (sem pulo, sem estado novo), na mesma grade e no mesmo plano de jogo de
sempre — só que texturizado como corda esticada, ligação entre dois pontos que a rua não conecta.

**O que guarda:** categoria: célula de leitura espacial. A célula alcançada é um vão interno do
quarteirão, cercado de prédios e sem entrada pela rua (só o varal liga as duas bordas). Geometricamente,
esse ponto fica mais perto de várias bocas de viela ao mesmo tempo do que qualquer célula acessível
pela rua, que ficam espalhadas cada uma numa esquina diferente do quarteirão. Com o alcance de Scan
que o jogador já tem ali (Óculos Táticos captando, Matriz Ortodôntica amplificando — o mesmo raio de
sempre, upgradável como em qualquer outro lugar do jogo, sem raio dedicado a esta célula), várias
bocas caem dentro dele de uma vez só a partir dali. A recompensa é essa revelação simultânea no HUD
(reorientação, não item). Conteúdo material: produção.

**O que conta:** ao ver várias bocas de viela acenderem juntas no HUD, de um ponto que só o varal
alcança, o jogador enxerga que o labirinto de vielas tem uma lógica de bairro remendado que
nenhuma rota no chão revela de uma vez só: o padrão só se lê inteiro de um centro que a rua não
alcança.

## Área secreta C: O sigilo na porta

**Pista:** uma porta lateral tem um grafite discreto, um sigilo simples, que repete um símbolo já
visto numa barraca de Tesla ou Einstein na Praça: a única porta do labirinto marcada assim.

**Como se entra:** reconhecer o símbolo repetido (não decifrá-lo, só notar que se repete) e
bater/abrir naquela porta específica.

**O que guarda:** categoria: fragmento de lore dos refugiados cooperativos, recurso. Conteúdo
exato: produção.

**O que conta:** a rede de refugiados tem uma linguagem própria de marcas, invisível a quem não
sabe procurar: o jogador aprende que existe uma segunda camada de comunicação na Periferia,
paralela à fala.

## Área secreta D: A caixa que range diferente

**Pista:** uma pilha de caixas empilhadas contra uma parede tem um som de "oco" perceptível ao se
aproximar, diferente do som surdo das outras pilhas do mesmo tipo na mesma rua.

**Como se entra:** empurrar a pilha específica (a que soa oca), revelando um vão de rastejar.

**O que guarda:** categoria: recurso, pequeno cache. Conteúdo exato: produção.

**O que conta:** textura de sobrevivência cotidiana: esconderijos improvisados fazem parte da
vida na Periferia, sem precisar de um NPC pra dizer isso.

## Área secreta E: A escada vista de baixo

**Pista:** de um ponto central do labirinto, a Praça do Compilador, uma escada de incêndio
distante é visível, mas nenhum caminho até ali parece existir a partir de onde o jogador está.

**Como se entra:** clássico beat de labirinto: ver antes de poder alcançar; o jogador precisa
guardar a imagem e voltar até ela depois de mapear o resto do labirinto por outra rota.

**O que guarda:** categoria: a maior recompensa das 5 (posição de fechamento do conjunto),
crafting raro ou trecho de lore mais denso. Conteúdo exato: produção.

**O que conta:** fecha o arco de orientação espacial da dungeon inteira: o jogador só a alcança
depois de já ter aprendido a ler o bairro, recompensando quem persistiu com as outras quatro.

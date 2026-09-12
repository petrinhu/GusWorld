# Puzzle do Jardim (Selve Sombria)

> **Status:** CONCEITO (mesmo grão que as demais dungeons registradas em
> `docs/design/mundo-topologia.md` §4: tema, gimmick e forma; layout sala a sala fica para
> produção, quando a engine de mapa existir e o GlintFx tiver `present/`, L-06/L-27). Nada aqui é
> planta nem disposição fina de encontro.
>
> **Decisão do líder, 12/09/2026.** O mecanismo de resolução é o Jogo da Vida de Conway (autômato
> celular), batizado no jogo apenas como "o jardim". Local escolhido por `AskUserQuestion`, entre
> outras opções apresentadas: a Selve Sombria, item 9 de `mundo-topologia.md` §4, porque o gimmick
> daquela dungeon já era, desde antes desta decisão, "flora matemática, plantas em sequência, abrir
> caminho lendo o padrão de crescimento". O autômato vira o MECANISMO concreto desse gimmick, sem
> substituí-lo nem apagá-lo.
>
> **Cross-refs:** `docs/design/mundo-topologia.md` §4 item 9 (Selve Sombria: trilha orgânica
> ramificada; gimmick de flora matemática + fauna-bug, intacto) e as notas de resolução do mesmo §4,
> item 13 (a espiral de Ulam, mesma trava de "o jogador resolve") e item 10 (as cordas emaranhadas,
> segundo eco de lore, ver seção 10 abaixo). `docs/design/dungeons/areas-secretas/09-selve-sombria.md`
> (as 2 áreas secretas já existentes desta dungeon; este obstáculo é distinto delas, não uma
> terceira). `docs/narrative/environments/02-selve-sombria.md` §2 (landmark Orla Recursiva, o prop
> "anel de cogumelos pulsando em padrão recorrente perfeito", reaproveitado aqui) e §7 (ganchos
> mecânicos já registrados, que este documento acrescenta por referência mínima).
> `docs/design/roster-analogos/13-von-neumann.md` (precursor histórico real do autômato celular,
> medido e intocado, seção 10). L-26 (perspectiva 3/4 top-down fixa, grade quadrada, sem eixo de
> altura). L-27 (nenhuma marcação de interface antes de o GlintFx traduzir marcação).
>
> **A resposta do enigma (qual arranjo caminha e como plantá-lo) e a matemática real do autômato
> são área cifrada:** `docs/_secret/dungeons/selve-sombria-jardim-de-conway-solucao.md`.

---

## 1. O padrão, em termos de jogo

Os canteiros da Selve são a grade. Cada canteiro está vivo (planta crescida) ou morto (terra nua).
A cada rega, todo canteiro se recalcula ao mesmo tempo, olhando só os oito vizinhos:

- canteiro vivo com duas ou três vizinhas vivas continua vivo;
- canteiro vivo com menos de duas ou mais de três vizinhas vivas seca;
- canteiro morto com exatamente três vizinhas vivas germina.

O jogador não planta o resultado final: planta a semente inicial (escolhe quais canteiros começam
vivos) e rega, canteiro a canteiro, o crescimento que essas três regras produzem sozinhas. Alguns
arranjos secam de imediato. Outros ficam parados no lugar. Outros ainda se comportam de formas
distintas entre si, e uma antecâmara (seção 3) ensina, à vista do jogador e sem risco de falha, as
formas de crescimento que importam antes de a câmara cheia cobrar.

## 2. Por que esta dungeon, e não outra

A Selve Sombria já era descrita, antes desta decisão, como "um sistema vivo escrito em matemática"
(`docs/narrative/environments/02-selve-sombria.md` §1), e o gimmick da dungeon já dizia, com essas
palavras, "abrir caminho lendo o padrão de crescimento" (`mundo-topologia.md` §4 item 9). O autômato
não chega para justificar um lugar novo: ele dá músculo mecânico concreto a uma frase que o canon já
escrevia sozinha. A flora da Selve sempre cresceu por regra fixa e observável (Pillar 2: natureza é
matemática rígida); o jardim é essa regra, jogável.

## 3. A antecâmara: como o jogo ensina antes de cobrar

Antes da câmara cheia, uma antecâmara pequena, num canteiro de grade 5x5, sem risco de falha (nada
aqui pune, nada aqui tranca), ensina em três passos, cada um plantado à vista do jogador, formas
distintas de crescimento que a grade produz sozinha, a partir de sementes diferentes. O jogador que
rega essas três lições sozinho, sem nomear nada, já viu tudo que precisa para resolver a câmara
cheia. O conteúdo exato de cada lição é área cifrada (topo deste documento).

## 4. Grade e câmera

O jardim inteiro vive no piso, em grade quadrada 2D, sem eixo de altura e sem câmera que gire ou
mude de ângulo (L-26): cabe direto na perspectiva 3/4 top-down fixa do jogo, do mesmo jeito que a
espiral de Ulam. A antecâmara usa grade 5x5; a câmara cheia usa grade 13x13, grande o bastante para
a solução se desenrolar por uma distância real, pequena o bastante para caber inteira num único
olhar de quem está diante dela.

## 5. A câmara cheia: os canteiros-alvo

Um grupo pequeno de canteiros-alvo, marcados pelo mesmo prop já registrado no ambiente (o anel de
cogumelos luminescentes, `docs/narrative/environments/02-selve-sombria.md` §2/§3), fica do outro lado
de uma brecha na trilha, fora do alcance de quem caminha: a party não pisa lá. A flora, ao contrário
da party, não precisa de chão firme para crescer, e é essa diferença que faz o jardim valer a pena
resolver.

O jogador planta a semente inicial num canteiro dentro do próprio alcance, com um orçamento fixo de
oito sementes por plantio, suficiente para a solução com alguma margem de tentativa e erro,
insuficiente para cobrir o jardim de sementes e forçar por volume. A tarefa do jogador não é contar
vizinhas: é notar, de olho, o que a rega produz, e usar isso para que os canteiros-alvo fiquem vivos,
mesmo que por um instante. A mecânica exata da solução é área cifrada (topo deste documento).

## 6. O que perde quem desiste

A "trilha orgânica ramificada" do gimmick original (`mundo-topologia.md` §4 item 9) já é, por nome,
uma trilha com mais de um ramal. O jardim abre o ramal mais curto da Selve Sombria; quem não resolve
segue pelo ramal mais longo, com mais fauna-bug pelo caminho (coerente com o tipo "mista" da
dungeon), sem ficar trancado em lugar nenhum. Ninguém fica impedido de terminar a dungeon por não
resolver o jardim; perde tempo e enfrenta mais encontros, nunca menos jogo.

## 7. A trava obrigatória: o jogador resolve

Mesma exigência da espiral de Ulam e das cordas emaranhadas (`mundo-topologia.md` §4, itens 13 e 10,
decisão do líder, 12/09/2026, verbatim: "o jogador resolve, nao é pra ser automatico"). Três coisas
nunca acontecem neste jardim:

1. **Nenhum personagem anuncia o padrão.** O Gus não nomeia o mecanismo por trás do jardim, não
   conta vizinhas em voz alta, e não resolve por diálogo.
2. **Nenhuma marcação de interface conta vizinhas, prevê o próximo passo, destaca um canteiro viável
   ou desenha a trajetória do arranjo.** Sem contorno de destaque, sem contador de vizinhas, sem
   prompt. O anel de cogumelos que marca o canteiro-alvo é ambiente fixo, do mesmo jeito que já era
   antes desta decisão; ele mostra onde está o alvo, nunca como chegar até ele.
3. **A passagem não cede por insistência nem por plantio exaustivo.** Regar sem parar não abre nada
   por si só, e plantar oito sementes espalhadas ao acaso não substitui plantar o arranjo certo no
   ponto certo.

**Frase-guarda:** qualquer solução em que o jogo aponte a resposta contradiz esta decisão. Ela existe
para quem implementar este jardim daqui a meses, não para quem já sabe o que ele quer dizer.

## 8. O que o jogo confere

Só o acerto: os canteiros-alvo vivos, por ter sido alcançados pela solução certa. Resolver sem saber
nomear o mecanismo vale exatamente o mesmo que resolver sabendo os nomes. O jogo nunca pergunta ao
jogador o vocabulário, só observa o estado dos canteiros-alvo.

## 9. Ao errar: limpar e replantar

Uma semente que seca antes de chegar ao alvo não mata ninguém, não tira item e não escala castigo:
o mesmo arranjo, a mesma regra, sem punição maior a cada tentativa nova. O jogador limpa o canteiro
e replanta, mas replantar não é de graça: o orçamento de oito sementes por plantio se gasta por
completo a cada tentativa, e reabastecer exige caminhar até o ponto onde a Selve mantém sementes
disponíveis para colher, fora do próprio canteiro, e voltar. O custo do erro é o mesmo das outras
duas câmaras da mesma decisão: tempo e caminho percorrido, nunca dificuldade maior nem perda de
progresso.

## 10. Âncora de lore

**John Horton Conway não está no roster de análogos históricos** (`docs/design/roster-analogos/`,
medido por busca: nenhuma entrada o cita). Por essa razão, ele não entra no roster nem ganha
encontro, interior ou carta: fica só como eco de lore, do mesmo jeito que a nota da espiral de Ulam
trata von Neumann.

**Von Neumann é precursor histórico real do autômato celular** (`docs/design/roster-analogos/13-von-neumann.md`
§1, "autômatos auto-replicantes", projetados em papel décadas antes de qualquer computador rodar um):
o autômato celular de 29 estados dele antecede e inspira o de dois estados de Conway. Isso não muda
nada do canon já fechado sobre von Neumann: ele continua sem interior nem puzzle novo na Área
faraday (`mundo-topologia.md` §4 item 13, última linha da nota da espiral de Ulam), e este documento
não acrescenta nada a ele. É eco sobre eco, não conteúdo novo.

**O laço que vale registrar:** o mesmo Conway batizou também um fenômeno da matemática real que dá
nome ao mecanismo de outro puzzle deste jogo, o das cordas emaranhadas de Catedrais #1. Os dois
puzzles se olham de longe: o mesmo matemático real que inventou o jardim também deu nome ao
fenômeno por trás do nó de cordas. Isto é eco de lore entre dois documentos de mecânica, não um
encontro novo; o laço completo é área cifrada.

## 11. O que não está decidido

- A forma exata da solução, na antecâmara e na câmara cheia (orientação, ponto exato de partida na
  grade 13x13) fica para produção, junto do layout fino da dungeon (`mundo-topologia.md` §10
  item 1).
- A posição exata dos canteiros-alvo dentro da grade 13x13 fica para produção, pelo mesmo motivo.
- Se este jardim guarda algum item ou carta além de abrir o ramal mais curto, o conteúdo exato fica
  para produção, no mesmo padrão já usado nas áreas secretas desta dungeon
  (`docs/design/dungeons/areas-secretas/09-selve-sombria.md`).
- Redação em prosa da cena (o que a party vê e diz ao entrar no jardim, se algo é dito) fica para
  `narrative-writer`, só depois de aprovação deste conceito.

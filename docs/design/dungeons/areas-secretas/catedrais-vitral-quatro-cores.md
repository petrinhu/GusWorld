# O Vitral das Quatro Cores (Catedrais #1, enigma da Área secreta B)

> **Status:** CONCEITO (mesmo grão que as demais dungeons e áreas secretas registradas em
> `docs/design/mundo-topologia.md` §4/§4.1: tema, gimmick e forma; layout sala a sala fica para
> produção, quando a engine de mapa existir e o GlintFx tiver `present/`, L-06/L-27). Nada aqui é
> planta nem disposição fina de encontro.
>
> **Decisão do líder, 12/09/2026.** Local escolhido por `AskUserQuestion` entre três opções:
> **área secreta das Catedrais**, não câmara do caminho principal. Isso fixa o lugar como
> recompensa opcional para quem procura: quem não acha, ou acha e não resolve, segue o jogo
> inteiro sem prejuízo (§5 abaixo). O vitral é o enigma da **Área secreta B de Catedrais #1** ("O
> mosaico que não fecha a conta", `docs/design/dungeons/areas-secretas/10-catedrais-liturgico.md`):
> a peça de piso do pátio que não fecha a conta é a pista já fixada daquela área, e é ela que leva
> até a bancada e os torniquetes da rosácea. Catedrais #1 continua com **3** áreas secretas, o
> mesmo total de sempre: o vitral não abre capela numerada à parte, nem soma área nova à dungeon.
>
> **Cross-refs:** `docs/design/mundo-topologia.md` §4 item 10 (Catedrais #1: vertical, nave e
> campanário; gimmick de engrenagens/carrilhões já existente, que este documento não substitui) e
> a nota da espiral de Ulam no item 13 do mesmo arquivo (mesma trava de "o jogador resolve",
> adaptada aqui). `docs/design/dungeons/catedrais-cordas-emaranhadas.md` (segundo exemplar da
> mesma trava, e origem do motivo corda/guincho reaproveitado abaixo). `docs/design/dungeons/areas-secretas/10-catedrais-liturgico.md`
> (Área secreta B, cuja pista de piso abre o caminho até este enigma; as outras 2 áreas da mesma
> dungeon, A e C, seguem com pista e enigma próprios, sem tocar o vitral). `docs/narrative/environments/03-catedrais-neo-sylvania.md`
> §2.1 e §3 (vitral em arco-quente, vocabulário nativo de vitral-tech) e §7 (ganchos mecânicos já
> registrados). `docs/design/mundo-topologia.md` §7 ("pedra + vitral-tech" já é a paleta/ecossistema
> fixado das Catedrais). L-26 (perspectiva 3/4 top-down fixa, sem eixo de altura: o vitral é
> parede, o jogador não sobe nem escala para trabalhar nele). L-27 (nenhuma marcação de interface
> antes de o GlintFx traduzir marcação).
>
> **A lógica de coloração das regiões e o teorema real por trás do vitral são área cifrada:**
> `docs/_secret/dungeons/catedrais-vitral-quatro-cores-solucao.md`.

---

## 1. O padrão, em termos de jogo

Um vitral gótico, uma rosácea, está estilhaçado: os cacos de vidro coloridos caíram da armação de
chumbo e ficam soltos numa bancada baixa ao pé da parede. A party os recompõe. As peças vêm em
quatro cores de vidro, e o vitral só acende (a luz volta a atravessá-lo, de dentro para fora, na
hora seguinte em que o sol bate ali) quando nenhuma peça encaixada é vizinha de outra da mesma
cor.

**A disposição das peças (fixado aqui; o traçado exato de chumbo fica para produção):** oito
regiões, honestas com a grade quadrada e legíveis de uma olhada só, sem precisar contar peças na
tela: um núcleo central, seis pétalas dispostas em anel irregular ao redor dele (o vitral está
estilhaçado, não precisa ser simétrico: um vitral real quebrado não racha em fatias iguais), e uma
moldura externa que envolve o anel por fora. A lógica exata de qual região toca qual é área cifrada
(topo deste documento).

**Ponto de apoio:** uma das seis pétalas já chega encaixada e colorida, fixa desde o início. O
jogador nunca começa do zero: tem uma peça-âncora para comparar contra ela mesma antes de decidir
o resto.

## 2. A armadilha de leitura honesta

Duas regiões que só se tocam levemente podem, em certos casos, repetir a cor; noutros, não podem.
Quem não lê a fronteira entre duas regiões com cuidado tende a jogar pelo seguro, e aí a bancada
disponível deixa de bastar. A trava não é impedir essa tentativa (a bancada não limita quantas
peças de cada cor o jogador pode testar): é deixar o próprio vitral, apagado, ser a única resposta
de que a leitura errada custa uma cor. A lógica exata da armadilha é área cifrada (topo deste
documento).

## 3. Como o jogador manipula as peças, sem escalar a parede

A rosácea fica alta na parede, fora de alcance (L-26: perspectiva 3/4 top-down fixa, sem eixo de
altura, o jogador não sobe nem escala para trabalhar nela). Cada uma das oito regiões da armação
de chumbo tem sua própria corda de guincho, presa a um torniquete baixo ao pé da parede, no
mesmo vocabulário mecânico já nativo da Catedral (cordas, roldanas, relojoaria de latão): o
jogador pega uma peça de vidro na bancada, encaixa na armação-berço do torniquete correspondente
e puxa a corda, que ergue a peça até o lugar dela na rosácea. Errar não exige descer nada: a
corda desce de novo com um puxão simples, a peça volta à bancada, e outra pode subir no lugar.
Todo o quebra-cabeça se resolve com os pés no chão.

## 4. Como o jogo ensina antes de cobrar

Perto da bancada, uma vidraça de amostra, pequena, com só quatro peças, mostra em miniatura a mesma
distinção que a armadilha de §2 cobra na rosácea principal. A amostra não acende nem apaga; é só
demonstração, sem risco de falha. O jogador que a examina antes de subir as oito peças da rosácea
principal já viu a armadilha em miniatura, sem que ninguém tenha dito nada em voz alta. O conteúdo
exato da distinção é área cifrada (topo deste documento).

## 5. O que perde quem desiste

Área secreta, não câmara do caminho principal: quem não acha a capela onde a rosácea fica, ou
acha e não resolve, segue o jogo inteiro sem prejuízo algum. Não há item de progressão trancado
atrás do vitral, só a recompensa opcional de quem procura (§8).

## 6. A trava obrigatória: o jogador resolve

Mesma exigência das cordas emaranhadas e da espiral de Ulam
(`docs/design/mundo-topologia.md` §4 item 13, decisão do líder, 12/09/2026, verbatim: "o jogador
resolve, nao é pra ser automatico"). Três coisas nunca acontecem nesta câmara:

1. **Nenhum personagem anuncia o padrão.** O Gus não nomeia o teorema, não aponta região nenhuma
   em fala ou aparte, e não resolve por diálogo.
2. **Nenhuma marcação de interface** pinta a cor certa sozinha, recusa visualmente o encaixe
   errado antes de a peça subir, destaca a vizinhança de uma peça ou conta quantas cores já
   foram usadas. A amostra de §4 é ensino fixo, ambiental, nunca reativo ao progresso do jogador
   na rosácea principal.
3. **O vitral não acende por insistência nem por embaralhamento exaustivo.** Subir e descer
   peças ao acaso até acertar por sorte é sempre possível (a bancada não limita tentativas), mas
   cada tentativa errada custa o tempo de descer e subir de novo (§7), nunca menos que isso, e
   nunca uma pista de graça.

**Frase-guarda:** qualquer solução em que o jogo aponte a resposta contradiz esta decisão.

**O que o jogo confere:** só o acerto, a rosácea inteira acesa sem duas peças vizinhas da mesma
cor, nunca o vocabulário. Resolver sem conseguir nomear o teorema por trás do vitral vale
exatamente o mesmo que resolver sabendo o nome.

## 7. Ao errar

Como é área secreta e não caminho obrigatório, o custo do erro é mais leve que o das câmaras de
puzzle do caminho principal (cordas emaranhadas: teleporte ao início da dungeon; espiral de Ulam:
devolução ao ponto de leitura). Aqui, uma peça mal encaixada simplesmente não se fixa: ela desce
de volta à bancada no puxão seguinte da corda, sem penalidade de tempo de percurso, sem dano, sem
teleporte, sem escalar castigo entre uma tentativa e a próxima. O jogador tenta de novo na hora,
com a mesma rosácea, a mesma bancada, as mesmas quatro cores.

## 8. O que guarda (decisão do líder: as duas opções, combinadas)

O achado é um só, com duas metades que vêm juntas do mesmo esconderijo sob o piso do pátio, coerente
com o tema da câmara (uma prova que só fechou com máquina, dentro de uma catedral que venera
compilar):

- **Metade de lore:** um registro da Ordem Recursiva (diário, cripto-glifo ou entrada de aprendiz)
  refletindo sobre o vitral: a rosácea foi montada por um mestre que só conseguiu provar, num
  papel, a coloração das peças que tinha em mãos depois de testar caso por caso, à mão, por dias,
  sem nunca fechar a prova geral. Morreu sem saber que o resultado geral já existia, comprovado por
  máquina, do outro lado do mundo, décadas depois. O laço com o teorema real fica implícito, nunca
  dito por nome, e o teorema em si é área cifrada (topo deste documento).
- **Metade material:** material de crafting em vidro-tech (fragmentos de vitral tratado), na mesma
  categoria de "matéria-prima ligada ao tema da câmara" já usada na área faraday
  (`docs/design/dungeons/areas-secretas/13-area-faraday.md`, área A).

As duas metades vêm juntas porque fazem sentido juntas: quem quebra a peça do piso, atravessa a
capela e resolve a rosácea não fica só com a história de quem tentou antes, fica também com o
material físico daquilo que essa pessoa deixou inacabado.

## 9. Âncora de lore, não encontro novo

As Catedrais Neo-Sylvania já hospedam, no campus temático de "ciência sagrada da Era 1"
(`mundo-topologia.md` §6), os interiores de Giordano Bruno, John Dee, Pitágoras e Newton. O
vitral não presta homenagem a nenhum deles em particular: o teorema real por trás dele é matemática
do século XX, sem par nenhum no roster atual dos 20 mestres do Codex, e este documento **não
acrescenta mestre novo**. A âncora de lore é o lugar e o vocabulário, não uma pessoa: "vitral-tech"
já é parte fixada do ecossistema das Catedrais (`mundo-topologia.md` §7, "pedra + vitral-tech") e
já existe um vitral funcional citado no documento de ambiente (§2.1, §3, vitral em arco-quente,
reativo à temperatura). Este é um segundo vitral, distinto daquele: um que se resolve, não um que
reage ao clima.

## 10. O que não está decidido

- Redação em prosa da cena (o que a party vê e diz ao entrar na capela, se algo é dito) fica para
  `narrative-writer`, só depois de aprovação deste conceito.
- Posição exata da capela dentro do grafo interno de Catedrais #1 e o traçado fino de chumbo entre
  as 8 regiões ficam para produção, junto do layout fino da dungeon
  (`mundo-topologia.md` §10 item 1).

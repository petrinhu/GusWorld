# ANÁLISE: resultados do estudo de simulação da mira inimiga

> **Status: ANÁLISE, não canon.** Este documento interpreta os dados do estudo MIRA-SIM
> (10.080.000 lutas, 7 lotes x 6 braços x 240.000, zero erros internos) e apresenta opções
> de decisão. **Nenhuma opção aqui vira regra do jogo: a decisão é do líder.**
> Autor: Capitolino (CPO), 2026-08-01.
>
> Fontes: dados brutos em `/var/tmp/mira_sim_dados_final.txt` (2 camadas: números por
> lote/braço + MCID par a par); protocolo aprovado em
> `proposta-protocolo-simulacao-mira.md`. Piso de relevância fixado pelo líder ANTES dos
> dados: diferença menor que 5 pontos percentuais (ou 0,5 rodada) é EMPATE, mesmo com
> significância estatística.
>
> Validade interna: o lote L1 exigia por construção que os braços C, D, E e F saíssem
> idênticos (ninguém defende, então o fator F4 nunca dispara). Saíram idênticos até a
> última casa decimal, em L1, L4 e L5. O harness passou no próprio teste de sanidade.

---

## 1. O que o estudo respondeu, em uma página

**A coisa mais séria do estudo inteiro, e ela abre esta entrega: no jogo de hoje, ficar
só defendendo cria um empate perfeito e inquebrável.** No lote 3 (os três heróis só
defendem, exatamente o que o líder fez no playtest), sob a mira atual, 100% das 240.000
lutas terminaram em empate técnico: 30 rodadas, zero quedas, party com 100% do HP no fim,
100% do dano inimigo absorvido pelo escudo. O jogador não pode vencer (não ataca), mas
também **não pode perder, nunca**. A luta vira uma fotografia até o jogo desistir. Não é
impressão de playtest: é uma lei do motor atual.

**Segunda descoberta, que muda a pergunta original do líder:** com os statlines de
referência, **bater em quem defende causa literalmente ZERO de dano**. Em todos os lotes
onde alguém defende, a absorção do escudo foi 100% (99,8% no cenário de elite, e o 0,2%
que vazou foi o pico raro da variância de +30%). O escudo cobre o golpe inteiro e renasce
cheio na rodada seguinte, de graça. Logo, a pergunta "é melhor derrubar a parede logo ou
deixar para depois?" tem uma resposta que dissolve a pergunta: **não existe derrubar a
parede. Ela não é uma parede, é um cofre que engole socos sem arranhar.** Nem cedo, nem
tarde: bater nela nunca vale a pena, em nenhum cenário testado, nem para o inimigo de
elite.

**Terceira descoberta, a inversão do senso comum:** quanto mais esperto o inimigo (quanto
mais ele evita a parede), **pior para a party**. No lote 2 (Jaci defende todo turno), o
braço em que o inimigo quase nunca bate na parede derruba o Gus em 46% das lutas; o braço
em que o inimigo faz questão de bater na parede derruba o Gus em só 5,9%. O motivo é
simples: cada golpe que o inimigo gasta no cofre é um golpe jogado fora, e cada golpe que
ele NÃO gasta no cofre cai em quem não tem cofre, e esse dano não volta (ninguém regenera
HP de graça no nosso combate). Um defensor dedicado funciona como para-raios: ele só
protege a party se o inimigo for burro o bastante para mirar nele.

**Quarta descoberta, sobre a queixa de origem (o saco de pancadas):** o que espalha o
fogo é trocar a mira fixa por QUALQUER sorteio (a concentração cai de 100% para a faixa
de 57% a 72%). Mas, entre os braços ponderados, o ajuste fino do F4 **não mexe na
concentração**: no lote 2, todos ficam em ~72%, com diferenças abaixo do piso de 5
pontos, ou seja, empate por decisão prévia do líder. Em linguagem de jogo: com um
defensor na party, quase três quartos da pancadaria caem sempre na mesma criança, e girar
o botão do F4 não muda isso. O que muda a concentração de verdade é **quantos alvos
"moles" existem** (composição da party) e **as cartas de agro**: no lote 5, honeypot +
firewall entregaram o melhor espalhamento do estudo inteiro (57,8%, e "luta do saco de
pancadas" despenca para 14%).

**Quinta descoberta, o veto que paira sobre tudo: o Gus é o único que cai.** Nos cinco
lotes de encontro comum, toda queda registrada é dele: Cauã e Jaci, zero. A Jaci não caiu
em NENHUM dos 42 cenários do estudo. E a janela canônica de 3 a 5 rodadas quase nunca é
atingida (o melhor caso realista chega a 32%, e pelo motivo errado). Esses dois problemas
não são da mira: são dos statlines de referência e do desenho do Defender, e viram
pendências separadas na seção 5.

---

## 2. Confronto com as minhas hipóteses (o que acertei, o que errei)

Declarei três hipóteses e três condições de mudança de ideia ANTES dos dados (seção 1 do
protocolo). O confronto, sem reescrever hipótese para caber no resultado:

### H1: "evitar a parede é mecanicamente mais forte para o inimigo". CONFIRMADA, e com folga maior do que eu previa.

No lote 2, o inimigo que evita a parede (braço D) vence 46,3% das lutas; o que bate na
parede de propósito (braço E) vence 5,9%. Eu previa vantagem; os dados mostraram um
abismo de 40 pontos. E o mecanismo é o que eu apostei: o dano na parede evapora (100%
absorvido, pool renasce), o dano nos outros é permanente.

### H2: "F4 forte recria a queixa com o sinal trocado e defender vira botão de invulnerabilidade". O MECANISMO QUE EU PREVI ESTAVA ERRADO. Digo isso com todas as letras.

Eu previ duas coisas sob F4 forte (braço D): concentração de fogo pior, e exploit de
defesa eterna. **Nenhuma das duas aconteceu.** A concentração sob D é estatisticamente
igual à dos outros braços ponderados (72,4% contra 71,9% a 72,5%: empate pelo piso do
líder). E o exploit de defesa eterna sob D é o MENOR do estudo: no lote 3, turtle total
sob D perde 100% das lutas em ~7 rodadas, zero lutas batem no cap. Quem preserva o
empate eterno é a mira ATUAL (braço A, 100% de cap), não o F4 forte.

Minha condição de mudança de ideia número 2 dizia: "se D não produzir concentração nem
exploit nos lotes 2 e 3, minha ressalva era medo infundado e F4 pode ser agressivo".
**A condição foi acionada.** O medo específico que eu tinha era infundado.

Porém (e isto não é reescrever a hipótese, é reportar o que os dados mostraram no lugar
dela): D reprova por um motivo que eu NÃO previ. Sob D, defender um aliado desvia o fogo
inteiro para os moles, e o Gus cai em 46,3% das lutas de trash com parede. Isso fura o
contrato de fragilidade do Pillar 4 ("ameaçado, nunca massacrado") e dobra a
dificuldade sem ninguém ter pedido (vitória despenca de 82,6% sem F4 para 53,7%). Ou
seja: minha conclusão prática ("F4 moderado, não agressivo") sobrevive, mas pela porta
do Pillar 4 e do balance, não pela porta que eu apontei. Errei o diagnóstico e acertei o
remédio por sorte parcial. Registrado.

### H3: "quebrar a parede cedo só vence contra elite, quando o golpe médio ultrapassa o pool". REFUTADA.

Não existe cenário testado onde quebrar a parede vence, nem o de elite. No lote 6
(Daemon-Guard com Atk 18 contra a Def 10 da Jaci) a absorção ficou em 99,8%: até o golpe
de elite morre inteiro no escudo, salvo o pico raro de variância. Com a fórmula e os
statlines atuais, o overflow que sustentaria a H3 simplesmente não acontece. A exceção
que eu previ não existe no motor de hoje.

### As três condições de mudança de ideia, uma a uma

1. **"Se o braço E vencer mais para o inimigo, com espalhamento saudável e janela 3-5":
   NÃO acionada.** E é a pior política inimiga do estudo (a party vence 94,1% no lote 2).
2. **"Se D não produzir concentração nem exploit": ACIONADA**, como descrito acima. O
   medo mecânico da H2 caiu; o veto a D fica de pé por Pillar 4 e dificuldade.
3. **"Se a parede não mover nenhuma métrica, F4 é cosmético": MEIO acionada, e a nuance
   é a lição central do estudo.** F4 não move a métrica da queixa (concentração: tudo
   empate), mas move MUITO a taxa de vitória e as quedas (de 5,9% a 46,3% de quedas do
   Gus no lote 2, só girando o F4). Tradução: **F4 não é um botão de espalhamento, é um
   botão de dificuldade e de personalidade do inimigo.** Eu desenhei o estudo achando que
   media espalhamento com ele; ele mede letalidade.

---

## 3. As opções de decisão

Antes das opções, a régua honesta: pela métrica da queixa de origem (concentração de
dano), os braços B, C, D, E e F **empatam** entre si pelo piso que o líder fixou. A
escolha entre eles NÃO se decide por estatística de espalhamento: decide-se por como
cada um faz o jogo se sentir, quanto custa em quedas do Gus, e o que faz com o turtling.
É exatamente a decisão de produto que os números não tomam sozinhos. Por isso cada opção
abaixo é descrita pela experiência, com os números só como testemunhas.

Um lembrete de lore que pesa a favor da família ponderada como um todo: nossos inimigos
de trash SÃO programas (Sentinela-Bit, daemons). Uma mira que segue heurísticas legíveis
(revida quem bateu, fareja o ferido, caça o curandeiro, respeita ou despreza a guarda) é
**diegética**: o inimigo se comporta como o software que o lore diz que ele é, e o
jogador vence lendo o algoritmo. Isso é o Pillar 1 e o Pillar 2 jogando a favor. A mira
fixa (A) e a moeda (B) não têm algoritmo nenhum para ler.

### Opção A: manter a mira de hoje (sempre o primeiro da fila)

**Como o jogo se sente:** o inimigo bate no mesmo alvo em 92% dos turnos. A criança de
11 anos percebe o padrão na segunda luta e ele nunca mais surpreende: vira decoreba, não
leitura. O adulto nostálgico reconhece na hora o "inimigo burro de RPG de 1988" e não no
bom sentido: os clássicos que ele ama (Chrono Trigger à frente) já variavam alvo. E o
pior: é a opção que produz o empate eterno do turtling (100% das lutas de defesa total
não terminam NUNCA), que foi exatamente a experiência que gerou este estudo.

**O que ganha:** nada que as outras não deem. Zero custo de implementação.

**O que perde:** a queixa de origem continua (100% de concentração, o saco de pancadas
perfeito), o turtle-empate continua, e o Pillar 1 (vencer lendo o sistema) fica sem
sistema para ler.

**Veredicto do CPO:** insustentável. É a única opção que os dados reprovam sem apelação.

### Opção B: sorteio puro (moeda)

**Como o jogo se sente:** o fogo espalha (concentração cai para a faixa de 62-72%) e o
Gus cai pouco (10% no trash). Mas o inimigo não tem POR QUÊ. A criança pergunta "por que
ele veio em mim?" e a resposta verdadeira é "por nada, deu na moeda". Não há teoria para
formar, não há jogada para provocar ou evitar. O adulto nostálgico sente roleta, não
inteligência. E o turtling ainda meio-funciona (21% das lutas de defesa total batem no
cap de 30 rodadas: uma em cada cinco vira a fotografia de novo).

**O que ganha:** espalhamento honesto, implementação trivial, Gus protegido.

**O que perde:** Pillar 1 inteiro (não existe sistema para ler) e a diegese (um daemon
que sorteia alvo não parece um programa, parece um dado). Deixa vivo um terço do
problema do turtle.

**Veredicto do CPO:** é o remédio que cura a febre matando o paciente. Não recomendo.

### Opção C: mira ponderada com F4 suave (defensor atrai metade do normal). A MINHA RECOMENDAÇÃO.

**Como o jogo se sente:** o inimigo parece um programa com instinto. Ele revida quem
bateu nele (F1), fareja o ferido (F2), caça o curandeiro (F3), e **respeita a guarda sem
temê-la**: defender reduz a chance de ser alvo pela metade, mas não a zera. Na tela, a
criança vê consequência nas próprias escolhas: "eu bati forte, ele veio em mim; eu
defendi, ele hesitou". Dá para formar teoria, e teoria que funciona: é o jogo se
vencendo por leitura, que é a fantasia inteira do Gus. O adulto nostálgico reconhece o
comportamento dos bons RPGs de SNES, com um tempero a mais de coerência.

O turtling morre como exploit, e morre do jeito certo: defesa total perde 99,9% das
vezes, em ~11 rodadas. Devagar o bastante para a criança sentir que a muralha segurou um
tempo, rápido o bastante para aprender que muralha sozinha não vence guerra. A lição de
design que o jogo ensina sem tutorial: **defender é ferramenta, não estratégia.**

**O que ganha:** legibilidade (Pillar 1), diegese (Pillar 2), fim do turtle-empate, fim
da decoreba, e o melhor compromisso do estudo entre espalhamento e inteligência
aparente.

**O que perde (e precisa de contrapeso, sem exceção):** dificuldade sobe. No cenário de
parede dedicada, a vitória cai de 100% (hoje) para 69,5%, e **o Gus cai em 30,5% das
lutas**. Isso, cru, fura o contrato "ameaçado, nunca massacrado" do Pillar 4: quase uma
luta em três com o protagonista no chão é massacre, não ameaça. O contrapeso existe e o
próprio estudo o validou: no lote 5, com as cartas de agro em jogo (honeypot do Cauã +
firewall no Gus), as quedas do Gus despencam para 0,4% e o espalhamento é o melhor do
estudo. **Logo, a Opção C só é aprovável em pacote: mira C + firewall barato e acessível
cedo no jogo + revisão dos statlines de referência (seção 5).** Sem o pacote, C reprova
no Pillar 4.

### Opção D: mira ponderada com F4 forte (defensor quase intocável)

**Como o jogo se sente:** o inimigo é um predador cirúrgico. Ele quase nunca desperdiça
um golpe na guarda: desvia e esmaga quem está exposto. É o comportamento inimigo mais
"inteligente" do estudo e o mais legível dos ponderados (repete o alvo em 44% dos
turnos: dá para prever a próxima jogada dele, o que telegrapha bem). E é o carrasco
definitivo do turtling: defesa total perde 100% das vezes em 7 rodadas.

Mas a experiência para a criança tem um veneno escondido: **defender um aliado deixa de
proteger a party**. Você levanta o escudo na Jaci e o fogo inteiro escorre para o Gus e
o Cauã. A jogada que toda criança aprende como "cuidar dos amigos" vira, na prática,
"entregar os amigos". No número: o Gus cai em **46,3% das lutas de trash com parede**.
Quase metade das brigas de esquina termina com o menino de 11 anos no chão. Isso não é
tensão, é surra, e fura o Pillar 4 de forma frontal. O adulto hardcore talvez respeite a
crueldade; o nosso público de 11 anos vai sentir o jogo como injusto, e injustiça
percebida é a emoção que mais rápido faz uma criança largar o controle.

**O que ganha:** a IA mais impressionante, o melhor telegraph, a melhor punição de
turtle, e (ironia registrada na seção 2) a melhor janela 3-5 do estudo (32%).

**O que perde:** o Pillar 4, pelo motivo acima. E a janela 3-5 que ele "ganha" é
conquistada do jeito errado: as lutas encurtam porque o Gus morre cedo, não porque o
ritmo melhorou.

**Veredicto do CPO:** vetado como padrão de trash. MAS não joguem fora: D é uma
personalidade de inimigo excelente para um arquétipo específico (um "Caçador" que o lore
apresente como rastreador de fraquezas, em tier mais alto, com telegraph claro). Como
tempero raro e anunciado, a crueldade vira drama. Como dieta diária, vira churn.

### Opção E: F4 invertido (o inimigo prefere bater em quem defende: "quebrar a parede cedo")

Esta era a hipótese rival que motivou a pergunta-mãe. Os dados foram impiedosos com ela.

**Como o jogo se sente:** parece ótimo na primeira hora. A criança defende e VÊ o escudo
brilhar engolindo os golpes: sensação imediata de "funcionou!". Mas o motor por trás é o
inimigo jogando dano no lixo (100% de absorção, escudo renasce), e o jogo desaba de
dificuldade: a party vence 94,1% no cenário de parede. Pior: nasce uma estratégia
dominante silenciosa. Se defender atrai o inimigo E defende perfeitamente, então
"alguém defende, os outros batem" vira a resposta certa de TODA luta: um honeypot
grátis, permanente, sem carta e sem custo. Isso canibaliza a carta de honeypot (por que
gastar uma carta para fazer o que o botão Defender já faz de graça?) e reinstala o
empate preguiçoso: 30,7% das lutas de defesa total ainda batem no cap de 30 rodadas. O
adulto nostálgico descobre o padrão em duas lutas e nunca mais pensa: é o anti-Pillar 1.

**O que ganha:** feedback visual delicioso de escudo trabalhando, jogo mais fácil (se
algum dia quisermos um modo bebê, E é ele).

**O que perde:** o desafio, a carta de honeypot, e a honestidade da IA (um daemon que
escolhe a pior jogada disponível não parece software, parece defeito).

**Veredicto do CPO:** reprovado como padrão. A intuição "quebre a parede enquanto está
forte" pressupõe uma parede quebrável, e a nossa não é (seção 5, pendência 1). Registro
com respeito: era uma hipótese legítima, e foi exatamente para isso que o estudo rodou.

### Opção F: mira ponderada sem F4 (a guarda é ignorada pelo sorteio)

**Como o jogo se sente:** quase igual à C, com um detalhe elegante: mesmo sem peso
explícito, o defensor JÁ atrai menos fogo com o tempo, porque quem defende não causa
dano, e quem não causa dano perde o peso de revide (F1). A "evitação da parede" emerge
sozinha do sistema, em vez de ser programada. Filosoficamente é a opção mais bonita
(menos botões, comportamento emergente). Na prática: vitória 82,6%, Gus cai 17,4%,
turtle perde 97% (mas em 16 rodadas, arrastado), e a repetição de alvo fica em 32%,
quase no nível da moeda: o inimigo reage à guarda tão sutilmente que o jogador talvez
nem perceba que ela importa.

**O que ganha:** simplicidade (um fator a menos para balancear e explicar), risco menor
para o Gus que a C crua (17,4% contra 30,5%).

**O que perde:** legibilidade da relação com a guarda. Se o jogador não percebe que
defender muda a mira, perdemos a conversa tática ("vou defender para ele me largar")
que é o coração do que queremos criar. E o turtle morre devagar demais: 16 rodadas de
agonia é tédio, não lição.

**Veredicto do CPO:** segunda colocada digna. Se o líder preferir começar minimalista e
só adicionar F4 se o playtest pedir, F é uma escolha defensável e barata de evoluir.

### Resumo comparativo em uma tabela (lote 2, o cenário da pergunta-mãe)

| Braço | Vitória da party | Quedas do Gus | Turtle bate no cap (L3) | Repetição de alvo | Sensação em uma frase |
|---|---|---|---|---|---|
| A hoje | 100% | 0% | **100% (empate eterno)** | 92% | Inimigo decoreba, luta-fotografia |
| B moeda | 89,7% | 10,3% | 21% | 31% | Roleta sem porquê |
| **C suave** | **69,5%** | **30,5% (0,4% com cartas)** | **0,1%** | **36%** | **Programa com instinto que respeita a guarda** |
| D forte | 53,7% | 46,3% | 0% | 44% | Predador cirúrgico, cruel demais para 11 anos |
| E invertido | 94,1% | 5,9% | 31% | 33% | Escudo-ímã, jogo fácil, IA com cara de defeito |
| F sem F4 | 82,6% | 17,4% | 2,9% | 32% | Elegante e emergente, mas sutil demais |

### Recomendação (a decisão é do líder)

**Recomendo a Opção C, em pacote fechado com três contrapesos**, porque é a única que
entrega ao mesmo tempo: sistema legível para vencer lendo (Pillar 1), inimigo que se
comporta como o software que o lore promete (Pillar 2), morte limpa do turtle-empate, e
fim do saco de pancadas. Os contrapesos NÃO são opcionais:

1. **Firewall/proteção do Gus acessível cedo** (o lote 5 prova que zera o problema:
   quedas de 30,5% para 0,4%).
2. **Revisão dos statlines de referência** antes de qualquer número virar canon
   (pendências 2 e 3 da seção 5: a janela 3-5 e o Defend binário não se consertam pela
   mira).
3. **Telegraph na tela** (ícone de intenção/frase), porque legibilidade medida no
   headless só vira legibilidade sentida com apresentação.

Alternativa defensável se o líder preferir partir do mínimo: **Opção F agora, F4 suave
depois** se o playtest mostrar que a relação guarda-mira está invisível. E guardar a
Opção D como personalidade de um arquétipo raro de inimigo, não como padrão.

A escolha entre C, D, E e F não é estatística (empate na métrica da queixa, pelo piso
que o próprio líder fixou): é uma escolha de sensação e de pilar. Por isso ela é dele.

---

## 4. O que o estudo NÃO respondeu (e só o playtest humano responde)

A lista da seção 6 do protocolo continua valendo inteira; atualizo com o que aprendi:

1. **Diversão continua fora do alcance.** Medimos proxies. O braço C "parece" certo nos
   números; se ele É divertido, só o líder e o Gus Dragon jogando dizem.
2. **Legibilidade percebida ganhou urgência.** O estudo mostrou que a diferença entre C
   e F é justamente "o jogador percebe ou não percebe a reação à guarda". Isso é 100%
   telegraph de tela, invisível ao headless. Sem o telegraph pronto, o playtest não
   consegue nem comparar C com F honestamente.
3. **A sensação de injustiça da criança.** 30,5% de quedas do Gus sob C cru é número;
   se com as cartas na mão a criança SENTE proteção ou sente medo, é playtest.
4. **O drama de ser focado (F7, vendeta)** segue fora por desenho: dramaturgia.
5. **O jogador real que muda de plano no meio da luta.** O bot defende todo turno ou
   ataca todo turno; um humano alterna, e a dança mira-reage-jogador-reage é o produto
   final. O headless não dança.
6. **Chefes** seguem fora: puzzle desenhado à mão.
7. **NOVO, aprendido agora: o mecanismo exato do dano que fura a defesa total.** No lote
   3, mesmo com os três defendendo e absorção 100%, o dano entra aos poucos nos braços
   B a F (o Gus cai na rodada ~20 sob B). A explicação provável está nas janelas da fila
   CTB em que o escudo de Duration 1 expira antes do herói re-defender, mas isso é
   inferência minha: recomendo re-rodar meia dúzia de seeds do L3-B golpe a golpe (o
   harness permite) ANTES de aceitar esse comportamento como canônico, porque na tela
   ele pode parecer "eu defendi e levei dano mesmo assim", que é a receita da injustiça
   percebida. Se for esse o mecanismo, ele precisa de telegraph próprio ou de ajuste.

---

## 5. Achados colaterais (não eram a pergunta, mas mandam no roadmap)

**Colateral 1, o mais grave: o empate perfeito do turtling é lei do motor atual.**
Mira fixa + defesa total = 30 rodadas, zero dano em qualquer direção, 100% das vezes.
Qualquer opção exceto A conserta; mas o achado merece registro próprio porque é a prova
de que o playtest do líder não foi azar: foi determinismo.

**Colateral 2: o Defend é binário e o overflow é letra morta.** Com dano tipo Atk menos
Def e Shield com pool igual à Def, o golpe de trash contra qualquer defensor é absorvido
INTEIRO (absorção 100% em todos os braços de todos os lotes com defensor; 99,8% até
contra elite de Atk 18). Consequências de design: (a) não existe "quebrar a parede", a
pergunta-mãe não tem objeto no motor de hoje; (b) defender é imunidade total enquanto
dura, o que empobrece a escolha (ou o inimigo bate no cofre e desperdiça, ou desvia e o
cofre é irrelevante). Se o líder QUISER que "derrubo a parede agora ou depois?" exista
como dilema real no jogo (e é um dilema saboroso), o Shield precisa mudar: pool que
persiste e desgasta entre rodadas, ou absorção parcial, ou custo de recurso para
re-defender. Isso é uma decisão de design separada, one-way door leve, que este estudo
apenas escancarou.

**Colateral 3: os statlines de referência quebram nas duas pontas.** Com os números
`//SIM` atuais: a curandeira torna o trash IMPERDÍVEL (lote 4: 100% de vitória em todos
os braços, cura 12 anula o dano líquido, zero tensão), e o elite torna a luta INGANHÁVEL
(lotes 6 e 7: 100% de derrota em praticamente todos os braços; a party não mata um
Daemon-Guard nem em 9 rodadas). Nenhuma política de mira conserta ponta nenhuma. Os
statlines precisam de estudo próprio antes do primeiro número virar canon.

**Colateral 4: a janela canônica de 3 a 5 rodadas está fora de alcance, e a culpa NÃO é
da mira.** Trash vanilla dura 7 rodadas cravadas (mediana 7 em todos os braços do lote
1; a duração entre braços ponderados difere menos de 0,5 rodada, empate pelo piso). O
único braço que se aproxima da janela (D, 32%) chega lá matando o Gus cedo, que é o
jeito errado. Para cumprir o canon dos 3 a 5, o caminho é dano/HP (statlines, fórmula),
não mira. Recomendo estudo-irmão: "MIRA-SIM 2: pacing", variando statlines com a mira já
decidida.

**Colateral 5, o achado feliz: as cartas de agro funcionam exatamente como prometido.**
Honeypot não é suicídio (vitória 99,6% no lote 5, o portador com +10% Def aguenta),
firewall protege sem imortalizar (fallback de re-normalização funcionou, zero erro
interno), e juntas entregam o melhor espalhamento do estudo (57,8%, saco de pancadas
14%). A camada tática de cartas é a alavanca REAL de espalhamento e de proteção do Gus,
muito mais que qualquer ajuste de F4. Isso fortalece o coração do jogo: quem quer
controlar a mira inimiga deve fazê-lo JOGANDO CARTAS, não girando um número escondido.
Magia é software: o jogador programa o comportamento do inimigo. É a nossa fantasia
central, validada por 1,4 milhão de lutas.

**Colateral 6: a mira ponderada é mais letal que a moeda mesmo sem F4, e a vítima é
sempre o Gus.** No lote 1 (ninguém defende), a família ponderada vence 83,3% contra
89,7% da moeda, e as quedas do Gus sobem de 10,3% para 16,7%. O culpado é o F2 (ferido
atrai): o Gus, mais frágil, apanha, fica ferido, e ferido atrai mais. É um ciclo de
realimentação que o design precisa conhecer: o F2 é o fator que transforma "inimigo
esperto" em "inimigo que persegue o protagonista". Se o playtest mostrar excesso,
o primeiro botão a girar é o teto do F2, não o F4.

**Colateral 7: a Jaci é indestrutível e o Cauã quase.** Jaci: zero quedas em 42 cenários
(HP 55, Def 10: o trash não a arranha nem sem escudo). Cauã só cai contra elite. A
fragilidade da party inteira mora no Gus (HP 34, Def 5). Isso é parcialmente desejado
(contrato de fragilidade), mas o grau atual concentra TODO o risco do jogo num único
personagem, o que empobrece a tensão dos outros dois. Vai junto para o estudo de
statlines do Colateral 3.

---

*Análise escrita por Capitolino (CPO) em 2026-08-01. Não é canon; não foi commitada.
Todos os números citados vêm de `/var/tmp/mira_sim_dados_final.txt` e podem ser
reproduzidos por seed. A decisão, em cada item, é do líder.*

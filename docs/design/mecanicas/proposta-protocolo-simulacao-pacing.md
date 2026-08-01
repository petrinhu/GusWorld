# PROPOSTA: Protocolo do estudo de simulação de pacing (MIRA-SIM 2)

> ### ✅ PRÉ-REGISTRO APROVADO PELO LÍDER EM 2026-08-01 (antes de qualquer dado)
>
> | Parâmetro | Valor fixado |
> |---|---|
> | **Taxa de vitória alvo, trash** | **90% a 97%** |
> | **Taxa de vitória alvo, elite** | **55% a 80%** |
> | **Teto de quedas do Gus, trash** | **8% a 12%** |
> | **Teto de quedas do Gus, elite** | **30% a 40%** |
> | **N** | **amostra cheia (240.000) em TODO ponto do espaço**, não escalonada: ~40 milhões de lutas, ~13 min |
> | **Piso de relevância (MCID)** | mantido em **5 pontos percentuais / 0,5 rodada**, o mesmo do estudo da mira |
>
> Os quatro primeiros são **decisão de produto pré-registrada**: foram fixados ANTES de
> qualquer número existir, exatamente como o MCID foi no estudo anterior, para que a
> aprovação de um candidato não possa ser negociada depois de ver o resultado.
>
> O líder recusou a amostra escalonada e pediu amostra cheia também na varredura ampla,
> pelo mesmo motivo que subiu o N do estudo anterior: o custo de máquina é irrelevante e
> estatística é ciência de medir repetições.
>
> ### ⚠️ PENDÊNCIA A RESOLVER ANTES DE CALIBRAR: turno ou rodada?
>
> A hipótese central deste protocolo se apoia na defasagem entre a inflação de HP de
> 2026-06-03 e o aperto da janela de 2026-07-19. **A verificação independente do
> orquestrador confirmou a inflação** (`combat.md` §17: *"HP pós-inflação +60% (Trash
> 34→55, Elite 89→144)"*, e o registro de revisão de 2026-06-03 na mesma página), **mas
> achou uma ambiguidade de UNIDADE no mesmo trecho**: a nota da inflação diz *"TTK alvo
> 3-5 **turnos**"*, enquanto a decisão de 2026-07-19 (§15.1) fala em *"3-5 **rodadas**"*.
>
> Não são a mesma coisa: §4.1 define **RODADA = uma volta completa da fila CTB**, logo
> com 3 heróis e 3 inimigos uma rodada são 6 turnos. Se a inflação de junho mirou uma
> unidade e a janela de julho mira outra, parte do desalinhamento medido é **confusão de
> unidade no histórico do projeto**, não só defasagem de calibragem. Calibrar 40 milhões
> de lutas contra um alvo cuja unidade está em dúvida é o erro caro deste estudo.
>
> **Resolver ANTES de rodar**, e declarar a resposta no relatório.

> **Status: PROPOSTA, não canon.** Nada aqui vira regra do jogo. Este é o protocolo do
> estudo-irmão de ritmo, autorizado pelo líder em 2026-08-01 após o estudo da mira.
> Autor: Capitolino (CPO). Nenhum número deste doc entra no motor como valor de
> produção; todos os valores de simulação são marcados `//SIM`. **Statline de personagem
> é matéria canônica e algumas peças são one-way door de sensação: este estudo propõe
> FAIXAS e trade-offs; os números finais são SEMPRE decisão do líder.**
>
> Origem: achado do MIRA-SIM (ver `analise-mira-resultados.md`, colaterais 3 e 4): a
> janela canônica de 3 a 5 rodadas (`combat.md` §15.1) não é alcançada por causa da
> mira. O trash dura 7 rodadas em todos os seis braços testados, com diferenças abaixo
> de 0,5 rodada (empate pelo piso do líder). A causa está nos statlines e no ritmo de
> dano, não em quem o inimigo escolhe. E os statlines de referência quebram nas duas
> pontas: a curandeira torna o trash imperdível (L4: 100% de vitória em todo braço) e o
> elite torna a luta inganhável (L6/L7: ~100% de derrota).
>
> Cross-ref: `combat.md` (§11 fórmula, §13.1 AP por tier, §15.1 janela, tabela de stats
> de referência), `proposta-protocolo-simulacao-mira.md` (método-pai),
> `analise-mira-resultados.md` (achados que motivam), `economia.md` (death-loop
> econômico §3.3), `pillars.md`.

---

## 0. Princípio do estudo (a régua que decide "certo")

**A pergunta:** quais números colocam o encontro comum dentro da janela de 3 a 5
rodadas SEM destruir o resto da experiência?

A armadilha desta pergunta está na segunda metade, e eu a declaro antes de qualquer
número: **a janela é um meio, não o fim.** É trivial pôr uma luta em 3 rodadas: basta
dar dano absurdo à party (a luta vira formalidade oca) ou dano absurdo ao inimigo (a
luta vira loteria de quem morre primeiro, e quem morre é o Gus). As duas "soluções"
cumprem a métrica e destroem o jogo. Por isso este protocolo trata a duração como
métrica primária CERCADA de guarda-corpos pré-declarados (seção 4), e nenhum candidato
a statline aprova por entrar na janela: aprova por entrar na janela **com todos os
guarda-corpos verdes ao mesmo tempo**.

**Insumo novo do líder (decisão de 2026-08-01):** a esperteza da mira escalona por
periculosidade do inimigo. Minion comum usa política mais burra na maioria das vezes
(o jogador vence mais e fica feliz, sem perder o desafio) e a esperteza sobe
proporcionalmente até o mestre final, dentro da doutrina canônica em elaboração
("quem é comedido sofre menos no final; o esbanjador sofre mais"). O `economy-designer`
está canonizando a doutrina e a tabela tier para política de mira **em paralelo a esta
proposta**. Este protocolo NÃO propõe outra tabela: **consome a dele como insumo**
(seção 6, valor V1). A mira fica FIXA por tier durante o estudo; a variável
experimental agora é o espaço de números.

**Validade interna, herdada do estudo-pai:** motor real (`CombatStateMachine` headless,
mesma FSM dos testes), fórmula de dano §11 real (ataque básico subtrativo
`clamp(Atk - Def, 1)`; carta pela cadeia divisiva com canal FALHA/CRIT/COMUM), RNG
injetado e seedável, cada luta reproduzível golpe a golpe pela seed.

---

## 1. Opinião prévia declarada (para os dados poderem me contrariar)

Mesma disciplina do estudo anterior: opinião por escrito ANTES dos dados, com as
condições que me fariam mudar de ideia. No estudo da mira, o mecanismo da minha H2
estava errado e eu disse isso com todas as letras; mantenho o padrão.

### 1.1 O fato do motor que estrutura tudo

A duração de uma luta de trash é, na essência, uma divisão: HP total dos inimigos
dividido pelo dano que a party entrega por rodada (chamo de TTK-out, "tempo até matar").
Já o perigo da luta é outra divisão: HP dos heróis dividido pelo dano que o inimigo
entrega por rodada (TTK-in). **As duas divisões usam números diferentes**: TTK-out
depende do HP inimigo e do ataque da party; TTK-in depende do Atk inimigo e do HP/Def
dos heróis. Essa quase-independência é a esperança do estudo inteiro: talvez dê para
encurtar a luta (mexer no TTK-out) sem tornar o Gus mais massacrável (sem mexer no
TTK-in).

**Correção de narrativa causal (2026-08-01, achado do orquestrador, verificado pelo
team-lead e decidido pelo líder):** a versão anterior deste parágrafo dizia que o HP do
trash foi "calibrado quando a janela-alvo ainda era 4 a 8 rodadas". Essa frase não tem
lastro documental. O `git log -S` em `combat.md` mostra que a string "4-8" não existe em
nenhum commit antes de 2026-07-19 — a primeira aparição é o PRÓPRIO commit que decide
"3 a 5" (`ed411ec9`, §15.1), que chama "4-8" de "janela histórica" sem nenhum registro
escrito anterior que a sustente. O que está de fato escrito em 2026-06-03 (§17, mesmo
commit que traz o HP +60%) é **"TTK alvo 3-5 TURNOS"**, não rodadas. E "turno", no
vocabulário do próprio documento naquele dia, significa a vez de UM ator (a mesma seção
de AP, no mesmo commit, diz "AP pertence ao ator ativo no turno... cada membro da party
tem seus 3 AP independentes quando age na fila" — bate com o motor real de hoje, onde
`begin_turn()` é por ator, não por rodada). Prova adicional: no MESMO dia, em outra
seção (§18, ciclo de ambientes), foi preciso um override explícito — "'Turnos' do ciclo
= rodadas completas de fila" — e só se escreve override quando o sentido padrão é
outro. Ou seja: a inflação de HP de junho foi calibrada contra uma **unidade diferente**
(turnos de ator, não rodadas), e a duração em RODADAS provavelmente nunca foi verificada
até o MIRA-SIM medir 7, cravadas, em 2026-08-01. O alvo ATIVO deste estudo continua
sendo **3 a 5 RODADAS** (§15.1, decisão do líder com o Gus em 2026-07-19; rodada definida
sem ambiguidade em §4.1 como "uma volta completa da fila CTB") — isso não muda. O que
muda é a história de por que o HP está fora da janela: não é uma janela que apertou por
cima de uma calibração antiga na MESMA unidade, é uma calibração antiga numa unidade
diferente, talvez nunca verificada em rodadas. O estudo da mira mediu a consequência
mensurável, em rodadas: 7 cravadas.

**Segunda decisão do líder daí derivada:** já que junho falava de "turnos de ator", e
essa unidade mede naturalmente algo mais parecido com "quantos golpes derrubam um
inimigo" do que "quantas rodadas dura a luta", este estudo mede TAMBÉM essa métrica
separada (E11, seção 4) — não para substituir a janela de 3-5 rodadas (que é lei,
§15.1), mas para recuperar a intenção original de junho em vez de descartá-la. **São
dois alvos distintos que coincidem no número "3 a 5" por acidente histórico de
terminologia**; o relatório final declara os dois separadamente, para ninguém confundir
de novo.

### 1.2 Minhas hipóteses

- **H1 (a alavanca):** a duração do trash é dominada pelo TTK-out, e a alavanca mais
  barata e mais segura é o **HP do inimigo**, não o ataque da party (que é canônico e
  one-way door de sensação) nem o Atk inimigo (que mexe no perigo, não no ritmo).
  Aposto que o HP de trash que entra na janela está entre 60% e 80% da referência
  atual (isto é, desfazer parte da inflação de +60%).
- **H2 (o desacoplamento):** dá para entrar na janela 3 a 5 sem elevar as quedas do
  Gus acima do teto que o líder fixar, porque duração e perigo têm alavancas quase
  ortogonais. O acoplamento residual que temo: menos rodadas = menos golpes sofridos
  no total (bom para o Gus), mas se a mira ponderada estiver ligada, o fator F2
  (ferido atrai) tem menos tempo para formar o ciclo de perseguição (também bom).
  Aposto que as quedas do Gus CAEM junto com a duração, não sobem.
- **H3 (as pontas):** as duas pontas quebradas se consertam com número, sem redesign:
  a curandeira deixa de trivializar quando o dano líquido inimigo por rodada superar a
  cura por rodada (hoje: cura 12 `//SIM` engole os ~15 de dano bruto que viram ~5
  líquidos); o elite vira ganhável quando o TTK-out da party contra o HP 144 couber no
  envelope de ~5 a 8 rodadas com o elite pressionando de volta.
- **H3b (a exceção que NÃO se conserta por número, e declaro para não fingir depois):**
  o Defender binário (absorção 100%, colateral 2 do estudo-pai) **não sai por statline
  dentro de faixa razoável**. Com ataque subtrativo, o dano do trash contra quem
  defende nunca supera o pool do Shield (que é a própria Def), a menos que o Atk
  inimigo suba a ponto de massacrar quem NÃO defende. Se o líder quiser que "quebrar a
  parede" exista como dilema, é mudança de REGRA do Shield (design, one-way door
  separado), não deste estudo. O estudo vai MEDIR a absorção em todos os candidatos
  para provar ou refutar isto, mas não vai perseguir conserto.

### 1.3 O que me faria mudar de ideia

1. **Se baixar o HP inimigo até a janela arrastar as quedas do Gus acima do teto** (ou
   o contrário: se nenhum ponto entrar na janela sem estourar quedas), o desacoplamento
   da H2 era ilusão, duração e perigo estão acoplados de verdade, e a resposta honesta
   vira "não há tabela de números boa: é preciso mexer em regra" (AP, cura, Shield).
2. **Se NENHUM ponto da grade entrar na janela com guarda-corpos verdes**, mesmo no
   refino, o problema não são os números, é a FÓRMULA (o ataque subtrativo linear com
   clamp em 1 cria degraus grossos demais). Aí o estudo devolve "redesign necessário"
   em vez de tabela, e digo isso sem maquiar.
3. **Se a variância dominar a duração** (candidato com média 4 mas p10-p90 tipo 2 a 9:
   metade das lutas fora da janela mesmo com a média perfeita), a alavanca não é
   statline, é a curva de variância do Knowledge (§11), e isso é outra decisão, com
   outro dono (o líder), que eu levaria a ele em separado.

---

## 2. Desenho do experimento

### 2.1 Por que força bruta não cabe, e o que fazemos no lugar

No estudo da mira havia 6 políticas discretas: 6 braços, ponto final. Aqui o espaço é
contínuo e multiplicativo: HP, Atk, Def, SPD de 2 tipos de inimigo, mais 3 heróis com 4
stats cada, mais AP por tier, mais o valor da cura. Uma grade ingênua de 5 valores por
eixo em 12 eixos daria 244 milhões de pontos; a 240.000 lutas por ponto, é mais de um
quatrilhão de lutas. Não é questão de paciência, é questão de que 99,99% desses pontos
não interessam a ninguém.

O método proposto, em três fases, com a dimensão CORTADA antes de qualquer grade:

**Corte de dimensão (a decisão mais importante do desenho).** Ficam FIXOS no valor
canônico/de referência, fora da variação:

- **Os statlines da PARTY inteiros** (Gus 34/8/5/9, Cauã 55/14/8/13, Jaci 55/9/10/7).
  Motivo: são matéria canônica com peças one-way door (HP 34 do Gus é hard cap §2.1, o
  contrato de fragilidade É o personagem). Mexer no inimigo é reversível e invisível ao
  jogador; mexer no herói muda a identidade de quem ele controla. A party entra só num
  ANEXO de sensibilidade (seção 2.4), informativo, sem candidatura.
- **A fórmula de dano (§11)**: canon, não se toca.
- **SPD de todos**: mexe em ordem de turno, que é sensação de iniciativa, outra
  conversa. Fixa nos valores do estudo-pai (`//SIM` V9: Sentinela 8, Daemon 10).
- **Def da party e Def inimiga**: a Def inimiga interage com o clamp do ataque básico
  (Gus com Atk 8 contra Def 8 causa 1 de dano: o clamp já está ativo) e com a cadeia
  divisiva das cartas; variá-la duplicaria a grade para responder a mesma pergunta que
  o HP responde mais limpo (as duas são "esponja"). Declaro o trade-off: se o refino
  mostrar que HP sozinho não basta, Def inimiga entra numa extensão, com aviso.

Sobram **4 eixos experimentais**, todos do lado inimigo ou de regra já pendente:

| Eixo | O que é | Valores da grade grossa (`//SIM`) |
|---|---|---|
| **X1** | HP do inimigo (multiplicador sobre a referência) | trash: 40% / 60% / 80% / 100% (HP 22 / 33 / 44 / 55). elite: 62% / 80% / 100% (HP 90 / 115 / 144) |
| **X2** | Atk do inimigo | trash: 8 / 10 / 12 / 14. elite: 14 / 18 / 22 |
| **X3** | AP por turno do elite | 1 / 2 (§13.1 manda 2 na regra de design; o motor entrega 1; o estudo mede as DUAS para dar ao líder o custo real do 2) |
| **X4** | Cura da Jaci por uso (só no cenário com healer) | 6 / 9 / 12 |

**Fase A: grade grossa fatorial (screening).** Todos os cruzamentos dos eixos, por
tier, sob os cenários da seção 2.2. Fatorial e não um-eixo-por-vez, de propósito: o
estudo da mira ensinou que as interações são onde mora a surpresa (F4 não mexia na
métrica que eu esperava e mexia muito na que eu não esperava). Variar um eixo por vez
esconde interação HP×Atk (ex.: HP baixo + Atk alto = luta curta E mortal, o pior
mundo). A grade grossa é barata (seção 3) e mapeia o terreno inteiro.

**Fase B: refino local.** A resposta duração-por-HP é monotônica (mais HP, mais
rodadas), então entre os dois pontos da grade grossa que cercam a janela dá para
bissecar: 1 a 2 passos de refino com granularidade de ~10% de HP e ±1 de Atk, só na
vizinhança dos pontos que passaram na Fase A com guarda-corpos verdes.

**Fase C: confirmação adversarial.** Os 2 a 4 candidatos finais rodam com N pleno
(240.000) na bateria COMPLETA de métricas e nos cenários de validação (seção 2.3),
incluindo os cenários que quebraram no estudo-pai (parede dedicada, turtle total,
healer). Um candidato só chega ao líder se sobreviver à Fase C inteira.

### 2.2 Cenários por tier (a população de cada medição)

Herdados do estudo-pai, enxugados para os que produziram informação:

| # | Cenário | Composição | Pergunta que responde |
|---|---|---|---|
| **P1** | Trash vanilla | party 3 vs 3 Sentinela-Bit, bot ataca | A janela, na forma pura |
| **P2** | Trash com parede | Jaci defende todo turno | A janela sobrevive a um defensor? E o Gus? |
| **P3** | Trash com healer | Jaci cura o mais ferido (eixo X4) | O trash deixa de ser imperdível? Vira perdível DEMAIS? |
| **P4** | Elite + escolta | 1 Daemon-Guard (X2, X3) + 2 Sentinela-Bit, heróis a 70% HP | O elite vira ganhável sob pressão? |
| **P5** | Elite solo | 1 Daemon-Guard vs party 3 | O envelope do elite sem diluição |
| **P6** | Turtle total (só validação, Fase C) | os 3 só defendem | Regressão: o empate eterno tem que continuar morto e a derrota do turtle não pode virar massacre relâmpago |

Mira fixa por tier em TODOS os cenários, vinda da tabela do `economy-designer` (V1).
Como robustez, a Fase C re-roda os candidatos sob UMA mira alternativa (a vizinha na
tabela dele): um statline que só funciona sob uma mira exata é frágil demais para
canonizar.

### 2.3 O que fica de fora, e por quê

- **Boss e mini-boss:** puzzle desenhado à mão (canon §15.1); o envelope 12 a 18
  rodadas se valida no design da luta, não em estatística de statline.
- **Cartas do jogador além do bot sub-ótimo:** o `AutoResolveBrain` continua sendo o
  jogador simulado (ataque básico + no máximo 1 carta single-target). Viés declarado:
  jogador real com deck bom mata mais rápido, então a janela real tende a ficar MAIS
  curta que a medida. O erro é na direção segura (se o headless entra na janela, o
  humano entra com folga).
- **Economia entre encontros** (a doutrina do comedido/esbanjador, custo de Hospital,
  net por encontro): é a fronteira com o trabalho do `economy-designer`. Este estudo
  entrega a ele o dano médio SOFRIDO por encontro de cada candidato (métrica E10), que
  é o número que a economia dele consome; a simulação de campanha é dele, não daqui.
- **Knowledge acima de zero:** tudo roda no pior caso (kills = 0, variância ±30%,
  falha 5%), como no estudo-pai. A campanha real só melhora a partir daí.
- **Mudança de regra do Shield/Defend:** medida, reportada, não perseguida (H3b).

### 2.4 Anexo de sensibilidade da party (informativo, sem candidatura)

Uma varredura pequena e separada: os candidatos finais re-rodam com Atk da party ±2 e
HP da party ±20%, N reduzido. NÃO produz recomendação de mexer na party; produz o mapa
de "quanto o equilíbrio aguenta antes de quebrar" para quando o líder for decidir
statline de personagem novo ou item de equipamento. Barato agora, caro de refazer
depois.

---

## 3. Quantas lutas (o N e a conta do custo, medida e não estimada)

Régua de máquina: **~50.000 lutas/segundo** no harness real (medição de 2026-08-01,
binário `linux-release`).

| Fase | Pontos (conta explícita) | N por ponto | Lutas | Tempo estimado |
|---|---|---|---|---|
| **A screening trash** | X1(4) × X2(4) × cenários P1-P3, com P3 desdobrando X4(3): 16 × (1+1+3) = 80 pontos | 24.000 | 1.920.000 | ~40 s |
| **A screening elite** | X1(3) × X2(3) × X3(2) × cenários P4-P5(2) = 36 pontos | 24.000 | 864.000 | ~17 s |
| **B refino** | ~30 pontos na vizinhança dos aprovados | 60.000 | 1.800.000 | ~36 s |
| **C confirmação** | ≤4 candidatos × 6 cenários × 2 miras (titular + robustez) ≈ 48 células | 240.000 | 11.520.000 | ~4 min |
| **Anexo party** | ~16 células | 24.000 | 384.000 | ~8 s |
| **Total** | | | **~16,5 milhões** | **~6 min de CPU** |

Por que N escalonado e não 240.000 em tudo: no screening a pergunta é só "este ponto
merece refino?", e 24.000 lutas dão margem de erro de ~±0,6 ponto percentual, dez vezes
mais fina que o piso de relevância de 5 pontos. Gastar 240.000 ali seria precisão de
farmácia para pesar melancia. O N pleno fica para a Fase C, onde a decisão mora. Se o
líder preferir N pleno em tudo por princípio, o total sobe para ~40 milhões de lutas
(~13 min): segue barato, é decisão dele.

---

## 4. Métricas: a primária e os guarda-corpos que impedem o erro óbvio

A batalha da seção 0 em forma de tabela. **E2 é a primária; TODAS as outras são
condição de aprovação, não enfeite.** Um candidato reprova por qualquer guarda-corpo
vermelho, mesmo com E2 perfeita.

| # | Métrica | O que mede | Régua de aprovação |
|---|---|---|---|
| **E2** | **Duração**: % de lutas na janela 3-5, média, mediana, p10-p90 | A pergunta do estudo | **Primária:** maximizar % na janela; p10 ≥ 2 e p90 ≤ 7 (a CAUDA importa: média boa com cauda longa é mentira estatística) |
| **E11 (nova, 2026-08-01)** | **Golpes para derrubar UM inimigo comum**: quantos ataques básicos da party incidem sobre o PRIMEIRO inimigo abatido (cenário P1) até ele morrer, média/mediana/p10-p90 | Recupera a intenção original de junho de 2026 ("TTK alvo 3-5 TURNOS" = turno de ATOR, não rodada - ver §1.1) como métrica PRÓPRIA, separada de E2. **NÃO é a mesma pergunta que E2** ("quantos golpes matam um inimigo" ≠ "quantas rodadas dura a luta inteira contra vários inimigos"); os dois alvos coincidem em "3 a 5" por acidente de terminologia, não por serem a mesma medida | Proposto (a fixar pelo líder, mesmo pré-registro de E1/E4 na seção 4.1): mediana entre 3 e 5 golpes. Informativa até o líder fixar a régua; NUNCA usada para aprovar/reprovar candidato sem essa fixação prévia |
| **E1** | Taxa de vitória da party | O "vence mais e fica feliz" do líder | Faixa-alvo por tier FIXADA PELO LÍDER antes de rodar (proposta na seção 4.1) |
| **E4** | Quedas, por personagem | Pillar 4: Gus ameaçado, nunca massacrado | Teto de quedas do Gus por tier FIXADO PELO LÍDER antes de rodar (seção 4.1); Cauã/Jaci caírem às vezes é BÔNUS (hoje o risco mora todo no Gus, colateral 7 do estudo-pai) |
| **E8** | Rodada da 1ª queda | Paulada injusta vs tensão construída | Nenhum candidato com mediana de 1ª queda nas rodadas 1-2 no trash |
| **E9 (nova)** | **Trivialidade**: % de lutas em que a party termina SEM PERDER NENHUM HP, e HP final médio | O erro óbvio que E2 sozinha não vê: luta curta E oca | Trash: % sem-dano abaixo de teto a fixar (proposto: ≤ 40%); HP final médio na faixa 55-90% (perde algo, não muito). Elite: HP final médio ≤ 70% (elite tem que doer) |
| **E10 (nova)** | Dano médio sofrido pela party por encontro (em HP e em % do pool) | O número que o `economy-designer` consome (custo de cura por encontro, doutrina do comedido) | Informativa aqui; régua é dele |
| **E3** | Concentração de dano | Sentinela de regressão: o retune não pode ressuscitar o saco de pancadas | Não piorar mais que o piso (5pp) vs o mesmo cenário no estudo-pai |
| **E6** | Absorção do Shield | Prova ou refuta H3b (Defender binário não sai por número) | Informativa; documenta a pendência de design do Shield |
| **E7** | Cap de 30 rodadas batido (P6) | Regressão: o turtle-empate tem que continuar morto | ~0% de cap; e a derrota do turtle não pode virar massacre em ≤3 rodadas (lição tem tempo de chegar) |

### 4.1 Números que o LÍDER fixa ANTES de rodar (pré-registro, como o MCID da vez passada)

O estudo-pai provou o valor de fixar a régua antes dos dados (o piso de 5pp decidiu
metade das leituras sem margem para pesca de resultado). Aqui há DUAS réguas novas que
são decisão de produto pura, e proponho faixas para ele escolher (ou mandar outras):

1. **Taxa de vitória alvo por tier** (E1). Proposta para escolha: trash entre 90 e 97%
   (o "vence mais e fica feliz" da decisão dele, sem chegar aos 100% ocos do L4);
   elite entre 55 e 80% (dói, mas não é muro). Qualquer faixa serve ao estudo; o que
   não serve é escolher DEPOIS de ver os dados.
2. **Teto de quedas do Gus por tier** (E4). Proposta para escolha: trash ≤ 8 a 12%
   (referência: a moeda pura dava 10,3% e ninguém reclamou de massacre; os 30,5% do
   braço C cru reprovaram na análise); elite ≤ 30 a 40% (elite PODE derrubar o Gus às
   vezes: é para isso que a Análise Preditiva §2.1 existe).

---

## 5. Piso de relevância (MCID)

O piso do líder no estudo-pai: 5 pontos percentuais para métricas em %, 0,5 rodada
para métricas em rodadas. **Proposta: manter os dois, com um acréscimo e uma
mudança de uso.**

- **Mantém, porque:** a natureza das métricas é a mesma, e mudar régua entre estudos
  irmãos convida a escolher a régua que favorece o resultado.
- **Mudança de uso:** no estudo-pai o MCID comparava braços entre si. Aqui a métrica
  primária é um ALVO (a janela), não uma comparação; o MCID passa a servir para o
  desempate entre CANDIDATOS na Fase C: dois candidatos com todas as métricas dentro
  de 5pp/0,5 rodada um do outro são EMPATE, e o desempate pré-declarado é (nesta
  ordem): o mais próximo dos valores canônicos atuais (menor mudança), o de menor Atk
  inimigo (menor risco ao Gus), o de número mais redondo (legibilidade de design).
- **Acréscimo:** para E9 (trivialidade) e E10 (dano sofrido), o mesmo 5pp; para o
  p10-p90 de E2, o análogo de 0,5 rodada em cada percentil.

---

## 6. Valores inventados (a maior fonte de erro, toda ela visível)

| # | O que falta | Valor `//SIM` proposto | Justificativa |
|---|---|---|---|
| V1 | **Mira fixa por tier** | **FECHADO 2026-08-01, não é mais placeholder.** Fonte: tabela tier-mira do `economy-designer` (`proposta-economia-comedimento.md`, tabela de tiers), última pergunta decidida pelo líder no mesmo dia. Trash comum = `F_NoF4` (ponderada por F1/F2/F3, sem F4); Elite = `C_F4Soft` | O líder **rejeitou explicitamente** o sorteio uniforme (braço B): "espalha mas é roleta sem porquê, o jogador não consegue formar teoria sobre o inimigo, fere o Pillar 1". A decisão sobre "minion burro" foi a leitura F: reage a quem bate e a quem está ferido, mas ignora quem se defende - burro no sentido de não fazer a jogada esperta, não no sentido de ser aleatório |
| V2 | Faixas da grade (X1-X4) | seção 2.1 | Cercam a referência por baixo (a suspeita é HP inflado); passos de ~20% no screening, ~10% no refino |
| V3 | SPD inimigos | Sentinela 8, Daemon 10 (herdado do estudo-pai) | Fora do escopo (ordem de turno é outra conversa); mesmo valor para comparabilidade entre os dois estudos |
| V4 | Atk do Daemon-Guard | eixo X2 (14/18/22), centro no 18 do estudo-pai | TBD no canon; o estudo existe para propor a faixa |
| V5 | Cura da Jaci | eixo X4 (6/9/12) | O 12 do estudo-pai trivializou o trash (L4); o estudo mede onde a cura deixa de ser botão de imortalidade sem virar inútil |
| V6 | Bot da party | `AutoResolveBrain` + variantes por cenário (defensor, healer), idênticos ao estudo-pai | Comparabilidade; viés declarado (bot pior que humano, erro na direção segura) |
| V7 | Cap de rodadas | 30, "empate técnico" | Herdado; trava de segurança |
| V8 | KnowledgeKills | 0 (pior caso) | Herdado; se funciona no encontro mais ruidoso, funciona nos calmos |
| V9 | N por fase | 24k / 60k / 240k (seção 3) | Margens de erro 10x mais finas que o piso de relevância em cada fase |
| V10 | Heróis a 70% HP no P4 | herdado do L6 do estudo-pai | Cenário de pressão; comparabilidade |
| V11 | Faixas de E1 e teto de E4 | seção 4.1, **a fixar pelo líder antes de rodar** | Pré-registro; decisão de produto pura |

**Escada completa de tiers (V1, para quem ler depois entender de onde vieram os dois
braços deste estudo):** Trash comum = F (`F_NoF4`); Trash avançado = C (`C_F4Soft`);
Elite = C com `UtilityBrain`, sem F8; Mini-boss = D (`D_F4Strong`) com os contrapesos
obrigatórios; Boss = D + F8; Boss final = D + F7 + F8, com o Patch-Zero fora da escada
por manter `is_chaotic`. Este estudo só exercita trash comum e elite (protocolo seção
2.2); os demais degraus ficam registrados aqui para rastreabilidade da tabela do
`economy-designer`, não para uso neste harness.

---

## 7. O que a simulação NÃO responde (e aqui é mais grave que no estudo da mira)

Sendo duro, como pedido, porque neste estudo a tentação de acreditar no número é maior:

1. **Ritmo é sensação, e a simulação mede contagem.** Três rodadas travadas em
   animação lenta são mais longas, no relógio do corpo, que sete rodadas fluidas. O
   headless conta rodadas; quem transforma rodada em SENSAÇÃO de ritmo é o
   `PacingDirector`, a velocidade dos floaters, o tempo de tela entre ação e resultado
   (battle-screen.md §5.2). **Um candidato aprovado aqui pode parecer arrastado na
   tela, e um reprovado poderia parecer ótimo.** A conversão canônica (~25-30s por
   rodada, §15.1) é estimativa de modelo, não medição com jogador.
2. **A janela 3-5 pode estar errada.** O estudo TOMA a janela como lei (é canon,
   one-way door de 2026-07-19) e busca números que a cumpram. Ele não testa se 3 a 5 é
   de fato o que diverte o nosso público; se o playtest com os números novos parecer
   apressado, a resposta certa pode ser reabrir a janela com o líder, não forçar mais
   os statlines. Registro o risco para ninguém tratar o cumprimento da métrica como
   prova de diversão.
3. **"Vencer mais e ficar feliz" tem metade invisível.** A felicidade do jogador ao
   vencer de um minion burro depende de PERCEBER que venceu por mérito (leitura,
   Pillar 1), não de taxa de vitória. Taxa alta com sensação de esteira rolante é
   tédio. Isso é apresentação, telegraph e variedade de encontro: invisível ao
   headless.
4. **O custo emocional da queda do Gus não é linear.** 8% de quedas medidos podem ser
   sentidos como "quase nunca" ou como "toda sessão longa, uma vez", dependendo de
   ONDE caem (início de dungeon com autosave perto, ou fim de corredor sem save). A
   simulação não vê o mapa nem o save; o teto de E4 é proxy, não veredito.
5. **A doutrina do comedido/esbanjador atravessa ENCONTROS, e este estudo vê um por
   vez.** O acúmulo de dano sofrido, o custo de Hospital, a decisão de gastar carta
   agora ou poupar: isso é a simulação de campanha do `economy-designer`, alimentada
   pela nossa E10. Prometer que este estudo "valida a doutrina" seria mentira de
   escopo.
6. **Bot não é criança.** O `AutoResolveBrain` não entra em pânico, não repete a carta
   favorita por amor, não turtleia por medo. Os padrões degenerados humanos que o
   playtest do líder revelou (defesa total) entram como cenário FIXO (P6), mas o
   espaço de comportamentos reais de uma criança de 11 anos é maior que os nossos 6
   cenários.
7. **Variância percebida.** O canal FALHA/CRIT (25/50/25 com kills = 0) faz lutas
   irmãs divergirem. O p10-p90 mede a dispersão aritmética, não a sensação de "esse
   jogo é injusto" que uma sequência de FALHAs causa. Só olho no rosto do jogador
   responde.

O estudo decide **os números do ritmo**. A **sensação** do ritmo se valida no playtest
com o `PacingDirector` ligado, e o protocolo de playtest é etapa posterior, com o
líder e o Gus Dragon.

---

## 8. Protocolo de execução (as cláusulas, na ordem do método aprovado)

1. **Aprovação prévia do líder:** eixos, faixas, cenários, N por fase, e OS DOIS
   PRÉ-REGISTROS da seção 4.1 (taxa de vitória alvo e teto de quedas do Gus), item a
   item se ele quiser. Nada roda antes.
2. **Insumo V1:** a tabela tier-mira do `economy-designer` entra assim que publicada;
   se o estudo rodar antes dela, o placeholder da V1 precisa de aprovação explícita do
   líder (e a Fase C re-roda os candidatos sob a tabela final quando ela sair, antes
   de qualquer canonização). **Atualização 2026-08-01: a tabela saiu antes do estudo
   rodar, com a última pergunta decidida pelo líder no mesmo dia (ver V1, seção 6). Não
   há mais placeholder nem re-rodagem pendente por esta cláusula.**
3. **Harness:** extensão do `mira_sim_harness.hpp` existente (que já valida a janela
   3-5 e as métricas E1-E8), por agente de engenharia (backend-engineer /
   gameplay_engineer), não inline. Novidades: statline parametrizável por ponto da
   grade, métricas E9/E10, AP 2 no elite (exige destravar o gate de 1 ação/turno do
   ScriptedBrain no harness, nota para o engenheiro: é variação de teste, NÃO mudança
   do motor de produção).
4. **QA independente tenta quebrar o harness antes do estudo rodar** (mesma regra da
   casa: implementer e verificador são agentes diferentes; mutation testing nos pontos
   novos, sobretudo E9/E10 e o loop de AP).
5. **Seeds determinísticas:** seed base fixa + índice; as MESMAS seeds de um cenário
   rodam em todos os pontos da grade daquele cenário (comparação pareada); qualquer
   luta re-rodável golpe a golpe.
6. **Progresso no terminal:** `fase [A|B|C], ponto [k] de [K], simulação [n] de [N]`,
   impresso a cada 1% do ponto (mesma solução do estudo-pai).
7. **Entrega em duas camadas:** primeiro os dados (tabela por ponto × cenário, com
   margens de erro e MCID aplicado aos candidatos), depois a análise de C-level com
   opções em linguagem simples, no mesmo formato de `analise-mira-resultados.md`:
   como cada candidato faz o jogo se SENTIR, o que ganha, o que perde, que pillar
   toca. A decisão final de cada número é do líder.
8. **Fora de escopo blindado:** nenhum número deste estudo entra em `combat.md` sem
   decisão explícita do líder; o doc de resultado nasce marcado ANÁLISE, como o
   anterior.

---

*Proposta escrita por Capitolino (CPO) em 2026-08-01. Não é canon. Não commitada por
decisão de processo (commit e push só com ordem do líder).*

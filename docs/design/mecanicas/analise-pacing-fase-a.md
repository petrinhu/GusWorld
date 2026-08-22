# ANÁLISE: Fase A do estudo de pacing (PACING-SIM, grade grossa)

> **Status: ANÁLISE, não canon.** Nenhum número deste documento entra em `combat.md`
> nem no motor sem decisão explícita do líder. Autor: Capitolino (CPO), 2026-08-01.
> Dados: `/var/tmp/pacing_fase_a.txt` (116 pontos, 240.000 lutas por ponto, 27,84
> milhões de lutas no total, zero erros internos). Protocolo e pré-registro do líder:
> `proposta-protocolo-simulacao-pacing.md`. Não commitado por decisão de processo.

---

## Resumo executivo (em linguagem simples)

1. **A hipótese central do estudo se confirmou.** A duração da luta comum é controlada
   por um único botão: a quantidade de vida do inimigo. Baixando a vida, a luta
   encurta de forma previsível e em escada: 40% da vida atual dá 2 rodadas, 60% dá 4,
   80% dá 5, 100% dá as 7 rodadas que o estudo da mira já tinha medido. E mexer nesse
   botão não deixa o Gus mais frágil; pelo contrário, luta mais curta derruba o Gus
   MENOS. Duração e perigo são mesmo botões separados.
2. **Existe um vencedor claro no trash**, e ele conta uma história: vida do inimigo em
   33 (o valor antes da inflação de junho era 34) com ataque 12. Vitória em 92,8% das
   lutas, 96,9% delas dentro da janela de 3 a 5 rodadas, Gus caindo em 7,2% (dentro do
   teto de 12% que o líder fixou). O mesmo ponto também passa no cenário com defensor.
   **A inflação de vida de junho é, comprovadamente, a causa direta da janela
   quebrada.** Mas desfazer a inflação sozinha não basta: com o ataque atual (10,
   provisório no canon), a luta fica segura demais e reprova nas próprias réguas do
   líder. O pacote vencedor é vida de volta ao patamar antigo E ataque 2 pontos acima
   do provisório.
3. **O terceiro "aprovado" (cenário com curandeira) é um falso aprovado.** Ele passou
   nas travas formais mas fica fora da janela (6,7 rodadas, só 4,4% das lutas entre 3
   e 5). A trava de duração media a cauda da distribuição, não o alvo em si; é um furo
   do protocolo, admitido nesta análise, com proposta de conserto pré-registrado antes
   da Fase B.
4. **O braço elite não é conclusivo e não deve virar decisão de números.** O robô que
   joga pela party só dá ataque básico, e contra a defesa 14 do elite todo ataque
   básico causa exatamente 1 de dano (o piso da fórmula). A prova está nos próprios
   dados: nas vitórias, o número de golpes até o elite cair é exatamente igual ao HP
   dele (90, 115 e 144 golpes para 90, 115 e 144 de vida). O estudo mediu a party
   cutucando o elite com palito. Jogador real usa cartas. Antes de propor números
   para o elite, o harness precisa de um robô que jogue cartas.
5. **Recomendação de método para a Fase B: bissecção direcional** ao redor do
   vencedor, porque a relação vida-duração provou ser monotônica e quase
   determinística (o pressuposto da bissecção deixou de ser suposição e virou dado).
   Mais um braço extra para a curandeira (a cura 6 ainda torna a luta imperdível
   perto do vencedor; é preciso testar cura menor). Elite fica fora da Fase B até a
   medição ser consertada. Nenhum corte automático: a tabela final vai inteira ao
   líder.

---

## 1. O que rodou, e a checagem de validade interna

- 116 pontos (80 trash P1/P2/P3, 36 elite P4/P5), amostra cheia de 240.000 lutas por
  ponto como o líder mandou, seed base 20260801, zero erros internos em 27,84 milhões
  de lutas.
- **Aprovados pelos guarda-corpos pré-registrados: 3 de 116.**
  - Ponto 31, trash P1 vanilla, HP×0,60 (33 HP), Atk 12: vitória 92,81%, duração média
    3,89, janela 3-5 em 96,92%, quedas do Gus 7,19%, 1ª queda mediana na rodada 3,
    HP final médio 75,3%, dano sofrido 30,8 HP por encontro (E10).
  - Ponto 32, trash P2 parede, mesmos eixos: vitória 92,56%, janela 96,85%, quedas do
    Gus 7,44%, HP final 77,5%.
  - Ponto 78, trash P3 healer, HP×1,00, Atk 14, cura 6: vitória 92,63%, quedas 7,37%,
    mas duração média 6,68 e janela em só 4,39% (ver seção 6: falso aprovado).
- **Checagem de validade interna que os dados entregaram de graça:** nos 18 pontos do
  P4 (elite com escolta), os resultados são idênticos número a número entre os três
  níveis de HP do elite (ex.: pontos 81, 93 e 105 têm as mesmas médias até a sexta
  casa). É o comportamento esperado das seeds pareadas quando a vitória é 0%: o elite
  nunca morre, o HP dele nunca é consultado, a trajetória da luta é a mesma. As seeds
  pareadas estão funcionando como desenhadas, e o P4 não contém informação nenhuma
  sobre HP de elite.

## 2. H1: a alavanca de HP se confirmou, e isso decide o método da Fase B

**H1 dizia:** a duração do trash é dominada pelo TTK-out, a alavanca é o HP do
inimigo, e o valor que entra na janela está entre 60% e 80% da referência.

**Os dados confirmam as três partes.** Duração no P1 com Atk 8 (o inimigo quase não
machuca, isolando a variável):

| HP×mult | HP | Rodadas (média) | p10-p90 | Na janela 3-5 |
|---|---|---|---|---|
| 0,40 | 22 | 2,00 | 2 a 2 | 0% |
| 0,60 | 33 | 4,00 | 4 a 4 | 100% |
| 0,80 | 44 | 5,00 | 5 a 5 | 100% |
| 1,00 | 55 | 7,00 | 7 a 7 | 0,0008% |

Três propriedades, todas relevantes para o método:

1. **Monotônica, sem exceção:** mais HP, mais rodadas, nos 4 de 4 pontos medidos, e
   o mesmo padrão se repete em toda coluna de Atk.
2. **Quase determinística:** p10 = p90 em toda célula pura. A luta do mesmo ponto
   dura praticamente sempre o mesmo número de rodadas (o dano básico subtrativo não
   tem variância; o ruído da mira e do canal de carta quase não move a contagem).
3. **Em degraus (escada), não contínua:** a resposta pula de 2 para 4, de 4 para 5,
   de 5 para 7. Nenhum HP da grade grossa entrega 3 ou 6. É consequência do dano
   determinístico: a duração é um teto-inteiro de HP dividido por dano por rodada.
4. **O Atk quase não mexe na duração** (4,00 / 4,00 / 3,89 / 3,57 no HP×0,60 conforme
   o Atk sobe de 8 a 14), e o pouco que mexe é artefato de truncamento: luta perdida
   acaba mais cedo. A quase-ortogonalidade da seção 1.1 do protocolo é real.

**Consequência direta, como o protocolo prometeu:** a bissecção direcional da Fase B
deixa de ser suposição e vira conclusão apoiada. Recomendo **bissecção direcional**,
não o mini-grid simétrico, por três razões medidas: (a) a monotonia vale em todo o
espaço observado, então dividir o intervalo converge e não perde nada; (b) a resposta
é em degraus, e a bissecção é exatamente o método barato de achar a BORDA de um
degrau (o mini-grid simétrico gastaria células remedindo platôs que já sabemos
planos); (c) a variância por ponto é quase nula, então N de 60.000 por célula
resolve qualquer comparação com folga sobre o piso de 5pp / 0,5 rodada.

Duas emendas ao que o protocolo escreveu sobre a Fase B, ambas derivadas dos dados:

- **A busca não é 1D.** O HP fixa a duração, mas quem fixa a taxa de vitória (e as
  quedas) é o Atk, e a faixa E1 de 90-97% é estreita: Atk 10 dá 99,88% (reprova por
  cima), Atk 12 dá 92,81% (aprova), Atk 14 dá 77,93% (reprova por baixo). A bissecção
  precisa de um passo em Atk (testar 11 e 13) além do passo em HP.
- **A borda dos degraus importa como produto, não só como método:** saber em que HP a
  luta pula de 4 para 5 rodadas diz quanta folga o design tem para variantes de
  inimigo antes de sair da janela. A Fase B deve reportar as bordas, não só o ponto
  ótimo.

## 3. Confronto com as hipóteses e as condições de mudança de ideia

Mesma disciplina do estudo da mira: o que eu escrevi antes dos dados, contra o que
os dados disseram.

### H1 (alavanca = HP, janela entre 60% e 80%): CONFIRMADA

Ver seção 2. O palpite de faixa acertou: 0,60 e 0,80 são os dois multiplicadores
dentro da janela (4 e 5 rodadas), e o aprovado real está no 0,60.

### H2 (desacoplamento; quedas caem junto com a duração): CONFIRMADA

Aposta central: encurtar a luta não tornaria o Gus mais massacrável, porque menos
rodadas = menos golpes sofridos. Medido no P1 com Atk 12 fixo:

| HP×mult | Rodadas | Quedas do Gus |
|---|---|---|
| 0,40 | 2,0 | 0% |
| 0,60 | 3,9 | 7,19% |
| 0,80 | 4,6 | 19,88% |
| 1,00 | 5,3 | 50,95% |

Quedas CAEM com o HP menor, exatamente na direção apostada. A exposição do Gus é
duração × pressão: baixar HP inimigo compra segurança de graça enquanto encurta a
luta. A condição de mudança de ideia nº 1 ("se baixar HP arrastar as quedas acima do
teto") **não disparou**.

### A armadilha prevista ("HP baixo + Atk alto = curta E mortal"): apareceu, mas em OUTRO lugar

Eu previ o pior mundo no canto HP mínimo + Atk máximo. Os dados dizem que esse canto
é inofensivo: no HP×0,40 com Atk 14 a luta acaba em 2 rodadas com 0% de quedas, o
inimigo morre antes de acumular golpes suficientes para derrubar o Gus. **A
interação HP×Atk é real, mas a geometria é outra: o perigo mora no meio da grade,
onde a luta é longa o bastante para o Atk alto se acumular.** O pior mundo medido é
HP×0,60-0,80 com Atk 14: pontos 36 (3,57 rodadas, 86% na janela, e 22,07% de quedas)
e 56 (4,00 rodadas, 78% na janela, e 39,72% de quedas). Lutas DENTRO da janela e
mortais além do teto. O desenho fatorial pegou isso; a varredura um-eixo-por-vez que
o protocolo recusou teria escondido. Registro o acerto do método e o erro da minha
localização prevista.

### H3 (as pontas se consertam por número): METADE REPROVADA, e é a metade que importa para a Fase B

- **Curandeira:** eu apostei que a cura deixaria de trivializar quando o dano líquido
  inimigo superasse a cura por rodada. O único P3 que entrou na faixa E1 foi o ponto
  78 (HP×1,00, Atk 14, cura 6), que está fora da janela de duração. **Na vizinhança
  do vencedor (HP×0,60, Atk 12), a cura 6 ainda torna a luta imperdível: 100% de
  vitória, zero quedas, reprovado.** O mesmo vale para cura 9 e 12, e até com Atk 14
  no HP×0,60 a vitória fica em 99,1-100%. Ou seja: dentro da grade testada, NENHUM
  trio (HP, Atk, cura) aprova nos três cenários de trash ao mesmo tempo. A cura 6 por
  uso, contra o dano de encontro do ponto vencedor (E10 em torno de 28-31 HP por luta
  inteira), apaga o custo da luta quase por completo. A Fase B precisa estender o
  eixo da cura para BAIXO de 6 (proposta: 2, 3, 4, 5). Se nem cura 2-3 sair da
  trivialização sem virar inútil, a resposta honesta vira regra (cadência ou custo de
  cura), não número, e eu direi isso sem maquiar.
- **Elite:** ver seção 5. O que eu apostei (elite vira ganhável se o TTK-out couber
  no envelope) nem chegou a ser testado de verdade, porque o TTK-out do bot contra a
  Def 14 é fixo no pior valor possível (1 de dano por golpe). Não declaro H3-elite
  reprovada nem confirmada: declaro a medição inválida para essa pergunta.

### H3b (Defender binário não sai por número): CONFIRMADA

E6 (absorção do Shield) = 100% em todas as células com defensor (P2 inteiro, ex.:
ponto 32), em toda a faixa de Atk 8-14. Nenhum statline da grade fez o dano do trash
superar o pool do Shield. Como declarado no protocolo, isso é pendência de REGRA do
Shield (one-way door separado do líder), não deste estudo. Está medido e provado;
não persegui conserto.

### Condição de mudança de ideia nº 2 ("nenhum ponto entra com guarda-corpos verdes")

Não disparou no trash (dois aprovados legítimos). **Disparou formalmente no braço
elite** (zero aprovados em 36 pontos), mas me recuso a puxar o gatilho de "redesign
necessário" com uma medição que a seção 5 demonstra ser artefato de bot. A conclusão
honesta ali é "medição insuficiente", não "fórmula quebrada". A ÚNICA afirmação
estrutural que os dados do elite sustentam é a do clamp (seção 5), e essa é sobre o
ataque básico, não sobre o espaço de números do elite.

### Condição de mudança de ideia nº 3 ("se a variância dominar a duração")

Não disparou, e o contrário dela é que apareceu: a variância é quase nula (p10 = p90
em toda célula pura do trash). O risco que eu temia (média boa com metade das lutas
fora da janela) não existe; o risco novo, que registro para o playtest, é o oposto:
**toda luta do mesmo encontro dura quase exatamente o mesmo número de rodadas.** O
headless não mede monotonia percebida; se isso vai parecer repetitivo na tela é
pergunta do playtest com o PacingDirector ligado (limite nº 1 e nº 7 da seção 7 do
protocolo), não deste harness.

### Observação estrutural que nenhuma hipótese previu: E1 e E4 são a MESMA régua no trash

Nas 80 células de trash, a taxa de derrota é numericamente idêntica à taxa de queda
do Gus, célula por célula (ex.: ponto 31, derrota 7,18667% e queda do Gus 7,18667%;
ponto 76, derrota 68,55% e queda do Gus 68,55%, com Cauã caindo 1,36% em lutas que
ainda assim foram vencidas). O motor encerra a luta como derrota quando o Gus cai.
Consequência para as réguas do pré-registro: no trash, vitória = 100% menos quedas
do Gus, então a faixa E1 de 90-97% JÁ IMPLICA quedas do Gus entre 3% e 10%, e o teto
E4 de 12% nunca é o gate que morde (a faixa E1 é sempre mais apertada). Não proponho
mudar nada agora (mudar régua no meio do estudo é pesca); registro para o líder
saber que, no trash, os dois números que ele fixou são um dial só, e para a Fase C
reavaliar se E4 por personagem merece régua própria quando Cauã/Jaci começarem a
cair (hoje quase não caem: bônus do colateral 7 do estudo-pai continua não
resgatado).

## 4. O achado histórico: HP 33 versus os 34 de antes da inflação

**A leitura se sustenta no essencial, com duas ressalvas obrigatórias.**

O que se sustenta: o ponto vencedor usa HP×0,60 = 33, e o HP do trash antes da
inflação de 2026-06-03 era 34 (`combat.md` §17: Trash 34→55). Com 55 HP a luta dura
7,00 rodadas (medido, ponto 61); com 33 HP dura 4,00 (medido, ponto 21). **A
inflação de +60% é a causa direta, mensurável e agora comprovada da janela
quebrada, e desfazê-la devolve a duração ao centro exato da janela.** Isso não é
narrativa: é a mesma escada da seção 2, lida no ponto onde o histórico do projeto
está sentado.

**Ressalva 1: desfazer a inflação SOZINHA não aprova nada.** Com HP 33 e o Atk
canônico atual (10, marcado PROVISÓRIO em `combat.md` §"Stats de referência", nota
de 2026-06-25), o resultado é o ponto 26: vitória 99,88%, quedas 0,12%, luta segura
demais, reprovada pela própria faixa E1 do líder (e o E9 fica no limite). O pacote
vencedor é HP de volta ao patamar pré-inflação E Atk 12, dois pontos acima do
provisório. Em outras palavras: junho errou o HP para cima, mas o Atk provisório de
hoje também está baixo demais para a luta custar algo. O conserto honesto tem dois
movimentos, não um.

**Ressalva 2: reverter a inflação NÃO recupera a intenção original de junho, e é
importante ninguém vender que recupera.** A intenção de junho era "TTK alvo 3-5
TURNOS" de ator (golpes). A métrica E11 mede exatamente isso, e ela diz: no ponto
vencedor, o primeiro inimigo cai com mediana de 11 golpes (7 no HP×0,40; 14 no
HP×0,80; 20 no HP×1,00). **Nenhum ponto da grade inteira chega perto de 3-5 golpes,
nem o piso de 22 HP.** Se o líder um dia fixar a régua E11 em 3-5 golpes, ela é
inalcançável por HP dentro de faixa razoável: exigiria mexer em Atk da party
(one-way door canônico) ou na Def do trash. Os dois alvos "3 a 5" (rodadas de julho,
golpes de junho) seguem sendo perguntas diferentes, como o protocolo já tinha
separado; a Fase A cumpre o primeiro e prova que o segundo nunca foi verdade, nem
antes da inflação (34 HP dariam os mesmos ~11 golpes, porque o dano da party não
mudou). Registro os dois separadamente, como o pré-registro mandava.

Nota de desempate para a Fase B: o pré-registro de desempate da Fase C prefere "o
mais próximo dos valores canônicos" e "o número mais redondo". Entre 33 (0,60×55) e
o 34 histórico não deve haver diferença mensurável de duração (mesmo degrau da
escada), e o 34 tem a dupla vantagem de ser o valor pré-inflação exato e um número
da casa (Fibonacci, como o HP 34 do Gus). A Fase B deve incluir a célula HP=34 exata
para essa comparação sair medida em vez de presumida.

## 5. O elite: o resultado é ARTEFATO de medição, e aqui está a prova

Resultado bruto: zero aprovados em 36 pontos; melhor caso 22,5% de vitória (P5 solo,
HP×0,625, Atk 14, AP 1); quedas do Gus de 77,5% a 100% (acima de 99% em 15 dos 18
pontos do P5); P4 (escolta, heróis a 70%) com 0% de vitória em todas as 18 células.

**Antes de qualquer número novo, a pergunta do team-lead: válido ou artefato?
Artefato, e desta vez dá para demonstrar com aritmética, não com suspeita:**

1. **O clamp está cravado no piso contra o elite.** Daemon-Guard tem Def 14
   (`combat.md`, tabela de stats de referência). Ataque básico é `clamp(Atk - Def,
   1)`: Cauã (Atk 14) causa 1, Jaci (Atk 9) causa 1, Gus (Atk 8) causa 1. **Todo
   golpe básico da party contra o elite causa exatamente 1 de dano.**
2. **Os dados confirmam a conta de forma exata.** Nas vitórias do P5, o E11 (golpes
   até o inimigo cair) dá média = mediana = p10 = p90 = 90 golpes com HP 90, 115
   golpes com HP 115 e 144 golpes com HP 144 (pontos 82, 94, 106; a nota do harness
   avisa que E11 fora do P1 é informativo, e é exatamente como informativo que ele
   entrega a prova). Não existe dispersão nenhuma: cada vitória custou exatamente
   HP golpes de 1. A party venceu, quando venceu, cutucando o elite 90 a 144 vezes.
3. **A duração do elite não responde ao HP dele** (7,50 / 7,92 / 8,05 rodadas
   conforme o HP sobe, com a fração na janela IDÊNTICA, 27,2333%, nos três níveis):
   porque a duração é dominada pelas derrotas, e a derrota é cronometrada pelo tempo
   de sobrevivência da party, que não depende do HP do elite. A alavanca H1 (HP)
   simplesmente não existe no braço elite tal como medido.

O `AutoResolveBrain` dá ataque básico e no máximo 1 carta single-target. Contra
trash (Def 8) isso é um viés aceitável e declarado ("bot pior que humano, erro na
direção segura"). Contra Def 14, o viés deixa de ser margem e vira o resultado
inteiro: **o braço elite mediu "dá para vencer um elite só com palitos?", e a
resposta (não) era conhecida antes de rodar 8,6 milhões de lutas.** O estudo da mira
já tinha visto ~100% de derrota nos cenários L6/L7; a Fase A não adicionou
informação nova sobre o DESIGN do elite, adicionou a demonstração de POR QUE a
medição não informa.

**O que seria preciso para o braço elite ser conclusivo:**

1. **Bot capaz de jogar carta de verdade** (deck mínimo canônico + política simples
   e pré-registrada de uso, ex.: carta de dano quando AP permite, foco no elite),
   implementado no harness pelo engenheiro e quebrado pelo QA antes de rodar, como
   manda a cláusula 3/4 do protocolo. As cartas atravessam a cadeia divisiva, que é
   a via de dano que a Def 14 não achata para 1; é ela que o jogador real usa.
2. **Uma decisão de design do líder ANTES da re-medição**, porque ela muda o que
   "conclusivo" significa: o elite ser quase imune a ataque básico e cair por carta
   É intencional? A favor: casa com o Pillar 1 (magia = software; a resposta certa
   para hardware blindado é o script certo, não força bruta) e dá ao elite
   identidade mecânica de puzzle leve. Contra: se o jogador ficar sem carta (bateria,
   custo), a luta vira os 90 palitos que o bot mediu, e o envelope de 5-8 rodadas
   fica impossível. Eu recomendo assumir "cai por carta" como hipótese de design e
   medir com o bot novo; mas é decisão dele, não minha.
3. **Só depois disso, números.** Re-rodar o screening elite (X1×X2×X3) com o bot
   novo. O AP 2 do §13.1 deve ser reavaliado nessa re-medição: nos dados atuais ele
   é devastador (com AP 2, derrota ~100% com mediana de 1 a 3 rodadas mesmo no
   elite mais fraco), mas esse número carrega o mesmo artefato (a party não mata, então
   qualquer pressão parece infinita). Custo real do AP 2 só existe medido contra
   um bot que reage com cartas.

Enquanto isso, o braço elite da Fase A fica registrado como o que é: prova do clamp,
prova de que o bot atual não exercita a via de dano relevante, e nada além disso.

## 6. O ponto 78 e o guarda-corpo de cauda: furo admitido, com conserto proposto

A observação do team-lead procede integralmente, e a resposta honesta é: **é um furo
do protocolo, não uma intenção minha.** O que eu escrevi na régua E2 foi "Primária:
maximizar % na janela; p10 >= 2 e p90 <= 7". O harness implementou exatamente isso:
o único GATE de duração é a cauda (p10/p90), porque "maximizar" não é um limiar. O
ponto 78 (mediana 7, p90 = 7, janela 4,39%) passa na cauda por definição e ganha o
carimbo [APROVADO], que na Fase A significa apenas "todos os gates pré-registrados
verdes", não "dentro do alvo canônico". Eu desenhei a cauda supondo que a
maximização puxaria a mediana para dentro da janela; não previ que um ponto pudesse
passar em TODOS os gates com a mediana inteira fora do alvo. O p90 <= 7 admite
mediana 7, e isso torna o gate de cauda insuficiente como critério solitário.

**Conserto proposto, e o timing é a parte importante:** pré-registrar ANTES da Fase
B rodar, por decisão do líder, um piso da métrica primária como gate. Proposta:
mediana de E2 dentro de 3 a 5 E fração na janela >= 60% (o valor exato é dele; o que
não pode é escolher depois de ver os números da B). Declaro o efeito com todas as
letras: **esse gate novo desqualifica o ponto 78 retroativamente como candidato.**
Não é pesca de resultado, pelas três razões que me permitem propor isso de cabeça
erguida: o furo foi apontado de fora (team-lead), está sendo consertado antes da
fase seguinte e não protege nenhum candidato meu (pelo contrário, reduz a lista de
3 para 2). O ponto 78 continua nos dados como informação útil sobre a curandeira
(é o único lugar da grade onde a cura 6 não trivializa, porque o Atk 14 contra 55
HP de esponja machuca o bastante), mas informação não é candidatura.

## 7. Recomendação para a Fase B

**Método: bissecção direcional na vizinhança do vencedor** (sustentação na seção 2:
monotonia confirmada em toda a grade, resposta quase determinística, degraus que a
bissecção localiza barato). Sem corte automático de candidatos: toda célula medida
vai à tabela final do líder, com o MCID de 5pp / 0,5 rodada aplicado só como leitura
de empate, como no estudo da mira.

### Braço 1: trash, refino do vencedor (P1 + P2)

- **HP:** 30, 33, **34** (célula histórica/Fibonacci, ver seção 4), 36, 39 (multiplicadores
  ~0,55 a 0,71), bissecando as bordas dos degraus 3→4 e 4→5.
- **Atk:** 11, 12, 13 (a faixa E1 de 90-97% é estreita: 10 reprova por cima, 14 por
  baixo; falta sondar os vizinhos imediatos do 12).
- N = 60.000 por célula (protocolo, seção 3). ~30 células, segundos de CPU.
- Reportar: célula ótima E bordas dos degraus (tolerância de design para variantes).

### Braço 2: curandeira (P3), extensão declarada do eixo X4

- **Cura: 2, 3, 4, 5** (o 6 fica como âncora de comparação), sobre HP {33, 34, 36} ×
  Atk {12, 13}.
- Pergunta pré-declarada: onde a cura deixa de ser botão de imortalidade sem virar
  inútil, NA vizinhança do candidato real (o estudo-pai respondeu isso na vizinhança
  errada). Se nenhuma célula aprovar, a resposta é regra de cura (cadência/custo),
  não número, e o relatório dirá isso.

### Braço 3: elite. NÃO refinar números agora

Pré-requisitos antes de qualquer grade nova (seção 5): decisão de design do líder
sobre "elite cai por carta", bot com carta no harness (engenheiro + QA adversarial),
e só então re-screening. Isso é trabalho de harness, não de Fase B; proponho tratar
como estudo-irmão curto (PACING-SIM elite-bis) para não segurar o trash, que já tem
candidato maduro.

### Pré-registros novos que o líder precisa fixar ANTES da B rodar

1. O piso da primária E2 como gate (seção 6). Proposta: mediana em 3-5 E janela >= 60%.
2. A decisão de design do elite (seção 5, item 2), que pode rodar em paralelo à B do
   trash sem bloquear nada.

### O que a Fase B NÃO vai fazer (escopo blindado, reafirmado)

Def inimiga continua fora (o trade-off declarado na seção 2.1 do protocolo dizia
"entra numa extensão se HP sozinho não bastar"; HP bastou). Party fixa. Fórmula
intocada. Shield é pendência de regra com dono (líder), medida e documentada (E6
100%), não perseguida. Nenhum número entra em `combat.md` sem decisão explícita do
líder.

---

*Análise escrita por Capitolino (CPO) em 2026-08-01 sobre os dados da Fase A. Não é
canon. Não commitada por decisão de processo (commit e push só com ordem do líder).*

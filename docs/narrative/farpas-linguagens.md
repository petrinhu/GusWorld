# Farpas de disputa de linguagem (barks disparados por evento)

> **CANON. Aprovadas UMA A UMA pelo criador em 2026-07-26.** Qualquer mudança futura, inclusive de uma palavra, exige nova autorização explícita dele.

**O que é este arquivo.** Vinte e duas falas curtas de **par**, disparadas por **evento de jogo** (cast lento, dano alto, loot, colisão de efeitos, vitória sem baixas). É formato distinto do que vive em [`vozes-party.md`](vozes-party.md), que guarda **banter de caminhada e descanso**, mais longo e não disparado por evento. Ali estão as vozes; aqui está o que dispara.

**Origem.** Frente `LINGUAGENS-COMICAS-DISPUTAS`. Um inventário do material existente mostrou que o projeto já tinha muito banter escrito, e que a lacuna real era **farpa curta de par disparada por evento**, que não existia para nenhum par. Das 45 falas escritas antes do inventário, 23 foram descartadas por duplicarem canon. O par **Gus × Bento não recebeu nenhuma**, por já ter 6 trocas fechadas, 4 apartes, 2 revides e 2 cenas inteiras.

## A lei que decide quem ganha cada disputa

**Compilado = rápido = melhor. Interpretado = lento = pior. O eixo nunca inverte**, nem por equilíbrio, nem por simpatia com o personagem. É definição do Gus original e é mais pétrea que os pillars.

Consequência: **Pythia (Cauã e Jaci) perde no eixo da execução, sempre.** A única saída rápida honesta é **compilar de verdade** (`@jit`, ou builtin escrito em C). O que impede isso de virar humilhação repetida é que eles **ganham em outros eixos** e **concedem o fato sem discutir** quando o assunto é execução. Ver F19, que é o modelo da casa: a Jaci concede a lentidão na primeira palavra e mostra o que aquele atraso comprou.

**Nunca escrever a Jaci nem o Cauã negando a lentidão.** No dia em que negarem, a lei vira negociável.

## Requisitos de implementação

| Requisito | Aplica a | O que o sistema de barks precisa saber |
|---|---|---|
| **Elenco** | F11 a F22 (pares Cauã × Iara e Jaci × Linda) | Só disparam com o **Gus FORA do grupo ativo**. Sem o Gus na sala os dois chegam sozinhos a conclusões que favorecem o ausente, e é isso que faz o par existir em vez de virar plateia. |
| **Estado do bordão** | F02, F13 | Ficam melhores **depois** do payoff da [Cena 15](comic-reliefs.md) (o bordão do Cauã tem três estados, ver `CAUA-BORDAO-3-ESTADOS` no `TODO.md`). |
| **Versão por idioma** | F09 | A fala pt-br e a inglesa usam **pares de termos diferentes**, não tradução. Ver a nota na própria farpa e a seção 11 do [guia de diálogos](guia-dialogos.md). |

---

## 1. Gus × Cauã

### F01 · gatilho: carta Pythia demora a resolver

> **GUS:** "Ela está lendo o teu feitiço agora, Cauã. Enquanto ele acontece. Uma linha de cada vez."
>
> **CAUÃ:** "Está lendo com atenção. Diferente de você, que já saiu correndo."

Versão de evento do aparte A4, que é de caminhada. Dispara quando a lentidão está visível na tela, que é onde a piada tem lastro. O Cauã devolve transformando lentidão em atenção.

### F02 · gatilho: Cauã casta carta Pythia com `@jit`

> **CAUÃ:** "Essa aqui eu compilei. Compilou de verdade." *(a carta dispara sem atraso nenhum)*
>
> **GUS:** "Eu sei." *(pausa)* "Reparou que ela ficou rápida no exato segundo em que parou de ser interpretada?"
>
> **CAUÃ:** "Reparei. Não vou comentar."

**Primeira vez que a lei do eixo é dita em voz alta no jogo.** Até aqui ela só era demonstrada. O "não vou comentar" é a concessão do Cauã sem perder a cara.

### F03 · gatilho: descanso ou menu · trégua

> **CAUÃ:** "Enquanto você monta o teu, eu já tentei três jeitos diferentes do meu."
>
> **GUS:** "E dois estavam errados."
>
> **CAUÃ:** "E um estava certo. Eu só precisava de um."

Único respiro de um par cujas seis trocas canônicas são todas disputa. O Cauã fecha, e fecha com razão: iteração barata é a vantagem real dele, e o eixo da execução não é tocado aqui.

---

## 2. Gus × Iara

### F04 · gatilho: loot de carta Óxido

> **GUS:** "Por que a tua carta pesa três vezes mais que a minha pra fazer a mesma coisa?"
>
> **IARA:** "Porque ela leva a biblioteca inteira junto." *(sussurra o fim)* "Você prefere leve. Eu prefiro que chegue inteira."

Resposta que faltava ao aparte A7, provocativo e sem réplica desde que os 29 apartes foram aprovados. Ela **concede o fato** (a ligação é estática mesmo) e reformula em preferência.

### F05 · gatilho: o Gus toma dano do próprio conjuro, ou uma carta dele falha

> **IARA:** "Você pegou a memória do vizinho de novo, Gus."
>
> **GUS:** "Ela estava ali."
>
> **IARA:** "Estava ali. Não era tua."

Versão curta e de combate do revide R-06. **Não repete o número** do R-06 de propósito: estatística em fala de duas linhas vira locutor de rádio. Ela não corrige o fato, corrige a posse.

### F06 · gatilho: defesa bem-sucedida, aliado protegido · trégua

> **IARA:** "Bonito não é o contrário de seguro, Gus."
>
> **GUS:** "Legível também não."
>
> **IARA:** "Nisso a gente concorda." *(pausa)* "Só nisso."

Único acordo do par. O "só nisso" preserva a diferença permanente que a Cena 5 estabeleceu. Dispara quando a segurança dela acabou de salvar alguém, então a fala tem lastro no que o jogador viu.

---

## 3. Gus × Dante

As quatro se encaixam nas **três fases do deslize** já desenhadas em `vozes-party.md` §4 do guia do Dante. Nenhuma calibragem nova foi inventada. **Ninguém corrige o deslize na hora**, em nenhuma delas: o instante em que alguém aponta, o twist vaza.

### F07 · fase inicial · gatilho: ociosidade, party andando

> **DANTE:** "O Bento reza pra linguagem. Eu só uso."
>
> **GUS:** "É a mesma linguagem."
>
> **DANTE:** "É a mesma **ferramenta**." *(olha pro lado)*

Trocar "linguagem" por "ferramenta" é o esvaziamento inteiro do Asmódico numa palavra só, e casa com o guia dele ("fala o sotaque sem a fé"). Zero vocabulário de C-Arcane, respeitando a proibição da fase inicial. Na primeira leitura é o prático da turma; na releitura é a definição do personagem dita por ele mesmo.

### F08 · fase intermediária · gatilho: o Gus toma dano alto

> **DANTE:** "Deixa eu ver o teu Drive."
>
> **GUS:** "Está funcionando."
>
> **DANTE:** "Está. Eu confiro mesmo assim." *(olha pro lado)*

Aqui não vaza vocabulário, vaza **comportamento**: o Tavus-Drive é a única coisa que ele nunca deixa de conferir. Na superfície é zelo de mecânico, e é zelo de verdade, porque a competência e o cuidado dele são reais. É isso que faz a traição cortar.

### F09 · fase tardia · gatilho: conserto sob pressão

> **DANTE:** "Segue o ponteiro." *(pausa)* "O **endereço**. Segue o endereço." *(olha pro lado, mais longo que o normal)*
>
> **GUS:** "O endereço." *(volta pro que estava fazendo)*

**Ponteiro** é marca do C-Arcane e já é canon na boca do Gus; **endereço** é o que o Asmódico manipula. O padrão da fase tardia é vazamento mais autocorreção tardia, e é a autocorreção que denuncia: quem fala a própria língua não precisa se emendar. O que dói na releitura é o Gus aceitar como sinônimo e seguir.

> **Versão por idioma (regra do guia de diálogos, seção 11).** Em inglês o par natural é `compile` → `assemble`, que funciona porque *assembler* é o nome da ferramenta e o verbo é de uso corrente. **Esse par MORRE em pt-br**, onde se usa a própria palavra "assembly" e derivados, e ninguém diz "montar". Não traduzir esta fala: usar a versão de cada idioma.

### F10 · fase tardia · gatilho: manutenção do aparelho ortodôntico

> **DANTE:** "Fica parado. Não vai doer."
>
> **GUS:** "Nunca doeu."
>
> **DANTE:** *(pausa)* "Pois é..."

Não há deslize de vocabulário: o que carrega é a **repetição**. "Não vai doer" é frase que o guia já marca como dita por ele nas duas situações, o combate e a manutenção. "Nunca doeu" é literalmente verdade, e é o ponto: ele nunca machucou o Gus. As reticências do "Pois é..." deixam a fala suspensa, com ele quase dizendo mais e não dizendo.

---

## 4. Cauã × Iara

**Requisito de elenco: só com o Gus FORA do grupo ativo.**

**Eixo do par:** iteração quente e barata (Cauã) contra decisão cara e definitiva (Iara). É Pythia contra Óxido sem intermediário.

### F11 · gatilho: a Iara demora a preparar uma carta

> **CAUÃ:** "Eu já usei a minha três vezes no tempo que você levou até agora."
>
> **IARA:** *(sem levantar os olhos)* "Duas você errou."

Ela não nega a velocidade dele. Conta os erros.

### F12 · gatilho: Cauã casta carta Pythia lenta

> **IARA:** "Ela está pensando."
>
> **CAUÃ:** "Ela está CARREGANDO."
>
> **IARA:** "Do lado de fora é a mesma coisa."

Ela elogia com ironia, ele morde a isca e corrige em maiúscula, e ela fecha com o que importa em combate: **de fora, esperar é esperar**. Respeita a lei sem enunciá-la.

### F13 · gatilho: Cauã casta carta Pythia com `@jit`

> **CAUÃ:** "Essa aqui compila antes de rodar. De verdade." *(a carta dispara instantânea)*
>
> **IARA:** *(pausa)* "...essa é da minha família."
>
> **CAUÃ:** "É da minha família. Só passou a andar mais rápido."

A Iara **reivindica a carta** para o lado compilado, e a pausa é o reconhecimento da passagem de fronteira. Diferença que importa: com o Gus (F02) o Cauã concede sem comentar; sem o Gus, ele **defende a posse**. A mesma pessoa se comporta diferente conforme quem está olhando.

### F14 · gatilho: Cauã fala da comunidade dos Dutos

> **CAUÃ:** "Um moleque de oito anos dos Dutos escreveu isso numa tarde."
>
> **IARA:** "Quanto tempo até ele entender por que funciona?"
>
> **CAUÃ:** "O bloco dele já está protegido. Ele entende depois."

Única do par em que a disputa é sobre o **uso**, não sobre a linguagem. A objeção dela é legítima (acessível demais significa gente operando o que não entende) e ele fecha com o goal canônico dele. Eco deliberado: o mesmo moleque de oito anos que dá razão a ele aqui é quem o desmonta com uma pergunta na Cena 15.

### F15 · gatilho: dois efeitos da party colidem, ou um buff sobrescreve o outro

> **IARA:** "O meu tem dono. Cada coisa, um dono, e escrito onde."
>
> **CAUÃ:** "O meu tem quem chegar primeiro."
>
> **IARA:** "É o que eu disse."

Dispara quando o jogador **acabou de ver dois efeitos brigarem**. Ele descreve o próprio modelo achando que defende, e ela só aponta que ele acabou de dar razão a ela. Base factual: posse verificada antes de rodar, sem custo em execução (A12, fontado).

### F16 · gatilho: loot de carta nova

> **CAUÃ:** "Por que a tua carta é tão pesada?"
>
> **IARA:** "Porque ela chega inteira em qualquer lugar."
>
> **CAUÃ:** "A minha chega leve e pede um copo d'água quando chega."

Mesma pergunta da F04, sem o Gus: com ele ela sussurrou uma preferência, com o Cauã ela afirma sem rodeio. Ele **concede o fato** (a dele precisa do interpretador do outro lado, aparte A6) e sai por cima da piada. Modelo de concessão limpa.

---

## 5. Jaci × Linda

**Requisito de elenco: só com o Gus FORA do grupo ativo.**

**Eixo do par:** as duas contam o tempo inteiro, por motivos opostos. A **Linda conta pra frente**, para sincronizar (o compasso). A **Jaci conta pra trás**, para conferir (a recontagem). **Nenhuma percebeu que a outra também é uma contadora.** Elas não brigam, se desencontram.

**Cuidados de voz herdados:** a Jaci nunca grita, a Linda nunca ornamenta, e **nenhuma farpa tira os fones dela** (o ritmo da Linda só para em dor real, e a F22 é a exceção que confirma). O tic de off-by-one da Jaci pode ser tocado de leve, **nunca como punchline**: a Cena 9 já gastou essa piada.

### F17 · gatilho: início de combate, ou preparação de investida

> **LINDA:** "Três, dois, um."
>
> **JACI:** "Espera. Deixa eu contar de novo."
>
> **LINDA:** "Eu já contei."
>
> **JACI:** "Você contou pra frente." *(pausa)* "Eu conto pra trás."

### F18 · gatilho: cura sob pressão, ou uso do último item

> **LINDA:** "O meu não monta com peça faltando. Ele para antes de mim."
>
> **JACI:** "O meu monta. E se faltar, ele me diz qual faltou. Com o nome e o número."

Única troca do arquivo em que **as duas estão certas e não há vencedor**: a recusa de compilar de um lado, o erro identificado com nome e número do outro (R-02, fontado). "Ele para antes de mim" é a fala de quem já se viu errando.

### F19 · gatilho: a cura da Jaci resolve tarde

> **LINDA:** "Sua cura demorou meio compasso."
>
> **JACI:** "Demorou." *(não discute)* "Ele respirou duas vezes nesse meio compasso."

**Modelo da casa para a lei do eixo.** Ela concede a lentidão na primeira palavra e mostra o que o atraso comprou. Nunca escrever a Jaci negando a lentidão.

### F20 · gatilho: Jaci usa carta cujo trabalho pesado é builtin compilado

> **JACI:** "Essa aqui não é bem minha." *(pausa)* "Quer dizer, é. Mas por baixo dela tem uma peça compilada, e é ela que faz o trabalho."
>
> **LINDA:** *(ajusta o fone)* "Foi no compasso."
>
> **JACI:** "Foi. Porque a peça compilou."

Quem enuncia a lei aqui é a própria Jaci, sobre a própria carta, **sem ressentimento**. Base factual: o núcleo numérico e os builtins do CPython são C pré-compilado (A2 e R-08, fontados).

### F21 · gatilho: distribuição de itens antes de uma missão

> **LINDA:** "Decide agora quem fica com a última."
>
> **JACI:** "Eu não sei quem vai precisar antes de abrir a caixa."
>
> **LINDA:** "O meu exige que você saiba." *(pausa)* "**Antes**."

Argumento mais forte da Linda, e não é sobre linguagem: é sobre **decidir antes de ter certeza**. Dispara quando o jogador tem que escolher quem leva o último item sem saber quem vai apanhar.

### F22 · gatilho: vitória sem baixas · trégua

> **LINDA:** *(tira um fone)* "Ninguém sangrando?"
>
> **JACI:** "Ninguém." *(confere de novo)* "Agora ninguém."

Fecha o eixo do par sem explicar nada: a Linda **tira o fone**, gesto reservado para quando o ritmo dela para de verdade, e a Jaci **reconta** antes de confirmar. O "agora ninguém" não é correção: ela recontou e a resposta continuou verdadeira. As duas contadoras finalmente contando a mesma coisa, sem nunca nomear isso.

---

**Cross-ref:** [`vozes-party.md`](vozes-party.md) (vozes e banter de caminhada) · [`gus-apartes-c-arcane.md`](gus-apartes-c-arcane.md) (os 29 apartes e a régua fato x juízo) · [`comic-reliefs.md`](comic-reliefs.md) (cenas fechadas, regras de tom e a regra dos pesos) · [`characters/party.md`](characters/party.md) (matriz de dinâmicas).

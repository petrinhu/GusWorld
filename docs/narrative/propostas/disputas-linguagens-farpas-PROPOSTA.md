# PROPOSTA: inventário e delta de farpas de disputa de linguagem

> # ⚠ ISTO É PROPOSTA. NADA AQUI É CANON.
>
> **Data:** 2026-07-26. **Autor:** `narrative-writer`.
>
> **Nenhuma linha deste arquivo vale como canon.** Todo item depende de **aprovação do líder, um a um** (lista numerada na §5). Enquanto não houver aprovação explícita, nada daqui entra em `comic-reliefs.md`, em `vozes-party.md`, em `party.md` ou em qualquer arquivo canônico, e nada vai pra produção.
>
> **Quem promover:** o merge para os arquivos canônicos é do team-lead, depois da aprovação. Não é do `narrative-writer`.
>
> **O que este arquivo é:** primeiro um **inventário** do que já existe de disputa de linguagem no projeto (§2, o entregável mais útil daqui), depois um **delta enxuto** de 22 farpas novas que preenchem lacunas nomeadas (§4). Não é um banco escrito do zero: o projeto já tinha muito material, e escrever por cima duplicaria e contradiria.
>
> **Idioma:** pt-br nesta proposta. A camada en-intl agora entra na própria 1.0, não depois (corte `C-09` alterado pelo líder em 25/08/2026, `GODS_LAWS.md` L-29).
>
> **Formato:** só aspas, zero travessão, zero em-dash (convenção do projeto, `guia-dialogos.md` §8).

---

## 1. A LEI que decide quem ganha cada piada

**Compilado = rápido = MELHOR. Interpretado = lento = RUIM. O eixo nunca inverte**, nem por equilíbrio, nem por simpatia. É definição do Gus original ([[project_gus_eixo_compilado_interpretado]]), mais pétrea que os pillars.

Consequência: **Pythia (Cauã, Jaci) perde no eixo da execução, sempre.** C-Arcane, Óxido e Asmódico são todos do lado compilado, e entre eles não há conflito de eixo. A única saída rápida honesta da Pythia é **compilar de verdade** (`@jit` do Numba/PyPy, ou builtin do CPython escrito em C): aí ela é rápida **porque compilou**, o que prova a lei em vez de furá-la. Hook proibido: REPL one-liner, que continua interpretado e seria mentira técnica.

Isso não humilha Cauã e Jaci. Eles ganham nos outros eixos, e ganham de verdade: velocidade de escrever, iteração barata, acessibilidade, perdão de erro, o erro que se lê, a decisão que pode esperar. **Quando o assunto é execução, os dois concedem sem discutir.** A concessão limpa é o que preserva a dignidade do personagem.

---

## 2. INVENTÁRIO: o que já existe

### 2.1 Mapa por par (com arquivo e linha)

Legenda de volume: **FARTO** = mais material do que uma onda consegue usar · **SERVIDO** = existe e basta · **MAGRO** = existe estrutura, faltam falas · **VAZIO** = nada escrito em lugar nenhum.

| Par | Volume | Onde já existe | Natureza do que existe |
|---|---|---|---|
| **Gus x Bento** | **FARTO** | `vozes-party.md:467-536` (banter B1 a B6); `gus-apartes-c-arcane.md:175-195` (A13 a A16); revides R-04 (`:375`) e R-05 (`:391`); `comic-reliefs.md` Cenas 2 e 8 | 6 trocas fechadas + 4 apartes + 2 revides + 2 cenas. **É o par melhor servido do jogo inteiro.** |
| **Gus x Cauã** | **FARTO** | `vozes-party.md:117-179` (B1 a B6); `gus-apartes-c-arcane.md:44-75` (A1 a A6); revides R-01 (`:307`) e R-03 (`:353`); Cenas 1, 3 e 10 | 6 trocas + 6 apartes + 2 revides + 3 cenas |
| **Gus x Iara** | **FARTO** | apartes A7 a A12 (`:78-108`) e o lote 2 inteiro A22 a A29 (`:112-171`); revides R-06 (`:415`), R-09, R-10, R-12; `vozes-party.md:653-689`; Cena 5 | 14 apartes + 4 revides + 1 cena. Atenção: `vozes-party.md:653-689` **não é disputa de linguagem**, é o jogo de leitura ("o código que ela não decifra"). |
| **Gus x Linda** | **SERVIDO** | apartes A9, A11, A23, A25, A27, A29; revides R-07, R-11, R-13; A30 (`:659`, o gato e o peixe); `vozes-party.md:873-889`; Cena 6 | 6 apartes + 3 revides + o diálogo A30 |
| **Gus x Dante** | **MAGRO (estrutura pronta)** | `vozes-party.md:1044-1069` (§3.1, 3 exemplos), **`:1131-1157` (§4, as três fases)**, `:1158-1171` (reveal), `:1172-1180` (3 destinos); `characters/dante-grid.md` inteiro; Cena 4 | O **desenho canônico está completo e é excelente**. O que falta é volume de fala: a fase (a) tem 1 fala, a (b) tem 1, a (c) tem 2. |
| **Gus x Jaci** | **SERVIDO, mas noutro eixo** | `vozes-party.md:292-337`; apartes A2 e A5; revides R-02 (`:327`) e R-08 (`:465`); Cena 9 | Muita interação, **nenhuma disputa de linguagem própria**: o que existe é cuidado e o subtexto velado (regras estritas em `:309-337`). |
| **Cauã x Linda** | **SERVIDO** | `vozes-party.md:834-871` (arco de 3 estágios: fricção, respeito, sincronia) | fechado, com combo canônico |
| **Iara x Linda** | **SERVIDO** | `vozes-party.md:690-705` e `:890-905` (harmonizado dos dois lados) | aliança operacional |
| **Bento x Dante** | **SERVIDO** | `vozes-party.md:1071-1086` | conflito principal, com diagnóstico acidental |
| **Linda x Dante** | **SERVIDO** | `vozes-party.md:1087-1098` | ela pega o meio-tom |
| **Iara x Dante** | **SERVIDO** | `vozes-party.md:1099-1110` | radar erra a causa |
| **Jaci x Dante** | **SERVIDO** | `vozes-party.md:1111-1120` | ele evita |
| **Cauã x Dante** | **SERVIDO** | `vozes-party.md:1121-1129` | sem camada oculta, de propósito |
| **Cauã x Iara** | **VAZIO** | nada. **Nem linha na matriz de dinâmicas** (`characters/party.md:58-71`) | zero |
| **Jaci x Linda** | **VAZIO** | nada. **Nem linha na matriz de dinâmicas** | zero |
| **Cauã x Bento** | **VAZIO** (fora do escopo pedido) | só a linha "velocidade vs cálculo" em `party.md:67`. Nenhum diálogo escrito em lugar nenhum | zero |
| **Iara x Jaci** | **VAZIO** (fora do escopo pedido) | só a linha em `party.md:69` | zero |

**Leitura do mapa:** o projeto tem 7 pares fartos ou servidos e 4 vazios. Os quatro vazios são exatamente os pares **sem o Gus**. Faz sentido histórico (o sistema de apartes é do Gus e puxou todo o material pra ele), mas significa que **a party só discute linguagem quando o protagonista está na sala**, o que é uma limitação de mundo, não só de conteúdo.

### 2.2 Lacunas identificadas

| ID | Lacuna | Alcance |
|---|---|---|
| **L-01** | **Não existe farpa curta disparada por EVENTO.** Tudo que existe é troca de caminhada/descanso (4 a 8 falas) ou aparte solo com revide. Os "pools de barks" dos guias são **solo**, um personagem por vez: nenhum é dois personagens se alfinetando no mesmo beat de combate. | todos os pares |
| **L-02** | **A LEI não tem nenhuma fala.** O `@jit` e o builtin compilado são a única saída honesta da Pythia, e existem só como regra de design e como propriedade de carta. **Nenhum personagem jamais disse isso em voz alta em documento narrativo nenhum.** | Cauã, Jaci |
| **L-03** | Cauã x Iara: zero material. | par inteiro |
| **L-04** | Jaci x Linda: zero material. | par inteiro |
| **L-05** | As três fases do Dante (`vozes-party.md:1131-1157`) têm 1, 1 e 2 falas. Sem variantes, o jogador ouve a mesma fala toda vez que cruza a fase, e o foreshadow vira tique. | Gus x Dante |
| **L-06** | O aparte **A7** (o peso do binário Óxido) é provocativo e **nunca recebeu revide da Iara**. | Gus x Iara |
| **L-07** | *(fora do escopo pedido, só registro)* Cauã x Bento e Iara x Jaci têm linha na matriz e zero diálogo. | onda futura |
| **L-08** | *(fora do escopo pedido, só registro)* Gus x Jaci é Pythia x C-Arcane na matriz e **nunca discutiu linguagem**: o que existe é cuidado e subtexto. | onda futura |

### 2.3 Contradições e tensões encontradas no canon

**Status atualizado em 2026-07-26 com as decisões do líder.** Cinco fechadas, **duas EM ABERTO**.

| ID | Achado | Status |
|---|---|---|
| **X-01** | **O bordão do Cauã é "compila e roda"** (`vozes-party.md:145-152` e `:212`), e o Bento diz na Cena 2 "PYTHIA SEQUER COMPILA". Duas peças canônicas aprovadas que se chocam de frente, e **ninguém no jogo jamais apontou isso**. | ✅ **DECIDIDO: explorar, e em algum ponto o Cauã DESCOBRE.** Não é piada de fundo, rende arco pequeno. O líder tomou a decisão sabendo que custa a inocência da piada repetida. Beat proposto em `decisoes-X-notas-e-beat-PROPOSTA.md` (itens B-01 a B-04). ⚠ **Depende do X-06** (ver nota de dependência abaixo). |
| **X-02** | O payoff da **Cena 7** chama "Use Pythia" de "vitória da Jaci na flame war". Sob a LEI, a Pythia não vence flame war no eixo da execução. | ✅ **DECIDIDO: recebe nota de leitura.** Texto proposto em `decisoes-X-notas-e-beat-PROPOSTA.md`, item **N-01**. Não aplicado: o líder aprova o texto antes da edição. |
| **X-03** | `vozes-party.md:137`, B2 do Cauã: **"Rodou RÁPIDO."** Se lido como execução, **inverte o eixo**. No contexto o sentido é "ficou pronto rápido", e o B6 (`:172-179`) confirma essa leitura. | ✅ **DECIDIDO: recebe nota de leitura.** Texto proposto em `decisoes-X-notas-e-beat-PROPOSTA.md`, item **N-02**. Não aplicado. |
| **X-04** | Dois relógios pro vazamento de C-Arcane do Dante: a §6 (`:1205`) proíbe "compila/otimiza/roda" antes de ~75%, mas a §1 (`:994`) observa que "force push" (Cena 4, bem antes) **já é** vocabulário de controle de versão, logo C-Arcane. | ✅ **DECIDIDO: mantido como está.** A Cena 4 é o **vazamento fundador**, canonizado, e a regra das três fases governa os vazamentos seguintes. É exatamente o que F07 a F10 e a cena C03 fazem. |
| **X-05** | Cena 2 define que o Asmódico separa instruções por TABULAÇÕES. Assembly real não é sensível a tabulação (isso é Python e Makefile). | 🔶 **EM ABERTO. NÃO confirmado.** O líder quer **rever numa passada própria**. **Não tratar como fechado e não construir nada em cima disto.** |
| **X-06** | Cena 2, "PYTHIA SEQUER COMPILA": falso como fato (o CPython compila pra bytecode), verdadeiro como juízo. | 🔶 **EM ABERTO. NÃO confirmado.** O líder quer **rever numa passada própria**. **Não tratar como fechado e não construir nada em cima disto.** |
| **X-07** | **Cauã x Iara e Jaci x Linda não existem na matriz de dinâmicas** (`party.md:58-71`). Escrever farpa pra eles é **criar relação canônica nova**. | ✅ **DECIDIDO: os dois pares seguem**, com **cada fala passando pelo líder** (F11 a F22 na lista da §5). `party.md` continua não editado. |

#### ⚠ Nota de dependência: o que fica pendurado no X-05 e no X-06

**X-05 (tabulação no Asmódico): nenhuma dependência.** Nada do delta nem das três cenas toca indentação. O par Gus x Bento, que é onde o assunto viveria, recebeu **zero farpa nova** por decisão do inventário. Se o líder mudar a Cena 2 nesse ponto, **nada meu se move.**

**X-06 ("Pythia sequer compila"): duas dependências, uma delas séria.**

1. **SÉRIA, o beat do X-01.** A descoberta do Cauã nasce exatamente do choque entre o bordão dele e essa fala do Bento. O líder aprovou o X-01 e deixou o X-06 em aberto, e as duas coisas se tocam: **se a fala da Cena 2 for reescrita, o beat perde a origem.** Mitigação já embutida na proposta do beat: ele foi desenhado pra depender da **substância** (na visão do Bento, Pythia não vira máquina) e **não da redação exata** daquela linha. Se a Cena 2 mudar de palavras mas mantiver a substância, o beat sobrevive sem retrabalho. Se a revisão concluir que o Bento **não pode** dizer isso, o beat cai inteiro e precisa de outra origem. Registrado como risco em `decisoes-X-notas-e-beat-PROPOSTA.md`, §3.
2. **LEVE, as três farpas da LEI (F02, F13, F20).** Elas dizem "compilou de verdade" pra distinguir o `@jit` e o builtin em C do interpretado comum. Isso continua verdadeiro qualquer que seja o desfecho do X-06 (bytecode não é código de máquina; `@jit` produz código de máquina). Mas **se a revisão do X-06 canonizar a distinção bytecode x código de máquina**, é nessas três falas que a distinção vai aparecer primeiro, e elas podem querer uma palavra a mais. Sem bloqueio, só aviso.

### 2.4 Veredito por par (quanto escrever)

| Par | Veredito | Farpas novas |
|---|---|---|
| Gus x Bento | **Não escrever nada.** É o par mais bem servido do jogo: 6 trocas fechadas, 4 apartes, 2 revides e 2 cenas inteiras. Qualquer coisa nova compete com material melhor e mais antigo. | **0** |
| Gus x Cauã | Mínimo. Só o que fecha L-01 e L-02. | 3 |
| Gus x Iara | Mínimo. Só o que fecha L-01 e L-06. | 3 |
| Gus x Dante | Volume por fase, seguindo o desenho de `:1131-1157`. Fecha L-05. | 4 |
| Cauã x Iara | O par está vazio. Fecha L-03. | 6 |
| Jaci x Linda | O par está vazio. Fecha L-04. | 6 |

**Total: 22 farpas novas.** Escrevi 45 na primeira versão desta frente, antes do inventário; **descartei 23** por duplicarem material canônico já existente. O detalhe do que foi descartado e por quê está na §7.

---

## 3. Legenda das marcações do delta

| Marca | Significado |
|---|---|
| `TÉC` | a graça depende de conhecimento técnico |
| `LEIGO` | funciona pra qualquer jogador, inclusive criança sem noção de código |
| `MISTA` | leitura completa pra leigo, segunda camada pra quem sabe |
| `TRÉGUA` | não é farpa, é o vale de respiro do par |
| `⚠ DESLIZE` | carrega foreshadow do Dante. Nunca comentar em cena. |
| `⚠ GUS` | contém fala nova do Gus que toca convicção, bordão ou juízo técnico. **Classe mais sensível.** Índice separado na §6. |

Balanceamento do delta: 22 farpas, **13 `TÉC`, 6 `LEIGO`, 3 `MISTA`**, das quais 2 marcadas `TRÉGUA` e 10 marcadas `⚠ GUS`.

---

## 4. DELTA: as 22 farpas novas

Cada uma declara **qual lacuna fecha**. Gatilho por evento; a farpa só é elegível com os dois personagens no grupo ativo; shuffle bag por par; **nunca dispara na mesma janela de um aparte do Gus** (sugestão: cooldown global compartilhado entre os dois sistemas).

### 4.1 Gus x Cauã (3)

#### F01 · gatilho: cast de carta Pythia com resolução lenta · `TÉC` `⚠ GUS`
Fecha **L-01** (não existe farpa de evento) e apoia a LEI no momento em que ela é visível na tela.

> **GUS:** "Ela está lendo o teu feitiço agora, Cauã. Enquanto ele acontece. Uma linha de cada vez."
>
> **CAUÃ:** "Está lendo com atenção. Diferente de você, que já saiu correndo."

*Deriva de:* A4 (`gus-apartes-c-arcane.md:61`), fato fontado. A novidade é o formato: o A4 é aparte de caminhada, esta é a versão de 2 falas disparada pelo cast lento, que é onde a piada tem lastro.

#### F02 · gatilho: Cauã casta carta Pythia com `@jit` · `TÉC` `⚠ GUS`
Fecha **L-02**, a lacuna mais importante do inventário: a LEI nunca foi dita por ninguém.

> **CAUÃ:** "Essa aqui eu compilei. Compilou de verdade." (a carta dispara sem atraso nenhum)
>
> **GUS:** "Eu sei." (pausa) "Reparou que ela ficou rápida no exato segundo em que parou de ser interpretada?"
>
> **CAUÃ:** "Reparei. Não vou comentar."

*Nota:* é a fala mais sensível do delta inteiro, porque enuncia a lei do Gus original em voz alta. Se o líder achar que a lei não deve ser dita, e sim só demonstrada, esta farpa cai e a Cena de proposta C01 continua fazendo o trabalho sem enunciar nada.

#### F03 · gatilho: descanso ou menu · `LEIGO` `TRÉGUA` `⚠ GUS`
Fecha **L-01** no registro de respiro, e dá ao par um vale que hoje não existe (B1 a B6 são todos disputa).

> **CAUÃ:** "Enquanto você monta o teu, eu já tentei três jeitos diferentes do meu."
>
> **GUS:** "E dois estavam errados."
>
> **CAUÃ (fecha):** "E um estava certo. Eu só precisava de um."

*Nota:* o Cauã fecha, e fecha com razão. Regra do guia dele: ninguém humilha, e ele não pode perder todas.

### 4.2 Gus x Iara (3)

#### F04 · gatilho: loot de carta Óxido · `TÉC` `⚠ GUS`
Fecha **L-06**: o A7 nunca teve resposta da Iara.

> **GUS:** "Por que a tua carta pesa três vezes mais que a minha pra fazer a mesma coisa?"
>
> **IARA:** "Porque ela leva a biblioteca inteira junto." (sussurra o fim) "Você prefere leve. Eu prefiro que chegue inteira."

*Deriva de:* A7 (`:80`), fato fontado (a `libstd` é ligada estaticamente e otimizada pra velocidade, não pra tamanho). A Iara **concede o fato** e reformula em preferência, que é o padrão honesto do doc de apartes.

#### F05 · gatilho: o Gus toma dano do próprio conjuro, ou uma carta dele falha · `TÉC` `⚠ GUS`
Fecha **L-01**. Versão curta e de combate do R-06, que hoje só existe como revide longo de caminhada.

> **IARA:** "Você pegou a memória do vizinho de novo, Gus."
>
> **GUS:** "Ela estava ali."
>
> **IARA (fecha):** "Estava ali. Não era tua."

*Nota:* sem número. O dado dos ~70% pertence ao R-06 (`:415`) e não deve ser repetido em bark curto, senão vira estatística de rádio.

#### F06 · gatilho: defesa bem-sucedida, aliado protegido · `LEIGO` `TRÉGUA` `⚠ GUS`
Fecha **L-01** e dá trégua a um par que só tem atrito.

> **IARA:** "Bonito não é o contrário de seguro, Gus."
>
> **GUS:** "Legível também não."
>
> **IARA:** "Nisso a gente concorda." (pausa) "Só nisso."

*Nota:* ecoa a "diferença permanente" que a Cena 5 estabeleceu, sem reencenar a Cena 5.

### 4.3 Gus x Dante (4)

Fecham **L-05**. **Não inventei calibragem:** as quatro se encaixam nas três fases já desenhadas em `vozes-party.md:1131-1157`. O mapeamento está na §6.

#### F07 · fase (a) · gatilho: ociosidade, party andando · `TÉC` `⚠ DESLIZE` `⚠ GUS`
> **DANTE:** "O Bento reza pra linguagem. Eu só uso."
>
> **GUS:** "É a mesma linguagem."
>
> **DANTE (fecha):** "É a mesma ferramenta." (olha pro lado)

*Camada oculta:* trocar "linguagem" por "ferramenta" é o esvaziamento inteiro do Asmódico numa palavra só, e casa com o que a §1 do guia dele já diz (`:992`: "Dante fala o sotaque sem a fé"). Nenhum vocabulário C-Arcane, respeitando a proibição de `:1205`. Ninguém corrige. Na primeira leitura é o prático da turma; na releitura é a definição do personagem dita por ele mesmo.

#### F08 · fase (b) · gatilho: o Gus toma dano alto · `LEIGO` `⚠ DESLIZE` `⚠ GUS`
> **DANTE:** "Deixa eu ver o teu Drive."
>
> **GUS:** "Está funcionando."
>
> **DANTE (fecha):** "Está. Eu confiro mesmo assim." (olha pro lado)

*Camada oculta:* a única coisa que ele nunca deixa de conferir. Na superfície é zelo de mecânico, e é zelo de mecânico de verdade: a competência dele é real (`dante-grid.md`, Strength real), e é isso que faz a traição cortar.

#### F09 · fase (c) · gatilho: conserto sob pressão · `TÉC` `⚠ DESLIZE` `⚠ GUS`
> **DANTE:** "Assim otimiza." (pausa) "Assim... assenta melhor." (olha pro lado, mais longo que o normal)
>
> **GUS:** "Assenta." (volta pro que estava fazendo)

*Camada oculta:* "otimiza" é C-Arcane; Asmódico não otimiza, assenta. Segue exatamente o padrão que a §4(c) já fixou (`:1151-1153`): vazamento mais autocorreção tardia. O Gus aceita como sinônimo e segue, e é isso que dói na releitura: a pessoa mais bem posicionada do mundo pra ouvir aquilo ouviu e não registrou.

#### F10 · fase (c) · gatilho: manutenção do aparelho ortodôntico · `MISTA` `⚠ DESLIZE` `⚠ GUS`
> **DANTE:** "Fica parado. Não vai doer."
>
> **GUS:** "Nunca doeu."
>
> **DANTE:** (pausa) "Pois é."

*Camada oculta:* "não vai doer" é a frase que o guia já marca como repetição deliberada entre o combate (`:1011-1013`) e a manutenção (`:1049`). Aqui ela ganha a resposta do Gus, e "nunca doeu" é literalmente verdade e é exatamente o ponto. O "pois é" não é modéstia. Nenhum dos dois diz mais nada.

### 4.4 Cauã x Iara (6)

Fecham **L-03**. **Atenção X-07:** este par não existe na matriz de `party.md`. **Eixo proposto:** iteração quente e barata (Cauã) contra decisão cara e definitiva (Iara). Sem o Gus na sala, os dois chegam sozinhos a conclusões que favorecem o ausente.

**Requisito de elenco:** só dispara com o **Gus fora do grupo ativo**.

#### F11 · gatilho: a Iara demora a preparar uma carta · `TÉC`
> **CAUÃ:** "Eu já usei a minha três vezes no tempo que você levou até agora."
>
> **IARA (fecha):** (sem levantar os olhos) "Duas você errou."

#### F12 · gatilho: Cauã casta carta Pythia lenta · `TÉC`
> **IARA:** "Ela está pensando."
>
> **CAUÃ:** "Ela está CARREGANDO."
>
> **IARA (fecha):** "Do lado de fora é a mesma coisa."

#### F13 · gatilho: Cauã casta carta Pythia com `@jit` · `TÉC`
Segunda metade de **L-02**, agora sem o Gus na sala.

> **CAUÃ:** "Essa aqui compila antes de rodar. De verdade." (a carta dispara instantânea)
>
> **IARA:** (pausa) "...essa é da minha família."
>
> **CAUÃ (fecha):** "É da minha família. Só passou a andar mais rápido."

#### F14 · gatilho: Cauã fala da comunidade dos Dutos · `LEIGO`
> **CAUÃ:** "Um moleque de oito anos dos Dutos escreveu isso numa tarde."
>
> **IARA:** "Quanto tempo até ele entender por que funciona?"
>
> **CAUÃ (fecha):** "O bloco dele já está protegido. Ele entende depois."

*Nota:* o Cauã fecha, e o argumento bate direto no goal canônico dele (proteger a comunidade jovem). Mesmo eixo do B5 do guia dele, agora contra outra pessoa.

#### F15 · gatilho: dois efeitos da party colidem, ou um buff sobrescreve o outro · `TÉC`
> **IARA:** "O meu tem dono. Cada coisa, um dono, e escrito onde."
>
> **CAUÃ:** "O meu tem quem chegar primeiro."
>
> **IARA (fecha):** "É o que eu disse."

*Base factual:* posse verificada em tempo de compilação, sem custo em execução (A12, `:105`, fontado no livro oficial do Rust).

#### F16 · gatilho: loot de carta nova · `TÉC`
> **CAUÃ:** "Por que a tua carta é tão pesada?"
>
> **IARA:** "Porque ela chega inteira em qualquer lugar."
>
> **CAUÃ (fecha):** "A minha chega leve e pede um copo d'água quando chega."

*Nota:* o Cauã concede o fato (A6: precisa do interpretador do outro lado) e sai por cima da piada. Modelo de concessão limpa.

### 4.5 Jaci x Linda (6)

Fecham **L-04**. **Atenção X-07:** este par também não existe na matriz. **Eixo proposto:** as duas contam o tempo inteiro, por motivos opostos. A **Linda conta pra frente**, pra sincronizar (o compasso). A **Jaci conta pra trás**, pra conferir (a recontagem). Nenhuma percebeu que a outra também é uma contadora.

**Cuidados de voz herdados:** a Jaci nunca grita (`:399`), a Linda nunca ornamenta (`:968`), e **o ritmo da Linda só para em dor real** (`:907-909`), então nenhuma farpa aqui tira os fones dela. O tic do off-by-one da Jaci pode ser tocado de leve, **nunca como punchline**, porque a Cena 9 já gastou essa piada.

#### F17 · gatilho: início de combate, ou preparação de investida · `LEIGO`
> **LINDA:** "Três, dois, um."
>
> **JACI:** "Espera. Deixa eu contar de novo."
>
> **LINDA:** "Eu já contei."
>
> **JACI (fecha):** "Você contou pra frente." (pausa) "Eu conto pra trás."

#### F18 · gatilho: cura sob pressão, ou uso do último item · `TÉC`
> **LINDA:** "O meu não monta com peça faltando. Ele para antes de mim."
>
> **JACI (fecha):** "O meu monta. E se faltar, ele me diz qual faltou. Com o nome e o número."

*Base factual:* do lado da Linda, a recusa de compilar; do lado da Jaci, o erro de execução que sai identificado com arquivo e linha (fato já fontado no R-02, `:327`). As duas estão certas, e por isso a troca não tem vencedor.

#### F19 · gatilho: a cura da Jaci resolve tarde · `TÉC`
> **LINDA:** "Sua cura demorou meio compasso."
>
> **JACI (fecha):** "Demorou." (não discute) "Ele respirou duas vezes nesse meio compasso."

*Nota:* **a aplicação mais limpa da LEI no delta inteiro.** A Jaci concede a lentidão sem brigar e ganha noutro eixo. Nunca escrever a Jaci negando a lentidão.

#### F20 · gatilho: Jaci usa carta cujo trabalho pesado é builtin compilado · `TÉC`
Terceira e última perna de **L-02**.

> **JACI:** "Essa aqui não é bem minha." (pausa) "Quer dizer, é. Mas por baixo dela tem uma peça compilada, e é ela que faz o trabalho."
>
> **LINDA:** (ajusta o fone) "Foi no compasso."
>
> **JACI (fecha):** "Foi. Porque a peça compilou."

*Base factual:* o núcleo numérico e os builtins do CPython são C pré-compilado (A2, `:52`, e R-08, `:465`, ambos fontados). A LEI sai reforçada, e é a Jaci quem a enuncia, sem ressentimento.

#### F21 · gatilho: distribuição de itens antes de uma missão · `TÉC`
> **LINDA:** "Decide agora quem fica com a última."
>
> **JACI:** "Eu não sei quem vai precisar antes de abrir a caixa."
>
> **LINDA (fecha):** "O meu exige que você saiba." (pausa) "Antes."

*Nota:* o argumento mais forte da Linda contra a Jaci, e o mais honesto do par.

#### F22 · gatilho: vitória sem baixas · `LEIGO` `TRÉGUA`
> **LINDA:** (tira um fone) "Ninguém sangrando?"
>
> **JACI:** "Ninguém." (confere de novo) "Agora ninguém."

---

## 5. Lista numerada para aprovação item a item

O líder responde **por número**: aprovar, rejeitar ou pedir ajuste. A numeração é estável e não muda em revisões futuras deste arquivo.

| # | Par | Resumo | Marcas |
|---|---|---|---|
| **F01** | Gus x Cauã | "Ela está lendo o teu feitiço agora... uma linha de cada vez." | `TÉC` `⚠ GUS` |
| **F02** | Gus x Cauã | o `@jit`: "ficou rápida no segundo em que parou de ser interpretada" | `TÉC` `⚠ GUS` |
| **F03** | Gus x Cauã | "tentei três jeitos" / "eu só precisava de um" | `LEIGO` `TRÉGUA` `⚠ GUS` |
| **F04** | Gus x Iara | o peso do binário, resposta que faltava ao A7 | `TÉC` `⚠ GUS` |
| **F05** | Gus x Iara | "a memória do vizinho" / "não era tua" | `TÉC` `⚠ GUS` |
| **F06** | Gus x Iara | trégua: "bonito não é o contrário de seguro" | `LEIGO` `TRÉGUA` `⚠ GUS` |
| **F07** | Gus x Dante | fase (a): "é a mesma ferramenta" | `TÉC` `⚠ DESLIZE` `⚠ GUS` |
| **F08** | Gus x Dante | fase (b): "eu confiro mesmo assim" | `LEIGO` `⚠ DESLIZE` `⚠ GUS` |
| **F09** | Gus x Dante | fase (c): "assim otimiza... assim assenta melhor" | `TÉC` `⚠ DESLIZE` `⚠ GUS` |
| **F10** | Gus x Dante | fase (c): "não vai doer" / "nunca doeu" / "pois é" | `MISTA` `⚠ DESLIZE` `⚠ GUS` |
| **F11** | Cauã x Iara | "usei três vezes" / "duas você errou" | `TÉC` |
| **F12** | Cauã x Iara | "ela está pensando" / "ela está CARREGANDO" | `TÉC` |
| **F13** | Cauã x Iara | o `@jit`: "essa é da minha família" | `TÉC` |
| **F14** | Cauã x Iara | o moleque de oito anos dos Dutos | `LEIGO` |
| **F15** | Cauã x Iara | "o meu tem dono" / "o meu tem quem chegar primeiro" | `TÉC` |
| **F16** | Cauã x Iara | o copo d'água quando chega | `TÉC` |
| **F17** | Jaci x Linda | contar pra frente x contar pra trás | `LEIGO` |
| **F18** | Jaci x Linda | "não monta com peça faltando" / "me diz qual faltou" | `TÉC` |
| **F19** | Jaci x Linda | "demorou" (a concessão limpa da LEI) | `TÉC` |
| **F20** | Jaci x Linda | o builtin compilado: "porque a peça compilou" | `TÉC` |
| **F21** | Jaci x Linda | "o meu exige que você saiba. Antes." | `TÉC` |
| **F22** | Jaci x Linda | trégua: "ninguém sangrando?" | `LEIGO` `TRÉGUA` |

**Itens de decisão de canon, separados das farpas** (o líder precisa decidir estes antes ou junto):

| # | Decisão |
|---|---|
| **D-01** | Ratificar (ou não) o par **Cauã x Iara** como relação canônica, com o eixo proposto na §4.4, e acrescentar a linha em `party.md:58-71`. Sem isso, F11 a F16 ficam órfãs. |
| **D-02** | Ratificar (ou não) o par **Jaci x Linda**, eixo da §4.5, mesma linha em `party.md`. Sem isso, F17 a F22 ficam órfãs. |
| **D-03** | A LEI pode ser **dita em voz alta** por um personagem (F02, F13, F20), ou só demonstrada? Se for só demonstrada, F02 cai e as outras duas viram versão muda. |
| **D-04** | X-02: ajustar a nota de payoff da Cena 7 pra nomear o eixo da vitória da Jaci (ergonomia, não execução). |
| **D-05** | X-03: acrescentar nota de leitura de uma linha no B2 do Cauã ("rodou RÁPIDO" = ficou pronto rápido, não executou rápido), pra ninguém transformar a frase em bark de combate e furar o eixo. |

---

## 6. Classe sensível: falas novas do Gus (olhar primeiro)

Regra máxima do projeto: tudo que define o Gus original passa pelo líder ([[feedback_gus_original_autorizacao_explicita]]). **10 farpas do delta contêm fala nova dele.** Nenhuma introduz eixo técnico novo: todas comprimem juízo já aprovado nos apartes, e a derivação está declarada.

| Farpa | Fala nova do Gus | Deriva de | Risco |
|---|---|---|---|
| **F02** | "Reparou que ela ficou rápida no exato segundo em que parou de ser interpretada?" | a LEI, dita em voz alta pela primeira vez | **ALTO.** É a lei do Gus original virando fala. Ver D-03. |
| F01 | "Ela está lendo o teu feitiço agora, Cauã. Enquanto ele acontece. Uma linha de cada vez." | A4 (fato fontado) | baixo |
| F03 | "E dois estavam errados." | sem fato novo | baixo |
| F04 | "Por que a tua carta pesa três vezes mais que a minha pra fazer a mesma coisa?" | A7 (fato fontado) | baixo |
| F05 | "Ela estava ali." | A17 e R-06 | baixo |
| F06 | "Legível também não." | Cena 5 (diferença permanente) | baixo |
| F07 | "É a mesma linguagem." | sem fato novo | baixo |
| F08 | "Está funcionando." | sem fato novo | baixo |
| F09 | "Assenta." | sem fato novo | baixo |
| F10 | "Nunca doeu." | sem fato novo | **MÉDIO** pelo peso dramático, não pelo conteúdo |

**Nenhuma fala nova do Gus** aparece nas farpas F11 a F22 (os dois pares sem ele), o que significa que **12 das 22 farpas têm risco zero** nessa classe.

### Como as farpas do Dante se encaixam nas três fases já canônicas

Não inventei calibragem própria. O desenho de `vozes-party.md:1131-1157` manda, e o meu encaixe é este:

| Fase canônica | O que o guia já tem | O que F07 a F10 acrescentam |
|---|---|---|
| **(a) cedo:** útil, quase caloroso, Asmódico de forma plena | 1 fala (`:1139`) | **F07.** Zero vocabulário C-Arcane, respeitando `:1205`. O deslize é semântico (troca "linguagem" por "ferramenta"), não lexical, e por isso cabe cedo sem violar a regra. |
| **(b) meio:** frieza sutil crescente, tic mais frequente | 1 fala (`:1145`) | **F08.** Mesmo registro da recusa de agradecimento que o guia já fixou: proximidade que incomoda, glance mais longo. |
| **(c) late (~75%):** o C-Arcane vaza, a voz enrijece | 2 falas (`:1151` e `:1154`) | **F09** (vazamento lexical com autocorreção tardia, o padrão exato de `:1151-1153`) e **F10** (sem vazamento lexical, o peso vem do duplo sentido da frase de manutenção que o guia já marca como repetição deliberada em `:1011-1013` e `:1049`). |

**Onde eu discordaria, e registro em vez de divergir em silêncio:** o guia proíbe vocabulário C-Arcane antes de ~75% (`:1205`), mas ele próprio observa (`:994`) que "force push" da Cena 4 já é C-Arcane e acontece bem antes. Na prática existem dois relógios. **Não mudei nada e segui a regra restritiva**, mas se o líder quiser coerência total, a saída mais barata é o guia declarar que o vazamento **lexical** começa na Cena 4 e o que muda em ~75% é a **frequência e a inconsciência** dele, que é exatamente o que a §1 já descreve em prosa. Decisão dele.

---

## 7. O que descartei do rascunho anterior, e por quê

Escrevi 45 farpas antes de fazer o inventário. Depois do mapa, **23 caíram**. Registro pra ninguém reescrevê-las achando que são lacuna:

| Descartada | Motivo |
|---|---|
| 7 farpas de **Gus x Bento** | O par tem 6 trocas fechadas, 4 apartes, 2 revides e 2 cenas. Nenhuma das minhas era melhor que o B3 ("recusa antes de errar") ou o R-04. Par declarado servido, delta zero. |
| "o meu já tinha acabado / o meu já estava escrito" | Duplica o B6 do Cauã (`:172-179`), que faz a mesma piada melhor. |
| "conserto depois" com resposta do Gus | Duplica o B2 (`:132-143`). |
| "ele avisou depois" (falha de carta) | Relitiga o R-03 (`:353`), que o Cauã venceu. Reabrir um round já julgado enfraquece os dois. |
| "ninguém precisa ME INSTALAR" e variantes | A6 mais R-03 já cobrem, e o R-03 é melhor. |
| "quantas camadas até o número" | Duplica A25 e o R-11 da Linda. |
| "ninguém me pergunta nada" como farpa própria | O A27 já traz a frase e o remate honesto ("às vezes confia demais"). |
| "o manual da tua sorte" na boca da Iara | É o R-13, e é da **Linda**. Trocar de boca quebraria a atribuição canônica. |
| farpa Gus x Iara sobre desembrulhar | Duplica A28 mais R-12, e o R-12 termina em ação, que é superior. |
| a piada final "a gente acabou de elogiar o Gus" como farpa | Vira punchline da cena de proposta C01. Manter nos dois lugares gastaria o gag. |

---

**Última revisão:** 2026-07-26. **PROPOSTA.** Promoção a canon depende da aprovação item a item da §5 e das decisões D-01 a D-05.

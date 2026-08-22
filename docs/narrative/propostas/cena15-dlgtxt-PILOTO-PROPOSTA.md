# PROPOSTA: conversão da Cena 15 para `.dlg.txt` (piloto)

> # ⚠ ISTO É PROPOSTA. NADA AQUI É CANON. NADA FOI APLICADO.
>
> **Data:** 2026-07-28. **Autor:** `narrative-writer`.
>
> **Nada foi escrito em `resources/`.** Os três `.dlg.txt` e as 65 chaves i18n estão aqui em bloco de código e em tabela. **O merge para `resources/dialogues/` e `resources/translations/` é do team-lead**, depois da aprovação.
>
> **Escopo:** **apenas a Cena 15**, como piloto de formato e de registro de narração. As Cenas 16, 17, 18 e 19 **não** foram convertidas e não devem ser até este formato ser aprovado.
>
> **Fonte:** `docs/narrative/comic-reliefs.md`, Cena 15 "Pergunta amanhã", transcrita **sem alterar nenhuma fala**. A única reescrita de texto está declarada no item DLG-09 e é de narração, não de fala.
>
> **Formato:** só aspas, zero travessão, zero em-dash.

---

## 1. As três decisões propostas

### DLG-01: o id do speaker de narração

Duas opções, e o líder escolhe. Em ambas, o **registro** da narração é `terminal` (já aprovado para logs e telas de sistema); o que muda é o **id**.

| | **Opção A: `narrador`** | **Opção B: `terminal`** |
|---|---|---|
| **A favor** | Auto-explicativo: quem abre o `.dlg.txt` entende na hora o que aquela linha é. Zero risco de leitura diegética errada. Casa com a forma-livro, em que narração é narração. | Casa o id com o registro e com uma estética que o jogo **já tem**: o terminal é voz diegética aqui (todo efeito loga no terminal, e o log de combate fala com o jogador). O jogador lê como o próprio jogo narrando, do mesmo jeito que já lê o log. Nada novo é inventado no mundo. |
| **Contra** | Inventa uma entidade fora do mundo. Se a UI imprimir o nome do speaker, aparece "Narrador" na tela, que é linguagem de livro dentro de um jogo diegético. | `speaker: terminal` ao lado de `register: terminal` parece redundância pra quem lê o arquivo. E "terminal" é nome de registro, não de falante. |
| **Risco de UI** | precisa que a UI saiba não desenhar retrato nem nome pra esse id | mesmo requisito |

**Recomendo a B (`terminal`)**, pelo argumento diegético: o jogo já ensinou o jogador a receber informação do terminal, e a narração entra nesse canal sem custo de aprendizado. Se o líder achar o arquivo confuso de ler, a A resolve isso e não custa mais nada.

**Os três arquivos abaixo estão escritos com a Opção B.** Trocar para A é achar e substituir um token, sem nenhum outro efeito.

### DLG-02: um arquivo ou três. **Três, e não é preferência, é o formato que obriga**

O parser (`dialogue_text.hpp`) tem `#meta entry`, nós lineares, nós de escolha, flags e `@exit`. **Não tem espera, não tem retomada e não tem entrada condicional:** um grafo entra pelo `entry` e corre até o `@exit`. Um único arquivo com os três movimentos **tocaria os três de uma vez**, e o intervalo em tempo de campanha é justamente o miolo da cena. Um arquivo destruiria o beat.

Então: **três arquivos**, um por movimento, cada um disparado pelo seu próprio gatilho.

| Arquivo | Movimento | Gatilho |
|---|---|---|
| `cena15_m1_pergunta.dlg.txt` | 1, a pergunta | entrar no bloco em reconstrução, durante a mini-quest posterior do Cauã |
| `cena15_m2_intervalo.dlg.txt` | 2, o intervalo | falar com o Bento na Catedral, com a flag do M1 escrita |
| `cena15_m3_coda.dlg.txt` | 3, a coda | voltar ao caixote depois do payoff em combate |

**E um quarto pedaço que NÃO é `.dlg.txt`:** o payoff em combate do Movimento 3 ("Compila e roda." / "Essa aqui compilou mesmo.") são **duas barks**, não um grafo de diálogo. Abrir caixa de diálogo no meio de um combate pra duas linhas seria pior em todos os aspectos, e o próprio canon já diz que barks cabem no sistema atual sem adaptação. Ver §2.

**Bônus de engenharia, e é de graça:** as duas flags abaixo **já codificam os três estados do bordão** (item `CAUA-BORDAO-3-ESTADOS` do TODO), sem nenhuma máquina de estado nova:

| Estado do bordão | Condição |
|---|---|
| antes | `!cena15.m1_done` |
| durante (o bordão sai do pool) | `cena15.m1_done && !cena15.m3_done` |
| depois (volta, mais a variante literal) | `cena15.m3_done` |

### DLG-03: convenção de chave i18n

A convenção **já existe implícita** no único diálogo do repo: `DIALOGUE_NPC_INTRO_N0_GREET` vem de `dialogue_id = npc_intro` mais `node_id = n0_greet`. Proponho apenas **tornar a regra explícita e mecânica**, sem inventar padrão novo:

> **`DIALOGUE_` + `<dialogue_id em MAIÚSCULAS>` + `_` + `<node_id em MAIÚSCULAS>`**

Aplicada aqui: `dialogue_id = cena15_m1` mais `node_id = n07_leitura` dá `DIALOGUE_CENA15_M1_N07_LEITURA`.

Três vantagens: é derivável do arquivo sem tabela auxiliar, é **verificável por script** (dá pra escrever um gate que confere chave contra nó, no mesmo espírito do gate de paridade), e mantém o mnemônico no fim, que é o que ajuda quem traduz.

Numeração dos nós com **dois dígitos** (`n01`, não `n1`), porque estas cenas passam de dez nós e a ordenação alfabética quebraria.

---

## 2. O que NÃO converte, e por quê

**1. O par de barks de combate (Movimento 3).** São duas linhas do Cauã no meio de uma luta. Vão pro sistema de barks, não pro runtime de diálogo. **Elas continuam precisando de chave i18n**, e eu não sei qual convenção o sistema de barks adotou, então deixei as duas fora da minha contagem e sinalizo como pendência (DLG-10):

> "Compila e roda." / "Essa aqui compilou mesmo."
> EN: "Compile and run." / "This one actually compiled."

**2. O beat mudo da Linda, no intervalo.** É a única parte da cena que eu recomendo **não converter de jeito nenhum**. Ela não tem fala: é ela ajustando o fone e olhando um segundo a mais, sem dizer nada. Transformar isso em linha de narração seria o jogo **apontando** para o sinal que a cena inteira foi construída pra não apontar, e a regra de ferro da Cena 15 é "ninguém comenta". Fica como nota de encenação para quem fizer a animação de combate, ou não acontece. **Perder isso é melhor que narrá-lo.**

---

## 3. Os três arquivos

### 3.1 `cena15_m1_pergunta.dlg.txt`

```
// cena15_m1_pergunta.dlg.txt
//
// Cena 15 "Pergunta amanha", Movimento 1: a pergunta.
// Fonte canonica: docs/narrative/comic-reliefs.md, Cena 15 (nenhuma fala alterada).
// Forma-jogo da cena; a forma-livro continua sendo a prosa do comic-reliefs.md
// (comic-reliefs.md, "Como estas cenas chegam ao jogador").
//
// REGISTRO: warm nas falas (humanos, caixa quente + retrato); terminal na
// narracao (mesmo registro dos logs e telas de sistema).
//
// GATILHO: entrar no bloco em reconstrucao durante a mini-quest posterior do
// Caua (characters/caua-volt.md:65-67).
//
// FLAG ESCRITA: cena15.m1_done (no ultimo no). Junto com cena15.m3_done, codifica
// os tres estados do bordao (CAUA-BORDAO-3-ESTADOS).
//
// REGRA DE FERRO DA CENA: ninguem comenta. A mao parada do Caua no ultimo no e a
// unica marca da ausencia do tic, e nenhum personagem repara nela.

#meta dialogue_id cena15_m1
#meta default_register warm
#meta entry n01_abertura

@node n01_abertura
speaker: terminal
text: DIALOGUE_CENA15_M1_N01_ABERTURA
register: terminal
-> n02_escreve

@node n02_escreve
speaker: caua
text: DIALOGUE_CENA15_M1_N02_ESCREVE
-> n03_errado

@node n03_errado
speaker: moleque
text: DIALOGUE_CENA15_M1_N03_ERRADO
-> n04_perdoa

@node n04_perdoa
speaker: caua
text: DIALOGUE_CENA15_M1_N04_PERDOA
-> n05_jaescreve

@node n05_jaescreve
speaker: terminal
text: DIALOGUE_CENA15_M1_N05_JAESCREVE
register: terminal
-> n06_pronto

@node n06_pronto
speaker: moleque
text: DIALOGUE_CENA15_M1_N06_PRONTO
-> n07_leitura

@node n07_leitura
speaker: terminal
text: DIALOGUE_CENA15_M1_N07_LEITURA
register: terminal
-> n08_ligatudo

@node n08_ligatudo
speaker: caua
text: DIALOGUE_CENA15_M1_N08_LIGATUDO
-> n09_equeeuquero

@node n09_equeeuquero
speaker: moleque
text: DIALOGUE_CENA15_M1_N09_EQUEEUQUERO
-> n10_tudooque

@node n10_tudooque
speaker: caua
text: DIALOGUE_CENA15_M1_N10_TUDOOQUE
-> n11_tudo

@node n11_tudo
speaker: moleque
text: DIALOGUE_CENA15_M1_N11_TUDO
-> n12_apaga

@node n12_apaga
speaker: caua
text: DIALOGUE_CENA15_M1_N12_APAGA
-> n13_riem

@node n13_riem
speaker: terminal
text: DIALOGUE_CENA15_M1_N13_RIEM
register: terminal
-> n14_consertar

@node n14_consertar
speaker: moleque
text: DIALOGUE_CENA15_M1_N14_CONSERTAR
-> n15_mechama

@node n15_mechama
speaker: caua
text: DIALOGUE_CENA15_M1_N15_MECHAMA
-> n16_naoestiver

@node n16_naoestiver
speaker: moleque
text: DIALOGUE_CENA15_M1_N16_NAOESTIVER
-> n17_olhabloco

@node n17_olhabloco
speaker: terminal
text: DIALOGUE_CENA15_M1_N17_OLHABLOCO
register: terminal
-> n18_outro

@node n18_outro
speaker: caua
text: DIALOGUE_CENA15_M1_N18_OUTRO
-> n19_aceita

@node n19_aceita
speaker: terminal
text: DIALOGUE_CENA15_M1_N19_ACEITA
register: terminal
-> n20_caua

@node n20_caua
speaker: moleque
text: DIALOGUE_CENA15_M1_N20_CAUA
-> n21_oi

@node n21_oi
speaker: caua
text: DIALOGUE_CENA15_M1_N21_OI
-> n22_compilar

@node n22_compilar
speaker: moleque
text: DIALOGUE_CENA15_M1_N22_COMPILAR
-> n23_silencio

@node n23_silencio
speaker: terminal
text: DIALOGUE_CENA15_M1_N23_SILENCIO
register: terminal
-> n24_equando

@node n24_equando
speaker: caua
text: DIALOGUE_CENA15_M1_N24_EQUANDO
-> n25_naocontinua

@node n25_naocontinua
speaker: terminal
text: DIALOGUE_CENA15_M1_N25_NAOCONTINUA
register: terminal
-> n26_amanha

@node n26_amanha
speaker: caua
text: DIALOGUE_CENA15_M1_N26_AMANHA
-> n27_saida

@node n27_saida
speaker: terminal
text: DIALOGUE_CENA15_M1_N27_SAIDA
register: terminal
on_enter: cena15.m1_done=true
-> @exit
```

### 3.2 `cena15_m2_intervalo.dlg.txt`

```
// cena15_m2_intervalo.dlg.txt
//
// Cena 15 "Pergunta amanha", Movimento 2: o intervalo.
// Fonte canonica: docs/narrative/comic-reliefs.md, Cena 15 (nenhuma fala alterada).
//
// GATILHO: falar com o Bento na Catedral de Neo-Sylvania com cena15.m1_done escrita.
//
// FLAG ESCRITA: cena15.m2_done (no ultimo no).
//
// NOTA DE ENCENACAO (nao vira no): durante o intervalo que comeca aqui, o bordao
// "compila e roda" sai do pool de barks do Caua, junto com o tic de estalar os
// dedos por empolgacao. O beat mudo da Linda (ela ajusta o fone e olha um segundo
// a mais, sem falar) NAO foi convertido de proposito: narrar seria apontar, e a
// regra de ferro da cena e que ninguem comenta.
//
// O ECO: a Catedral devolve cada som meio segundo depois. Nos nos n20 e n32 o eco
// e narrado, e o avanco da caixa faz o atraso acontecer de verdade na mao do
// jogador.

#meta dialogue_id cena15_m2
#meta default_register warm
#meta entry n01_catedral

@node n01_catedral
speaker: terminal
text: DIALOGUE_CENA15_M2_N01_CATEDRAL
register: terminal
-> n02_mosaico

@node n02_mosaico
speaker: bento
text: DIALOGUE_CENA15_M2_N02_MOSAICO
-> n03_chaointeiro

@node n03_chaointeiro
speaker: caua
text: DIALOGUE_CENA15_M2_N03_CHAOINTEIRO
-> n04_tem

@node n04_tem
speaker: bento
text: DIALOGUE_CENA15_M2_N04_TEM
-> n05_episando

@node n05_episando
speaker: bento
text: DIALOGUE_CENA15_M2_N05_EPISANDO
-> n06_pes

@node n06_pes
speaker: terminal
text: DIALOGUE_CENA15_M2_N06_PES
register: terminal
-> n07_perguntar

@node n07_perguntar
speaker: caua
text: DIALOGUE_CENA15_M2_N07_PERGUNTAR
-> n08_pergunta

@node n08_pergunta
speaker: bento
text: DIALOGUE_CENA15_M2_N08_PERGUNTA
-> n09_naopergunta

@node n09_naopergunta
speaker: terminal
text: DIALOGUE_CENA15_M2_N09_NAOPERGUNTA
register: terminal
-> n10_moleque

@node n10_moleque
speaker: caua
text: DIALOGUE_CENA15_M2_N10_MOLEQUE
-> n11_e

@node n11_e
speaker: bento
text: DIALOGUE_CENA15_M2_N11_E
-> n12_amanha

@node n12_amanha
speaker: caua
text: DIALOGUE_CENA15_M2_N12_AMANHA
-> n13_amanhaehoje

@node n13_amanhaehoje
speaker: caua
text: DIALOGUE_CENA15_M2_N13_AMANHAEHOJE
-> n14_limpa

@node n14_limpa
speaker: terminal
text: DIALOGUE_CENA15_M2_N14_LIMPA
register: terminal
-> n15_nemcompila

@node n15_nemcompila
speaker: caua
text: DIALOGUE_CENA15_M2_N15_NEMCOMPILA
-> n16_falei

@node n16_falei
speaker: bento
text: DIALOGUE_CENA15_M2_N16_FALEI
-> n17_zoando

@node n17_zoando
speaker: caua
text: DIALOGUE_CENA15_M2_N17_ZOANDO
-> n18_para

@node n18_para
speaker: terminal
text: DIALOGUE_CENA15_M2_N18_PARA
register: terminal
-> n19_nao

@node n19_nao
speaker: bento
text: DIALOGUE_CENA15_M2_N19_NAO
-> n20_eco

@node n20_eco
speaker: terminal
text: DIALOGUE_CENA15_M2_N20_ECO
register: terminal
-> n21_silencio

@node n21_silencio
speaker: terminal
text: DIALOGUE_CENA15_M2_N21_SILENCIO
register: terminal
-> n22_palavra

@node n22_palavra
speaker: bento
text: DIALOGUE_CENA15_M2_N22_PALAVRA
-> n23_promessa

@node n23_promessa
speaker: bento
text: DIALOGUE_CENA15_M2_N23_PROMESSA
-> n24_senta

@node n24_senta
speaker: terminal
text: DIALOGUE_CENA15_M2_N24_SENTA
register: terminal
-> n25_promessadeque

@node n25_promessadeque
speaker: caua
text: DIALOGUE_CENA15_M2_N25_PROMESSADEQUE
-> n26_verdade

@node n26_verdade
speaker: bento
text: DIALOGUE_CENA15_M2_N26_VERDADE
-> n27_naodecido

@node n27_naodecido
speaker: bento
text: DIALOGUE_CENA15_M2_N27_NAODECIDO
-> n28_levanta

@node n28_levanta
speaker: terminal
text: DIALOGUE_CENA15_M2_N28_LEVANTA
register: terminal
-> n29_bento

@node n29_bento
speaker: caua
text: DIALOGUE_CENA15_M2_N29_BENTO
-> n30_garoto

@node n30_garoto
speaker: bento
text: DIALOGUE_CENA15_M2_N30_GAROTO
-> n31_obrigado

@node n31_obrigado
speaker: caua
text: DIALOGUE_CENA15_M2_N31_OBRIGADO
-> n32_fecho

@node n32_fecho
speaker: terminal
text: DIALOGUE_CENA15_M2_N32_FECHO
register: terminal
on_enter: cena15.m2_done=true
-> @exit
```

### 3.3 `cena15_m3_coda.dlg.txt`

```
// cena15_m3_coda.dlg.txt
//
// Cena 15 "Pergunta amanha", Movimento 3: a coda.
// Fonte canonica: docs/narrative/comic-reliefs.md, Cena 15 (nenhuma fala alterada).
//
// GATILHO: voltar ao caixote nos Dutos DEPOIS do payoff em combate. O payoff em si
// ("Compila e roda." / "Essa aqui compilou mesmo.") sao DUAS BARKS de combate, nao
// entram neste grafo: abrir caixa de dialogo no meio da luta por duas linhas seria
// pior em tudo.
//
// FLAG ESCRITA: cena15.m3_done (no ultimo no). Devolve o bordao ao pool de barks e
// habilita a variante literal.
//
// A resposta a pergunta do moleque NUNCA e mostrada. A cena nao e sobre a resposta.

#meta dialogue_id cena15_m3
#meta default_register warm
#meta entry n01_caixote

@node n01_caixote
speaker: terminal
text: DIALOGUE_CENA15_M3_N01_CAIXOTE
register: terminal
-> n02_falou

@node n02_falou
speaker: moleque
text: DIALOGUE_CENA15_M3_N02_FALOU
-> n03_falei

@node n03_falei
speaker: caua
text: DIALOGUE_CENA15_M3_N03_FALEI
-> n04_eamanha

@node n04_eamanha
speaker: moleque
text: DIALOGUE_CENA15_M3_N04_EAMANHA
-> n05_senta

@node n05_senta
speaker: terminal
text: DIALOGUE_CENA15_M3_N05_SENTA
register: terminal
-> n06_elongo

@node n06_elongo
speaker: caua
text: DIALOGUE_CENA15_M3_N06_ELONGO
on_enter: cena15.m3_done=true
-> @exit
```

---

## 4. Chaves i18n (pt-br e en-intl)

**65 chaves novas**, todas entrando nos **dois** catálogos. Ver §6 para a contagem do gate.

### 4.1 `cena15_m1` (27 chaves)

| Chave | pt-br | en-intl |
|---|---|---|
| `DIALOGUE_CENA15_M1_N01_ABERTURA` | Os Dutos Infernais, no bloco em reconstrução. Andaime improvisado, cabo novo passado por cima do cabo queimado. Cauã ensina cinco moleques sentados em caixotes, e o menor deles não desgruda. | The Infernal Ducts, in the block being rebuilt. Makeshift scaffolding, new cable run over the burnt one. Cauã is teaching five kids sitting on crates, and the smallest one won't leave his side. |
| `DIALOGUE_CENA15_M1_N02_ESCREVE` | Escreve o passo. Fala o que você quer. Ela entende. | Write the step. Say what you want. She gets it. |
| `DIALOGUE_CENA15_M1_N03_ERRADO` | E se eu escrever errado? | And if I write it wrong? |
| `DIALOGUE_CENA15_M1_N04_PERDOA` | Ela perdoa. Você conserta depois. | She forgives. You fix it later. |
| `DIALOGUE_CENA15_M1_N05_JAESCREVE` | O menor escreve antes de o Cauã terminar de falar. | The smallest one starts writing before Cauã finishes talking. |
| `DIALOGUE_CENA15_M1_N06_PRONTO` | Pronto. | Done. |
| `DIALOGUE_CENA15_M1_N07_LEITURA` | Cauã lê por cima do ombro dele. | Cauã reads over his shoulder. |
| `DIALOGUE_CENA15_M1_N08_LIGATUDO` | Você escreveu "liga tudo". | You wrote "turn everything on". |
| `DIALOGUE_CENA15_M1_N09_EQUEEUQUERO` | É o que eu quero. | That's what I want. |
| `DIALOGUE_CENA15_M1_N10_TUDOOQUE` | Tudo o quê? | Everything what? |
| `DIALOGUE_CENA15_M1_N11_TUDO` | Tudo. | Everything. |
| `DIALOGUE_CENA15_M1_N12_APAGA` | Apaga o "tudo". | Delete the "everything". |
| `DIALOGUE_CENA15_M1_N13_RIEM` | Os outros moleques riem. O menor não acha graça nenhuma, e apaga com muita seriedade. | The other kids laugh. The smallest one doesn't find it funny at all, and erases it with great seriousness. |
| `DIALOGUE_CENA15_M1_N14_CONSERTAR` | E se eu não souber consertar? | And if I don't know how to fix it? |
| `DIALOGUE_CENA15_M1_N15_MECHAMA` | Aí você me chama. | Then you call me. |
| `DIALOGUE_CENA15_M1_N16_NAOESTIVER` | E se você não estiver? | And if you're not there? |
| `DIALOGUE_CENA15_M1_N17_OLHABLOCO` | Cauã para. Olha pro moleque. Depois olha pro bloco em volta, que até duas semanas atrás tinha mais gente sentada nele. | Cauã stops. He looks at the kid. Then he looks at the block around them, which two weeks ago had more people sitting in it. |
| `DIALOGUE_CENA15_M1_N18_OUTRO` | Aí você chama outro. Sempre tem outro. | Then you call someone else. There's always someone else. |
| `DIALOGUE_CENA15_M1_N19_ACEITA` | O moleque aceita a resposta na hora, do jeito que só quem tem oito anos aceita. Volta a escrever. Cauã solta o ar. | The kid accepts the answer right away, the way only an eight-year-old can. He goes back to writing. Cauã lets out his breath. |
| `DIALOGUE_CENA15_M1_N20_CAUA` | Cauã. | Cauã. |
| `DIALOGUE_CENA15_M1_N21_OI` | Oi. | Yeah. |
| `DIALOGUE_CENA15_M1_N22_COMPILAR` | O que é compilar? | What does compile mean? |
| `DIALOGUE_CENA15_M1_N23_SILENCIO` | Silêncio. Um dos moleques maiores levanta a cabeça, curioso, porque também nunca perguntou. | Silence. One of the older kids looks up, curious, because he never asked either. |
| `DIALOGUE_CENA15_M1_N24_EQUANDO` | É quando... | It's when... |
| `DIALOGUE_CENA15_M1_N25_NAOCONTINUA` | A frase não continua. | The sentence doesn't continue. |
| `DIALOGUE_CENA15_M1_N26_AMANHA` | Pergunta amanhã. | Ask me tomorrow. |
| `DIALOGUE_CENA15_M1_N27_SAIDA` | Ele guarda o estilete e vai embora devagar. A mão dele fica parada o caminho inteiro. Nenhum dos moleques acha isso estranho: o Cauã sempre volta. | He puts the stylus away and leaves slowly. His hand stays still the whole way. None of the kids find that strange: Cauã always comes back. |

### 4.2 `cena15_m2` (32 chaves)

| Chave | pt-br | en-intl |
|---|---|---|
| `DIALOGUE_CENA15_M2_N01_CATEDRAL` | A Catedral de Neo-Sylvania, fim de tarde. O eco devolve cada passo com meio segundo de atraso. Cauã entra e odeia o lugar imediatamente. Bento está sentado num degrau, limpando uma engrenagem do escudo com um pano, e não levanta a cabeça. | The Cathedral of Neo-Sylvania, late afternoon. The echo gives every footstep back half a second later. Cauã walks in and hates the place immediately. Bento is sitting on a step, cleaning one of the shield's gears with a rag, and doesn't look up. |
| `DIALOGUE_CENA15_M2_N02_MOSAICO` | Você está pisando no mosaico. | You are standing on the mosaic. |
| `DIALOGUE_CENA15_M2_N03_CHAOINTEIRO` | Tem mosaico no chão inteiro. | There's mosaic on the whole floor. |
| `DIALOGUE_CENA15_M2_N04_TEM` | Tem. | There is. |
| `DIALOGUE_CENA15_M2_N05_EPISANDO` | E você está pisando nele. | And you are standing on it. |
| `DIALOGUE_CENA15_M2_N06_PES` | Cauã olha pros próprios pés. Dá um passo pro lado. Não melhora nada. Desiste. | Cauã looks down at his own feet. He takes a step to the side. It doesn't help. He gives up. |
| `DIALOGUE_CENA15_M2_N07_PERGUNTAR` | Preciso perguntar uma parada. | I gotta ask you something. |
| `DIALOGUE_CENA15_M2_N08_PERGUNTA` | Pergunta. | Ask. |
| `DIALOGUE_CENA15_M2_N09_NAOPERGUNTA` | Cauã não pergunta. Bento continua limpando a engrenagem e não ajuda. Passam uns bons segundos, e o eco não devolve nada, porque ninguém falou. | Cauã doesn't ask. Bento keeps cleaning the gear and doesn't help. A good few seconds go by, and the echo gives nothing back, because nobody spoke. |
| `DIALOGUE_CENA15_M2_N10_MOLEQUE` | Um moleque me perguntou uma coisa ontem. | A kid asked me something yesterday. |
| `DIALOGUE_CENA15_M2_N11_E` | E? | And? |
| `DIALOGUE_CENA15_M2_N12_AMANHA` | E eu falei pra ele perguntar amanhã. | And I told him to ask me tomorrow. |
| `DIALOGUE_CENA15_M2_N13_AMANHAEHOJE` | Amanhã é hoje. | Tomorrow is today. |
| `DIALOGUE_CENA15_M2_N14_LIMPA` | Bento limpa a engrenagem. | Bento cleans the gear. |
| `DIALOGUE_CENA15_M2_N15_NEMCOMPILA` | Você falou que a minha nem compila. | You said mine doesn't even compile. |
| `DIALOGUE_CENA15_M2_N16_FALEI` | Falei. | I did. |
| `DIALOGUE_CENA15_M2_N17_ZOANDO` | Você tava zoando? | Were you messing with me? |
| `DIALOGUE_CENA15_M2_N18_PARA` | Bento para de limpar. Põe o pano no colo. | Bento stops cleaning. He sets the rag down in his lap. |
| `DIALOGUE_CENA15_M2_N19_NAO` | Não. | No. |
| `DIALOGUE_CENA15_M2_N20_ECO` | O eco devolve o não meio segundo depois. Cauã escuta duas vezes. | The echo gives the no back half a second later. Cauã hears it twice. |
| `DIALOGUE_CENA15_M2_N21_SILENCIO` | Bento não acrescenta nada, e a pausa passa do ponto em que seria confortável acrescentar. | Bento doesn't add anything, and the pause goes past the point where adding something would still be comfortable. |
| `DIALOGUE_CENA15_M2_N22_PALAVRA` | Mas você diz a palavra todo dia. | But you say the word every day. |
| `DIALOGUE_CENA15_M2_N23_PROMESSA` | Isso não é mentira, garoto. É promessa. | That is not a lie, lad. It is a promise. |
| `DIALOGUE_CENA15_M2_N24_SENTA` | Cauã senta no degrau, dois abaixo dele. Não pede licença. Bento não reclama. | Cauã sits on the step, two below him. He doesn't ask permission. Bento doesn't complain. |
| `DIALOGUE_CENA15_M2_N25_PROMESSADEQUE` | Promessa de quê? | A promise of what? |
| `DIALOGUE_CENA15_M2_N26_VERDADE` | De que um dia a palavra vai ser verdade na tua boca. | That one day the word will be true in your mouth. |
| `DIALOGUE_CENA15_M2_N27_NAODECIDO` | Não sou eu que decido quando. | That is not mine to decide. |
| `DIALOGUE_CENA15_M2_N28_LEVANTA` | Cauã fica com isso um tempo. Depois levanta e vai embora. No meio da nave, para. | Cauã sits with that for a while. Then he gets up and leaves. Halfway down the nave, he stops. |
| `DIALOGUE_CENA15_M2_N29_BENTO` | Bento. | Bento. |
| `DIALOGUE_CENA15_M2_N30_GAROTO` | Garoto. | Lad. |
| `DIALOGUE_CENA15_M2_N31_OBRIGADO` | Obrigado. | Thank you. |
| `DIALOGUE_CENA15_M2_N32_FECHO` | Bento não responde. Espera o eco devolver a palavra e só então volta a limpar a engrenagem. | Bento doesn't answer. He waits for the echo to give the word back, and only then goes back to cleaning the gear. |

### 4.3 `cena15_m3` (6 chaves)

| Chave | pt-br | en-intl |
|---|---|---|
| `DIALOGUE_CENA15_M3_N01_CAIXOTE` | Os Dutos Infernais, depois. O moleque está sentado no mesmo caixote, com o mesmo estilete. | The Infernal Ducts, later. The kid is sitting on the same crate, with the same stylus. |
| `DIALOGUE_CENA15_M3_N02_FALOU` | Você falou pra eu perguntar amanhã. | You told me to ask you tomorrow. |
| `DIALOGUE_CENA15_M3_N03_FALEI` | Falei. | I did. |
| `DIALOGUE_CENA15_M3_N04_EAMANHA` | É amanhã. | It's tomorrow. |
| `DIALOGUE_CENA15_M3_N05_SENTA` | Cauã senta ao lado dele no caixote. | Cauã sits down next to him on the crate. |
| `DIALOGUE_CENA15_M3_N06_ELONGO` | Então senta direito, que é longo. | Then sit properly, because this takes a while. |

### 4.4 Triagem da §11 do guia de diálogos (trocadilho e jargão)

Rodei a triagem **uma a uma**, e o resultado é limpo:

- **Nenhuma fala precisa de versão própria em inglês.** O motor da cena inteira é a palavra "compilar", e o par `compilar` / `compile` sobrevive intacto na passagem, junto com "compila e roda" / "compile and run", que **já é canon aprovado** (`vozes-party.md:212`). Este é exatamente o caso 1 da §11: o par sobrevive, então traduz normal e não se inventa versão nova.
- **Três traduções foram por EFEITO e não literais**, e declaro cada uma:
  1. `N27_NAODECIDO`: "Não sou eu que decido quando" virou **"That is not mine to decide."** A literal ("It's not me who decides when") é truncada em inglês e perde a gravidade. O efeito é o mesmo: ele recusa a autoridade sobre o prazo.
  2. `M1_N21_OI`: "Oi" virou **"Yeah."** "Hi" seria cumprimento, e aqui é um moleque respondendo a quem já está do lado dele.
  3. `M2_N07_PERGUNTAR`: "uma parada" é gíria e não tem par literal. **"I gotta ask you something"** preserva o registro contraído do Cauã, que o guia dele exige.
- **Duas pendências que não são minhas** (DLG-12): o nome en-intl de **Dutos Infernais** (usei "Infernal Ducts" como proposta) e a confirmação de **Neo-Sylvania** como nome próprio que não traduz. Nome de lugar é decisão de canon, não de tradutor.

---

## 5. Onde a conversão ganhou, e onde perdeu

**Ganhou em dois pontos, e um deles foi surpresa:**

1. **O avanço da caixa É a pausa.** Todo `[pausa]` da prosa vira, de graça, o intervalo entre o jogador ler e apertar o botão. A cena tem pausa marcada em nove lugares, e nenhum deles precisou de texto.
2. **O eco de meio segundo ficou melhor no jogo do que na prosa.** No papel, "o eco devolve o não meio segundo depois" é uma frase que o leitor processa junto com o "Não.". No jogo, o jogador lê "Não.", **aperta o botão**, e só então recebe a frase do eco. O atraso acontece de verdade, na mão dele, com a duração que ele mesmo deu. Isso é o meio fazendo o trabalho que a prosa só descreve.

**Perdeu em três, e dois são recuperáveis:**

1. **O beat mudo da Linda não converte** (§2). É perda real e eu recomendo aceitá-la: narrar seria apontar. **Recuperável fora do diálogo**, se a animação de combate quiser mostrar o olhar.
2. **A simultaneidade some.** Na prosa, "Ele guarda o estilete. Vai embora devagar. Ele não estala os dedos." acontece num bloco só, e o leitor recebe as três coisas juntas. Numa caixa de diálogo elas viram uma frase sequencial. Consolidei o bloco num nó só pra minimizar, mas o efeito de simultaneidade não existe no formato.
3. **A ausência do tic vira uma frase lida**, e é o ponto mais delicado da conversão inteira. Ver DLG-09 logo abaixo.

### DLG-09: a única reescrita de texto que fiz, e ela precisa de decisão

A prosa diz, como rubrica: **"[Ele não estala os dedos.]"**

Numa caixa de diálogo, uma negação assim é uma **frase que o jogo escreve na tela pro jogador ler**, e negação chama atenção pra exatamente aquilo que ela nega. A cena inteira foi construída sobre não apontar.

Escrevi, na narração: **"A mão dele fica parada o caminho inteiro."**

Mesma informação, e o tic não acontece sem que ninguém diga o nome do tic. **Alternativa, se o líder preferir fidelidade literal:** trocar por "Ele não estala os dedos.", que é a rubrica original. As duas cabem no mesmo nó e nada mais muda.

---

## 6. Gate de paridade i18n: a contagem

O gate (`tools/i18n_parity.py`, via `tools/check.sh`) compara os dois catálogos. Hoje há **19 chaves `DIALOGUE_` em cada**.

| | antes | novas | depois |
|---|---|---|---|
| `pt_br.md` | 19 | +65 | **84** |
| `en_intl.md` | 19 | +65 | **84** |

**As 65 entram nos dois arquivos, na mesma ordem, no mesmo commit.** Sugiro uma sub-seção própria no catálogo (`### Cena 15 (comic-reliefs)`) nos dois, pra o bloco ficar localizável quando as outras quatro cenas vierem.

**As duas barks de combate (DLG-10) NÃO estão nessa contagem**, porque não sei em que catálogo o sistema de barks vive. Se elas forem para o mesmo catálogo, o total vira 86 em cada, e continua empatado.

---

## 7. Lista numerada para aprovação item a item

| # | Item | Natureza |
|---|---|---|
| **DLG-01** | Id do speaker de narração: **Opção A `narrador`** ou **Opção B `terminal`** (recomendo B) | decisão de formato, afeta todas as cenas futuras |
| **DLG-02** | **Três arquivos**, um por movimento, mais o payoff de combate como bark | decisão de formato (o parser obriga; sem espera nem retomada num grafo) |
| **DLG-03** | Convenção de chave: `DIALOGUE_` + dialogue_id + node_id, tudo em maiúsculas, nós com dois dígitos | decisão de formato, afeta todas as cenas futuras |
| **DLG-04** | As três flags `cena15.m1_done`, `cena15.m2_done`, `cena15.m3_done`, que já codificam os três estados do bordão sem máquina nova | decisão técnica |
| **DLG-05** | `cena15_m1_pergunta.dlg.txt` inteiro | arquivo |
| **DLG-06** | `cena15_m2_intervalo.dlg.txt` inteiro | arquivo |
| **DLG-07** | `cena15_m3_coda.dlg.txt` inteiro | arquivo |
| **DLG-08** | As **65 chaves i18n**, pt-br e en-intl, nos dois catálogos | conteúdo |
| **DLG-09** | **"A mão dele fica parada o caminho inteiro."** no lugar da rubrica "Ele não estala os dedos." (alternativa: manter a rubrica literal) | **única reescrita de texto da conversão** |
| **DLG-10** | As duas barks de combate ficam **fora** do `.dlg.txt` e precisam de chave na convenção do sistema de barks | pendência de outro sistema |
| **DLG-11** | O beat mudo da Linda **não converte** e vira nota de encenação | perda declarada |
| **DLG-12** | Nome en-intl de **Dutos Infernais** ("Infernal Ducts", proposta) e confirmação de **Neo-Sylvania** como nome próprio | decisão de canon de nomes |

**Classe sensível: nenhum item.** A Cena 15 não tem nenhuma fala do Gus, então esta conversão inteira tem risco zero nessa classe. **É por isso que ela é o piloto certo.**

---

**Última revisão:** 2026-07-28. **PROPOSTA.** Nada aplicado, nada canonizado. As Cenas 16, 17, 18 e 19 **não** foram convertidas.

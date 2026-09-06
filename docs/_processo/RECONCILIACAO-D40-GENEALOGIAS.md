<!--
SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
SPDX-License-Identifier: AGPL-3.0-or-later
-->

> **Escrito em 06/09/2026, sob o item `D40` do `TODO.md`, reprovado duas vezes: a primeira por não cruzar as três cadeias genealógicas que o líder confirmou em 01/09/2026 serem objetos distintos (institucional, descendência de sangue, matrilinear canônica); a segunda porque os achados dessa reconferência foram registrados em quatro linhas soltas da fila de exceção do `TODO.md` (drenadas ali entre os commits `2ee4405` e `447670b`, mais uma quarta linha independente), sem arquivo nem linha, e o relatório que o item exige nunca foi escrito.** Este documento fecha essa lacuna: cada achado abaixo carrega arquivo e linha, e nenhum foi corrigido aqui — correção é item próprio, aberto depois, como a linha do `D40` já registra (L-33 do projeto).
>
> **Convenção desta reconferência:** cada achado é reportado com [FATO] (o que o `grep`/a leitura confirma, com arquivo:linha) separado de [INFERÊNCIA] (a causa apurada ou a leitura editorial, quando não é fato bruto) — L-18 global. Onde a premissa recebida da fila de exceção não pôde ser confirmada byte a byte contra o corpus atual, isto é dito explicitamente, não silenciado.

---

# Reconciliação D40: quatro achados de genealogia, com arquivo e linha

## 1. Contagens consolidadas

| Achado | Documento(s)-fonte | Números em conflito | Status |
|---|---|---|---|
| 1. Cadeia institucional | `era-1.../capitulo-10...md`:25 e :1791, `CHARS.md`:230 | dez vs. "oito" com rótulo oitava/nona incoerente | VIVO, canon do líder |
| 2. Sucessão matrilinear (cronistas Atelaiá) | `era-2-boom-tecnico.md`:1445 vs. `CHARS.md`:228 e :719 | cinco (pós-Atelaiá) vs. seis (incluindo Atelaiá) | Convenção, não erro — mas cruzam mal |
| 3. Linhagem de sangue da narradora | `capitulo-10...md`:1791 | "oito gerações" declaradas vs. nove nomes enumerados na mesma frase | VIVO, causa apurada |
| 4. Árvore Vanderbist (Jaci) | `docs/narrative/deep/characters/jaci-proxy.md`:9 | oito gerações vs. "bisavó da bisavó" (grau de parentesco muito menor) | VIVO, contradição interna |

Nenhum dos quatro foi corrigido nesta rodada. **Achado 3 do briefing original ("linhagem do protagonista, já corrigida") foi reclassificado**: a busca não achou nenhuma correção já aplicada a uma linhagem de Gus Vector Tavus Vance (a cadeia Gustaf I-VII está consistente em "sete gerações" em todas as sete fontes verificadas — ver §3 abaixo) nem a "oito nomes" concordando com glossário e índice em lugar nenhum do corpus. O achado que de fato bate com o padrão descrito ("diz um número dez vezes, lista outro") é a passagem da própria cronista sobre sua linhagem de sangue, em `capitulo-10...md`:1791, que **segue viva**, não corrigida. Isto é dito aqui explicitamente porque a premissa recebida não resistiu à verificação (L-18).

---

## 2. Achado 1 — Cadeia institucional: dez nomes num lugar, "oito" mal rotulado noutro

[FATO] `docs/narrative/deep/eras/era-1-pre-codigo/capitulo-10-trecho-in-character-cronista-era-3.md:25`:
> "A cadeia institucional segue em sequência canônica: Atelaiá Chevalier (-115); Esmeralda Argéndia-Chevalier; Luísa Argéndia-Chevalier; Anastácia Vyrcátrix-Acaceiro a Paciente; Hipátia Atelaiá-Vargas; Olímpia Cardoso-Acaceiro; Cassandra Yvanova-Calígrafa; Verônica Atelaiá (transicional); Cassandra "Bento" Chevalier (contemporânea); Beatriz Argéndia-Vargas (em formação inicial). **Dez nomes**, oito séculos."

[FATO] `CHARS.md:230` cita esse mesmo trecho e repete o número: "também registra a **cadeia institucional** do ofício, **dez nomes** (linha 25)". Esta não é uma segunda fonte independente — é a mesma contagem, citada.

[FATO] `docs/narrative/deep/eras/era-1-pre-codigo/capitulo-10-trecho-in-character-cronista-era-3.md:1791`, mais adiante no mesmo capítulo:
> "Sobrevive a linhagem cronística feminina. De Atelaiá Chevalier em -115 até Cassandra Chevalier, minha sucessora direta: **oito gerações sucessivas** em descendência ininterrupta. Entre Atelaiá e mim, a cadeia: Esmeralda Argéndia-Chevalier (filha direta de Atelaiá, segunda geração); Luísa Argéndia-Chevalier (terceira); Anastácia Vyrcátrix-Acaceiro a Paciente (quarta); Hipátia Atelaiá-Vargas (quinta); Olímpia Cardoso-Acaceiro (sexta); Cassandra Yvanova-Calígrafa (sétima); **eu mesma (transicional, da sétima à oitava)**; Cassandra "Bento" Chevalier (**oitava** canônica); Beatriz Argéndia-Vargas (**nona**, em formação)."

[INFERÊNCIA] Contando os nomes desta segunda passagem (Atelaiá + Esmeralda + Luísa + Anastácia + Hipátia + Olímpia + Cassandra Yvanova + a narradora + Cassandra Bento + Beatriz), há **dez** elos nomeados — o mesmo total da linha 25 — mas o texto rotula Cassandra Bento de "oitava" e Beatriz de "nona", um a menos do que a posição real de cada uma na sequência (nono e décimo elo, respectivamente). A causa: a narradora ocupa uma posição real na cadeia ("transicional, da sétima à oitava") sem receber ordinal própria, e a numeração que vem depois dela não é reajustada para compensar. É exatamente o padrão que a fila de exceção do `TODO.md` já apontava ("a causa é uma pessoa sem número... o nono nome ser chamado de oitavo"), confirmado por leitura direta desta rodada.

[FATO — verificação de lacuna] Busquei `"cadeia institucional"` em todo `docs/book/` (onde vivem o índice e o glossário do livro, `BIBLE-V1-INDICE.md` e `BIBLE-V1-GLOSSARIO.md`): **zero ocorrências**. Não encontrei uma terceira fonte, no índice do livro, com uma contagem própria e distinta desta cadeia. Se a premissa da fila de exceção ("uma contagem no inventário de personagens e no índice do livro") tinha em mente uma entrada específica do índice, ela não foi localizada nesta rodada — o inventário de personagens (`CHARS.md`) só repete o número da linha 25, como mostrado acima.

**Decisão do líder:** a numeração ordinal da segunda passagem (linha 1791) precisa reconhecer a posição da narradora com número próprio (o que empurraria Cassandra Bento para "nona" e Beatriz para "décima"), ou a narradora continua deliberadamente fora da numeração (como o "elo sem nome" que `CHARS.md:230` já registra para a cadeia de sangue) e a frase de abertura ("oito gerações sucessivas") precisa dizer "dez elos" para bater com a lista. As duas soluções não são equivalentes: a primeira muda todos os rótulos ordinais a partir da narradora; a segunda só corrige a frase de abertura.

---

## 3. Achado 2 — Sucessão matrilinear cronistas Atelaiá: cinco convenção "pós-Atelaiá", seis convenção "com Atelaiá"

[FATO] `docs/narrative/deep/eras/era-2-boom-tecnico.md:1445`, título de seção: **"### 10.8. Sucessão de cinco gerações cronistas (-78): texto autônomo cerimonial"**. O corpo inteiro da seção (linhas 1447-1471) é internamente consistente com esse número: enumera "Primeira geração" (Antoneta) até "Quinta geração" (cronista contemporânea), sempre "pós-Atelaiá" — cinco gerações depois da codificadora-mãe, sem contá-la.

[FATO] `CHARS.md:228`: **"Linhagem matrilinear cronistas Atelaiá (6 gerações canonizadas...)"**, listando Atelaiá Chevalier → Antoneta → Verônica → Tarsila → Felícia → cronista contemporânea = seis nomes, contando Atelaiá como a primeira geração. Mesma convenção em `era-2-boom-tecnico.md:719` ("seis gerações canon entre a codificadora-mãe e a contemporânea") e em `docs/narrative/deep/factions/ordem-recursiva.md` (seis ocorrências de "sexta geração" para a cronista contemporânea, ex. linhas 190, 242, 246, 268, 272, 306).

[INFERÊNCIA] As duas contagens não são um erro isolado: cada uma é internamente consistente dentro do documento onde vive (§10.8 nunca contradiz a si mesma; `CHARS.md` e `ordem-recursiva.md` também não). A diferença é a convenção — contar a partir de Atelaiá (6) ou a partir da geração seguinte a ela (5) — e as duas descrevem a mesma cadeia de seis pessoas. **Ressalva sobre a premissa recebida:** a fila de exceção descrevia isto como "uma seção" com título 5 e corpo 6; a verificação desta rodada não achou essa contradição dentro de uma única seção — achou-a **entre duas seções de documentos diferentes** que descrevem o mesmo objeto sob convenções opostas, e cada uma delas é internamente limpa. O efeito de leitura é o mesmo (lidas juntas, os números parecem discordar), mas a localização exata diverge do que a fila de exceção registrou, e isso é dito aqui em vez de forçar a citação a caber na descrição original (L-18).

**Não corrijo nada aqui.** O líder já decidiu, no dia de hoje, que a ancestral (Atelaiá) conta como primeira geração — mas uma verificação desta mesma rodada mostrou que aplicar essa regra à seção `era-2-boom-tecnico.md:1445` (mudar "cinco" para "seis" no título e nos cinco rótulos ordinais do corpo) exige renumerar em cascata **quatro documentos que usam a convenção "cinco pós-Atelaiá"** — `era-2-boom-tecnico.md` §10.8 é um deles, e é preciso identificar os outros três antes de tocar em qualquer um, porque um deles é inventário protegido (`CHARS.md` mesmo, se a convenção "seis" ali precisar de ajuste na direção oposta). Este levantamento dos quatro documentos é trabalho novo, não feito nesta rodada.

---

## 4. Achado 3 — Linhagem de sangue da narradora: "oito gerações" na abertura, nove nomes na mesma frase

Este é o mesmo trecho já citado no Achado 1 (`capitulo-10...md:1791`), mas o ângulo aqui é a contagem, não a numeração ordinal.

[FATO] A frase de abertura declara **"oito gerações sucessivas"** entre Atelaiá Chevalier e Cassandra Chevalier.

[FATO] A enumeração que segue, na mesma frase, nomeia: Esmeralda, Luísa, Anastácia, Hipátia, Olímpia, Cassandra Yvanova, a narradora ("eu mesma"), Cassandra "Bento" Chevalier, Beatriz Argéndia-Vargas — **nove nomes**, mais Atelaiá citada no início da frase como ponto de partida ("De Atelaiá Chevalier... até Cassandra Chevalier") = **dez elos no total**, coincidindo com a contagem da linha 25 (Achado 1), não com o "oito" desta mesma frase.

[FATO — contraste com a passagem irmã] A passagem paralela sobre a mesma descendência de sangue, em `capitulo-10...md:91` e `:369` (já registrada por `CHARS.md:230` como "sete gerações", achado já cruzado pelo líder em 01/09/2026), enumera exatamente **sete** nomes (Atelaiá, Castália, Decília, Penélope, Mariana, Sibila, eu) e a frase "sete gerações" bate com a lista — essa passagem está correta e consistente. A passagem em `:1791` é uma cadeia **diferente** (a institucional, com dez elos, não a de sangue, com sete) que empresta o número errado: declara "oito" onde a lista entrega dez.

[INFERÊNCIA] A causa mais provável é contaminação entre as duas cadeias que o capítulo mantém: a "descendência de sangue" (sete, linhas 91/369) e a "cadeia institucional" (dez, linha 25) — e a passagem de `:1791` mistura a linguagem de uma ("gerações sucessivas", "descendência ininterrupta") com os nomes da outra, produzindo um terceiro número ("oito") que não corresponde a nenhuma das duas listas fixadas alhures no mesmo documento.

**Decisão do líder:** qual das duas cadeias a passagem de `:1791` deveria estar descrevendo (a resposta muda se o número correto é sete, dez, ou um terceiro valor específico dessa passagem).

---

## 5. Achado 4 — Árvore Vanderbist (Jaci): oito gerações, mas "bisavó da bisavó" dela

[FATO] `docs/narrative/deep/characters/jaci-proxy.md:9`:
> "O caderno é da **bisavó da bisavó** dela, Anhuera Vanderbist, herborista de quarta geração da linhagem fundadora [...]. Jaci desce dessa linhagem por descendência ininterrupta, **oito gerações canônicas** levantadas em ata anual da Vigília Neo-Sylvania por cronistas que se sucederam sem perder a continuidade da caligrafia."

[INFERÊNCIA] Na mesma frase, o mesmo parentesco (Anhuera → Jaci) recebe dois graus incompatíveis: "bisavó da bisavó" é um termo de parentesco direto que, em qualquer leitura literal, descreve uma distância bem menor que oito gerações; e a própria Anhuera é descrita, poucas palavras depois, como "herborista de **quarta geração** da linhagem fundadora" — um terceiro número, distinto tanto do grau de parentesco quanto das "oito gerações canônicas". Não é convenção de contagem (como o Achado 2): é o documento discordando de si mesmo dentro da mesma frase, com três leituras possíveis (grau de parentesco, "quarta geração", "oito gerações") que não se conciliam por reinterpretação — alguma delas está simplesmente errada.

[FATO — achado colateral] `docs/narrative/deep/antologia/07-jaci-proxy.md:97` e `:215` usam a fórmula "bisavó da bisavó da bisavó da tua/minha bisavó" para o mesmo item (o caderno-semente), num contexto de conto (fala de personagem, registro poético) — a linha 215 chama a tradutora que recebe a semente de "oitava em descendência ininterrupta", o que é pelo menos compatível em ordem de grandeza com "oito gerações" de `jaci-proxy.md:9`, mas usa uma fórmula de parentesco em cascata (quatro "bisavós" empilhadas) que não corresponde a nenhuma convenção de contagem geracional documentada em canon — é licença poética de conto, não dado de ficha de personagem, e a ficha (`jaci-proxy.md`) é onde a inconsistência pesa.

**Decisão do líder:** qual dos três números descreve corretamente a distância entre Anhuera e Jaci (o grau de parentesco em `jaci-proxy.md:9`, a "quarta geração" ali mesma, ou as "oito gerações canônicas"), e se a fórmula poética do conto (`07-jaci-proxy.md`) deve refletir esse número corrigido ou continuar como licença estilística separada da ficha de personagem.

---

## 6. O que fica para o líder (canon, não conserto)

- Achado 1: renumerar a partir da narradora em `capitulo-10...md:1791` (empurrando Cassandra Bento a "nona" e Beatriz a "décima"), ou corrigir só a frase de abertura para "dez elos".
- Achado 2: aplicar a convenção "ancestral conta como primeira geração" (fixada hoje pelo líder) à seção `era-2-boom-tecnico.md` §10.8 exige primeiro levantar os quatro documentos que usam a convenção oposta — trabalho não feito nesta rodada.
- Achado 3: decidir se `capitulo-10...md:1791` deveria dizer "sete" (cadeia de sangue) ou "dez" (cadeia institucional), ou se é uma terceira cadeia com número próprio.
- Achado 4: decidir o número correto de gerações entre Anhuera Vanderbist e Jaci em `jaci-proxy.md:9`, entre as três leituras incompatíveis ali presentes.

## 7. Status

**Nenhum achado foi corrigido neste item.** Quatro achados, quatro vivos, zero corrigidos, zero sem âncora (todos com arquivo e linha). A premissa recebida de que um dos quatro ("linhagem do protagonista") já estava resolvida não se confirmou: a busca não achou correção aplicada a nenhuma linhagem de Gus, e o achado que de fato corresponde ao padrão descrito segue vivo (Achado 3 acima). Conserto de qualquer um dos quatro é item próprio, a abrir depois do `D40` (L-33 do projeto).

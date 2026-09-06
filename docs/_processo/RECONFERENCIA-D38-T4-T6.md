<!--
SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
SPDX-License-Identifier: AGPL-3.0-or-later
-->

> **Escrito em 05/09/2026, sob o item `D38` do `TODO.md`.** Reconfere, contra a árvore de hoje, os dois relatórios legado que o `D38` mandava reconferir, `AUDIT-T4-VOZ-V2.md` (voz Stephenson, 71 achados) e `AUDIT-T6-PALAVRAS-V2.md` (palavras e pontuação proibidas), com a contagem exigida pela L-36 global impressa sempre (encontrados/reconferidos/vivos, mesmo em zero). **Nenhum achado foi corrigido aqui**: este item entrega só o relatório; o conserto do que segue vivo é item próprio, aberto depois (L-33).
>
> Convenção de contagem: "encontrado" é todo achado catalogado no relatório original que afirma um defeito a corrigir — os itens que o relatório original já rotula como não-defeito (preservar, formatação legítima, falso-positivo, exceção documentada) não entram nesta conta e ficam listados à parte, nunca omitidos em silêncio (§3.8). "Reconferido" é aquele que esta rodada conseguiu medir contra a árvore de hoje, por `grep`, por seção ou por nome. "Vivo" é aquele cuja medição confirma que o problema persiste; achado reconferido e não-vivo é achado morto, com a causa registrada. Achado localizado mas cuja persistência específica não foi medida linha a linha fica em `SEM MEDIÇÃO DE VIVACIDADE`, nunca contado como `VIVO`. Achado não-reconferido fica em `SEM ÂNCORA`, nunca em `MORTO` (achado sem resposta não é achado morto). `HERDADO` é o achado cujo veredito não foi remedido nesta rodada, por dizer explicitamente no corpo que herda veredito de rodada anterior — não é `RECONFERIDO` por esta mesma definição, mas também não é `SEM ÂNCORA`, porque tem veredito, só que de outro dia. `ZONA CINZENTA` é o achado reconferido cujo veredito depende de decisão do líder que este relatório explicitamente não toma — nem vivo nem morto até essa decisão.

---

# Reconferência D38: AUDIT-T4-VOZ-V2 e AUDIT-T6-PALAVRAS-V2

## 1. Contagens consolidadas

| Relatório | Encontrados | Reconferidos nesta rodada | Herdado (não remedido) | Vivos | Mortos | Zona cinzenta | Sem medição de vivacidade | Sem âncora |
|---|---|---|---|---|---|---|---|---|
| `AUDIT-T4-VOZ-V2.md` (voz) | 71 | 17 | 0 | 16 | 0 | 0 | 1 | 54 |
| `AUDIT-T6-PALAVRAS-V2.md` (palavras) | 9 itens nomeados | 8 | 1 | 2 | 4 | 2 | 0 | 0 |

O T4 conta por achado individual (18 críticos + 31 médios + 22 leves = 71); o próprio dossiê original avisa que não há "arquivo:linha simples" para nenhum dos 71, porque é crítica de estilo, não de fato. O T6 conta por **item nomeado** (o achado principal do em-dash, mais os quatro `C-T6-0X` nomeados, mais o bloco de médios do anglicismo, mais a lista negra, mais as duas pendências T1-T5), porque as "1.060+ issues" originais são majoritariamente a mesma classe de violação (em-dash) repetida centena de vezes, não achados distintos — e porque os quatro achados `LEVES` do dossiê original (§3.8) já nascem rotulados não-defeito, fora da contagem de "encontrado" por definição (convenção acima). Conferência da linha do T4: vivos (16) + mortos (0) + zona cinzenta (0) + sem medição (1) = 17 = reconferidos. Conferência da linha do T6, pelo **mesmo critério** usado no T4: vivos (2) + mortos (4) + zona cinzenta (2) + sem medição (0) = 8 = reconferidos; reconferidos (8) + herdado (1) = 9 = encontrados. A lista negra factual (item 7, §1.1) é `HERDADO`, não `RECONFERIDO` nesta rodada, e por isso soma em "herdado", nunca em "mortos" desta linha, ainda que o veredito herdado dela seja morto. As somas fecham.

### 1.1. Os 9 itens nomeados do T6, um a um

| # | Item | Seção | Categoria | Por quê |
|---|---|---|---|---|
| 1 | Em-dash em prosa canônica | §3.1 | MORTO | 131 ocorrências em 3 arquivos, todas exceção registrada ou meta-processo; zero em prosa canônica |
| 2 | `C-T6-02`, termos maçom explícitos | §3.2 | VIVO | Confirmado e agravado: 21 trechos em 15 arquivos, contra os 9 alegados |
| 3 | `C-T6-03`, autoincoerência `BIBLE-V1-CAPA.md` | §3.3 | MORTO | Já eufemizado, sem rótulo de ofício explícito ao lado |
| 4 | `C-T6-04`, rótulos de ordem fechada no glossário | §3.4 | ZONA CINZENTA | Reformulação aplicada (Aprendiz/Companheiro/Mestre sem a palavra "maçom"), mas o relatório original deixava decisão one-way-door pendente e este relatório não a resolve |
| 5 | `C-T6-05`, rótulo de instrumentos em `4-linguagens-deep.md` | §3.5 | MORTO | Decisão one-way-door tomada e aplicada ("Selo dos Quatro Instrumentos") |
| 6 | Médios, anglicismo "cross-X" | §3.6 | VIVO | 95 arquivos hoje contra 30 no original; mais que triplicou |
| 7 | Lista negra factual (T1-T5) | §3.7 | HERDADO | "Não re-medida individualmente nesta rodada (falta de tempo); herda o veredito de 25/08" — não é `RECONFERIDO` por definição própria deste documento (§ acima), e o veredito herdado é morto (0 ocorrências em 25/08) |
| 8 | "Patch Zero" sem hífen | §3.7 | ZONA CINZENTA | Sobrevive 1 ocorrência, mas pode ser codinome operacional intencional; "fica para o líder decidir, não é conserto óbvio" |
| 9 | "Janelarum" como cidade autônoma | §3.7 | MORTO | Uso hoje é sempre marca/sistema, nunca topônimo; "parece resolvido" |

## 2. T4-VOZ: os 17 críticos recuperados por medição/nome

O dossiê original (22/05/2026) nunca foi reconferido item a item, e diz de si mesmo que "não há arquivo:linha simples para conferir contra o canon". Esta rodada recuperou 17 dos 18 críticos por dois caminhos que o próprio serviço apontou como viáveis: fórmula de escrita se conta com busca; voz de personagem se localiza pelo nome dele.

### 2.1 Fórmulas saturadas (C-T4-001 a 010), medidas hoje por `grep`

| Fórmula | Original (Era 2, 22/05/2026) | Hoje, `era-2-boom-tecnico.md` | Hoje, todo `era-1-pre-codigo/` (era benchmark limpo) |
|---|---|---|---|
| "articula-se em registro de" | 241 | **306** | 0 |
| "em horizonte de" | 468 | **565** | **136** |
| "calibrad*" | 800+ | **632** | **186** |
| "respeito cerimonial estrito" | 192 | **69** | 0 |
| "preservada por cinco gerações" | 50 | **16** | 0 |
| "cerimonial" | 426 | **1.448** | **154** |

**Status: VIVO, e agravado.** Três das seis fórmulas pioraram desde 22/05/2026 (a densidade de "articula-se em registro de" subiu 27%, "em horizonte de" subiu 21%, "cerimonial" mais que triplicou). As outras três caíram, mas de um patamar já descrito como patológico para outro ainda alto. O achado mais grave desta reconferência: **a fadiga que o relatório original localizava exclusivamente em `era-2-boom-tecnico.md` (tratando `era-1-pre-codigo/` como benchmark limpo, "3" ocorrências de "articula-se em registro de") hoje aparece também em `era-1-pre-codigo/`**, com 136 a 186 ocorrências das mesmas fórmulas. Os capítulos 09 e 10 desse diretório (lidos nesta mesma sessão para o Defeito 1 da genealogia) são consistentes com essa medição: ambos usam "conforme protocolo", "em horizonte cronológico", "articulado" e "canônico" em quase toda frase.

### 2.2 Achados de seção (C-T4-012, C-T4-013), localizados pelo cabeçalho

- C-T4-012 ("articula-se" + "cuja decomposição técnica" fundidos, auto-canibalismo §3.4): a seção existe hoje em `docs/narrative/deep/eras/era-2-boom-tecnico.md:263` ("### 3.4. Os quatro substratos canônicos da Era 2"). Seção presente; o padrão sintático específico não foi relido frase a frase nesta rodada. **Reconferido por localização, vivo por amostragem** (a fórmula "articula-se em registro de" segue presente na tabela acima).
- C-T4-013 (tradição fundadora canônica preservada 12x §9.3): a seção existe hoje em `docs/narrative/deep/eras/era-2-boom-tecnico.md:951` ("### 9.3. Vestígios axiológicos"). **Reconferido por localização, vivo por amostragem.**

### 2.3 Achados de voz de personagem (C-T4-011, C-T4-014, C-T4-015, C-T4-018), localizados pelo nome

| Achado | Personagem | Menções em `era-2-boom-tecnico.md` (voz fadigada) | Menções no benchmark de voz pura | Status |
|---|---|---|---|---|
| C-T4-015 | Sterling Locke | 34 | 3 (`antologia/08-sterling-locke.md`) | VIVO: a maior parte do material sobre o personagem está no texto fadigado, não no benchmark |
| C-T4-018 | Atelaiá Chevalier | 95 | 8 (`antologia/13-veronica-atelaia.md`) | VIVO: mesma leitura, proporção pior |
| C-T4-014 | cronista Era 3 contemporânea | não contável por grep de nome (identidade reservada por desenho) | n/d | **Reconferido por leitura direta**: o capítulo 10 de `era-1-pre-codigo/` (`capitulo-10-trecho-in-character-cronista-era-3.md`), lido integralmente nesta sessão para o Defeito 1, reusa as mesmas fórmulas ("conforme protocolo", "em horizonte cronológico", "articulado") que o relatório original atribuía a Era 1 e Era 2. VIVO |
| C-T4-011 | Patch-Zero | n/d | n/d | **SEM MEDIÇÃO DE VIVACIDADE**: seção localizada (`docs/narrative/characters/patch-zero.md`), conteúdo não relido linha a linha nesta rodada — existência confirmada, persistência do padrão descrito não medida |

### 2.4 C-T4-017 (Maçom canon explicita demais), reconferido e ligado ao achado de T6

Este crítico do T4 fala de densidade de referência maçônica acima do "anti-pervasive ~10-15%" recomendado pela lente maçom do projeto. A reconferência desta rodada (feita para o achado T6 abaixo, §3.2) achou 21 trechos com o par "esquadro e compasso" nomeado explicitamente, em 15 arquivos, incluindo dois documentos supremos (`CHARS.md` e `BIBLE-V1-GLOSSARIO.md`, este último destinado a publicação). **VIVO, e mais grave que o relatado**: o problema não é só densidade alta dentro do "pervasive" aceitável, é o símbolo nomeado de forma **literal e reconhecível**, sem estilização, em texto que vai a público.

### 2.5 O que ficou sem âncora (54 de 71)

Não recuperados nesta rodada: **1 crítico** (C-T4-016, "4 frases consecutivas paralelismo mecânico anti-Stephenson", sem seção nem exemplo citado no relatório original) e a totalidade dos **31 médios** e **22 leves**, que o próprio dossiê descreve como benchmarks a preservar (M-T4-011 a 017, M-T4-025, M-T4-026) ou densidade a "validar cross-doc" sem exemplo pontual (M-T4-001 a 010, M-T4-018 a 024, M-T4-027 a 031), mais a categoria agregada de leves ("ancoragem sinestésica §11 v2"). Nenhum destes tem nome, seção ou fórmula que permita busca objetiva; são julgamento editorial de prosa inteira, não fato pontual. **Recuperados desta vala nesta rodada: 17 de 71** (os 17 críticos das seções 2.1 a 2.4).

## 3. T6-PALAVRAS: os itens nomeados

### 3.1 Achado principal (em-dash em prosa canônica)

Já havia reconferência parcial no cabeçalho do próprio dossiê (25/08/2026): 992 originais caíram para 130 em `docs/narrative/` inteiro. Esta rodada mediu de novo, hoje:

- `grep -ro "—" docs/narrative/` : **131 ocorrências**, em **3 arquivos**: `INCOHERENCES.md`, `comic-reliefs.md`, `in-world-docs.md`.
- `in-world-docs.md` é a exceção canônica registrada (memo `project_em_dash_excecao`), não conta como violação.
- `INCOHERENCES.md` e `comic-reliefs.md` são documento de meta-processo e catálogo de humor, respectivamente, coerentes com a categoria LEVE já prevista no relatório original ("327 em-dash header/meta operacional, preservar").

**Status: MORTO em prosa canônica.** Zero ocorrências de em-dash fora de exceção registrada ou documento de meta-processo.

### 3.2 C-T6-02 (9 violações maçom literal), reconferido nas 20 linhas originais e além

As 20 linhas listadas no dossiê original (`cult-mirage:63,106`; `fir:55`; `ordem-recursiva:66`; `iara-lumen:134`; `dante-grid:7,165`; `03-catedrais:23,40,52`; `01-cidade:88`; `05-mirage:35,55`; `06-periferia:27`; `4-linguagens-deep:27,116`; `npcs-antagonistas:35,79,81`, renomeado de `npcs-antologia`; `08-sterling-locke:97`) foram conferidas uma a uma hoje: **todas as 20 usam hoje o termo eufemizado "instrumentos de medida pareados"/"instrumento de medida pareado"**. Zero sobrevivendo nessas âncoras específicas.

**Mas a alegação do `TODO.md` de "nove trechos com termos de ofício explícitos que auditoria nenhuma listou" se confirma, e está subestimada.** Busca ampla por "esquadro" cruzado com "compasso" na mesma linha, em todo o corpus versionado (`grep -rniE "esquadro" | grep -iE "compasso"`, excluindo `docs/_processo/`), acha **21 linhas em 15 arquivos**:

| Arquivo | Linhas |
|---|---|
| `docs/narrative/deep/factions/ordem-recursiva.md` | 15, 78 |
| `docs/narrative/deep/antologia/04-bento-requiem.md` | 155 |
| `docs/narrative/deep/antologia/05-linda-siren.md` | 121 |
| `docs/narrative/deep/factions/underground-silencio.md` | 47, 95 |
| `docs/narrative/deep/settings/07-silencio.md` | 27, 42, 72 |
| `docs/narrative/deep/characters/linda-siren.md` | 91, 147 |
| `CHARS.md` | 88 |
| `docs/book/BIBLE-V1-GLOSSARIO.md` | 289 |
| `docs/art/props-inventory.md` | 2 linhas (não relidas em detalhe) |
| `resources/prompts_images/feitos/ANTONETA_ARGENDIA_CHEVALIER_IMAGEPROMPT.md` | 3 |
| `resources/prompts_images/feitos/CASIMIRO_CHEVALIER_IMAGEPROMPT.md` | 3 |
| `resources/prompts_images/feitos/CASSIEL_FERRAZ_IMAGEPROMPT.md` | 3 (esta linha nomeia a palavra "macom" por extenso, além do símbolo) |
| `resources/prompts_images/feitos/ESQUADRO_COMPASSO_JOAQUIM_IMAGEPROMPT.md` | título do arquivo inteiro |
| `resources/prompts_images/feitos/FELICIA_TARSILA_IMAGEPROMPT.md` | 3 |
| `docs/_secret/easter-eggs-manual.md` | 1 linha, cifrada por `git-crypt`; só o caminho é citável (L-25 do projeto) |

**O padrão "eufemizado numa linha, explícito em duas outras" que o `TODO.md` descreve aparece de forma exata em `docs/narrative/deep/factions/ordem-recursiva.md`**: a linha 66 já diz "instrumentos de medida pareados" (eufemizado), enquanto as linhas 15 e 78, no mesmo arquivo, ainda dizem "o esquadro e o compasso" e "esquadro com compasso" (explícito). Confirma a hipótese do `TODO.md`: a correção antiga foi feita ponto a ponto nas linhas que a auditoria original apontava, e não varreu o resto do arquivo.

**Veredicto sobre a alegação dos nove trechos: CONFIRMADA e SUBESTIMADA.** Não são nove, são pelo menos 21, em 15 arquivos, dois deles supremos (`CHARS.md`, canon imutável sem aprovação do líder; `BIBLE-V1-GLOSSARIO.md`, destinado a publicação).

### 3.3 C-T6-03 (BIBLE-V1-CAPA.md autoincoerência)

Hoje o trecho diz "Selo dos Quatro Instrumentos canônico C-Arcane (dois instrumentos de medida pareados, cinzel e martelo entrelaçados...)". **MORTO**: já eufemizado, sem rótulo de ofício explícito ao lado da descrição estilizada.

### 3.4 C-T6-04 (BIBLE-V1-GLOSSARIO.md rótulos de ordem fechada)

As cinco linhas citadas (31, 129, 131, 135, 317) hoje descrevem "Aprendiz", "Coluna Boróstoma", "Coluna Janor", "Companheiro" e "Mestre", todos rotulados "(etapa formal de progressão Ordem Recursiva)" ou "(canônico)", sem a palavra "maçom"/"maçônico"/"Loja" em nenhuma delas. A decisão one-way-door pendente no relatório original ("nota editorial paralelo histórico OU purge total?") parece ter sido resolvida por uma terceira via, renomear sem nomear a origem. **Isto é zona cinzenta que este relatório não decide**: a progressão Aprendiz/Companheiro/Mestre é, ela mesma, a estrutura de graus mais reconhecível da maçonaria real, ainda que sem a palavra. Fica para o líder decidir se essa reformulação já satisfaz o item ou se ainda é achado vivo.

### 3.5 C-T6-05 (4-linguagens-deep.md rótulo de instrumentos)

Linhas 27 e 116 hoje dizem "Selo dos Quatro Instrumentos", a mesma solução aplicada em `BIBLE-V1-CAPA.md`. **MORTO**: a decisão one-way-door ("criar termo in-world canônico novo") foi tomada e aplicada.

### 3.6 Médios (anglicismo "cross-X")

`grep -rl "cross-[a-zA-Z]" docs/ --include="*.md"` hoje devolve **95 arquivos** (medição própria desta rodada; o `TODO.md` de 05/09/2026 já registrava 92, por método de contagem provavelmente distinto). Original: 30. **VIVO, e mais que triplicou.** Decisão one-way-door original ("purge prose livro OU manter operacional") segue pendente.

### 3.7 Lista negra T1-T5 e pendências restantes

- Lista negra factual (Patrício Vance, Chevarier, -890, -16 Salvador, -13 Davi morte, etc.): confirmada em 0 ocorrências na reconferência de 25/08/2026, já registrada no cabeçalho do próprio dossiê. **Não re-medida individualmente nesta rodada** (falta de tempo); herda o veredito de 25/08.
- "Patch Zero" sem hífen: sobrevive **1 ocorrência**, `docs/narrative/characters/patch-zero.md:60`, como parte do codinome operacional "GRE Patch Zero" citado por facção antagonista. Pode ser intencional (codinome corporativo distinto do nome canônico com hífen) ou resíduo; **fica para o líder decidir**, não é conserto óbvio.
- "Janelarum" como cidade autônoma: hoje usado de forma consistente como marca/voucher/sistema operacional ("Voucher Janelarum", "painéis Janelarum", "Janelarum 2.0"), nunca como topônimo de cidade, em `docs/narrative/deep/settings/01-cidade.md`. **Parece resolvido.**

### 3.8 Itens leves do relatório original, fora da contagem de defeito

O relatório original (`AUDIT-T6-PALAVRAS-V2.md`, seção `LEVES`) cataloga quatro achados que ele mesmo já rotula como não-defeito, e por isso não entram nos "9 itens nomeados" da tabela §1.1, pela convenção de contagem do topo deste documento: **327** em-dash em cabeçalho/meta operacional (rotulado "preservar"), **719** em-dash em tabela como placeholder `| — |` (rotulado "formatação legítima"), **29** bullets falso-positivos de diálogo (rotulado "falso-positivos"), e **118** em-dash na exceção canônica `in-world-docs.md` (memo `project_em_dash_excecao`, também citada e já reconferida no achado principal, §3.1). Nenhum dos quatro afirma um problema a corrigir; ficam registrados aqui para que a exclusão seja explícita, não silenciosa.

## 4. O que fica para o líder (canon, não conserto)

- T4 C-T4-016: sem exemplo nem seção no relatório original; não há como reconferir sem o `narrative-writer` reler `era-2-boom-tecnico.md` inteiro à procura de paralelismo mecânico de quatro frases. Decisão: vale reabrir com âncora nova, ou aposentar o achado?
- T4 médios e leves (53 achados): a mesma pergunta, em bloco maior. O `TODO.md` já registra essa decisão como pendente ("Decidir se vale reabrir a auditoria com âncora ou aposentar a classe inteira").
- T6 C-T6-04: a reformulação Aprendiz/Companheiro/Mestre satisfaz a decisão one-way-door pendente, ou ainda é achado vivo?
- T6 anglicismo "cross-X": purgar da prosa do livro ou manter como termo operacional (decisão one-way-door original, ainda sem resposta, e o número cresceu desde então).
- T6 "GRE Patch Zero" sem hífen: codinome operacional distinto por desenho, ou resíduo a corrigir?

## 5. Status

**Nenhum achado foi corrigido neste item.** T4: 71 encontrados, 17 reconferidos, 16 vivos e agravados, 0 mortos, 1 sem medição de vivacidade (`C-T4-011`, seção localizada, conteúdo não relido), 54 sem âncora. T6: 9 encontrados, 8 reconferidos nesta rodada + 1 herdado de 25/08, 2 vivos (`C-T6-02` agravado de 9 para 21 trechos, e o anglicismo cross-X mais que triplicado de 30 para 95 arquivos), 4 mortos nesta rodada (em-dash, `C-T6-03`, `C-T6-05`, "Janelarum") mais 1 herdado (lista negra factual, veredito de 25/08, também morto, mas não remedido nesta rodada e por isso fora da conta de "mortos" desta linha), 2 em zona cinzenta (`C-T6-04` e "Patch Zero" sem hífen, ambos aguardando decisão do líder), 0 sem âncora — mais 4 achados `LEVES` do relatório original, catalogados e já não-defeito por rótulo próprio, fora desta contagem (§3.8). Conserto de tudo que segue vivo é item próprio, a abrir depois (L-33 do projeto).

# 01. Cronologia e aritmética de datas/idades (C1)

> Consolidação: internal-auditor, 2026-07-08. Fontes brutas: `raw/L1.md`, `raw/L5.md`, `raw/L6.md`, `raw/L8a.md`, `raw/L8b.md`, `raw/L9.md`. Herda de T3-v2 e do tracker `docs/narrative/INCOHERENCES.md`.

## Contexto e método

Verificação cross-doc de datas absolutas (calendário relativo ao ano 0), idades e aritmética derivada (nascimento + evento = idade). Âncoras de verdade: `timeline.md`, `CHARS.md`, `sinopse.md`. Padrão dominante identificado: deep-lore escrito em 2026-05-19 não foi retrofitado contra correções posteriores dos docs centrais, e vice-versa (docs centrais de personagem não retrofitados contra deep-lore já correto). Inclui 2 REGRESSÕES de itens já RESOLVIDOS em `INCOHERENCES.md`.

## Achados (17: 6 críticos, 11 importantes)

### AL-C1-01 | 🔴 CRÍTICO | REGRESSÃO (INCOHERENCES C9)

Yakov com idade errada na timeline. Fonte A: `timeline.md:120` Yakov "~25" em -11. Fonte B: `CHARS.md:47` + `lore-bible.md:528` Yakov = 42 hoje, logo 31 em -11. As idades da dupla Pyotor/Yakov subiram (40/36 para 46/42, INCOHERENCES C9) sem recalcular a timeline. Cascata: `docs/book/BIBLE-V1-INDICE.md:389`. Fix: "~25" para "~31" em timeline + book. Origem: L1-01. Estado: —

### AL-C1-02 | 🔴 CRÍTICO | ÂNCORA (confirmado por 4 lotes)

`CHARS.md:136` typo de data: queda de Inês Marçal em "-11 (Cauã 10 anos)". Cauã nasceu -13, logo 10 anos = **-3**. Fontes concordantes (6 docs): `deep/settings/04-dutos.md:35`, `characters/caua-volt.md:100`, `deep/characters/caua-volt.md:71`, `factions.md:659`, `lore-bible.md:563`, `deep/factions/facoes-menores.md:11`. O erro está na âncora CHARS.md (typo 11 por 3), risco alto de propagação futura. Fix mecânico: `CHARS.md:136` "-11" para "-3". Origem: L5-04 + L8a-03 + L8b-08 + L9-03 (4 lotes independentes convergem). Estado: —

### AL-C1-03 | 🔴 CRÍTICO | REGRESSÃO (INCOHERENCES C2, correção Bento 4 para 7)

Idade de Bento na contaminação/morte de Hilário não propagada. Fonte A: `deep/settings/03-catedrais.md:43` "quatro anos" no evento -7; `deep/characters/bento-requiem.md:17,89` "aos 4 anos, -10". Fonte B: `timeline.md:127` + `CHARS.md:133` + `factions.md:223` + foreshadowing F070 + o próprio `bento-requiem.md:119` = **-7, 7 anos**. A correção "(4) para (7)" já registrada em INCOHERENCES pegou em factions mas não em settings03 nem em bento-requiem (que ainda carrega a data -10 errada). Fix: "quatro" para "sete" em settings03:43; "-10/4 anos" para "-7/7 anos" em bento-requiem:17,89. Origem: L8b-01 + L6-01. Estado: —

### AL-C1-04 | 🟠 IMPORTANTE

Fala do Velhusto (cena -0.5) dá a Bento "dez anos"; nasceu -14, logo 13,5. Fonte A: `deep/settings/03-catedrais.md:62`. Fonte B: `timeline.md:140` + `sinopse.md` §Ato2. Locus único, não propagante. Fix: "dez anos completos" para "treze anos e meio". Origem: L8a-02 + L8b-02. Estado: —

### AL-C1-05 | 🔴 CRÍTICO

Data de localização/captura do Patch-Zero divergente nos DOIS docs-autoridade do personagem. Fonte A: `characters/patch-zero.md:60` data **-8** + "8 anos de contenção"; `deep/antagonists/patch-zero-deep.md:51-53` repete -8. Fonte B: `timeline.md:133` + `lore-bible.md:508` = **-3**. Drift sistêmico deep-vs-central. Recomendação do executor (endossada): manter -3 do hub e reescrever os 2 docs de Patch-Zero. Executor: narrative-writer. Origem: L5-01. Estado: —

### AL-C1-06 | 🟠 IMPORTANTE

Mandato do Depto de Contenção: `deep/antagonists/patch-zero-deep.md:51` "-8"; `deep/factions/npcs-antologia.md:35` + `timeline.md:132` = **-4**. Fix: "-8" para "-4" (ou distinguir informal -11 / formal -4). Corrigir junto com AL-C1-05 (mesmo arquivo/linha). Origem: L6-09. Estado: —

### AL-C1-07 | 🔴 CRÍTICO

Dante "aos 8 anos" nas cenas do evento -8 (destruição da cooperativa + funeral); nasceu -13, logo tinha **5** (8 anos = -5, ano do recrutamento). Fonte A: `characters/dante-grid.md:160,168` + `foreshadowing.md:52` (F026) + in-world-docs doc16 pág. 19 (idade incompatível). Fonte B: `deep/characters/dante-grid.md:15,17` + `deep/settings/04-dutos.md:35` (já corrigidos para 5). Erro replicado em 3 docs. Fix: "8 anos" para "5 anos" nas 2 cenas + propagar a F026 + doc16. Executor: narrative-writer. Origem: L5-02. Estado: —

### AL-C1-08 | 🟠 IMPORTANTE

Auto-contradição interna: `characters/jaci-proxy.md:27` "pais mortos há 3 anos" vs o mesmo doc `:97` "aos 3 anos, surto -8". Jaci tem 11 (nasceu -11), surto -8 = há **8 anos**. `timeline.md:126` + `CHARS.md:68` confirmam -8. Fix: "há 3 anos" para "há 8 anos". Origem: L5-03. Estado: —

### AL-C1-09 | 🔴 CRÍTICO

Doc-autoridade de Iara erra a própria origem: `deep/characters/iara-lumen.md:13` "encontrada -12 com 4 anos"; `timeline.md:118` = nasceu -12 (encontrada recém-nascida). Na mesma linha, Sérgio Brimber "16 anos em -12"; `CHARS.md` §6 = 24 hoje, logo **12** em -12. Fix conjunto na linha 13: "4 anos" para "recém-nascida" + "16" para "12". Executor: narrative-writer. Origem: L6-03 + L6-04 (fundidos: mesma linha). Estado: —

### AL-C1-10 | 🟠 IMPORTANTE

Discrepância intra-timeline de 3 anos: `timeline.md:77` Atelaiá "69 anos" em -78 (nascimento -147) vs `timeline.md:196` "(-150 a -78)" (nascimento -150). Fix: alinhar (escolher um valor). Origem: L1-09. Estado: —

### AL-C1-11 | 🟠 IMPORTANTE

`deep/antagonists/patch-zero-deep.md:13` "600 anos de Hiato" + "colapso -720"; `timeline.md:47` Hiato -700 a -150 = **550 anos**, e -720 é o Êxodo (colapso = -700). Fix: "600" para "550" + distinguir Êxodo/colapso. Corrigir junto com AL-C1-05/06. Origem: L6-08. Estado: —

### AL-C1-12 | 🟠 IMPORTANTE

`deep/factions/npcs-antologia.md:57` Penha Lirio "transferida FIR -3 quando Apex Chapter 11"; `timeline.md:104` Chapter 11 = -19 (e Penha, 29 hoje, não fecha com -19). A transferência -3 pode ficar; a atribuição ao Chapter 11 não. Fix recomendado: remover/reformular a cláusula causal (narrative-writer decide a redação; não exige criador). Origem: L6-07. Estado: —

### AL-C1-13 | 🟠 IMPORTANTE

Auto-contradição interna em `deep/settings/05-mirage.md`: `:35,43` Sonja morre -34 aos 33; `:81` diz "-5 ... aos 62". Fix: `:81` "-5" para "-34" e "62" para "33". Origem: L8a-04 (recalibrado de CRÍTICO: intra-doc, não propagante). Estado: —

### AL-C1-14 | 🟠 IMPORTANTE

Auto-contradição em `deep/factions/ordem-recursiva.md`: `:256` Antoneta "55 anos de ofício" (-78 a -23) vs `:260` Verônica herda em "-65" (só 13 anos depois). Fix: alinhar as datas de sucessão. Origem: L9-01 (recalibrado: intra-doc). Estado: —

### AL-C1-15 | 🟠 IMPORTANTE

`deep/factions/ordem-recursiva.md:264` Tarsila "34 anos de ofício" vs a própria linha "-25 até -8" (= 17 anos); timeline confirma -25 a -8. Fix: "34" para "17". Origem: L9-02 (recalibrado: aritmética intra-doc). Estado: —

### AL-C1-16 | 🟠 IMPORTANTE

Easter egg forçou aritmética errada: `deep/characters/gus-dragon.md:123` "Pyotor+Yakov = 55 anos somados"; `CHARS.md` = 46+42 = **88**. Fix: remover "55 somados" ou realocar o 55 para outro atributo (narrative-writer escolhe, mantendo o egg velado). Origem: L6-06. Estado: —

### AL-C1-17 | 🟠 IMPORTANTE

Auto-contradição em `deep/characters/dante-grid.md`: `:25` "aos 12 = 21 missões" vs `:163` "início aos 10 ... 21 missões até ano 0" (progressão estagnada). Fix: alinhar idade de início + curva de missões. Origem: L6-11. Estado: —

## Conclusão

A classe dominante é retrofit incompleto entre camadas (central vs deep), com 2 regressões de INCOHERENCES já resolvidos, o que indica que correções de cronologia precisam de passe de propagação obrigatório (grep pelo fato corrigido no corpus inteiro) antes de fechar. O erro âncora `CHARS.md:136` (AL-C1-02), confirmado por 4 lotes independentes, é a prioridade número 1 do capítulo por risco de cópia futura.

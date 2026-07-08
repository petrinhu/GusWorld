# 02. Personagens vs CHARS.md (C2) + housekeeping do CHARS.md

> Consolidação: internal-auditor, 2026-07-08. Fontes brutas: `raw/L1.md`, `raw/L5.md`, `raw/L6.md`, `raw/L8a.md`, `raw/L8b.md`, `raw/L9.md`. Herda de T1-v2.

## Contexto e método

Nome exato, idade, parentesco, traço, aparato e status de cada personagem nomeado contra a âncora `CHARS.md`. Verificado limpo pelos executores: grafias dos 8 nomes canônicos da party, matriz de linguagens de `party.md` (100%), aparatos do triângulo de hardware, axiologia dos antagonistas. Inclui 1 REGRESSÃO de INCOHERENCES C4.

## Achados (14: 4 críticos, 7 importantes, 3 cosméticos)

### AL-C2-01 | 🔴 CRÍTICO

Belinor Vance com lado familiar errado na âncora timeline. Fonte A: `timeline.md:201` "avó **materna**". Fonte B: `CHARS.md:49` + `deep/characters/gus-dragon.md:15` = avó **paterna** (mãe de Pyotor). Cascata: `comic-reliefs.md:977` (EE-13) atribui frase de Belinor a "seu avô" (masculino). Fix: timeline "materna" para "paterna"; comic-reliefs "avô" para "avó". Origem: L1-02. Estado: —

### AL-C2-02 | 🔴 CRÍTICO

Tao Berisi classificado como "NPC adulto" na âncora. Fonte A: `CHARS.md:235`. Fonte B (5 docs): `factions.md:658` + `lore-bible.md:562` + `deep/characters/caua-volt.md:69` + settings04 + env04 = **criança, 12 anos** (par de Cauã). CHARS é o único outlier (resquício de party antiga). Fix: `CHARS.md:235` remover "adulto", canonizar Tao = 12. Origem: L1-03. Estado: —

### AL-C2-03 | 🟠 IMPORTANTE

Sobrenome "Sevroski" do Padrinho Tiago existe em `factions.md:597` + `lore-bible.md:554` + `deep/characters/linda-siren.md:17`, mas `CHARS.md:83` + `timeline.md:137` registram só "Padrinho Tiago". Gap de âncora (não contradição). Fix: adicionar o sobrenome em CHARS (ou removê-lo dos 3 docs; recomendado: adicionar). Origem: L1-07 + L6-17. Estado: —

### AL-C2-04 | 🟠 IMPORTANTE

6 NPCs recorrentes ausentes de CHARS.md: Bito Caldeira, Bem-Te-Vi Caldeira, Helena Sirinhaém (settings02+08), Bel Galvão, Pirilampo, Tata Bruno (settings04), presentes também em factions/lore-bible. `CHARS.md` §9 já reconhece "auditoria pendente". Fix: adicionar entradas em CHARS §7 com fatos transcritos das fontes citadas. Origem: L1-08 + L8a-09. Estado: —

### AL-C2-05 | 🟠 IMPORTANTE

Joaquim Bartolomeu com ENTRADA DUPLICADA em `CHARS.md:87` e `CHARS.md:140` (mesmo NPC, feito -3), e `deep/settings/07-*.md:27,42` troca o gênero do ancestral ("bisavó" vs CHARS "bisavô"). Fix: mesclar as 2 linhas de CHARS + uniformizar o gênero nas 2 ocorrências de settings07. Origem: L8a-07 + L6-15. Estado: —

### AL-C2-06 | 🔴 CRÍTICO

Herança de Bruno Caval divergente em 2 docs, com mecânica de reveal dependente. Fonte A (erradas): `deep/settings/06-periferia.md:27,33` + `deep/factions/fir.md:55` "avô paterno -22". Fonte B: `CHARS.md:102` + `deep/factions/npcs-antologia.md:55` "bisavô (4 gerações)". A mecânica do reveal depende de 4 gerações. Fix: "avô" para "bisavô" nos 2 docs. Origem: L8a-06. Estado: —

### AL-C2-07 | 🔴 CRÍTICO | REGRESSÃO (INCOHERENCES C4, RESOLVIDO 2026-05-30)

Mateus Penkin "filho de Bartolo" em `deep/settings/06-periferia.md:19,25`; canon resolvido pelo criador = **sobrinho** (Bartolo = tio), conforme `CHARS.md:84,138` + `in-world-docs.md:715`. A resolução C4 não pegou em settings06. Fix: "filho" para "sobrinho" (2 loci). Origem: L8b-03. Estado: —

### AL-C2-08 | 🟠 IMPORTANTE

`deep/factions/sterling-corp.md:89` "Hilário Tepenkov mestre cronista"; `deep/factions/ordem-recursiva.md:55` + `factions.md:203` + `CHARS.md:133` = **aprendiz, 14 anos**. Locus único em doc não-autoridade do fato. Fix: "mestre cronista" para "aprendiz". Origem: L9-04 (recalibrado de CRÍTICO). Estado: —

### AL-C2-09 | 🟠 IMPORTANTE

Família Berenger com profissão errada em `deep/factions/sterling-corp.md:13,15,79` ("ferroviária"/"funcionava como ferrovia"); `lore-bible.md:525` + `timeline.md:105` + era-3:63,81 = família técnica de **QA**; Subestação 7 é elétrica (contexto fir.md). Fix: "ferroviária" para "QA" + remover a metáfora de ferrovia (3 loci, mesmo doc). Origem: L9-05 (recalibrado: confinado a 1 doc). Estado: —

### AL-C2-10 | 🟠 IMPORTANTE

`deep/characters/gus-dragon.md:1` intitula "Gus 'Dragon' Vector Tavus Vance" e o doc tem ZERO ocorrências de "Gustaf"; `CHARS.md` §1 = "Gustaf 'Gus' Vector Tavus Vance". Fix: preservar "Gustaf" em ao menos 1 menção do doc. Relaciona-se com AL-C7-01 (memória `project_nome_gus_canon` também desalinhada). Origem: L6-13. Estado: —

### AL-C2-11 | 🟢 COSMÉTICO

`characters/sterling-locke.md:13` cita "Iolanda" (antagonista descartado, lista negra `CHARS.md:257`) em nota editorial de histórico. Fix: reescrever a nota sem nomear. Origem: L5-06. Estado: —

### AL-C2-12 | 🟢 COSMÉTICO

Sobrenome "Vanderbist" de Solane (`deep/characters/jaci-proxy.md:78`) não registrado em `CHARS.md:135`. Adição não-conflitante. Origem: L6-16. Estado: —

### AL-C2-13 | 🟢 COSMÉTICO | BAIXA CONFIANÇA

Filomena Garda: `factions.md:504` "vítima marginalizada de acidente" vs `CHARS.md:92` "catalogadora ambiente das catacumbas". Pode ser complementar (mesma pessoa, dois momentos). Fix: confirmar antes de mexer; se complementar, fechar sem ação. Origem: L1-11. Estado: —

### AL-C2-14 | 🟠 IMPORTANTE | VERIFICAÇÃO PENDENTE

Cleomir Vasta "60+" em `docs/book/BIBLE-V2-INDICE.md:107` vs 37 no restante do canon. Proveniente de nota cross-lote do L1 (o lote do book não estava entre os 8 relatórios recebidos); confirmar o locus do "37" antes de corrigir o índice V2. Origem: L1 (nota cross-lote). Estado: —

## Conclusão

CHARS.md, a âncora, concentra os problemas de housekeeping (1 typo de data crítico já tratado em AL-C1-02, 1 entrada duplicada, 1 status errado de Tao, 2 sobrenomes ausentes, 6 NPCs faltantes). Recomenda-se um único passe mecânico de higiene do CHARS.md cobrindo AL-C1-02 + AL-C2-02/03/04/05/12 de uma vez, seguido de re-teste por grep.

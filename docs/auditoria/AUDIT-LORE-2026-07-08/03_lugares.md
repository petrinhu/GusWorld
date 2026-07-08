# 03. Lugares vs PLACES.md (C3) + gaps do PLACES.md

> Consolidação: internal-auditor, 2026-07-08. Fontes brutas: `raw/L1.md`, `raw/L6.md`, `raw/L8a.md`, `raw/L8b.md`, `raw/L9.md`. Herda de T2-v2.

## Contexto e método

Nomes, numeração de distritos, contagens estruturais (catedrais, aldeias) e geografia contra a âncora `PLACES.md` e o ground-truth `lore-bible.md` §5.

## Achados (8: 4 críticos, 3 importantes, 1 cosmético)

### AL-C3-01 | 🔴 CRÍTICO

Setor Mirage com número de distrito errado no doc-autoridade do próprio setting. Fonte A: `deep/settings/05-mirage.md:11,13,40` "Distrito 4" (3 ocorrências). Fonte B: `lore-bible.md:118-126` §5.1 (ground-truth) + `environments/07-*.md` = Distrito 4 é a **Zona do Silêncio**; Mirage = **Distrito 3**. Quem escrever conteúdo novo de Mirage lê settings05 primeiro, logo propaga. Fix: "Distrito 4" para "Distrito 3" nas 3 ocorrências. Origem: L8b-07. Estado: —

### AL-C3-02 | 🔴 CRÍTICO | DECISÃO DO CRIADOR

Árvores gigantes nomeadas Boróstoma/Janor (fendida por raio em -45) em DOIS lugares fisicamente incompatíveis. Fonte A: `deep/settings/02-selve-sombria.md:12,39,73` "duas árvores na entrada da Orla Recursiva". Fonte B: `deep/settings/08-selve-profunda.md:13,53,65,77` "cinco árvores rodeando o Núcleo Mandelbrot". Não pode ser a mesma árvore física. Recomendação do executor (endossada): par nomeado exclusivo do Núcleo; settings02 usa árvores/motivo diferente na Orla. Cabe ao criador escolher o local canônico. Origem: L8a-01 + L8b-05. Estado: —

### AL-C3-03 | 🟠 IMPORTANTE

`environments/08-selve-profunda.md:30` "aproximadamente quarenta casas" (Pelicano Branco); o cluster deep de 2026-05-19 (settings08 + settings02 + `deep/factions/pelicano-branco.md`, 4 docs) fixa **vinte e uma famílias** (valor load-bearing, egg velado). env08 (Bloco F antigo) não retrofitado. Fix: "quarenta casas" para "vinte e uma famílias". Origem: L8b-06 (recalibrado de CRÍTICO: 1 doc, não-autoridade atual). Estado: —

### AL-C3-04 | 🟠 IMPORTANTE

Contradição interna em `environments/01-cidade-cyber-gotica.md`: placa dos 4 engenheiros "na lateral da Praça da Compilação" (`:16,43`) vs "átrio da Universidade Pública" (`:68`, reforçado por `tradicoes-cultura.md:41`). Fix: alinhar (recomendado: átrio, que tem 2 fontes). Origem: L8b-09. Estado: —

### AL-C3-05 | 🟠 IMPORTANTE

"Universidade Pública" recorre em 6+ docs (env01/07/08, settings01, tradicoes-cultura) mas NÃO tem entrada em `PLACES.md`. Gap da âncora. Fix mecânico: adicionar entrada em PLACES.md §5. Origem: L8b-10. Estado: —

### AL-C3-06 | 🔴 CRÍTICO

Tabela do §9 de `tradicoes-cultura.md:270-280` diz "5 catedrais ATIVAS" mas lista 2 saqueadas (Atelaiá/São Vargas) + a Catedral-Mãe lacrada, e a Quinta (a realmente ativa) está AUSENTE. Fontes de verdade: `factions.md:149` + `PLACES.md:41` + `deep/factions/ordem-recursiva.md:41` = **3 ativas** (São Camilo/Quarta/Quinta). Doc-hub divergindo da âncora em contagem estrutural. Fix: trocar "Catedral-Mãe" por "Quinta" na tabela + header "3 ativas + 2 saqueadas". Origem: L1-05 + L9-09. Estado: —

### AL-C3-07 | 🟢 COSMÉTICO

Grafia "Pântano **de** Markov" em `deep/antagonists/patch-zero-deep.md:43,73,85`; âncora `PLACES.md:89` + timeline + dante-grid = "Pântano Markov". Fix: padronizar sem "de" (3 loci). Origem: L6-14 (recalibrado de IMPORTANTE: convenção de grafia, sem ambiguidade). Estado: —

### AL-C3-08 | 🔴 CRÍTICO

Origem das 3 catedrais tributárias trocada em 2 docs. Fonte A (erradas): `deep/factions/pelicano-branco.md:9,61` + `factions.md:415` põem Tucano-Cinza/Pelicano Roxo/Tartaruga-Fractal como sobreviventes do Êxodo -720. Fonte B: `PLACES.md` §4b (canon T2-C3) = os 3 são "Era 1 NÃO-Êxodo, anel tributário"; `deep/factions/ordem-recursiva.md:172` ACERTA (só 6 Vyr-aldeias no Êxodo). Fix: distinguir a origem em pelicano-branco + factions.md:415. Origem: L9-06. Estado: —

## Conclusão

Dois erros estruturais em docs-autoridade (Mirage/distrito e catedrais ativas) e um conflito físico que exige decisão do criador (Boróstoma/Janor). PLACES.md em si está saudável (1 gap de entrada, nenhum erro interno achado); o problema é o corpus divergir dela.

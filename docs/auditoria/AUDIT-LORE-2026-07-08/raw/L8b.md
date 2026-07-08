# Relatório bruto — Lote L8b (environments × deep/settings)

11 achados: **7 CRÍTICOS, 3 IMPORTANTES, 1 LEVE**. Padrão dominante: deep-lore recente (2026-05-19) não retrofitado contra correções já em INCOHERENCES.md + Bloco F antigo + divergência entre clusters paralelos.

## CRÍTICOS
- **AL-L8b-01 | K1/K5** — `deep/settings/03-catedrais.md:43` diz Bento "quatro anos" no evento -7; timeline.md:127 + CHARS.md:133 + factions.md:223 dizem **7 anos**. INCOHERENCES.md C2 já corrigiu esse "(4)→(7)" em factions mas não propagou pra settings03. Fix: "quatro"→"sete anos".
- **AL-L8b-02 | K1** — `deep/settings/03-catedrais.md:62` (voz Velhusto, -0.5): Bento "dez anos completos"; timeline.md:140 + sinopse §Ato2 dizem **13,5 anos** (nasceu -14). Frase internamente incoerente. Fix: "dez anos completos"→"treze anos e meio".
- **AL-L8b-03 | K2/K5** — `deep/settings/06-periferia.md:19,25` diz Mateus Penkin "filho de Bartolo"; CHARS.md:84/138 + in-world-docs.md:715 dizem **sobrinho** (Bartolo=tio). INCOHERENCES.md C4 RESOLVEU isso 2026-05-30, não pegou em settings06. Fix: "filho"→"sobrinho".
- **AL-L8b-04 | K5** — causa mortis oficial de Salviano Alencar (-8) diverge: timeline.md:124 + diary (2 docs) = "acidente/overdose"; `deep/settings/06-periferia.md:13,39` + dante-grid.md:17 + fir.md:15 + sterling-corp.md:83 = "insuficiência cardíaca". Ambos dizem ser a versão "oficial". Fix: escolher UMA (revisor sugere "insuficiência cardíaca", maioria 4-3) e retrofitar o outro grupo. **DECISÃO DO CRIADOR.**
- **AL-L8b-05 | K5/K3** — árvores gigantes nomeadas Boróstoma/Janor (fendida por raio em -45) em DOIS lugares incompatíveis: `deep/settings/02-selve-sombria.md:73` "duas árvores na entrada da Orla Recursiva" vs `deep/settings/08-selve-profunda.md:53,65,77` "cinco árvores rodeando o Núcleo Mandelbrot". Não pode ser a mesma árvore física. Fix: settings02 usar árvores/motivo diferente na Orla; par nomeado exclusivo do Núcleo. **DECISÃO DO CRIADOR.**
- **AL-L8b-06 | K1/K5** — `environments/08-selve-profunda.md:30` "aproximadamente quarenta casas" (Pelicano Branco); settings08 + settings02 + pelicano-branco.md (4 docs, 2026-05-19) fixam **vinte e uma famílias**. env08 (Bloco F antigo) não retrofitado. Fix: "quarenta casas"→"vinte e uma".
- **AL-L8b-07 | K3** — `deep/settings/05-mirage.md:11,13,40` diz Setor Mirage = "Distrito 4"; lore-bible.md:118-126 §5.1 (ground-truth) + environments/07 = Distrito 4 é **Zona do Silêncio**; Mirage = Distrito 3. Fix: settings05 "Distrito 4"→"Distrito 3" (3 ocorrências).

## IMPORTANTES
- **AL-L8b-08 | K1 (ÂNCORA)** — `CHARS.md:136` diz queda de Inês Marçal em "-11 (Cauã 10 anos)"; settings04 + factions.md:659 + lore-bible.md:563 + caua-volt.md:71 (6 docs) dizem **-3** (Cauã nasceu -13, 10 anos = -3). CHARS.md (âncora!) tem typo 11↔3. Fix: CHARS.md:136 "-11"→"-3".
- **AL-L8b-09 | K3** — contradição INTERNA em `environments/01-cidade-cyber-gotica.md`: placa dos 4 engenheiros "na lateral da Praça da Compilação" (:16,43) vs "átrio da Universidade Pública" (:68, reforçado por tradicoes-cultura.md:41). Fix: alinhar.
- **AL-L8b-10 | K3** — "Universidade Pública" recorre em ≥6 docs (env01/07/08, settings01, tradicoes) mas NÃO está em PLACES.md. Fix: adicionar entrada em PLACES.md §5.

## LEVE
- **AL-L8b-11 | K6 EASTER EGG ROTULADO (repo público!)** — "**Abelha-Fibonacci**" (nome de espécie) em settings02 (:19,29,75) + settings08:31; "**padrão Fibonacci**" explícito em `environments/07-zona-do-silencio.md:157`. A sequência numérica não é achado, mas o RÓTULO "Fibonacci" no nome/qualificador viola o velado. Também em characters/jaci-proxy.md (fora do lote). Fix: renomear espécie (ex. "abelha-espiral") + "padrão Fibonacci"→"padrão numérico recorrente".

## Classes limpas (verificadas): K1(resto)/K2(resto)/K3(resto)/K4(todos cross-refs settings OK, nenhum nome renomeado obsoleto)/K6(zero em-dash/palavrão/maçonaria explícita)/K7(foreshadows Dante/Iara/Bento todos com entrada F-numerada)/K8.

# F5-BK.AUDIT.FULL T6 v2 — PALAVRAS PROIBIDAS CROSS-DOC

**Data:** 2026-05-22 (TEXTREVIEW v2)
**Status:** **BLOQUEADO MASSIVO**, 1.060+ issues
**Issues:** 1.001 críticos + 30 médios + 356 leves

---

## STATUS DE CASCATA (revisão F1-DL.TRACKER-CLOSE 2026-05-30)

> Marcador de bookkeeping. NÃO altera o canon.

**Os ~1.001 CRÍTICOS T6 v2 (majoritariamente em-dash em prose) permanecem PENDENTES (BLOQUEADO).** Natureza: anti-pattern de pontuação (992 em-dash) + maçom literal (9) + termos de glossário. **Não são incoerências factuais de canon**; são limpeza de pontuação/estilo + decisões de terminologia in-world (6 one-way doors pendentes). Fora do escopo de F1-DL.TRACKER-CLOSE (incoerências factuais C1-C15). Nenhum crítico marcado resolvido. NOTA: a LISTA NEGRA factual citada aqui (Patrício Vance, -13 Davi morte, Chevarier, Salvador -16) está confirmada **0 ocorrências** em prose canon (cf. INCOHERENCES C1/C3); essa parte está coerente com o fechamento das incoerências.

## ACHADO PRINCIPAL: 992 violações em-dash prose canon central

Top ofensores cross-doc:
- factions.md: 128
- lore-bible.md: 78
- BIBLE-V2-STRUCTURE.md: 48
- diary/entries-docs-descobriveis.md: 45
- environments/04-dutos-infernais.md: 43
- tradicoes-cultura.md: 39
- environments/03-catedrais: 34
- foreshadowing.md: 31
- ... 60+ docs

Top 20 docs = ~68% dos 992 em-dash totais.

## Outras CRÍTICAS

### C-T6-02: 9 violações maçom literal cross-doc deep-lore
- deep/factions: cult-mirage:63,106 + fir:55 + ordem-recursiva:66
- deep/characters: iara-lumen:134 + dante-grid:7,165
- deep/settings: 03-catedrais:23,40,52 + 01-cidade:88 + 05-mirage:35,55 + 06-periferia:27
- deep/magic: 4-linguagens-deep:27,116
- deep/antagonists: npcs-antologia:35,79,81
- deep/antologia: 08-sterling-locke:97

### C-T6-03: BIBLE-V1-CAPA.md:25 autoincoerência
Trecho com instrumentos entrelaçados estilizados como engenharia rúnica traz, na ressalva entre parênteses, um rótulo de ofício cifrado explícito; remover o rótulo, manter a descrição do instrumento.

### C-T6-04: BIBLE-V1-GLOSSARIO.md rótulos de ordem fechada nomeados
L31+129+131+135+317 trazem termos de ordem fechada nomeados de forma explícita. Decisão criador: meta-glossário pode manter como "Nota editorial paralelo histórico" OU purge total?

### C-T6-05: 4-linguagens-deep.md rótulo de instrumentos entrelaçados explícito
L27+L116. Decisão criador: criar termo in-world canônico novo ("Selo dos Quatro Instrumentos" / "Selo Tavus-Argéndia").

## MÉDIOS

- 30 docs com anglicismo "cross-X" pervasive
- Decisão criador: purge prose livro OU manter operacional

## LEVES

- 327 em-dash header/meta operacional (preservar em AUDIT/TODO/INCOHERENCES)
- 719 em-dash tabela placeholder `| — |` (formatação legítima)
- 29 bullets falso-positivos diálogo
- 118 em-dash exceção `in-world-docs.md` (canon supremo memo `project_em_dash_excecao`)

## LISTA NEGRA T1-T5 v2: 99% LIMPA

- Patrício Vance + Vênea + Chevarier + -890 + -16 Salvador + -13 Davi morte + Augustus Tavus + Aldebrando-Velho + Hilário Chevalier + Lucélia/Antonella/Helga + Sylvania-Catedrais + Vyr-Aldeia + Polis Vermelha s/hífen + Sonja "-6" = **0 ocorrências prose canon**

## PENDÊNCIAS RESTANTES T1-T5

- "Patch Zero" sem hífen (natureza-matematica-rigida-deep:97) — T1 v2 LV-T1v2-01
- "Janelarum" como cidade autônoma (deep/settings/01-cidade:35) — verificar canon

## CASCATA FIX RECOMENDADA (~1.250 edits 35 docs P0+P1+P2)

### P0 (livro publicável)
- BIBLE-V1-CAPA + GLOSSARIO + STRUCTURE + BIBLE-V2-STRUCTURE: ~320 edits

### P1 (narrative prose canon central)
- factions + lore-bible + foreshadowing + arco-principal + tradicoes-cultura +
  timeline + comic-reliefs + characters/* + environments/* + diary/*: ~800 edits

### P2 (deep-lore maçom literal)
- 9 violações maçom + criar termo "Selo dos Quatro Instrumentos": ~25 edits

### P3 (operacional opcional)
- TODO + CLAUDE + AUDIT-T*-V2 + INCOHERENCES headers/meta: ~250 edits (não-bloqueador)

### P4 (verificações T1-T5 pendentes)
- Patch Zero sem hífen + Janelarum: ~2 edits

## QUOTE CANON TEXTREVIEW §1 v2

"O canon é ABSOLUTO. Se tiver de escolher entre as regras de revisão e o canon, escolha o canon."

Exceção `in-world-docs.md` canon supremo preservada. Demais 60+ docs sob anti-pattern global.

## DECISÕES ONE-WAY DOOR PENDENTES (6)

1. Escopo cascata: P0+P1+P2 (1.250 edits) OU + P3 (1.580 edits)?
2. Glossário V1 rótulos de ordem fechada: nota editorial OU purge?
3. Selo Quatro Engenheiros: novo termo in-world?
4. Capa V1 ornamento: redesenhar descrição?
5. Cross-X purge prose livro?
6. §3.6 v2 travessão diálogo Vol 2 contos: memo dedicado?

## STATUS

**BLOQUEADO** publicação livro Vol 1 + Vol 2 até cascata P0+P1+P2 aplicada.

Cascata massiva — estimativa 30-50 dispatches narrative-writer.

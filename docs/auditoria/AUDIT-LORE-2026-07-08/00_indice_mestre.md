# AUDIT-LORE 2026-07-08 - Auditoria profunda do lore (índice mestre)

> **Dono do dossiê:** internal-auditor. **Executores:** revisor-textual (conteúdo, lotes L1/L5/L6/L8a/L8b/L9) + Cosimo (memórias + organização, fatias M1/M2), disparados pela thread principal. **Fase atual: CONSOLIDADA** (relatórios brutos preservados em `raw/`).
> Predecessora: F5-BK.AUDIT.FULL T1-T10 v2 (2026-05-22, `docs/auditoria/AUDIT-T*-V2.md`) + tracker vivo `docs/narrative/INCOHERENCES.md`.
>
> ## ✅ REMEDIAÇÃO CONCLUÍDA 2026-07-08
> Todas as 5 ondas executadas (11 decisões do criador em `DECISOES-CRIADOR.md`). **Onda 1** mecânico (CHARS/PLACES/timeline/CLAUDE.md/_INDEX/rename npcs-antagonistas/C20). **Onda 2** 13 memórias stale + promoção da regra GUI-médico a global + Gus "Dragon" reconciliado. **Onda 3** prosa deep (~35 fixes de cronologia/nome) + fusão D2 (morte do Aldebrando = Catedral Menor de Atelaiá + Sterling, expedição do arco preservada). **Onda 4** terminologia: Deck Rúnico→Codex de Conjuros + Beat Ten→Etapa 1/Beat 8 (~22 docs). **Onda 5** delabelização: Fibonacci→"sequência numérica recorrente", abelha-espiral, esquadro/compasso→"instrumentos de medida pareados", olho-no-triângulo→"símbolo ocular cerimonial" (narrativa+design+book). **Verificação final:** 0 rótulos de easter egg em docs públicos; residuais só meta-docs do rename + sub-beat "Ten" legítimo. **Decisões-leve do criador:** manter o cross-ref do nome de memória no backlog (risco baixo) e manter as citações "Dan Brown" no book (bibliografia declarada). Estado Auditado de todos os capítulos: ✓ remediado.

## SUMÁRIO EXECUTIVO

**84 achados brutos + 1 nota promovida, 17 fundidos por dedup cross-lote = 68 achados consolidados.**

| Sev | Total | C1 Cronologia | C2 Personagens | C3 Lugares | C4 Cross-refs | C5 Canon-vs-canon | C6 Easter eggs | C7 Memórias | C8 Organização |
|-----|-------|----|----|----|----|----|----|----|----|
| 🔴 CRÍTICO | **24** | 6 | 4 | 4 | 1 | 4 | 1 | 2 | 2 |
| 🟠 IMPORTANTE | **36** | 11 | 7 | 3 | 0 | 1 | 0 | 10 | 4 |
| 🟢 COSMÉTICO | **8** | 0 | 3 | 1 | 0 | 0 | 0 | 3 | 1 |
| **Total** | **68** | 17 | 14 | 8 | 1 | 5 | 1 | 15 | 7 |

**Parecer geral.** O corpus (~680k palavras, 112 docs + 68 memórias) está estruturalmente são: axiologia sem inversão, matriz de linguagens 100%, cross-refs quase intactas (1 quebra em 1018), INCOHERENCES.md exemplar. As falhas concentram-se em 4 classes sistêmicas, todas tratáveis:

1. **Retrofit incompleto entre camadas** (a maior classe): o deep-lore de 2026-05-19 e os docs centrais divergiram nos dois sentidos, incluindo **3 REGRESSÕES de itens já RESOLVIDOS** em INCOHERENCES (C2 Bento 4/7 em settings03+bento-requiem; C4 Mateus filho/sobrinho em settings06; C9 Yakov na timeline). Lição estrutural: correção de canon exige passe de propagação (grep do fato no corpus inteiro) antes de fechar.
2. **Erros em docs-âncora que amplificam**: `CHARS.md:136` (typo -11/-3, confirmado por 4 lotes independentes), Tao Berisi "adulto", Belinor "materna" na timeline, ref quebrada em `PLACES.md:70`.
3. **Migrações terminológicas nunca propagadas**: "Deck Rúnico" (20+ docs) e "Beat Ten" (~20 docs), viraram 2 PROGRAMAS de retrofit condicionados a decisão do criador.
4. **Easter eggs ROTULADOS em repo público** (delabelização nunca rodou como passe único): 1 PROGRAMA crítico full-corpus (rótulo "Fibonacci" em 20+ docs; 3 rótulos maçônicos), inventário no capítulo 06.

Memórias: 15 achados tocando ~18 das 68 (2 críticos: nome do protagonista contradito, caminho de código errado). Organização: CLAUDE.md e `deep/_INDEX.md` são mapas defasados (era Godot / pasta antagonists invisível), fixes mecânicos de alto retorno.

**Decisões que exigem o criador (Onda 0, detalhe em `99_remediacao.md`):**

- **D1** Causa mortis de Salviano: acidente vs insuficiência cardíaca (AL-C5-01).
- **D2** Morte de Aldebrando: local e circunstância, split 2x2 (AL-C5-02).
- **D3** Árvores Boróstoma/Janor: Orla Recursiva vs Núcleo Mandelbrot (AL-C3-02).
- **D4** Programa Deck Rúnico: retrofit 20+ docs OU reverter lore-bible §7.10 (AL-C5-03).
- **D5** Programa Beat Ten: formalizar sinônimo OU trocar em ~20 docs (AL-C5-04).
- **D6/D7** Delabelização: escopo em docs de design + novo nome da "abelha-Fibonacci" (AL-C6-01).
- **D8-D11** (decisões-lite): hook travessão antologia, .glb legado, promoção da memória GUI-médico a global, rename npcs-antologia.

**Prontidão:** dossiê pronto para a rodada de decisões do criador; ondas 1-4 de remediação destravadas na sequência (prosa = narrative-writer; mecânicos = main-thread; memórias = memo; programas = passe único). Nenhum crítico sem plano.

## Cobertura desta execução

- Recebidos e consolidados: L1 (canon central, 8 docs), L5 (characters/), L6 (deep/characters + antagonists), L8a (deep/settings), L8b (environments x settings), L9 (deep/factions + factions.md), M1 (31 memórias de lore), M2 (37 memórias restantes + C8) + script mecânico L0 (refs).
- Nota cross-lote não coberta por lote executado: Cleomir Vasta em `BIBLE-V2-INDICE.md:107` (promovida a AL-C2-14, verificação pendente). O miolo de `docs/book/` (11 docs) não teve lote dedicado nesta rodada; recomenda-se varredura complementar quando o book for tocado.

## Escopo

- **Corpus lore:** 112 arquivos .md (~680k palavras): `sinopse.md`, `CHARS.md`, `PLACES.md`, `docs/narrative/` (98), `docs/design/` (pillars, gdd, brainstorm-backlog), `docs/book/` (11).
- **Memórias:** 68 arquivos em `~/.claude/projects/-...-gusworld/memory/` (índice `MEMORY.md` + 67 tipadas; 31 apontam para arquivos de lore).
- **Motivação:** crescimento do corpus gerou risco de esquecimentos e contradições (ex. reais: idade Helíaco Vyr -950 vs -833; renames `cosmologia-deep.md` com refs residuais).

## Rubrica de severidade (com recalibração da consolidação)

| Sev | Critério |
|---|---|
| 🔴 CRÍTICO | Contradição que **propaga**: erro em doc-âncora (CHARS/PLACES/timeline/lore-bible/sinopse); erro replicado em 2+ docs; erro no doc-autoridade do próprio assunto; REGRESSÃO de INCOHERENCES resolvido; ref quebrada em âncora; memória que contradiz canon vigente; easter egg rotulado em doc público |
| 🟠 IMPORTANTE | Inconsistência local/não-propagante: aritmética intra-doc; linha solta em doc não-autoridade; gap de âncora (entrada faltante); foreshadow órfão; memória stale não-contraditória; contradição entre memórias |
| 🟢 COSMÉTICO | Ortografia/grafia/formatação; duplicação inofensiva; adição não-conflitante; achado de baixa confiança pendente de confirmação |

Recalibração aplicada na consolidação: 9 achados brutos rebaixados de CRÍTICO (intra-doc/locus único) e 2 promovidos; achado sem evidência dupla não entrou (1 mantido explicitamente como baixa-confiança: AL-C2-13).

## Arquivos do dossiê

| Arquivo | Conteúdo | Achados |
|---|---|---|
| `00_indice_mestre.md` | este índice + sumário executivo | - |
| `01_cronologia.md` | C1: datas/idades cross-doc | 17 |
| `02_personagens.md` | C2: personagens vs CHARS.md + housekeeping | 14 |
| `03_lugares.md` | C3: lugares vs PLACES.md + gaps | 8 |
| `04_crossrefs.md` | C4: integridade de refs | 1 |
| `05_canon-vs-canon.md` | C5: contradições de fato + programas terminológicos | 5 |
| `06_easter-eggs.md` | C6: programa de delabelização | 1 |
| `07_memorias.md` | C7: memórias vs realidade | 15 |
| `08_organizacao.md` | C8: CLAUDE.md, índices, registro | 7 |
| `99_remediacao.md` | plano em ondas (0 decisões / 1 prosa / 2 mecânicos / 3 memórias / 4 programas), Estado por achado | 68 |
| `raw/` | 8 relatórios brutos dos executores (L1, L5, L6, L8a, L8b, L9, M1, M2) | 84 |
| `BRIEF-A-revisor.md`, `BRIEF-B-cosimo.md` | briefs da Fase 2 (histórico) | - |

## Formato de achado (usado nos capítulos)

`AL-C<cap>-<nn> | Sev | flags (REGRESSÃO/DECISÃO/PROGRAMA) | Fonte A (arquivo:linha) x Fonte B (arquivo:linha) | conflito | cascata | fix | origem (IDs brutos) | Estado (—)`

IDs brutos dos executores: `AL-L<lote>-<nn>` (revisor-textual) e `ML-<nn>` (Cosimo; M1 e M2 colidiram na numeração, renumerados como AL-C7-nn/AL-C8-nn na consolidação).

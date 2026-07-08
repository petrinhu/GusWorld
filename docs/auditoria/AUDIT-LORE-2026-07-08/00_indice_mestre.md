# AUDIT-LORE <data> — Auditoria profunda do lore (índice mestre)

> **Dono do dossiê:** internal-auditor. **Executores:** revisor-textual (conteúdo, Lotes L1-L11) + Cosimo (memórias, Fatias M1-M2), disparados pela thread principal.
> **Renomear esta pasta** para `AUDIT-LORE-<AAAA-MM-DD>` na abertura da execução (data preenchida pelo orquestrador).
> Predecessora: F5-BK.AUDIT.FULL T1-T10 v2 (2026-05-22, `docs/auditoria/AUDIT-T*-V2.md`) + tracker vivo `docs/narrative/INCOHERENCES.md`.

## Escopo

- **Corpus lore:** 112 arquivos .md (~680k palavras): `sinopse.md`, `CHARS.md`, `PLACES.md`, `docs/narrative/` (98), `docs/design/` (pillars, gdd, brainstorm-backlog), `docs/book/` (11).
- **Memórias:** 68 arquivos em `~/.claude/projects/-...-gusworld/memory/` (índice `MEMORY.md` + 67 tipadas; 31 apontam para arquivos de lore).
- **Motivação:** crescimento do corpus gerou risco de esquecimentos e contradições (ex. reais: idade Helíaco Vyr -950 vs -833; renames `cosmologia-deep.md` → `cosmologia-formal-deep.md` / `cosmologia-origem-deep.md` com refs residuais — 1 já achada em `PLACES.md:70`).

## Rubrica de severidade

| Sev | Critério |
|---|---|
| 🔴 CRÍTICO | Contradição canon-vs-canon que **propaga**: data/idade/nome/fato estrutural divergente entre dois docs canônicos; aritmética de cronologia impossível; ref quebrada em doc canônico central (CHARS/PLACES/timeline/lore-bible); memória que **contradiz** o canon vigente; easter egg **rotulado** em doc público |
| 🟠 IMPORTANTE | Inconsistência local ou não-propagante: aritmética interna de um único doc; ref `arquivo:linha` com arquivo certo mas linha defasada; arco/foreshadow órfão sem payoff nem registro; memória stale (fato superado mas não contraditório); memória no tipo/escopo errado; duplicação divergente entre memórias |
| 🟢 COSMÉTICO | Ortografia/gramática/formatação; duplicação inofensiva; inconsistência de convenção de ref; índice `_INDEX.md` incompleto sem link morto |

Regra: achado sem evidência dupla (as DUAS fontes citadas com arquivo:linha) não entra no livro. Nenhum 🔴 fecha sem plano de remediação + re-teste.

## Capítulos

| Cap | Tema | Executor | Herda de |
|---|---|---|---|
| C1 | Cronologia e aritmética de datas/idades cross-doc | revisor-textual | T3-v2 |
| C2 | Personagens vs CHARS.md (nome exato, idade, traço, aparato) | revisor-textual | T1-v2 |
| C3 | Lugares vs PLACES.md | revisor-textual | T2-v2 |
| C4 | Integridade de cross-refs (1018 refs backtick; 217 com nº de linha; renames) | mecânico (script L0) + revisor-textual | T8-v2 |
| C5 | Contradições canon-vs-canon (fatos, facções, magia, axiologia) | revisor-textual | T7/T10-v2 + INCOHERENCES.md |
| C6 | Easter eggs velados em docs públicos (zero rótulo) | revisor-textual | T5-v2 |
| C7 | Memórias ↔ arquivos (ponteiros, staleness, tipo/escopo, duplicação, index) | Cosimo | novo |
| C8 | Organização: índices (`_INDEX.md`), CLAUDE.md defasado, localização de docs | Cosimo + internal-auditor | novo |

## Arquivos do dossiê (preencher na consolidação)

- `00_indice_mestre.md` (este) + sumário executivo + contagem por severidade
- `01_cronologia.md` … `06_eastereggs.md` (achados do revisor-textual, por capítulo)
- `07_memorias.md` + `08_organizacao.md` (achados do Cosimo)
- `99_remediacao.md` (plano rastreado por achado, Estado Auditado `—`/`✓`/`⚠`)

## Formato de achado (obrigatório, todos os executores)

`ID | Sev | Capítulo | Fonte A (arquivo:linha + citação) | Fonte B (arquivo:linha + citação) | Descrição do conflito | Cascata (docs afetados) | Remediação sugerida | Estado (—)`

IDs: `AL-<lote>-<nn>` (revisor-textual), `ML-<nn>` (Cosimo).

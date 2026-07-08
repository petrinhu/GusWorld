# AUDIT-LORE 2026-07-08 — Decisões do criador supremo (Onda 0)

As 11 decisões que destravam a remediação. Colhidas via AskUserQuestion em 3 blocos.

## Splits de canon
- **D1 — Salviano Alencar (pai do Dante), causa mortis oficial (-8):** **INSUFICIÊNCIA CARDÍACA.** Retrofitar `timeline.md:124`, `diary/entries-mapas-timeline.md:520`, `diary/entries-fichas-bestiary.md:224` (que dizem "acidente/overdose") pra bater com o deep-lore.
- **D2 — Aldebrando Chevalier (pai do Bento), morte:** **Catedral Menor de Atelaiá, Vigília restrita de 12 mestres, interferência ressonante calibrada (onze sobreviveram).** Retrofitar `timeline.md:140`, `lore-bible.md:511`, `factions.md:128` (que dizem "catedral perdida / queda em ruína"). Registrar como INCOHERENCES C20 resolvido.
- **D3 — Árvores Boróstoma/Janor:** ficam no **Núcleo Mandelbrot (5 árvores, settings08 canon)**. `settings02` passa a descrever árvores/motivo DIFERENTE na Orla Recursiva (não reusar Boróstoma/Janor lá).

## Programas de terminologia (retrofit amplo)
- **D4 — "Deck Rúnico":** **MIGRAR** para o vocabulário Glyph/Token/Conjuro/Codex (dropar "rúnico/runa", alinha Pillar 1). Retrofitar `pillars.md`, `gdd.md` + ~20 docs. NÃO reverter a lore-bible §7.10.
- **D5 — "Beat Ten":** **TROCAR** por "Etapa 1 / Beat 8 (Clímax)" em ~20 docs + o elemento de UI literal (`diary/ui-spec.md:87`). Não formalizar como sinônimo.

## Easter eggs (delabelização)
- **D6 — Escopo:** **narrativa + design** (todos os docs públicos, incl. `docs/design/mecanicas/*`). `docs/_secret` e `docs/auditoria` ficam FORA (já privados).
- **D7 — "abelha-Fibonacci" renomeada para "abelha-espiral"** (em ~3 docs: settings02, settings08, patch-zero-deep + cascata). Demais rótulos velados: "padrão Fibonacci"→"padrão numérico recorrente"; "esquadro e compasso" nomeados→perífrase velada (como fir.md já faz); "olho-no-triângulo" → remover a cláusula, manter só "olho refratado em prisma".

## Higiene / metadados
- **D8 — Hook travessão:** **marcar a memória `feedback_dialogo_travessao_vol2` como SUPERADA** (a prática já usa aspas; a exceção do hook não é necessária). Não mexer no hook.
- **D9 — Arquivos .glb (Gus/Yakov/movimento):** **MANTER.** Serão usados como **arte conceitual nos livros futuros**. Documentar esse propósito nas 2 memórias (`reference_codeberg_lfs`, `reference_libs_vendorizadas`) e reconciliar a contradição (não são "sendo deletados"; são acervo p/ os livros).
- **D10 — Memória `feedback_nao_automacao_gui_ambiente_medico`:** **PROMOVER pra global** `~/.claude/memory/` (regra de segurança do ambiente médico, vale pra todo projeto), com o GusWorld referenciando por cross-ref.
- **D11 — `deep/antagonists/npcs-antologia.md`:** **RENOMEAR para `npcs-antagonistas.md`** (tira a colisão com a pasta `antologia/`) + atualizar cross-refs + o `deep/_INDEX.md`.

## Fixes que NÃO precisavam de decisão (mecânicos, entram na remediação direto)
- CHARS.md:136 Cauã "-11"→"-3" (typo de âncora, 4 lotes confirmaram).
- CHARS.md: mesclar entrada duplicada de Joaquim Bartolomeu (:87 e :140); Tao Berisi = criança 12 (remover "NPC adulto" :235); Belinor "avó paterna"; adicionar NPCs faltantes (Bito Caldeira, Bem-Te-Vi, Helena Sirinhaém, Bel Galvão, Pirilampo, Tata Bruno) §7.
- timeline.md:120 Yakov "~25"→"~31" (regressão C9); timeline.md Atelaiá -147/-150 alinhar; Belinor "materna"→"paterna" (:201) + comic-reliefs "avô"→"avó" (:977).
- PLACES.md:70 ref `cosmologia-deep.md:67` → `deep/eras/cosmologia-origem-deep.md`; adicionar entrada "Universidade Pública" §5.
- CLAUDE.md: reescrever "Estrutura de repositório" (GusEngine/ + resources/, engine/game legado dormente) + status dos docs canônicos + árvore characters/ (13 arquivos).
- deep/_INDEX.md: regravar do find real (7 subpastas com nomes defasados + pasta antagonists/ invisível).
- ~13 memórias stale (asset_paths.hpp core/, RmlUi 6.3, glintfx header v0.6.0, gameplay_engineer caveat, CONTRACT.md existe, dificuldade 4 níveis cross-ref, economia §3.3/3.4, brainstorm-backlog seed#1 canonizado, nome_gus_canon Dragon, session_atual, etc.).
- Prosa deep (idades/datas): Bento -10/4→-7/7, Iara+Sérgio, Cauã Apex/Nexus, Hiato 600→550, Depto Contenção -8→-4, Dante missões, gus-dragon preservar "Gustaf", Pântano Markov grafia, etc. → SEMPRE narrative-writer, agrupado por arquivo.

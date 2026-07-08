# BRIEF B — Cosimo (auditoria da fatia de MEMÓRIAS, capítulos C7/C8)

> Você recebeu uma FATIA no prompt (M1 ou M2). Leia este preâmbulo + o bloco da sua fatia, execute, e devolva os achados como TEXTO. READ-ONLY: não edite memória nem lore, não commite, não pushe.

## Preâmbulo comum

Você é o Cosimo executando uma fatia da AUDIT-LORE do GusWorld (capítulos C7/C8).
- Dir de memórias: `/home/petrus/.claude/projects/-home-petrus-IDrive-Documentos-projetos-claudebrain-Projects-gusworld/memory/`
- Dir do projeto: `/home/petrus/IDrive/Documentos/projetos_claudebrain/Projects/gusworld`

CONTEXTO: 68 arquivos (`MEMORY.md` + 67 tipadas). Check mecânico do índice já feito: bidirecionalmente íntegro (toda tipada listada; nenhum link morto). Seu trabalho é o que máquina não pega: CONTEÚDO stale, contradição com canon, escopo errado, duplicação. Renames de hoje (2026-07-08): `ontologia/cosmologia-deep.md` → `cosmologia-formal-deep.md`; `eras/cosmologia-deep.md` → `eras/cosmologia-origem-deep.md`. Grep simples não achou o nome antigo nas memórias, mas re-verifique variantes (path parcial, wikilink, menção em prosa).

**PARA CADA MEMÓRIA, 5 checks:**
- **(a) PONTEIROS:** todo arquivo/path/comando citado existe hoje com esse nome? (use find/ls, não confie na memória). Inclui paths de assets, scripts (`~/.local/bin/rag-safe`), marcadores (`.bigtech-porte`), pastas.
- **(b) FATO vs CANON:** a afirmação canônica da memória bate com o doc canônico vigente? Abra o doc-alvo e confira o fato específico (idade, data, nome, mecânica, decisão). Contradição = **CRITICO**; fato superado sem contradição ativa = **IMPORTANTE** (stale).
- **(c) TIPO/ESCOPO:** `project_` = fato canônico do projeto; `feedback_` = norma de processo; `reference_` = referência técnica. Conteúdo no prefixo errado = IMPORTANTE. Memória de escopo global vivendo só aqui (ou vice-versa) = IMPORTANTE.
- **(d) INDEX:** a LINHA-RESUMO da memória no `MEMORY.md` ainda descreve o conteúdo atual do arquivo? (resumo defasado é a forma mais sorrateira de stale).
- **(e) DUPLICAÇÃO/SOBREPOSIÇÃO:** pares que tratam do mesmo tema divergem?

**Clusters de sobreposição a verificar explicitamente:**
- `project_fibonacci_easter_egg` × `project_eastereggs_maconaria_canonica` × `project_alvo_palavras_pos_era_1` (Fibonacci descontinuado em CONTAGEM de palavras mas mantido no TEXTO — as três compatíveis?)
- `project_nome_gus_canon` × `project_personagens` × `feedback_nomes_personagens_canonicos` × `project_familia_vance_canonica` × `CHARS.md`
- `project_morte_dificuldade_canon` × `project_save_dungeon_pem_faraday`
- `project_em_dash_excecao` × `feedback_dialogo_travessao_vol2` (escopos disjuntos e claros?)
- `project_pillars_canonicos` × `docs/design/pillars.md`
- `project_economia_canon` × `docs/design/mecanicas/economia.md`
- `project_terminologia` × `deep/magic/glyph-token-conjuro-codex-deep.md`
- `reference_brainstorm_backlog` × `docs/design/brainstorm-backlog.md` (a memória lista as mesmas seeds?)
- `project_gusworld_overview` (banner C++/SDL3 no topo + corpo era-Godot): o banner cobre TODAS as afirmações defasadas do corpo?
- Memórias DORMENTE (godot_docs, godot_csharp, funplay, csharp_lsp): dormência clara, sem instrução ativa vazando?
- `project_session_atual`: snapshot condiz com o estado real do repo (git log)?

**FORMATO:** `ML-<nn> | severidade | check (a-e) | memória (arquivo) | evidência da memória (citação) | evidência do canon/disco (arquivo:linha ou saída de comando) | descrição | fix sugerido (atualizar memória / atualizar canon / mover / fundir / apagar — NÃO aplicar)`. Memória limpa = liste no rol "verificadas sem achado" (cobertura auditável: as memórias da sua fatia têm de aparecer no relatório, com ou sem achado).

---

## Fatia M1 — as 31 memórias que apontam para lore (checks a+b+d+e completos)

```
feedback_arquivos_importantes_pasta_projeto, feedback_deep_lore_colaborativo_rag_visivel,
feedback_deep_lore_sempre_narrative_writer, feedback_dialogo_travessao_vol2,
feedback_nomes_personagens_canonicos, feedback_reforcar_caracteristicas_canonicas_geracao,
project_axiologia_canonica, project_censura_palavroes_canonica, project_conlang_sylvarin,
project_dragon_victory_canon, project_economia_canon, project_em_dash_excecao,
project_familia_vance_canonica, project_fibonacci_easter_egg, project_gusworld_overview,
project_image_prompts_nano_banana, project_inspiracoes_pessoais, project_locomotion_animacao,
project_morte_dificuldade_canon, project_nome_gus_canon, project_personagens,
project_pillars_canonicos, project_save_dungeon_pem_faraday, project_session_atual,
project_terminologia, reference_brainstorm_backlog, reference_pixellab_mcp,
reference_prompts_musica_suno, reference_rag_elvish, reference_rag_scoring_oom,
reference_vibe_composer_midi_mcp
```

## Fatia M2 — as 36 memórias restantes (checks a+c+d+e) + capítulo C8 (organização)

Restantes = todas as demais tipadas (processo/técnica: hitbox, glintfx, codeberg-lfs, libs vendorizadas, etc. — liste-as via `ls` do dir de memórias e subtraia as 31 de M1). Adicionalmente, **C8 (organização)**:
- **C8.1** `CLAUDE.md` do projeto vs realidade: `grep "AGUARDA REVISÃO USER" docs/` = 0, mas o CLAUDE.md ainda marca pillars/gdd/architecture assim; a seção "Estrutura de repositório" lista a árvore da Fase 1 — confira contra o `find` real e liste TODA divergência estrutural (IMPORTANTE, não corrigir).
- **C8.2** `_INDEX.md` de `deep/`, `diary/`, `environments/`: todo arquivo do dir está no índice e todo item do índice existe?
- **C8.3** Docs de lore fora de lugar (lore em docs/design/, design em docs/narrative/, characters/ raso vs deep/characters/ com escopo confuso).
- **C8.4** `INCOHERENCES.md`: todos os itens têm status (RESOLVIDO/aberto)? Abertos antigos sem dono = IMPORTANTE.

# 07. Memórias vs realidade (C7)

> Consolidação: internal-auditor, 2026-07-08. Fontes brutas: `raw/M1.md` (31 memórias que apontam lore) + `raw/M2.md` (37 restantes). Executor: Cosimo. Cobertura: 68/68 memórias.
> Nota de consolidação: M1 e M2 usaram AMBOS a série de IDs `ML-01..`, colidindo entre si; aqui renumerados como `AL-C7-nn` (origem bruta anotada em cada achado: M1-ML-nn / M2-ML-nn).

## Contexto e método

Cada memória tipada verificada contra o estado real do repo/canon: (a) ponteiros arquivo/linha, (b) staleness de fato, (c) tipo/escopo, (d) snapshot, (e) contradição com canon ou entre memórias. 52 de 68 memórias verificadas SEM achado (incl. cluster Fibonacci/maçonaria/alvo-palavras compatíveis, dragon_victory cross-checado OK, 4 DORMENTE com banner claro).

## Achados (15: 2 críticos, 10 importantes, 3 cosméticos)

### AL-C7-01 | 🔴 CRÍTICO

`project_nome_gus_canon.md` diz "'Vector' = codinome" e nunca cita Dragon; contradiz `character-spec-gus.md:6` + `CHARS.md:22` + memórias `project_dragon_victory_canon` / `project_personagens` / `feedback_nomes_personagens_canonicos` (codinome = **"Dragon"**; "Vector Tavus" = nome do meio). Memória fechada como one-way door 1 dia DEPOIS do Dragon canon, sem reconciliar. Memória que contradiz canon vigente = crítico (envenena sessões futuras). Fix: atualizar a memória. Relaciona AL-C2-10. Origem: M1-ML-01. Estado: —

### AL-C7-02 | 🟠 IMPORTANTE | DECISÃO-LITE

`feedback_dialogo_travessao_vol2.md` prescreve exceção no hook `no_mdash.py` para travessão em `antologia/`; o hook só tem exceção para `in-world-docs.md` e os contos usam ZERO em-dash (aspas). Regra letra morta, não contradiz canon. Fix: decidir com o líder: implementar a exceção no hook OU marcar a memória como superada. Origem: M1-ML-02 (recalibrado de CRÍTICO: stale, não contraditória de canon). Estado: —

### AL-C7-03 | 🟠 IMPORTANTE

`project_pillars_canonicos` + `project_gusworld_overview` dizem dificuldade "Normal/Hard" (2 níveis); superado por `project_morte_dificuldade_canon` (2026-07-03: 4 níveis Fácil/Médio/Difícil/Hardcore). Pendência associada: `docs/design/pillars.md:134-137` ainda no modelo antigo (canonizar os 4 modos lá é fix de doc, listado na onda mecânica). Fix: cross-ref nas 2 memórias. Origem: M1-ML-03. Estado: —

### AL-C7-04 | 🟠 IMPORTANTE

`project_economia_canon` cobre só até §3.2; `economia.md` cresceu §3.3 (safe mode/dívida, 2026-06-24) e §3.4 (auto-resolve, 2026-06-25). Fix: atualizar a memória. Origem: M1-ML-04. Estado: —

### AL-C7-05 | 🟠 IMPORTANTE

`project_session_atual` congelado em `c6b0522` + "próximo=SAVE-LOAD-UI"; HEAD real 15 commits à frente. Snapshot enganoso na abertura de sessão nova. Fix: atualizar no fim da sessão corrente. Origem: M1-ML-05. Estado: —

### AL-C7-06 | 🟠 IMPORTANTE

`reference_brainstorm_backlog` diz "nada é canon fechado"; a seed #1 já tem frame fechado + canonizado (`cosmologia-origem-deep.md`). Fix: anotar seed #1 como parcialmente canonizada. Origem: M1-ML-06. Estado: —

### AL-C7-07 | 🟢 COSMÉTICO

`project_session_atual` cita "GDD §5.4" inexistente (gdd.md §5 sem subdivisão); fato plausível, locator errado. Fix junto com AL-C7-05. Origem: M1-ML-07. Estado: —

### AL-C7-08 | 🟢 COSMÉTICO

4 memórias (`feedback_reforcar_caracteristicas_canonicas_geracao`, `feedback_nomes_personagens_canonicos`, `project_image_prompts_nano_banana`, `project_gusworld_overview`) citam `Resources/gusworld/character-spec-*.md` como se fosse relativo ao projeto; é raiz do vault. Fix: qualificar o caminho nas 4. Origem: M1-ML-08. Estado: —

### AL-C7-09 | 🔴 CRÍTICO

`feedback_caminhos_assets_centralizados` aponta header em `GusEngine/app/include/gus/app/asset_paths.hpp` ns `gus::app`; real: `GusEngine/core/include/gus/core/asset_paths.hpp` ns `gus::core` (movido para a fundação). Memória de referência de código com caminho E namespace errados induz edit/include errado. Fix: atualizar caminho + namespace. Origem: M2-ML-01. Estado: —

### AL-C7-10 | 🟠 IMPORTANTE

`project_rmlui_ui_stack` diz RmlUi pin 6.2; real 6.3 (SHA, ADR-010). Fix: 6.3. Origem: M2-ML-02. Estado: —

### AL-C7-11 | 🟢 COSMÉTICO

`reference_glintfx_api` com cabeçalho v0.3.1 e corpo já em v0.6.0 (pin real). Fix: atualizar o header. Origem: M2-ML-03. Estado: —

### AL-C7-12 | 🟠 IMPORTANTE | DECISÃO-LITE

Contradição entre memórias: `reference_codeberg_lfs` trata os `.glb` (Gus/Yakov) como "setup canônico vigente"; `reference_libs_vendorizadas` diz que estão "sendo DELETADOS". Real: os 3 .glb existem intactos. Fix: líder decide (manter como legado 3D documentado OU deletar de fato) e as 2 memórias são reconciliadas. Origem: M2-ML-04 + M2-ML-05 (fundidos). Estado: —

### AL-C7-13 | 🟠 IMPORTANTE

`project_agente_gameplay_engineer` mantém caveat "agent ainda declara Godot"; o agent já é C++20/SDL3. Fix: remover a sinalização. Origem: M2-ML-06. Estado: —

### AL-C7-14 | 🟠 IMPORTANTE

`project_qa_deploy_disciplina` diz "sem CONTRACT.md"; existe hoje. Fix: atualizar a seção "Estado atual 2026-05-16". Origem: M2-ML-07. Estado: —

### AL-C7-15 | 🟠 IMPORTANTE | DECISÃO-LITE

`feedback_nao_automacao_gui_ambiente_medico` auto-declara escopo GLOBAL/máquina mas vive só no GusWorld e não está no índice global. Regra de segurança de ambiente médico merece escopo real global. Fix: promover para `~/.claude/memory/` (com aval do líder, é o espaço global dele). Origem: M2-ML-08. Estado: —

## Conclusão

Os 15 achados tocam ~18 arquivos de memória (de 68, ~26%), sendo 2 críticos (uma contradiz canon de nome do protagonista, outra aponta código para caminho errado). Padrão: memórias fechadas como "canônicas" na véspera de decisões que as superaram, sem passe de reconciliação. Recomendação estrutural: ao canonizar decisão nova, grep nas memórias por termos afetados antes de fechar a sessão (mesma lição do retrofit de lore).

# 99. Plano de remediação rastreado (AUDIT-LORE 2026-07-08)

> Dono: internal-auditor. 68 achados consolidados (de 84 brutos + 1 nota promovida, 17 fundidos por dedup). Estado Auditado: `—` pendente / `✓` remediado e re-testado / `⚠` remediado sem re-teste ou parcial.
> Regras: nenhum 🔴 fecha sem re-teste (grep do fato corrigido no corpus INTEIRO, não só no arquivo tocado). Prosa de lore = SEMPRE `narrative-writer` (regra do projeto). Push só com autorização do líder.

## Onda 0 - Decisões do criador (bloqueia partes das ondas 1 e 4)

Splits sem resposta certa; via AskUserQuestion, linguagem simples, opção recomendada primeiro.

| # | Achado | Decisão | Recomendação do auditor |
|---|--------|---------|--------------------------|
| D1 | AL-C5-01 🔴 | Causa mortis oficial de Salviano: "acidente/overdose" vs "insuficiência cardíaca" | Insuficiência cardíaca (maioria 4x3 dos docs; endosso dos 2 executores) |
| D2 | AL-C5-02 🔴 | Morte de Aldebrando: Catedral Menor de Atelaiá/Vigília vs catedral perdida/queda em ruína (split 2x2) | Sem maioria; apresentar os 2 quadros ao criador |
| D3 | AL-C3-02 🔴 | Árvores Boróstoma/Janor: Orla Recursiva (2 árvores) vs Núcleo Mandelbrot (5 árvores) | Núcleo Mandelbrot fica com o par nomeado; Orla ganha motivo próprio |
| D4 | AL-C5-03 🔴 | PROGRAMA Deck Rúnico: retrofit de "rúnico" em 20+ docs OU reverter lore-bible §7.10 (sinônimo histórico legítimo) | Reverter §7.10 como sinônimo histórico é 20x mais barato; retrofit só se o criador quiser o termo morto |
| D5 | AL-C5-04 🟠 | PROGRAMA Beat Ten: formalizar como sinônimo canônico do Twist em arco-principal.md OU trocar em ~20 docs | Formalizar sinônimo (barato); mas corrigir o literal de UI em qualquer cenário |
| D6 | AL-C6-01 (escopo) 🔴 | Delabelização: docs de design/mecânicas também perifraseiam "Fibonacci" ou ficam como termo técnico? | Perifrasear também (repo público) ou mover miolo para docs/_secret |
| D7 | AL-C6-01 (nome) 🔴 | Novo nome da espécie "abelha-Fibonacci" | "abelha-espiral" ou "abelha-áurea" |
| D8 | AL-C7-02 🟠 | Exceção de travessão p/ antologia no hook no_mdash.py: implementar ou marcar memória superada | Marcar superada (contos usam aspas, zero em-dash na prática) |
| D9 | AL-C7-12 🟠 | .glb legado (Gus/Yakov): manter documentado como legado 3D ou deletar de fato | Manter como legado documentado (barato, reversível) |
| D10 | AL-C7-15 🟠 | Promover memória GUI-médico para escopo global `~/.claude/memory/` | Promover (regra de segurança de máquina, não de projeto) |
| D11 | AL-C8-05 🟠 | Renomear `npcs-antologia.md` (colisão com pasta antologia/) | Renomear p/ npcs-antagonistas + passe de refs |

## Onda 1 - Fixes de PROSA de lore (executor: narrative-writer; agrupado por arquivo; críticos primeiro)

| ID | Sev | Arquivo(s) | Fix | Depende | Estado |
|----|-----|-----------|-----|---------|--------|
| AL-C1-03 | 🔴 REGR | `deep/settings/03-catedrais.md:43` + `deep/characters/bento-requiem.md:17,89` | Bento "quatro anos/-10" p/ "sete anos/-7" | - | — |
| AL-C1-05 | 🔴 | `characters/patch-zero.md:60` + `deep/antagonists/patch-zero-deep.md:51-53` | captura -8 p/ -3 + "8 anos de contenção" recalculado | - | — |
| AL-C1-07 | 🔴 | `characters/dante-grid.md:160,168` + `foreshadowing.md:52` + in-world doc16 | Dante "8 anos" p/ "5 anos" | - | — |
| AL-C1-09 | 🔴 | `deep/characters/iara-lumen.md:13` | "4 anos" p/ recém-nascida + Sérgio 16 p/ 12 | - | — |
| AL-C2-01 | 🔴 | `timeline.md:201` + `comic-reliefs.md:977` | Belinor materna p/ paterna; "avô" p/ "avó" | - | — |
| AL-C2-06 | 🔴 | `deep/settings/06-periferia.md:27,33` + `deep/factions/fir.md:55` | "avô" p/ "bisavô (4 gerações)" | - | — |
| AL-C2-07 | 🔴 REGR | `deep/settings/06-periferia.md:19,25` | Mateus "filho" p/ "sobrinho" | - | — |
| AL-C3-01 | 🔴 | `deep/settings/05-mirage.md:11,13,40` | "Distrito 4" p/ "Distrito 3" | - | — |
| AL-C3-06 | 🔴 | `tradicoes-cultura.md:270-280` | tabela §9: Catedral-Mãe p/ Quinta + "3 ativas + 2 saqueadas" | - | — |
| AL-C3-08 | 🔴 | `deep/factions/pelicano-branco.md:9,61` + `factions.md:415` | 3 catedrais tributárias = Era 1 não-Êxodo | - | — |
| AL-C5-01 | 🔴 | grupo perdedor (timeline+diary OU settings06+dante+fir+sterling-corp) | causa mortis Salviano unificada | D1 | — |
| AL-C5-02 | 🔴 | grupo perdedor (bento-requiem+CHARS OU timeline+lore-bible+factions) | morte Aldebrando unificada | D2 | — |
| AL-C5-05 | 🔴 | `deep/characters/caua-volt.md:11` | "Nexus-Cloud" p/ "Apex-Data" | - | — |
| AL-C1-01 | 🔴 REGR | `timeline.md:120` + `BIBLE-V1-INDICE.md:389` | Yakov "~25" p/ "~31" | - | — |
| AL-C3-02 | 🔴 | `deep/settings/02-selve-sombria.md:12,39,73` (ou 08) | Boróstoma/Janor num local só | D3 | — |
| AL-C1-04 | 🟠 | `deep/settings/03-catedrais.md:62` | "dez anos" p/ "treze anos e meio" | - | — |
| AL-C1-06 | 🟠 | `deep/antagonists/patch-zero-deep.md:51` | mandato -8 p/ -4 | - | — |
| AL-C1-08 | 🟠 | `characters/jaci-proxy.md:27` | "há 3 anos" p/ "há 8 anos" | - | — |
| AL-C1-10 | 🟠 | `timeline.md:77` vs `:196` | Atelaiá: alinhar nascimento (-147 vs -150) | - | — |
| AL-C1-11 | 🟠 | `deep/antagonists/patch-zero-deep.md:13` | Hiato "600" p/ "550" + Êxodo/colapso | - | — |
| AL-C1-12 | 🟠 | `deep/factions/npcs-antologia.md:57` | Penha: remover cláusula "quando Apex Chapter 11" | - | — |
| AL-C1-13 | 🟠 | `deep/settings/05-mirage.md:81` | Sonja "-5/62" p/ "-34/33" | - | — |
| AL-C1-14 | 🟠 | `deep/factions/ordem-recursiva.md:256,260` | Antoneta/Verônica: alinhar sucessão | - | — |
| AL-C1-15 | 🟠 | `deep/factions/ordem-recursiva.md:264` | Tarsila "34" p/ "17" | - | — |
| AL-C1-16 | 🟠 | `deep/characters/gus-dragon.md:123` | remover/realocar "55 somados" (46+42=88) | - | — |
| AL-C1-17 | 🟠 | `deep/characters/dante-grid.md:25,163` | alinhar idade de início + curva de missões | - | — |
| AL-C2-08 | 🟠 | `deep/factions/sterling-corp.md:89` | Hilário "mestre" p/ "aprendiz" | - | — |
| AL-C2-09 | 🟠 | `deep/factions/sterling-corp.md:13,15,79` | Berenger "ferroviária" p/ "QA" | - | — |
| AL-C2-10 | 🟠 | `deep/characters/gus-dragon.md` | inserir 1 menção "Gustaf" | - | — |
| AL-C3-03 | 🟠 | `environments/08-selve-profunda.md:30` | "quarenta casas" p/ "vinte e uma famílias" | - | — |
| AL-C3-04 | 🟠 | `environments/01-cidade-cyber-gotica.md:16,43,68` | placa dos 4 engenheiros num local só (recom.: átrio) | - | — |
| AL-C2-05b | 🟠 | `deep/settings/07-*.md:27,42` | gênero do ancestral de Joaquim uniformizado | - | — |
| AL-C2-11 | 🟢 | `characters/sterling-locke.md:13` | nota editorial sem "Iolanda" | - | — |
| AL-C2-13 | 🟢 | `factions.md:504` vs `CHARS.md:92` | Filomena: CONFIRMAR se conflita; se complementar, fechar | - | — |
| AL-C3-07 | 🟢 | `deep/antagonists/patch-zero-deep.md:43,73,85` | "Pântano de Markov" p/ "Pântano Markov" | - | — |

Agrupamento por arquivo p/ dispatch (menos sessões de narrative-writer): patch-zero-deep (C1-05, C1-06, C1-11, C3-07), settings03 (C1-03, C1-04), settings05 (C3-01, C1-13), settings06 (C2-06, C2-07, C5-01?), ordem-recursiva (C1-14, C1-15), sterling-corp (C2-08, C2-09), gus-dragon (C1-16, C2-10), timeline (C1-01, C1-10, C2-01).

## Onda 2 - Fixes MECÂNICOS (executor: main-thread/script; sem prosa)

| ID | Sev | Alvo | Fix | Estado |
|----|-----|------|-----|--------|
| AL-C1-02 | 🔴 REGR-adj | `CHARS.md:136` | typo "-11" p/ "-3" (confirmado por 4 lotes) | — |
| AL-C2-02 | 🔴 | `CHARS.md:235` | Tao Berisi: remover "adulto", registrar 12 anos | — |
| AL-C4-01 | 🔴 | `PLACES.md:70` | ref p/ `deep/eras/cosmologia-origem-deep.md` + linha revalidada | — |
| AL-C8-01 | 🔴 | `CLAUDE.md` | reescrever "Estrutura de repositório" (GusEngine/ real) + seção Comandos | — |
| AL-C8-02 | 🔴 | `deep/_INDEX.md` | regravar do find real + pasta antagonists/ + tabela de status | — |
| AL-C2-03 | 🟠 | `CHARS.md:83` | adicionar sobrenome Sevroski | — |
| AL-C2-04 | 🟠 | `CHARS.md` §7 | adicionar 6 NPCs (fatos transcritos das fontes) | — |
| AL-C2-05 | 🟠 | `CHARS.md:87,140` | mesclar entradas duplicadas Joaquim Bartolomeu | — |
| AL-C2-14 | 🟠 | `BIBLE-V2-INDICE.md:107` | verificar Cleomir 60+ vs 37 e corrigir índice | — |
| AL-C3-05 | 🟠 | `PLACES.md` §5 | entrada Universidade Pública | — |
| AL-C8-03 | 🟠 | `CLAUDE.md` | status de revisão dos 5 docs atualizado | — |
| AL-C8-06 | 🟠 | `pillars.md` + `gdd.md` | find-replace 47 em-dash | — |
| AL-C8-07 | 🟠 | `foreshadowing.md` ou backlog | catalogar nó de sequel settings03:72 | — |
| AL-C8-05 | 🟠 | `deep/_INDEX.md` (+rename D11) | nota de fronteira das 3 camadas characters | — |
| AL-C2-12 | 🟢 | `CHARS.md:135` | sobrenome Vanderbist de Solane | — |
| AL-C8-04 | 🟢 | `CLAUDE.md` | árvore characters/ 13 docs | — |
| (assoc. C7-03) | 🟠 | `pillars.md:134-137` | canonizar 4 níveis de dificuldade (fato já decidido 2026-07-03) | — |

## Onda 3 - Fixes de MEMÓRIA (via atualização de memória; críticos primeiro)

| ID | Sev | Memória | Fix | Estado |
|----|-----|---------|-----|--------|
| AL-C7-01 | 🔴 | project_nome_gus_canon | reconciliar com Dragon canon | — |
| AL-C7-09 | 🔴 | feedback_caminhos_assets_centralizados | caminho core/ + ns gus::core | — |
| AL-C7-03 | 🟠 | project_pillars_canonicos + project_gusworld_overview | cross-ref 4 níveis | — |
| AL-C7-04 | 🟠 | project_economia_canon | cobrir §3.3/§3.4 | — |
| AL-C7-05 | 🟠 | project_session_atual | snapshot atualizado (+ locator GDD, C7-07) | — |
| AL-C7-06 | 🟠 | reference_brainstorm_backlog | seed #1 parcialmente canonizada | — |
| AL-C7-10 | 🟠 | project_rmlui_ui_stack | pin 6.3 | — |
| AL-C7-13 | 🟠 | project_agente_gameplay_engineer | remover caveat Godot | — |
| AL-C7-14 | 🟠 | project_qa_deploy_disciplina | CONTRACT.md existe | — |
| AL-C7-02 | 🟠 | feedback_dialogo_travessao_vol2 | conforme D8 | — |
| AL-C7-12 | 🟠 | reference_codeberg_lfs + reference_libs_vendorizadas | conforme D9 | — |
| AL-C7-15 | 🟠 | feedback_nao_automacao_gui_ambiente_medico | conforme D10 | — |
| AL-C7-08 | 🟢 | 4 memórias | qualificar caminho Resources/ | — |
| AL-C7-11 | 🟢 | reference_glintfx_api | header v0.6.0 | — |

## Onda 4 - PROGRAMAS full-corpus (pós-decisão; executor: narrative-writer)

| ID | Sev | Programa | Escopo | Depende | Re-teste | Estado |
|----|-----|----------|--------|---------|----------|--------|
| AL-C6-01 | 🔴 | Delabelização easter eggs (Fibonacci + maçonaria) | ~20 docs narrativa + design (conforme D6/D7); inventário no cap. 06 | D6, D7 | grep zero-hit fora de _secret/auditoria | — |
| AL-C5-03 | 🔴 | Terminologia Deck Rúnico | 20+ docs OU só lore-bible §7.10 | D4 | grep "rúnico"/"Deck" conforme decisão | — |
| AL-C5-04 | 🟠 | Terminologia Beat Ten | ~20 docs OU só arco-principal + ui-spec | D5 | grep "Beat Ten" | — |

## Pós-remediação (fecha o ciclo)

1. Registrar D1/D2 decididos em `INCOHERENCES.md` (C20/C21) com data e resolução.
2. Re-teste global: grep por cada fato corrigido no corpus inteiro (lição das 3 regressões: correção sem passe de propagação reabre).
3. Atualizar coluna Estado desta tabela (`—` p/ `✓` só com re-teste; `⚠` se parcial) e o sumário do `00_indice_mestre.md`.
4. Commits citando `AUD-LORE`; push só com autorização explícita do líder.

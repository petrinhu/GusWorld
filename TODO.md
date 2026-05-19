# GusWorld — Planejamento

Tabela canônica de pendências e planejamento. Atualizar via skill `/tab_pendencias`.

| ID | Grupo | Descrição Técnica | Prioridade | Pré-requisito | Dificuldade | Status | Estado Auditado |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| F1.1 | F1-Concepção | Pillars + GDD 1-page (`docs/design/pillars.md`, `docs/design/gdd.md`) | Alta | — | Média | ✅ Concluído | — |
| F1.2 | F1-Concepção | Lore bible + arco principal + party + 6 companions + Sterling + Patch-Zero (`docs/narrative/`) | Alta | — | Alta | ✅ Concluído | — |
| F1.3 | F1-Concepção | Bloco G — lore expansion: timeline, in-world-docs, tradicoes-cultura, factions expandido | Alta | F1.2 | Alta | ✅ Concluído | — |
| F1.4 | F1-Concepção | Bloco F — environmental storytelling 8 settings (`docs/narrative/environments/`) | Alta | F1.2, F1.3 | Alta | ✅ Concluído | — |
| F1.5 | F1-Concepção | Bloco I — foreshadowing tracker 130 plants (`docs/narrative/foreshadowing.md`) | Alta | F1.4 | Alta | ✅ Concluído | — |
| F1.6 | F1-Concepção | Bloco H — Diário do Gus pasta modular 8 docs (`docs/narrative/diary/`) | Alta | F1.5 | Alta | ✅ Concluído | — |
| F1.7 | F1-Concepção | Viralização 5 propostas F-PROP → F126-F130 (foreshadow tracker) | Alta | F1.5 | Média | ✅ Concluído | — |
| F1.8 | F1-Concepção | Viralização 8 propostas DD-016 a DD-023 (in-world-docs) | Alta | F1.6 | Média | ✅ Concluído | — |
| F1.9 | F1-Concepção | Style guide 3D low-poly (`docs/art/style-guide.md`) | Alta | — | Média | ✅ Concluído | — |
| F1.10 | F1-Concepção | Specs canônicas 8 personagens (`Resources/gusworld/character-spec-*.md`) | Alta | F1.2 | Alta | ✅ Concluído | — |
| F1.11 | F1-Concepção | Arquitetura Godot 4 modular + build pipeline (`docs/tech/architecture.md`, `engine-modules.md`, `build.md`) | Alta | — | Alta | 🟡 Parcial | ⚠ |
| F1.12 | F1-Concepção | GDD consolidado pós-reforma (`docs/design/gdd.md`) | Alta | F1.1-F1.6 | Média | 🟡 Parcial | ⚠ |
| F1.13 | F1-Concepção | Art sheets produção 7 chars faltantes (`docs/art/characters/`) | Média | F1.10 | Alta | ⏳ Pendente | — |
| F1.14 | F1-Concepção | Validação concept art PNGs vs specs canônicas | Média | F1.10 | Baixa | ⏳ Pendente | — |
| F1-DL.0 | F1-DeepLore | Setup pasta `docs/narrative/deep/` + `_INDEX.md` canônico + memo `project_deep_lore_canonico` | Alta | F1.5, F1.6 | Baixa | ✅ Concluído | — |
| F1-DL.1 | F1-DeepLore | R1 — Eras (era-1, era-2, era-3, transicoes — 4 docs). Era 1 alvo Fibonacci base 4: 286k pal | Alta | F1-DL.0 | Alta | 🔄 Em andamento | — |
| F1-DL.1.6 | F1-DeepLore | Era 1 §6 Acústica e luz reativa (14.5k pal, 8 NPCs Ordem Recursiva, 4 sub-locais) | Alta | F1-DL.0 | Alta | ✅ Concluído | — |
| F1-DL.1.7 | F1-DeepLore | Era 1 §7 Bancos sementes-relíquia + Ordem Recursiva embrionária (33.4k pal, 52 queries; 11 NPCs + 9 sub-locais; axiologia canon aplicada) | Alta | F1-DL.1.6 | Alta | ✅ Concluído | — |
| F1-DL.1.8 | F1-DeepLore | Era 1 §8 Queda multi-hipótese (~42k pal, 84 queries Fibonacci) | Alta | F1-DL.1.7 | Alta | ✅ Concluído | — |
| F1-DL.REFAC | F1-DeepLore | Refactor fluidez prose deep-lore + retrofit easter eggs maçônicos + densificação §10 — (a) reduz repetição marcadores ritualísticos preservando conteúdo canônico/NPCs/geografia/datações/axiologia/Fibonacci. (b) insere easter eggs maçônicos canon §§6-8 (memo `project_eastereggs_maconaria_canonica`). (c) **densifica §10 +17k pal pra atingir alvo Fibonacci 110k** (atual 93k, foco saturação ritualística remanescente Parte I + densificação narrativa Parte II/III). Aplicar §§6-10. Via narrative-writer (briefing fluência Stephenson/Krasznahorkai + densidade técnica + easter eggs ~10-15%). Bloqueia F5-BK.1 | Alta | F1-DL.1.10 | Alta | ⏳ Pendente | — |
| F1-DL.1.9 | F1-DeepLore | Era 1 §9 Vestígios cross-era (~68k pal, 136 queries Fibonacci) | Alta | F1-DL.1.8 | Alta | ✅ Concluído | — |
| F1-DL.1.10 | F1-DeepLore | Era 1 §10 Trecho in-character Verônica Atelaiá (~110k pal, 220 queries Fibonacci) — entregue 93k pal (~85% alvo). 7 dispatches incrementais voz Eco Adso fluida. Lacuna +17k em F1-DL.REFAC | Alta | F1-DL.1.9 | Alta | ✅ Concluído | — |
| F1-DL.2 | F1-DeepLore | R2 — Facções (Sterling Corp, FIR, Ordem Recursiva, Cult Mirage, Underground Silêncio, Pelicano Branco, menores — 7 docs, 22.030 pal entregues; 20 queries RAG densas; audit narrative-designer aprovado com 6 fixes críticos aplicados) | Alta | F1-DL.1 | Alta | ✅ 2026-05-19 | — |
| F1-DL.3 | F1-DeepLore | R3 — Settings deep (8 settings, 25.201 pal entregues; 20 queries RAG densas; audit narrative-designer aprovado com 4 fixes críticos aplicados) | Alta | F1-DL.2 | Alta | ✅ 2026-05-19 | — |
| F1-DL.4 | F1-DeepLore | R4 — Characters deep (Gus "Dragon" + 6 companions ~35k) | Alta | F1-DL.3 | Alta | ⏳ Pendente | — |
| F1-DL.5 | F1-DeepLore | R5 — Antagonistas + NPCs (Sterling Locke + Patch-Zero + NPCs antologia ~12k) | Alta | F1-DL.4 | Alta | ⏳ Pendente | — |
| F1-DL.6 | F1-DeepLore | R6 — Magic (Glyph/Token/Conjuro/Codex + 4 linguagens + natureza-matemática ~10k) | Alta | F1-DL.5 | Alta | ⏳ Pendente | — |
| F1-DL.7 | F1-DeepLore | R7 — Ontologia (tech 3 eras + cosmologia + leitmotivs ~8k) | Alta | F1-DL.6 | Média | ⏳ Pendente | — |
| F1-DL.8 | F1-DeepLore | R8 — Stinger (sequel hooks + post-credits ~4k) | Média | F1-DL.7 | Média | ⏳ Pendente | — |
| F1-DL.9 | F1-DeepLore | R9 — Antologia Vol 2 (14 contos in-character ~42k) | Alta | F1-DL.8 | Alta | ⏳ Pendente | — |
| F2-S.1 | F2-Setup | Instalar Godot 4 (LTS estável) — **PAUSADA até deep-lore F1-DL.9 completa** | Alta | F1-DL.9 | Baixa | ⏳ Pendente | — |
| F2-S.2 | F2-Setup | Criar `/game/project.godot` inicial (Godot 4, GDScript, forward+ renderer) | Alta | F2-S.1 | Baixa | ⏳ Pendente | — |
| F2-S.3 | F2-Setup | `git init` + primeiro commit baseline | Alta | — | Baixa | ⏳ Pendente | — |
| F2-S.4 | F2-Setup | Criar repo Forgejo `gusworld` privado via MCP `mcp__forgejo__create_repo` | Alta | F2-S.3 | Baixa | ⏳ Pendente | — |
| F2-S.5 | F2-Setup | Decisão: `gusworld-engine` repo separado vs submódulo | Alta | F2-S.4 | Média | 💡 Decisão tomada | — |
| F2-S.6 | F2-Setup | Setup `.gitignore` Godot completo (`.import/`, `*.translation`, etc) | Alta | F2-S.3 | Baixa | 🟡 Parcial | — |
| F2-S.7 | F2-Setup | Setup Forgejo Actions runner local | Alta | F2-S.4 | Média | ⏳ Pendente | — |
| F2-S.8 | F2-Setup | Setup `CONTRACT.md` projeto (RFC 2119 + SOLID + Conventional Commits) | Alta | F2-S.3 | Baixa | ⏳ Pendente | — |
| F2-S.9 | F2-Setup | Setup `TESTES.md` projeto (T1-T15 / A1-A13 do manual canônico) | Alta | F2-S.8 | Baixa | ⏳ Pendente | — |
| F2-S.10 | F2-Setup | Setup `CHANGELOG.md` + `README.md` (hub-and-spoke) | Alta | F2-S.3 | Baixa | ⏳ Pendente | — |
| F2-S.11 | F2-Setup | Setup i18n Godot estrutura (`tr()` + `game/translations/strings.csv` header `keys,pt_br,en_intl` + fallback `pt_BR → en_intl` + fonts Latin Extended Noto/Inter). Dev fica em pt-br; coluna en_intl vazia até pós-release v1.0.0 | Alta | F2-S.2, F2-S.9 | Média | ⏳ Pendente | — |
| F2-S.12 | F2-Setup | Audit + CI lint rule: zero hardcoded strings user-facing em GDScript (grep regex em CI fora de `tr()`) | Alta | F2-S.11, F2-CI.2 | Média | ⏳ Pendente | — |
| F2-S.13 | F2-Setup | `docs/i18n/translatable-assets.md` (inventário de assets visuais com texto: sinais, placas, vitrais, propaganda Sterling, banners — plano de swap por locale pós-release) | Média | F2-S.11 | Baixa | ⏳ Pendente | — |
| F2-E.1 | F2-Engine | `engine/orbital_camera/` — câmera 3/4 rotação livre + zoom + collision-aware | Alta | F2-S.2 | Alta | ⏳ Pendente | — |
| F2-E.2 | F2-Engine | `engine/event_bus/` — autoload de signals globais | Alta | F2-S.2 | Baixa | ⏳ Pendente | — |
| F2-E.3 | F2-Engine | `engine/save_system/` — JSON `save_version: 1` + migrators stub | Alta | F2-S.2 | Média | ⏳ Pendente | — |
| F2-E.4 | F2-Engine | `engine/scene_manager/` — fade in/out + scene loading async | Alta | F2-E.2 | Média | ⏳ Pendente | — |
| F2-E.5 | F2-Engine | `engine/turn_based_combat/` — initiative queue + action resolution + state machine | Alta | F2-E.2 | Alta | ⏳ Pendente | — |
| F2-E.6 | F2-Engine | `engine/dialogue_system/` — Ink ou DialogueManager (decidir) + branching base | Alta | F2-E.2 | Alta | 🎨 Pendente design | — |
| F2-E.7 | F2-Engine | `engine/input_remap/` — controles remappáveis (acessibilidade D1) | Alta | F2-S.2 | Média | ⏳ Pendente | — |
| F2-E.8 | F2-Engine | `engine/diary/` — UI Diário do Gus (caderno layer 1 + texto limpo layer 2) | Alta | F2-E.3, F2-E.4 | Alta | ⏳ Pendente | — |
| F2-E.9 | F2-Engine | `engine/knowledge_system/` — Knowledge Progression score híbrido + HUD + gates | Alta | F2-E.3 | Média | ⏳ Pendente | — |
| F2-G.1 | F2-Game | Blockout 1 área Distritos Inferiores (cidade ato 1, ~5min explore) | Alta | F2-E.1 | Alta | ⏳ Pendente | — |
| F2-G.2 | F2-Game | Modelo low-poly Gus (3k-4.5k tris, 512² atlas, silhouette test 8 ângulos) | Alta | F1.10 | Alta | ⏳ Pendente | — |
| F2-G.3 | F2-Game | Locomotion básico Gus (idle / walk / run / interagir) | Alta | F2-G.2 | Média | ⏳ Pendente | — |
| F2-G.4 | F2-Game | 1 inimigo comum (800-1.5k tris) | Alta | F1.10 | Média | ⏳ Pendente | — |
| F2-G.5 | F2-Game | Combate turn-based 1v1 + Compilação Deck Rúnico (3-5 cartas básicas) | Alta | F2-E.5 | Alta | ⏳ Pendente | — |
| F2-G.6 | F2-Game | Puzzle único Vetor do Gambito (predição trajetória, mini-board holográfico) | Alta | F2-E.5 | Alta | 🎨 Pendente design | — |
| F2-G.7 | F2-Game | Dialogue 1 NPC introdutório | Alta | F2-E.6 | Média | ⏳ Pendente | — |
| F2-G.8 | F2-Game | Save/load funcional (JSON `save_version: 1`) | Alta | F2-E.3 | Média | ⏳ Pendente | — |
| F2-G.9 | F2-Game | HUD básico (HP, action points, deck) | Alta | F2-G.5 | Média | ⏳ Pendente | — |
| F2-A.1 | F2-Arte | Atlas gradient cidade (256²/512², paleta ato 1) | Alta | F1.9 | Média | ⏳ Pendente | — |
| F2-A.2 | F2-Arte | Shader outline inverted-hull reutilizável | Alta | F2-S.2 | Alta | ⏳ Pendente | — |
| F2-A.3 | F2-Arte | Shader holograma (cartas + UI rúnica) | Alta | F2-S.2 | Alta | ⏳ Pendente | — |
| F2-A.4 | F2-Arte | Material toon Gus | Alta | F2-G.2, F2-A.2 | Média | ⏳ Pendente | — |
| F2-A.5 | F2-Arte | 5-10 props modulares cidade (tile-based) | Alta | F2-A.1 | Média | ⏳ Pendente | — |
| F2-AU.1 | F2-Áudio | 1 track ambient cidade noturna (pós-punk + drone) | Média | — | Média | ⏳ Pendente | — |
| F2-AU.2 | F2-Áudio | SFX base (footstep, card-play, hit, scan-óculos, UI confirm/cancel) | Alta | — | Baixa | ⏳ Pendente | — |
| F2-AU.3 | F2-Áudio | Audio bus setup Godot (master / music / sfx / ui / vo) | Alta | F2-S.2 | Baixa | ⏳ Pendente | — |
| F2-CI.1 | F2-Build/CI | Wrappers shell `scripts/build_linux.sh` + `scripts/build_windows.sh` | Alta | F2-S.2 | Baixa | ⏳ Pendente | — |
| F2-CI.2 | F2-Build/CI | Forgejo Actions YAML inicial (lint + import + export Linux/Win em PR) | Alta | F2-S.4, F2-S.7 | Média | ⏳ Pendente | — |
| F2-CI.3 | F2-Build/CI | Versioning convention (`v0.0.x` durante VS, `v0.1.0` no VS done) | Alta | F2-S.3 | Baixa | 💡 Decisão tomada | — |
| F2-M.1 | F2-Milestone | VS end-to-end coeso 5-10min gameplay | Alta | F2-G.1-F2-G.9 | Alta | ⏳ Pendente | — |
| F2-M.2 | F2-Milestone | 60fps @ 1080p em GTX 1060+ | Alta | F2-M.1 | Alta | ⏳ Pendente | — |
| F2-M.3 | F2-Milestone | 5 playtesters externos: time-to-fun ≤ 5min (métrica GDD) | Alta | F2-M.1 | Média | ⏳ Pendente | — |
| F2-M.4 | F2-Milestone | Build Linux + Windows distribuível (.tar.gz / .zip) | Alta | F2-CI.1, F2-CI.2 | Média | ⏳ Pendente | — |
| F2-M.5 | F2-Milestone | Revisão style guide com dados reais de perf | Alta | F2-M.2 | Baixa | ⏳ Pendente | — |
| F3.1 | F3-Alpha (~8m) | VS expandido + ato 2 Selve em blockout | Média | F2-M.1 | Alta | ⏳ Pendente | — |
| F4.1 | F4-Beta (~10m) | Content-complete + balance pass + localização base estrutural | Média | F3.1 | Alta | ⏳ Pendente | — |
| F5.1 | F5-Gold/RC (~11-12m) | Cert (Steam manifest) + age rating IARC + zero critical bug | Alta | F4.1 | Alta | ⏳ Pendente | — |
| F5.2 | F5-Gold/RC | Auditoria final: `AUD` grupo zerado, `docs/auditoria/deploys/` populado | Alta | F5.1 | Média | ⏳ Pendente | — |
| F5.3 | F5-Gold/RC | Release tag v1.0.0 + release notes | Alta | F5.2 | Baixa | ⏳ Pendente | — |
| F5.4 | F5-Gold/RC | Post-launch patch plan | Média | F5.3 | Média | ⏳ Pendente | — |
| F5-BK.1 | F5-Livro | Consolidação Volume 1 — Bíblia worldbuilding (revisão + prefácio + sumário + glossário + índice + capa) | Alta | F5.3, F1-DL.9, F1-DL.REFAC | Alta | ⏳ Pendente | — |
| F5-BK.2 | F5-Livro | Consolidação Volume 2 — Antologia narrativa (14 contos in-character revisados + arco coletivo + prefácio) | Alta | F5-BK.1 | Alta | ⏳ Pendente | — |
| F5-BK.3 | F5-Livro | Tradução en-intl dos 2 volumes (post-release v1.0.0; usar i18n CSV pra parts já marcadas) | Média | F5-BK.2, F2-S.11 | Alta | ⏳ Pendente | — |
| F5-BK.4 | F5-Livro | Publicação livro (digital + print-on-demand: KDP, Lulu ou similar) | Média | F5-BK.3 | Média | ⏳ Pendente | — |
| CUT.1 | Cortes-G1 | Multiplayer / co-op | — | — | — | 💡 Decisão tomada | — |
| CUT.2 | Cortes-G1 | Open-world / mundo persistente | — | — | — | 💡 Decisão tomada | — |
| CUT.3 | Cortes-G1 | Crafting system / economia complexa | — | — | — | 💡 Decisão tomada | — |
| CUT.4 | Cortes-G1 | VO (voice acting) | — | — | — | 💡 Decisão tomada | — |
| CUT.5 | Cortes-G1 | Mocap | — | — | — | 💡 Decisão tomada | — |
| CUT.6 | Cortes-G1 | DDA (dynamic difficulty) | — | — | — | 💡 Decisão tomada | — |
| CUT.7 | Cortes-G1 | Mod support / in-game editor | — | — | — | 💡 Decisão tomada | — |
| CUT.8 | Cortes-G1 | Achievements / leaderboards (post-launch talvez) | — | — | — | 💡 Decisão tomada | — |
| CUT.9 | Cortes-G1 | Localização **conteúdo** além de pt-br em v1 (estrutura i18n-ready desde F2-S.11; tradução en-intl real só pós-release v1.0.0). Ver memo `project_i18n_canonico.md` | — | — | — | 💡 Decisão tomada | — |
| CUT.10 | Cortes-G1 | Console cert (Steam Deck verified vem depois) | — | — | — | 💡 Decisão tomada | — |

## Legenda de status

- ✅ Concluído | 🔄 Em andamento | 🟡 Parcial | ⏳ Pendente
- 💡 Decisão tomada (abordagem definida, implementação futura ou descartada)
- 🎨 Pendente design (aguarda spec/brainstorm)
- 🔍 Pendente verificação (implementado, aguarda validação)

## Estado Auditado

- `—` não auditado | `✓` auditado e aprovado | `⚠` auditado com ressalvas

## Notas

- **F1.11** (`docs/tech/*`) e **F1.12** (GDD consolidado): marcados 🟡 ⚠ — gerados em modo autônomo primeira passada antes da reforma do squad. Aguardam revisão ponto-a-ponto do criador supremo antes de virar canônicos.
- **F1.13 + F1.14**: art sheets de produção e validação concept art são pré-Fase 2; ficam aqui pra rastreamento.
- Grupos `F2-*` agrupam pendências da fase 2 (impl). Pré-requisitos respeitam dependência operacional (Godot install → project.godot → módulos engine → game features).
- Grupos `F3-F5` placeholders altos. Vão ser quebrados em itens granulares quando F2 fechar.
- **Cortes G1 (`CUT.*`)** = decisões one-way doors do GDD §9. Não fazer em v1.
- **Disciplina antes de release** (ver memória `project_qa_deploy_disciplina`): AUD zerado + assinatura nominal em deploy irreversível (sub-fase 5.3: 48h + 30d offline).
- **F1-DL.REFAC (débito literário)**: prose deep-lore §§6-8 atual prioriza densidade técnica obsessiva sobre fluência literária. Marcadores ritualísticos hiper-repetidos quebram cadência. Refactor obrigatório antes de F5-BK.1 (consolidação livro). Não bloqueia ciclos 9-10 (preferível terminar Era 1 inteira e refactorar tudo de uma vez).
- **Fibonacci descontinuado pós-Era 1 EM MÉTRICAS DE PROCESSO** (decisão 2026-05-19): aplica-se a (a) contagem de palavras de seções/capítulos — alvos referenciais arredondados (~20k/~26k etc), e (b) quantidade de RAG queries — 20 fixas por ciclo, cada uma mais densa/longa (15-30+ palavras). Trade-off canônico queries: menos queries × mais densidade = melhor sinal/ruído corpus ~163k chunks. Era 1 §§7-10 seguiram Fibonacci antigo (33k/42k/68k/93k pal). Memo: `project_alvo_palavras_pos_era_1.md`.
- **Easter eggs Fibonacci + maçonaria MANTIDOS no TEXTO** (decisão 2026-05-19): Fibonacci pervasivo permanece em datações/contagens narrativas/idades personagens/dimensões arquitetônicas/compassos sonoros (~10-20% densidade). Maçonaria pervasiva permanece em pavimento tesselado/ashlar/esquadro+compasso/3-5-7 degraus/cordão 89 nós/Placas Recursivas/Helíaco Vyr=Hiram Abiff (~10-15% densidade). Apenas Fibonacci EM MÉTRICAS DE PROCESSO (palavras/queries) foi descontinuado. Memos: `project_fibonacci_easter_egg.md`, `project_eastereggs_maconaria_canonica.md`.

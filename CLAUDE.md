# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Estado atual (vigente — atualizado 2026-07-15)

**Stack vigente: C++20 + SDL3 + glintfx/RmlUi.** Solo indie, Linux (v1.0.0) + Windows (pós-v1, CI real MSVC já validado), single-player puro. Gênero: RPG + Puzzle + Aventura + combate turn-based, **visual 2D estilizado** (sprites/pixel art, não 3D). Ver "Decisões fechadas" abaixo para a stack completa e os ADRs.

**Board M0-M9 (migração da engine, ver ROADMAP.md/TODO.md para o board vivo):** M0-M6 entregues e M7-DIALOGO entregue; só falta o playthrough ao vivo do líder para fechar o M7 (paridade jogável). M8 (decommission Godot/C#) e M9 (higienização) vêm depois. Em paralelo ao board, a onda `CARDS` desenvolve o motor de cartas techMagic ([ADR-016](docs/tech/adr/ADR-016-techmagic-effect-engine-data-driven.md)).

**Mirror + Wiki publicados (2026-07-14):** repo espelhado em `petrinhu/GusWorld` no GitHub (push dual-remote) além do Codeberg canônico; Wiki inicial publicada nos dois remotos (Codeberg bilíngue EN/PT para contribuidor técnico; GitHub PT-br para leigo/iniciante); `AI-DISCLOSURE.md` adicionado.

**Deep-lore canon entregue (~365k pal):** Era 1 §§1-10 (~318k), R2 Facções (22k, 7 docs), R3 Settings (25k, 8 docs). Deep-lore restante segue PARALELO ORGÂNICO (não bloqueia código), entrando entre steps técnicos.

<details>
<summary>Histórico — Fase 1 → Fase 2 e a era Godot (superado, mantido por registro; ver "Decisões fechadas" para o vigente)</summary>

**Início do projeto: 2026-05-15** (primeiro commit `97ed2fe`, Fase 1 concepção + deep-lore).

**PIVOT Fase 1 → Fase 2 (ADR-001, 2026-05-19).** Na época: Godot 4.6.stable.mono instalado (`~/.local/bin/godot`), C# .NET 8 AOT (ADR-002), câmera 3/4 rotacional + zoom orbital (Chrono Trigger reference, mas 3D real). Essa decisão de engine/câmera/visual foi **SUPERADA por dois pivôs de stack em sequência**: (1) Godot+C# → C++ com Qt6 (decisão RF-1/RF-7, brainstorm bigtech 2026-06-21); (2) Qt6 → C++20+SDL3 ([ADR-008](docs/tech/adr/ADR-008-repivot-qt-to-sdl3.md), 2026-06-22) — a tentativa Qt6 nunca chegou a ter jogo jogável antes do segundo pivô. O jogo nunca chegou a ser 3D em produção. O "porquê" de cada pivô está documentado nos ADRs (`docs/tech/adr/`), matéria do "cemitério de ideias mortas" do projeto.

</details>

### Modo de trabalho com agentes (CRÍTICO — REGRA CANÔNICA INQUEBRÁVEL)

**User é criador supremo do projeto.** TODA decisão de design, arquitetura, engenharia, narrativa ou arte DEVE ser apresentada ao user via `AskUserQuestion` antes de ser implementada. **Nenhum agente decide sozinho — jamais.**

Escopo da regra (SEM EXCEÇÃO):
- **Design:** mecânicas, balanceamento, sistemas de jogo, fórmulas, curvas, economia
- **Arquitetura:** padrões, estrutura de módulos, escolhas de framework, contratos de API
- **Engenharia:** algoritmos, fórmulas de cálculo, estrutura de dados, abordagens de implementação
- **Narrativa / Arte:** personagens, lore, visuais, estilo — qualquer coisa canônica

**Como aplicar:**
1. Agente de implementação (ex: `backend-engineer`) NÃO escolhe abordagem sozinho — o main thread coleta a decisão via `AskUserQuestion` ANTES de briefar o agente.
2. Se surgir decisão nova DURANTE a implementação, o agente para, reporta as opções, e o main thread pergunta ao user via `AskUserQuestion`.
3. Modo autônomo SÓ se o prompt contém literal `MODO AUTÔNOMO` / `decide sozinho` / `sem perguntar`. Mesmo em autônomo, one-way doors (pillars, antagonista, ending, render style, proporção char, engine, save format) sempre exigem confirmação.

Bloco "Modo de operação" inserido em cada agent em `~/.claude/agents/`.

**Dever de contra-argumentar (canonizado 2026-05-30):** Agentes NÃO devem aceitar decisões cegamente. Se uma decisão do criador for destrutiva, violar pillar, quebrar dependência de ondas, ou inviabilizar milestone — o agente DEVE contra-argumentar antes de executar. Tom: direto, sem rodeios. Formato: (1) nomeie a decisão, (2) explique o risco específico, (3) proponha alternativa, (4) deixe a decisão final com o criador via AskUserQuestion. O criador tem autoridade final — o agente executa após a resposta — mas o silêncio passivo NÃO é aceitável.

### Decisões fechadas (vigentes)

- **Engine/linguagem:** **C++20 + SDL3**, engine própria escrita do zero, single repo (não `/engine/` + `/game/` separados; ver "Estrutura de repositório" abaixo). Substitui Godot/C# (ADR-001/ADR-002) e a tentativa intermediária com Qt6 (RF-1/RF-7). Ver `docs/tech/adr/ADR-008-repivot-qt-to-sdl3.md` e `docs/tech/pivot/engine-design.md`.
- **Visual:** **2D estilizado** (sprites/pixel art via PixelLab, não 3D). Cockpit de batalha "Tático" com retratos, moldura de carta e HUD com barras de HP/AP/Mana. Referências de sensação: Chrono Trigger / Sea of Stars / Stardew Valley na exploração; Pokémon na apresentação de batalha.
- **Arquitetura:** 4 camadas dentro de `GusEngine/` — `core/` (POCO genérico: tempo, RNG, ECS leve, recursos, eventos) → `domain/` (POCO das regras do jogo: save, i18n, progressão, templates, combate, diálogo) → `platform/` (única fronteira que toca bibliotecas externas: SDL3/GL/RmlUi/miniaudio — janela, render2d, input, áudio, arquivos) → `app/` (telas do jogo, gameplay, ferramentas internas). Regra: cada camada só depende das de baixo, nunca o contrário; gate de CI proíbe violação (inclusive `<glintfx` vazando pra `core`/`domain`).
- **UI/HUD:** servida pelo **glintfx** via embed mode (`glintfx::UiLayer`), que embrulha RmlUi 6.3 + backend GL3 e compõe sobre o contexto GL da casca SDL; o backend RmlUi vendorizado à mão foi aposentado. Consumido via FetchContent (pin atual em `GusEngine/CMakeLists.txt`, `GLINTFX_BACKEND_GLFW=OFF`). Ver `docs/tech/adr/ADR-010-adopt-glintfx-embed-mode.md`.
- **Motor de cartas (magia = software, na prática):** executor de conjuros `techMagic`, data-driven (não VM), com `EffectKind` por carta especial (ChainDamage/Tesla, DelayAction/Einstein, EM-Shield/Faraday, AoE-Stun/Newton, Null-Proof/Gödel, etc.), um handler por vez, cada um coberto por TDD + mutation testing adversarial. Ver `docs/tech/adr/ADR-016-techmagic-effect-engine-data-driven.md`, `docs/design/mecanicas/combat.md` e `cartas-technomagik.md` — atualizados a cada `EffectKind` novo da onda `CARD-ENGINE-MANIFESTO` (o agente implementador reflete a carta nesses docs no mesmo PR).
- **Save format:** JSON versionado com **AEAD (XChaCha20/Monocypher) + HMAC-SHA256** cifrando e autenticando o save inteiro (ADR-006, ADR-007 save V4, ADR-015 save security v2), migrators desde D1. Anti-tamper e anti-rollback são mais fortes no modo Hardcore (machine-bind, âncora selada).
- **Localização (i18n):** dev em pt-br; chaves i18n custom (`pt_br.md`/`en_intl.md`) consumidas por um translator C++ próprio (não Godot `tr()`, não ICU). Tradução en-intl planejada pós-v1.0.0.
- **CI:** Forgejo Actions (canônico, política local-first: jobs pesados só em PR/release) + espelho no GitHub Actions com CI Windows real (MSVC), validado verde desde 2026-07-14.
- **Plataformas:** Linux é a plataforma alvo da v1.0.0 (CMakePresets `linux-release`); Windows (`windows-release`) planejado para pós-v1.0.0, já com CI real validado.
- **Repositório:** Codeberg (`petrinhu/gusworld`) é o remoto canônico; espelho público em GitHub (`petrinhu/GusWorld`, push dual-remote). Wiki inicial publicada nos dois (2026-07-14).

### Documentos canônicos (Fase 1)

```
docs/
├── design/
│   ├── pillars.md          # Revisão 1, concluída em sessão colaborativa (2026-05-15). Canônico.
│   └── gdd.md              # v0.2 (pré-produção, F1.12-REFRESH 2026-06-02, higienização de stack 2026-06-23)
├── narrative/
│   ├── lore-bible.md            # REVISÃO 1 — canônico (expansão Bloco G: eventos cross-eras, NPCs, tradições)
│   ├── arco-principal.md        # REVISÃO 1 — canônico (3 atos, 8 beats Kishōtenketsu, Dante traidor, 3 endings)
│   ├── factions.md              # REVISÃO 1 — canônico (expansão Bloco G: 7 facções com NPCs, tradições, histórico)
│   ├── comic-reliefs.md         # canônico (14 cenas LucasArts + 18 easter eggs)
│   ├── timeline.md              # REVISÃO 1 — canônico (expansão Bloco G: cronologia 3 eras, 50+ eventos)
│   ├── in-world-docs.md         # REVISÃO 1 — canônico (expansão Bloco G: 15 docs descobríveis, 3 gate Ouro)
│   ├── tradicoes-cultura.md     # REVISÃO 1 — canônico (expansão Bloco G: 9 tradições, calendário, costumes)
│   ├── deep/                    # deep-lore paralelo (eras, facções, settings, characters, antagonists, antologia; ver deep/_INDEX.md)
│   └── characters/              # REVISÃO 1 — canônico (expansão Bloco G: + "Memórias formativas"); 13 docs no disco
│       ├── party.md             # ÍNDICE + resumo + dinâmicas + matriz linguagens
│       ├── gus.md               # protagonista (narrativo — NÃO confundir com art/characters/gus.md de produção)
│       ├── caua-volt.md         # Striker (Pythia, Dutos Infernais)
│       ├── iara-lumen.md        # Infiltradora (Óxido, Setor Mirage, desertora Cult)
│       ├── bento-requiem.md     # Tanque (Asmódico, Catedrais Neo-Sylvania, exceção P2)
│       ├── linda-siren.md       # Crowd Control (Óxido, Zona do Silêncio)
│       ├── dante-grid.md        # TRAIDOR (Asmódico→C-Arcane, Periferia, FIR vassalo) — memórias double-layer
│       ├── jaci-proxy.md        # Healer biológica (Pythia, Selve Sombria)
│       ├── sterling-locke.md    # antagonista adulto (DRE, GRE, predador corporativo)
│       ├── patch-zero.md        # antagonista-sistema (anti-padrão + 4-canais)
│       ├── prelore_vilao.md     # backstory integral Sterling Locke pré-jogo
│       ├── brunus-vetorial.md   # NPC mentor-boticário (Era 3, ~700 anos, ligação Dragon Victory)
│       └── brunus-vetorial-conto.md  # conto do Brunus Vetorial
├── art/
│   └── style-guide.md      # REVISADO: SD 1:1:1, cel-shading, exceções normal/specular
└── tech/
    ├── architecture.md     # SUPERADO 2026-06-23 (era Godot/C#; fonte atual: docs/tech/pivot/engine-design.md)
    ├── engine-modules.md   # SUPERADO 2026-06-23 (era Godot/C#; fonte atual: docs/tech/pivot/engine-design.md)
    └── build.md            # SUPERADO 2026-06-23 (era Godot/C#; build atual: CMake + Ninja + CMakePresets, ver README)
```

### Specs canônicas externas (Resources do vault)

**8 character specs canônicas em `Resources/gusworld/`** (índice: `Resources/gusworld/_INDEX.md`). Padrão unificado: cel-shaded 3D, proporção SD 1:1:1, fundo `#FFFFFF` unlit, prompt SD/MJ incluso.

- `character-spec-gus.md` — Gus Vector Tavus Vance (protagonista, 11)
- `character-spec-caua-volt.md` — Cauã Berenger (Striker, 13)
- `character-spec-iara-lumen.md` — Iara Koslov (Infiltradora, 12)
- `character-spec-bento-requiem.md` — Bento Chevalier (Tanque, 14, exceção Pillar 2)
- `character-spec-linda-siren.md` — Linda Neumann (Crowd Control, 12)
- `character-spec-dante-grid.md` — Dante Alencar (**TRAIDOR**, 13)
- `character-spec-jaci-proxy.md` — Jaci Vanderbist (Healer, 11)
- `character-spec-sterling-locke.md` — Sterling Locke (antagonista adulto, monolítico)

Imutáveis sem aprovação user.

Base canônica imutável: `sinopse.md` (worldbuilding + protagonista) + `Resources/gusworld/character-spec-gus.md`. NUNCA contradizer.

**Status de revisão:** todos os docs listados acima já saíram do estado "AGUARDA REVISÃO USER" da primeira passada autônoma (antes da reforma do squad); pillars.md e gdd.md são canônicos (Revisão 1 / v0.2), architecture.md/engine-modules.md/build.md foram formalmente marcados SUPERADO em 2026-06-23 pelo pivot pra C++20 + SDL3.

### Estrutura de repositório

Stack vigente: **C++20 + SDL3** (pivot ADR-008, 2026-06-22). `engine/` e `game/` (Godot/C#) são **legado dormente até M8**; não usar como referência de código atual.

```
gusworld/
├── CLAUDE.md
├── TODO.md              # backlog canônico (skill tab_pendencias)
├── CHARS.md             # inventário canônico de TODOS personagens nomeados (atualizar sempre que criar novo)
├── PLACES.md            # inventário canônico de TODOS lugares nomeados (settings/cidades/catedrais/sub-locais; atualizar sempre que criar novo)
├── CONTRACT.md          # contrato de qualidade/processo do projeto
├── TESTES.md            # plano de testes não-unitários
├── AUDITORIAS.md        # rastreio de auditorias aplicáveis ao stack
├── ROADMAP.md           # marcos M0-M9+
├── README.md            # build/run atual (CMake + Ninja + CMakePresets)
├── sinopse.md           # base canônica imutável
├── docs/                # ver acima (design, narrative, art, tech, auditoria, book)
├── GusEngine/            # engine + jogo em C++20 + SDL3 (stack vigente)
│   ├── core/            # POCO puro: time, rng, ecs_lite, resource, events
│   ├── domain/           # POCO puro: save, i18n, progression, templates, combat
│   ├── platform/         # única fronteira SDL3/GL/RmlUi (window, render2d, input, audio, fs)
│   ├── app/              # gameplay + telas + UI (screens/, tools/)
│   ├── third_party/      # libs vendorizadas (FetchContent sob demanda)
│   └── build/            # builds locais (não versionado como release)
├── resources/            # ativos versionados fora do código
│   ├── sprites/          # sprites PixelLab por personagem
│   ├── livros/           # corpus RAG (bibliografia)
│   ├── prompts_images/   # prompts de geração de imagem (nano banana / PixelLab)
│   ├── maps/             # mapas .gmap
│   ├── pers_3d/          # arte conceitual 3D (glb) para os livros futuros
│   ├── glb/, images/, vfx/
├── engine/              # LEGADO Godot C# (dormente até M8; não editar como se fosse vigente)
├── game/                # LEGADO projeto Godot (dormente até M8)
├── assets/              # sources arte/som legados (Blender, Krita, Aseprite, audio raw)
└── build/               # outputs export legados
```

## Pilares criativos (regras de consistência narrativa — imutáveis)

Toda decisão valida contra esses 5 pillars. Detalhes em `docs/design/pillars.md`.

- **Magia = software.** Feitiços são scripts rúnicos compilados; linguagens citadas (ex: *C-Arcane*) seguem analogia de programação de baixo nível. Não tratar magia como força arbitrária.
- **Natureza é matemática rígida**, não caos. Fauna/flora da Selve Sombria obedecem sequências, fractais e funções recursivas. Anomalias = bugs/vírus.
- **Loop de habilidades acoplado ao hardware do personagem.** Óculos táticos (varredura/HUD) ↔ Matriz Ortodôntica (antena UHF/VHF amplificadora) ↔ Tavus-Drive de pulso (executor de cartões). Toda nova mecânica deve se encaixar nesse triângulo ou justificar exceção.
- **Idade 11 anos é canônica.** Tom: prodígio analítico, não power-fantasy adulta. Estética dos antagonistas/cenário pode ser sombria, mas o protagonista resolve por lógica (xadrez, TCG, otimização), não por força.
- **Setting bipartido:** megacidade ciber-gótica × Selve Sombria. Manter contraste deliberado. Misturar paletas só no ato 3 (recompensa narrativa).

## Easter eggs pervasivos canon (densidade ~10-15%)

Dois sistemas de easter eggs sutis aplicam a TODOS docs deep-lore + assets do jogo. **SEM siglas, SEM gestos rituais nomeados.** Leitor familiar reconhece; leigo não nota.

- **Fibonacci #1** (memória `project_fibonacci_easter_egg`): visual/arquitetural/textual/mecânico/sonoro/narrativo. Números 1,1,2,3,5,8,13,21,34,55,89,144 em datações/contagens/HP/dano/loot/proporções/compassos
- **Maçonaria canon** (memória `project_eastereggs_maconaria_canonica`): vetor central Pigpen cipher ↔ cripto-glifo Era 1 (grade 3×3 + X + pontos). Símbolos visuais (pavimento tesselado, ashlar bruto/polido, esquadro+compasso, Colunas Boróstoma+Janor, acaceiro-tronco-vermelho, avental couro). Numéricos (3-5-7 degraus, 47ª proposição, cordão 89 nós). Lendário (Helíaco Vyr = Hiram Abiff echo)

## Comandos (vigentes — CMake + Ninja, ver README.md para a versão completa)

Pré-requisitos: compilador C++20 (GCC/Clang/MSVC-MinGW), CMake, Ninja, Git. SDL3, RmlUi, glintfx e Catch2 são baixados e fixados automaticamente via `FetchContent` (sem instalação manual).

```bash
cd GusEngine

# Configurar (primeira vez; gera build/linux-release/)
cmake --preset linux-release

# Compilar
cmake --build --preset linux-release

# Rodar o jogo
./build/linux-release/app/gusworld_app

# Rodar a suíte de testes (Catch2)
ctest --preset linux-release
```

Linux é a plataforma alvo do lançamento v1.0.0; existe preset Windows (`windows-release`), com CI real (MSVC) validado, planejado pra pós-v1.0.0. `game/` (Godot) e `engine/` (C# legado) são código morto — não usar como referência nem como alvo de build.

## Skills de projeto

Projeto de **jogo** — usar `proj_jogo` (não `proj_software`). Agentes relevantes:

- **Fase 1 (concluída):** `lead-game-designer`, `narrative-designer`, `art-director`, `software-architect`
- **Fase 2 (ativa — vertical slice em C++20+SDL3):** `gameplay_engineer` (mecânicas de gameplay puro: combate, exploração, progressão, inventário, IA, loot, status — consome POCO do `backend-engineer`, NÃO cria domínio/persistência), `backend-engineer` (POCO de domínio, persistência, serialização, save/crypto, motor de cartas techMagic), `engine-graphics-programmer` (render2d SDL3, tilemap, câmera top-down 2D, shaders), `level-designer` (blockout), `game-animator` (locomotion + combat anims 2D), `security-engineer` (save crypto v2, AEAD), `audio-designer-composer` (música adaptive + SFX via miniaudio)
- **Fase 4 (QA/compliance):** `qa-engineer` (playtest plan; verificação adversarial/mutation testing de cada EffectKind novo do motor de cartas), `compliance-legal` (age rating IARC/ESRB/PEGI), `accessibility-specialist` (controles remappáveis, color)
- **Fase 5 (release):** `devops-sre` (Forgejo Actions + GitHub Actions Windows CI), `i18n-l10n-specialist` (se localizar v1.x), `technical-writer` (docs, wiki, ROADMAP/CHANGELOG)

Skills auxiliares: `tab_pendencias` (TODO.md), `memo_persistente` (checkpoint), `forgejo` (CI), `caveman` (compressão de comm).

## Vault Obsidian / PARA

Diretório vive dentro de `~/IDrive/Documentos/projetos_claudebrain/Projects/gusworld/` — pasta `Projects/` do vault PARA. Regras herdadas:

- Wikilinks `[[X]]` resolvem contra os 5 manuais canônicos do vault.
- Não modificar arquivos canônicos externos a partir daqui.
- Notas de design soltas podem usar estilo livre; entregáveis estruturados (GDD, lore bible, style guide) já existem em `docs/`.

## Próximos passos (estado 2026-07-15, ver ROADMAP.md/TODO.md para o board vivo)

1. **Fechar M7 (paridade jogável):** todos os pré-requisitos técnicos já entregues; falta só um playthrough de ~5min ao vivo do líder.
2. **M8 (decommission):** apagar Godot/C#/addons legados assim que M7 fechar — gate de build Windows já pré-cumprido.
3. **M9 (higienização):** limpar a árvore pós-porte.
4. **Onda `CARDS` (paralela, não bloqueia M7/M8/M9):** próximo `EffectKind` do motor de cartas techMagic = DamageQuantize/Planck; estética visual "terminal" para logs de combate em implementação.
5. Manter `TODO.md` atualizado via `/tab_pendencias`.

## Quando o projeto evoluir

Atualizar este CLAUDE.md adicionando:

- Decisões one-way door novas em ADRs leves (em `docs/tech/adr/` se necessário) e refletir aqui em "Decisões fechadas".
- Sair de "vertical slice em andamento" pra "vertical slice fechado" quando M7-M9 completarem.
- Se o repo `GusEngine/` mudar de estrutura de camadas, atualizar a árvore em "Estrutura de repositório".

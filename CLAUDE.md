# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Estado atual

**PIVOT Fase 1 → Fase 2 (ADR-001, 2026-05-19).** Solo indie G1, Godot 4.6.1 stable instalado (`/usr/bin/godot`), PC Linux+Windows, single-player puro. Gênero: RPG + Puzzle + Aventura + combate turn-based. Câmera 3/4 rotacional + zoom orbital (Chrono Trigger reference, mas 3D real).

**Deep-lore canon entregue (~365k pal):** Era 1 §§1-10 (~318k), R2 Facções (22k, 7 docs), R3 Settings (25k, 8 docs).

**Deep-lore restante PARALELO ORGÂNICO** (não bloqueia código): F1-DL.4-9 + F1-DL.REFAC (~111k pal restantes) entram entre steps técnicos conforme necessidade ou descanso criativo. Ver `docs/tech/adr/ADR-001-pivot-lore-to-engine.md`.

**Fase 2 ativa:** vertical slice (4-6 meses meta) — engine modular reutilizável + 1 área cidade + 1 encontro turn-based + 1 puzzle Vetor Gambito.

### Modo de trabalho com agentes (CRÍTICO)

**User é criador supremo do projeto.** Todos os agentes do squad criativo (`lead-game-designer`, `narrative-designer`, `art-director`, `software-architect`) operam em **modo colaborativo por default** — apresentam opções e aguardam decisão do user antes de gravar entregáveis canônicos.

Modo autônomo SÓ se prompt contém literal `MODO AUTÔNOMO` / `decide sozinho` / `sem perguntar`. Mesmo em autônomo, one-way doors (pillars, antagonista, ending, render style, proporção char, engine, save format) sempre exigem confirmação.

Bloco "Modo de operação" inserido em cada agent em `~/.claude/agents/`.

### Decisões fechadas

- **Engine:** Godot 4 + GDScript. C# rejeitado pra G1 (iteração lenta solo). C++ GDExtension só sob pressão de perf.
- **Visual:** 3D estilizado low-poly (Sea of Stars / Sable / Death's Door refs). Sem PBR. Gradient atlas + vertex color. ~5 shaders custom.
- **Arquitetura:** engine modular reutilizável (`/engine/`) separada do game-specific (`/game/`). Engine vira repo próprio + Godot addons.
- **Save format:** JSON versionado `save_version: 1` com migrators desde D1.
- **Localização:** Godot `tr()` + CSV. ICU recusado pra G1.
- **CI:** Forgejo Actions (esqueleto em `docs/tech/build.md`).
- **Plataformas:** Linux (AppImage/.tar.gz) + Windows (sem signing em G1).

### Documentos canônicos (Fase 1)

```
docs/
├── design/
│   ├── pillars.md          # 5 pillars testáveis (SIM/NÃO) — AGUARDA REVISÃO USER
│   └── gdd.md              # 1-page GDD — AGUARDA REVISÃO USER
├── narrative/
│   ├── lore-bible.md            # REVISÃO 1 — canônico (expansão Bloco G: eventos cross-eras, NPCs, tradições)
│   ├── arco-principal.md        # REVISÃO 1 — canônico (3 atos, 8 beats Kishōtenketsu, Dante traidor, 3 endings)
│   ├── factions.md              # REVISÃO 1 — canônico (expansão Bloco G: 7 facções com NPCs, tradições, histórico)
│   ├── comic-reliefs.md         # canônico (14 cenas LucasArts + 18 easter eggs)
│   ├── timeline.md              # REVISÃO 1 — canônico (expansão Bloco G: cronologia 3 eras, 50+ eventos)
│   ├── in-world-docs.md         # REVISÃO 1 — canônico (expansão Bloco G: 15 docs descobríveis, 3 gate Ouro)
│   ├── tradicoes-cultura.md     # REVISÃO 1 — canônico (expansão Bloco G: 9 tradições, calendário, costumes)
│   └── characters/              # REVISÃO 1 — canônico (expansão Bloco G: + "Memórias formativas")
│       ├── party.md             # ÍNDICE + resumo + dinâmicas + matriz linguagens
│       ├── gus.md               # protagonista (narrativo — NÃO confundir com art/characters/gus.md de produção)
│       ├── caua-volt.md         # Striker (Pythia, Dutos Infernais)
│       ├── iara-lumen.md        # Infiltradora (Óxido, Setor Mirage, desertora Cult)
│       ├── bento-requiem.md     # Tanque (Asmódico, Catedrais Neo-Sylvania, exceção P2)
│       ├── linda-siren.md       # Crowd Control (Óxido, Zona do Silêncio)
│       ├── dante-grid.md        # TRAIDOR (Asmódico→C-Arcane, Periferia, FIR vassalo) — memórias double-layer
│       ├── jaci-proxy.md        # Healer biológica (Pythia, Selve Sombria)
│       ├── sterling-locke.md    # antagonista adulto (DRE, GRE, predador corporativo)
│       └── patch-zero.md        # antagonista-sistema (anti-padrão + 4-canais)
├── art/
│   └── style-guide.md      # REVISADO: SD 1:1:1, cel-shading, exceções normal/specular
└── tech/
    ├── architecture.md     # AGUARDA REVISÃO USER
    ├── engine-modules.md   # AGUARDA REVISÃO USER
    └── build.md            # AGUARDA REVISÃO USER
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

**Status de revisão:** docs marcados "AGUARDA REVISÃO USER" foram gerados em modo autônomo na primeira passada (antes da reforma do squad). User vai revisar ponto-a-ponto antes de validar como canônicos.

### Estrutura de repositório

```
gusworld/
├── CLAUDE.md
├── TODO.md              # backlog canônico (skill tab_pendencias)
├── CHARS.md             # inventário canônico de TODOS personagens nomeados (atualizar sempre que criar novo)
├── PLACES.md            # inventário canônico de TODOS lugares nomeados (settings/cidades/catedrais/sub-locais; atualizar sempre que criar novo)
├── sinopse.md           # base canônica imutável
├── docs/                # ver acima
├── engine/              # módulos Godot reutilizáveis (orbital_camera, turn-based, dialogue, save)
├── game/                # projeto Godot do jogo (project.godot, scenes, scripts game-specific)
├── assets/              # sources arte/som (Blender, Krita, Aseprite, audio raw)
│   ├── models/
│   ├── textures/
│   ├── sprites/
│   ├── sfx/
│   └── music/
└── build/               # outputs export (linux/, windows/, v<X.Y.Z>/)
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

## Comandos (preliminares — Godot ainda não instalado)

Build/test/run **ainda não existem**. Pipeline definido em `docs/tech/build.md`:

```bash
# Quando Godot 4 estiver instalado e /game/ tiver project.godot:
godot --headless --import                          # primeira vez
godot --path ./game                                # editor
godot --headless --path ./game --export-release "Linux/X11" ./build/linux/gusworld.x86_64
godot --headless --path ./game --export-release "Windows Desktop" ./build/windows/gusworld.exe
```

Nada de inventar comandos antes de existirem. Atualizar esta seção quando primeiro protótipo rodar.

## Skills de projeto

Projeto de **jogo** — usar `proj_jogo` (não `proj_software`). Agentes relevantes:

- **Fase 1 (concluída):** `lead-game-designer`, `narrative-designer`, `art-director`, `software-architect`
- **Fase 2 (próxima):** `engine-graphics-programmer` (orbital camera, shaders), `level-designer` (blockout), `game-animator` (locomotion + combat anims), `3d-artist-rigger` (chars + props low-poly), `audio-designer-composer` (música adaptive + SFX)
- **Fase 4 (QA/compliance):** `qa-engineer` (playtest plan), `compliance-legal` (age rating IARC/ESRB/PEGI), `accessibility-specialist` (controles remappáveis, color)
- **Fase 5 (release):** `devops-sre` (Forgejo Actions build), `i18n-l10n-specialist` (se localizar v1.x)

Skills auxiliares: `tab_pendencias` (TODO.md), `memo_persistente` (checkpoint), `forgejo` (CI), `caveman` (compressão de comm).

## Vault Obsidian / PARA

Diretório vive dentro de `~/IDrive/Documentos/projetos_claudebrain/Projects/gusworld/` — pasta `Projects/` do vault PARA. Regras herdadas:

- Wikilinks `[[X]]` resolvem contra os 5 manuais canônicos do vault.
- Não modificar arquivos canônicos externos a partir daqui.
- Notas de design soltas podem usar estilo livre; entregáveis estruturados (GDD, lore bible, style guide) já existem em `docs/`.

## Próximos passos (Fase 2)

1. **Instalar Godot 4** + criar `/game/project.godot` inicial.
2. **Criar repo Forgejo** `gusworld` (privado inicial) + `gusworld-engine` (separado, candidato a public).
3. **Primeiro protótipo:** módulo `orbital_camera` em `/engine/` — câmera 3/4 com rotação + zoom + collision-aware.
4. **Vertical slice scope:** 1 área pequena cidade + 1 encontro turn-based + 1 puzzle Vetor do Gambito. Meta: 4-6 meses.
5. Manter `TODO.md` atualizado via `/tab_pendencias`.

## Quando o projeto evoluir

Atualizar este CLAUDE.md adicionando:

- Comandos reais de build/run/test assim que primeiro protótipo existir.
- Estrutura de pasta `/engine/` e `/game/` quando popular.
- Decisões one-way door novas em ADRs leves (em `docs/tech/adr/` se necessário).
- Sair de "Fase 1 concluída" pra "Vertical slice em produção" quando bater milestone.

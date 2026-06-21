# GusWorld

> RPG turn-based 3D estilizado. Prodígio-hacker de 11 anos contra megacorporação ciber-gótica.

**Status:** Fase 2 (vertical slice em produção). Pivot Fase 1 canonizado em [ADR-001](docs/tech/adr/ADR-001-pivot-lore-to-engine.md).

**Solo indie.** Petrinhu, 2026. Linux + Windows. Single-player puro. Godot 4.

---

## Pilares criativos (imutáveis)

1. **Magia = software.** Feitiços são scripts rúnicos compilados (Glyph/Token/Conjuro/Codex).
2. **Natureza é matemática rígida**, não caos (Fibonacci, fractais, ruído coerente).
3. **Hardware loop:** Óculos táticos + Matriz Ortodôntica + Tavus-Drive.
4. **Idade canônica 11 anos.** Prodígio analítico, não power-fantasy adulta.
5. **Setting bipartido:** megacidade ciber-gótica × Selve Sombria.

Detalhes em [`docs/design/pillars.md`](docs/design/pillars.md).

---

## Estrutura

```
gusworld/
├── CLAUDE.md            (estado atual + decisões fechadas)
├── CONTRACT.md          (disciplinas técnicas canon)
├── TODO.md              (backlog canônico via skill tab_pendencias)
├── TESTES.md            (T-sections + A-sections adaptados Godot)
├── CHANGELOG.md         (Keep a Changelog)
├── CHARS.md             (inventário canônico personagens nomeados)
├── PLACES.md            (inventário canônico lugares nomeados)
├── sinopse.md           (base canônica imutável)
├── docs/
│   ├── design/          (pillars, GDD)
│   ├── narrative/       (lore-bible, characters, factions, timeline + deep-lore)
│   ├── art/             (style guide)
│   └── tech/            (architecture, engine-modules, build, ADRs)
├── engine/              (módulos Godot reutilizáveis: orbital_camera, save, dialogue, turn-based)
├── game/                (projeto Godot game-specific; project.godot, scenes, scripts)
│   ├── project.godot
│   └── VERSION          (single source of truth versionamento)
├── assets/              (sources arte/som: Blender, Krita, Aseprite, audio raw)
└── build/               (outputs export Linux + Windows)
```

---

## Documentos canônicos

| Doc | Autoridade |
|---|---|
| [CLAUDE.md](CLAUDE.md) | Estado atual + decisões fechadas |
| [CONTRACT.md](CONTRACT.md) | Disciplinas técnicas (RFC 2119, Conventional Commits, branching, DoD, perf budget, a11y) |
| [TODO.md](TODO.md) | Backlog canônico (skill `tab_pendencias`) |
| [TESTES.md](TESTES.md) | Suíte de testes + auditorias (T+A sections adaptadas Godot) |
| [CHANGELOG.md](CHANGELOG.md) | Histórico de releases (Keep a Changelog) |
| [CHARS.md](CHARS.md) | Inventário canônico de personagens nomeados |
| [PLACES.md](PLACES.md) | Inventário canônico de lugares nomeados |
| [sinopse.md](sinopse.md) | Worldbuilding + protagonista (imutável) |
| [docs/design/pillars.md](docs/design/pillars.md) | 5 pillars canon |
| [docs/design/gdd.md](docs/design/gdd.md) | Game Design Document 1-page |
| [docs/tech/architecture.md](docs/tech/architecture.md) | Arquitetura de software |
| [docs/tech/engine-modules.md](docs/tech/engine-modules.md) | Módulos engine |
| [docs/tech/build.md](docs/tech/build.md) | Pipeline build + CI |
| [docs/tech/adr/](docs/tech/adr/) | Architecture Decision Records |

---

## Build / Run

### Pré-requisitos

- Godot 4.4+ stable (testado 4.6.1)
- Linux ou Windows
- Git

### Desenvolvimento local

```bash
# Importar projeto (primeira vez)
godot --headless --path ./game --import

# Abrir editor
godot --path ./game --editor

# Rodar (após main_scene definido)
godot --path ./game
```

### Export release

```bash
# Linux
godot --headless --path ./game --export-release "Linux/X11" ../build/linux/gusworld.x86_64

# Windows
godot --headless --path ./game --export-release "Windows Desktop" ../build/windows/gusworld.exe
```

Detalhes em [`docs/tech/build.md`](docs/tech/build.md).

---

## Tech stack

- **Engine:** Godot 4 + **C# .NET 8 AOT** (ADR-002 2026-05-19, supera decisão GDScript do ADR-001). GDScript MAY pra tooling editor-only. C++ GDExtension sob pressão perf medida.
- **Renderer:** Forward+ (Godot 4)
- **Visual:** 3D estilizado low-poly (referências: Sea of Stars, Sable, Death's Door). Sem PBR. Gradient atlas + vertex color. ~5 shaders custom.
- **Câmera:** 3/4 rotacional + zoom orbital (referência Chrono Trigger, 3D real)
- **Save format:** JSON versionado `save_version: N` + migrators forward-only desde D1
- **Localização:** Godot `tr()` + CSV. Dev pt-br. Tradução en-intl pós-release v1.0.0.
- **CI:** Forgejo Actions (esqueleto em `docs/tech/build.md`)
- **Plataformas:** Linux (AppImage + tar.gz) + Windows (sem signing G1)
- **Target hardware:** GTX 1050 + 4GB VRAM + 8GB RAM (cobre Steam Deck e laptops gaming 2017+)

---

## Roadmap

| Marco | Status | Descrição |
|---|---|---|
| Fase 1 (Concepção) | ✅ Concluída pivot ADR-001 | Pillars + GDD + lore canon ~365k pal + character specs + style guide + tech docs primeira passada |
| F2-Setup | 🔄 Em andamento | `project.godot` + CONTRACT.md + README + CHANGELOG + TESTES + CI |
| F2-Engine | ⏳ Pendente | `engine/event_bus` + `orbital_camera` (prioridade máxima) + `save_system` + `turn_based_combat` + `dialogue_system` |
| F2-Game | ⏳ Pendente | Blockout 1 área cidade + Gus placeholder + locomotion + HUD |
| F2-M.1 | ⏳ Pendente | Vertical slice coeso 5-10min jogável (alvo 4-6 meses) |
| F2-M.2 | ⏳ Pendente | Perf budget validado (60fps @ 1080p GTX 1050+) |
| F2-M.3 | ⏳ Pendente | Playtest externo (time-to-fun ≤ 5min, N ≥ 5) |
| F3-F4 | ⏳ Pendente | Production + QA + Beta |
| F5 | ⏳ Pendente | Release v1.0.0 + livro Vol 1 (bíblia worldbuilding) + Vol 2 (antologia 14 contos) |

Backlog completo em [TODO.md](TODO.md).

---

## Contribuição

**Solo indie.** Não aceita PRs externos durante G1.

Bug reports + feedback (pós-release) via issues Codeberg: <https://codeberg.org/petrinhu/gusworld/issues>

---

## ☕ Apoie o projeto

GusWorld é **freeware** (de graça, pra sempre). Se curtir e quiser ajudar a tocar o desenvolvimento (inclusive os tokens de IA que ajudam a construir o jogo):

[![Buy me a coffee and some AI tokens](resources/buymecoffe.png)](https://www.paypal.com/donate/?business=9XNZQ4RND67KL&no_recurring=0&currency_code=BRL)

**Buy me a coffee and some AI tokens.** Via [PayPal](https://www.paypal.com/donate/?business=9XNZQ4RND67KL&no_recurring=0&currency_code=BRL) _(totalmente opcional, nunca obrigatório)._

Ou aponte a câmera do celular no QR Code:

![QR Code de doação PayPal](resources/QRCode.png)

---

## Licença

**Código-fonte:** [GNU General Public License v3.0 (GPLv3)](LICENSE), copyleft forte, compatível com Qt (GPL/LGPL) sem custo. _(migrado de AGPL-3.0 para GPLv3 em 2026-06-21, pivot RF-9.)_
**Lore e arte (assets):** [CC-BY-SA-4.0](ASSETS-LICENSE.md), exceto os livros Vol1/Vol2 (direitos reservados, obra à parte). Atribuições de terceiros em [THIRD-PARTY-LICENSES.md](THIRD-PARTY-LICENSES.md).

---

## Créditos

- **Direção criativa + código + arte + narrativa + tudo:** petrinhu (2026)
- **Engine:** Godot 4 (MIT)
- **Lore-bible canon (~365k pal):** Era 1 §§1-10 + R2 Facções + R3 Settings + Bloco F/G/H/I
- **Apoio técnico narrativo:** Squad Claude Code (narrative-writer, narrative-designer, software-architect, etc) sob direção do criador supremo

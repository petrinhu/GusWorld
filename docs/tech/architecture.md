# Architecture, GusWorld G1

> **STATUS: SUPERADO (2026-06-23).** Este documento descreve a arquitetura do stack **Godot 4 + C# .NET 8 AOT** (5 camadas, AutoLoad, signals, Resources, save JSON), **aposentado** pelo pivot para engine própria C++/Qt6 (engine-design.md, 2026-06-21) e depois pelo [ADR-008](adr/ADR-008-repivot-qt-to-sdl3.md) (plataforma SDL3 + RmlUi + miniaudio, 2026-06-22). **NÃO é mais a arquitetura canônica.** A fonte canônica atual é [`docs/tech/pivot/engine-design.md`](pivot/engine-design.md) (4 camadas C++20: `core/`+`domain/` POCO puro, `platform/` SDL3, `app/`). Este arquivo fica como **registro histórico de leitura** até o decommission no marco M8; não editar como se fosse vigente. Reescrever a arquitetura para o stack atual é decisão do criador (já materializada no engine-design.md).
>
> **Autoridade (histórica):** docs/tech/architecture.md > engine-modules.md > build.md. Cross-ref ADR-001 + ADR-002 + CONTRACT.md + pillars.md.

---

## §1. Engine e linguagem

### §1.1. Engine: Godot 4.6+ Mono

- **Versão:** Godot 4.6.1+ stable (instalado `/usr/bin/godot`). Mono build obrigatório (suporte C# .NET 8).
- **Renderer:** Forward+ (canon `game/project.godot`).
- **Plataformas G1:** Linux (AppImage + tar.gz) + Windows (sem signing G1).
- **Target hardware canon:** GTX 1050 + 4GB VRAM + 8GB RAM dual-core 3.0GHz+ (cobre Steam Deck + laptops 2017+).

### §1.2. Linguagem: C# .NET 8 LTS AOT (ADR-002)

- **C# .NET 8 LTS** (suporte até 2026-11) como linguagem primária canon.
- **GDScript DEPRECATED 100%** em todo repo. Zero `.gd` files (após F2-S.MIG concluir migração).
- **C++ GDExtension** apenas sob pressão de perf medida (profile real M.2). Não em G1 default.
- **AOT compilation MUST** em release **+ dev** (mais estrito que padrão Microsoft).
- **Hot-reload:** ~5-15s rebuild via `dotnet build` watcher. Aceitável; reavaliar em F2-M.1.

#### Naming convention (Microsoft padrão)

| Element | Convention | Example |
|---|---|---|
| Classes | PascalCase | `class EventBus` |
| Métodos | PascalCase | `public void EmitGameStarted()` |
| Propriedades públicas | PascalCase | `public int Health { get; set; }` |
| Fields privados | _camelCase | `private int _signalCounter` |
| Constantes | UPPER_SNAKE_CASE | `const int MAX_HP = 100` |
| Namespaces | PascalCase dotted | `GusWorld.Engine.Foundation.Buses` |
| Files | PascalCase matching class | `EventBus.cs` ↔ `class EventBus` |
| Interfaces | I-prefix PascalCase | `interface ISaveable` |

#### Static typing

C# é statically-typed obrigatório. Zero `dynamic`, zero reflection runtime não-trimming-friendly (AOT-compat).

---

## §2. Camadas arquiteturais (5 layers canon ADR-002)

```
┌─────────────────────────────────────────────────┐
│  Front       game/scenes + game/ui (.tscn + UI) │  User-facing visual layer
├─────────────────────────────────────────────────┤
│  Mid         game/scripts (game-logic + ECS)    │  GusWorld-specific game logic
├─────────────────────────────────────────────────┤
│  Back        engine/back/ (gameplay reusável)   │  Engine-level gameplay modules
├─────────────────────────────────────────────────┤
│  Foundation  engine/foundation/ (core infra)    │  Critical non-game-specific
├─────────────────────────────────────────────────┤
│  Assets      assets/ (sources arte/som)         │  Source files (Blender, Krita)
└─────────────────────────────────────────────────┘
```

### §2.1. Foundation layer

**Críticos non-game-specific.** Reusáveis em qualquer projeto Godot.

```
engine/foundation/
├── buses/                  (4 buses split, ADR-002 batch 3)
│   ├── GameStateBus.cs     (game_started, game_paused, game_saved, game_loaded)
│   ├── PlayerBus.cs        (player_moved, player_interacted, player_hp_changed, player_died)
│   ├── CombatBus.cs        (combat_started, combat_ended, turn_started, turn_ended, action_resolved)
│   └── UIBus.cs            (dialogue_shown, dialogue_choice_made, menu_opened, menu_closed)
├── save_system/            (JSON System.Text.Json source-gen + HMAC-SHA256 anti-cheat)
├── localization/           (custom MD loader + AutoLoad)
├── input_remap/            (InputMap remap + persist em save)
├── audio_director/         (audio bus + AudioStreamPlayer wrappers)
└── scene_router/           (fade in/out + async loading)
```

### §2.2. Back layer

**Gameplay reusável.** Padrões turn-based + adventure indie genéricos.

```
engine/back/
├── orbital_camera/         (Camera3D + SpringArm3D + Node3D pivot, 3/4 rotacional)
├── turn_combat/            (state machine + initiative queue)
├── party/                  (roster ≤7, active ≤3, swap mechanics)
├── card_engine/            (deck operations + CardResource + effects)
├── dialogue/               (branching + variables persist)
└── puzzle_kit/             (Vetor do Gambito predição + outros futuros)
```

### §2.3. Mid layer

**GusWorld-specific game-logic.**

```
game/scripts/
├── knowledge_system/       (Knowledge Progression score híbrido + gates)
├── arcs/                   (state machine progressão arcos companions)
├── encounters/             (encounter design + balance specifics)
├── characters/             (Gus + companions stats + abilities específicos)
└── world/                  (world state + flags + canon variables)
```

### §2.4. Front layer

**User-facing visual + interaction.**

```
game/
├── scenes/                 (.tscn cenas Godot)
│   ├── main.tscn
│   ├── levels/             (distrito_inferiores.tscn, selve_sombria_01.tscn, etc)
│   ├── characters/         (gus.tscn, npcs/, enemies/)
│   └── ui/                 (hud.tscn, menus/, dialogue_ui.tscn)
├── translations/           (pt_br.md + en_intl.md custom format)
└── tools/                  (validate_autoloads.cs + dev helpers, pós-F2-S.MIG)
```

### §2.5. Assets layer

**Source files.** Não-código.

```
assets/
├── models/                 (.blend Blender sources)
├── textures/               (.kra Krita + atlas gradient PNG export)
├── sprites/                (.aseprite UI + 2D)
├── sfx/                    (audio raw .wav)
└── music/                  (DAW source + .ogg export)
```

### §2.6. Regras de dependência

- **Foundation NÃO importa de** Back/Mid/Front/Assets.
- **Back importa de** Foundation only.
- **Mid importa de** Foundation + Back.
- **Front importa de** Foundation + Back + Mid (Front = composição final).
- **Assets** são consumidos por Front (load runtime), não importam código.

Audit via CI: `grep -r "GusWorld.Game" engine/` deve retornar empty.

---

## §3. Engine repo separado (gus_dragon-engine)

ADR-002 batch 3 canonizou repo separado **desde agora**.

### §3.1. Setup

- **Repo:** `gus_dragon-engine` em Codeberg `petrinhu/gus_dragon-engine`.
- **Inclusão em gusworld:** via `git submodule add`.
- **Path em gusworld:** `engine/` (root level).
- **Convenção:** game project `game/engine` é symlink pra `../engine/` (que é submodule pra repo separado).

### §3.2. Versionamento

- Semver simples por commit hash em submodule.
- Game cravado em commit específico do submodule.
- Bump deliberado: revisar mudanças engine antes de bumpar reference em gusworld.

### §3.3. Quando publicar (ordem standalone ADR-002 batch 7)

1. **save_system** (1º, mais reutilizável cross-genre).
2. **event_bus + buses split** (2º).
3. **localization** (3º).
4. **input_remap** (4º).
5. **orbital_camera** (5º, alto valor reuso indie).
6. **card_engine + dialogue** (6º-7º).

Publicação standalone = repo público próprio + Godot Asset Library opcional (pós-1.0.0).

---

## §4. Sistemas core (visão detalhada)

### §4.1. Orbital camera (Back layer)

**Path:** `game/scripts/back/orbital_camera/OrbitalCamera.cs`

Setup canon Godot 4:
```
Node3D (pivot, follows player position)
└── SpringArm3D
    └── Camera3D
```

- Rotação 3/4 + zoom min/max.
- Spring length ajusta dinamicamente (collision-aware).
- `collision_mask` exclui player layer.
- Tests: `engine/tests/OrbitalCameraTests.cs (TODO)` (xUnit).

### §4.2. Turn-based combat (Back layer)

**Path:** `engine/back/turn_combat/TurnCombatManager.cs`

State machine explícita:
- `Round` then `Turn` then `ActionSelect` then `Resolve` then `TurnEnd`
- Initiative queue (priority queue por agility stat).
- Signals via `CombatBus.Instance`.
- Tests cover state transitions + edge cases (tie-breaking, status effects).

### §4.3. Card engine (Back layer)

**Path:** `engine/back/card_engine/CardEngine.cs`

- `CardResource` (Godot Resource wrapper + C# DataClass POCO, ADR-002 batch 4).
- Deck ops: shuffle, draw, discard, reshuffle.
- Effects via Callable-like pattern (delegate-based em C#, AOT-friendly).
- Tests cover deck ops + effect resolution.

### §4.4. Dialogue (Back layer)

**Path:** `engine/back/dialogue/DialogueSystem.cs`

- Decisão lib (Ink vs DialogueManager vs custom): ADR-003 futuro em F2-E.6 task.
- Branching base + variáveis persistentes via SaveSystem.
- Signals via `UIBus.Instance.DialogueShown` + `DialogueChoiceMade`.

### §4.5. Party (Back layer)

**Path:** `engine/back/party/PartyManager.cs`

- Roster total: ≤7 (Gus + 6 companions canon).
- Active in combat: ≤3.
- Swap mechanics: trigger em hub seguro (não em combate).

### §4.6. Save/load (Foundation layer)

**Path:** `engine/foundation/save_system/SaveManager.cs`

#### Formato

- **JSON via `System.Text.Json` source-generated** (AOT-compat, perf max, no reflection).
- Vector3/Color/Quaternion serializados via custom `JsonConverter<T>`.
- Schema versionado `save_version: N` desde D1.
- Forward-only migrators (`Migrator_N_to_N_plus_1.cs`).

#### Anti-cheat (ADR-002 batch 5)

**HMAC-SHA256 com chave embarcada** (strict anti-modding):

```csharp
// Estrutura save
{
    "save_version": 1,
    "payload": { ... game state ... },
    "integrity_hmac": "abc123def...sha256hex"
}
```

- Em save: compute `HMAC-SHA256(payload, embedded_key)` resulta em `integrity_hmac`.
- Em load: recompute + compare. Mismatch = **reject load** (não tenta repair).
- Backup chain N=3: 3 autosaves anteriores preservados em rotation. Se atual rejected, oferece anterior.
- Chave embarcada hardcoded na build (não em ENV, não em config). Trade-off: modders avançados podem extrair, mas casual cheating bloqueado.

#### Caminhos cross-platform

- Linux: `~/.local/share/godot/app_userdata/GusWorld/saves/`
- Windows: `%APPDATA%/Godot/app_userdata/GusWorld/saves/`
- Acesso via `user://saves/slot_N.json`.

### §4.7. Inventory (Mid layer)

**Path:** `game/scripts/world/InventoryManager.cs`

- Equipment slots: `glasses`, `ortho_matrix`, `drive` (canônicos Gus, **game-specific NÃO em engine**).
- Items consumíveis + key items + lore docs descobertos.
- Persist via save.

### §4.8. Puzzle hooks (Back layer)

**Path:** `engine/back/puzzle_kit/PuzzleKit.cs`

- Vetor do Gambito predição (turn-based preview).
- Outros puzzles canon (a definir conforme arcos).
- Padrão extensível via interface `IPuzzle`.

---

## §5. Save format

Canonizado em §4.6 + CONTRACT.md §7. Resumo:

- JSON `System.Text.Json` source-generated.
- `save_version: N` + forward-only migrators desde D1.
- HMAC-SHA256 anti-cheat.
- 3-save backup chain rotation.
- Custom converters Vector3/Color/Quaternion.

---

## §6. Localização (Foundation layer)

Canonizado em F2-S.11 + CONTRACT.md §6 (Gate 4 a11y).

- **Custom MD loader** (decisão criador supremo, não-CSV canon Godot).
- AutoLoad `Localization` C# pattern (singleton).
- Locales: `pt_br` (dev primário), `en_intl` (alvo pós-v1.0.0).
- Fallback chain: corrente, en_intl, pt_br, literal.
- Hot-reload em dev via `Localization.Instance.Reload()`.

Detalhes em `engine/foundation/localization/README.md`.

---

## §7. CI/CD strategy

- **Forgejo Actions** em Codeberg.
- Pipeline:
  1. `lint`: `dotnet format --verify-no-changes` + `dotnet build /warnaserror`.
  2. `test`: `dotnet test` (xUnit).
  3. `import`: `godot --headless --path ./game --import`.
  4. `build`: `dotnet publish -c Release` + `godot --export-release` (Linux + Windows).
  5. `aot_check`: validar publish AOT exit 0 sem warnings.
- CI image: `mcr.microsoft.com/dotnet/sdk:8.0` + Godot Mono binary cached.

Detalhes em `build.md`.

---

## §8. Branching strategy

Canonizado em CONTRACT.md §3:

- `main` sempre verde.
- `feat/*` pra features grandes (≥5 arquivos OU one-way door OU toca save).
- **12h cooling-off** em features grandes antes de merge.
- Squash merge feat to main; fast-forward bugfix to main.

---

## §9. Acessibilidade gates D1

4 gates D1 obrigatórios pra v1.0.0 (CONTRACT.md §6):

1. Controles remappáveis (InputMap + persist).
2. Contraste WCAG 2.2 AA (4.5:1 corpo, 3:1 large/UI).
3. Reduce motion toggle (screen shake, motion blur, parallax).
4. Subtitles + closed captions (fonte ajustável, BG opcional).

Implementação em Mid/Front layers conforme módulos UI.

---

## §10. Anti-objetivos G1 (NÃO fazer)

- ❌ Multiplayer (single-player puro decisão fechada).
- ❌ Modding API público.
- ❌ Cloud save (3-save backup chain local suficiente).
- ❌ Signing Windows G1 (SmartScreen warning aceito).
- ❌ Steamworks G1 (Linux + Windows DRM-free initial).
- ❌ Asset Library publish em G1 (publicar pós-1.0.0).
- ❌ Localization en_intl pré-v1.0.0 (dev pt-br only).
- ❌ AI/LLM em runtime (gameplay determinístico, AI tradicional).

---

## §11. Trade-offs aceitos (com justificativa)

- **C# hot-reload mais lento que GDScript:** aceito pra ganhar perf nativa AOT (ADR-002 §5).
- **AOT MUST em dev** (mais estrito que padrão): garante perf consistente, evita surprises em release.
- **HMAC-SHA256 anti-cheat strict:** trade-off modding casual bloqueado pra anti-cheat. Solo single-player aceita.
- **5 camadas (não 3):** overhead organizacional pra disciplina arquitetural máxima.
- **Engine repo separado desde D1:** custo setup ~2-4h pra disciplina early.
- **CSV/PO canon Godot rejeitado** (i18n custom MD): trade-off custom loader pra legibilidade lore longa.
- **Backup chain N=3** (não maior): trade-off espaço disk pra robustez. N=3 cobre 95% casos.

---

## §12. AOT compilation + perf budget detalhado

### §12.1. AOT policy

- **AOT MUST em release + dev** (ADR-002 batch 1).
- Justificativa: garantia perf máxima sem variabilidade JIT vs AOT entre dev e prod.
- Override: emergência debug (debugger não suporta AOT bem), commit body `OVERRIDE CONTRACT §AOT` + follow-up task.

### §12.2. AOT-compatibility rules

- Zero `dynamic` keyword.
- Reflection limitada: usar `[DynamicallyAccessedMembers]` quando necessário.
- Source generators preferidos sobre runtime reflection (`System.Text.Json`, MessagePack, etc).
- Trim warnings = build errors. CI flags.

### §12.3. Perf budget (cross-ref CONTRACT.md §5)

Target hardware: GTX 1050 + 4GB VRAM + 8GB RAM dual-core 3.0GHz.

| Métrica | Alvo | Trigger refac |
|---|---|---|
| Frame rate sustentado | 60fps @ 1080p | < 55fps por > 3s |
| Frame time pico | < 16.6ms | > 25ms p99 |
| Memory RAM | < 1.5GB residente | > 2GB típica |
| Memory VRAM | < 1GB | > 2GB pico |
| Load scene | < 3s | > 5s |
| Save/load roundtrip + HMAC | < 1s | > 2s |
| Cold start | < 5s | > 8s |
| Turn resolve combat | < 100ms | > 250ms |

### §12.4. Profile workflow

- **dotnet-trace** pra CPU profiling C# (mais granular que Godot profiler).
- **Godot Profiler** built-in pra rendering + scene tree.
- **Coverlet** + OpenCover pra coverage tests.
- Profile em milestones (M.1, M.2, M.3, M.4).
- Otimização **MUST** ser data-driven (profiler real).
- GDExtension C++ MAY ser introduzido quando hot path C# medido < target.

---

## §13. Cross-refs

- `docs/tech/adr/ADR-001-pivot-lore-to-engine.md` (pivot Fase 1 to Fase 2).
- `docs/tech/adr/ADR-002-csharp-aot-over-gdscript.md` (esta arquitetura).
- `docs/tech/engine-modules.md` (módulos detalhados).
- `docs/tech/build.md` (pipeline build + CI).
- `CONTRACT.md` (disciplinas técnicas).
- `TESTES.md` (T+A sections).
- `docs/design/pillars.md` (5 pillars criativos imutáveis).
- Memory `reference_godot_docs.md` + `reference_godot_csharp.md`.

---

**Última revisão:** 2026-05-19. Pós-ADR-002 30 decisões canonizadas em 8 batches AskUser. Revisão obrigatória em F2-M.1 (vertical slice done).

# Engine Modules (gus_dragon-engine)

> **STATUS: SUPERADO (2026-06-23).** Este documento cataloga os módulos da engine no stack **C# .NET 8 + Godot** (`engine/foundation/` + `engine/back/`, AutoLoads, signals, `Resource`, AOT), **aposentado** pelo pivot para C++/Qt6 (engine-design.md) e depois pelo [ADR-008](adr/ADR-008-repivot-qt-to-sdl3.md) (SDL3 + RmlUi + miniaudio). **NÃO é mais o catálogo canônico.** A fonte canônica atual é [`docs/tech/pivot/engine-design.md`](pivot/engine-design.md) (módulos C++20: `core/time|rng|ecs_lite|resource|events`, `domain/save|i18n|progression|templates|combat`, `platform/window|render2d|input|audio|fs`). O decommission do stack Godot/C# fechou no marco **M8 (2026-07-22)**; fica como **registro histórico PERMANENTE** (não editar como se fosse vigente, não mover nem apagar; preserva o link do "cemitério de ideias mortas" do projeto).
>
> **Escopo (histórico):** módulos C# .NET 8 em `engine/foundation/` (Foundation layer) + `engine/back/` (Back layer). Repo separado `gus_dragon-engine` em Codeberg (ADR-002 batch 3).

---

## §1. Visão geral

Engine modular C# .NET 8 AOT. Distribuição por camada arquitetural:

| Camada | Path | Característica |
|---|---|---|
| Foundation | `engine/foundation/` | Críticos non-game-specific. Reusáveis em qualquer projeto Godot 4. |
| Back | `engine/back/` | Gameplay reusável (turn-based + adventure indie). |

Cada módulo C# tem:
- `<Module>.cs` (implementação principal).
- `README.md` (API pública + uso).
- `tests/<Module>Tests.cs` (xUnit suite).
- Namespace: `GusWorld.Engine.<Layer>.<Module>`.

---

## §2. Foundation layer (6 módulos)

### §2.1. `engine/foundation/buses/` (4 buses split, ADR-002 batch 3)

Bus splits canonizados pra evitar god-object EventBus único.

#### §2.1.1. `GameStateBus.cs`

Game lifecycle signals.

```csharp
namespace GusWorld.Engine.Foundation.Buses;

public partial class GameStateBus : Node
{
    public static GameStateBus Instance { get; private set; }

    [Signal] public delegate void GameStartedEventHandler();
    [Signal] public delegate void GamePausedEventHandler();
    [Signal] public delegate void GameResumedEventHandler();
    [Signal] public delegate void GameSavedEventHandler(int slot);
    [Signal] public delegate void GameLoadedEventHandler(int slot);

    public override void _Ready() => Instance = this;
    public override void _ExitTree() { if (Instance == this) Instance = null; }
}
```

#### §2.1.2. `PlayerBus.cs`

Player events.

```csharp
[Signal] PlayerMoved(Vector3 from, Vector3 to)
[Signal] PlayerInteracted(string targetName)
[Signal] PlayerHpChanged(int current, int maximum)
[Signal] PlayerDied()
```

#### §2.1.3. `CombatBus.cs`

Turn-based combat events.

```csharp
[Signal] CombatStarted(Godot.Collections.Array combatants)
[Signal] CombatEnded(string winner)
[Signal] TurnStarted(string combatantName)
[Signal] TurnEnded(string combatantName)
[Signal] ActionResolved(string actor, string target, string actionType, int value)
```

#### §2.1.4. `UIBus.cs`

UI events.

```csharp
[Signal] DialogueShown(string dialogueId)
[Signal] DialogueChoiceMade(string dialogueId, int choiceIndex)
[Signal] MenuOpened(string menuName)
[Signal] MenuClosed(string menuName)
```

**Total signals across 4 buses:** 5 + 4 + 5 + 4 = 18 (orçamento anti-god-object 20 mantido).

**AutoLoad registro em `game/project.godot`:**
```ini
[autoload]
GameStateBus="*res://scripts/foundation/buses/GameStateBus.cs"
PlayerBus="*res://scripts/foundation/buses/PlayerBus.cs"
CombatBus="*res://scripts/foundation/buses/CombatBus.cs"
UIBus="*res://scripts/foundation/buses/UIBus.cs"
```

### §2.2. `engine/foundation/save_system/`

**JSON System.Text.Json source-generated + HMAC-SHA256 anti-cheat** (ADR-002 batch 5).

API pública (resumo):

```csharp
namespace GusWorld.Engine.Foundation.SaveSystem;

public partial class SaveManager : Node
{
    public static SaveManager Instance { get; private set; }

    public void SaveGame(int slot, SaveData data);
    public SaveData LoadGame(int slot);  // throws SaveIntegrityException se HMAC falha
    public bool HasSave(int slot);
    public void DeleteSave(int slot);

    // Backup chain rotation N=3
    public IReadOnlyList<int> GetBackupSlots();
}
```

Migrators forward-only em `migrators/Migrator_<N>_to_<N_plus_1>.cs`.

Tests obrigatórios: roundtrip + HMAC reject + migrator chain.

### §2.3. `engine/foundation/localization/`

Custom MD loader (decisão criador supremo, não-CSV canon Godot). F2-S.11 já implementado em GDScript; migração C# em F2-S.MIG.

```csharp
namespace GusWorld.Engine.Foundation.Localization;

public partial class Localization : Node
{
    public static Localization Instance { get; private set; }

    public string TrMd(string key, object[] args = null);
    public void SetLocale(string locale);
    public string GetLocale();
    public IReadOnlyList<string> GetAvailableLocales();
    public void Reload();  // dev-only iteration
}

public static class MdTranslationLoader
{
    public static Dictionary<string, string> LoadFromFile(string path);
    public static Dictionary<string, string> Parse(string content);
}
```

### §2.4. `engine/foundation/input_remap/`

InputMap remap canon (CONTRACT.md §6 Gate 1 a11y).

```csharp
public partial class InputRemapManager : Node
{
    public static InputRemapManager Instance { get; private set; }

    public void RemapAction(StringName action, InputEvent newEvent);
    public void ResetToDefaults();
    public Dictionary<StringName, InputEvent[]> ExportBindings();
    public void ImportBindings(Dictionary<StringName, InputEvent[]> bindings);
}
```

Persist via SaveSystem.

### §2.5. `engine/foundation/audio_director/`

Audio bus + AudioStreamPlayer wrappers.

```csharp
public partial class AudioDirector : Node
{
    public static AudioDirector Instance { get; private set; }

    public void PlaySfx(string streamPath, float volumeDb = 0f);
    public void PlayMusic(string streamPath, float fadeSeconds = 1.0f);
    public void StopMusic(float fadeSeconds = 1.0f);
    public void SetBusVolume(string busName, float volumeDb);
}
```

Buses: Master, Music, SFX, UI, Voice.

### §2.6. `engine/foundation/scene_router/`

Fade in/out + async scene loading.

```csharp
public partial class SceneRouter : Node
{
    public static SceneRouter Instance { get; private set; }

    public async Task GoToScene(string scenePath, float fadeSeconds = 0.5f);
    public async Task ReloadCurrent();
    public string GetCurrentScenePath();
}
```

---

## §3. Back layer (6 módulos)

### §3.1. `engine/back/orbital_camera/`

Camera3D + SpringArm3D + Node3D pivot (canon Godot 4 + memo `reference_godot_docs.md`).

```csharp
namespace GusWorld.Engine.Back.OrbitalCamera;

public partial class OrbitalCamera : Node3D
{
    [Export] public float ZoomMin { get; set; } = 2.0f;
    [Export] public float ZoomMax { get; set; } = 12.0f;
    [Export] public float RotationSpeed { get; set; } = 2.0f;
    [Export] public Node3D FollowTarget { get; set; }

    public void RotateBy(float deltaX, float deltaY);
    public void ZoomBy(float delta);
    public void ResetView();
}
```

Cenas teste em `tests/OrbitalCameraTests.cs`.

### §3.2. `engine/back/turn_combat/`

State machine + initiative queue.

```csharp
public partial class TurnCombatManager : Node
{
    public enum CombatState { Idle, Round, Turn, ActionSelect, Resolve, TurnEnd, Ended }

    public CombatState CurrentState { get; private set; }
    public IReadOnlyList<ICombatant> InitiativeOrder { get; private set; }

    public void StartCombat(IEnumerable<ICombatant> participants);
    public void SubmitAction(ICombatAction action);
    public void EndCombat(string winner);
}
```

Tests cobrem: tie-breaking, status effects, action queue ordering.

### §3.3. `engine/back/party/`

Roster ≤7, active ≤3, swap mechanics.

```csharp
public partial class PartyManager : Node
{
    public IReadOnlyList<PartyMember> Roster { get; }
    public IReadOnlyList<PartyMember> Active { get; }

    public void AddMember(PartyMember member);
    public void SetActiveSlot(int slot, PartyMember member);
    public bool CanSwap();  // false em combate
}
```

### §3.4. `engine/back/card_engine/`

Deck + CardResource + effects.

```csharp
public partial class CardEngine : Node
{
    public IReadOnlyList<CardResource> Hand { get; }
    public IReadOnlyList<CardResource> Deck { get; }
    public IReadOnlyList<CardResource> Discard { get; }

    public void Draw(int count);
    public void Play(CardResource card, ICombatant target);
    public void Shuffle();
    public void Reshuffle();
}

public partial class CardResource : Resource
{
    [Export] public string Id { get; set; }
    [Export] public string NameKey { get; set; }  // i18n key
    [Export] public int Cost { get; set; }
    [Export] public CardEffectType EffectType { get; set; }
    [Export] public int EffectValue { get; set; }
}
```

### §3.5. `engine/back/dialogue/`

Branching + variáveis persistentes.

ADR-003 futuro pra decisão de lib (custom vs Ink C# port vs DialogueManager port).

```csharp
public partial class DialogueSystem : Node
{
    public static DialogueSystem Instance { get; private set; }

    public void StartDialogue(string dialogueId);
    public void Continue();
    public void Choose(int choiceIndex);
    public Variant GetVariable(string name);
    public void SetVariable(string name, Variant value);
}
```

### §3.6. `engine/back/puzzle_kit/`

Padrão extensível via interface `IPuzzle`.

```csharp
public interface IPuzzle
{
    string PuzzleId { get; }
    void Initialize();
    bool CheckSolution(Variant input);
    void OnSolved();
}

public partial class PuzzleKit : Node
{
    public static PuzzleKit Instance { get; private set; }
    public void RegisterPuzzle(IPuzzle puzzle);
    public IPuzzle GetPuzzle(string puzzleId);
}
```

Vetor do Gambito implementa `IPuzzle`.

---

## §4. Diagrama de dependências

```
Foundation (engine/foundation/)
  ├── buses (4 buses)              ← raíz, depende de nada
  ├── save_system                  ← usa buses (signals)
  ├── localization                 ← independente
  ├── input_remap                  ← usa save_system (persist)
  ├── audio_director               ← usa buses (signals)
  └── scene_router                 ← usa buses + localization

Back (engine/back/)
  ├── orbital_camera               ← Foundation: buses + input_remap
  ├── turn_combat                  ← Foundation: buses + save_system
  ├── party                        ← Foundation: buses + save_system
  ├── card_engine                  ← Foundation: buses + save_system
  ├── dialogue                     ← Foundation: buses + save_system + localization
  └── puzzle_kit                   ← Foundation: buses

Mid (game/scripts/)                 ← Foundation + Back
Front (game/scenes/, game/ui/)      ← Foundation + Back + Mid + Assets
```

**Audit rule (CI):** `grep -r "GusWorld\.Game" engine/` MUST retornar empty.

---

## §5. Candidatos a publicação standalone (ordem ADR-002 batch 7)

Sequência canonizada baseada em valor de reuso cross-genre:

1. **`save_system`** (1º). Reusable em qualquer projeto Godot.
2. **`buses` (event_bus split)** (2º). Pattern global pra signals desacoplados.
3. **`localization`** (3º). Custom MD format ainda nicho, mas exemplar.
4. **`input_remap`** (4º). A11y gate D1, demanda comunidade.
5. **`orbital_camera`** (5º). Alto valor indie 3rd-person.
6. **`card_engine`** (6º). Específico turn-based, valor médio.
7. **`dialogue`** (7º). Depende de decisão ADR-003 da lib.

**Publicação = repo público `gus_dragon-engine` + Godot Asset Library opcional pós-1.0.0.**

---

## §6. Testes obrigatórios por módulo (xUnit)

Cross-ref TESTES.md T1.

| Módulo | Cobertura alvo | Suites críticas |
|---|---|---|
| save_system | 80% | Roundtrip + HMAC + Migrators chain |
| buses (4) | 60% | Emit + connect + disconnect + cleanup |
| localization | 70% | tr_md + fallback + interpolation + missing key |
| input_remap | 70% | Remap + persist + reset defaults |
| turn_combat | 70% | State transitions + initiative + actions |
| card_engine | 70% | Shuffle + draw + play + reshuffle |
| orbital_camera | 50% | Input handling (rotation, zoom) |
| dialogue | 60% | Branching + variables persist |
| audio_director | 40% | Bus volume + stream play |
| scene_router | 50% | Async load + fade |
| party | 60% | Roster + active swap |
| puzzle_kit | 50% | Register + solve flow |

---

## §7. Cross-refs

- `docs/tech/architecture.md` (§2 camadas, §4 sistemas detalhados).
- `docs/tech/build.md` (pipeline build + .csproj structure).
- `CONTRACT.md` §4 DoD por tipo.
- `TESTES.md` T+A sections.
- ADR-002 (canon C# AOT + camadas + buses split).
- Memory `reference_godot_csharp.md`.

---

**Última revisão:** 2026-05-19. Pós-ADR-002 canonization. Revisão obrigatória em F2-M.1.

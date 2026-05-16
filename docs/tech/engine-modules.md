# GusWorld Engine — Módulos

> Catálogo dos módulos reutilizáveis em `/engine/addons/`. Cada um é um Godot addon standalone (tem `plugin.cfg`). Interface pública é o **único contrato estável** — internals podem refatorar livre.

---

## 1. Visão geral

| Módulo | Tipo | Autoload? | Standalone-ready? |
|---|---|---|---|
| `event_bus` | core | sim | sim |
| `localization` | core | sim | sim |
| `audio_director` | core | sim | sim |
| `input_remap` | core | sim | sim |
| `save_system` | core | sim | sim (após maturação) |
| `scene_router` | core | sim | sim |
| `orbital_camera` | gameplay | não (nó) | **sim — primeiro candidato a publicar** |
| `turn_combat` | gameplay | parcial | sim (após maturação) |
| `party` | gameplay | sim | sim |
| `card_engine` | gameplay | parcial | sim — útil em qualquer TCG/deckbuilder |
| `dialogue` | gameplay | sim | sim — útil em qualquer RPG |
| `puzzle_kit` | gameplay | não (nós) | sim |

"Standalone-ready" = pode virar addon publicável independente do GusWorld (sem dependência game-specific).

---

## 2. Módulos core

### 2.1. `event_bus`

**Responsabilidade:** signals globais tipados pra eventos cross-system. Anti god-object: limite ~20 sinais.

**Interface pública (`EventBus` autoload):**
```gdscript
signal game_paused(paused: bool)
signal scene_changing(from: String, to: String)
signal save_requested(slot: int)
signal save_completed(slot: int, success: bool)
signal combat_started(encounter_id: String)
signal combat_ended(result: int)   # enum Victory/Defeat/Flee
signal dialogue_started(dialogue_id: String)
signal dialogue_ended(dialogue_id: String)
signal flag_changed(flag_name: String, value: Variant)
# ... (manter <=20 — quando estourar, criar bus dedicado)
```

**Standalone:** sim. Zero dependência.

---

### 2.2. `localization`

**Responsabilidade:** wrapper sobre `tr()` + `tr_n()` com reload runtime e formatação por dicionário.

**Interface pública (`L10n` autoload):**
```gdscript
func t(key: String, args: Dictionary = {}) -> String
func tn(key: String, key_plural: String, count: int, args: Dictionary = {}) -> String
func set_locale(code: String) -> void
func get_available_locales() -> Array[String]
signal locale_changed(new_locale: String)
```

**Dependências:** nenhuma (usa apenas TranslationServer nativo do Godot).

**Standalone:** sim.

---

### 2.3. `audio_director`

**Responsabilidade:** gerencia AudioBuses (Master/Music/SFX/Voice/UI), crossfade entre tracks, ducking de música durante diálogo.

**Interface pública (`Audio` autoload):**
```gdscript
func play_music(stream: AudioStream, fade_sec: float = 1.0) -> void
func stop_music(fade_sec: float = 1.0) -> void
func play_sfx(stream: AudioStream, position: Vector3 = Vector3.ZERO) -> void
func play_ui(stream: AudioStream) -> void
func set_bus_volume(bus: String, db: float) -> void
func duck_music(amount_db: float, duration_sec: float) -> void
```

**Dependências:** `event_bus` (opcional: escuta `dialogue_started/ended` pra duck automático).

**Standalone:** sim (dependência de event_bus é opcional via if-exists).

---

### 2.4. `input_remap`

**Responsabilidade:** tabela de bindings editável runtime, persistida em `user://input.cfg`, suporta keyboard+mouse+gamepad.

**Interface pública (`InputMap` autoload):**
```gdscript
func get_binding(action: String) -> Array[InputEvent]
func set_binding(action: String, events: Array[InputEvent]) -> void
func reset_to_defaults() -> void
func save() -> void
func load() -> void
signal binding_changed(action: String)
```

**Dependências:** nenhuma.

**Standalone:** sim.

---

### 2.5. `save_system`

**Responsabilidade:** snapshot/restore de estado do jogo. Slots, autosave, versionamento, migrações.

**Interface pública (`SaveSystem` autoload):**
```gdscript
func save(slot: int, payload: Dictionary) -> bool
func load(slot: int) -> Dictionary    # já migrado pra schema atual
func list_slots() -> Array[Dictionary] # [{slot, timestamp, playtime, scene}]
func delete(slot: int) -> bool
func quicksave() -> bool
func quickload() -> Dictionary
func autosave() -> bool
const CURRENT_VERSION := 1
```

**Como games registram dados:** cada sistema do game implementa `save_data() -> Dictionary` e `load_data(d: Dictionary)`. SaveSystem agrega via signals do `event_bus` (`save_requested` → coleta de todos os listeners).

**Migrações:** `addons/save_system/migrations/migrate_N_to_M.gd`, função pura `static func migrate(d: Dictionary) -> Dictionary`.

**Dependências:** `event_bus`.

**Standalone:** sim (após estabilização — schema é game-specific, mas o framework é genérico).

---

### 2.6. `scene_router`

**Responsabilidade:** stack de cenas com transições fade, persistência opcional de estado entre cenas, prevenção de double-load.

**Interface pública (`SceneRouter` autoload):**
```gdscript
func goto(scene_path: String, transition: String = "fade") -> void
func push(scene_path: String) -> void       # empilha (overlay)
func pop() -> void
func current() -> String
signal scene_changed(path: String)
```

**Dependências:** `event_bus`.

**Standalone:** sim.

---

## 3. Módulos gameplay

### 3.1. `orbital_camera`

**Responsabilidade:** Camera3D 3/4 isométrica-livre, orbital, com zoom e follow.

**Interface pública (nó `OrbitalCamera` extends Node3D):**
```gdscript
@export var target: Node3D
@export var orbit_stops: int = 8           # 0 = contínuo
@export var zoom_min: float = 6.0
@export var zoom_max: float = 20.0
@export var pitch_degrees: float = 32.0

func rotate_left() -> void
func rotate_right() -> void
func zoom_in() -> void
func zoom_out() -> void
func snap_to_angle(degrees: float) -> void
func shake(intensity: float, duration_sec: float) -> void
func set_target(node: Node3D) -> void
```

**Dependências:** nenhuma.

**Standalone:** **sim — primeiro candidato a publicar na Asset Library** quando estável. Câmera isométrica orbital genérica é demanda forte em Godot 4.

---

### 3.2. `turn_combat`

**Responsabilidade:** state machine de turnos, fila de ações, hook de IA.

**Interface pública (`TurnCombat` autoload + nó `CombatScene`):**
```gdscript
# TurnCombat (autoload)
func start_encounter(actors: Array[CombatActor], encounter_id: String) -> void
func end_encounter(result: int) -> void
func current_actor() -> CombatActor
func queue_action(action: CombatAction) -> void

# CombatActor (Resource)
@export var name: String
@export var stats: Dictionary    # HP/MP/AP/Speed/Defense
@export var ai_brain: Resource   # null = controlado pelo jogador
@export var team: int            # 0=player, 1=enemy, etc.

# CombatAction (Resource)
@export var id: String
@export var cost_ap: int
@export var range_tiles: int
@export var target_type: int     # enum self/ally/enemy/tile
@export var effect: Callable
```

**Estados (signals):** `round_started`, `turn_started(actor)`, `action_resolved(action)`, `turn_ended(actor)`, `round_ended`, `encounter_ended(result)`.

**Hooks Vetores do Gambito:** método `simulate_next_turn(actor) -> SimulationResult` que executa branch dry-run pra preview.

**Dependências:** `event_bus`, `party` (opcional — combate pode usar actors avulsos).

**Standalone:** sim (após maturação).

---

### 3.3. `party`

**Responsabilidade:** roster de personagens, ativos vs reserva, stats/equipment.

**Interface pública (`Party` autoload):**
```gdscript
func add_member(member: PartyMember) -> void
func remove_member(id: String) -> void
func set_active(ids: Array[String]) -> void   # quais entram em combate
func get_active() -> Array[PartyMember]
func get_all() -> Array[PartyMember]
func get_member(id: String) -> PartyMember
```

**Dependências:** `event_bus` (anuncia mudanças via signal).

**Standalone:** sim.

---

### 3.4. `card_engine`

**Responsabilidade:** deck/hand/discard/exile, draw/play/discard, combo detection.

**Interface pública (`CardEngine` autoload + Resources):**
```gdscript
# CardEngine
func create_deck(cards: Array[CardResource], seed: int = 0) -> Deck
func draw(deck: Deck, n: int) -> Array[CardResource]
func play(deck: Deck, card: CardResource, context: Dictionary) -> bool
func discard(deck: Deck, cards: Array[CardResource]) -> void
func find_combos(hand: Array[CardResource], combo_table: Array) -> Array[Combo]

# CardResource
@export var id: String
@export var display_name_key: String  # localization key
@export var cost: Dictionary          # {"ap": 1, "mp": 2}
@export var tags: Array[String]       # ["raiz","eletrico","gambito"]
@export var effect: Callable
@export var art: Texture2D
```

**Dependências:** `event_bus`.

**Standalone:** **sim — segundo candidato a publicar.** TCG/deckbuilder framework genérico é raro em Godot.

---

### 3.5. `dialogue`

**Responsabilidade:** grafo de diálogo, parser de arquivo `.dialogue`, runtime (nodes/choices/conditions/variables).

**Interface pública (`DialogueRunner` autoload):**
```gdscript
func start(dialogue_id: String) -> void
func choose(option_index: int) -> void
func get_current_line() -> DialogueLine    # {speaker, text_key, choices, ...}
func set_variable(name: String, value: Variant) -> void
func get_variable(name: String) -> Variant
signal line_changed(line: DialogueLine)
signal dialogue_finished(dialogue_id: String)
```

**Formato `.dialogue`:** texto custom inspirado em Ink/Yarn (escolher Yarn-like por simplicidade). Compilado em `DialogueGraph` (Resource) no import via `EditorImportPlugin`.

**Dependências:** `event_bus`, `localization`.

**Standalone:** sim (depende de localization, mas é dependência leve).

---

### 3.6. `puzzle_kit`

**Responsabilidade:** primitivas reusáveis pra puzzles. Não implementa puzzles concretos.

**Nós fornecidos:**
- `PuzzleTrigger` — emite signal quando o player entra na área.
- `PuzzleSwitch` — toggle on/off, signal `state_changed`.
- `PuzzleGate` — abre/fecha em resposta a condição.
- `PuzzleSequence` — valida ordem de ativação de N switches.
- `PuzzleValidator` — node genérico com função `validate() -> bool` exportada como Callable.

**Dependências:** `event_bus`.

**Standalone:** sim.

---

## 4. Diagrama de dependências

```
                          +-----------------+
                          |    event_bus    |  (raiz; sem deps)
                          +--------+--------+
                                   |
        +--------------+-----------+-----------+--------------+--------------+
        |              |                       |              |              |
        v              v                       v              v              v
 +-------------+ +-----------+         +---------------+ +-----------+ +-----------+
 |localization | |audio_dir. |         | scene_router  | |save_system| | party     |
 +------+------+ +-----+-----+         +-------+-------+ +-----+-----+ +-----+-----+
        |              |                       |              |              |
        |              | (duck on dialogue)    |              |              |
        |              |                       |              |              |
        v              |                       v              v              v
 +-------------+       |              +-----------------+                     |
 |  dialogue   |<------+              | (qualquer cena) |                     |
 +------+------+                      +-----------------+                     |
        |                                                                    |
        |                                                                    |
        |    +-------------+                                                  |
        |    | card_engine |                                                  |
        |    +------+------+                                                  |
        |           |                                                        |
        |           v                                                        |
        |    +-------------+                                                  |
        +--->|turn_combat  |<-------------------------------------------------+
             +------+------+
                    |
                    | (combate consome dialogue/cards/party)
                    v
             +-----------------+
             |  game-specific  |
             +-----------------+

 +----------------+   +----------------+
 |orbital_camera  |   |  puzzle_kit    |   (independentes; só event_bus opt.)
 +----------------+   +----------------+

 +----------------+
 |  input_remap   |   (independente)
 +----------------+
```

**Regras:**
- `event_bus` não depende de ninguém. Tudo pode depender dele.
- Sentido único: gameplay depende de core, **nunca o inverso**.
- `localization` só depende de TranslationServer nativo (acima do diagrama em prioridade de boot).
- `dialogue` é a única dep transversal (precisa de localization). Aceito — diálogo sem L10n é raro.
- `turn_combat` é o módulo mais "acoplado" (combina party + cards + event_bus). Tolerável pela natureza do feature.

---

## 5. Candidatos a publicação standalone (ordem)

1. **`orbital_camera`** — demanda alta na comunidade, escopo claro, zero dependência.
2. **`card_engine`** — nicho, mas underserved. Útil pra qualquer deckbuilder.
3. **`dialogue`** — competição maior (Dialogue Manager, Dialogic), mas se a sintaxe ficar elegante, vale.
4. **`save_system`** — útil generalizado; framework de migração é diferenciador.
5. **`puzzle_kit`** — primitivas simples; publicar só se virar maduro.

Os outros (`event_bus`, `localization`, `audio_director`, `input_remap`, `party`, `turn_combat`, `scene_router`) ficam internos da engine por enquanto. Publicar custa documentação + suporte; G1 não tem orçamento pra isso.

# engine/event_bus

> **Status:** F2-E.2 canon. Primeiro módulo engine.
>
> **Autoload registrado como:** `EventBus` em `game/project.godot`.

Bus global de signals desacoplados pra comunicação cross-module. Limite ~20 signals (anti god-object).

## Uso

```gdscript
# Emit (em qualquer script):
EventBus.game_paused.emit()
EventBus.player_hp_changed.emit(80, 100)

# Listen (em qualquer script):
EventBus.combat_started.connect(_on_combat_started)

func _on_combat_started(combatants: Array) -> void:
    print("Combat iniciado com %d combatentes" % combatants.size())
```

## Signals canônicos (17 total, dentro do budget 20)

### Game state (5)
- `game_started`
- `game_paused`
- `game_resumed`
- `game_saved(slot: int)`
- `game_loaded(slot: int)`

### Player (4)
- `player_moved(from_position: Vector3, to_position: Vector3)`
- `player_interacted(target_name: String)`
- `player_hp_changed(current: int, maximum: int)`
- `player_died`

### Combat turn-based (5)
- `combat_started(combatants: Array)`
- `combat_ended(winner: String)`
- `turn_started(combatant_name: String)`
- `turn_ended(combatant_name: String)`
- `action_resolved(actor: String, target: String, action_type: String, value: int)`

### UI (4)
- `dialogue_shown(dialogue_id: String)`
- `dialogue_choice_made(dialogue_id: String, choice_index: int)`
- `menu_opened(menu_name: String)`
- `menu_closed(menu_name: String)`

## Anti-patterns

- ❌ Encadear signals (signal_a → signal_b automático) sem documentar.
- ❌ Adicionar signal sem docstring + parâmetros tipados.
- ❌ Crescer além de 20 signals sem dividir em buses domain-specific.
- ❌ Listen no `_ready` sem `disconnect` em `_exit_tree` (memory leak).

## Debug

Em debug builds (Editor + dev exports), `EventBus.get_signal_stats()` retorna Dictionary com contagem de emissões. Reset via `EventBus.reset_signal_stats()`.

## Testes

Suite GUT em `test_event_bus.gd` (placeholder até F2-CI.X instalar GUT addon).

## Próximos passos

- Quando estourar 20 signals, dividir: `EventBus` core + `UIEventBus` + `CombatEventBus`.
- Avaliar middleware (logging, replay) só se debugging exigir.

Ver `CONTRACT.md` §1 + `docs/tech/engine-modules.md` §3.

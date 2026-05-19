## event_bus.gd
##
## AutoLoad global de signals desacoplados para comunicação cross-module.
##
## Princípios:
## - Limite ~20 signals (anti god-object). Quando estourar, dividir em buses por domínio.
## - Cada signal MUST ter docstring explicando contexto + parâmetros.
## - Emit em código game/* e listen em engine/* (ou vice-versa) sem dependência direta.
## - NUNCA encadeie signals (signal_a emite signal_b automaticamente). Mantenha plano.
##
## Registrar em Project Settings > AutoLoad como "EventBus".
##
## Uso típico:
##   # Em qualquer script:
##   EventBus.game_paused.emit()
##   EventBus.player_hp_changed.connect(_on_player_hp_changed)
##
## Cross-ref: docs/tech/engine-modules.md §3 + CONTRACT.md §1 (princípios).

extends Node


# =========================================================================
# §1. Game state lifecycle
# =========================================================================

## Emitido quando jogo inicia (após splash + main menu, antes da primeira cena).
signal game_started

## Emitido quando jogo é pausado (player abriu menu in-game).
signal game_paused

## Emitido quando jogo retoma de pause.
signal game_resumed

## Emitido após save bem-sucedido em slot. Parâmetro: número do slot (0..N).
signal game_saved(slot: int)

## Emitido após load bem-sucedido. Parâmetro: número do slot carregado.
signal game_loaded(slot: int)


# =========================================================================
# §2. Player events
# =========================================================================

## Emitido a cada movimento de locomoção concluído. Parâmetros: posição antiga + nova.
signal player_moved(from_position: Vector3, to_position: Vector3)

## Emitido quando player interage com NPC, objeto ou trigger. Parâmetro: nome do alvo.
signal player_interacted(target_name: String)

## Emitido quando HP do player muda. Parâmetros: novo HP atual + HP máximo.
signal player_hp_changed(current: int, maximum: int)

## Emitido quando player morre. Game over flow começa aqui.
signal player_died


# =========================================================================
# §3. Combat events (turn-based)
# =========================================================================

## Emitido no início de uma cena de combate. Parâmetro: lista de combatentes (path nodes).
signal combat_started(combatants: Array)

## Emitido no fim de cena de combate. Parâmetro: vencedor ("player", "enemy", "draw").
signal combat_ended(winner: String)

## Emitido quando turn de um combatente começa. Parâmetro: nome do combatente.
signal turn_started(combatant_name: String)

## Emitido quando turn termina. Parâmetro: nome do combatente.
signal turn_ended(combatant_name: String)

## Emitido após ação ser resolvida (card jogado, dano aplicado, status mudado).
## Parâmetros: ator, alvo, tipo de ação, valor (dano/heal/etc).
signal action_resolved(actor: String, target: String, action_type: String, value: int)


# =========================================================================
# §4. UI events
# =========================================================================

## Emitido quando dialogue UI abre. Parâmetro: id do dialogue node (Ink/DialogueManager).
signal dialogue_shown(dialogue_id: String)

## Emitido quando jogador escolhe opção em branching dialogue. Parâmetros: id + escolha.
signal dialogue_choice_made(dialogue_id: String, choice_index: int)

## Emitido quando menu (settings, save, pause, inventory) abre. Parâmetro: nome do menu.
signal menu_opened(menu_name: String)

## Emitido quando menu fecha. Parâmetro: nome do menu.
signal menu_closed(menu_name: String)


# =========================================================================
# §5. Debug helpers (DEV ONLY, remover em release builds via #ifdef futuro)
# =========================================================================

## Conta sinais emitidos por nome desde início da sessão (debug only).
var _signal_counters: Dictionary = {}


func _ready() -> void:
	# Em dev builds, log automático ao conectar todos signals deste node
	# pra contagem de emissões. Não bloqueia perf significativamente.
	if OS.is_debug_build():
		_setup_debug_counters()


func _setup_debug_counters() -> void:
	for signal_data in get_signal_list():
		var signal_name: StringName = signal_data["name"]
		# Não conta sinais herdados de Node (script_changed, etc).
		if signal_name in ["script_changed", "property_list_changed"]:
			continue
		_signal_counters[signal_name] = 0
		var callable := Callable(self, "_count_signal").bind(signal_name)
		connect(signal_name, callable)


func _count_signal(_arg0 = null, _arg1 = null, _arg2 = null, _arg3 = null, signal_name: StringName = "") -> void:
	# Bind funciona como último arg em Godot 4. Conta + opcionalmente loga em debug verboso.
	if signal_name in _signal_counters:
		_signal_counters[signal_name] += 1


## Retorna snapshot do contador (debug). Uso: print(EventBus.get_signal_stats()).
func get_signal_stats() -> Dictionary:
	return _signal_counters.duplicate()


## Reseta contadores (útil entre cenas de teste).
func reset_signal_stats() -> void:
	for key in _signal_counters.keys():
		_signal_counters[key] = 0

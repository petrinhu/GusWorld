## test_event_bus.gd
##
## Suite GUT placeholder pro módulo event_bus.
##
## Pré-requisito: GUT addon instalado em game/addons/gut/.
## Rodar via: godot --headless --path ./game -d -s addons/gut/gut_cmdln.gd -gtest=res://engine/event_bus/test_event_bus.gd
##
## Cobertura alvo: emit + connect + signal stats counter + reset.

extends "res://addons/gut/test.gd"


var _received_payload: Variant = null


func before_each() -> void:
	_received_payload = null
	# Reset stats antes de cada teste pra isolamento.
	if Engine.has_singleton("EventBus"):
		EventBus.reset_signal_stats()


func test_game_started_emit_no_params() -> void:
	if not Engine.has_singleton("EventBus"):
		pending("EventBus AutoLoad não registrado, skip")
		return
	var captured := [false]
	var callable := func() -> void:
		captured[0] = true
	EventBus.game_started.connect(callable)
	EventBus.game_started.emit()
	assert_true(captured[0], "game_started deve ter sido capturado")
	EventBus.game_started.disconnect(callable)


func test_player_hp_changed_emit_with_params() -> void:
	if not Engine.has_singleton("EventBus"):
		pending("EventBus AutoLoad não registrado, skip")
		return
	var captured := {"current": -1, "maximum": -1}
	var callable := func(current: int, maximum: int) -> void:
		captured["current"] = current
		captured["maximum"] = maximum
	EventBus.player_hp_changed.connect(callable)
	EventBus.player_hp_changed.emit(80, 100)
	assert_eq(captured["current"], 80)
	assert_eq(captured["maximum"], 100)
	EventBus.player_hp_changed.disconnect(callable)


func test_signal_stats_counter() -> void:
	if not Engine.has_singleton("EventBus"):
		pending("EventBus AutoLoad não registrado, skip")
		return
	if not OS.is_debug_build():
		pending("Counters só em debug builds, skip")
		return
	EventBus.game_paused.emit()
	EventBus.game_paused.emit()
	EventBus.game_paused.emit()
	var stats := EventBus.get_signal_stats()
	assert_eq(stats.get("game_paused", 0), 3, "game_paused deve ter sido contado 3 vezes")

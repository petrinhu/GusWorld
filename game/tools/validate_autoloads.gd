## validate_autoloads.gd
##
## Script standalone pra validar AutoLoads no boot.
##
## Uso:
##   cd game/
##   godot --headless -s tools/validate_autoloads.gd
##
## Testa:
## - EventBus existe + tem signals esperados
## - Localization existe + carrega pt_br.md
## - MDTranslationLoader parse correto
##
## Exit code 0 = sucesso, 1 = falha.

extends SceneTree


func _initialize() -> void:
	# Em SceneTree -s scripts, _initialize roda antes de AutoLoads _ready.
	# Force carregamento explícito antes de validar.
	var loc_early := root.get_node_or_null("Localization")
	if loc_early and loc_early.has_method("load_all_locales"):
		loc_early.load_all_locales()
	print("=== Validação AutoLoads + i18n ===")
	var errors: int = 0

	# Test 1: EventBus AutoLoad existe
	var event_bus_node := root.get_node_or_null("EventBus")
	if event_bus_node == null:
		printerr("FAIL: EventBus AutoLoad não registrado")
		errors += 1
	else:
		print("OK: EventBus AutoLoad presente")
		# Verifica alguns signals canon
		var expected_signals := ["game_started", "player_hp_changed", "combat_started", "dialogue_shown"]
		for sig in expected_signals:
			if not event_bus_node.has_signal(sig):
				printerr("FAIL: EventBus signal '%s' não encontrado" % sig)
				errors += 1
		if errors == 0:
			print("OK: 4 signals canon verificados")

	# Test 2: Localization AutoLoad existe + carrega pt_br
	var loc_node := root.get_node_or_null("Localization")
	if loc_node == null:
		printerr("FAIL: Localization AutoLoad não registrado")
		errors += 1
	else:
		print("OK: Localization AutoLoad presente")
		var locales: Array = loc_node.get_available_locales()
		print("    Locales carregados: %s" % str(locales))
		if not "pt_br" in locales:
			printerr("FAIL: pt_br não carregado")
			errors += 1
		if not "en_intl" in locales:
			printerr("FAIL: en_intl não carregado")
			errors += 1
		# Test 3: tradução pt_br funciona
		var test_value: String = loc_node.tr_md("MENU_START_GAME")
		if test_value == "Iniciar jogo":
			print("OK: tr_md MENU_START_GAME → '%s'" % test_value)
		else:
			printerr("FAIL: tr_md MENU_START_GAME esperava 'Iniciar jogo' mas obteve '%s'" % test_value)
			errors += 1
		# Test 4: chave inexistente retorna literal
		var missing: String = loc_node.tr_md("CHAVE_QUE_NAO_EXISTE_XYZ")
		if missing == "CHAVE_QUE_NAO_EXISTE_XYZ":
			print("OK: fallback chave inexistente retorna literal")
		else:
			printerr("FAIL: fallback inesperado: '%s'" % missing)
			errors += 1
		# Test 5: interpolação {0}
		var interpolated: String = loc_node.tr_md("SAVE_SLOT_LABEL", ["3"])
		if interpolated == "Slot 3":
			print("OK: interpolação {0} → '%s'" % interpolated)
		else:
			printerr("FAIL: interpolação esperava 'Slot 3' obteve '%s'" % interpolated)
			errors += 1

	print("=== Resultado: %d erro(s) ===" % errors)
	quit(0 if errors == 0 else 1)

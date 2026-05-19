## localization.gd
##
## AutoLoad wrapper de i18n com formato MD custom (não-CSV).
##
## Registrar em Project Settings > AutoLoad como "Localization".
##
## Uso:
##   Localization.tr_md("MENU_START_GAME")        # → "Iniciar jogo" (pt_br atual)
##   Localization.set_locale("en_intl")
##   Localization.tr_md("MENU_START_GAME")        # → "Start game"
##
## Princípios:
## - Locale default: pt_br (canon, dev sem fallback).
## - Fallback chain: pt_br → en_intl → key literal.
## - Recarrega arquivos MD em runtime (debug builds) via `reload()`.
## - NÃO substitui Godot `tr()` nativo; convive com ele se necessidade futura.
##
## Cross-ref: docs/tech/architecture.md §6 + CONTRACT.md §6 (a11y) + project_i18n_canonico memo.

extends Node


const TRANSLATIONS_DIR: String = "res://translations"
const DEFAULT_LOCALE: String = "pt_br"
const FALLBACK_LOCALE: String = "en_intl"

## Locales suportados (hardcoded porque Godot DirAccess não vê .md não-importados em res://).
## Adicionar novo locale: criar arquivo + adicionar string aqui.
const SUPPORTED_LOCALES: Array[String] = ["pt_br", "en_intl"]

var _strings: Dictionary = {}  # {locale: {key: value}}
var _current_locale: String = DEFAULT_LOCALE


func _ready() -> void:
	load_all_locales()
	_current_locale = DEFAULT_LOCALE


## Carrega arquivos MD pra cada locale em SUPPORTED_LOCALES.
##
## Arquivos esperados em TRANSLATIONS_DIR/<locale>.md (ex: res://translations/pt_br.md).
func load_all_locales() -> void:
	_strings.clear()
	for locale in SUPPORTED_LOCALES:
		var full_path := "%s/%s.md" % [TRANSLATIONS_DIR, locale]
		if FileAccess.file_exists(full_path):
			_strings[locale] = MDTranslationLoader.load_from_file(full_path)
		else:
			push_warning("Localization: arquivo locale não encontrado: %s" % full_path)


## Recarrega arquivos MD (útil em dev/playtest pra iterar sem reiniciar jogo).
func reload() -> void:
	load_all_locales()


## Define locale corrente. Se locale não estiver carregado, faz nothing + warning.
func set_locale(locale: String) -> void:
	if not _strings.has(locale):
		push_warning("Localization: locale '%s' não carregado, mantendo '%s'" % [locale, _current_locale])
		return
	_current_locale = locale


## Retorna locale corrente.
func get_locale() -> String:
	return _current_locale


## Retorna lista de locales disponíveis (arquivos MD encontrados).
func get_available_locales() -> Array:
	return _strings.keys()


## Traduz chave usando locale corrente + fallback chain.
##
## Ordem de busca:
##   1. _strings[current_locale][key]
##   2. _strings[FALLBACK_LOCALE][key]
##   3. _strings[DEFAULT_LOCALE][key]
##   4. key literal (string passada)
##
## Args opcionais pra interpolação estilo {0} {1}:
##   Localization.tr_md("DAMAGE_DEALT", [str(dmg), target_name])
##   # arquivo: "Causou {0} de dano em {1}."
func tr_md(key: String, args: Array = []) -> String:
	var value: String = ""

	# Locale corrente
	if _strings.has(_current_locale) and _strings[_current_locale].has(key):
		value = _strings[_current_locale][key]
	# Fallback en_intl
	elif _strings.has(FALLBACK_LOCALE) and _strings[FALLBACK_LOCALE].has(key):
		value = _strings[FALLBACK_LOCALE][key]
	# Fallback pt_br
	elif _strings.has(DEFAULT_LOCALE) and _strings[DEFAULT_LOCALE].has(key):
		value = _strings[DEFAULT_LOCALE][key]
	else:
		# Key faltando: retorna literal + warning em debug
		if OS.is_debug_build():
			push_warning("Localization: chave '%s' não encontrada em nenhum locale" % key)
		return key

	# Interpolação simples {0}, {1}, etc
	if not args.is_empty():
		for i in range(args.size()):
			value = value.replace("{%d}" % i, str(args[i]))

	return value


## Checa se key existe no locale atual (sem fallback).
func has_key(key: String) -> bool:
	return _strings.has(_current_locale) and _strings[_current_locale].has(key)


## Lista todas chaves do locale atual (debug).
func list_keys() -> Array:
	if _strings.has(_current_locale):
		return _strings[_current_locale].keys()
	return []

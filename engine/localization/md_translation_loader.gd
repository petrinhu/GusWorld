## md_translation_loader.gd
##
## Custom MD parser pra arquivos de tradução em formato Markdown.
##
## Formato esperado:
##
##     ## NOME_DA_CHAVE
##     Valor da chave aqui. Pode ter múltiplas linhas
##     até a próxima chave (## ).
##
##     ## OUTRA_CHAVE
##     Outro valor.
##
## Linhas em branco entre keys são ignoradas.
## Linhas começando com `#` que NÃO sejam `## ` (ex: `# Título`, `### Sub`) também são ignoradas.
##
## Não usa Godot `tr()` nativo (que espera CSV/PO). Em vez disso, expõe método `lookup(key)`.
## Localization AutoLoad usa este loader internamente.
##
## Cross-ref: game/translations/README.md (formato + workflow).

class_name MDTranslationLoader
extends RefCounted


## Parsea arquivo MD e retorna Dictionary {chave: valor}.
##
## Path absoluto Godot (ex: "res://game/translations/pt_br.md").
## Retorna Dictionary vazio se arquivo não existe (com push_warning).
static func load_from_file(path: String) -> Dictionary:
	if not FileAccess.file_exists(path):
		push_warning("MDTranslationLoader: arquivo não encontrado: %s" % path)
		return {}

	var file := FileAccess.open(path, FileAccess.READ)
	if file == null:
		push_error("MDTranslationLoader: falha ao abrir %s (erro %d)" % [path, FileAccess.get_open_error()])
		return {}

	var content := file.get_as_text()
	file.close()
	return parse(content)


## Parsea string MD e retorna Dictionary.
##
## Convenção: chave inicia com `## ` (exatamente, sem espaço extra).
## Valor coleta até próxima `## ` ou EOF. Trim de whitespace nas pontas.
static func parse(content: String) -> Dictionary:
	var result: Dictionary = {}
	var current_key: String = ""
	var current_value_lines: Array[String] = []

	for raw_line in content.split("\n"):
		var line: String = raw_line

		if line.begins_with("## "):
			# Salva entry anterior se existir.
			if current_key != "":
				result[current_key] = _join_and_trim(current_value_lines)

			# Inicia nova entry. Strip "## " prefix.
			current_key = line.substr(3).strip_edges()
			current_value_lines = []
		elif current_key != "":
			# Acumula linhas de valor.
			# Filtra linhas que são headers MD não-chave (# X, ### X, etc).
			if line.begins_with("#") and not line.begins_with("## "):
				continue
			current_value_lines.append(line)

	# Não esquecer última entry.
	if current_key != "":
		result[current_key] = _join_and_trim(current_value_lines)

	return result


## Junta linhas de valor + trim de whitespace nas pontas.
static func _join_and_trim(lines: Array[String]) -> String:
	if lines.is_empty():
		return ""
	return "\n".join(lines).strip_edges()

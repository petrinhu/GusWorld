# game/translations, formato + workflow editorial canon

> **Status:** Canônico. F2-S.11 setup.
>
> **Locales:** `pt_br` (dev primário, sempre completo), `en_intl` (alvo pós-v1.0.0, vazio durante G1), `es_la` (alvo v1.x+, ainda não existe).
>
> **Loader:** `engine/localization/md_translation_loader.gd` (custom MD parser).

---

## Formato canon

```markdown
## CHAVE_NOME_UPPER_SNAKE
Valor da chave. Pode ter
múltiplas linhas até a próxima ## .

## OUTRA_CHAVE
Outro valor.

## DIALOGUE_GUS_INTRO_001
Bloco de diálogo longo.
Várias linhas preservadas.
Quebras de linha respeitadas.
```

### Regras

1. **Chave inicia com `## ` exato** (dois `#` + espaço + nome). Sem espaço extra.
2. **Chave em UPPER_SNAKE_CASE** sempre. Validar via `gdlint` futuro.
3. **Valor coleta até próxima `## `** ou EOF. Trim de whitespace nas pontas.
4. **Linhas com 1 `#` ou 3+ `###` são ignoradas** (servem como organização visual do arquivo).
5. **Linhas em branco entre chaves são ignoradas.**
6. **Interpolação:** placeholders `{0}`, `{1}`, ..., resolvidos via `Localization.tr_md(key, args)`.

---

## Organização por seções (convenção)

Cada arquivo locale tem seções `## §N. Título` (1 `#` = comentário organizacional, não chave).

Seções canon:

| §N | Domínio | Exemplos |
|---|---|---|
| §1 | Menu principal | `MENU_START_GAME`, `MENU_QUIT` |
| §2 | Settings | `SETTINGS_VIDEO`, `SETTINGS_AUDIO` |
| §3 | Acessibilidade | `A11Y_REMAP_CONTROLS`, `A11Y_REDUCE_MOTION` |
| §4 | Save/Load | `SAVE_SUCCESS`, `LOAD_FAILED` |
| §5 | HUD + combate | `HUD_HP_LABEL`, `COMBAT_VICTORY` |
| §6 | Dialogue | `DIALOGUE_GUS_INTRO_001` (id-based por arco) |
| §7 | Errors + system | `ERROR_GENERIC`, `INFO_AUTOSAVE` |

Expandir seções conforme arcos: `§8 Items`, `§9 Quests`, `§10 In-world docs`, etc.

---

## Naming convention

| Prefix | Domínio | Exemplo |
|---|---|---|
| `MENU_` | Texto de menu principal | `MENU_START_GAME` |
| `SETTINGS_` | Settings UI | `SETTINGS_VIDEO` |
| `A11Y_` | Acessibilidade | `A11Y_REMAP_KEYBOARD` |
| `SAVE_` | Save system | `SAVE_SUCCESS` |
| `LOAD_` | Load system | `LOAD_FAILED` |
| `HUD_` | HUD elements | `HUD_HP_LABEL` |
| `COMBAT_` | Combate turn-based | `COMBAT_VICTORY` |
| `DIALOGUE_<ARC>_<ID>` | Dialogue por arco | `DIALOGUE_GUS_INTRO_001` |
| `ERROR_` | Mensagens de erro | `ERROR_FILE_NOT_FOUND` |
| `INFO_` | Mensagens informativas | `INFO_AUTOSAVE` |
| `ITEM_<NAME>` | Items inventário (futuro) | `ITEM_TAVUS_DRIVE` |
| `QUEST_<NAME>_<KEY>` | Quests | `QUEST_PELICANO_TITLE` |
| `LORE_<DOC>_<KEY>` | In-world docs lore | `LORE_PACTO_SENSORIAL_PARAGRAPH_03` |

---

## Workflow editorial

### Adicionar nova chave (dev pt-br)

1. Editar `pt_br.md`, adicionar `## NOVA_CHAVE` + valor em seção apropriada.
2. **MUST** adicionar mesma chave em `en_intl.md` com valor **vazio** (paridade estrutural).
3. Validar parsing: `Localization.reload()` em editor + verificar `Localization.has_key("NOVA_CHAVE")`.
4. Usar via `Localization.tr_md("NOVA_CHAVE")` no código.

### Tradução en_intl pós-v1.0.0

1. Sweep completo de `en_intl.md`, preencher valores.
2. Validar paridade: toda chave em pt_br MUST existir em en_intl com valor não-vazio.
3. Audit cultural: gírias regionais, expressões idiomáticas, ajustes Brasil → Internacional.
4. Test in-game: trocar locale via debug menu, ver overflow de texto em UI.

### Nova locale (es_la, fr_intl, jp, etc, futuro)

1. Criar `<locale>.md` copiando estrutura de `en_intl.md` (vazia).
2. Traduzir progressivamente.
3. Registrar em `engine/localization/localization.gd` se locale precisar tratamento especial.
4. Adicionar em settings menu de idiomas (`SETTINGS_LANGUAGE` opcoes).

---

## Anti-patterns proibidos

- ❌ Strings hardcoded em código GDScript (sempre via `Localization.tr_md`).
- ❌ Adicionar key em pt_br sem placeholder em en_intl.
- ❌ Chaves duplicadas (parser usa última, comportamento silencioso).
- ❌ Chaves em camelCase / snake_case / kebab-case (sempre UPPER_SNAKE_CASE).
- ❌ Interpolação via `%` ou f-string. Usar `{0}, {1}` + args array.
- ❌ Lore textos longos hardcoded em arquivos `.gd`. Mover pra `LORE_*` keys.

---

## Validação

### Manual (G1)

- `Localization.list_keys()` no console retorna chaves do locale atual.
- `Localization.has_key("X")` retorna true/false.
- `Localization.tr_md("CHAVE_INEXISTENTE")` retorna a string literal + warning em debug.

### Automatizada (futuro F2-CI.X)

- Job CI valida paridade estrutural (toda chave pt_br MUST existir em en_intl).
- Lint key naming convention (UPPER_SNAKE_CASE).
- Detecta chaves órfãs (key existe mas nunca usada em código via grep).

---

## Cross-refs

- `engine/localization/README.md` (API do loader)
- `engine/localization/md_translation_loader.gd` (parser implementação)
- `engine/localization/localization.gd` (AutoLoad wrapper)
- `CONTRACT.md` §4 (DoD: strings user-facing via tr_md)
- `CONTRACT.md` §6 (acessibilidade D1, subtitles + closed captions)
- Memo: `project_i18n_canonico` (decisão dev pt-br + en-intl pós-v1.0.0)

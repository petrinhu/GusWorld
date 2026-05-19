# engine/localization

> **Status:** F2-S.11 canon. Sistema de i18n custom em formato MD.
>
> **Autoload registrado como:** `Localization` em `game/project.godot`.

Sistema de tradução baseado em **arquivos Markdown por locale** (não-CSV canônico Godot, decisão criador supremo). Custom loader parseia `## CHAVE` + valor.

## Uso

```gdscript
# Em qualquer script:
$Label.text = Localization.tr_md("MENU_START_GAME")
# → "Iniciar jogo" (locale pt_br atual)

# Com interpolação:
var msg = Localization.tr_md("DAMAGE_DEALT", [str(damage), target_name])
# → "Causou 42 de dano em Sterling."

# Trocar locale (pós-v1.0.0 quando en_intl existir):
Localization.set_locale("en_intl")

# Reload em dev pra iterar narrativa sem reiniciar:
Localization.reload()
```

## Arquivos

```
game/translations/
├── pt_br.md     (dev primário, sempre completo)
├── en_intl.md   (alvo pós-v1.0.0; vazio até lá)
├── es_la.md     (alvo futuro v1.x; ainda não existe)
└── README.md    (convenção + workflow editorial)
```

## Fallback chain

1. Locale corrente (ex: `en_intl`)
2. `en_intl` (fallback internacional canon)
3. `pt_br` (default dev, sempre completo)
4. Literal da key (último recurso, log warning em debug)

## Formato MD canon

```markdown
## CHAVE_UPPER_SNAKE_CASE
Valor da chave. Pode ter
múltiplas linhas até a próxima ## chave.

## OUTRA_CHAVE
Outro valor.

## DIALOGUE_GUS_INTRO_001
Meus óculos detectaram alguma coisa estranha.
Os números não fecham. Sterling está mentindo.
```

Detalhes em `game/translations/README.md`.

## Anti-patterns proibidos

- ❌ Strings hardcoded user-facing (sempre via `Localization.tr_md`).
- ❌ Adicionar key em pt_br sem placeholder vazio em en_intl (mantém paridade estrutural).
- ❌ Chaves duplicadas no mesmo arquivo (parser usa última).
- ❌ Chaves em camelCase ou snake_case. **MUST** UPPER_SNAKE_CASE.
- ❌ Interpolação manual via `%`. Usar `{0}, {1}` + args.

## Testes

Suite GUT em `test_localization.gd` (placeholder, criar quando GUT addon instalado).

Cross-ref: `CONTRACT.md` §4 (DoD strings via tr_md), memo `project_i18n_canonico`.

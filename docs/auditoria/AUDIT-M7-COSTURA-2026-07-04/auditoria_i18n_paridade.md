# Auditoria: Paridade i18n (chaves novas - menu + flavor da derrota)

- Subsistemas: `game/translations/{pt_br,en_intl}.md`, `tools/i18n_parity.py`, loader C++ (`domain/i18n`), `app/i18n/translator`
- Criterio: AUD-I18N (paridade estrutural; zero string hardcoded; interpolacao de placeholders de fato aplicada)

## Contexto e metodo

O M7-COSTURA adicionou chaves i18n em duas frentes: o menu de sistema (§9 do catalogo: `MENU_SYSTEM_KICKER`, `MENU_PAUSE_TITLE`, `MENU_PAUSE_HINT`, `SETTINGS_MUSIC_VOLUME`, `SETTINGS_SFX_VOLUME`, `MENU_PLACEHOLDER_TEXT`) e o flavor da derrota (`COMBAT_DEFEAT_BARK`, `COMBAT_DEFEAT_BARK_GENERIC`, `COMBAT_DEFEAT_CHESS_NOTE`). Regra canon (ADR-012 §7, memoria `project_i18n_canonico`): zero string user-facing hardcoded, tudo via `tr()`; `en_intl` pode ter VALORES vazios ate pos-v1.0.0 (a paridade ESTRUTURAL e obrigatoria, o % de conteudo nao reprova). Metodo: `tools/i18n_parity.py` (espelha o loader C++) + grep das chaves nos call-sites + verificacao de presenca estrutural nas duas frentes.

## Achados

| ID | Sev | Descricao | Evidencia | Estado |
|---|---|---|---|---|
| I18N-1 | (OK) | Paridade ESTRUTURAL intacta: 132 chaves na fonte pt_br, 0 faltando / 0 extra / 0 duplicata no en_intl | `python3 tools/i18n_parity.py` -> `en_intl 132 7 5.3% 0 0` (GATE exit 0; o mesmo do `tools/check.sh`) | ✓ |
| I18N-2 | (OK) | As 9 chaves novas (6 do menu §9 + 3 do defeat flavor) estao presentes ESTRUTURALMENTE nas DUAS locales (pt_br com conteudo, en_intl vazio por canon) | grep `^## <chave>$` -> pt_br=1 en_intl=1 para todas as 9 | ✓ |
| I18N-3 | (OK) | Zero string user-facing hardcoded no menu: todo texto sai de `tr.tr(KEY)` (kicker/titulo/pills/placeholder/sliders) | `grep -oE '"[A-Z][A-Z0-9_]{3,}"' system_menu_rml.cpp` -> so chaves i18n conhecidas (MENU_*/SETTINGS_*); nenhum literal de UI | ✓ |
| I18N-4 | (OK) | O defeat flavor sai por `tr()` (bark + bark_generic + chess_note), com `{0}` interpolado a mao pro nome do companion | `battle_scene.cpp:2070-2084`; teste #1038 (nome interpolado), #1039 (GENERIC sem nome) | ✓ |
| **ACH-2** | **🟠 IMPORTANTE** | **`MENU_PAUSE_HINT` em pt_br = `"{0} confirma, {1} volta ao jogo"`, mas `system_menu_rml.cpp:399` renderiza `tr.tr("MENU_PAUSE_HINT")` SEM substituir os placeholders.** O `Translator` de app/ so tem `tr(key)` (sem overload de interpolacao - `translator.hpp:45`); o loader do dominio deixa `{0}/{1}` literais de proposito (`md_translation_loader.hpp:43`, "interpolacao e responsabilidade de quem exibe"), e quem precisa faz `find/replace` a mao (a `battle_scene.cpp:2071` faz pro bark, o menu NUNCA foi ligado). **Resultado: o jogador ve o texto literal `{0} confirma, {1} volta ao jogo` no rodape da tela de pausa.** O teste `system_menu_rml_test.cpp:37` nao pega porque o fixture usa `"Enter confirma, ESC volta"` (sem `{0}/{1}`) | `grep -A1 '^## MENU_PAUSE_HINT$' pt_br.md` -> `{0} confirma, {1} volta ao jogo`; `system_menu_rml.cpp:399` (sem `.find/.replace` - `grep '{0}\|.replace' system_menu_rml.cpp` -> VAZIO); `system_menu_rml_test.cpp:37` (fixture sem placeholder) | - |

## Severidade de ACH-2 (justificativa)

Classificado 🟠 IMPORTANTE, nao 🟢 COSMETICO: e um defeito user-facing VISIVEL (tokens de template crus na tela) num feature marcado como done e aprovado ao vivo - trata-lo como mero estilo seria minimizar. Nao e 🔴 porque nao crasha, nao corrompe estado, e o sentido da dica ainda e legivel ("confirma", "volta ao jogo") e a navegacao (Enter/Esc/Continuar) funciona 100%. O lider aprovou "sem bugs perceptiveis" provavelmente sem escrutinar o rodape.

## Remediacao proposta (ACH-2)

Duas opcoes de baixo custo (decisao do lider/da thread principal, ver secao 8 do indice mestre):
1. **Interpolar** os rotulos das teclas confirmar/voltar na dica (mesmo `find/replace` do bark, OU dar ao `Translator` um overload `tr(key, args...)` reusavel - endereca a raiz e evita o padrao espalhado).
2. **Remover** os `{0}/{1}` da string `MENU_PAUSE_HINT` (e do en_intl correspondente quando preenchido) se a dica nao precisa dos rotulos dinamicos.
Em ambos os casos: endurecer o fixture do `system_menu_rml_test` para usar uma dica COM `{0}/{1}` e assertar que os placeholders SOMEM do render (regressao). Ao fechar + re-teste, o Estado Auditado do item sobe de `⚠` para `✓`.

## Conclusao

Paridade estrutural intacta (0 faltando/0 extra), zero string hardcoded, defeat flavor i18n correto com interpolacao. O unico furo e ACH-2 (🟠): a dica do menu de pausa exibe `{0}/{1}` crus por falta de interpolacao no call-site. Fix trivial, com regressao de teste sugerida.

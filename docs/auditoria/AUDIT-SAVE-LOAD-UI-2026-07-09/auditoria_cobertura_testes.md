# Auditoria: Cobertura de testes / TDD (SAVE-LOAD-UI)

- Subsistemas: `domain/tests/save_policy_test.cpp`, `platform/tests/
  save_file_store_test.cpp`, `app/tests/{save_load_menu,save_load_menu_rml,
  save_load_menu_interaction,title_menu,title_menu_rml,maestro_logic}_test.cpp`.
- Critério: AUD-COV (cobertura medida dos comportamentos novos, casos de borda,
  paths críticos) + `TESTES.md` (TDD red/green/refactor).

## Contexto e método

Leitura de todos os `TEST_CASE` dos 8 arquivos de teste tocados por esta feature, cruzada
com os comportamentos que o brief pediu para verificar explicitamente (delete apaga a
cadeia INTEIRA de backup; autosave respeita a política; gate Vitória; empate de
timestamp no `most_recent_occupied_slot`; limiares do fallback de capítulo; o harness
realmente exercita o clique). Reprodução real da execução (não só leitura) via os
binários já compilados (`gusengine_domain_tests`, `gusengine_platform_tests`,
`gusengine_app_tests`), inclusive sob ASan/UBSan e sob Xvfb :99 (GL real).

## Achados — comportamentos com cobertura confirmada (execução real)

| ID | Sev | Descrição | Evidência | Estado |
|---|---|---|---|---|
| COV-1 | (OK) | `delete_save` apaga o PRIMÁRIO + a cadeia INTEIRA de backup (`kBackupChainDepth`=3); idempotente (slot já vazio); não mexe em outros slots; apaga o autosave igual a qualquer manual; slot inválido é fail-fast (`std::out_of_range`) | `platform/tests/save_file_store_test.cpp:260-325`; reexecutado sob ASan isolado por tag `[save]` → 69 assertions / 19 test cases, exit 0 | ✓ |
| COV-2 | (OK) | `autosave_allowed_at` (política por local) testada nos 5 combos booleanos relevantes (cidade sempre ON; dungeon sem overrides OFF; dungeon com só PEM ON; dungeon com só a carta ON; dungeon com os 2 ON) + `static_assert` em tempo de compilação | `domain/tests/save_policy_test.cpp:14-58`; reexecutado sob ASan → dentro das 427 assertions/75 test cases de `[save]` no domínio, exit 0 | ✓ |
| COV-3 | (OK) | Gate "só Vitória autosalva no retorno de batalha" (`should_autosave_after_battle`) testado nos 4 `CombatOutcome` + `static_assert` | `app/tests/maestro_logic_test.cpp:93-105`; reexecutado sob ASan (`[maestro]`) → 143 assertions/39 test cases, exit 0 | ✓ |
| COV-4 | (OK) | `most_recent_occupied_slot`: nenhum ocupado (-1); único ocupado; maior timestamp entre vários; **empate de timestamp resolvido pelo PRIMEIRO índice** (testado explicitamente) | `app/tests/save_load_menu_test.cpp:669-707` (4 `TEST_CASE`, incluindo o de empate na linha 707) | ✓ |
| COV-5 | (OK) | `chapter_from_quest_progress`/`chapter_from_xp_fallback`: vazio+xp=0→Cap.1; `main_story` presente ignora xp; clamp no teto (`kChapterCount`=6); `main_story` negativo (defensivo) clampa em 1; bandas de XP monotonicamente crescentes; xp gigante clampa no teto; xp negativo (defensivo) no piso 1 | `app/tests/save_load_menu_test.cpp:37-98` (8 `TEST_CASE` cobrindo os limiares e casos defensivos) | ✓ |
| COV-6 | (OK) | O harness `save_load_menu_interaction_test.cpp` **de fato** exercita GL real (não é um "sempre pula" disfarçado) e um clique de mouse REAL (`SDL_PushEvent`, mesma fila que o loop de produção consome) dispara o save de verdade em disco | reexecutado ao vivo sob Xvfb :99 nesta auditoria: `[save_load_menu_interaction]` → **29 assertions em 2 test cases** (≠ 0 — confirma que NÃO caiu no ramo de degradação segura "sem display"); o teste 2 verifica `build_called` e `has_save(1,...)` após o clique real | ✓ |
| COV-7 | (OK) | Roteamento de teclado + mouse das 3 confirmações (sobrescrita, exclusão, Novo Jogo) simétrico e completo: ESC/clique-Não/tecla-eixo alternam a pill, Enter/clique confirmam a escolha atual, nunca 2 diálogos simultâneos | leitura de `save_load_menu.cpp:244-317` (roteamento por teclado) + `save_load_menu_loop.cpp:435-512` (mouse); testes dedicados em `save_load_menu_test.cpp` para cada ramo (`click_overwrite_confirm`/`click_delete_confirm`/`request_delete` com guards de "já tem diálogo aberto") | ✓ |

## Achado de lacuna (gera o CRIT-1 do capítulo de integridade)

| ID | Sev | Descrição | Evidência | Estado |
|---|---|---|---|---|
| COV-8 | 🔴 (ver `auditoria_integridade_dados_save.md`) | `save_load_menu_loop.cpp` (o arquivo que combina `build_previews_and_cache` + `confirm_selected_slot` + `do_save`) é **explicitamente não-testado por unidade** — o próprio cabeçalho do arquivo declara "sem unidade de teste direta pro loop em si". Essa é exatamente a combinação onde mora o CRIT-1 (slot com primário corrompido, exibido como vazio, grava sem confirmar): nenhum teste (unitário ou do harness GL) cobre "slot ocupado-mas-corrompido + tentativa de Salvar por cima" — o harness GL só cobre posicionamento/clique básico com dados 100% saudáveis (`make_mixed_slots()`, sempre `LoadResult::Ok`) | `save_load_menu_loop.cpp:5-9` (comentário do cabeçalho); busca por `HmacInvalid`/`Corrupt`/`LoadResult` em `save_load_menu_test.cpp`/`save_load_menu_interaction_test.cpp` → nenhum hit (grep confirma) | ⚠ aberto |

## Conclusão

A cobertura dos comportamentos **já antecipados** pela especificação (backup completo,
gate de política/Vitória, empates, limiares de capítulo, clique real via harness GL) é
sólida e **executada com sucesso nesta auditoria** (não só lida) — inclusive sob
ASan/UBSan. A lacuna real (COV-8) é a combinação "arquivo presente mas corrompido +
interação de Salvar", que fica inteiramente dentro do `.cpp` que o próprio time já
sinalizou como fora da malha de teste unitário — e é exatamente aí que o achado 🔴
CRÍTICO deste dossiê mora (ver `auditoria_integridade_dados_save.md`). Recomenda-se, ao
remediar o CRIT-1, adicionar 1 teste (harness GL ou um teste de integração leve dos 3
arquivos, como o programa de verificação usado nesta auditoria) que trave o cenário
"slot corrompido → clique em modo Save → confirmação DEVE abrir" como regressão
permanente.

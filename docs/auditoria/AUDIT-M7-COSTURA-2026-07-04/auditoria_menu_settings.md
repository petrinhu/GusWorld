# Auditoria: Menu de sistema (arvore de 7 telas) + persistencia settings.json

- Subsistemas: `app/screens/system_menu.{hpp,cpp}` (POCO), `system_menu_rml.{hpp,cpp}` (render), `system_menu_loop.{hpp,cpp}` (loop GL), `domain/settings/*` (JSON POCO), `platform/fs/settings_file_store.*` (I/O)
- Criterio: AUD-ARCH + AUD-QUALITY + AUD-SEC (logica pura testavel; separacao estado/render/IO; degradacao segura de arquivo do jogador; permissao LGPD leve)

## Contexto e metodo

MENU-PAUSA-CONFIG-SOM entrega uma arvore hierarquica de navegacao (7 telas: Pause -> Save/ConfigCategories -> Audio/Video/Language) com sliders de volume, hover/click NATIVOS do glintfx v0.3.1 e persistencia em `settings.json` no padrao do `controls.json` (ADR-007). A separacao e em 3 camadas dentro de app/: ESTADO/DECISAO (`system_menu.hpp`, POCO), RENDER (`system_menu_rml`, le o estado), LOOP GL + INTEGRACAO (`system_menu_loop`, chama AudioEngine + persistencia). Metodo: leitura + reproducao dos testes de menu/settings sob ASan.

## Achados - logica do menu (POCO)

| ID | Sev | Descricao | Evidencia | Estado |
|---|---|---|---|---|
| MEN-1 | (OK) | A arvore (`parent_screen_of`) e um mapa fixo pai->filho; "Voltar"/ESC sobe exatamente 1 nivel preservando a selecao do pai (campo `*_selected` por tela com >1 item) | `system_menu.hpp:52-130`; teste #1135 (`close` de qualquer tela funda) | ✓ |
| MEN-2 | (OK) | Navegacao com WRAP nos dois sentidos; contagens de item por tela sao fonte unica (`kPauseItemCount=4`, etc.) | `system_menu.hpp:63-66`; testes #897, #1111-1118 | ✓ |
| MEN-3 | (OK) | `system_menu_key_down` no-op seguro (None) em Hidden ou tecla sem efeito; ENTER em Sair devolve `RequestQuit` SEM fechar sozinho (o chamador decide encerrar) | `system_menu.hpp:160-165`; teste #1118 | ✓ |
| MEN-4 | (OK) | Mouse: `set_slider_ratio` clampa [0,1]; `click_option` = focar+confirmar por tela (Audio: Musica/SFX so FOCAM, Voltar confirma); indices fora do range = no-op | `system_menu.hpp:173-195`; testes #1137-1142 | ✓ |
| MEN-5 | (OK) | Hover: `system_menu_hover_index` (hit-test N caixas, respeita o count da tela) + `hover_entered_new_item` (edge-detect: toca so ao ENTRAR item novo, nao ao sair, re-dispara ao voltar) - logica PURA sem GL | `system_menu.hpp:213-249`; testes #1143-1148 | ✓ |
| MEN-6 | (OK) | Render RML data-driven le o estado (nao decide): item selecionado ganha `.focused`, `pressed_index` marca so o item indicado com `.pressed`, Hidden devolve doc minimo | `system_menu_rml.cpp`; testes #1149-1159 | ✓ |
| MEN-7 | (OK) | Som de hover+click provado headless: 5 moves (0,1,2,3,0) = 5 entradas novas + 1 click no choke-point `flash_pressed` = 6 em `sfx_play_count`; MESMO caminho de codigo do input real (sem `SDL_PushEvent`) | `system_menu_loop.cpp:400-430` (self-test `GUSWORLD_SYSMENU_HOVER_SELFTEST`) | ✓ |
| MEN-8 | (OK) | Click e hover VISUAIS sao nativos do glintfx (`:hover` RCSS, `set_click_callback`), so o SOM tem logica propria (edge-detect) - sem re-implementar hit-test visual | `system_menu_loop.cpp:307-353` | ✓ |

## Achados - persistencia settings.json (domain POCO + platform I/O)

| ID | Sev | Descricao | Evidencia | Estado |
|---|---|---|---|---|
| SET-1 | (OK) | Serializer/parser JSON e POCO puro que NUNCA lanca (recursive-descent minimo, toda falha -> `SystemSettingsParseError`); opera sobre string (I/O fica em platform) | `system_settings_json.cpp:1-36`; testes #666-673 (roundtrip, vazio, malformado sem crash, `{}` -> defaults, forward-compat, clamp, forma legivel) | ✓ |
| SET-2 | (OK) | Corpus de JSON malformado NAO crasha (degrada pra defaults); chave desconhecida ignorada (forward-compat); valores fora de [0,1] clampados (mesmo contrato de `AudioEngine::set_*_volume`) | testes #669, #671, #672 | ✓ |
| SET-3 | (OK) | I/O real (`load/save_system_settings`) degrada seguro: dir/arquivo ausente -> defaults sem log de erro (1a execucao); corrompido -> defaults com log; abertura falha -> defaults | `settings_file_store.cpp:48-75`; testes #768, #771 | ✓ |
| SET-4 | (OK) | LGPD leve: cria o diretorio com `0700` (owner_all) e o arquivo com `0600` (owner rw) - dado de preferencia do jogador so ele le/escreve; falha de permissao loga e SEGUE (nao perde a gravacao) | `settings_file_store.cpp:90-120`; teste #770 | ✓ |
| SET-5 | (OK) | `resolve_settings_dir` respeita `GUSWORLD_HOME` > `HOME` > fallback CWD (`.gusworld`), nunca lanca em HOME ausente | `settings_file_store.cpp:28-42`; testes #773, #774 | ✓ |
| SET-6 | 🟢 COSMETICO | Struct `JsonMember { std::string key; JsonValue* value=nullptr; }` (`system_settings_json.cpp:40-43`) e DEAD CODE: a arvore real usa `std::vector<std::pair<std::string, JsonValue>> obj` por valor; o struct com o ponteiro cru nao e usado em lugar nenhum (grep confirma). Sem risco (nao ha `new`/`delete` no arquivo, tudo RAII por valor), so ruido | `grep -nE '\bnew\b\|\bdelete\b' system_settings_json.cpp` -> VAZIO; `JsonMember` so aparece na definicao (linha 40) | ⚠ (trivial) |

## Memory-safety (ASan)

O parser de `settings.json` e entrada NAO-confiavel (o jogador pode editar o arquivo). Sob ASan+UBSan, a suite `domain` ([settings],[i18n], 33 test cases incluindo o corpus de malformados #669) roda LIMPA (141 assertions, exit 0): zero UAF, zero leak, zero UB no parser. Gerencia 100% por valor/containers (sem `new`/`delete`), apesar do struct morto SET-6.

## Conclusao

O menu e a persistencia estao bem estruturados: estado/render/IO separados, logica de decisao 100% POCO testavel, hover/click nativos com som edge-detectado provado headless, e a persistencia degrada seguro em todo caminho de falha com permissao de arquivo correta (LGPD leve). Parser de entrada nao-confiavel limpo sob ASan. Unico achado: SET-6 (dead code trivial, 🟢). Nenhum 🔴/🟠 neste capitulo.

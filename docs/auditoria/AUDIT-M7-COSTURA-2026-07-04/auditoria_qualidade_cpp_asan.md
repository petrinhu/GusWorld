# Auditoria: Qualidade C++ + memory-safety (ASan/UBSan)

- Subsistemas: codigo proprio do M7-COSTURA (maestro/menu/settings/fade) + a lib vendorizada `third_party/miniaudio`
- Criterio: AUD-MEM + AUD-QUALITY (sem UAF/leak/UB; RAII; ponteiros nao-donos claros; erros tipados)

## Contexto e metodo

Build extra de hardening (AddressSanitizer + UndefinedBehaviorSanitizer), reusando o build dir `build/asan/` ja parametrizado (`CMAKE_CXX_FLAGS=-fsanitize=address,undefined -fno-omit-frame-pointer -g`, RelWithDebInfo). Rebuild incremental dos 4 alvos de teste e execucao de cada suite. Os dossies M3/M5 so puseram core/domain sob ASan; **esta e a 1a passada de ASan sobre `platform/` (incluindo `platform/audio`)** no historico do projeto.

## Resultado por suite (evidencia primaria)

```
core     (fade_transition + demais):   All tests passed (2286 assertions in 150 test cases)   exit 0
domain   ([settings],[i18n]):          All tests passed (141  assertions in 33  test cases)   exit 0
app      (maestro/menu/defeat flavor): All tests passed (3172 assertions in 387 test cases)   exit 0
platform (audio):                      heap-use-after-free -> ABORT                            exit != 0
```

## Achados

| ID | Sev | Descricao | Evidencia | Estado |
|---|---|---|---|---|
| CPP-1 | (OK) | Codigo PROPRIO do M7-COSTURA (maestro_logic, system_menu POCO, system_settings JSON, fade_transition, integracao app/) LIMPO sob ASan+UBSan: 570 test cases (150 core + 33 domain + 387 app), 5599 assertions, zero UAF/leak/UB | saidas acima (exit 0 nas 3 suites) | ✓ |
| CPP-2 | (OK) | Sem `new`/`delete` cru no codigo novo (RAII por valor/containers; `unique_ptr` pra `SdlWindow`; `AudioEngine`/`SaveData` por valor) | `grep -rnE '\bnew\b\|\bdelete\b'` em maestro.cpp/system_menu*.cpp/settings -> VAZIO | ✓ |
| CPP-3 | (OK) | Ponteiro nao-dono `AudioEngine*` na battle_preview documentado (observador; o dono e o Maestro); regra dos 5 no Maestro (copia `= delete`); ordem de destruicao correta (renderer antes da janela) | `maestro.hpp:76-77, 130-137`; `maestro.cpp:66-72` (dtor) | ✓ |
| CPP-4 | (OK) | Erros por degradacao segura (nunca lanca em caminho de UI/IO): parser JSON never-throws; audio ausente -> `kInvalidSound` no-op; renderer/janela falha -> loga e segue | `system_settings_json.cpp:10`; `settings_file_store.cpp`; `maestro.cpp` (varios logs de degradacao) | ✓ |
| **ACH-1** | **🟠 IMPORTANTE** | **heap-use-after-free INTERNO da lib vendorizada `third_party/miniaudio/miniaudio.h`, na trilha de FALHA de carga de som.** `ma_resource_manager_data_buffer_node_acquire` libera `pDataBufferNode` (miniaudio.h:70918, `ma_free`, no ramo `result != MA_SUCCESS && nodeAlreadyExists == FALSE`) e LOGO EM SEGUIDA le `pDataBufferNode->isDataOwnedByResourceManager` (miniaudio.h:70926) do ponteiro JA liberado. Dispara sempre que um som NOVO (nao-cacheado) FALHA ao carregar (arquivo ausente/corrompido/formato nao decodificavel) | ASan: `heap-use-after-free ... in ma_resource_manager_data_buffer_node_acquire miniaudio.h:70926`; free em `:70918`; teste que dispara: `AudioEngine load_sfx com caminho inexistente degrada para kInvalidSound`. Codigo confirmado por leitura de `miniaudio.h:70914-70930` | - |

## Analise de ACH-1 (escopo, risco, por que 🟠 e nao 🔴)

**O que e:** bug de memory-safety DENTRO da dependencia vendorizada miniaudio (nao codigo GusWorld). O ramo de limpeza de falha (`done:` label) libera o node e depois le um campo dele para decidir se desinicializa uma notificacao ASYNC.

**Origem - NAO e regressao do M7-COSTURA:** a arvore miniaudio foi vendorizada em `a4ae4eb` e o wrapper `AudioEngine` (`load_music`/`load_sfx` -> `ma_sound_init_from_file`) entrou no **M6** (`93c3a4d`, ADR-011). O M7-COSTURA nao tocou o resource manager; so ADICIONOU `set_music_volume`/`set_sfx_volume` (que nao entram nessa trilha) e subiu a posse do device pro Maestro. O UAF sempre existiu; so aparece agora porque esta e a 1a auditoria a rodar `platform/audio` sob ASan.

**Por que e RELEVANTE ao M7-COSTURA mesmo assim:** o Inc 2/3 desenhou explicitamente a degradacao "se `battle_music_id_` ficar invalido (load falhou) a Maestro cai de volta pro tema da cidade" (`maestro.hpp:143-150`). Ou seja, a trilha de FALHA de carga - exatamente a que dispara o UAF - e um caminho de PRODUCAO planejado (arquivo de musica ausente no pacote, MP3 corrompido). Se `Arena_GusWorld.mp3` faltar, a Maestro entra nessa trilha.

**Por que NAO crasha o release (suite 1309/1309 verde):** e UB, nao um crash deterministico. Em release (sem ASan), a leitura de `pDataBufferNode->isDataOwnedByResourceManager` le memoria recem-liberada mas ainda nao reusada -> valor "provavel" ainda coerente; e o ramo que ele guarda (`... && (flags & ASYNC) != 0`) NAO e tomado para as cargas SINCRONAS do GusWorld (o AudioEngine carrega sincrono). Logo o UB "nao se manifesta" no caminho quente atual. Isso e sorte de layout, nao seguranca - um bump da miniaudio, uma flag ASYNC futura, ou um allocator diferente pode transformar em crash/corrupcao.

**Por que 🟠 e nao 🔴 (para o milestone M7-COSTURA interno):** (a) e third-party, nao codigo GusWorld; (b) nao e regressao do M7-COSTURA; (c) nao crasha o build de release, a suite 1309/1309 e verde; (d) a entrada e CONFIAVEL (assets proprios empacotados; um arquivo ausente e erro de dev/packaging, nao input de atacante). NAO bloqueia o milestone interno ja aprovado ao vivo. **POReM**, pela regra de severidade do `AUDITORIAS.md` ("UAF corrompe memoria = classe CRITICO para gate de RELEASE"), este achado **DEVE** ser tratado como bloqueador do gate T4/ASan de ship v1.0.0 (`TESTES.md` A-sections): nenhuma tag de release deve sair com UAF conhecido.

## Remediacao proposta (ACH-1)

1. **Bump/patch da miniaudio** para versao com o fix de `node_acquire` na trilha de falha (checar upstream; a arvore vive em `third_party/miniaudio/miniaudio.h`, memoria `reference_libs_vendorizadas`). OU
2. **Guarda no wrapper** `AudioEngine::load_music/load_sfx`: checar existencia/decodificabilidade do arquivo ANTES de delegar a `ma_sound_init_from_file` (curto-circuito para `kInvalidSound` sem entrar no resource manager), evitando a trilha de falha da lib por completo. Mais barato e sob nosso controle.
3. Adicionar um **job ASan sobre a suite `platform` no CI** (`.forgejo/workflows/ci.yml`) para o gate T4 de v1.0.0 - hoje o GATE nao roda sob sanitizer.
Sugestao: abrir item `AUD-MINIAUDIO-UAF` (ou `FIX-AUDIO-LOAD-GUARD`) na TODO.md, onda de higiene (M9) ou pre-v1.0.0.

## Conclusao

O codigo PROPRIO do M7-COSTURA e memory-safe (limpo sob ASan+UBSan, 570 test cases, RAII, ponteiros nao-donos claros, erros por degradacao segura). O unico achado de memory-safety (ACH-1) e um UAF PRE-EXISTENTE dentro da lib vendorizada miniaudio, na trilha de falha de carga de som - 🟠 para o milestone interno (nao regressao, nao crasha release, entrada confiavel), mas de tratamento obrigatorio antes do gate de ship v1.0.0.

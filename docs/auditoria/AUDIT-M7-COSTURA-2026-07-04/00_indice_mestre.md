# Dossie de Auditoria Interna: Marco M7-COSTURA + MENU-PAUSA-CONFIG-SOM

- Projeto: GusWorld (jogo indie solo, engine propria C++20 + SDL3; UI via glintfx/RmlUi; audio via miniaudio)
- Marco auditado: **M7-COSTURA** (costura cidade<->batalha: maestro, fade/boot-pixel, crossfade de musica, flavor da derrota) + **MENU-PAUSA-CONFIG-SOM** (menu de sistema em arvore de 7 telas + persistencia settings.json)
- Data: 2026-07-04
- Auditor interno: internal-auditor (dono do livro)
- Auditoria LOCAL (rodada na maquina; Bash/Read/Grep + build + smoke + GATE + ctest + build extra ASan/UBSan)
- Porte: jogo indie solo, escopo de auditoria ENXUTO e proporcional (mesmo padrao dos dossies AUDIT-M3 e AUDIT-M5-MOTOR)
- Commits auditados (origin/main, HEAD=7aa61ab): a cadeia M7-COSTURA `f0a7380`..`7b60d88` + os ajustes de status `4bacbe4`/`7aa61ab`:
  - `f0a7380`/`ef73b8a`/`4600726` costura Inc 1 (battle_preview embeddable, SdlWindow anexado, esqueleto do loop)
  - `e9d6e56`..`55c9e3d` fixes do playtest ao vivo (inimigo visivel/alcancavel, gus-centric, BUG-6 edge-trigger)
  - `9e85e5b`/`365f7fe`/`583416a`/`f996701` Inc 2 (AudioEngine sobe pro Maestro, fade preto + crossfade, boot pixelizado)
  - `aa8b2ea` Inc 3 (flavor da derrota - reboot nao-canonico)
  - `b32af0d`/`69b7e96`/`57729f3`/`69fed89`/`1a9cd04`/`921b8ff`/`d8f103f`/`e51f680`/`a3db687`/`7b60d88` MENU-PAUSA-CONFIG-SOM (fundacao -> arvore -> hover/click nativo + som)
- Referencias: ADR-012 (plano do M7), ADR-011 (audio onda 1, divida da battle_preview que o Inc 2 paga), ADR-010 (glintfx embed), ADR-007 (settings.json no padrao do controls.json), `AUDITORIAS.md` (manual criado nesta rodada), `TESTES.md`, `CONTRACT.md`

## 1. Escopo

Capitulos do livro (proporcional ao porte: costura de app/ + menu de UI + persistencia local; nenhum backend, rede ou banco):

| Capitulo (subsistema) | Arquivos auditados | Detalhamento |
|---|---|---|
| Invariante das 4 camadas (GATE) | `GusEngine/core`, `GusEngine/domain` (grep) | secao 4 + `auditoria_invariante_4camadas.md` |
| Maestro + ownership do AudioEngine | `app/{include,src}/maestro.hpp/.cpp`, `app/{include,src}/maestro_logic.*` | `auditoria_maestro_audio.md` |
| Transicao: fade preto + boot pixelizado + flavor da derrota | `core/anim/fade_transition.*`, `app/sdl_window.*`, `app/screens/battle_scene.*` (defeat_flavor) | `auditoria_transicao_defeat.md` |
| Menu de sistema (arvore 7 telas) + settings.json | `app/screens/system_menu*.{hpp,cpp}`, `domain/settings/*`, `platform/fs/settings_file_store.*` | `auditoria_menu_settings.md` |
| Paridade i18n (chaves novas: menu + defeat flavor) | `game/translations/{pt_br,en_intl}.md`, `tools/i18n_parity.py` | `auditoria_i18n_paridade.md` |
| Qualidade C++ + memory-safety (ASan/UBSan) | headers/src dos subsistemas + `third_party/miniaudio` | `auditoria_qualidade_cpp_asan.md` |

Metodo: leitura de codigo + reproducao do `tools/check.sh` (build+smoke+GATE+suite) + build extra sob AddressSanitizer + UndefinedBehaviorSanitizer rodando as 4 suites (core/domain/platform/app). Achado sem evidencia (arquivo:linha ou saida de comando) NAO entra.

## 2. Reproducao do build (evidencia primaria)

Comando canonico do projeto (`tools/check.sh` da raiz - build incremental + smoke headless + GATE de arquitetura + GATE de i18n + ctest):

```
=== build + smoke + gate + suite ===
BUILD=0
GusEngine 0.1.0 smoke OK (SDL): 120 ticks, cena=cidade real, jogador em (30.8, 2.4), 43 primitivos desenhados
SMOKE=0
GATE(arch): OK (sem Qt, SDL, RmlUi nem glintfx em core/ ou domain/).
source: game/translations/pt_br.md  (132 chaves)
locale     total traduz      % falta extra
------------------------------------------
en_intl      132      7   5.3%     0     0
GATE=0
100% tests passed, 0 tests failed out of 1309
Total Test time (real) =   5.10 sec
SUITE=0
=== resultado: OK (rc=0) ===
```

**Numero real de testes verdes confirmado: 1309/1309** (o TODO.md dizia "1309+"; bate). Build verde, smoke headless (SDL dummy) verde, GATE de 4 camadas verde, GATE de paridade i18n verde (0 faltando / 0 extra), suite verde.

Build extra de hardening (ASan + UBSan, RelWithDebInfo, reuso do build dir `build/asan/` ja parametrizado `-fsanitize=address,undefined -fno-omit-frame-pointer -g`), rodando cada suite:

```
core     (fade_transition + demais):   All tests passed (2286 assertions in 150 test cases)   exit 0
domain   ([settings],[i18n]):          All tests passed (141  assertions in 33  test cases)   exit 0
app      (maestro/menu/defeat flavor): All tests passed (3172 assertions in 387 test cases)   exit 0
platform (settings_file_store OK; AUDIO -> ABORT)                                             exit != 0
```

O codigo PROPRIO do M7-COSTURA (maestro_logic, system_menu POCO, system_settings JSON, fade_transition, e a integracao em app/) reconfirma **LIMPO** sob ASan+UBSan: zero use-after-free, zero leak, zero UB nas 3 suites que o exercitam (core/domain/app, somando 570 test cases). A suite `platform` ABORTA sob ASan, mas por um **use-after-free interno da lib vendorizada miniaudio** (nao do codigo GusWorld) na trilha de FALHA de carga de som - ver achado ACH-1 em `auditoria_qualidade_cpp_asan.md`.

## 3. Sumario executivo

O M7-COSTURA e o MENU-PAUSA-CONFIG-SOM estao **funcionalmente solidos e aprovados ao vivo pelo lider** (2026-07-04, "tudo funcionando, sem bugs ou alteracoes perceptiveis"). A arquitetura respeita a invariante das 4 camadas de forma comprovavel: a logica de decisao da costura (`maestro_logic`) e do menu (`system_menu`) e POCO puro headless; a serializacao de `settings.json` vive em `domain/settings` (POCO, opera sobre string, zero I/O); o I/O real fica em `platform/fs/settings_file_store` (fronteira). O GATE de arquitetura (`grep` por Qt/SDL/RmlUi/glintfx em `core/`+`domain/`) esta **vazio** nas duas checagens (include e namespace). A posse do AudioEngine subiu corretamente da `battle_preview` para o `Maestro` (paga a divida do ADR-011): 1 instancia viva pro loop inteiro, device nao reaberto a cada batalha, crossfade cidade<->arena cronometrado no pico da opacidade do fade preto, com degradacao segura (`kInvalidSound` -> no-op) se um arquivo de musica faltar. O flavor da derrota (Inc 3) e gateado por `CombatOutcome::Defeat`, envelhece o timer so no Defeat e trava (nao religa), com bark do companion vivo (nome interpolado via `{0}`) ou variante GENERIC - coberto por 5 testes dedicados. O menu em arvore (Pause -> Save/ConfigCategories -> Audio/Video/Language) preserva a selecao do pai, navega por teclado+mouse, com hover/click NATIVOS do glintfx (v0.3.1) e som edge-detectado (toca so ao ENTRAR num item novo) - provado headless (5 hovers + 1 click = 6 em `sfx_play_count`). A persistencia grava com permissao `0600`/`0700` (dado do jogador, LGPD leve) e degrada para defaults em arquivo ausente/corrompido sem crashar (teste com corpus malformado).

**A auditoria NAO e um selo limpo.** Ha DOIS achados **🟠 IMPORTANTE** abertos (zero 🔴 no codigo proprio do M7-COSTURA; zero 🟢-only):

- **ACH-1 (🟠 IMPORTANTE) - use-after-free na lib vendorizada miniaudio, trilha de FALHA de carga de som.** Sob ASan, `ma_resource_manager_data_buffer_node_acquire` (miniaudio.h:70918 libera `pDataBufferNode`; :70926 le `pDataBufferNode->isDataOwnedByResourceManager` do ponteiro JA liberado) - dispara quando um arquivo de musica/SFX **falha ao carregar** (ausente/corrompido/formato nao decodificavel). NAO e codigo GusWorld, NAO e regressao do M7-COSTURA (miniaudio foi vendorizada e o wrapper `AudioEngine` entrou no **M6**, commit `93c3a4d`; esta e a 1a passada de ASan sobre `platform/audio` - os dossies M3/M5 so cobriam core/domain). Mas e **relevante ao M7-COSTURA** porque o Inc 2/3 desenhou explicitamente a degradacao "load falhou -> cai de volta pro tema da cidade", roteando exatamente por essa trilha bugada. Nao crasha o build de release (a suite 1309/1309 e verde: em release o ramo guardado pela leitura do ponteiro liberado nao e tomado para cargas sincronas - e UB que "nao se manifesta"). Ver `auditoria_qualidade_cpp_asan.md`.

- **ACH-2 (🟠 IMPORTANTE) - placeholders `{0}/{1}` nao interpolados na dica do menu de pausa.** `game/translations/pt_br.md` traz `MENU_PAUSE_HINT = "{0} confirma, {1} volta ao jogo"`, mas `system_menu_rml.cpp:399` renderiza `tr.tr("MENU_PAUSE_HINT")` **sem** substituir os placeholders (o `Translator` de app/ nao tem overload de interpolacao; quem precisa faz `find/replace` a mao - a `battle_scene.cpp:2071` faz isso pro bark da derrota, mas o menu nunca foi ligado). O jogador ve o texto literal `{0} confirma, {1} volta ao jogo` no rodape da tela de pausa. O teste `system_menu_rml_test.cpp:37` NAO pega isso porque o fixture usa uma dica SEM placeholders (`"Enter confirma, ESC volta"`). Fix de baixo custo (interpolar os rotulos de tecla confirmar/voltar, OU tirar os `{0}/{1}` da string). Ver `auditoria_i18n_paridade.md`.

Parecer: o milestone esta **APTO com ressalvas**. Os dois achados sao 🟠 (nao 🔴): nenhum crasha o jogo em uso normal, nenhum corrompe save, nenhum quebra a costura ou a navegacao do menu. A recomendacao de status esta na secao 7.

## 4. Invariante das 4 camadas (GATE) - chave

| ID | Sev | Descricao | Evidencia | Remediacao | Estado |
|---|---|---|---|---|---|
| INV-1 | (OK) | Nenhum include de Qt/SDL/RmlUi/glintfx em `core/`+`domain/` | `grep -rnE '#include <(Q[A-Za-z]\|SDL\|RmlUi\|glintfx)' core domain` -> VAZIO (o mesmo grep do `tools/check.sh` GATE(arch) e do CI) | n/a | ✓ |
| INV-2 | (OK) | Nenhum uso de namespace `Rml::` nem `glintfx::` em `core/`+`domain/` | `grep -rnE '\b(Rml\|glintfx)::' core domain` -> VAZIO | n/a | ✓ |
| INV-3 | (OK) | Nenhum I/O direto (`<fstream>`/`<filesystem>`/`ifstream`/`ofstream`/`std::filesystem`) em `core/`+`domain/`; a serializacao de settings.json em `domain/settings` opera sobre STRING (o I/O vive em `platform/fs`) | `grep -rlnE '<fstream>\|<filesystem>\|std::filesystem\|ifstream\|ofstream' core domain` -> VAZIO | n/a | ✓ |
| INV-4 | (OK) | `domain/settings` (novo no M7-COSTURA) e POCO: zero SDL | `grep -rn SDL domain/.../settings` -> VAZIO | n/a | ✓ |

Conclusao: `core/` e `domain/`, incluindo o novo `domain/settings`, sao POCO puro headless. O SDL (`SDL_Keycode`) usado pela logica pura do menu (`app/screens/system_menu.hpp`) vive na camada `app/` (SDL-aware por design, mesmo padrao de `battle_key_down`), fora do GATE. Invariante das 4 camadas **CUMPRIDA**.

## 5. Contagem de achados por severidade

| Severidade | Quantidade |
|---|---|
| 🔴 CRITICO | 0 |
| 🟠 IMPORTANTE | 2 (ACH-1 miniaudio UAF na trilha de falha de carga; ACH-2 `{0}/{1}` nao interpolado na dica do menu) |
| 🟢 COSMETICO | 1 (SET-6: struct `JsonMember` dead-code no parser de settings.json - ver `auditoria_menu_settings.md`) |
| Total | 3 |

Nota fora-de-escopo: no diff NAO-commitado de `battle_preview.cpp` (COCKPIT-BARRAS-MANA-AP, item de PI8) ha um comentario registrando que o lider quer TESTAR barra vs pip pro AP. NAO e achado do M7-COSTURA (e trabalho em andamento de outro item, nao commitado); registrado so por transparencia, nao entra na contagem.

## 6. Historico de commits vs TODO.md (verificacao pedida)

O `git log` da cadeia M7-COSTURA bate com o que o TODO.md descreve: fundacao (`b32af0d`) -> integracao Esc->pausa + slider polygon + persistencia (`57729f3`) -> painel centralizado + navegacao WASD+mouse (`69fed89`) -> onda arvore (`1a9cd04`) -> fixes de captura acento/slider (`921b8ff`) -> bump glintfx v0.3.0->v0.3.1 gradiente+fix UAF-fantasma (`d8f103f`) -> gradiente no hexagono + fantasma morto (`e51f680`) -> SFX hover+click (`a3db687`) -> centraliza pills + hover nativo + som (`7b60d88`). O pin do glintfx no `GusEngine/CMakeLists.txt:105` e **`v0.3.1`**, coerente com o TODO.md e o CLAUDE.md. O GLINTFX-GHOST-PIN e citado como resolvido no v0.3.1 (o commit `d8f103f` traz "fix load() fantasma/UAF" na lib) - coerente. Nenhuma divergencia entre o board e o codigo.

## 7. Parecer de prontidao + recomendacao de status

**APTO com ressalvas.** O dossie e honesto: lista os 2 achados 🟠 (ACH-1 miniaudio UAF; ACH-2 `{0}/{1}` na dica) e o 🟢 fora-de-escopo (COS-1). Nenhum 🔴. A suite de 1309 testes cobre os criterios de aceite da costura, do menu e da persistencia, e o codigo PROPRIO do M7-COSTURA reconfirma limpo sob ASan+UBSan (570 test cases). O lider aprovou o comportamento ao vivo.

**Recomendacao (o internal-auditor NAO promove sozinho - decisao de status sobe ao lider):**

- **NAO** marcar `✅ Concluido` + `✓` de forma limpa. Ha 2 achados 🟠 abertos.
- Opcao recomendada: marcar o item como **`✅ Concluido` (feature entregue e aprovada ao vivo) mas com Estado Auditado `⚠` (com ressalvas)**, rastreando os 2 🟠 como follow-up:
  - **ACH-2** (`{0}/{1}` no menu) - fix trivial de baixo custo, cabe fechar ainda nesta onda ou como polish rapido; ao fechar + re-teste, o `⚠` sobe pra `✓`.
  - **ACH-1** (miniaudio UAF) - NAO e regressao do M7-COSTURA nem bloqueia o milestone interno; mas e um item de memory-safety que **DEVE** ser remediado antes do gate T4/ASan de ship v1.0.0 (`TESTES.md` A-sections): bump/patch da miniaudio, ou guarda no wrapper `AudioEngine` que evite chamar `ma_sound_init_from_file` em arquivo inexistente. Sugiro abrir um item proprio (ex.: `AUD-MINIAUDIO-UAF`) na `TODO.md`, onda de higiene (M9) ou pre-v1.0.0.
- So marcar `✓` (aprovado limpo) apos ACH-2 fechado + re-teste; e registrar ACH-1 como debito conhecido rastreado (nao bloqueia o `⚠`, bloqueia o ship gate).

Decisao final de status e do lider.

## 8. Recomendacao de especialistas (para a thread principal disparar, NAO bloqueia o dossie)

- `security-engineer` / `devops-sre`: enderecar **ACH-1** - avaliar bump da miniaudio (a arvore vendorizada e `third_party/miniaudio/miniaudio.h`, ver `reference_libs_vendorizadas`) para versao com o fix do UAF de `node_acquire` na trilha de falha, OU endurecer o wrapper `AudioEngine::load_music/load_sfx` para checar existencia/decodificabilidade do arquivo ANTES de delegar a miniaudio (curto-circuito para `kInvalidSound` sem entrar no resource manager). Adicionar um job ASan sobre a suite `platform` no CI (hoje o GATE nao roda sob sanitizer) para o gate T4 de v1.0.0.
- `backend-engineer` / `technical-writer` (ou quem tocar a UI do menu): enderecar **ACH-2** - ou dar ao `Translator` de app/ um overload `tr(key, args...)` que interpole `{0..n}` (reuso em vez de `find/replace` a mao espalhado), ou remover os placeholders da string `MENU_PAUSE_HINT` se a dica nao precisa dos rotulos de tecla dinamicos. Endurecer o fixture do teste `system_menu_rml_test` para usar uma dica COM `{0}/{1}` e assertar que os placeholders somem do render.
- `qa-engineer` (opcional): teste de regressao que carrega um arquivo de musica INEXISTENTE via a Maestro e verifica degradacao segura fim-a-fim (hoje coberto no nivel de unidade do `AudioEngine`, mas nao integrado na Maestro sob ASan).

## 9. Estado dos arquivos do livro

- `00_indice_mestre.md` (este)
- `auditoria_invariante_4camadas.md`
- `auditoria_maestro_audio.md`
- `auditoria_transicao_defeat.md`
- `auditoria_menu_settings.md`
- `auditoria_i18n_paridade.md`
- `auditoria_qualidade_cpp_asan.md`

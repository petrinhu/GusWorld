# Dossiê de Auditoria Interna: SAVE-LOAD-UI (tela de título + salvar/carregar + autosave)

- Projeto: GusWorld (jogo indie solo, engine própria C++20 + SDL3; UI via glintfx/RmlUi)
- Sistema auditado: **SAVE-LOAD-UI** completo — tela de título (Continuar/Novo Jogo/Sair),
  autosave por gatilho + política de local, telas de Salvar/Carregar (7 slots: Auto + 6
  manuais), feature Apagar, persistência real em disco (`FsSaveStore`), harness de
  interação GL headless.
- Data: 2026-07-09/10
- Auditor interno: `internal-auditor` (dono do livro)
- Auditoria **LOCAL, READ-ONLY** (rodada na máquina; nenhum código de produção alterado).
  Método: leitura de código + reprodução do `tools/check.sh` (build+smoke+GATE+i18n+suite)
  + build extra sob ASan/UBSan (reuso de `build/asan/`) + reprodução AO VIVO da captura
  visual real (`GUSWORLD_SAVELOAD_SCREENSHOT_DIR`/`GUSWORLD_TITLE_SCREENSHOT_DIR`, Xvfb
  :99) + um programa de verificação isolado em `/tmp` (não commitado, não parte do repo)
  ligado contra as bibliotecas estáticas já compiladas, para reproduzir com evidência real
  um achado de integridade de dados (ver capítulo 4).
- Commits auditados (origin/main, HEAD=`5a5fb23`): a cadeia SAVE-LOAD-UI
  `91d0e64`..`67a81db` + o registro de status `5a5fb23`:
  - `92e1051` tela de salvar/carregar (POCO + RML), etapas 1-3 parciais
  - `91d0e64` silencia warning + remove `using` morto
  - `a24a699`/`42f3bd3` wiring real de Salvar/Carregar no menu de pausa (etapa 6) +
    bump `kManualSlotCount` 5→6
  - `85188e8`/`375d669` tela de título + autosave por local (etapas 4/5) + ajuste
    autosave-só-vitória
  - `bc1cb8a` SFX de hover/clique no menu de título (paridade)
  - `67a81db`/`5a5fb23` retoque ao vivo (mouse na tela de save/load + feature Apagar +
    harness GL)
- Referências: ADR-006 (política de slots/backup), ADR-007 (I/O não-lançante), ADR-012
  (M7-COSTURA), `AUDITORIAS.md` (manual canônico do stack), `TESTES.md`, `CONTRACT.md`,
  memória `project_save_dungeon_pem_faraday`, memória `project_morte_dificuldade_canon`.

## 1. Escopo

Capítulos do livro (proporcional ao porte: 1 feature de UI + I/O de disco local, sem
rede/backend/banco):

| Capítulo (subsistema) | Arquivos auditados | Detalhamento |
|---|---|---|
| Invariante das 4 camadas (GATE) | `GusEngine/domain/save/*`, `GusEngine/platform/fs/save_file_store.*`, `GusEngine/app/screens/{save_load_menu,title_menu}*` | `auditoria_invariante_4camadas.md` |
| Cobertura de testes / TDD | `app/tests/{save_load_menu,save_load_menu_rml,save_load_menu_interaction,title_menu,title_menu_rml,maestro_logic}_test.cpp`, `platform/tests/save_file_store_test.cpp`, `domain/tests/save_policy_test.cpp` | `auditoria_cobertura_testes.md` |
| Integridade de dados de save (achado-chave) | `save_load_menu_loop.cpp` (build_previews_and_cache/do_save/do_delete), `save_load_menu.cpp` (confirm_selected_slot), `save_file_store.cpp` (delete_save/backup) | `auditoria_integridade_dados_save.md` |
| Segurança do I/O de save (path/symlink/permissão) | `save_file_store.cpp`, `maestro.cpp` (frozen_city_snapshot_path) | `auditoria_seguranca_io.md` |
| Paridade i18n + integridade da evidência visual | `game/translations/{pt_br,en_intl}.md`, `tools/i18n_parity.py`, `docs/design/mockups/menu_capturas/*.png` | `auditoria_i18n_paridade.md` |
| UX: pontos sinalizados pro líder + gap conhecido | `save_load_menu_loop.cpp` (SFX genérico), feature Apagar no Auto, avisos de load não implementados | `auditoria_ux_pontos_lider.md` |

Achado **sem evidência** (arquivo:linha ou saída de comando/execução real) **NÃO entra**
no livro.

## 2. Reprodução do build + suíte (evidência primária)

`tools/check.sh` da raiz (build incremental + smoke headless + GATE de arquitetura + GATE
de i18n + ctest):

```
BUILD=0
GusEngine 0.1.0 smoke OK (SDL): 120 ticks, cena=cidade real, jogador em (30.8, 2.4), 43 primitivos desenhados
SMOKE=0
GATE(arch): OK (sem Qt, SDL, RmlUi nem glintfx em core/ ou domain/).
source: game/translations/pt_br.md  (217 chaves)
locale     total traduz      % falta extra
------------------------------------------
en_intl      217      8   3.7%     0     0
GATE=0
100% tests passed, 0 tests failed out of 1625
Total Test time (real) =  13.32 sec
SUITE=0
=== resultado: OK (rc=0) ===
```

**1625/1625 confirmado** (bate com o que a `TODO.md` registra). GATE de 4 camadas vazio
(reproduzido também isoladamente, ver capítulo 3). GATE de i18n com **0 chave faltando /
0 extra** entre `pt_br`/`en_intl` (217 chaves, incluindo as ~30 novas `SAVE_*`/`TITLE_*`
desta feature).

Build extra sob ASan+UBSan (reuso de `build/asan/`, já parametrizado):

```
domain "[save]"                          : All tests passed (427 assertions, 75 test cases)  exit 0
platform "[save]" (save_file_store isolado): All tests passed (69 assertions, 19 test cases)  exit 0
app "[maestro]" (autosave/edge/crossfade) : All tests passed (143 assertions, 39 test cases)  exit 0
app "[save_load_menu],[title_menu]"      : All tests passed (191 assertions, 79 test cases)  exit 0
```

Nota: a suíte `platform` **completa** ainda aborta sob ASan por causa do **ACH-1 já
conhecido e rastreado** (use-after-free na lib vendorizada miniaudio, ver dossiê
`AUDIT-M7-COSTURA-2026-07-04/auditoria_qualidade_cpp_asan.md` — não é regressão desta
onda, não toca `domain/save`/`platform/fs`). Isolar por tag `[save]` contorna o abort e
confirma que o código **PRÓPRIO** desta feature está limpo sob ASan+UBSan.

Harness de interação GL real (Xvfb :99, `DISPLAY=:99 SDL_VIDEODRIVER=x11`):

```
gusengine_app_tests "[save_load_menu_interaction]" -> All tests passed (29 assertions in 2 test cases)
```

(29 assertions ≠ 0 confirma que o harness **exerceu GL de verdade** — não caiu no ramo de
degradação segura "sem display"; ver `auditoria_cobertura_testes.md`.)

Reprodução AO VIVO da captura visual (binário real `gusworld_app`, `GUSWORLD_HOME` de
scratch, nunca o `$HOME` do líder):

```
DISPLAY=:99 GUSWORLD_HOME=<scratch> GUSWORLD_SAVELOAD_SCREENSHOT_DIR=<scratch> ./gusworld_app
-> save_load_save.png / save_load_load.png (1280x720) gerados com sucesso
```

PNGs abertos e conferidos (ver capítulo 5 — achado IMP-1: eles **divergem** dos PNGs
commitados no dossiê de mockups).

## 3. Invariante das 4 camadas (GATE) — chave

| ID | Sev | Descrição | Evidência | Estado |
|---|---|---|---|---|
| INV-1 | (OK) | `gus::domain::save::save_policy` (novo, política de autosave por local) é POCO 100% puro — zero I/O, zero SDL, `constexpr` | `save_policy.hpp:46-51`; `grep -n "SDL\|fstream\|filesystem" save_policy.hpp` → só comentário, zero uso real | ✓ |
| INV-2 | (OK) | Nenhum include de Qt/SDL/RmlUi/glintfx em `core/`+`domain/` (mesmo grep do GATE/CI) | `grep -rnE '#include <(Q[A-Za-z]\|SDL\|RmlUi\|glintfx)' core domain` → VAZIO | ✓ |
| INV-3 | (OK) | Nenhum uso de namespace `Rml::`/`glintfx::` em `core/`+`domain/` | `grep -rnE '\b(Rml\|glintfx)::' core domain` → VAZIO | ✓ |
| INV-4 | (OK) | Nenhum I/O direto (`fstream`/`filesystem`) em `core/`+`domain/` — inclusive o novo `domain/save/save_policy.hpp` | `grep -rlnE '<fstream>\|<filesystem>\|std::filesystem\|ifstream\|ofstream' core domain` → VAZIO | ✓ |
| INV-5 | (OK) | `save_load_menu.hpp`/`title_menu.hpp` (lógica de UI, `app/`) são POCO testáveis sem GL/janela — só dependem de `SDL_Keycode` (tipo, não runtime); o I/O real (`FsSaveStore`) fica isolado em `platform/fs/`, consumido só pelos `*_loop.cpp` (GL-heavy, `app/`) | leitura de `save_load_menu.hpp`/`.cpp`, `title_menu.hpp`/`.cpp` — zero `#include <fstream>`/`SDL_Init`/glintfx nesses 4 arquivos | ✓ |
| INV-6 | (OK) | `FsSaveStore` (a única fronteira real de bytes↔disco) vive em `platform/fs/`, implementa o port `SaveStore` do domínio (Dependency Inversion) | `save_file_store.hpp:53` (`class FsSaveStore final : public gus::domain::save::SaveStore`) | ✓ |

Conclusão: nenhuma regressão de camada entrou com o SAVE-LOAD-UI. Detalhe completo em
`auditoria_invariante_4camadas.md`.

## 4. Achados por severidade

| Severidade | Quantidade |
|---|---|
| 🔴 CRÍTICO | 1 (CRIT-1: slot corrompido exibido como "Vazio" pula a confirmação de sobrescrita no modo Salvar — repro real executado) |
| 🟠 IMPORTANTE | 1 (IMP-1: PNGs de "prova visual" commitados divergem do catálogo real de i18n) |
| 🟢 COSMÉTICO | 1 (COS-1: comentário desatualizado em `save_slots.hpp` pós-bump 5→6) |
| Total achados de severidade | 3 |
| Pontos sinalizados pro líder (não é achado, ver §6) | 2 |
| Gap conhecido (aceito pelo líder, não é achado) | 1 (os 2 avisos de load — versão incompatível/corrompido e controles-diferentes) |

## 5. Sumário executivo

O SAVE-LOAD-UI está **funcionalmente sólido e bem coberto de testes** na superfície que
o próprio time já cobre: a arquitetura respeita as 4 camadas de forma comprovável
(`save_policy` é POCO puro e testado com os 5 combos booleanos + `constexpr`; a lógica
de navegação/estado das 3 telas — título, salvar, carregar — é 100% POCO testável sem
GL); a paridade i18n está **limpa** (0 faltando/0 extra, 217 chaves); o harness de
interação (`save_load_menu_interaction_test.cpp`) **de fato** exercita um clique real de
mouse via `SDL_PushEvent` na mesma fila que o loop de produção consome, e prova
empiricamente (29 assertions, não 0) que o clique grava em disco — não é um teste
decorativo. `delete_save` apaga a cadeia INTEIRA de backup e tem cobertura dedicada
(idempotência, não mexe em outros slots, autosave apagável, slot inválido fail-fast). O
gate Vitória-só-autosalva no retorno de batalha está isolado num predicado `constexpr`
puro e testado nos 4 outcomes.

**A auditoria encontrou 1 achado 🔴 CRÍTICO real, com repro executado e evidência de
saída** (não é teórico): quando o arquivo PRIMÁRIO de um slot existe mas está
corrompido/adulterado (`LoadResult` != `Ok` — HmacInvalid, Corrupt, VersionTooNew,
Invalid ou WrongSlot), a tela de Salvar o exibe como **"Vazio"** e, por isso, um
clique/Enter nesse slot grava **DIRETO, sem abrir o mini-diálogo de confirmação de
sobrescrita** — mesmo havendo dado recuperável na cadeia de backup daquele slot. A
gravação subsequente rotaciona a cadeia sem qualquer sinal ao jogador, e gravações
"inocentes" repetidas nesse slot que ele acredita vazio eventualmente empurram dado bom
pra fora da janela de profundidade N=3 (perda definitiva). Ver capítulo 4 do livro
(`auditoria_integridade_dados_save.md`) para o repro completo, executado com o código
real (biblioteca já compilada do projeto, sem alterar nada).

Também há 1 achado 🟠 IMPORTANTE de **integridade da trilha de evidência** (não é um bug
de produção): os 2 PNGs de "prova visual" commitados em
`docs/design/mockups/menu_capturas/` (`save_load_save.png`/`save_load_load.png`) não
refletem o catálogo real de i18n nem uma execução real do self-test descrito no commit —
mostram literais `"x"` de um fixture de teste em vez de "Praça da Compilação"/frases
completas, e um timestamp/XP/Capítulo que não batem com nenhuma captura real possível
via `GUSWORLD_SAVELOAD_SCREENSHOT_DIR`. Reproduzi a captura real ao vivo (Xvfb :99,
binário `gusworld_app` de produção) e ela mostra o catálogo correto — ou seja, **o código
de produção está certo**, só a evidência commitada está errada/obsoleta.

1 achado 🟢 cosmético (comentário stale) e 2 pontos que o brief pediu para **NÃO decidir**
e só registrar pro líder (som de clique genérico; Auto apagável com o mesmo diálogo) —
ver `auditoria_ux_pontos_lider.md`. O gap dos 2 avisos de load (deferidos por decisão do
líder) é reafirmado como gap conhecido, não achado.

## 6. Parecer de prontidão + recomendação de status

**NÃO APTO para fechar `✅`/`✓` no Estado Auditado enquanto CRIT-1 estiver aberto.** Por
definição do próprio `AUDITORIAS.md` ("Nenhum item vai para uma tag de release com
achado 🔴 CRÍTICO aberto" / "Um 🔴 aberto impede marcar o item como ✅"), este achado
**bloqueia** o fechamento limpo do item `SAVE-LOAD-UI` na `TODO.md`, embora **não bloqueie
o vertical slice em si** (a pré-condição para o bug é um arquivo de save já corrompido —
raro em uso normal, mas dentro do modelo de ameaça que o próprio `AUDITORIAS.md`/AUD-SEC
já define como escopo: "arquivos que o jogador pode adulterar").

**Recomendação (o `internal-auditor` NÃO promove sozinho — decisão de status sobe ao
líder):**

- Remediar **CRIT-1** antes de marcar `✅`: distinguir, no `SaveSlotPreview`/no fluxo de
  `build_previews_and_cache`, "genuinamente vazio" (`has_save()==false`) de "presente mas
  ilegível" (arquivo existe, `LoadResult != Ok`) — e gatear a confirmação de sobrescrita
  em "existe arquivo primário", não só no `occupied` de exibição. É um fix cirúrgico
  (não exige os 2 avisos completos de UI já deferidos — só fechar a brecha de segurança
  da confirmação). Ver proposta detalhada em `auditoria_integridade_dados_save.md` §3.
- Regenerar os 2 PNGs de **IMP-1** via `GUSWORLD_SAVELOAD_SCREENSHOT_DIR` real (comando
  já confirmado nesta auditoria, §2 acima) e re-commitar.
- **COS-1**: ajuste trivial de comentário, cabe no mesmo commit de qualquer um dos dois
  acima.
- Os 2 pontos de UX (§6 de `auditoria_ux_pontos_lider.md`) sobem pro líder decidir — não
  bloqueiam.
- Depois de CRIT-1 fechado + re-teste (idealmente com 1 teste novo que cubra
  exatamente o cenário do repro: slot com primário corrompido + clique em modo Save),
  o Estado Auditado pode subir a `✓`.

Decisão final de status é do líder.

## 7. Estado dos arquivos do livro

- `00_indice_mestre.md` (este)
- `auditoria_invariante_4camadas.md`
- `auditoria_cobertura_testes.md`
- `auditoria_integridade_dados_save.md`
- `auditoria_seguranca_io.md`
- `auditoria_i18n_paridade.md`
- `auditoria_ux_pontos_lider.md`

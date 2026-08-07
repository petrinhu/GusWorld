# Plano — reorganizar `app/` tocando SDL3 direto (quebra do contrato de 4 camadas)

**Status:** proposto, aguardando aprovação do líder. **Este documento não altera código, `CMakeLists.txt`, `tools/check.sh` nem `TODO.md`.**
**Medido em:** commit `b0d9dcd` (2026-07-29). Verificado que nada em `GusEngine/app/` mudou até o HEAD atual `c514851` na hora de escrever este plano (`git diff --stat b0d9dcd c514851 -- GusEngine/app/` vazio) — os números abaixo continuam válidos, mas a árvore está sob edição concorrente de vários agentes (`bump-glintfx-023`, `cockpit-8-para-9`, `dossie-fonte-draw2d`, `fix-font-atlas-bake`, `impl-render2d-glintfx`, `verif-4dir`); **reconferir os greps abaixo antes de executar qualquer fatia**.
**CORREÇÃO 2026-08-07 (`FATIA1-LOG-CLOCK`):** a categoria `log` deste plano dizia **30 ocorrências**; recontagem independente com o mesmo `strip_comments()` do gate (`tools/sdl_log_clock_zero.py`), sobre a árvore no commit imediatamente anterior à migração (`4f4bc9ee`), deu **32** (4 arquivos: `app_icon.cpp` 3, `maestro.cpp` 5, `sdl_window.cpp` 7, `npc_dialogue_loop_gl.cpp` 17). Os totais dependentes (linha 27/48/61/107/146) foram corrigidos junto. `SDL_GetTicksNS` (categoria `relógio`) reconferiu em **12**, dentro dos 17 já citados (os outros 5 são `SDL_Delay`, que não migrou) — sem mudança aí.

## 0. O contrato quebrado

`CLAUDE.md` do projeto: `platform/` é "a única fronteira que toca bibliotecas externas"; `app/` é "telas do jogo, gameplay, ferramentas internas". Na prática, `app/` inclui `#include <SDL3/SDL.h>` e chama a API SDL3 direto em **62 arquivos** (produção + testes + ferramentas). `tools/check.sh` só varre `core/` e `domain/` no `GATE(arch)` (`tools/check.sh:92-99`) e só varre RmlUi (não SDL) no `GATE(excl)` sobre `platform/`+`app/` (`tools/check.sh:115-127`) — **nunca existiu trava pra isso.**

## 1. Metodologia de medição (reproduzível)

Comando usado para listar arquivos candidatos:
```
rg -l 'SDL_' GusEngine/app -g '*.cpp' -g '*.hpp'
```
Isso devolveu **74 arquivos**. Mas `grep`/`rg` cru mente por comentário (a mesma armadilha já registrada no bus do glintfx hoje: "flush aparece 17× ... todas em comentário"). Escrevi um script Python (`/var/tmp/builds/.../scratchpad/measure_sdl.py`, preservado no scratchpad da sessão) que:

1. Remove `//...`, `/*...*/`, dentro de strings/chars não conta como comentário (parser de caractere, não regex ingênua) — a mesma classe de erro citada no recado do glintfx (L1: "flush aparece 17×, todas em comentário").
2. Extrai todo token `\bSDL_[A-Za-z0-9_]*\b` do texto já sem comentário.
3. Categoriza cada token por uma tabela de regex fechada (janela/contexto GL, eventos/loop, input, render SDL, log, relógio, diversos) e soma por arquivo.
4. Confirma que a contagem não é zero por escape errado (a outra armadilha do dia): rodei o script, ele achou tokens reais em 62/74 arquivos — os 12 restantes só tinham `SDL_` dentro de comentário (falso positivo do `rg` cru, confirmado manualmente em 3 amostras).

Resultado: **62 arquivos** (não 74) realmente tocam SDL3 fora de comentário. Divididos por papel no build (confirmado em `app/CMakeLists.txt:191` — só `add_subdirectory(tests)`; `app/tools/` **não** é `add_subdirectory` de lugar nenhum, tem `CMakeLists.txt` **próprio e standalone**, `app/tools/CMakeLists.txt:1-3`, que importa os `.a` já compilados do preset principal — não compila dentro de `gusworld_app`):

| grupo | arquivos | total de tokens SDL_ | onde compila |
|---|---|---|---|
| **produção** (`src/` + `include/gus/app/`, exceto `tests/`) | **32** | 459 | `gusengine_app` (linka em `gusworld_app`, o binário shippado) |
| `tests/` | 16 | 540 | binário `ctest` próprio (Catch2), não shippa |
| `tools/` | 14 | 453 | projeto CMake **standalone e separado** (`app/tools/CMakeLists.txt`), não shippa, não faz parte de `gusengine_app` |

⚠️ **Reconciliação com os números do brief (36) e a correção do team-lead (50), feita com o mesmo rigor de medição, não por estimativa:** o team-lead corrigiu o próprio número de 36 para 50 (36 em `src`+`include`, +14 de `tools/` que tinha esquecido de somar). Refiz a comparação exata:

```
rg -l 'SDL_' app/src app/include -g '*.cpp' -g '*.hpp'  →  36 arquivos (comentário incluso)
rg -l 'SDL_' app/tools                -g '*.cpp' -g '*.hpp'  →  14 arquivos (comentário incluso)
36 + 14 = 50   ← bate EXATO com a correção do team-lead
```

O "36" bruto **bate exatamente** com o `rg -l` cru sobre `src`+`include` (sem filtrar comentário); o "50" corrigido bate exatamente com `36+14`. Meu número filtrado (32, seção acima) é **4 a menos** que o 36 bruto porque removi 4 arquivos onde `SDL_` só aparece dentro de comentário — confirmado manualmente, linha por linha, nos 4:

```
app/include/gus/app/audio_smoke.hpp:20        // Nao abre janela nem toca SDL - roda e sai antes de qualquer SDL_Init.
app/include/gus/app/boot_pixel_overlay.hpp:30 // - funciona IDENTICO no SDL_Renderer classico da cidade e no GL3 da arena...
app/include/gus/app/maestro_logic.hpp:219     // SDL_Init - Maestro::to_battle() devolve `battle_requested_quit=true`...
app/src/screens/battle_cockpit_rml.cpp:57     // ...achado da F0: o SDL_Renderer nao fazia.
```

Nenhum dos 4 tem `SDL_` fora de comentário — são zero infração, não um "quase". **Os dois números (36 do brief e 32 meu) estão corretos para o que cada um mediu**; a diferença é 100% o filtro de comentário, não erro de escopo. `tools/` já estava na minha medição desde o início (14 arquivos, contados à parte, seção 1) — não foi esquecido aqui. **Total real do diretório `app/` inteiro, produção+testes+tools, pós-filtro de comentário: 62 arquivos, 1452 ocorrências** (459 produção + 540 testes + 453 tools; produção corrigida de 457 para 459 em 2026-08-07, ver nota de correção no topo do documento). Uso **32 (produção real, shippada em `gusworld_app`)** como número-guia do resto do plano, porque é o que reflete o binário do jogo; `tests/` e `tools/` têm tratamento próprio na seção 6 (exceções), reforçado abaixo com a trava que a correção do team-lead pediu.

## 2. Números por categoria (produção — os 32 arquivos que shippam)

| categoria | ocorrências | arquivos | símbolos típicos |
|---|---|---|---|
| janela/contexto GL | 208 | ~20 | `SDL_Window`, `SDL_GLContext`, `SDL_GL_CreateContext/SetAttribute/MakeCurrent/DestroyContext/SwapWindow/GetProcAddress`, `SDL_CreateWindow`, `SDL_Init(VIDEO)` |
| eventos/loop | 94 | ~18 | `SDL_Event`, `SDL_EVENT_*`, `SDL_PollEvent` |
| diversos | 65 | ~14 | `SDL_GetError`, `SDL_BUTTON_*`, `SDL_Rect`, `SDL_DisplayID` |
| input | 39 | 9 | `SDL_Keycode`, `SDL_KMOD_*`, `SDL_GetMouseState` |
| log | 32 | 4 | `SDL_Log` |
| relógio | 17 | ~8 | `SDL_GetTicksNS`, `SDL_Delay` |
| render SDL | 4 | 1 | `SDL_Renderer`, `SDL_CreateWindowAndRenderer` |
| **total** | **459** | **32** | |

Os 32 arquivos, por peso (ver lista completa no fim, seção 9):

```
67  src/screens/battle_preview.cpp
53  src/screens/npc_dialogue_loop_gl.cpp
48  src/screens/system_menu_loop.cpp
46  src/maestro.cpp
44  src/sdl_window.cpp
40  src/screens/title_menu_loop.cpp
27  src/screens/anim_preview.cpp
20  src/screens/save_load_menu_loop.cpp
19  src/screens/battle_input.cpp
17  src/screens/difficulty_menu_loop.cpp
13  src/app_icon.cpp
 8  src/screens/save_load_menu.cpp        (100% categoria "input")
 6  src/screens/system_menu.cpp           (100% categoria "input")
 5  src/screens/title_menu.cpp            (100% categoria "input")
 5  src/screens/difficulty_menu.cpp       (100% categoria "input")
 + 17 headers (1-4 ocorrências cada, espelhando as assinaturas acima)
```

**Nenhum destes 32 é violação trivial** (confirma o achado do brief): todos usam mais do que log/relógio.

## 3. Achado que muda o cálculo: parte do "janela/contexto GL" já está MORTA, não é o motor do F4

Antes de perguntar "quanto a F4 mata", achei uma fatia que **já morreu e ninguém tirou o corpo** — não depende de F4 nenhuma, é limpeza pura.

`ADR-018` (aceito, 2026-07-17): unificar tudo em **um único contexto GL, criado 1x no boot pela `Maestro`**, vivo até o shutdown. As variantes de tela viram núcleos `_gl_current` que **assumem contexto já corrente** (zero `SDL_GL_CreateContext`/`DestroyContext` em transição de tela). Confirmei que isso é o caminho REAL hoje, lendo `src/maestro.cpp`: ela chama `run_title_menu_loop_gl_current` (`maestro.cpp:556`), `run_system_menu_loop_gl_current` (`:625`), `run_battle_preview_embedded_gl_current` (`:1100`), `run_npc_dialogue_loop_gl_current` (`:1200`) — **nunca** as variantes "owning"/"embedded" que criam contexto próprio.

Rastreei quem ainda chama essas variantes "owning" (grep completo em `src/`, `tests/`, `tools/`, `main.cpp`):

| função "owning" (cria/destrói contexto GL) | call-sites encontrados | veredito |
|---|---|---|
| `run_title_menu_loop_owning_gl` | **zero** (só a própria definição/declaração) | **código morto, seguro remover** |
| `run_system_menu_loop_owning_gl` | **zero** (só a própria definição/declaração) | **código morto, seguro remover** |
| `run_npc_dialogue_loop_gl` (a casca "owning" do diálogo) | 1: `app/tools/npcdlg_hover_probe.cpp` (projeto CMake standalone, não shippa) | morto pro binário do jogo; avisar quem mantém o probe antes de apagar |
| `run_battle_preview()` (a casca "embedded" que cria/destrói contexto por entrada em batalha) | 1: `main.cpp:242`, atrás da flag `--battle` (viewer de dev) | **não é morto** — é o modo standalone `--battle`; decisão separada (seção 6) |

Spot-check em `battle_preview.cpp` (função por função, contando tokens `SDL_` dentro de cada corpo): dos 67 tokens do arquivo, **38 vivem dentro de `run_battle_preview_embedded`/`run_battle_preview()`** (a casca "embedded" do `--battle`), só **1 no núcleo `_gl_current`** (o caminho real do jogo) e o resto espalhado em código de classe (`BattleScreen`). Ou seja: **mais da metade do peso deste arquivo é o viewer de dev, não o jogo**. Não tive tempo de fazer o mesmo corte cirúrgico nos outros 5 arquivos da família `ScreenState` (`title_menu_loop.cpp`, `system_menu_loop.cpp`, `difficulty_menu_loop.cpp`, `save_load_menu_loop.cpp`, `npc_dialogue_loop_gl.cpp`) — **fica marcado como "a confirmar arquivo por arquivo" na Fatia 0**, mas a convenção de nomes (`_owning_gl` existe em `title_menu_loop.hpp:261` e `system_menu_loop.hpp:328` também) e o resultado do grep de call-sites (zero) já bastam pra saber que **pelo menos os dois "owning" sem nenhum call-site inteiro (title, system) morrem sem análise adicional**.

## 4. Quanto a F4 mata vs quanto sobra em qualquer cenário

| categoria | total produção | morre garantido (F4 ou já-morto) | sobra em qualquer cenário | incerto/depende de decisão |
|---|---|---|---|---|
| log | 32 | 0 pela F4 | 0 | **32 — mata na Fatia 1 (decisão já tomada, zero relação com F4)** |
| relógio | 17 | 0 pela F4 | 0 | **17 — mata na Fatia 1 (idem)** |
| janela/contexto GL | 208 | ~48 já morto (Fatia 0: `sdl_window.cpp` inteiro entra em decommission no F4-4; `run_title/system_menu_loop_owning_gl` já mortos hoje) + o essencial de `maestro.cpp` (~21, F4-3 cria `glintfx::App` no lugar) | **0 garantido** — mesmo o `SDL_GL_SwapWindow`/`GetWindowSizeInPixels` chamado dentro do `tick()` de cada tela pode sobreviver se o glintfx App mode exigir isso do host (lacuna L-coabitação-GL ainda sem resposta) | **~139 (o resto): depende INTEIRAMENTE da lacuna de coabitação GL já pedida ao glintfx e ainda sem resposta** |
| eventos/loop | 94 | 0 confirmado | **o TIPO `SDL_Event` pode sobreviver inteiro** — ver seção 5 | **94 — depende de uma decisão de contrato que a F3 ainda não tomou explicitamente (ver seção 5)** |
| input | 39 | 0 pela F4 | `battle_input.cpp` (23, ver seção 6) fica de qualquer forma se não houver decisão de mover pra `platform/` | **21 morrem já (Fatia 2, independente da F4)** — as 4 telas de menu puro (`title_menu.cpp`, `difficulty_menu.cpp`, `system_menu.cpp`, `save_load_menu.cpp`) usam `SDL_Keycode` só como TIPO de parâmetro, e o F4-2 **já** criou a tabela neutra (Godot-Key) que resolve isso sem esperar F4-3/F4-4 |
| render SDL | 4 | 0 pela F4 (não é escopo da F4, é `anim_preview.cpp`, um viewer `--anim-preview`) | 4 (decisão do F4-4 sobre `render2d_sdl` não cobre este arquivo, que é um dev tool à parte) | pendente de decisão sobre o destino de `anim_preview.cpp` (seção 6) |
| diversos | 65 | reflete majoritariamente `SDL_GetError`/`SDL_BUTTON_*`/`SDL_Rect` amarrados às categorias acima — morre/sobra junto com elas | — | — |

**Leitura honesta:** a F4 sozinha **não** garante zerar `app/`. Ela mata com segurança pelo menos os ~48 de `sdl_window.cpp` + os ~21 de `maestro.cpp` relacionados a boot de janela/contexto — **~69 ocorrências, ~15% do total de produção**. O resto (janela/contexto GL residual nos 6 arquivos `ScreenState`, e o tipo `SDL_Event` do próprio contrato `ScreenState::handle_event`) depende de **duas perguntas ainda sem resposta**, uma já formalmente pendente do glintfx (coabitação GL) e outra que nem chegou a ser formulada como pergunta (o tipo do evento sobrevive ao cutover?) — ver seção 5.

## 5. A pergunta que falta fazer: `ScreenState::handle_event(const SDL_Event&)` sobrevive à F4-3?

`app/include/gus/app/screen_state.hpp` (o contrato que toda tela modal implementa desde F4-1) tem `SDL_Event` **na própria assinatura pública** (`handle_event(const SDL_Event& ev)`, `screen_state.hpp:80`) e dois `std::function` que já abstraem a FONTE do evento (`EventPollFn`, default `SDL_PollEvent`) e do relógio (`ClockNowNsFn`, default `SDL_GetTicksNS`) — mas o **TIPO** carregado por esses hooks continua sendo `SDL_Event`.

O comentário que fundou a F4-1 (`screen_state.hpp:14-17`) diz: *"o LOOP de cima muda quando a casca SDL virar o App mode do glintfx; o CONTRATO de tela NÃO muda"*. Isso foi escrito **antes** da F4-2 decidir, em 2026-07-24, que a leitura de tecla seria por **polling de nível** (`is_key_down` por quadro) em vez de edge-callback (`TODO.md` linha do item F4-2, "2ª decisão do líder"). Um modelo de polling não produz naturalmente uma fila de `SDL_Event` — então **não está claro se o F4-3 vai (a) sintetizar `SDL_Event` a partir do polling do glintfx só pra não quebrar a assinatura, o que seria gambiarra pra preservar um tipo que deixou de fazer sentido, ou (b) trocar o tipo de `handle_event` pra algo neutro (`gus::app::InputEvent` ou similar), o que é mudança de contrato real** e contradiz o comentário original.

**Isto não é uma decisão que este plano toma.** É arquitetura pura (tipo de uma API pública entre camadas) e cai direto na regra do projeto: nenhuma decisão de arquitetura sem `AskUserQuestion`. Vai para a seção 8 como item de decisão do líder, com a recomendação de resolver **antes** de começar a F4-3 (não durante) — decidir o tipo do contrato com o cutover parado é barato; descobrir no meio da implementação que o tipo precisa mudar é o "workaround que vira o next lacuna" que o líder já proibiu para o glintfx e que se aplica igualmente aqui, dentro de casa.

## 6. Exceções legítimas (e as que não são)

- **`app/tools/` (14 arquivos, 453 ocorrências): exceção legítima, mas SÓ com trava mecânica — o team-lead está certo em cobrar isso, e a violação ainda está crescendo, não é só estoque parado.** Prova disso: `app/tools/cockpit_hp_text_probe.cpp` foi **criado hoje mesmo** (commit `b0d9dcd`, o mesmo SHA usado pra medir este plano) e já nasceu com `#include <SDL3/SDL.h>` direto (27 ocorrências na minha contagem, ~18 na do team-lead — a diferença é só regex de categorização, o fato de o arquivo ser novo-hoje-com-SDL é o que importa). Ninguém instruiu o autor contra isso porque **a regra não estava travada em lugar nenhum** — exatamente o argumento "contra a exceção" do team-lead, e ele procede.

  **Critério verificável (não "é ferramenta", que é opinião — é caminho + estrutura de build, que é fato):** um arquivo está na exceção `tools/` **se e somente se** (a) o caminho está sob `app/tools/`, **e** (b) nem `app/CMakeLists.txt` nem o `CMakeLists.txt` raiz fazem `add_subdirectory(tools)` (hoje é o caso: `app/CMakeLists.txt:191` só tem `add_subdirectory(tests)`). A condição (b) é o que torna a exceção **real e não just-so** — um diretório "de ferramentas" que fosse linkado no binário não se qualificaria, não importa o nome.

  **A trava que impede a exceção de vazar (proposta de gate, execução fora deste doc):** `GATE(tools-isolation)` em `tools/check.sh` — `grep -n 'add_subdirectory(tools)' $ENGINE/CMakeLists.txt $ENGINE/app/CMakeLists.txt` **precisa dar VAZIO**. É um gate barato (uma linha de grep), preciso (não depende de contar ocorrência de SDL, só verifica a condição estrutural que sustenta a exceção inteira) e **hoje já passaria** — vira rede de segurança, não bloqueio novo. Sem essa trava, a exceção é uma alegação; com ela, é uma invariante vigiada.

  **O que acontece quando um probe vira produção:** no dia em que alguém precisar do código de um probe dentro do jogo de verdade (não como sonda descartável), a AÇÃO que dispara o contrato não é "mover o arquivo de pasta" — é **linká-lo em `gusengine_app`** (via `add_subdirectory` ou movendo o `.cpp` pra `app/src/`). A partir desse instante o arquivo deixa de estar coberto pela exceção e entra no teto de não-regressão da seção 8 como qualquer arquivo novo de produção — inclusive contando contra o ratchet, que só pode cair. Não existe promoção "silenciosa": o próprio `GATE(tools-isolation)` denuncia o `add_subdirectory(tools)` no instante em que alguém tentar.

  **O que NÃO muda com isso:** `tools/` continua podendo crescer livremente em número de arquivos/ocorrências de SDL — a exceção não impõe teto de crescimento ao conteúdo dela, só impõe que ela **nunca entre no link de produção** sem que isso seja notado. Achei essa diferença importante de deixar explícita: o problema que o team-lead apontou (crescimento sem vigilância) não se resolve limitando `tools/`, se resolve vigiando a fronteira entre `tools/` e o resto.
- **`app/tests/` (16 arquivos, 540 ocorrências): exceção parcial, com ressalva.** Testes de interação que constroem `SDL_Event` sintético pra injetar em `run_screen_state()` via `EventPollFn` são o próprio MECANISMO de teste headless que a F4-1 desenhou de propósito (ver `screen_state.hpp:120-125`) — enquanto o contrato de produção usar `SDL_Event`, os testes têm que usar o mesmo tipo pra exercitá-lo de verdade. **Se a seção 5 decidir trocar o tipo**, os testes acompanham automaticamente (não é uma exceção que sobrevive pra sempre — está amarrada à mesma decisão pendente).
- **`anim_preview.cpp`/`.hpp` (27+1 ocorrências): viewer de dev `--anim-preview`, cria a PRÓPRIA `SDL_Window`/`SDL_Renderer`** (não usa a janela da Maestro). Mesmo perfil do `--battle`: ferramenta interna, mas **dentro do binário shippado** (compilada em `gusengine_app`, ao contrário de `app/tools/`). Isso é uma exceção real ou uma violação disfarçada de "ferramenta interna"? **Decisão do líder** (seção 8) — não decido sozinho porque "ferramenta interna" no `CLAUDE.md` foi escrito pensando em `app/tools/`, não necessariamente em código que compila dentro do binário de produção.
- **`battle_input.cpp`/`.hpp` (23 ocorrências): candidato a mover pra `platform/`, não a "exceção".** Ele traduz `SDL_Event`/`SDL_Keycode`/mouse pra intenção de jogo — exatamente o papel que `platform/src/input/key_translation.cpp` já cumpre para o teclado geral. Pode ser genuinamente específico de batalha (verbos de combate) ou pode ser fronteira mal-colocada. **Não decido isso sozinho** (é exatamente o tipo de escolha que a régua do `docs/tech/glintfx-boundary.md` pede pra aplicar no planejamento, não no meio de uma fatia de limpeza) — vai para seção 8.
- **Não há exceção geral para `src/`/`include/gus/app/` fora dos dois casos acima.** O alvo para o restante da produção é **zero** SDL3 direto, com o destino de cada arquivo determinado pelas fatias 0-3.

## 7. Fatias ordenadas por dependência

| # | fatia | depende de | remove (produção) | risco | reversibilidade |
|---|---|---|---|---|---|
| **0** | Apagar `run_title_menu_loop_owning_gl` + `run_system_menu_loop_owning_gl` (confirmados sem NENHUM call-site); avaliar (não apagar sem checar) `run_npc_dialogue_loop_gl` (1 call-site em probe de `tools/`, avisar o dono antes) | nada — pode rodar hoje | ~42 ocorrências confirmadas (mais o que a análise arquivo-por-arquivo de `battle_preview.cpp`/`npc_dialogue_loop_gl.cpp` revelar ao aplicar o mesmo corte) | Baixíssimo — grep exaustivo já provou zero uso; reconferir de novo antes de aplicar (árvore concorrente) | Two-way door trivial (é `git revert` de uma remoção) |
| **1** | Relógio (`SDL_GetTicksNS`/`SDL_Delay` → `std::chrono::steady_clock`) + log (`SDL_Log*` → utilitário em `core/`) | nada — decisão já tomada pelo líder/CTO, não depende de F4 nem de glintfx | 49 ocorrências (32 log + 17 relógio), ~9-10 arquivos | Baixo — troca mecânica de implementação, comportamento observável idêntico | Two-way door |
| **2** | `SDL_Keycode` como TIPO de parâmetro nas 4 telas de lógica pura (`title_menu.cpp`, `difficulty_menu.cpp`, `system_menu.cpp`, `save_load_menu.cpp`) → tipo neutro já criado pelo F4-2 (tabela Godot-Key) | F4-2 (✅ já fechado, 🔍 pendente só de validação ao vivo pós-cutover, não bloqueia este uso) | ~21 ocorrências, 100% categoria "input" | Baixo — a tabela de tradução já existe e já foi testada adversarialmente na F4-2 | Two-way door |
| **3** | F4-2 fecha validação ao vivo → **F4-3 (cutover, ponto que decide a seção 5)** → **F4-4 (decommission, ponto de não-retorno)** | decisões do líder pendentes (F4-3/F4-4 já marcadas ⏳ no `TODO.md`) + resposta do glintfx à lacuna de coabitação GL + a decisão da seção 5 sobre o tipo de `handle_event` | ~139-208 ocorrências de janela/contexto GL + até 94 de eventos/loop, a depender EXATAMENTE de como a seção 5 for decidida | Alto — é o cutover real, `sdl_window.cpp` inteiro some, ponto de não-retorno por definição do próprio `TODO.md` (F4-4) | F4-3 é two-way door (paralelo, SDL ainda existe); F4-4 é **one-way door** por desenho |
| **4** | Decisão sobre `anim_preview.cpp` (viewer `--anim-preview`) e `battle_input.cpp` (mover pra `platform/`?) | decisão do líder (seção 8), pode rodar em paralelo com a 3 | 27 (anim_preview) + 23 (battle_input) = 50 ocorrências, SE o líder decidir mover/eliminar | Médio — mexe em fronteira de camada, precisa de teste de regressão | Depende da opção escolhida |

**Ordem recomendada: 0 → 1 → 2, em paralelo com a F4 já em andamento (3), com a 4 decidida a qualquer momento entre a 2 e a 3.** As fatias 0/1/2 não competem por arquivo com a F4 (F4 é dona de `app/screens/battle_*` e `platform/input/**`; as fatias 0-2 tocam `title_menu_loop`/`system_menu_loop`/os 4 `_menu.cpp` puros — checar sobreposição exata com quem estiver executando F4-2/F4-3 nesta sessão antes de começar, dado o número de agentes ativos agora).

## 8. Estratégia de gate

**Não ligar hoje um gate estrito sobre `app/` inteiro** — reprovaria as ~139-208 ocorrências que dependem da F4/glintfx e travaria o projeto sem que ninguém possa fazer nada a respeito (a mesma lição do `GATE(arch)` original: virou papel morto porque ninguém tinha como cumprir na hora que foi escrito, e por isso nunca foi estendido a `app/`).

**Proposta: três gates, escopos diferentes, entrando em fatias diferentes.**

0. **`GATE(tools-isolation)`, entra JÁ, hoje, independente de qualquer fatia.** `grep -n 'add_subdirectory(tools)' $ENGINE/CMakeLists.txt $ENGINE/app/CMakeLists.txt` precisa dar vazio (ver seção 6). É a trava que sustenta a exceção de `app/tools/` — sem ela, a exceção é uma alegação no papel, exatamente o que o team-lead cobrou. Custo de implementação: uma linha. Hoje já passaria (verificado nesta medição).
1. **Gate de categoria fechada (zero-tolerância), entra junto com a Fatia 1.** Assim que log/relógio saírem de `app/` de produção, adicionar ao `tools/check.sh` (fora do escopo deste plano — é execução, não documentação) uma checagem `grep -c 'SDL_Log\|SDL_GetTicksNS\|SDL_Delay' $ENGINE/app/src $ENGINE/app/include` (excluindo `tools/`) que **precisa dar zero**. Não é ratchet, é zero-tolerância imediata — mais simples que um teto numérico e sem ambiguidade, porque a decisão já foi tomada e não há transição a acomodar.
2. **Gate de teto de não-regressão (ratchet), entra HOJE, cobrindo as categorias ainda em fluxo (janela/contexto GL + eventos/loop + input residual).** Um número máximo de **arquivos infratores** em `app/src` + `app/include/gus/app` (excluindo `app/tests` e `app/tools`, pelos motivos da seção 6), começando em **32** (o número medido nesta seção) e só podendo CAIR — falha se subir. Prefiro contagem de ARQUIVOS a contagem de TOKENS como teto principal porque é mais fácil de auditar à mão num `git diff` de PR (basta ver se um arquivo novo passou a ter `#include <SDL3` fora de `platform/`), mas o script de medição deste plano (preservado no scratchpad, pode ser adaptado e commitado se o líder aprovar) dá o número exato de tokens por categoria pra quem quiser um ratchet mais fino depois.
   - **Por que ratchet e não zero direto:** a F4 ainda está em andamento e paralela a outras ondas (CARDS-HW-ENGINE também mexe perto); zero imediato bloquearia trabalho já em curso sem dar tempo de sequenciar. O ratchet deixa a baseline SÓ CAIR (Fatia 0 já a leva de 32 pra ~30; Fatia 2 não muda contagem de ARQUIVO se os 4 arquivos de menu continuarem tocando SDL só um pouco menos — por isso o ratchet de arquivo não capta toda melhoria; é o preço de escolher a métrica mais simples de auditar).
   - **Alternativa avaliada e descartada:** ratchet por token total. Mais sensível a progresso parcial, mas exige manter o script de contagem por categoria como parte do CI (mais superfície pra manter, mais um lugar pra "a ferramenta de verificação mentir" se o parser de comentário tiver um bug não descoberto). Para o estágio atual (poucas fatias, todas bem definidas), o ratchet por arquivo é suficiente e mais barato de auditar.
3. **Gate final estrito (zero-tolerância total sobre janela/contexto GL + eventos/loop em `app/src`+`app/include/gus/app`) entra depois do F4-4.** É o ponto em que `sdl_window.cpp` já não existe e o cutover é irreversível por desenho — nesse momento faz sentido subir o gate de "teto decrescente" para "zero", igual ao padrão que `GATE(arch)` já usa para `core/`/`domain/`. Antes disso, zero é prematuro pelas incertezas da seção 5 e da lacuna de coabitação GL ainda sem resposta do glintfx.

## 9. Riscos e reversibilidade (resumo por fatia)

- **Fatia 0:** risco baixíssimo, mas **a árvore está sendo editada por outros agentes agora** — reconferir os grep de call-site imediatamente antes de aplicar, não confiar no resultado deste documento se passarem mais que algumas horas.
- **Fatia 1:** risco baixo; único cuidado é o `ClockNowNsFn`/`EventPollFn` de teste que hoje podem depender do valor exato retornado por `SDL_GetTicksNS` em algum teste de timing — checar antes de trocar a implementação default.
- **Fatia 2:** risco baixo; a tabela de tradução do F4-2 já passou por mutation testing adversarial (registrado no `TODO.md`, F4-2.1/2.2, 8/8 mutantes mortos) — reuso maduro, não invenção nova.
- **Fatia 3 (F4-3/F4-4):** risco alto por definição (é o cutover); **gate-bloqueante explícito do líder já em vigor** ("workaround bloqueia a fatia" — se a lacuna de coabitação GL não for respondida pelo glintfx, a fatia PARA, não contorna). F4-4 é one-way door reconhecido no próprio `TODO.md`.
- **Fatia 4:** risco médio, depende inteiramente de qual opção o líder escolher para `anim_preview.cpp`/`battle_input.cpp`.

## 10. Decisões que ficam com o líder (não decidi nenhuma sozinho)

1. **O tipo de `ScreenState::handle_event`** sobrevive como `SDL_Event` (com alguma ponte sintética do glintfx) ou muda pra um tipo neutro `gus::app::InputEvent` no F4-3? (seção 5) — recomendo decidir ANTES de começar a implementar F4-3, não durante.
2. **`anim_preview.cpp` é exceção legítima de "ferramenta interna dentro do binário shippado" ou deveria virar um projeto standalone tipo `app/tools/` (fora do link de produção)?** (seção 6)
3. **`battle_input.cpp` é regra específica de jogo (fica em `app/`) ou é fronteira de tradução SDL→domínio que deveria morar em `platform/`, ao lado de `key_translation.cpp`?** (seção 6) — aplicar a régua do `docs/tech/glintfx-boundary.md` requer julgamento de "genérico vs específico" que não é meu para fazer sozinho.
4. **Ordem de execução das fatias 0-2 em relação às ondas já em curso hoje** (F4-2/F4-3, CARDS-HW-ENGINE) — este plano recomenda paralelismo, mas a decisão final de sequenciamento e quem executa cada fatia é do líder/CTO.
5. **Aprovação do próprio plano** antes de qualquer execução — nenhuma linha de código foi tocada para produzir este documento.

## Apêndice — lista completa dos 32 arquivos de produção (medição, ordenada por peso)

```
 67  src/screens/battle_preview.cpp                    {janela_gl:39 eventos:11 input:1 relogio:3 diversos:13}
 53  src/screens/npc_dialogue_loop_gl.cpp               {janela_gl:26 eventos:7 input:2 log:15 relogio:1 diversos:2}
 48  src/screens/system_menu_loop.cpp                   {janela_gl:26 eventos:14 input:2 relogio:1 diversos:5}
 46  src/maestro.cpp                                    {janela_gl:21 eventos:3 log:5 relogio:8 diversos:9}
 44  src/sdl_window.cpp                                 {janela_gl:29 eventos:3 log:7 relogio:1 diversos:4}
 40  src/screens/title_menu_loop.cpp                    {janela_gl:26 eventos:9 relogio:2 diversos:3}
 27  src/screens/anim_preview.cpp                       {janela_gl:7 eventos:4 render_sdl:4 diversos:12}
 20  src/screens/save_load_menu_loop.cpp                {janela_gl:6 eventos:10 input:1 diversos:3}
 19  src/screens/battle_input.cpp                       {janela_gl:2 eventos:10 input:2 diversos:5}
 17  src/screens/difficulty_menu_loop.cpp                {janela_gl:6 eventos:9 diversos:2}
 13  src/app_icon.cpp                                    {janela_gl:3 log:3 diversos:7}
  8  src/screens/save_load_menu.cpp                      {input:8}
  6  src/screens/system_menu.cpp                         {input:6}
  5  src/screens/title_menu.cpp                          {input:5}
  5  src/screens/difficulty_menu.cpp                     {input:5}
  4  include/gus/app/sdl_window.hpp                       {janela_gl:3 eventos:1}
  4  include/gus/app/screens/battle_input.hpp             {janela_gl:1 eventos:1 input:2}
  4  src/screen_state.cpp                                 {eventos:3 relogio:1}
  3  include/gus/app/screen_state.hpp                      {eventos:3}
  3  include/gus/app/screens/npc_dialogue_loop_gl.hpp       {janela_gl:2 input:1}
  3  include/gus/app/screens/title_menu_loop.hpp            {janela_gl:2 eventos:1}
  3  include/gus/app/screens/system_menu_loop.hpp           {janela_gl:2 eventos:1}
  3  include/gus/app/screens/battle_preview.hpp             {janela_gl:2 eventos:1}
  2  include/gus/app/maestro.hpp                            {janela_gl:2}
  2  include/gus/app/screens/difficulty_menu_loop.hpp       {janela_gl:1 eventos:1}
  2  include/gus/app/screens/save_load_menu_loop.hpp        {janela_gl:1 eventos:1}
  1  include/gus/app/app_icon.hpp                           {janela_gl:1}
  1  include/gus/app/screens/title_menu.hpp                 {input:1}
  1  include/gus/app/screens/difficulty_menu.hpp            {input:1}
  1  include/gus/app/screens/save_load_menu.hpp             {input:1}
  1  include/gus/app/screens/system_menu.hpp                {input:1}
  1  include/gus/app/screens/anim_preview.hpp               {eventos:1}
```

# Plano: contexto de vídeo único (fix do flash ao fechar menu/título)

- **Autor:** Caetano (CTO), 2026-07-17
- **Branch de trabalho:** `fix/menu-flash-contexto-unico` (main `b1a4e22` intocado; correção incerta fica isolada, por ordem do líder)
- **Status:** PLANO (nenhum código de produção alterado por este documento)
- **Item de backlog:** registrar via `/tab_pendencias` (sugestão de ID: `FLASH-CTX`)

## 1. Diagnóstico (fechado; dado de entrada)

A cidade desenha com `SDL_Renderer` 2D (driver "opengl", `Render2dSdl`) e cada tela modal (pausa/Salvar/Carregar/Config/título/diálogo/batalha) cria um contexto `SDL_GL_CreateContext` próprio (3.3 core + stencil 8) na MESMA janela, via glintfx/RmlUi GL3; a transição destrói um lado e cria o outro (`Maestro::open_pause_from_city` -> `capture_frame_to_png` -> `release_renderer`/`SDL_DestroyRenderer` -> menu -> `SDL_GL_DestroyContext` -> `reacquire_renderer`/`SDL_CreateRenderer`), e o "flash" (zoom/scale por 1-2 frames) acontece exatamente e somente ao CRIAR o `SDL_Renderer` (fechar menu e boot). Compositor KDE, 2ª janela, resize, driver, nosso render 2D e RCSS foram descartados com evidência.

**Achado adicional de leitura de código (reforça o diagnóstico, não o reabre):** no SDL3 vendorizado (`build/linux-release/_deps/sdl3-src/src/render/opengl/SDL_render_gl.c`, linhas 43-44 e 1793-1806), o backend "opengl" do `SDL_Renderer` pede GL **2.1 sem profile** e, se os atributos GL correntes do processo forem outros (o menu acabou de setar 3.3 core + stencil 8) ou a janela não tiver a flag `SDL_WINDOW_OPENGL`, ele chama **`SDL_ReconfigureWindow`**, que recria a superfície nativa da janela. Isso explica a assimetria provada: abrir o menu (`SDL_GL_CreateContext` numa janela já-GL) não reconfigura nada; fechar o menu (`SDL_CreateRenderer` com atributos 3.3 ainda setados) reconfigura a janela por baixo, e o compositor apresenta 1-2 frames de buffer velho/reescalado. INCERTO - validar empiricamente no branch (basta logar `changed_window`, ou testar `SDL_GL_ResetAttributes()` antes do reacquire como experimento de confirmação, não como fix).

## 2. Objetivo

Um único contexto de vídeo, vivo do boot ao shutdown, servindo cidade E menus/diálogo/batalha. Sem `SDL_DestroyRenderer`/`SDL_CreateRenderer` nem `SDL_GL_CreateContext`/`SDL_GL_DestroyContext` nas transições. Sem troca -> sem flash.

## 3. As três opções

### Opção A - menus reusam o contexto GL interno do SDL_Renderer

Manter o `SDL_Renderer` da cidade vivo; ao abrir menu, `SDL_FlushRenderer` e desenhar o glintfx/RmlUi por cima, no contexto GL que o próprio `SDL_Renderer` criou.

- **Viabilidade:** frágil. Fatos verificados no fonte do SDL3 vendorizado:
  - Não existe propriedade pública que exponha o contexto GL do renderer (`SDL_GetRendererProperties` só expõe D3D/Vulkan/GPU; nenhum `SDL_PROP_RENDERER_OPENGL_*` de contexto). O caminho seria `SDL_GL_GetCurrentContext()` logo após `SDL_CreateRenderer` (o backend faz make-current na criação). INCERTO - validar.
  - O contexto do renderer é **GL 2.1 sem profile e sem stencil** (`RENDERER_CONTEXT_MAJOR/MINOR = 2/1`; nenhum `SDL_GL_STENCIL_SIZE` é pedido). O RmlUi GL3 (dentro da glintfx) usa GLSL 330 e stencil para clip mask. Em Mesa/NVIDIA um pedido 2.1 costuma devolver um contexto compatibility 4.x onde GLSL 330 compila, mas isso é comportamento de driver, não garantia; e stencil 0 no framebuffer default quebraria clipping com border-radius/transform. INCERTO - validar por driver. Pré-setar atributos antes do `SDL_CreateRenderer` pode ou não ser respeitado (o backend sobrescreve para 2.1 quando difere). 
  - Interop de estado: `SDL_FlushRenderer` existe para isso, mas o cache de estado GL interno do `SDL_Renderer` fica obsoleto após GL cru; o SDL não expõe "invalidate state". Artefatos intermitentes são o modo de falha típico.
- **Esforço:** M (protótipo pequeno, mas a validação por driver/plataforma é o custo real).
- **Risco:** ALTO. Empilha três incertezas de driver e amarra o jogo ao backend "opengl" do `SDL_Renderer` também no Windows (onde o default é D3D11 - teríamos que forçar opengl lá, contra a corrente da plataforma).
- **Impacto glintfx:** provável pedido de um "modo compat/sem stencil" no backend GL3 dela (mudança NA lib) caso o contexto 2.1-compat não sirva como está. Ou seja: a opção mais frágil é também a única que provavelmente exigiria mexer na glintfx.

### Opção B - menus via backend SDL_Renderer do RmlUi (RmlUi_Renderer_SDL)

Manter o `SDL_Renderer` vivo e desenhar o menu com o renderer SDL que o RmlUi upstream oferece.

- **Viabilidade:** tecnicamente existe no upstream do RmlUi, mas significa **abandonar a glintfx nos menus**: a glintfx é a integração RmlUi do projeto (ADR-010) e é GL3 por construção (efeitos/shaders são a razão de existir dela; o renderer SDL do RmlUi não suporta os efeitos avançados de shader). Todos os loops de menu já são escritos contra `glintfx::UiLayer` (load/update/render/process_event/hover/click/scroll) - seria reescrever a integração de UI dos menus contra RmlUi cru, mantendo DUAS pilhas de UI no jogo (menus sem glintfx, HUD de batalha com glintfx). Alternativa "pedir um backend SDL_Renderer à glintfx" é um pedido tamanho G na lib para um resultado pior (sem efeitos).
- **Esforço:** G (reescrita da camada de UI dos menus + manutenção dupla).
- **Risco:** médio de execução, alto de produto (perda de capacidade visual, bifurcação da stack de UI).
- **Impacto glintfx:** nenhum (se abandonarmos a lib nos menus) ou um backend novo inteiro (se pedirmos). Ambos ruins.
- **Veredito:** rejeitada.

### Opção C - unificar tudo em GL (cidade migra Render2dSdl -> Render2dGl3) [RECOMENDADA]

Um contexto GL 3.3 core + stencil 8 criado UMA vez no boot pela Maestro (janela criada com `SDL_WINDOW_OPENGL`), vivo até o shutdown. A cidade desenha com `Render2dGl3`; menus/diálogo/batalha criam `glintfx::UiLayer` nesse mesmo contexto corrente e compõem por cima; um único `SDL_GL_SwapWindow` por frame, de quem for o dono do frame.

- **Viabilidade:** alta, com as peças já existentes e provadas no próprio repo:
  - `Render2dGl3` implementa o `IRenderer` COMPLETO (mesma interface do `Render2dSdl`: `load_texture`, `draw_text`, `texture_content_bbox`, defer/present, modo headless para CI) e já é o backend da arena de batalha em produção. O `OverworldSim` fala só com `IRenderer` - a cidade não sabe qual backend a desenha.
  - A batalha já prova a coexistência exata que precisamos: `Render2dGl3` (arena) + `glintfx::UiLayer` (HUD) no MESMO contexto, um swap único (ADR-009/ADR-010).
  - A glintfx foi desenhada para isso: `UiLayer` "attaches the UI+effects engine to a GL context the HOST owns", render compose-only, `load_gl` idempotente por processo (`ui_layer.hpp` do pin vendorizado). Criar/destruir `UiLayer` repetidamente num mesmo contexto já acontece hoje (pausa -> save/load -> pausa recria o UiLayer sem recriar contexto).
  - O padrão de refactor dos loops já existe: `run_system_menu_loop_gl_current` (núcleo agnóstico de contexto) vs `run_system_menu_loop_owning_gl` (casca que cria/destrói). Falta só extrair a mesma casca dos outros loops.
- **Ganho estrutural além do flash:** apaga a máquina `release_renderer`/`reacquire_renderer`/recarga de texturas/`hold_frozen_frame`, que já produziu 3 bugs reais (SIGSEGV do LOAD com renderer solto, Gus andando sozinho por input preso, o próprio flash + máscara). Texturas passam a sobreviver às transições. E alinha com o porte Windows: GL 3.3 uniforme, sem depender do backend "opengl" do `SDL_Renderer` (que no Windows não é o default).
- **Esforço:** M. Toca ~6-10 arquivos de `app/` + 1 helper em `platform/` (captura via `glReadPixels`, se mantivermos o fundo congelado em PNG). Zero mudança em `core/`/`domain/`.
- **Risco:** médio-baixo. Modos de falha prováveis: diferença sutil de rasterização SDL vs GL na cidade (cores/blend/nearest - verificável por screenshot headless), vazamento acumulado em ciclos longos de criar/destruir `UiLayer` num contexto que agora nunca morre (coberto por soak + ASan local), e a captura do fundo congelado que hoje usa `SDL_RenderReadPixels` (reimplementar com `glReadPixels`). Nenhum é de driver.
- **Impacto glintfx:** **nenhum pedido necessário** (ver §6).
- **Nota sobre a batalha:** a batalha também sofre a mesma costura ao voltar (o retorno cria `SDL_Renderer`), mas o boot pixelizado (`kFromBattleRevealing`) cobre a janela temporal inteira - por isso o flash não aparece lá. Com a Opção C a costura deixa de existir também nesse caminho (o overlay vira só estética, não máscara).

## 4. Recomendação do CTO

**Opção C.** É a única que remove a causa (a troca de contexto) em vez de contorná-la; reusa três coisas já provadas em produção no próprio jogo (Render2dGl3 na arena, UiLayer em contexto do host, o split `_gl_current`/`_owning_gl` do system menu); não exige nenhuma mudança na glintfx; simplifica o Maestro apagando a máquina release/reacquire que é fonte recorrente de bugs; e é o único caminho coerente com o porte Windows. A Opção A economizaria o refactor da cidade mas ao preço de três incertezas de driver empilhadas e de provável mudança na glintfx - exatamente o tipo de "correção incerta" que o líder mandou isolar. A B perde o produto (efeitos) para ganhar nada.

Trade-off assumido na C: perde-se o backend `SDL_Renderer` da cidade (e a comodidade de `SDL_SetRenderVSync`/`SDL_RenderReadPixels`), ganhos triviais de repor em GL (`SDL_GL_SetSwapInterval(1)`, `glReadPixels`). `Render2dSdl` NÃO é apagado neste branch (classe e testes ficam; decisão de remoção definitiva é do M9, com o líder).

## 5. Passos concretos (Opção C, neste branch)

Ordem pensada para ter prova de conceito cedo e reversibilidade a cada passo. Implementação por agentes especialistas (engine-graphics-programmer para os passos 2-4, gameplay/app para 5-6, qa-engineer para 7), briefados pela thread principal; cada commit cita o ID do item do TODO.

1. **POC (prova antes do refactor):** probe headless (padrão dos `app/tools/*_probe.cpp` existentes, Xvfb) que: cria janela `SDL_WINDOW_OPENGL` + contexto 3.3 core/stencil 8, desenha a cidade com `Render2dGl3`, abre e fecha `run_system_menu_loop_gl_current` N vezes SEM destruir o contexto, capturando PNG dos frames pós-fechamento. Critério: zero frame divergente (comparação por hash/pixel) e cidade visualmente idêntica à referência SDL. Se o POC reprovar, paramos aqui com o main intacto.
2. **Boot único de GL na Maestro:** janela criada com `SDL_WINDOW_OPENGL`; `SDL_GL_SetAttribute` (3.3 core, doublebuffer, stencil 8) UMA vez antes de criar o contexto; `SDL_GL_CreateContext` + `SDL_GL_MakeCurrent` + `gl3_load_functions` + `SDL_GL_SetSwapInterval(1)` UMA vez no init. O contexto vive no Maestro até o dtor.
3. **Cidade em GL:** `SdlWindow` troca `Render2dSdl` por `Render2dGl3` (membro via `IRenderer`; construção com `gl_active=true`). O swap do frame da cidade vira `SDL_GL_SwapWindow` no fim de `step`/`step_with_fade`/`render_dialogue_overlay_frame` (o `Render2dGl3::present()` não faz swap por design - o dono do frame faz; mesma receita da batalha). `SDL_SetRenderVSync` sai (o SetSwapInterval do passo 2 cobre).
4. **Fundo congelado:** `capture_frame_to_png` reimplementado com `glReadPixels` (helper pequeno em `platform/`, ou dentro do fluxo do `Render2dGl3` com present diferido, mesma janela temporal de leitura antes do swap). Alternativa mais simples a avaliar durante a implementação: com contexto único, o menu pode simplesmente redesenhar a cidade parada por trás da UI a cada frame (como a batalha desenha a arena sob o HUD) e o PNG intermediário morre; decisão local do implementador com aprovação do líder se mudar o visual em algo perceptível.
5. **Transições sem morte de contexto:** `open_pause_from_city`, `show_title_screen`, diálogo do NPC e `to_battle` deixam de chamar `release_renderer`/`reacquire_renderer`/`hold_frozen_frame`; os loops passam a ser chamados nas variantes `_gl_current` (system menu já tem; extrair a mesma casca de `title_menu_loop`, `npc_dialogue_loop_gl` e `battle_preview` - hoje todos criam contexto próprio). Restrições preservadas: nunca 2 `UiLayer` vivos ao mesmo tempo (limite RmlUi já documentado); `clear_input` nas bordas dos modais continua.
6. **Poda:** `release_renderer`/`reacquire_renderer`/`hold_frozen_frame` e as recargas de textura associadas (`load_player_sprites`/`load_enemy_marker_texture`/etc. nos pontos de reacquire) viram código morto; remover as CHAMADAS neste branch e marcar os métodos como deprecated (remoção física fica pro M9, decisão do líder). Atualizar comentários que afirmam "troca de backend na mesma janela" (ADR-012 ganha adendo, ou ADR novo curto referenciando este plano).
7. **Verificação (gate antes de mostrar ao líder):** suíte `ctest` completa; probes headless existentes (sysmenu/save_load/frozen_bg/npcdlg) re-rodadas; ASan local (runner local-first, política do projeto); soak de abrir/fechar menu 100+ ciclos observando RSS (vazamento de UiLayer/contexto-longevo); screenshots antes/depois comparadas por `qa-engineer` independente do implementador. Só depois: playthrough ao vivo do líder no branch. Merge ao main SÓ com autorização explícita do líder.

Pontos INCERTO - validar durante a implementação: (a) `Render2dGl3::begin_frame` como dono de frame full-screen da cidade (clear/viewport idênticos ao uso da batalha); (b) comportamento de resize/maximizar da cidade em GL (a batalha já mapeia lógico->pixels reais com a mesma `viewport_transform`, deve transpor 1:1); (c) custo de criar/destruir `UiLayer` por abertura de menu num contexto longevo (esperado barato; medir no soak).

## 6. Pedido ao dev da glintfx

**Nenhum pedido necessário para a opção recomendada.** A fachada embed atual (`UiLayer` anexando a contexto GL corrente do host, render compose-only, `load_gl` idempotente) é exatamente o contrato que a Opção C consome, e o uso é o mesmo já provado no HUD da batalha.

**Contingência:** se o POC (passo 1) ou o soak (passo 7) revelarem conflito de estado GL entre `Render2dGl3` e o `UiLayer` no mesmo contexto (improvável - a batalha já os compõe hoje) ou vazamento interno em ciclos longos de criar/destruir `UiLayer` num contexto que nunca morre, aí sim abrimos pedido ao dev da glintfx com o repro mínimo anexado. Por determinação do líder, **qualquer trabalho na glintfx decorrente deste plano deve ser feito num BRANCH da glintfx** (mesmo racional deste branch aqui: não arriscar o que funciona por uma correção incerta), com tag/pin bump só depois do branch validado contra o nosso POC. Nós não editamos a glintfx em hipótese alguma; só enviamos o pedido com repro (política `feedback_glintfx_nao_mexer_so_pedir`).

## 7. Referências de código (evidência deste plano)

- `GusEngine/app/src/maestro.cpp` (479-569 `open_pause_from_city`; 413-477 `show_title_screen`; 815-875 `to_battle`/retorno)
- `GusEngine/app/src/sdl_window.cpp` (214-243 release/reacquire; 412-430 `hold_frozen_frame`; 432-490 `capture_frame_to_png`)
- `GusEngine/app/src/screens/system_menu_loop.cpp` (1476-1519 casca owning vs núcleo `_gl_current`; 363-390 UiLayer + `Render2dGl3` backdrop no mesmo contexto)
- `GusEngine/platform/include/gus/platform/render2d/i_renderer.hpp` e `render2d_gl3.hpp` (paridade completa de interface)
- SDL3 vendorizado: `src/render/opengl/SDL_render_gl.c` 43-44, 1793-1806 (`RENDERER_CONTEXT_MAJOR/MINOR` 2.1 + `SDL_ReconfigureWindow`); `include/SDL3/SDL_render.h` (ausência de propriedade de contexto GL do renderer)
- glintfx vendorizada (pin atual): `glintfx/include/glintfx/ui_layer.hpp` (contrato embed/host-owned context)

## 8. Plano de execução (workgroup)

- **Autor:** Caetano (CTO), 2026-07-17. POC (passo 1) PASSOU e foi re-verificado pelo orquestrador; líder autorizou o refactor de produção neste branch.
- **Modelo de operação:** o CTO planeja (este §8); a THREAD PRINCIPAL (orquestrador) dispara os agentes e re-verifica cada gate. Agentes NÃO fazem push (política 2026-07-07); todo commit cita `FLASH-CTX` e toca o `Status` do item no `TODO.md` no mesmo commit (implementação entregue = `🔍 Pendente verificação`).
- **Pre-flight do orquestrador (antes do 1º agente):** (a) confirmar item `FLASH-CTX` no `TODO.md` (criar via `/tab_pendencias` se faltar); (b) confirmar branch `fix/menu-flash-contexto-unico` ativo e main `b1a4e22` intocado; (c) builds pesados/ASan com `TMPDIR=/var/tmp` (tmpfs OOM, política 2026-07-16).

### 8.1 Veredito de paralelismo: SEQUENCIAL, 1 agente por vez (4 agentes no total)

Quase todos os passos 2-6 tocam `maestro.cpp` e/ou `sdl_window.cpp` (compartilhados) — paralelo é proibido neles. O único bloco genuinamente disjunto (extração das cascas `_gl_current` nos arquivos de loop, que são disjuntos entre si e de maestro/sdl_window) até PODERIA rodar em paralelo com o bloco GL, mas ambos os agentes iteram build+ctest continuamente e a regra é 1 build pesado por vez — o ganho de wall-clock não paga a contenção nem o risco. Decisão: **cadeia sequencial A2 → A1 → A3 → A4 (QA)**. O líder autorizou "até 4"; usamos exatamente 4, um por vez.

A inversão deliberada (extração ANTES do boot GL) é a principal mitigação de risco: com as cascas `_gl_current` prontas e provadas equivalentes ainda no mundo SDL, a janela em que o branch fica "meio migrado" encolhe (ver §8.5).

### 8.2 Briefs de agente (na ordem de disparo)

**A2 — `gameplay_engineer` — "Extração das cascas `_gl_current`" (pré-requisito do passo 5; behavior-preserving)**
- **Escopo (arquivos):** `app/src/screens/title_menu_loop.cpp` + `.hpp`, `app/src/screens/npc_dialogue_loop_gl.cpp` + `.hpp`, `app/src/screens/battle_preview.cpp` + `.hpp`; VERIFICAR e incluir se também criarem contexto próprio: `save_load_menu_loop.cpp`, `difficulty_menu_loop.cpp`, `boot_pixel_overlay.cpp` (todos aparecem no grep de `SDL_GL_CreateContext`). **PROIBIDO tocar** `maestro.cpp`, `sdl_window.cpp`, qualquer coisa de `platform/`.
- **O que muda:** replicar o padrão já existente em `system_menu_loop.cpp` (1476-1519): extrair de cada loop um núcleo `_gl_current` (assume contexto GL corrente, não cria nem destrói nada) e manter a casca `_owning_gl` atual como wrapper fino que cria contexto → chama o núcleo → destrói. ZERO mudança de comportamento; os call-sites (maestro) continuam chamando as cascas owning.
- **Critério de pronto:** build limpo; `ctest` completo verde; probes de menu existentes (sysmenu/npcdlg/save_load/frozen_bg) re-rodadas com resultado idêntico ao pré-refactor; `git diff --stat` confinado aos arquivos do escopo; commit(s) citando `FLASH-CTX`.

**A1 — `engine-graphics-programmer` — "Boot único GL + cidade em GL + captura" (passos 2, 3 e 4 do §5)**
- **Escopo (arquivos):** `app/src/maestro.cpp` + `maestro.hpp` (init/dtor do contexto, `SDL_WINDOW_OPENGL`, atributos 3.3 core/doublebuffer/stencil 8, `gl3_load_functions`, `SDL_GL_SetSwapInterval(1)`); `app/src/sdl_window.cpp` + `.hpp` (membro `IRenderer` vira `Render2dGl3` com `gl_active=true`; swap via `SDL_GL_SwapWindow` no fim de `step`/`step_with_fade`/`render_dialogue_overlay_frame`; `capture_frame_to_png` via `glReadPixels`); se necessário, 1 helper pequeno novo em `platform/` (leitura de pixels GL). **PROIBIDO tocar** os arquivos de loop do A2.
- **Ponte temporária obrigatória (mantém o branch bootável):** neste bloco as transições AINDA chamam as cascas owning (que criam contexto GL próprio por cima do contexto da Maestro — múltiplos contextos na mesma janela são válidos). O A1 deve: (a) neutralizar `release_renderer`/`reacquire_renderer` com guard no modo GL (no-op seguro, sem destruir nada), (b) restaurar `SDL_GL_MakeCurrent(window, ctx_maestro)` ao voltar de qualquer casca owning. O flash pode AINDA existir nesta etapa (a troca de contexto menu-side persiste até o A3) — isso é esperado e não reprova o gate.
- **Decisão em aberto (passo 4, alternativa "redesenhar a cidade sob a UI" vs PNG congelado):** é mudança potencialmente visível — o agente PARA e reporta as opções ao orquestrador, que pergunta ao líder via AskUserQuestion (regra canônica do projeto). Default se o líder não for consultável: manter PNG congelado via `glReadPixels` (paridade visual estrita).
- **Critério de pronto:** build limpo; `ctest` verde; POC `poc_gl_single_context_probe` re-rodado passando; cidade headless byte-comparável à referência GL do POC (tolerância só o delta de rasterização já medido, 0.21% vs SDL); jogo boota e roda a cidade sob Xvfb; commits citando `FLASH-CTX`; INCERTOS (a) e (b) do §5 validados e reportados (begin_frame full-screen; resize/maximizar).

**A3 — `engine-graphics-programmer` (novo agente, mesmo perfil) — "Transições sem morte de contexto + poda" (passos 5 e 6)**
- **Escopo (arquivos):** `app/src/maestro.cpp` (`open_pause_from_city`, `show_title_screen`, diálogo NPC, `to_battle`/retorno passam a chamar as variantes `_gl_current` do A2; remover chamadas a `release_renderer`/`reacquire_renderer`/`hold_frozen_frame` e as recargas de textura dos pontos de reacquire); `app/src/sdl_window.cpp` + `.hpp` (marcar os métodos da máquina release/reacquire como deprecated — remoção física só no M9, decisão do líder); remover a ponte temporária do A1; comentários/ADR (adendo ao ADR-012 ou ADR novo curto referenciando este plano). Cobrir TAMBÉM o caminho da batalha (`to_battle`/`kFromBattleRevealing` via `boot_pixel_overlay`) — a costura deixa de existir lá igualmente (§3, nota).
- **Restrições preservadas:** nunca 2 `UiLayer` vivos simultaneamente; `clear_input` nas bordas dos modais continua.
- **Critério de pronto:** build limpo; `ctest` verde; POC passando; `grep -rn "release_renderer\|reacquire_renderer\|hold_frozen_frame" app/src` retorna SÓ as definições deprecated (zero call-sites); todas as probes headless existentes verdes; smoke-soak de 20 ciclos abre/fecha menu sem crash; commits citando `FLASH-CTX`.

**A4 — `qa-engineer` — "Verificação independente" (passo 7; ver §8.4)** — independente dos implementadores, por regra do projeto.

### 8.3 Gates de re-verificação do orquestrador (entre agentes; relatório de agent não é prova)

Cada gate é executado PELO ORQUESTRADOR, com comandos objetivos, antes de disparar o agente seguinte. Reprovou = volta pro mesmo agente com o diff do problema, não avança.

- **G1 (após A2):** `cmake --build --preset linux-release` limpo; `ctest --preset linux-release` completo verde; re-rodar as probes de menu (sysmenu/npcdlg/save_load/frozen_bg) e comparar com o resultado pré-A2 (extração é behavior-preserving: diferença = reprova); `git diff --stat` do range de commits confinado ao escopo declarado; commits citam `FLASH-CTX`.
- **G2 (após A1):** build + `ctest`; `poc_gl_single_context_probe` re-rodado do zero (não aceitar log antigo); screenshot headless da cidade comparada à referência; boot do jogo sob Xvfb com entrada na cidade (spot-check manual de 1 min); teste manual de resize/maximizar (INCERTO b); diff confinado; nenhuma edição nos arquivos do A2.
- **G3 (após A3):** build + `ctest`; POC; o grep de call-sites zerados (comando do critério do A3, rodado pelo orquestrador); todas as probes; smoke-soak 20 ciclos; verificação POSITIVA do fix: rodar o jogo sob Xvfb, abrir/fechar menu capturando os frames pós-fechamento e confirmar zero frame divergente (o mesmo critério do POC, agora no jogo real); `git log` confirma zero push (política: push só com ordem do líder).
- **G4 (após A4):** re-conferir o relatório do QA (o QA também erra — precedente do falso-negativo de sprite): re-rodar por amostragem 1 probe + abrir e OLHAR os PNGs do comparativo antes/depois; só então apresentar ao líder.

### 8.4 Critério de "pronto pro passo 7" e escopo do QA (A4)

**Pronto pro passo 7 =** G1+G2+G3 todos verdes, branch com jogo bootável, zero call-site da máquina release/reacquire, POC e probes verdes, nenhum INCERTO do §5 pendente de decisão (decisões novas já resolvidas pelo líder via AskUserQuestion).

**O `qa-engineer` valida (executando, não lendo):**
1. Suíte `ctest` completa + TODAS as probes headless (sysmenu/save_load/frozen_bg/npcdlg + POC), do zero.
2. **Soak 100+ ciclos** de abrir/fechar menu (e título→cidade→pausa→save/load→batalha-preview alternados) observando RSS ao longo do tempo — critério: RSS estabiliza (sem crescimento monotônico = sem vazamento de `UiLayer`/contexto longevo; INCERTO c do §5 medido e reportado com números).
3. **ASan local** (runner local-first, `TMPDIR=/var/tmp`): boot + ciclo de menus + batalha ida-e-volta, zero findings.
4. **Comparativo visual before/after:** screenshots da cidade/menus no main `b1a4e22` vs branch, comparadas por pixel; delta esperado só o de rasterização SDL→GL já medido (0.21%); qualquer regressão visual perceptível = achado. O QA OBSERVA os PNGs (regra 2026-07-16: observação visual é do qa-engineer, nunca do líder).
5. **Verificação do sintoma-alvo:** captura de frames na janela exata do fechar-menu (o repro original) provando zero flash.
6. Relatório pass/fail item a item com repro mínimo de qualquer achado.

**Depois do G4:** playthrough ao vivo do LÍDER no branch (paridade sentida na mão dele) → só com aprovação explícita dele: merge ao main (push/merge = autorização explícita, nunca implícita).

### 8.5 Riscos de sequência e como a ordem os mitiga

1. **Janela "meio migrada" (cidade GL, transições ainda velhas):** é o risco central — entre o passo 3 e o 5 as transições referenciam uma máquina de `SDL_Renderer` que não existe mais. Mitigações: (a) A2 ANTES de A1 deixa as cascas `_gl_current` prontas e provadas antes de qualquer mudança de backend; (b) a ponte temporária do A1 (guards + make-current restore) mantém o jogo bootável em TODO commit — nenhum commit do branch pode deixar o boot quebrado; (c) o flash residual durante a janela A1→A3 é esperado e documentado, não é regressão.
2. **`SDL_CreateRenderer` fantasma na janela já-GL:** durante a janela do A1, nada pode mais criar `SDL_Renderer` na janela (o `SDL_ReconfigureWindow` do §1 destruiria a superfície sob o contexto da Maestro). O guard do A1 cobre os caminhos conhecidos; o G2 inclui o boot+cidade+abrir menu como prova.
3. **Caminho da batalha esquecido:** `to_battle`/retorno e `boot_pixel_overlay` também criam contexto — se o A3 não cobrir, sobra uma troca de contexto escondida sob o overlay pixelizado (sem sintoma visível, mas com a máquina morta meio-viva). O grep do G3 pega (zero call-sites é critério objetivo, não confiança).
4. **Decisão visual do passo 4 tomada por agente:** proibido — a alternativa "redesenhar cidade sob a UI" muda visual potencialmente perceptível; brief do A1 manda PARAR e subir pro líder (regra canônica do projeto).
5. **Vazamento só visível no longo prazo:** criar/destruir `UiLayer` num contexto que nunca morre é padrão novo; o smoke-soak de 20 ciclos no G3 pega o grosseiro, o soak de 100+ do A4 com RSS pega o lento. Se vazar DENTRO da glintfx: não mexemos na lib — repro mínimo + pedido ao dev com branch na glintfx (§6, contingência).
6. **Poda agressiva demais:** remoção FÍSICA de `Render2dSdl`/máquina release-reacquire é decisão do M9 com o líder (§4); neste branch é só deprecated + zero call-sites. O G3 confere que as definições continuam existindo (build dos testes de `Render2dSdl` segue verde).

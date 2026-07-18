# ADR-018 — Contexto GL único (cidade+menus+diálogo+batalha), fecha o flash ao fechar o menu

**Status:** Aceito e implementado (branch `fix/menu-flash-contexto-unico`; workgroup A2→A1→A3, ver §8 do plano). Merge ao `main` só com autorização explícita do líder.
**Data:** 2026-07-17.
**Decisores:** Caetano (CTO, plano técnico) + líder (autorização do refactor de produção). Item `FLASH-CTX` do `TODO.md`.
**Plano completo (diagnóstico + as 3 opções + workgroup):** `docs/tech/pivot/menu-flash-contexto-unico-plano.md`.
**Cross-ref:** [ADR-009](ADR-009-rmlui.md) (adendo GL3 — origem do backend `Render2dGl3`/RmlUi-GL3 já provado na arena de batalha), [ADR-010](ADR-010-adopt-glintfx-embed-mode.md) (glintfx `UiLayer` embed, "anexa a um contexto GL que o HOST possui" — o contrato que este ADR consome sem mudança na lib), [ADR-012](ADR-012-m7-paridade-jogavel-plano.md) (Maestro/M7-COSTURA — descreve o swap cidade↔batalha na MESMA janela; **este ADR substitui a técnica concreta usada por baixo desse swap**, não o contrato do Maestro em si, que continua "contra quem"/"ganhou-perdeu").

## Contexto

Playtest ao vivo do líder (2026-07-17, filho dele, 11 anos) flagrou um flash (zoom/scale por 1-2 frames) **assimétrico: só ao FECHAR** o menu de pausa/Salvar/Carregar/Config/título, nunca ao abrir nem no boot. Diagnóstico fechado por depuração sistemática + leitura do fonte do SDL3 vendorizado (`SDL_render_gl.c`): a cidade desenhava com `SDL_Renderer` 2D (`Render2dSdl`, backend "opengl") e cada tela modal criava um contexto `SDL_GL_CreateContext` **próprio** (3.3 core + stencil 8) na MESMA janela — a transição destruía um lado e criava o outro. Criar o `SDL_Renderer` "opengl" com os atributos GL 3.3 ainda setados pelo menu (ele pede GL 2.1 sem profile) faz o backend chamar `SDL_ReconfigureWindow` internamente, recriando a superfície nativa da janela — daí o flash, e daí a assimetria (só ao voltar pro `SDL_Renderer`, nunca ao criar um contexto GL puro por cima de uma janela já-GL).

## Decisão

**Opção C do plano — unificar tudo em OpenGL.** Um único contexto GL 3.3 core/stencil 8, criado UMA vez pela `Maestro` no boot (`SDL_WINDOW_OPENGL` desde a criação da janela) e vivo até o shutdown. A cidade migrou de `Render2dSdl` para `Render2dGl3` (mesmo backend que a arena de batalha já usava desde o ADR-009); menus, diálogo, título e batalha desenham **direto** nesse contexto persistente via variantes `_gl_current` (núcleo que assume contexto já corrente, sem criar/destruir nada) — não mais via cascas `_owning_gl` (que criavam um contexto GL próprio por cima do da Maestro). Zero `SDL_CreateRenderer`/`SDL_DestroyRenderer`/`SDL_GL_CreateContext`/`SDL_GL_DestroyContext` durante qualquer transição de tela; só no boot (`Maestro::init()`) e no shutdown (`Maestro::~Maestro()`).

As opções A (menus reusando o contexto GL interno do `SDL_Renderer`) e B (backend `SDL_Renderer` do RmlUi upstream, abandonando a glintfx nos menus) foram avaliadas e rejeitadas no plano (§3) — A empilhava três incertezas de driver (GL 2.1 sem stencil do `SDL_Renderer`, cache de estado GL obsoleto, portabilidade Windows), B custava a capacidade visual da glintfx (efeitos/shaders) sem necessidade. Nenhuma das duas exigiria zero mudança na glintfx como a C.

## Consequências

**Positivas:** a causa raiz desaparece (não há mais troca de contexto pra mascarar) — a prova positiva (screenshots headless antes/depois de fechar o menu, cidade em produção real via Xvfb, zero divergência de pixel fora da animação viva do sprite) confirma. Apaga por completo a máquina `release_renderer`/`reacquire_renderer`/`hold_frozen_frame` (fonte de 3 bugs reais: SIGSEGV do LOAD com renderer solto, Gus andando sozinho por input preso, o próprio flash) — os 3 métodos ficam `[[deprecated]]` em `SdlWindow` (remoção física é decisão do M9), zero call-site de produção. Alinha com o porte Windows (GL 3.3 uniforme via `SDL_GL_CreateContext`, sem depender do backend "opengl" do `SDL_Renderer`, que no Windows não é o default). **Nenhum pedido necessário à glintfx** — o contrato `UiLayer` "anexa a um contexto GL que o host possui" (ADR-010) já é exatamente o que a Opção C consome; o mesmo padrão já rodava na arena de batalha desde o ADR-009.

**Negativas / aceitas:** perde-se o backend `SDL_Renderer` da cidade (e a comodidade de `SDL_SetRenderVSync`/`SDL_RenderReadPixels`) — repostos por `SDL_GL_SetSwapInterval(1)`/`glReadPixels` (via `gl3_read_backbuffer_rgba`, já existente pro smoke visual da batalha). `Render2dSdl` **não é apagado** neste branch (classe e testes seguem intactos; decisão de remoção definitiva é do M9). O fundo congelado do menu/diálogo/título continua sendo um PNG capturado via `glReadPixels` (paridade visual estrita, decisão do A1 no passo 4 do plano) em vez da alternativa "redesenhar a cidade sob a UI a cada frame" — mudança de visual potencialmente perceptível ficou registrada como decisão do líder NÃO tomada neste branch (default seguro mantido).

## Reversibilidade

Two-way door em DEV (jogo sem release pública). A interface `IRenderer` não mudou (`Render2dGl3` já implementava o contrato completo antes deste ADR, provado na arena) — voltar pro `SDL_Renderer` na cidade seria trocar 1 membro de volta, não reescrever `OverworldSim`/`sim_->render`. Os métodos deprecated (`release_renderer`/`reacquire_renderer`/`hold_frozen_frame`) continuam compilando (no-op) até o M9 decidir a remoção física — nenhum call-site externo remanescente (ex.: `app/tools/*_probe.cpp`) quebra com este ADR.

## Verificação

POC (`poc_gl_single_context_probe`, passo 1 do plano) passou e foi re-verificado (11+ frames GL3 byte-idênticos, 0 erros GL, contexto criado 1x). Build+`ctest` 2150/2150 verde. Jogo real sob Xvfb (áudio mudo): title→dificuldade→cidade→pausa aberta/fechada (screenshots antes/depois comparados por `compare`/ImageMagick — zero divergência fora da animação viva do sprite do Gus)→20 ciclos de abrir/fechar sem crash→batalha (transição, HUD, engajamento) sem crash→saída limpa. Playthrough ao vivo do líder e merge ao `main` ficam para depois do gate `A4` (`qa-engineer`, verificação independente).

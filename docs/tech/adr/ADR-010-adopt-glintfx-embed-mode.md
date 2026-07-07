# ADR-010: Adotar glintfx (embed mode) como motor de UI/HUD; aposentar o backend RmlUi vendorizado à mão

**Status:** Accepted (implementado; ver "Resultado da execução")
**Data:** 2026-06-30 (proposta) · 2026-07-01 (aceito, F0→F3 executadas)
**Decisores:** criador supremo (petrus) + software-architect (proposta)
**Cross-ref:** [ADR-008](ADR-008-repivot-qt-to-sdl3.md) (SDL3 como plataforma — MANTIDO), [ADR-009](ADR-009-rmlui.md) (RmlUi como UI/HUD via backend GL3 — este ADR-010 troca o COMO, não o QUÊ), glintfx `docs/adr/0008-embed-guest-mode.md` (a capacidade que viabiliza isto).

## Contexto

A F0/F1 do ADR-009 montou o HUD vendorizando à mão o backend oficial do RmlUi (`RmlUi_Renderer_GL3` + `RmlUi_Platform_SDL`), escrevendo um loader GL próprio (`gl3_loader.cpp`, glad) e mantendo na unha a disciplina de composição em `rmlui_hud.cpp` (arena desenha primeiro e é dona do clear; HUD compõe por cima sem limpar; a casca SDL faz o swap único). A branch `wip/rmlui-gl3-cockpit` chegou a "VISUAL INCOMPLETO": o encanamento é frágil, a disciplina no-clear/no-swap quebra fácil, e os efeitos (gradiente/box-shadow/glow) — a razão de adotar RmlUi — só apareceram depois de trocar SDL_Renderer→GL3 no meio do caminho.

Em paralelo nasceu o **glintfx** (lib C++17→23, MPL-2.0): empacota exatamente esse plumbing (RmlUi + RenderInterface_GL3 + loader + efeitos data-driven) atrás de uma fachada limpa, e planeja uma v2 de component library focada em UI de jogo (menus, diálogos, janelas, fontes, efeitos) — precisamente a nossa necessidade. O veredito de integração apontou que o glintfx v1 não encaixava (ele era dono da janela via GLFW e tinha frame fechado), mas o **ADR-0008 do glintfx** adicionou um **embed/guest mode** (`UiLayer`) que anexa o motor de UI ao NOSSO contexto GL, com frame compose-only e eventos injetados — fechando os gaps G1–G3.

Restrições inegociáveis: o ADR-008 (SDL3 dono de janela/input/gamepad/áudio/contexto GL) **fica**; `core/`+`domain/` POCO (~1013 testes) **não mudam**; a arena (`Render2dGl3` atrás de `IRenderer`) **fica**; a ordem de composição (arena→HUD→swap) **fica**.

## Decisão

Adotar o **glintfx via embed mode (`glintfx::UiLayer`)** como motor da camada de UI/HUD/chrome, no lugar do backend RmlUi vendorizado à mão. Concretamente:

1. **Aposentar** de `platform/rmlui/`: `RmlUi_Renderer_GL3.*`, `gl3_loader.*`, e o miolo de `rmlui_hud.*` que reimplementa init/compose do RmlUi. O `RmlUi_Platform_SDL.*` (sistema/eventos SDL→RmlUi) **permanece** como nosso adapter de eventos injetados (item (c) do embed mode).
2. **Substituir** por `glintfx::UiLayer`: construído contra o contexto GL que a casca SDL já cria e torna corrente; `render()` compose-only (sem clear/swap); nós alimentamos input via o adapter SDL; nós fazemos o swap (`SDL_GL_SwapWindow`).
3. **Manter** SDL3 (ADR-008), `Render2dGl3` (arena, NEAREST), a ordem de composição, e os view-models POCO (`battle_hud_model`/`battle_log_model`/...) como fonte de verdade — o `UiLayer` lê deles, nunca escreve no domínio (regra de ouro do ADR-009 intacta).
4. **GATE das 4 camadas** continua: `core/`+`domain/` sem `<RmlUi`/`<SDL`/`<glintfx`. O glintfx vive só em `platform/`+`app/`.
5. **Alinhar versão**: o GusEngine sobe RmlUi 6.2→**6.3** (versão do glintfx) — ou o glintfx confirma compat 6.2. Reconciliar FreeType + Pixel Operator Mono (a fonte do chrome continua FreeType; a arena segue com stb_truetype/font_atlas).

A arena **não** migra (já decidido no ADR-009): o glintfx é overlay de UI por cima do contexto que o jogo possui (modelo B do veredito).

## Opções consideradas

1. **Manter o backend vendorizado à mão (status quo)** — zero dependência nova, mas perpetua o pipeline frágil e o retrabalho de fidelidade visual; a v2 de componentes do glintfx ficaria fora de alcance. Rejeitada.
2. **glintfx como app shell (dono da janela, modelo A)** — exigiria re-pivotar pra GLFW, regredindo o ADR-008 (gamepad AAA/áudio/console) e jogando fora a casca SDL + a arena. Rejeitada.
3. **glintfx via embed mode (UiLayer) — ESCOLHIDA** — preserva ADR-008 + arena + ~1013 testes POCO; elimina o plumbing frágil; abre caminho pra v2 (componentes de UI de jogo). Custo: depender do glintfx (repo próprio do líder) + acordo de estado GL compartilhado.

## Consequências

**Positivas:** sai do plumbing frágil (RmlUi GL3 + loader + composição na unha) pra uma fachada testada e versionada; ganha os efeitos data-driven sem reimplementar; destrava a v2 (component library) pra cockpit/menus/diálogos; mantém a soberania do loop/gamepad/áudio (SDL3) e da arena. core/domain intactos.

**Negativas / aceitas:** dependência nova (glintfx via FetchContent/find_package — mas é repo do próprio líder, MPL-2.0, alinhado); reescrever os testes de `platform/rmlui` (os de `core`/`domain` não mudam); coordenar duas árvores (GusWorld + glintfx) — amortizado pelo reuso.

**Riscos / atenção:** (R1) estado GL compartilhado entre `UiLayer` e `Render2dGl3` no mesmo contexto — coberto pelo contrato save/restore do ADR-0008 do glintfx + nossa disciplina no-clear já aprendida; (R2) skew/ABI do RmlUi 6.2↔6.3; (R3) o embed mode do glintfx ainda será implementado — esta adoção depende da entrega dele (sequenciar: embed mode pronto → migração aqui).

## Reversibilidade

Two-way door no backend de UI (o `IRenderer`/arena e os view-models POCO isolam; o RmlUi_Renderer_GL3 vendorizado segue no histórico se preciso voltar). One-way apenas o esforço de migração. Sem releases públicas (jogo em DEV).

## Execução (faseada)

- **Pré-req:** glintfx entrega o embed mode (`UiLayer`) do ADR-0008.
- **F1:** subir RmlUi 6.2→6.3 no GusEngine; verde nos ~1013 testes.
- **F2:** introduzir `glintfx::UiLayer` na casca SDL atrás de um flag; portar o cockpit (`cockpit.rml`/`.rcss`) pra ele; manter o caminho antigo até paridade.
- **F3:** atingida paridade visual + pacing/motor verdes, remover `RmlUi_Renderer_GL3.*` + `gl3_loader.*` + o miolo do `rmlui_hud`; estender o GATE pra `<glintfx`.
- **F4:** adotar componentes da v2 do glintfx no cockpit/menus/diálogos.

## Resultado da execução

Executado e aceito ao vivo pelo criador em 2026-07-01. As fases planejadas rodaram assim:

- **F0** — alinhou o RmlUi vendorizado ao SHA `2cd2886` (6.3), a mesma versão que o glintfx usa, fechando o risco de skew/ABI (R2). Testes verdes.
- **F1** — smoke do embed mode atrás de flag, provando o `glintfx::UiLayer` anexado ao contexto GL que a casca SDL já cria (de-risk pré-cockpit).
- **R-dup-backend (Opção 2)** — o backend RmlUi vendorizado foi gateado ao build `OFF`, mantido só como rede de segurança durante a transição.
- **F2** — cockpit "Tático" portado pro `glintfx::UiLayer`, atingindo **paridade visual real** (gradientes, glow, `border-radius`, molduras nativos) e **dados vivos** via data-model (HP, verbo, alvo, log de batalha, retrato que segue o ator ativo, label do inimigo). Passou por polish e foi aprovado ao vivo pelo criador.
- **F3** — removido o backend RmlUi vendorizado (`RmlUi_Renderer_GL3.*`, `gl3_loader.*`, o miolo de `rmlui_hud`) e **estendido o GATE das 4 camadas** pra proibir `<glintfx` em `core/`+`domain/` (junto com `<RmlUi`/`<SDL`). O glintfx passa a ser o único motor de UI/HUD.

**Versão consumida:** glintfx **v0.2.4** via CMake FetchContent (`GIT_TAG v0.2.4`), com **`GLINTFX_BACKEND_GLFW=OFF`** (embed-only, sem GLFW, honrando o ADR-008/SDL3). A v0.2.4 carrega a UA-stylesheet do RmlUi (`div` = `block` por padrão). O `UiLayer` compõe sobre o FBO 0 (sem clear/swap, salva e restaura o estado GL), recebe input via `process_event(UiEvent)` e lê dados vivos via data-model (`create_data_model` → `bind_*` → `load` → `set_*`); texturas PNG via stb_image com premultiply; `dp_ratio` (960 lógico para 1080) + `asset_base_url`.

**Preservado:** SDL3 (ADR-008) segue dono de janela/loop/input/gamepad/contexto GL; a arena (`Render2dGl3`, loader glad próprio) intocada; a ordem de composição arena → UI → swap; `core/`+`domain/` POCO (~1013 testes) intactos; a invariante das 4 camadas mantida (agora também barrando `<glintfx`).

A **F4** (componentes da v2 do glintfx pra menus/diálogos) segue em aberto, sem bloquear esta decisão.

**Nota (adendo, AC-E9 AUDITORIA-COMPLETA-2026-07-06):** o corpo acima registra o pin da época (v0.2.4); pins subsequentes (v0.2.5, v0.3.0, v0.3.1) chegaram via commits de bump dedicados (`db2d643`, `69b7e96`, `d8f103f`) e não reescrevem este registro histórico. O pin REAL em vigor sempre vive em `GusEngine/CMakeLists.txt` (`GIT_TAG`); README.md e CLAUDE.md apontam pra lá em vez de repetir o número, pra não se defasarem de novo a cada bump.

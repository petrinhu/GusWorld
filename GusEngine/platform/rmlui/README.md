# platform/rmlui - UI/HUD RmlUi sobre OpenGL 3.3 (ADR-009 + adendo GL3)

Camada de UI/HUD do GusWorld sobre OpenGL 3.3 (a janela usa contexto GL, nao
SDL_Renderer). Vive SO em `platform/` (com `app/ui/`); `core/` e `domain/` permanecem
POCO (o GATE em `tools/check.sh` proibe `<RmlUi`, `<SDL` e `Rml::` la).

## Por que GL3 e nao SDL_Renderer (achado da F0)

O backend `RenderInterface_SDL` NAO implementa `CompileShader`/`CompileFilter`: gradiente,
box-shadow, blur, mask, drop-shadow NAO renderizam (saem como geometria quebrada). Esses
sao justamente os efeitos do mock. O backend `RenderInterface_GL3` (OpenGL 3.3 core)
implementa todos. Decisao ratificada pelo criador: usar GL3. Detalhe no ADR-009 (adendo).

## Arquivos

- `RmlUi_Renderer_GL3.{h,cpp}` + `RmlUi_Include_GL3.h`: backend de RENDER oficial GL3
  (vendorizado do pin 6.2; traz glad header-only). Patch GusWorld: `LoadTexture` usa
  `stb_image` (PNG) em vez do TGA-only original.
- `RmlUi_Platform_SDL.{h,cpp}`: backend de SISTEMA oficial (clock/clipboard/cursor +
  eventos SDL->RmlUi), vendorizado intacto. SDL ainda e dono de janela/input/contexto-GL;
  so o RENDER virou GL.
- `gl3_loader.{hpp,cpp}`: carrega o glad (GL 3.3) via o proc-address da casca
  (SDL_GL_GetProcAddress) + le o backbuffer pro PNG do smoke visual.
- `rmlui_hud.{hpp,cpp}`: wrapper FINO da engine (PImpl): inicializa o nucleo, carrega
  fonte (FreeType), parseia RML/RCSS, data-model, e COMPOE por cima da arena via GL3.

A ARENA tambem migrou pra GL: `platform/render2d/render2d_gl3.{hpp,cpp}` (mesma API
publica `IRenderer`, NEAREST pra pixel-art). Os dois compartilham o contexto GL.

## Composicao (regra de ouro)

A arena (`Render2dGl3`) desenha PRIMEIRO no backbuffer; o HUD (RmlUi-GL3) compoe DEPOIS;
o swap (`SDL_GL_SwapWindow`) e UNICO, feito pela casca:

```
Render2dGl3 renderer(/*gl_active=*/true);
scene.render(...)              // arena: clear + draws no backbuffer GL
hud.update(); hud.compose();   // HUD: GL3 BeginFrame (layer offscreen) -> Render ->
                               //   EndFrame (blita a layer no backbuffer, por cima)
SDL_GL_SwapWindow(window);     // swap unico (arena + HUD compostos)
```

O GL3 `BeginFrame` limpa SO a layer offscreen do HUD (nao o backbuffer); o `EndFrame`
blita essa layer no backbuffer com premultiplied alpha, compondo por cima da arena.

## Ciclo de vida (lifetime) e teardown (corrigido + nota de sanitizer)

- DESTRUICAO: a render interface (RenderInterface_GL3) e o SystemInterface_SDL sao MEMBROS
  do `RmlUiHud::Impl`, destruidos DEPOIS do corpo do `~Impl()` (ordem reversa) - logo
  continuam vivos durante o teardown do RmlUi (que os chama pra liberar texturas).
- BUG CORRIGIDO (achado por UBSan): o `~Impl()` chamava `UnloadDocument` + `Update()`, o
  que disparava `ReleaseUnloadedDocuments` no meio do teardown e baguncava a ordem interna
  de destruicao dos elementos (UAF). FIX: nao chamar `Update()` no destrutor; deixar
  `Rml::Shutdown()` (ultimo Hud) limpar todos os contextos/documentos na ordem controlada
  do proprio RmlUi.
- GUARD 0x0: em Wayland a janela pode reportar tamanho 0 (minimizada/ocluida); o
  `compose()` e o loop do `battle_preview` PULAM o frame quando `pixel < 1` (o GL3 do RmlUi
  asserta viewport >= 1; em Release o assert e no-op e `glViewport(0,0,0,0)` + FBO de 0
  daria erro de GL/crash em alguns drivers).
- NOTA UBSan (vendored): sob `-fsanitize=undefined`, o `Rml::Shutdown()` do RmlUi 6.2
  dispara um `vptr` UB ("member call ... does not point to ElementDocument") na propria
  destruicao de elementos (Element.cpp:2074 / ElementDocument.cpp:257). E UB DENTRO do
  RmlUi (chamada virtual via downcast durante a destruicao), NAO do nosso codigo. NAO
  crasha builds Release (validado: 800 frames + 15 ciclos de teardown saem 0; o loop e
  leak/UAF-free). Tratado como issue conhecido de terceiro; nao mascarar com supressao
  global (so afeta o build de sanitizer, que e diagnostico).

## Fonte

`RMLUI_FONT_ENGINE=freetype` (CMake raiz). RmlUi 6.2 usa `find_package(Freetype)` (NAO
FetchContent): no Linux vem do sistema (`libfreetype`). Cross-compile Windows e ponto de
atencao (R-font). A face e `Pixel Operator Mono` (`assets/fonts/`).

## Escala logica->real (pixel-perfect)

O cockpit e autorado em 960x540 (D1) em unidades `dp`. O contexto RmlUi recebe as
dimensoes em PIXELS REAIS (ex.: 1920x1080) e `dp_ratio = pixel_w/960`: 1dp = (real/logico)
px, entao o layout logico preenche o alvo escalado por inteiro (x2 em 1080p), nitido.

## RCSS no backend GL3: o que COBRE (validado na F1)

AGORA com shaders, COBRE os efeitos do mock:
- `vertical-gradient` / `horizontal-gradient` / `radial-gradient` / `linear-gradient`
- `box-shadow` (glow), `drop-shadow`, `filter: blur(...)`, `mask`
- `border`, `border-radius`, cor solida, `opacity`
- texto FreeType (regular/bold), `font-effect` (glow/outline no texto)
- imagem (`decorator: image(...)`), via o patch stb (PNG)
- layout, `data-*` binding, `@keyframes` + `animation`, `transform`
- `body { background: transparent; }`: OBRIGATORIO (senao o fundo branco tapa a arena)

## Sintaxe RCSS (pegadinhas pegas na F0/F1)

- gradiente: `vertical-gradient( #aaa #bbb )` (sem virgula entre as 2 cores); ja o
  `radial-gradient( circle closest-side, #aaa, #bbb )` PRECISA de virgula entre as cores.
- `box-shadow: <cor> <x> <y> <blur> <spread>;` (cor primeiro).
- `decorator: image( <path> cover );` resolve `<path>` contra o source_url do doc. Doc da
  memoria com source_url vazio canonicaliza e PERDE a barra inicial de caminhos absolutos;
  passe um source_url base (set_asset_base_url) e use caminhos RELATIVOS.
- `font-family` deve casar o NOME da face carregada: `"Pixel Operator Mono"`.
- elementos `position: absolute` ancoram no ANTECESSOR posicionado mais proximo: o pai
  precisa `position: relative` (senao ancoram no root 0,0).

## Smoke visual

```
SDL_VIDEODRIVER=offscreen \
GUSWORLD_RMLUI_HELLO=1 \
GUSWORLD_RMLUI_CAPTURE=/caminho/saida.png \
  gusworld_app --battle
```

Cria contexto GL, renderiza ~20 frames, le o backbuffer (arena + HUD compostos) num PNG e
sai. Sem as duas envs, `--battle` roda normal (sem HUD RmlUi). O driver `offscreen` do SDL
da um contexto GL via EGL (serve pro CI/captura sem display).

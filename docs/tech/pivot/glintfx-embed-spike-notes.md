# glintfx embed-mode — notas de spike (prep F0)

**Branch:** `wip/glintfx-embed-spike` · **Data:** 2026-06-30 · **Autor:** engine-graphics-programmer
**Escopo:** DESIGN/STUB. Nada aqui é wired no build. O glintfx **não** entra nesta tarefa
(aguarda v0.2.1 sem GLFW). Two-way door — ADR-010, fases F1+.

> **Status do pré-req F0 (feito):** RmlUi alinhado 6.2 → 6.3 (SHA exato
> `2cd28864ae25ed345b70598751703a5433b12356`, commit de 2026-06-17) via FetchContent;
> `GIT_SHALLOW` removido do Declare (incompatível com pin por SHA). Build limpo, **suíte
> 1021/1021** (baseline 1021 → 1021), GATE de 4 camadas verde. core/ e domain/ intactos.

Todos os caminhos de arquivo abaixo são relativos a `GusEngine/` salvo nota.

---

## (a) Ponto de attach do `glintfx::UiLayer`

A casca SDL que **cria e torna corrente o contexto GL** vive em `app/src/screens/battle_preview.cpp`,
dentro de `run_battle_preview()`. É aqui — depois do contexto corrente e do glad carregado,
junto da construção do HUD vendorizado — que o `glintfx::UiLayer` vai anexar no F1.

| Passo | Arquivo:linha | O quê |
|---|---|---|
| Atributos GL (core 3.3, double-buffer, stencil 8) | `battle_preview.cpp:375-379` | `SDL_GL_SetAttribute(...)` antes da janela |
| Cria a janela `SDL_WINDOW_OPENGL` | `battle_preview.cpp:381-383` | `SDL_CreateWindow(...)` |
| **Cria o contexto GL** | `battle_preview.cpp:389` | `SDL_GL_CreateContext(window)` |
| **Torna o contexto corrente** | `battle_preview.cpp:397` | `SDL_GL_MakeCurrent(window, gl)` ← **attach point do UiLayer** (logo após) |
| Carrega ponteiros GL (glad) | `battle_preview.cpp:402-409` | `gl3_load_functions(SDL_GL_GetProcAddress)` — UiLayer pode reusar o mesmo loader OU trazer o seu; **decidir no F1 quem é dono do glad** (risco abaixo) |
| Constrói a arena | `battle_preview.cpp:422` | `Render2dGl3 renderer(/*gl_active=*/true)` |
| **Constrói/inicia o HUD vendorizado (fallback)** | `battle_preview.cpp:431, 447` | `RmlUiHud hud;` + `hud.init(gl_active=true, pw0, ph0, 960, 540)` ← o `UiLayer` entra **lado a lado** disto, atrás de flag (item c) |

### Ordem de composição por frame (PRESERVADA — ADR-008/009)

Loop em `battle_preview.cpp:564-694`. Por frame, na ordem:

1. **Arena desenha primeiro** (dona do clear do backbuffer): `scene.render(renderer, pw, ph)` em
   `battle_preview.cpp:652` (no modo FXTEST, `renderer.begin_frame()/end_frame()` em `:649-650`).
   `Render2dGl3::present()` é no-op de swap por design (`platform/src/render2d/render2d_gl3.cpp:413-419`).
2. **HUD compõe por cima** (sem clear, sem swap): `hud.update()` + `hud.compose()` em
   `battle_preview.cpp:666-667` (guard `rmlui_hud_on`). O contrato no-clear/no-swap está em
   `platform/rmlui/rmlui_hud.cpp:12` e `platform/include/gus/platform/rmlui/rmlui_hud.hpp:13,106`.
   → No F1, `glintfx::UiLayer::render()` (compose-only) ocupa exatamente este lugar.
3. **Swap único** (dono do frame = a casca): `SDL_GL_SwapWindow(window)` em `battle_preview.cpp:685`.

Resumo: **arena → HUD → swap**, contexto GL único compartilhado, swap único pela casca.
O `UiLayer` herda o slot do `hud.compose()`; quem faz o swap continua sendo a casca (`:685`).

---

## (b) Ponte SDL → `glintfx::UiEvent` (mapa de design)

API neutra do glintfx (replicada do header `ui_event.hpp` que o líder forneceu — **não** está no
build, só referência):

```cpp
struct UiEvent {
    enum Type { MouseMove, MouseButton, Key, Text, Resize };
    Type type;
    float x, y;          // MouseMove / MouseButton (coords em pixels do backbuffer)
    int button;          // 0=L, 1=R, 2=M
    bool pressed;        // MouseButton / Key (down=true, up=false)
    Key key;             // Key
    int modifiers;       // OR de Mod
    const char* text;    // Text (UTF-8)
    int width, height;   // Resize (pixels)
};
enum class Key { None, Up, Down, Left, Right, Enter, Escape, Tab, Space, Backspace };
enum Mod { Mod_None=0, Mod_Shift=1, Mod_Ctrl=2, Mod_Alt=4 };
```

**Referência interna:** o adapter `RmlSDL::InputEventHandler` (`platform/rmlui/RmlUi_Platform_SDL.cpp:128`,
com `ConvertKey` em `:279`, `ConvertMouseButton` em `:464`, `GetKeyModifierState` em `:475`) já faz
SDL→RmlUi e **permanece** (ADR-010 item 1: o `RmlUi_Platform_SDL` segue como adapter de eventos
injetados). A tabela abaixo é o análogo SDL→`UiEvent` para o caminho glintfx.

> **Nota de fato (NÃO PROVADO antes):** hoje o HUD vendorizado é **display-only** — o loop de
> input em `battle_preview.cpp:566-608` traduz SDL **direto** para ações da cena
> (`scene.menu_move/menu_confirm/skip/start_combat`), e **nada** é injetado no `Context` do RmlUi.
> Logo a ponte de input para a UI é caminho **novo** (não há paridade prévia a manter); ela só
> passa a importar quando o `UiLayer` precisar de foco/navegação/cliques próprios (v2 de componentes).

### Mapa mouse

| SDL_Event | `UiEvent` |
|---|---|
| `SDL_EVENT_MOUSE_MOTION` | `MouseMove`: `x=ev.motion.x`, `y=ev.motion.y` |
| `SDL_EVENT_MOUSE_BUTTON_DOWN` | `MouseButton`: `button=map(ev.button.button)`, `pressed=true`, `x/y=ev.button.x/y` |
| `SDL_EVENT_MOUSE_BUTTON_UP` | `MouseButton`: idem, `pressed=false` |
| botão: `SDL_BUTTON_LEFT→0`, `SDL_BUTTON_RIGHT→1`, `SDL_BUTTON_MIDDLE→2` | (demais ignorados) |

> Coords: o `UiLayer` é informado em **pixels do backbuffer** (`pw,ph` de `SDL_GetWindowSizeInPixels`,
> `battle_preview.cpp:637-638`). Se o `UiLayer` usar dp/escala lógica própria, converter no F1 —
> **NÃO PROVADO** qual espaço o `UiLayer` espera (resolver com o líder/glintfx antes de wire).

### Mapa teclado (`SDL_EVENT_KEY_DOWN`→`pressed=true`, `SDL_EVENT_KEY_UP`→`pressed=false`)

| `ev.key.key` (SDLK_*) | `Key::` | Texto associado |
|---|---|---|
| `SDLK_UP` | `Up` | — |
| `SDLK_DOWN` | `Down` | — |
| `SDLK_LEFT` | `Left` | — |
| `SDLK_RIGHT` | `Right` | — |
| `SDLK_RETURN` / `SDLK_KP_ENTER` | `Enter` | — |
| `SDLK_ESCAPE` | `Escape` | — |
| `SDLK_TAB` | `Tab` | — |
| `SDLK_SPACE` | `Space` | — (ou Text " " — ver nota) |
| `SDLK_BACKSPACE` | `Backspace` | — |
| qualquer outra | `Key::None` | (texto vem por `SDL_EVENT_TEXT_INPUT`, não por keycode) |

Modificadores (`ev.key.mod`, `SDL_KMOD_*`) → `modifiers` (OR de `Mod`):

| SDL_KMOD_* | bit `Mod` |
|---|---|
| `SDL_KMOD_SHIFT` (LSHIFT\|RSHIFT) | `Mod_Shift` (1) |
| `SDL_KMOD_CTRL` (LCTRL\|RCTRL) | `Mod_Ctrl` (2) |
| `SDL_KMOD_ALT` (LALT\|RALT) | `Mod_Alt` (4) |
| nenhum | `Mod_None` (0) |

### Mapa texto / resize

| SDL_Event | `UiEvent` |
|---|---|
| `SDL_EVENT_TEXT_INPUT` | `Text`: `text=ev.text.text` (UTF-8; ponteiro válido só no escopo do evento — copiar se o `UiLayer` reter) |
| `SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED` | `Resize`: `width/height` de `SDL_GetWindowSizeInPixels` (pixels reais; é o que o backbuffer usa) |
| `SDL_EVENT_WINDOW_RESIZED` | (preferir o `PIXEL_SIZE_CHANGED`; em HiDPI o tamanho lógico ≠ pixels — o backend vendorizado já usa `WINDOW_PIXEL_SIZE_CHANGED`, `RmlUi_Platform_SDL.cpp:143`) |

### Mapa gamepad (navegação de foco — `SDL_EVENT_GAMEPAD_BUTTON_DOWN/UP`)

`ev.gbutton.button` (`SDL_GAMEPAD_BUTTON_*`) → `UiEvent::Key` com `pressed` conforme down/up:

| `SDL_GAMEPAD_BUTTON_*` | `Key::` |
|---|---|
| `DPAD_UP` | `Up` |
| `DPAD_DOWN` | `Down` |
| `DPAD_LEFT` | `Left` |
| `DPAD_RIGHT` | `Right` |
| `SOUTH` (A) | `Enter` (confirmar) |
| `EAST` (B) | `Escape` (voltar/cancelar) |

> Stick analógico → d-pad virtual e auto-repeat de navegação ficam **fora** deste mapa de prep
> (lógica de foco do `UiLayer`/v2). `modifiers` do gamepad = `Mod_None`.

### Esboço (COMENTADO — não compilado, não incluído no build)

```cpp
// ESBOÇO de design — NÃO faz parte do build (sem <glintfx, sem wire). F1 valida.
// glintfx::UiEvent to_ui_event(const SDL_Event& ev) {
//     using K = glintfx::Key;
//     glintfx::UiEvent e{};
//     switch (ev.type) {
//     case SDL_EVENT_MOUSE_MOTION:
//         e.type = glintfx::UiEvent::MouseMove; e.x = ev.motion.x; e.y = ev.motion.y; break;
//     case SDL_EVENT_MOUSE_BUTTON_DOWN:
//     case SDL_EVENT_MOUSE_BUTTON_UP:
//         e.type = glintfx::UiEvent::MouseButton;
//         e.x = ev.button.x; e.y = ev.button.y;
//         e.pressed = (ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN);
//         e.button = ev.button.button == SDL_BUTTON_RIGHT  ? 1
//                  : ev.button.button == SDL_BUTTON_MIDDLE ? 2 : 0;
//         break;
//     case SDL_EVENT_KEY_DOWN:
//     case SDL_EVENT_KEY_UP:
//         e.type = glintfx::UiEvent::Key;
//         e.pressed = (ev.type == SDL_EVENT_KEY_DOWN);
//         e.modifiers =
//             ((ev.key.mod & SDL_KMOD_SHIFT) ? glintfx::Mod_Shift : 0) |
//             ((ev.key.mod & SDL_KMOD_CTRL)  ? glintfx::Mod_Ctrl  : 0) |
//             ((ev.key.mod & SDL_KMOD_ALT)   ? glintfx::Mod_Alt   : 0);
//         switch (ev.key.key) {
//             case SDLK_UP: e.key = K::Up; break;       case SDLK_DOWN: e.key = K::Down; break;
//             case SDLK_LEFT: e.key = K::Left; break;   case SDLK_RIGHT: e.key = K::Right; break;
//             case SDLK_RETURN: case SDLK_KP_ENTER: e.key = K::Enter; break;
//             case SDLK_ESCAPE: e.key = K::Escape; break;
//             case SDLK_TAB: e.key = K::Tab; break;     case SDLK_SPACE: e.key = K::Space; break;
//             case SDLK_BACKSPACE: e.key = K::Backspace; break;
//             default: e.key = K::None; break;
//         }
//         break;
//     case SDL_EVENT_TEXT_INPUT:
//         e.type = glintfx::UiEvent::Text; e.text = ev.text.text; break;  // copiar se retido
//     case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
//         e.type = glintfx::UiEvent::Resize; /* width/height via SDL_GetWindowSizeInPixels */ break;
//     case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
//     case SDL_EVENT_GAMEPAD_BUTTON_UP:
//         e.type = glintfx::UiEvent::Key;
//         e.pressed = (ev.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN);
//         switch (ev.gbutton.button) {
//             case SDL_GAMEPAD_BUTTON_DPAD_UP: e.key = K::Up; break;
//             case SDL_GAMEPAD_BUTTON_DPAD_DOWN: e.key = K::Down; break;
//             case SDL_GAMEPAD_BUTTON_DPAD_LEFT: e.key = K::Left; break;
//             case SDL_GAMEPAD_BUTTON_DPAD_RIGHT: e.key = K::Right; break;
//             case SDL_GAMEPAD_BUTTON_SOUTH: e.key = K::Enter; break;
//             case SDL_GAMEPAD_BUTTON_EAST: e.key = K::Escape; break;
//             default: e.key = K::None; break;
//         }
//         break;
//     default: e.type = glintfx::UiEvent::MouseMove; e.x = e.y = -1; break;  // sentinela "ignorar"
//     }
//     return e;
// }
```

> O esboço retorna sempre um `UiEvent`; no F1 vale uma variante `std::optional<UiEvent>` (ou
> bool out-param) para o default "evento sem mapeamento" não virar ruído. Design only.

---

## (c) Scaffold do flag (proposta de design — NÃO wired)

Objetivo: poder alternar **HUD-vendorizado (fallback)** ⇄ **`glintfx::UiLayer`** sem **remover nada**
do caminho atual (remoção é F3, só pós-paridade). Dois eixos, espelhando o que já existe:

### Eixo 1 — compile-time: CMake option `GUSWORLD_GLINTFX` (default OFF)

Decide se o caminho glintfx é **sequer compilado**. Enquanto OFF (e nesta tarefa fica OFF/ausente),
o binário é byte-a-byte o de hoje — não há `<glintfx>` no include path nem FetchContent dele.

```cmake
# (design — NÃO adicionar agora) em GusEngine/CMakeLists.txt:
option(GUSWORLD_GLINTFX "Compila o caminho glintfx::UiLayer (ADR-010 F2)" OFF)
# Quando ON (F1+): FetchContent_Declare(glintfx ...) + target_compile_definitions(... GUSWORLD_GLINTFX=1)
# + glintfx::glintfx em target_link_libraries de platform/ e/ou app/ (NUNCA core/domain — GATE).
```

No código, o caminho glintfx fica atrás de `#ifdef GUSWORLD_GLINTFX` (assim o fallback vendorizado
compila e roda sozinho com a option OFF — zero regressão). O GATE de 4 camadas
(`tools/check.sh`, proíbe `<RmlUi`/`<SDL` em core/domain) **será estendido para `<glintfx`** só na F3.

### Eixo 2 — runtime: `GUSWORLD_UI_BACKEND=vendored|glintfx`

Espelha os toggles de env que já existem em `battle_preview.cpp`:
`GUSWORLD_RMLUI_OFF` (`:433-436`, desliga o HUD) e `GUSWORLD_RMLUI_FXTEST` (`:416-419`).
Proposta: `GUSWORLD_UI_BACKEND` (default `vendored`). Tabela de resolução:

| `GUSWORLD_GLINTFX` (compile) | `GUSWORLD_UI_BACKEND` (runtime) | Caminho efetivo |
|---|---|---|
| OFF (default) | qualquer | HUD vendorizado (`RmlUiHud`) — único compilado |
| ON | `glintfx` | `glintfx::UiLayer` |
| ON | `vendored` (ou ausente) | **sem HUD vendorizado** (ver R-dup-backend abaixo) |
| qualquer | `GUSWORLD_RMLUI_OFF=1` | sem UI (precede o backend; comportamento atual preservado) |

> **ATUALIZAÇÃO 2026-06-30 (R-dup-backend resolvido — decisão do líder, Opção 2):** o backend
> RmlUi vendorizado (`RmlUi_Renderer_GL3.cpp` + `RmlUi_Platform_SDL.cpp` + `rmlui_hud.cpp`) agora é
> **gateado ao build OFF**. Quando `GUSWORLD_GLINTFX=ON`, essas 3 TUs **não são compiladas/linkadas**
> — só o backend GL3 do **glintfx** fica (eliminando a duplicação que antes só linkava por ordem de
> link). **Consequência:** no build ON **não existe mais** o caminho de HUD vendorizado em runtime; o
> `RmlUiHud` nem é instanciado (`battle_preview.cpp` guarda a instanciação/compose com
> `#ifndef GUSWORLD_GLINTFX`). Logo, `GUSWORLD_UI_BACKEND=vendored` **no build ON** não tem backend
> RmlUi (vira HUD-off). **O fallback do caminho antigo passa a ser BUILDAR COM `GUSWORLD_GLINTFX=OFF`
> (default), que continua 100% intacto** — two-way door preservado (basta reconfigurar com a option
> OFF pra ter o caminho vendorizado completo + suíte 1021/1021).
>
> **Plumbing do glad (consequência técnica obrigatória):** o `RmlUi_Renderer_GL3.cpp` vendorizado era
> o **dono do `GLAD_GL_IMPLEMENTATION`** (a implementação do glad que a ARENA `Render2dGl3` e o
> `gl3_loader` usam). Como ele sai no ON, o `gl3_loader.cpp` (que fica nos **dois** builds, é o loader
> da arena) **assume a implementação do glad** quando o CMake passa `GUSWORLD_OWN_GLAD_IMPL` (só no ON).
> No OFF nada muda: o dono do glad segue sendo o `RmlUi_Renderer_GL3.cpp`. O glintfx usa **gl3w**
> (tabela de ponteiros própria), independente do glad — sem conflito de símbolos GL.
>
> Também gateado ao OFF: `platform/tests/rmlui_hud_test.cpp` (4 TEST_CASE que exercitam o `RmlUiHud`),
> pois no ON o `RmlUiHud` não existe pra testar. Suíte OFF segue 1021/1021.

### Onde o switch entra (sem remover nada)

O HUD **já está isolado** atrás de um único ponto de verdade:

- `BattleScene::set_hud_external(bool)` (`battle_preview.cpp:478`): quando `true`, a cena **não**
  desenha cockpit/log à mão (só arena/banner/floaters/fila), evitando dois cockpits sobrepostos.
  Esse contrato vale para QUALQUER UI externa — vendored **ou** glintfx. Nada muda nele.
- O bloco de criação/compose do HUD vive todo em `battle_preview.cpp` (`:431` criação, `:447` init,
  `:654-668` compose). O switch é local a esse bloco:

```
// design (não wired):
//   resolve backend = compile(GUSWORLD_GLINTFX) ∧ runtime(GUSWORLD_UI_BACKEND)
//   if backend == glintfx:  ui = glintfx::UiLayer(attach no gl corrente em :397); ui.render() em :667
//   else (vendored):        ui = RmlUiHud (caminho de hoje, intacto)
//   scene.set_hud_external(ui_ativo);     // mesmo contrato p/ os dois
//   SDL_GL_SwapWindow em :685             // swap único, inalterado
```

O `set_hud_external(bool)` e o slot único de `compose()`/`render()` antes do swap são o que torna a
troca um **two-way door**: dá pra rodar os dois lado a lado até a paridade (F2) e só então remover o
vendorizado (F3). Nenhuma linha do caminho atual é removida agora.

---

## NÃO PROVADO / riscos novos encontrados

- **R-dup-backend — RESOLVIDO (2026-06-30, Opção 2 do líder):** o `RmlUi_Renderer_GL3.cpp`
  duplicado (cópia nossa + a do glintfx, que só linkava por ordem de link) foi eliminado gateando o
  backend vendorizado ao build OFF. PROVA: no `build.ninja` do ON a única TU `RmlUi_Renderer_GL3.cpp.o`
  é a do glintfx (`_deps/glintfx-build/.../rmlui-src/Backends/`), e `nm` mostra
  `RenderInterface_GL3::RenderInterface_GL3()` com provedor único — sem mais dependência de ordem de
  link. Detalhe na seção (c) acima. NÃO PROVADO novo daqui: o **save/restore de estado GL** do glintfx
  ao compor por cima da arena segue como R-gl-state abaixo (o smoke não corrompeu, mas não é prova
  formal de paridade de estado).
- **R-glad-owner:** quem é dono do loader GL no F1 — o `gl3_load_functions` atual
  (`battle_preview.cpp:402`) ou o `UiLayer`. Duplo-load do glad pode ser benigno (idempotente) ou
  redefinir ponteiros; **validar no F1** quem chama o quê e em que ordem (contexto já corrente em `:397`).
- **R-gl-state:** estado GL compartilhado `UiLayer` × `Render2dGl3` no mesmo contexto (já é R1 do
  ADR-010). O backend vendorizado salva/restaura estado; precisamos confirmar que o `UiLayer` faz o
  mesmo contrato save/restore antes de compor por cima da arena. **NÃO PROVADO** até o embed mode existir.
- **R-coord-space:** espaço que o `UiLayer` espera para mouse/resize (pixels do backbuffer vs dp/escala
  lógica 960×540). O HUD vendorizado usa `init(pw0, ph0, logical=960×540)` (`battle_preview.cpp:447`);
  alinhar a convenção na ponte de eventos. **NÃO PROVADO.**
- **R-rmlui-6.3-untagged:** o SHA `2cd2886` é RmlUi 6.3 **in-dev** (não há tag `6.3` no upstream ainda;
  commit de 2026-06-17). Build e suíte verdes hoje, mas é um pin móvel-no-tempo do upstream — manter o
  SHA exato (não `branch`) e revisitar quando o glintfx publicar a v0.2.1.
- **R-input-novo:** injeção de input na UI é caminho **novo** (hoje o HUD é display-only); não há
  paridade de input a preservar, mas também não há cobertura — qualquer foco/clique no `UiLayer` nasce
  sem teste. Planejar testes da ponte SDL→`UiEvent` (POCO-testável: função pura `SDL_Event → UiEvent`).

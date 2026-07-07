# platform/rmlui - loader GL da ARENA (pos-ADR-010)

Esta pasta NAO hospeda mais UI/HUD nenhuma. O backend RmlUi vendorizado a mao (F0-F2
do ADR-009) foi aposentado no ADR-010 F3: a UI/HUD do GusWorld (cockpit, menus,
dialogos) e servida pelo **glintfx** via *embed mode* (`glintfx::UiLayer`), consumido
por `app/` via CMake `FetchContent` (pin em `GusEngine/CMakeLists.txt`) - ver
[ADR-010](../../docs/tech/adr/ADR-010-adopt-glintfx-embed-mode.md).

O que sobrevive aqui e so o **loader GL da ARENA** (`Render2dGl3`, o backend OpenGL
3.3 do combate/mapa - NAO da UI): a arena precisava de um jeito de carregar as
funcoes de GL 3.3 (glad) e de ler o backbuffer pro smoke visual ANTES do RmlUi
existir, e continua precisando DEPOIS dele sair. O glintfx usa `gl3w` (tabela de
ponteiros PROPRIA), entao nao ha conflito de simbolos GL entre o loader daqui e o do
glintfx.

## Arquivos

- `gl3_loader.{hpp,cpp}`: carrega o glad (GL 3.3 core) via o proc-address da casca
  (`SDL_GL_GetProcAddress`) e le o backbuffer (`gl3_read_backbuffer_rgba`) pro PNG do
  smoke visual. Compilado em `gusengine_platform` (ver `platform/CMakeLists.txt`) com
  `GUSWORLD_OWN_GLAD_IMPL=1` - esta e a UNICA TU que define
  `GLAD_GL_IMPLEMENTATION` (glad e header-only).
- `RmlUi_Include_GL3.h`: header vendorizado do glad (GL 3.3 core, gerado pelo
  [glad web loader](https://glad.dav1d.de/)) que `gl3_loader.cpp` inclui para as
  declaracoes/implementacao. O nome e resquicio da era RmlUi (o glad chegou junto
  daquele backend), mas o conteudo e generico OpenGL - nao tem nada de RmlUi dentro.

## OpenGL da arena vs UI

- **Arena** (`platform/render2d/render2d_gl3.{hpp,cpp}`, `Render2dGl3` atras da API
  `IRenderer`): desenha PRIMEIRO no backbuffer, usando o glad carregado por
  `gl3_loader.cpp`.
- **UI/HUD** (`glintfx::UiLayer`, vive em `app/`): compoe DEPOIS por cima (compose-
  only, sem clear/swap - salva e restaura o estado GL).
- O swap (`SDL_GL_SwapWindow`) e UNICO, feito pela casca SDL (`app/`) - mesma regra
  de ouro de composicao herdada do ADR-009/ADR-010 (arena -> UI -> swap).

## Smoke visual da arena

`gl3_read_backbuffer_rgba` e usado pelo smoke headless do app (`--smoke`, ver
`tools/check.sh`) para capturar o backbuffer renderizado num PNG sem display real
(`SDL_VIDEODRIVER=dummy`), provando que a arena desenhou algo sem precisar de olho
humano.

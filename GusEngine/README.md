# GusEngine

Engine própria do GusWorld em **C++20 + SDL3 + glintfx (RmlUi)**, escrita do zero. Este
diretório é a engine completa (M0-M8 entregues; ver `TODO.md`/`ROADMAP.md` na raiz para o
board vivo). O desenho original das 4 camadas está em
`docs/tech/pivot/engine-design.md` (boa parte do conteúdo técnico concreto daquele
documento é da era Qt6 e está superada; o próprio banner do documento explica o que
ainda vale: as 4 camadas e o plano de migração M0-M9).

## Arquitetura de 4 camadas

| Camada | Pasta | Toca framework? | Papel |
| :--- | :--- | :---: | :--- |
| core | `core/` | NÃO | POCO C++ puro, headless, 100% testável (tempo, cripto, player, spatial, animação, utilidades matemáticas, caminhos de asset) |
| domain | `domain/` | NÃO | POCO das regras do jogo (save, i18n, progressão, templates, combate, diálogo) |
| platform | `platform/` | SIM | única fronteira que toca SDL3 / OpenGL / glintfx (RmlUi) / áudio: janela, render2d, input, áudio, arquivos |
| app | `app/` | SIM | executável do jogo: telas, gameplay, ferramentas internas |

**Invariante auditável:** `core/` e `domain/` não incluem nem linkam SDL3, OpenGL ou
glintfx. Só `platform/` e `app/` tocam essas dependências. O CI audita includes que
vazam essa fronteira.

## Targets principais

- `gusengine_core` (STATIC, sem framework)
- `gusengine_domain` (STATIC, sem framework, depende de core)
- `gusengine_platform` (STATIC, linka SDL3 + glintfx/RmlUi)
- `gusengine_app` (STATIC, telas + gameplay)
- `gusworld_app` (executável, linka core + domain + platform + app)
- `gusengine_core_tests` (Catch2 v3, cobre core/ + domain/, headless)

## Build e teste

Os comandos completos de build/run (CMake + Ninja + CMakePresets) estão no
[README da raiz](../README.md). Resumo:

```sh
cd GusEngine
cmake --preset linux-release       # configura (gera build/linux-release/)
cmake --build --preset linux-release
ctest --preset linux-release
```

SDL3, RmlUi, glintfx e Catch2 são baixados e fixados automaticamente via
`FetchContent` (sem instalação manual).

## Testes

Catch2 v3 é o único framework de teste do projeto, cobrindo `core/`, `domain/`,
`platform/` e `app/` de forma headless (sem abrir janela real). Integra ao `ctest`
via `catch_discover_tests`.

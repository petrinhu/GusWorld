# GusEngine

Engine propria do GusWorld em C++20 + Qt6 (pivot pos-Godot). Este diretorio e
o andaime do marco **M0** (ver `docs/tech/pivot/engine-design.md` secao 4).

## Arquitetura de 4 camadas

| Camada | Pasta | Qt? | Papel |
| :--- | :--- | :---: | :--- |
| core | `core/` | NAO | POCO C++ puro, headless, 100% testavel (time, rng, ecs_lite, resource, events) |
| domain | `domain/` | NAO | POCO portado do C# (save, i18n, progression, templates, combat) |
| platform | `platform/` | SIM | fronteira Qt: window, render2d (Qt RHI), input, audio (Qt Multimedia), fs |
| app | `app/` | SIM | executavel GusWorld-specific (screens + main) |

**Invariante auditavel:** `core/` e `domain/` NAO incluem nem linkam Qt. So
`platform/` e `app/` tocam Qt. Os targets `gusengine_core` e `gusengine_domain`
jamais recebem `Qt6::*`. O CI audita includes `<Q...>` nessas duas pastas.

## Estado (M0, andaime)

So existe 1 stub por camada para dar targets ao build. Nenhuma logica de jogo
foi portada (o porte e M3+). O `app/main.cpp` apenas prova que Qt linka.

## Targets

- `gusengine_core` (STATIC, sem Qt)
- `gusengine_domain` (STATIC, sem Qt, depende de core)
- `gusengine_platform` (STATIC, linka Qt6 Core/Gui/Quick/Multimedia)
- `gusworld_app` (executavel, linka core + domain + platform)
- `gusengine_core_tests` (Catch2 v3, headless)

## Build e teste (presets do devops-sre)

Os `CMakePresets.json` (mantidos pela frente devops-sre) cuidam do
`CMAKE_PREFIX_PATH` para o Qt 6.8 LTS. Exemplo Linux:

```sh
cmake --preset linux-release       # configura (Ninja; fallback: linux-release-make)
cmake --build --preset linux-release
ctest --preset linux-release       # roda o teste dummy via ctest
```

Criterio de saida M0: `cmake --build` verde + 1 teste passa via ctest.

## Testes

Catch2 v3 (via FetchContent) para as camadas puras, headless, como a logica
roda hoje sem abrir o Godot. Qt Test entra em M1+ (camada `platform/`).

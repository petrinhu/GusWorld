# GusWorld

> RPG turn-based 2D estilizado. Prodígio-hacker de 11 anos contra megacorporação ciber-gótica.

**Status:** Pivot de stack em curso (Godot/C# para C++/Qt6 com engine própria). Migração faseada anti big-bang, Godot vivo até o decommission no marco M8. Decisão âncora em [`docs/tech/pivot/engine-design.md`](docs/tech/pivot/engine-design.md), ratificada pelo líder em 2026-06-21.

**Solo indie, freeware.** Petrinhu, 2026. Linux + Windows. Single-player puro. C++20 + Qt6.

---

## Pilares criativos (imutáveis)

1. **Magia = software.** Feitiços são scripts rúnicos compilados (Glyph/Token/Conjuro/Codex).
2. **Natureza é matemática rígida**, não caos (Fibonacci, fractais, ruído coerente).
3. **Hardware loop:** Óculos táticos + Matriz Ortodôntica + Tavus-Drive.
4. **Idade canônica 11 anos.** Prodígio analítico, não power-fantasy adulta.
5. **Setting bipartido:** megacidade ciber-gótica versus Selve Sombria.

Detalhes em [`docs/design/pillars.md`](docs/design/pillars.md).

---

## Estrutura

```
gusworld/
├── CLAUDE.md            (estado atual + decisões fechadas)
├── CONTRACT.md          (disciplinas técnicas canon)
├── TODO.md              (backlog canônico via skill tab_pendencias)
├── TESTES.md            (T-sections + A-sections)
├── CHANGELOG.md         (Keep a Changelog)
├── CHARS.md             (inventário canônico de personagens nomeados)
├── PLACES.md            (inventário canônico de lugares nomeados)
├── sinopse.md           (base canônica imutável)
├── docs/
│   ├── design/          (pillars, GDD, mecânicas)
│   ├── narrative/       (lore-bible, characters, factions, timeline + deep-lore)
│   ├── art/             (style guide)
│   └── tech/            (architecture, ADRs, pivot/engine-design.md)
├── GusEngine/           (engine própria C++/Qt6, em 4 camadas)
│   ├── core/            (POCO C++ puro: time, rng, ecs_lite, resource, events)
│   ├── domain/          (POCO C++ puro: save, i18n, progression, templates, combat)
│   ├── platform/        (única fronteira Qt: window, render2d, input, audio, fs)
│   ├── app/             (GusWorld-specific: screens, main)
│   ├── tests/           (Catch2 + Qt Test)
│   ├── CMakeLists.txt
│   └── CMakePresets.json
├── game/                (projeto Godot legado, referência de leitura até M8)
├── engine/              (engine C# legada, referência de leitura até M8)
├── assets/              (sources arte/som: Blender, Krita, Aseprite, audio raw)
└── build/               (outputs export Linux + Windows)
```

A engine antiga (`game/` Godot + `engine/` C#) permanece no repo como referência de leitura durante o porte e é apagada no marco M8 (decommission).

---

## Documentos canônicos

| Doc | Autoridade |
|---|---|
| [CLAUDE.md](CLAUDE.md) | Estado atual + decisões fechadas |
| [CONTRACT.md](CONTRACT.md) | Disciplinas técnicas (RFC 2119, Conventional Commits, branching, DoD, perf budget, a11y) |
| [TODO.md](TODO.md) | Backlog canônico + board do pivot M0-M9 (skill `tab_pendencias`) |
| [TESTES.md](TESTES.md) | Suíte de testes + auditorias (T+A sections) |
| [CHANGELOG.md](CHANGELOG.md) | Histórico de releases (Keep a Changelog) |
| [CHARS.md](CHARS.md) | Inventário canônico de personagens nomeados |
| [PLACES.md](PLACES.md) | Inventário canônico de lugares nomeados |
| [sinopse.md](sinopse.md) | Worldbuilding + protagonista (imutável) |
| [docs/design/pillars.md](docs/design/pillars.md) | 5 pillars canon |
| [docs/design/gdd.md](docs/design/gdd.md) | Game Design Document 1-page |
| [docs/tech/pivot/engine-design.md](docs/tech/pivot/engine-design.md) | Design da engine do pivot C++/Qt6 (fonte do stack atual) |
| [docs/tech/adr/](docs/tech/adr/) | Architecture Decision Records |

---

## Por onde o projeto anda

- [ROADMAP.md](ROADMAP.md): o caminho do projeto (Fase 1 lore concluída, Fase 2 vertical slice em andamento com marcos M0-M9, re-pivot SDL em 3 fases, trilha de arte, pós-VS).
- [CHANGELOG.md](CHANGELOG.md): histórico de mudanças, com destaque para os pivôs de stack (Godot/C# para C++/Qt6 e o re-pivot Qt6 para SDL3).

---

## Build / Run

### Pré-requisitos

- C++20 (GCC, Clang ou MSVC/MinGW)
- Qt6 6.8 LTS (componentes Core, Gui, Quick, Multimedia, Test, ShaderTools)
- CMake + Ninja
- Linux ou Windows
- Git

### Desenvolvimento local

```bash
# Entrar na engine (onde vivem o CMakeLists e o CMakePresets)
cd GusEngine

# Configurar (primeira vez; gera build/linux-release/)
cmake --preset linux-release

# Compilar
cmake --build --preset linux-release

# Rodar o jogo
./build/linux-release/app/gusworld_app

# Rodar a suíte de testes (Catch2 para a lógica + Qt Test para a camada Qt)
ctest --preset linux-release
```

Para Windows, troque o preset por `windows-release`. A lógica de `core/` e `domain/` roda headless (sem abrir janela). Detalhes em [`docs/tech/pivot/engine-design.md`](docs/tech/pivot/engine-design.md).

---

## Tech stack

- **Linguagem:** C++20 (RAII, value semantics, `std::`). Engine própria, sem runtime de terceiros.
- **Framework:** Qt6 6.8 LTS. Único na fronteira `platform/` + `app/`; `core/` + `domain/` são POCO C++ puro (zero Qt, zero I/O real, auditado por grep no CI).
- **Renderer:** Qt RHI (escolhe Vulkan ou OpenGL por GPU) para o mundo + Qt Quick/QML para UI, menus e telas de batalha. 2D-only.
- **Câmera:** ortográfica fixa top-down (clamp ao mapa). Zoom e follow ficam para refinamento futuro (RF-3).
- **Visual:** 2D estilizado, super-deformed (SD) 1:1:1. Pixel art à mão (estilo Zelda A Link to the Past, SNES) ou modelagem 3D no Blender baked para sprite (estilo Stardew Valley, Sea of Stars, Death's Door). O 3D é só ferramenta de produção, nunca runtime.
- **Save format:** binário próprio com criptografia própria (SHA-256 / HMAC, zero dependência externa, validada contra vetores FIPS 180-4 e RFC 4231), migrators forward-only, schema v4, anti-tamper.
- **RNG:** PRNG determinístico seedável e injetável (para save e replay).
- **Localização:** loader próprio + i18n próprio. Dev em pt-br. Tradução en-intl pós-release v1.0.0.
- **Build/Test:** CMake + CMakePresets + Qt6. Testes via `ctest` (Catch2 para a lógica pura, Qt Test para a camada Qt).
- **CI:** Forgejo Actions, matriz Linux + Windows.
- **Plataformas:** Linux (AppImage + tar.gz) + Windows (sem signing em G1).
- **Target hardware:** floor iGPU (Intel HD / AMD integrada, sem GPU dedicada); ceiling RTX 3050 Laptop 4GB.

Motivação do pivot: máxima performance em máquinas modestas. C++ é AOT por natureza, o que elimina toda a complexidade de AOT do .NET da fase anterior (ADR-002).

---

## Pipeline de arte

GusWorld é feito por uma pessoa só, então a produção de assets se apoia num pipeline assistido por IA, do lore ao sprite final. O Claude transforma o lore canônico (lore-bible, character specs) em prompts visuais detalhados e fiéis a cada personagem. Esses prompts alimentam a geração de imagem 2D no [nano banana (Google Gemini)](https://gemini.google.com/) e no [Grok (xAI)](https://grok.com/), que produzem as imagens-base dos personagens. Quando um asset pede volume 3D, o [Tripo3D](https://www.tripo3d.ai/) faz image-to-3D, convertendo a imagem-base num modelo 3D que alimenta o pipeline de bake. Vale o lembrete: **o jogo é 2D** (sprites desenhados pelo Qt RHI em runtime); o 3D existe apenas como ferramenta de produção (modelar ou gerar em 3D, renderizar e converter em sprite 2D), nunca em runtime. Por fim, o [PixelLab](https://www.pixellab.ai/) gera e anima os sprites multi-direção (personagem em várias direções + ciclos de animação a partir de uma imagem). Um agradecimento às camadas gratuitas dessas ferramentas, que ajudam muito um projeto solo e freeware a produzir arte.

---

## Roadmap (pivot C++/Qt6, marcos M0-M9)

Migração faseada anti big-bang. Cada marco fecha pelo seu critério de saída testável; o Godot legado só é apagado no M8, depois que a engine nova provar paridade jogável (M7). Board completo e critérios de saída em [TODO.md](TODO.md); design dos marcos em [`docs/tech/pivot/engine-design.md`](docs/tech/pivot/engine-design.md).

| Marco | Status | Descrição |
|---|---|---|
| M0 (Andaime) | 🔍 Em validação | Repo C++ + CMake + presets + link Qt6 + framework de teste. Build Linux verde + testes ctest passando |
| M1 (Janela + loop + sprite) | 🔍 Em validação | Janela Qt6 + render2d (Qt RHI) + loop de tempo fixo + ponte de input. Boneco-placeholder anda no mapa, câmera ortográfica presa ao mapa |
| M3 (Lógica pura portada) | ✅ Auditado | Save + i18n + progression + templates portados para POCO C++ puro. 174 testes verdes, crypto bate vetores FIPS/RFC, oráculo de save semântico |
| M2 (Input) | 🔍 Lógica feita | Eventos Qt para ações lógicas + porta de input_remap + persistência de controles + save v4. Falta o backend de evento Qt + I/O em disco |
| M4 (Cena top-down) | 🔍 Lógica feita | Tilemap + colisão de grid + clamp de câmera (lógica pura). Falta a parte visual (tilemap render Qt RHI) |
| M5 (Combate portado + tela de batalha) | 🔄 Motor portado, auditado | Motor `turn_combat` portado e endurecido (fórmula de dano §11 evoluída, auditada). Falta a BattleScreen (apresentação estilo Pokémon) |
| M6 (Áudio) | ⏳ Pendente | platform/audio sobre Qt Multimedia + música + SFX + fade entre telas |
| M7 (Paridade jogável) | ⏳ Pendente | Loop completo (andar, NPC, combate, save, carregar) 100% na engine nova, sem Godot |
| M8 (Decommission) | ⏳ Pendente | Apagar Godot + C# + addons. Repo compila e roda sem nenhum bit do stack antigo |
| M9 (Higienização) | ⏳ Pendente | Limpar a árvore pós-porte, remover resíduo do stack antigo, normalizar `GusEngine/` |

Meta: vertical slice jogável (1 área cidade + 1 encontro turn-based + 1 puzzle Vetor do Gambito).

---

## Contribuição

**Solo indie.** Não aceita PRs externos durante G1.

Bug reports + feedback (pós-release) via issues Codeberg: <https://codeberg.org/petrinhu/gusworld/issues>

---

## ☕ Apoie o projeto

GusWorld é **freeware** (de graça, pra sempre). Se curtir e quiser ajudar a tocar o desenvolvimento (inclusive os tokens de IA que ajudam a construir o jogo):

[![Buy me a coffee and some AI tokens](resources/buymecoffe.png)](https://www.paypal.com/donate/?business=9XNZQ4RND67KL&no_recurring=0&currency_code=BRL)

**Buy me a coffee and some AI tokens.** Via [PayPal](https://www.paypal.com/donate/?business=9XNZQ4RND67KL&no_recurring=0&currency_code=BRL) _(totalmente opcional, nunca obrigatório)._

Ou aponte a câmera do celular no QR Code:

![QR Code de doação PayPal](resources/QRCode.png)

---

## Licença

**Código-fonte:** [GNU General Public License v3.0 (GPLv3)](LICENSE), copyleft forte, compatível com Qt (GPL/LGPL) sem custo, inclusive em static-link. _(migrado de AGPL-3.0 para GPLv3 em 2026-06-21, pivot RF-9.)_
**Lore e arte (assets):** [CC-BY-SA-4.0](ASSETS-LICENSE.md), exceto os livros Vol1/Vol2 (direitos reservados, obra à parte). Atribuições de terceiros em [THIRD-PARTY-LICENSES.md](THIRD-PARTY-LICENSES.md).

---

## Créditos

- **Direção criativa + código + arte + narrativa + tudo:** petrinhu (2026)
- **Engine base:** Qt6 (LGPL/GPL). Godot 4 (MIT) permanece como referência de leitura até o decommission no marco M8.
- **Bibliotecas C++ vendorizadas:** libs header-only de licenças permissivas incorporadas em `GusEngine/third_party/` (filosofia zero-dep); lista e licenças em [THIRD-PARTY-LICENSES.md](THIRD-PARTY-LICENSES.md).
- **Geração de imagem 2D:** [nano banana (Google Gemini)](https://gemini.google.com/) + [Grok (xAI)](https://grok.com/), a partir de prompts derivados do lore canônico.
- **Geração 3D (ferramenta de produção):** [Tripo3D](https://www.tripo3d.ai/), image-to-3D para o pipeline de bake 3D-para-sprite. O jogo é 2D em runtime.
- **Pixel art / sprites:** [PixelLab](https://www.pixellab.ai/), gerador de pixel art por IA (personagem multi-direção + animação a partir de imagem). Agradecimento pela generosa camada gratuita, que ajuda muito um projeto solo e freeware a produzir sprites.
- **Lore-bible canon (~365k pal):** Era 1 §§1-10 + R2 Facções + R3 Settings + Bloco F/G/H/I
- **Apoio técnico narrativo:** Squad Claude Code (narrative-writer, narrative-designer, software-architect, etc) sob direção do criador supremo

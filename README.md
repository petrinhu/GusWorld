# GusWorld

> RPG turn-based 2D estilizado. Prodígio-hacker de 11 anos contra megacorporação ciber-gótica.

**Status:** Pivot de stack em curso (Godot/C# para C++20 com engine própria). A camada de plataforma é **SDL3** desde o re-pivot Qt6 para SDL3 ([ADR-008](docs/tech/adr/ADR-008-repivot-qt-to-sdl3.md), 2026-06-22); o Qt6 anterior foi aposentado. Migração faseada anti big-bang, Godot vivo até o decommission no marco M8. Decisão âncora da engine em [`docs/tech/pivot/engine-design.md`](docs/tech/pivot/engine-design.md), ratificada pelo líder em 2026-06-21.

**Solo indie, freeware.** Petrinhu, 2026. Linux + Windows. Single-player puro. C++20 + SDL3.

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
├── GusEngine/           (engine própria C++20, em 4 camadas)
│   ├── core/            (POCO C++ puro: time, rng, ecs_lite, resource, events)
│   ├── domain/          (POCO C++ puro: save, i18n, progression, templates, combat)
│   ├── platform/        (única fronteira SDL3: window, render2d, input, audio, fs)
│   ├── app/             (GusWorld-specific: screens, main)
│   ├── tests/           (Catch2)
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
| [docs/narrative/bibliografia-rag.md](docs/narrative/bibliografia-rag.md) | Bibliografia de inspiração da lore: ~306 obras no RAG principal + corpus élfico no RAG da conlang |
| [docs/tech/pivot/engine-design.md](docs/tech/pivot/engine-design.md) | Design da engine do pivot C++20 (4 camadas; plataforma superada pelo [ADR-008](docs/tech/adr/ADR-008-repivot-qt-to-sdl3.md)) |
| [docs/tech/adr/](docs/tech/adr/) | Architecture Decision Records |

---

## Por onde o projeto anda

- [ROADMAP.md](ROADMAP.md): o caminho do projeto (Fase 1 lore concluída, Fase 2 vertical slice em andamento com marcos M0-M9, re-pivot SDL em 3 fases, trilha de arte, pós-VS).
- [CHANGELOG.md](CHANGELOG.md): histórico de mudanças, com destaque para os pivôs de stack (Godot/C# para C++/Qt6 e o re-pivot Qt6 para SDL3).

---

## Build / Run

### Pré-requisitos

- C++20 (GCC, Clang ou MSVC/MinGW)
- SDL3 + RmlUi (via FetchContent; o CMake baixa e fixa a versão, build reprodutível Linux + Windows)
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

# Rodar a suíte de testes (Catch2)
ctest --preset linux-release
```

Para Windows, troque o preset por `windows-release`. A lógica de `core/` e `domain/` roda headless (sem abrir janela). Detalhes em [`docs/tech/pivot/engine-design.md`](docs/tech/pivot/engine-design.md).

---

## Tech stack

- **Linguagem:** C++20 (RAII, value semantics, `std::`). Engine própria, sem runtime de terceiros.
- **Framework:** SDL3 (janela, loop próprio, input, gamepad nativo, eventos) na fronteira `platform/` + `app/`; `core/` + `domain/` são POCO C++ puro (zero framework, zero I/O real, auditado por grep no CI). UI do jogador via **RmlUi** (HTML/CSS-like, retido, data binding MVC; chega na Fase 3 do re-pivot, marco M5+). Áudio via **miniaudio** (vendorizado em `third_party/`).
- **Renderer:** `SDL_Renderer` (2D, escolhe o backend de GPU disponível) para o mundo; RmlUi desenha sobre o mesmo `render2d` para UI e menus. Tudo atrás de uma interface `IRenderer` (trocar o backend = um arquivo). 2D-only.
- **Câmera:** ortográfica fixa top-down (clamp ao mapa). Zoom e follow ficam para refinamento futuro (RF-3).
- **Visual:** 2D estilizado, super-deformed (SD) 1:1:1. Pixel art à mão (estilo Zelda A Link to the Past, SNES) ou modelagem 3D no Blender baked para sprite (estilo Stardew Valley, Sea of Stars, Death's Door). O 3D é só ferramenta de produção, nunca runtime.
- **Save format:** binário próprio com criptografia própria (SHA-256 / HMAC, zero dependência externa, validada contra vetores FIPS 180-4 e RFC 4231), migrators forward-only, schema v4, anti-tamper.
- **RNG:** PRNG determinístico seedável e injetável (para save e replay).
- **Localização:** loader próprio + i18n próprio. Dev em pt-br. Tradução en-intl pós-release v1.0.0.
- **Build/Test:** CMake + CMakePresets. SDL3 + RmlUi entram via FetchContent (pin de versão); testes via `ctest` (Catch2). A camada de plataforma roda smoke headless com `SDL_VIDEODRIVER=dummy`.
- **CI:** Forgejo Actions, matriz Linux + Windows.
- **Plataformas:** Linux (AppImage + tar.gz) + Windows (sem signing em G1).
- **Target hardware:** floor iGPU (Intel HD / AMD integrada, sem GPU dedicada); ceiling RTX 3050 Laptop 4GB.

Motivação do pivot Godot/C# para C++20: máxima performance em máquinas modestas. C++ é AOT por natureza, o que elimina toda a complexidade de AOT do .NET da fase anterior (ADR-002).

Motivação do re-pivot Qt6 para SDL3 ([ADR-008](docs/tech/adr/ADR-008-repivot-qt-to-sdl3.md)): gamepad nativo de classe AAA (o Qt6 removeu o módulo QtGamepad), binário e deploy ~10x menores (licença zlib), fim do risco do Qt RHI (API semi-privada, agora substituída pelo `SDL_Renderer` público e estável) e portabilidade para mobile e console (caminho fechado no Qt). A lógica pura (`core`/`domain`, ~590 testes auditados) não muda no re-pivot: só a fronteira `platform/` + a casca `app/` foram reescritas.

---

## Pipeline de arte

GusWorld é feito por uma pessoa só, então a produção de assets se apoia num pipeline assistido por IA, do lore ao sprite final. O Claude transforma o lore canônico (lore-bible, character specs) em prompts visuais detalhados e fiéis a cada personagem. Esses prompts alimentam a geração de imagem 2D no [nano banana (Google Gemini)](https://gemini.google.com/) e no [Grok (xAI)](https://grok.com/), que produzem as imagens-base dos personagens. Quando um asset pede volume 3D, o [Tripo3D](https://www.tripo3d.ai/) faz image-to-3D, convertendo a imagem-base num modelo 3D que alimenta o pipeline de bake. Vale o lembrete: **o jogo é 2D** (sprites desenhados pelo `SDL_Renderer` em runtime); o 3D existe apenas como ferramenta de produção (modelar ou gerar em 3D, renderizar e converter em sprite 2D), nunca em runtime. Por fim, o [PixelLab](https://www.pixellab.ai/) gera e anima os sprites multi-direção (personagem em várias direções + ciclos de animação a partir de uma imagem). Um agradecimento às camadas gratuitas dessas ferramentas, que ajudam muito um projeto solo e freeware a produzir arte.

---

## Roadmap (pivot C++20, marcos M0-M9)

Migração faseada anti big-bang. Cada marco fecha pelo seu critério de saída testável; o Godot legado só é apagado no M8, depois que a engine nova provar paridade jogável (M7). Board completo e critérios de saída em [TODO.md](TODO.md); design dos marcos em [`docs/tech/pivot/engine-design.md`](docs/tech/pivot/engine-design.md).

| Marco | Status | Descrição |
|---|---|---|
| M0 (Andaime) | 🔍 Em validação | Repo C++ + CMake + presets + framework de teste. Build Linux verde + testes ctest passando |
| M1 (Janela + loop + sprite) | 🔍 Em validação | Janela SDL3 + render2d (`SDL_Renderer`) + loop de tempo fixo + ponte de input com gamepad. Boneco-placeholder anda no mapa, câmera ortográfica presa ao mapa. Fronteira já reescrita em SDL3 (Fase 1 do re-pivot, [ADR-008](docs/tech/adr/ADR-008-repivot-qt-to-sdl3.md)) |
| M3 (Lógica pura portada) | ✅ Auditado | Save + i18n + progression + templates portados para POCO C++ puro. 174 testes verdes, crypto bate vetores FIPS/RFC, oráculo de save semântico |
| M2 (Input) | 🔍 Lógica feita | Eventos da plataforma para ações lógicas + porta de input_remap + persistência de controles + save v4. Falta o backend de evento (SDL) + I/O em disco |
| M4 (Cena top-down) | 🔍 Lógica feita | Tilemap + colisão de grid + clamp de câmera (lógica pura). Falta a parte visual (tilemap render no `SDL_Renderer`, Fase 2 do re-pivot) |
| M5 (Combate portado + tela de batalha) | 🔄 Motor portado, auditado | Motor `turn_combat` portado e endurecido (fórmula de dano §11 evoluída, auditada). Falta a BattleScreen (apresentação estilo Pokémon) |
| M6 (Áudio) | ⏳ Pendente | platform/audio sobre miniaudio + música + SFX + fade entre telas |
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

**Código-fonte:** [GNU General Public License v3.0 (GPLv3)](LICENSE), copyleft forte. SDL3 (zlib) e RmlUi (MIT) são licenças permissivas, compatíveis com GPLv3 inclusive em static-link. _(migrado de AGPL-3.0 para GPLv3 em 2026-06-21, pivot RF-9.)_
**Lore e arte (assets):** [CC-BY-SA-4.0](ASSETS-LICENSE.md), exceto os livros Vol1/Vol2 (direitos reservados, obra à parte). Atribuições de terceiros em [THIRD-PARTY-LICENSES.md](THIRD-PARTY-LICENSES.md).

---

## Créditos

- **Direção criativa + código + arte + narrativa + tudo:** petrinhu (2026)
- **Engine base:** SDL3 (zlib) + RmlUi (MIT) + miniaudio (MIT-0/PD) na camada de plataforma. Godot 4 (MIT) permanece como referência de leitura até o decommission no marco M8.
- **Bibliotecas C++ vendorizadas:** libs header-only de licenças permissivas incorporadas em `GusEngine/third_party/` (filosofia zero-dep); lista e licenças em [THIRD-PARTY-LICENSES.md](THIRD-PARTY-LICENSES.md).
- **Geração de imagem 2D:** [nano banana (Google Gemini)](https://gemini.google.com/) + [Grok (xAI)](https://grok.com/), a partir de prompts derivados do lore canônico.
- **Geração 3D (ferramenta de produção):** [Tripo3D](https://www.tripo3d.ai/), image-to-3D para o pipeline de bake 3D-para-sprite. O jogo é 2D em runtime.
- **Pixel art / sprites:** [PixelLab](https://www.pixellab.ai/), gerador de pixel art por IA (personagem multi-direção + animação a partir de imagem). Agradecimento pela generosa camada gratuita, que ajuda muito um projeto solo e freeware a produzir sprites.
- **Lore-bible canon (~365k pal):** Era 1 §§1-10 + R2 Facções + R3 Settings + Bloco F/G/H/I
- **Apoio técnico narrativo:** Squad Claude Code (narrative-writer, narrative-designer, software-architect, etc) sob direção do criador supremo

---

## Agradecimentos

Este jogo existe por causa de muita gente. Primeiro, as pessoas:

- **Gus Dragon (meu filho):** inspiração do protagonista, na aparência e nos gostos. Parceiro nas decisões sobre o jogo e meu tester principal.
- **El Iagows (meu irmão):** inspiração do Yakov. Engenheiro de computação que me deu várias dicas de arquitetura e stack, e me orientou no uso de SDL e de spritesheets para o movimento. Criador de uma das melhores libs de rolagem de dados (800+ downloads): [@iagows/3d-dice-ts no npm](https://www.npmjs.com/package/@iagows/3d-dice-ts) ([código no GitLab](https://gitlab.com/iagows/3d-dice-ts)).
- **Od Fuinha Minduim, Thiago MadDog e Thiago Arcanjo:** profissionais de primeira classe em TI, que deram inúmeras dicas de testes, arquitetura, QA, segurança, CI e RAG, e me puseram para estudar.
- **A galera do grupo #metaleiros-PE** (hoje no WhatsApp, uma amizade que vem do mIRC desde ~1997): pela parceria de sempre.
- **Bruno Vettore:** deu a sugestão de criar uma língua para o jogo, numa conversa sobre Tolkien e repórteres intrometidos e inconvenientes, ao ver meus protótipos. O Sylvarin nasceu daí.

E as ferramentas de IA que ajudaram a construir o GusWorld:

- **[Claude Code (Anthropic)](https://claude.com/claude-code):** par de programação e a constelação de agentes ao longo de todo o desenvolvimento. Cerca de 800 milhões de tokens usados até aqui (estimativa), em torno de 15% do projeto concluído antes do lançamento.
- **[Gemini (nano banana)](https://gemini.google.com/) e [Grok (xAI)](https://grok.com/):** geração de imagem conceitual 2D.
- **[Tripo3D](https://www.tripo3d.ai/):** criação de arte conceitual em 3D.

Aos **autores das obras que inspiraram a lore**: de Tolkien aos demais nomes do corpus, cujos livros alimentaram o worldbuilding do GusWorld. A bibliografia completa (cerca de 306 obras no RAG principal, mais o corpus élfico da conlang) está em [docs/narrative/bibliografia-rag.md](docs/narrative/bibliografia-rag.md). O índice de busca semântica reúne **165.432 chunks** (163.443 do corpus principal + 1.989 do élfico), fatiados em ~2000 caracteres com 500 de sobreposição (janela de interposição), via `bge-m3` (1024 dimensões) + LanceDB. Nenhum texto foi copiado: serviram de inspiração e referência.

Por fim, as ferramentas livres e as inspirações que sustentam o projeto:

- **Engine e libs FOSS:** [SDL3](https://www.libsdl.org/), [RmlUi](https://github.com/mikke89/RmlUi), [miniaudio](https://miniaud.io/), [Catch2](https://github.com/catchorg/Catch2) e as bibliotecas header-only vendorizadas em `GusEngine/third_party/` (lista e licenças em [THIRD-PARTY-LICENSES.md](THIRD-PARTY-LICENSES.md)).
- **Ferramentas de produção:** [Blender](https://www.blender.org/), [PixelLab](https://www.pixellab.ai/) (sprites) e o stack local de busca da lore ([Ollama](https://ollama.com/) + bge-m3, [LanceDB](https://lancedb.com/), `rag_maker`).
- **Inspirações de design** (homenagem, nada copiado): Chrono Trigger, The Legend of Zelda: A Link to the Past (SNES), Stardew Valley, Sea of Stars, Sable e Death's Door.

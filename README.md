# GusWorld

> A stylized turn-based 2D RPG: an 11-year-old hacker prodigy against a cyber-gothic megacorporation.

**Solo indie, freeware.** Built by petrinhu, started 2026-05-15. C++20 + SDL3, with a small engine written from scratch (`GusEngine/`). Currently a **vertical slice in active development**: see [Status](#status) below.

---

## Table of contents

- [About](#about)
- [Status](#status)
- [Creative pillars](#creative-pillars)
- [Tech stack](#tech-stack)
- [Building and running](#building-and-running)
- [Roadmap](#roadmap)
- [Documentation](#documentation)
- [Contributing](#contributing)
- [Support the project](#support-the-project)
- [License](#license)
- [Credits and acknowledgments](#credits-and-acknowledgments)

---

## About

GusWorld is a single-player, turn-based RPG with puzzle and adventure elements. You play as Gus, an 11-year-old analytical prodigy, navigating a world split between a cyber-gothic megacity and a wild, mathematically precise forest called the Selve Sombria. Combat, exploration and puzzles all lean on the same idea: the world is a system to be understood and reasoned about, not brute-forced.

It's a one-person project: design, code, art direction and writing are all done by a single solo developer, with an AI-assisted production pipeline for art and a lot of help from the open-source ecosystem (see [Credits and acknowledgments](#credits-and-acknowledgments)).

**Historical figures:** GusWorld reimagines real, deceased historical figures (Faraday, Ada Lovelace, Hayek, and others) as fictional in-game characters, plus one fictional character inspired by a living contemporary figure. See the [Fiction disclaimer and methodology](docs/design/roster-analogos/OBRA-DE-FICCAO-E-METODOLOGIA.md) notice for what this is, and isn't.

## Status

GusWorld is **in active development**, currently building its first **vertical slice**: one small city area, one turn-based combat encounter, and one puzzle, all playable end-to-end on the new C++/SDL3 engine. The project has already gone through two stack pivots (from Godot/C# to Qt6, then from Qt6 to SDL3) as the team (of one) converged on the stack that best serves the performance target: smooth play on modest, integrated-GPU hardware.

The full milestone board (M0 through M9) lives in [ROADMAP.md](ROADMAP.md).

## Creative pillars

Five pillars keep the design honest:

1. **Magic is software.** Spells are compiled runic scripts; in-world programming languages follow real low-level programming analogies.
2. **Nature is rigid math, not chaos.** Flora and fauna in the Selve Sombria follow sequences, fractals and recursive functions.
3. **Abilities are tied to hardware.** Tactical glasses, an orthodontic antenna, and a wrist-mounted card executor form a closed triangle of gear-driven abilities.
4. **Age 11 is canon.** Gus is an analytical prodigy, not an adult power fantasy: he solves problems with logic (chess, TCGs, optimization), not brute force.
5. **A world split in two.** A cyber-gothic megacity versus the Selve Sombria, kept in deliberate visual and tonal contrast.

Full design detail lives in [`docs/design/pillars.md`](docs/design/pillars.md).

## Tech stack

GusWorld is built on a small, deliberately chosen set of open-source technologies:

- **[SDL3](https://www.libsdl.org/)**: window, main loop, input, gamepad and audio device, at the single platform boundary of the engine.
- **[glintfx](https://codeberg.org/petrinhu/glintfx)** ([GitHub](https://github.com/petrinhu/glintfx)): the UI/HUD engine, used in embed mode. It wraps [RmlUi](https://github.com/mikke89/RmlUi) 6.3 and a GL3 backend behind a clean, compose-only facade, so HTML/CSS-like menus and HUD (gradients, glow, data binding) render straight over the game's own OpenGL context.
- **[RmlUi](https://github.com/mikke89/RmlUi)**: the retained-mode UI library underneath glintfx, by [Michael R. P. Ragazzon (mikke89)](https://github.com/mikke89). See [Credits and acknowledgments](#credits-and-acknowledgments) for why we're so happy to be building on it.
- **[miniaudio](https://miniaud.io/)**: single-header audio engine (music, SFX, fades), vendored in `GusEngine/third_party/`.
- **[Monocypher](https://monocypher.org/)**: modern, auditable cryptography (AEAD) protecting save files.
- **[Catch2](https://github.com/catchorg/Catch2)**: the test framework behind the engine's automated test suite.
- A curated **toolkit of modern, header-only C++ libraries** ([glm](https://github.com/g-truc/glm), [EnTT](https://github.com/skypjack/entt), [Box2D](https://github.com/erincatto/box2d), [stb](https://github.com/nothings/stb), [fmt](https://github.com/fmtlib/fmt), and more) vendored in `GusEngine/third_party/`.

The full story (what each dependency does, its license, and a proper thank-you) is in [`ACKNOWLEDGMENTS.md`](ACKNOWLEDGMENTS.md) and on the wiki's [Tech Stack & Credits](https://codeberg.org/petrinhu/gusworld/wiki/Tech-Stack-and-Credits) page.

The engine (`GusEngine/`) is organized in four layers: `core/` and `domain/` are plain, framework-free C++ (POCO); `platform/` is the only layer that touches SDL3; `app/` is the GusWorld-specific game and screens layer.

## Building and running

Prerequisites: a C++20 compiler (GCC, Clang or MSVC/MinGW), CMake, Ninja, and Git. SDL3, RmlUi, glintfx and Catch2 are fetched and pinned automatically by CMake (`FetchContent`), no manual install needed for those.

```bash
cd GusEngine

# Configure (first run; generates build/linux-release/)
cmake --preset linux-release

# Build
cmake --build --preset linux-release

# Run the game
./build/linux-release/app/gusworld_app

# Run the test suite
ctest --preset linux-release
```

Linux is the target platform for the v1.0.0 release; a Windows preset exists (`windows-release`) and is planned for a post-v1.0.0 release. Full details and troubleshooting on the wiki's [Building and Running](https://codeberg.org/petrinhu/gusworld/wiki/Building-and-Running) page.

## Roadmap

The project moves through a milestone board (M0 to M9) covering the engine port, gameplay loop, combat, audio and final polish, on the way to the vertical slice and beyond. See [ROADMAP.md](ROADMAP.md) for the full board and [CHANGELOG.md](CHANGELOG.md) for what has already shipped.

## Documentation

- [ROADMAP.md](ROADMAP.md): where the project stands and where it's going.
- [CHANGELOG.md](CHANGELOG.md): release history (Keep a Changelog format).
- [`docs/design/pillars.md`](docs/design/pillars.md): the five creative pillars.
- [`docs/tech/adr/`](docs/tech/adr/): architecture decision records behind the engine's design choices.
- Wiki: [Home](https://codeberg.org/petrinhu/gusworld/wiki/Home) · [Building and Running](https://codeberg.org/petrinhu/gusworld/wiki/Building-and-Running) · [Tech Stack & Credits](https://codeberg.org/petrinhu/gusworld/wiki/Tech-Stack-and-Credits) · [Contributing](https://codeberg.org/petrinhu/gusworld/wiki/Contributing).

## Contributing

GusWorld is a **solo project** and isn't accepting external pull requests during this development phase. Bug reports and feedback are always welcome via [Codeberg issues](https://codeberg.org/petrinhu/gusworld/issues). See the wiki's [Contributing](https://codeberg.org/petrinhu/gusworld/wiki/Contributing) page for details.

**Issue policy:** only **technical** issues are accepted (bugs, build problems, crashes, documentation errors). Issues of a political, economic, philosophical, social or similar nature will be **closed or deleted without reply**. GusWorld is a work of fiction (see the [Fiction disclaimer](docs/design/roster-analogos/OBRA-DE-FICCAO-E-METODOLOGIA.md)); its design documents are not an invitation to real-world debate.

## Support the project

GusWorld is **freeware** (free, forever). If you enjoy it and want to help keep development going (including the AI tooling that helps build the game):

[![Buy me a coffee and some AI tokens](resources/buymecoffe.png)](https://www.paypal.com/donate/?business=9XNZQ4RND67KL&no_recurring=0&currency_code=BRL)

**Buy me a coffee and some AI tokens.** Via [PayPal](https://www.paypal.com/donate/?business=9XNZQ4RND67KL&no_recurring=0&currency_code=BRL) *(entirely optional, never required)*.

Or scan the QR code with your phone's camera:

![PayPal donation QR code](resources/QRCode.png)

## License

**Source code:** [GNU General Public License v3.0 (GPLv3)](LICENSE), strong copyleft. SDL3 (zlib) is permissive; glintfx (MPL-2.0, weak copyleft per-file) wraps RmlUi 6.3 (MIT); all of it is compatible with GPLv3, including static linking.

**Lore and art (assets):** [CC-BY-SA-4.0](ASSETS-LICENSE.md), except the Vol. 1 / Vol. 2 books (all rights reserved, a separate work). Full third-party attributions in [THIRD-PARTY-LICENSES.md](THIRD-PARTY-LICENSES.md).

## Credits and acknowledgments

This game exists because of many people. First, **the people**:

- **Gus Dragon (my son):** inspiration for the protagonist, in looks and tastes. My partner in game decisions and my lead tester.
- **El Iagows (my brother):** inspiration for Yakov. A computer engineer who gave me plenty of architecture and stack advice and guided me on using SDL and spritesheets for movement. Author of one of the best dice-rolling libraries (800+ downloads): [@iagows/3d-dice-ts on npm](https://www.npmjs.com/package/@iagows/3d-dice-ts) ([code on GitLab](https://gitlab.com/iagows/3d-dice-ts)).
- **Od Fuinha Minduim, Thiago MadDog and Thiago Arcanjo:** first-class IT professionals who gave countless tips on testing, architecture, QA, security, CI and RAG, and got me studying.
- **The #metaleiros-PE crew** (now on WhatsApp, a friendship going back to mIRC since ~1997): for the lasting camaraderie.
- **Bruno Vettore:** suggested creating a language for the game, in a conversation about Tolkien and nosy, inconvenient reporters, after seeing my prototypes. Sylvarin was born from that. He also inspired the character **Brunus "Vetorial" Solveckt**, the itinerant apothecary-physician who mentors Gus.

And the **technical credits and tools**:

- **Creative direction, code, art and writing:** petrinhu (2026).
- **Engine foundation:** SDL3 (zlib) + glintfx (MPL-2.0, UI/HUD engine wrapping RmlUi 6.3 MIT + GL3 backend) + miniaudio (Public Domain / MIT-0) at the platform layer.
- **Vendored C++ libraries:** header-only libraries under permissive licenses, listed with full attribution in [THIRD-PARTY-LICENSES.md](THIRD-PARTY-LICENSES.md).
- **2D image generation:** [Gemini (nano banana)](https://gemini.google.com/) and [Grok (xAI)](https://grok.com/), from prompts derived from the game's lore.
- **3D generation (production tool only):** [Tripo3D](https://www.tripo3d.ai/), image-to-3D for the bake-to-sprite pipeline. The game itself is 2D at runtime.
- **Pixel art / sprites:** [PixelLab](https://www.pixellab.ai/), AI-assisted pixel art generation (multi-direction characters + animation from a single image).
- **AI pair programming:** [Claude Code (Anthropic)](https://claude.com/claude-code), used throughout development.
- **Design inspirations** (homage, nothing copied): Chrono Trigger, The Legend of Zelda: A Link to the Past, Stardew Valley, Sea of Stars, Sable, and Death's Door.

The full acknowledgments (including a proper thank-you to the RmlUi and glintfx projects that make the game's UI possible) live in [`ACKNOWLEDGMENTS.md`](ACKNOWLEDGMENTS.md).

---

## 🇧🇷 Português

### GusWorld

> Um RPG 2D estilizado com combate por turnos: um prodígio-hacker de 11 anos contra uma megacorporação ciber-gótica.

**Solo indie, freeware.** Feito por petrinhu, iniciado em 2026-05-15. C++20 + SDL3, com uma engine própria escrita do zero (`GusEngine/`). Atualmente um **vertical slice em desenvolvimento ativo**: veja [Status](#status-1) abaixo.

### Sumário

- [Sobre](#sobre)
- [Status](#status-1)
- [Pilares criativos](#pilares-criativos)
- [Stack técnica](#stack-técnica)
- [Build e execução](#build-e-execução)
- [Roadmap](#roadmap-1)
- [Documentação](#documentação)
- [Contribuindo](#contribuindo)
- [Apoie o projeto](#apoie-o-projeto)
- [Licença](#licença)
- [Créditos e agradecimentos](#créditos-e-agradecimentos)

### Sobre

GusWorld é um RPG single-player com combate por turnos e elementos de puzzle e aventura. Você joga como Gus, um prodígio analítico de 11 anos, navegando por um mundo dividido entre uma megacidade ciber-gótica e uma floresta selvagem e matematicamente precisa chamada Selve Sombria. Combate, exploração e puzzles seguem a mesma ideia: o mundo é um sistema para entender e raciocinar, não para vencer na força bruta.

É um projeto de uma pessoa só: design, código, direção de arte e roteiro são feitos por um único desenvolvedor solo, com um pipeline de produção de arte assistido por IA e muita ajuda do ecossistema open-source (veja [Créditos e agradecimentos](#créditos-e-agradecimentos)).

**Figuras históricas:** GusWorld reimagina figuras históricas reais e falecidas (Faraday, Ada Lovelace, Hayek, entre outras) como personagens ficcionais in-game, mais um personagem fictício inspirado numa figura contemporânea viva. Veja o aviso de [Ficção e metodologia](docs/design/roster-analogos/OBRA-DE-FICCAO-E-METODOLOGIA.md) para entender o que isso é, e o que não é.

### Status

GusWorld está **em desenvolvimento ativo**, construindo agora seu primeiro **vertical slice**: uma pequena área de cidade, um encontro de combate por turnos e um puzzle, tudo jogável de ponta a ponta na nova engine C++/SDL3. O projeto já passou por dois pivôs de stack (de Godot/C# para Qt6, depois de Qt6 para SDL3) enquanto o time (de uma pessoa) convergia para a stack que melhor serve a meta de performance: rodar bem em hardware modesto, com GPU integrada.

O board completo de marcos (M0 a M9) vive em [ROADMAP.md](ROADMAP.md).

### Pilares criativos

Cinco pilares mantêm o design coerente:

1. **Magia é software.** Feitiços são scripts rúnicos compilados; as linguagens de programação in-world seguem analogias reais de programação de baixo nível.
2. **Natureza é matemática rígida, não caos.** Flora e fauna da Selve Sombria seguem sequências, fractais e funções recursivas.
3. **Habilidades acopladas a hardware.** Óculos táticos, uma antena ortodôntica e um executor de cartões no pulso formam um triângulo fechado de habilidades ligadas ao equipamento.
4. **Idade 11 anos é canônica.** Gus é um prodígio analítico, não uma fantasia de poder adulta: ele resolve problemas com lógica (xadrez, TCGs, otimização), não com força bruta.
5. **Um mundo dividido em dois.** Megacidade ciber-gótica versus Selve Sombria, mantidas em contraste visual e tonal deliberado.

Detalhes completos de design em [`docs/design/pillars.md`](docs/design/pillars.md).

### Stack técnica

GusWorld é construído sobre um conjunto pequeno e deliberado de tecnologias open-source:

- **[SDL3](https://www.libsdl.org/)**: janela, loop principal, input, gamepad e dispositivo de áudio, na única fronteira de plataforma da engine.
- **[glintfx](https://codeberg.org/petrinhu/glintfx)** ([GitHub](https://github.com/petrinhu/glintfx)): o motor de UI/HUD, usado em embed mode. Ele embrulha o [RmlUi](https://github.com/mikke89/RmlUi) 6.3 e um backend GL3 atrás de uma fachada limpa e compose-only, permitindo que menus e HUD HTML/CSS-like (degradês, glow, data binding) renderizem direto sobre o mesmo contexto OpenGL do jogo.
- **[RmlUi](https://github.com/mikke89/RmlUi)**: a biblioteca de UI retida por trás do glintfx, de [Michael R. P. Ragazzon (mikke89)](https://github.com/mikke89). Veja [Créditos e agradecimentos](#créditos-e-agradecimentos) pra entender por que estamos tão felizes de construir em cima dela.
- **[miniaudio](https://miniaud.io/)**: motor de áudio single-header (música, SFX, fades), vendorizado em `GusEngine/third_party/`.
- **[Monocypher](https://monocypher.org/)**: criptografia moderna e auditável (AEAD) protegendo os arquivos de save.
- **[Catch2](https://github.com/catchorg/Catch2)**: o framework de testes por trás da suíte de testes automatizados da engine.
- Um **kit selecionado de bibliotecas C++ modernas, header-only** ([glm](https://github.com/g-truc/glm), [EnTT](https://github.com/skypjack/entt), [Box2D](https://github.com/erincatto/box2d), [stb](https://github.com/nothings/stb), [fmt](https://github.com/fmtlib/fmt), entre outras) vendorizadas em `GusEngine/third_party/`.

A história completa (o que cada dependência faz, sua licença, e um agradecimento como manda o figurino) está em [`ACKNOWLEDGMENTS.md`](ACKNOWLEDGMENTS.md) e na página [Tech Stack & Credits](https://codeberg.org/petrinhu/gusworld/wiki/Tech-Stack-and-Credits) da wiki.

A engine (`GusEngine/`) é organizada em quatro camadas: `core/` e `domain/` são C++ puro, sem framework (POCO); `platform/` é a única camada que toca SDL3; `app/` é a camada de jogo e telas específica do GusWorld.

### Build e execução

Pré-requisitos: um compilador C++20 (GCC, Clang ou MSVC/MinGW), CMake, Ninja e Git. SDL3, RmlUi, glintfx e Catch2 são baixados e fixados automaticamente pelo CMake (`FetchContent`), sem instalação manual necessária pra eles.

```bash
cd GusEngine

# Configurar (primeira vez; gera build/linux-release/)
cmake --preset linux-release

# Compilar
cmake --build --preset linux-release

# Rodar o jogo
./build/linux-release/app/gusworld_app

# Rodar a suíte de testes
ctest --preset linux-release
```

Linux é a plataforma alvo do lançamento v1.0.0; existe um preset Windows (`windows-release`) planejado para um lançamento pós-v1.0.0. Detalhes completos e resolução de problemas na página [Building and Running](https://codeberg.org/petrinhu/gusworld/wiki/Building-and-Running) da wiki.

### Roadmap

O projeto avança por um board de marcos (M0 a M9) cobrindo o porte da engine, o loop de jogo, combate, áudio e polish final, a caminho do vertical slice e além. Veja [ROADMAP.md](ROADMAP.md) para o board completo e [CHANGELOG.md](CHANGELOG.md) para o que já foi lançado.

### Documentação

- [ROADMAP.md](ROADMAP.md): onde o projeto está e para onde vai.
- [CHANGELOG.md](CHANGELOG.md): histórico de lançamentos (formato Keep a Changelog).
- [`docs/design/pillars.md`](docs/design/pillars.md): os cinco pilares criativos.
- [`docs/tech/adr/`](docs/tech/adr/): registros de decisão de arquitetura por trás das escolhas de design da engine.
- Wiki: [Home](https://codeberg.org/petrinhu/gusworld/wiki/Home) · [Building and Running](https://codeberg.org/petrinhu/gusworld/wiki/Building-and-Running) · [Tech Stack & Credits](https://codeberg.org/petrinhu/gusworld/wiki/Tech-Stack-and-Credits) · [Contributing](https://codeberg.org/petrinhu/gusworld/wiki/Contributing).

### Contribuindo

GusWorld é um **projeto solo** e não aceita pull requests externos nesta fase de desenvolvimento. Relatos de bug e feedback são sempre bem-vindos via [issues do Codeberg](https://codeberg.org/petrinhu/gusworld/issues). Veja a página [Contributing](https://codeberg.org/petrinhu/gusworld/wiki/Contributing) da wiki para detalhes.

**Política de issues:** somente issues **técnicos** são aceitos (bugs, problemas de build, crashes, erros de documentação). Issues de fundo político, econômico, filosófico, social ou de natureza similar serão **fechados ou excluídos sem resposta**. GusWorld é uma obra de ficção (veja o [Aviso de Ficção](docs/design/roster-analogos/OBRA-DE-FICCAO-E-METODOLOGIA.md)); seus documentos de design não são um convite a debate de mundo real.

### Apoie o projeto

GusWorld é **freeware** (de graça, pra sempre). Se você curte o projeto e quer ajudar a manter o desenvolvimento (inclusive os tokens de IA que ajudam a construir o jogo):

[![Buy me a coffee and some AI tokens](resources/buymecoffe.png)](https://www.paypal.com/donate/?business=9XNZQ4RND67KL&no_recurring=0&currency_code=BRL)

**Buy me a coffee and some AI tokens.** Via [PayPal](https://www.paypal.com/donate/?business=9XNZQ4RND67KL&no_recurring=0&currency_code=BRL) *(totalmente opcional, nunca obrigatório)*.

Ou aponte a câmera do celular no QR Code:

![QR Code de doação PayPal](resources/QRCode.png)

### Licença

**Código-fonte:** [GNU General Public License v3.0 (GPLv3)](LICENSE), copyleft forte. SDL3 (zlib) é permissiva; o glintfx (MPL-2.0, copyleft fraco por arquivo) embrulha o RmlUi 6.3 (MIT); tudo compatível com GPLv3, inclusive em link estático.

**Lore e arte (assets):** [CC-BY-SA-4.0](ASSETS-LICENSE.md), exceto os livros Vol. 1 / Vol. 2 (direitos reservados, obra à parte). Atribuições completas de terceiros em [THIRD-PARTY-LICENSES.md](THIRD-PARTY-LICENSES.md).

### Créditos e agradecimentos

Este jogo existe por causa de muita gente. Primeiro, **as pessoas**:

- **Gus Dragon (meu filho):** inspiração do protagonista, na aparência e nos gostos. Parceiro nas decisões sobre o jogo e meu tester principal.
- **El Iagows (meu irmão):** inspiração do Yakov. Engenheiro de computação que me deu várias dicas de arquitetura e stack, e me orientou no uso de SDL e de spritesheets para o movimento. Criador de uma das melhores libs de rolagem de dados (800+ downloads): [@iagows/3d-dice-ts no npm](https://www.npmjs.com/package/@iagows/3d-dice-ts) ([código no GitLab](https://gitlab.com/iagows/3d-dice-ts)).
- **Od Fuinha Minduim, Thiago MadDog e Thiago Arcanjo:** profissionais de primeira classe em TI, que deram inúmeras dicas de testes, arquitetura, QA, segurança, CI e RAG, e me puseram para estudar.
- **A galera do grupo #metaleiros-PE** (hoje no WhatsApp, uma amizade que vem do mIRC desde ~1997): pela parceria de sempre.
- **Bruno Vettore:** deu a sugestão de criar uma língua para o jogo, numa conversa sobre Tolkien e repórteres intrometidos e inconvenientes, ao ver meus protótipos. O Sylvarin nasceu daí. Também inspirou o personagem **Brunus "Vetorial" Solveckt**, o boticário-médico itinerante mentor do Gus.

E os **créditos técnicos e ferramentas**:

- **Direção criativa, código, arte e roteiro:** petrinhu (2026).
- **Base da engine:** SDL3 (zlib) + glintfx (MPL-2.0, motor de UI/HUD que embrulha RmlUi 6.3 MIT + backend GL3) + miniaudio (Domínio Público / MIT-0) na camada de plataforma.
- **Bibliotecas C++ vendorizadas:** bibliotecas header-only sob licenças permissivas, listadas com atribuição completa em [THIRD-PARTY-LICENSES.md](THIRD-PARTY-LICENSES.md).
- **Geração de imagem 2D:** [Gemini (nano banana)](https://gemini.google.com/) e [Grok (xAI)](https://grok.com/), a partir de prompts derivados da lore do jogo.
- **Geração 3D (só ferramenta de produção):** [Tripo3D](https://www.tripo3d.ai/), image-to-3D para o pipeline de bake-para-sprite. O jogo em si é 2D em runtime.
- **Pixel art / sprites:** [PixelLab](https://www.pixellab.ai/), geração de pixel art assistida por IA (personagens multi-direção + animação a partir de uma única imagem).
- **Par de programação IA:** [Claude Code (Anthropic)](https://claude.com/claude-code), usado ao longo de todo o desenvolvimento.
- **Inspirações de design** (homenagem, nada copiado): Chrono Trigger, The Legend of Zelda: A Link to the Past, Stardew Valley, Sea of Stars, Sable e Death's Door.

Os agradecimentos completos (incluindo um agradecimento como manda o figurino aos projetos RmlUi e glintfx que tornam a UI do jogo possível) vivem em [`ACKNOWLEDGMENTS.md`](ACKNOWLEDGMENTS.md).

---

*Last updated / última atualização: 2026-07-10.*

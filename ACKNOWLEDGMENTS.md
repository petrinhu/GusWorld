# Acknowledgments

GusWorld is a solo indie project, but it is not built alone. It stands on a small, carefully chosen stack of open-source software, and this page exists to thank the people and projects behind it, properly and specifically, not as a generic list of logos.

If you maintain one of the projects below and you're reading this: thank you. This game would not exist in its current form without your work.

---

## Core engine and platform

### SDL3

[SDL3](https://www.libsdl.org/) (`github.com/libsdl-org/SDL`, zlib license) is the platform boundary of the whole engine: it owns the window, the main loop, input, gamepad support and the audio device. GusWorld pulls it in via CMake `FetchContent`, pinned to a specific release, and links it statically. SDL has been the backbone of open, portable game development for over two decades; a stable, well-documented, permissively licensed library at this layer is exactly what a small solo project needs. Thank you to Sam Lantinga and the whole SDL community for keeping it alive, modern and free.

### glintfx

[glintfx](https://github.com/petrinhu/glintfx) (MPL-2.0) is the UI/HUD engine GusWorld uses in embed mode, through `glintfx::UiLayer`. It wraps RmlUi 6.3 and an OpenGL GL3 backend behind a clean, compose-only facade: it never clears or swaps the framebuffer on its own, it composes the game's menus and HUD directly over the same GL context the world is rendered in, saving and restoring GL state around itself. That single design choice is what lets GusWorld have HTML/CSS-like UI (gradients, glow effects, data binding) without fighting a second rendering pipeline. Building glintfx is its own project, developed alongside GusWorld specifically to make this kind of UI possible for a small C++/SDL3 game. Full credit and thanks for that work.

### RmlUi

[RmlUi](https://github.com/mikke89/RmlUi) (MIT license, documentation at [mikke89.github.io/RmlUiDoc](https://mikke89.github.io/RmlUiDoc/)) is the library that glintfx wraps, and it deserves a paragraph of its own.

RmlUi is a retained-mode, HTML/CSS-like UI library for games and applications, and it is, frankly, one of the best-engineered pieces of open-source UI tooling in the C++ game space. The document/element model is elegant and genuinely easy to reason about once you've worked with it: a real DOM, real CSS-like styling, real data binding (the built-in MVC-style data model), instead of the immediate-mode juggling that most game UIs are stuck with. The decorator and effect system (gradients, blur, drop shadows, masks, and more, all composable via RCSS) gives a small team the kind of visual polish that would otherwise take a dedicated UI programmer to build from scratch. The documentation is thorough and the codebase has stayed maintained, coherent and genuinely pleasant to integrate against for years, which is rare for a project of this scope.

GusWorld's entire player-facing UI (menus, the battle cockpit, dialogue, HUD) exists because RmlUi exists. **Thank you, Michael R. P. Ragazzon (mikke89), for building and maintaining RmlUi.** It is exactly the kind of quiet, high-quality open-source infrastructure that makes projects like this one possible, and we're grateful to be building on it.

---

## Audio

### miniaudio

[miniaudio](https://miniaud.io/) (`github.com/mackron/miniaudio`, public domain / MIT-0, by David Reid) is still the audio engine behind GusWorld's music, SFX and cross-fades, but it no longer enters the project directly. Since 2026-07-22 the game's audio goes through `glintfx::Audio`, and miniaudio comes in transitively as the implementation glintfx vendors and compiles on its own; GusWorld's `AudioEngine` facade sits on top of that and never touches a `ma_*` type. The credit stands either way, and so does the reason miniaudio won in the first place: a single-header audio engine with no build system to fight and no dynamic linking to configure is exactly the kind of dependency a small solo project loves. Thank you, David Reid, for putting this level of audio engineering behind such a simple, generous license.

*Historical note: miniaudio used to be vendored directly in `GusEngine/third_party/miniaudio/`. That copy was removed in the M9 cleanup (2026-07-22), once `glintfx::Audio` had taken over the device.*

---

## Cryptography

### Monocypher

[Monocypher](https://monocypher.org/) (CC0 / BSD-2-Clause dual license, by Loup Vaillant) provides the modern AEAD cryptography (XChaCha20-Poly1305) that protects GusWorld's save files against tampering and corruption. It's vendored in `GusEngine/third_party/monocypher/`. Monocypher's whole design philosophy, a small, auditable, dependency-free implementation of modern primitives, is precisely what a save-file format for a small game needs: strong guarantees without pulling in a heavyweight crypto stack. Thank you, Loup Vaillant, for making serious cryptography approachable and auditable for projects like this one.

---

## Testing

### Catch2

[Catch2](https://github.com/catchorg/Catch2) is the test framework behind GusWorld's engine test suite (`ctest` + Catch2 across `core/`, `domain/`, `platform/` and `app/`). Its expressive assertion style and low ceremony make it easy to keep a solo project's tests honest. Thanks to the Catch2 maintainers and contributors.

---

## C++ toolkit

Beyond the platform-critical dependencies above, GusWorld vendors C++ source under `GusEngine/third_party/`, each library under its own permissive license (see [THIRD-PARTY-LICENSES.md](THIRD-PARTY-LICENSES.md) for the exact licenses). The list is short on purpose, and today it has exactly two entries:

- **[stb](https://github.com/nothings/stb)** (MIT / public domain), Sean Barrett's single-header libraries: image loading, writing, and TrueType rasterization, used by the render2d and font-atlas layers. A huge part of the indie game dev world runs on stb in one way or another.
- **[Monocypher](https://monocypher.org/)** (CC0 / BSD-2-Clause), thanked in full in the Cryptography section above, and the only vendored dependency with a build of its own.

*Historical note: an earlier round of vendoring (2026-06-22) pulled in a much larger "just in case" toolkit of header-only libraries, among them [glm](https://github.com/g-truc/glm) (MIT), [EnTT](https://github.com/skypjack/entt) (MIT), [Box2D](https://github.com/erincatto/box2d) (MIT) and [fmt](https://github.com/fmtlib/fmt) (MIT). Thirty-one of those libraries had no remaining `#include` outside `third_party/` by the time the M9 cleanup came around, and were removed from the repository on 2026-07-22. They are named here as history, not as current dependencies. The thanks to their authors stands anyway: being able to try a library, decide it is not needed, and drop it without friction is itself part of what permissive, header-only C++ gives a small team.*

*Historical note (2026-07-30): the stb vendor also carried `stb_rect_pack.h` (rectangle packing) and `stb_image_resize2.h` (image resizing), both single-header files from the same project. Neither ever had a consumer in the codebase, and both were removed as part of the migration-plan cleanup. The thanks to Sean Barrett for the whole stb collection stands regardless of which single-header files we happen to use.*

---

## Closing note

GusWorld is one person's project, but it's built on the shoulders of a lot of generous, careful open-source engineering. To everyone named on this page, and to everyone in the projects above who reviewed a pull request, wrote a doc page, or answered an issue: thank you.

If GusWorld ever helps someone discover one of these projects, that's a small way of paying it forward.

---

## 🇧🇷 Português

### Agradecimentos

GusWorld é um projeto indie solo, mas não é construído sozinho. Ele se apoia numa stack pequena e cuidadosamente escolhida de software open-source, e esta página existe para agradecer as pessoas e projetos por trás dela, de forma específica e sincera, não como uma lista genérica de logos.

Se você mantém um dos projetos abaixo e está lendo isto: obrigado. Este jogo não existiria na forma atual sem o seu trabalho.

### Engine e plataforma centrais

#### SDL3

[SDL3](https://www.libsdl.org/) (`github.com/libsdl-org/SDL`, licença zlib) é a fronteira de plataforma de toda a engine: cuida da janela, do loop principal, do input, do suporte a gamepad e do dispositivo de áudio. O GusWorld traz a lib via CMake `FetchContent`, fixada numa versão específica, e linka estaticamente. A SDL é a espinha dorsal do desenvolvimento de jogos aberto e portável há mais de duas décadas; uma biblioteca estável, bem documentada e sob licença permissiva nesta camada é exatamente o que um pequeno projeto solo precisa. Obrigado a Sam Lantinga e a toda a comunidade SDL por mantê-la viva, moderna e livre.

#### glintfx

O [glintfx](https://github.com/petrinhu/glintfx) (MPL-2.0) é o motor de UI/HUD que o GusWorld usa em embed mode, via `glintfx::UiLayer`. Ele embrulha o RmlUi 6.3 e um backend OpenGL GL3 atrás de uma fachada limpa e compose-only: nunca limpa nem troca o framebuffer sozinho, compõe os menus e o HUD do jogo diretamente sobre o mesmo contexto GL em que o mundo é renderizado, salvando e restaurando o estado GL ao redor de si mesmo. Essa única decisão de design é o que permite ao GusWorld ter uma UI HTML/CSS-like (degradês, efeitos de glow, data binding) sem brigar com um segundo pipeline de renderização. Construir o glintfx é um projeto à parte, desenvolvido junto com o GusWorld especificamente para tornar esse tipo de UI possível num jogo pequeno em C++/SDL3. Crédito e agradecimento completos por esse trabalho.

#### RmlUi

O [RmlUi](https://github.com/mikke89/RmlUi) (licença MIT, documentação em [mikke89.github.io/RmlUiDoc](https://mikke89.github.io/RmlUiDoc/)) é a biblioteca que o glintfx embrulha, e ela merece um parágrafo próprio.

O RmlUi é uma biblioteca de UI retida, HTML/CSS-like, para jogos e aplicações, e é, com toda sinceridade, uma das ferramentas de UI open-source mais bem projetadas do espaço C++ de jogos. O modelo de documento/elemento é elegante e genuinamente fácil de raciocinar depois que se trabalha com ele: um DOM de verdade, estilização CSS-like de verdade, data binding de verdade (o modelo de dados embutido, estilo MVC), em vez da malabarismo de modo imediato em que a maioria das UIs de jogo fica presa. O sistema de decorators e efeitos (degradês, blur, drop shadow, máscaras, e mais, tudo componível via RCSS) dá a um time pequeno o tipo de polimento visual que de outra forma exigiria um programador de UI dedicado construindo do zero. A documentação é completa e o código-fonte se mantém mantido, coerente e genuinamente agradável de integrar há anos, o que é raro num projeto deste porte.

Toda a UI voltada ao jogador do GusWorld (menus, o cockpit de batalha, diálogos, HUD) existe porque o RmlUi existe. **Obrigado, Michael R. P. Ragazzon (mikke89), por construir e manter o RmlUi.** É exatamente o tipo de infraestrutura open-source discreta e de alta qualidade que torna projetos como este possíveis, e estamos gratos por construir em cima dela.

### Áudio

#### miniaudio

O [miniaudio](https://miniaud.io/) (`github.com/mackron/miniaudio`, domínio público / MIT-0, de David Reid) continua sendo o motor de áudio por trás da música, dos SFX e dos cross-fades do GusWorld, mas não entra mais no projeto diretamente. Desde 2026-07-22 o áudio do jogo passa pelo `glintfx::Audio`, e o miniaudio chega de forma transitiva, como a implementação que o glintfx vendoriza e compila por conta própria; a fachada `AudioEngine` do GusWorld fica por cima disso e nunca toca num tipo `ma_*`. O crédito vale do mesmo jeito, e o motivo pelo qual o miniaudio ganhou continua valendo também: um motor de áudio em um header só, sem sistema de build para brigar e sem linkagem dinâmica para configurar, é exatamente o tipo de dependência que um pequeno projeto solo adora. Obrigado, David Reid, por colocar esse nível de engenharia de áudio atrás de uma licença tão simples e generosa.

*Nota histórica: o miniaudio já foi vendorizado direto em `GusEngine/third_party/miniaudio/`. Essa cópia foi removida na higienização do M9 (2026-07-22), depois que o `glintfx::Audio` assumiu o device.*

### Criptografia

#### Monocypher

O [Monocypher](https://monocypher.org/) (licença dupla CC0 / BSD-2-Clause, de Loup Vaillant) fornece a criptografia AEAD moderna (XChaCha20-Poly1305) que protege os arquivos de save do GusWorld contra adulteração e corrupção. Está vendorizado em `GusEngine/third_party/monocypher/`. Toda a filosofia de design do Monocypher, uma implementação pequena, auditável e sem dependências de primitivas modernas, é exatamente o que um formato de save de um jogo pequeno precisa: garantias fortes sem trazer junto uma stack de cripto pesada. Obrigado, Loup Vaillant, por tornar criptografia séria acessível e auditável para projetos como este.

### Testes

#### Catch2

O [Catch2](https://github.com/catchorg/Catch2) é o framework de testes por trás da suíte de testes da engine do GusWorld (`ctest` + Catch2 em `core/`, `domain/`, `platform/` e `app/`). Seu estilo expressivo de asserções e baixa cerimônia facilitam manter os testes de um projeto solo honestos. Obrigado aos mantenedores e contribuidores do Catch2.

### Kit de bibliotecas C++

Além das dependências críticas de plataforma citadas acima, o GusWorld vendoriza código C++ em `GusEngine/third_party/`, cada biblioteca sob sua própria licença permissiva (veja [THIRD-PARTY-LICENSES.md](THIRD-PARTY-LICENSES.md) para as licenças exatas). A lista é curta de propósito, e hoje tem exatamente duas entradas:

- **[stb](https://github.com/nothings/stb)** (MIT / domínio público), as bibliotecas single-header de Sean Barrett: carregamento e escrita de imagem, e rasterização TrueType, usadas pelas camadas de render2d e de atlas de fonte. Boa parte do mundo indie de desenvolvimento de jogos roda em cima do stb de alguma forma.
- **[Monocypher](https://monocypher.org/)** (CC0 / BSD-2-Clause), agradecido por extenso na seção de Criptografia acima, e o único vendor com build próprio.

*Nota histórica: uma rodada anterior de vendorização (2026-06-22) trouxe um kit bem maior de bibliotecas header-only "por via das dúvidas", entre elas [glm](https://github.com/g-truc/glm) (MIT), [EnTT](https://github.com/skypjack/entt) (MIT), [Box2D](https://github.com/erincatto/box2d) (MIT) e [fmt](https://github.com/fmtlib/fmt) (MIT). Trinta e uma dessas bibliotecas já não tinham nenhum `#include` fora de `third_party/` quando a higienização do M9 chegou, e foram removidas do repositório em 2026-07-22. Estão nomeadas aqui como registro histórico, não como dependência atual. O agradecimento aos autores vale de qualquer forma: poder experimentar uma biblioteca, concluir que ela não é necessária e removê-la sem atrito é parte do que o C++ header-only sob licença permissiva dá a um time pequeno.*

*Nota histórica (2026-07-30): o vendor do stb também trazia `stb_rect_pack.h` (empacotamento de retângulos) e `stb_image_resize2.h` (redimensionamento de imagem), dois arquivos single-header do mesmo projeto. Nenhum dos dois jamais teve consumidor no código, e ambos foram removidos como parte da limpeza do plano de migração. O agradecimento a Sean Barrett pela coleção inteira do stb vale de qualquer forma, independente de quais single-headers acabamos usando.*

### Nota final

GusWorld é o projeto de uma pessoa só, mas é construído sobre os ombros de muita engenharia open-source generosa e cuidadosa. A todos os nomes citados nesta página, e a todos nos projetos acima que revisaram um pull request, escreveram uma página de documentação, ou responderam uma issue: obrigado.

Se o GusWorld algum dia ajudar alguém a descobrir um desses projetos, essa já é uma pequena forma de retribuir.

---

*Last updated / última atualização: 2026-07-30 (two orphaned stb single-headers removed, `stb_rect_pack.h` and `stb_image_resize2.h`, zero consumers / dois single-headers órfãos do stb removidos, `stb_rect_pack.h` e `stb_image_resize2.h`, zero consumidores).*

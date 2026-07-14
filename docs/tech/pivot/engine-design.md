# Design da Engine — Pivot C++/Qt6 (GusWorld)

> ⚠️ **AVISO DE LEITURA (reforçado 2026-07-14, achado PS-Y8):** este doc foi escrito na era **Qt6** e a maior parte do conteúdo TÉCNICO CONCRETO de render/UI/build/licença descreve **Qt RHI, Qt Quick/QML, Qt Test e static-link do Qt, que NÃO são mais a stack.** A stack real há semanas é **SDL3 + glintfx/RmlUi + miniaudio** (ADR-008, ADR-009, ADR-010). Só continuam VÁLIDOS: as **4 camadas** (core/domain POCO puro / platform / app), o **plano de migração M0-M9**, e o reaproveitamento de lógica C#→C++. Tudo que fala de Qt (§§ de render/UI/toolchain/licença) está **SUPERADO** — ao consultar este doc como "autoridade técnica" (CONTRACT §8), trate as partes Qt como histórico, não como verdade atual. Fonte viva do stack: ADR-008/009/010 + README.
>
> **NOTA 2026-06-23:** a camada de PLATAFORMA (Qt RHI + Qt Quick/QML + Qt Multimedia) foi SUPERADA por SDL3 + RmlUi + miniaudio. Ver [`docs/tech/adr/ADR-008-repivot-qt-to-sdl3.md`](../adr/ADR-008-repivot-qt-to-sdl3.md). O restante do design (4 camadas, POCO core/domain, save, i18n, board M0-M9) segue válido.

**Status:** decisões macro **ratificadas pelo líder supremo** no brainstorm de 2026-06-21. O design da **engine** está fechado; os demais eixos do pivot (combate Pokémon, câmera, arte, áudio, porte, licença) ficam para brainstorms separados.

**Decisão âncora (one-way door):** GusWorld migra de **Godot 4.6 + C# .NET 8** para **C++ + Qt6 com engine própria**. Cross-ref: `TODO.md` → onda de Refatoração (RF-1, RF-2, RF-7).

**Reversibilidade:** one-way door de stack. Mitigado por **migração faseada** — o Godot permanece funcional até o último marco (M8).

**Origem:** brainstorm colaborativo (skill `superpowers:brainstorming`), trilha técnica do Caetano/CTO (`software-architect` + `engine-graphics-programmer`), decisões coletadas do líder via `AskUserQuestion`.

---

## 1. Decisões ratificadas

| Tema | Escolha | Observação |
| :--- | :--- | :--- |
| **Ambição** | **Híbrida** — enxuta agora, arquitetura limpa pra crescer | YAGNI no escopo, boundaries limpos. Não é full engine genérica |
| **Estilo visual** | **2D-only** | 3D (se houver) fica no Blender → convertido para sprite. "2D ou 3D" vira decisão de *pipeline de arte*, não de engine |
| **Camada gráfica** | **Qt RHI** (mundo) + **Qt Quick/QML** (UI/menus/telas de batalha) | RHI escolhe Vulkan/OpenGL por GPU; Linux+Win sem código duplicado. Tudo atrás de uma interface `IRenderer` |
| **Modelo de cena** | **Híbrido** — cena = casca de render/input/grid; **regras de jogo = POCO C++ fora da cena** | Preserva o padrão atual (lógica engine-agnostic). Recusa ECS formal (over-engineering) e árvore-de-nós profunda (modelo abandonado) |
| **Testes** | **Catch2** (lógica pura, headless) + **Qt Test** (camada Qt) | Ambos em `ctest`. Lógica roda sem abrir janela, como hoje roda sem Godot |
| **Build** | **CMake + Qt6 LTS** (6.5/6.8) | `CMakePresets.json` p/ Linux+Win reproduzível; `vcpkg` p/ deps (Qt, OpenSSL) |
| **Ordem de execução** | **Lógica pura primeiro** | save/i18n/knowledge/templates já têm testes e não precisam de gráficos — trava o mais valioso cedo |

---

## 2. Arquitetura de módulos (híbrida enxuta)

```
gusengine/                    (nome provisório)
├── core/        POCO C++ puro, ZERO Qt, 100% testável headless
│   ├── time/        loop de tempo fixo (fixed timestep) + delta
│   ├── rng/         PRNG determinístico seedável (PCG/xoshiro), p/ save/replay
│   ├── ecs_lite/    Entity + arrays de componentes (struct-of-vectors simples)
│   ├── resource/    cache de assets (id → asset, ref-count)
│   └── events/      sinal/slot interno (NÃO QObject — desacoplado de Qt)
├── platform/    fronteira Qt — ÚNICA camada que conhece Qt
│   ├── window/      janela + swap
│   ├── render2d/    batcher de sprites + atlas + tilemap (Qt RHI)
│   ├── input/       eventos Qt → ações lógicas (consome o mapa de input_remap)
│   ├── audio/       saída de áudio sobre Qt Multimedia (implementa o hook de core/)
│   └── fs/          I/O de arquivo (implementa save/i18n)
├── domain/      PORTADO do C# — POCO puro, ZERO Qt
│   ├── save/        HMAC-SHA256 + migrators forward-only + backup chain
│   ├── i18n/        loader .md + validador de paridade
│   ├── progression/ EnemyKnowledgeTracker + XpDifferential
│   ├── templates/   CharacterTemplate / EnemyTemplate
│   └── combat/      [REESCRITO] motor estilo Pokémon (eixo RF-4)
└── app/         GusWorld-specific
    ├── screens/     Overworld / Battle / Menu (máquina de estados de tela)
    └── main.cpp
```

**Regra de dependência (invariante das 4 camadas):** `core/` e `domain/` **não incluem `<Q...>`**. Só `platform/` e `app/` linkam Qt. Auditável no CI (`grep` de includes Qt em `core/`+`domain/` = vazio). É o que mantém a lógica reutilizável sem arrastar Qt — espelha a invariante atual (`GusWorld.Game` nunca aparece em `engine/`).

---

## 3. Reaproveitamento C#→C++ (auditado no disco)

A lógica de domínio atual é **engine-agnostic de verdade** (medido): acoplamento a Godot resume-se a `Godot.FileAccess` (I/O) e structs `Vector3`/`Color` (serialização). Porte = **reescrita idiomática de regras puras**, não desacoplamento de framework.

| Ativo | LOC | Ação | Por quê |
| :--- | ---: | :--- | :--- |
| save_system (HMAC, migrators, backup) | 442 | **PORTAR** | regra pura; `FileAccess`→QFile, HMAC→QCryptographicHash/OpenSSL |
| localization (loader + paridade) | 292 | **PORTAR** | só troca o I/O; formato `.md` é próprio |
| knowledge (tracker, XpDifferential) | 203 | **PORTAR** | POCO 100% puro, tradução ~1:1 |
| templates (Character/Enemy) | 1023 | **PORTAR** | `record`→`struct`/`class`; serializer equivalente |
| input_remap (mapa lógico) | 185 | **PORTAR** | o mapa ação→tecla é puro; só o backend de evento muda |
| RNG determinístico | — | **CRIAR** | engine própria exige PRNG seedável explícito |
| turn_combat (CTB+cartas+Gambito) | 3151 | **PORTAR** | **ESCLARECIDO 2026-06-21: sistema PRESERVADO.** "Pokémon" = só a apresentação/câmera da batalha (RF-4), não redesign. 0 acoplamento Godot → porte idiomático direto; os ~200 testes PORTAM |
| orbital_camera (3/4) | — | **DESCARTAR** | exploração vira câmera fixa top-down (trivial: offset + clamp ao mapa). Batalha tem câmera própria (tela estilo Pokémon, RF-4) |
| dialogue (plugin Godot) | — | **DESCARTAR→REESCREVER** | runner `.dialogue` mínimo em C++, ou adiar |
| buses / scene_router / audio_director | ~190 | **REESCREVER** | viram `core/events` + camada `platform/` |

**Estratégia:** reescrita idiomática C++ moderno (RAII, `std::`, value semantics) — **não** transpiração automática (gera C++ inintelígivel e não-idiomático).

**Preservar a cobertura de ~598 testes (ativo mais valioso):**
1. Testes xUnit = **spec executável** do comportamento canônico. Reescrevê-los no framework C++ lendo o xUnit como espec.
2. **Porte test-first** (vermelho→verde) por subsistema: escreve os testes C++ traduzidos, porta a lógica até passar. Reproduz a cobertura **e** valida o porte no mesmo passo.
3. **Oráculo de equivalência (save):** congelar saves `.gdt`/JSON de referência + HMACs gerados pelo C# atual como fixtures binárias; o save C++ deve lê-los byte-a-byte. Prova objetiva de não-corrupção.
4. **Os ~200 testes de `turn_combat` PORTAM** (sistema preservado — "Pokémon" é só a apresentação da batalha, RF-4). O combate ganha apenas uma camada de *apresentação* (tela de batalha), não um motor novo.

Saldo: **~590 testes portam** (incl. combate); quase nada descarta. Disciplina TDD intacta.

---

## 4. Plano de migração faseado (anti big-bang)

Godot só é apagado no **fim** (M8), quando a engine própria já roda o loop jogável. Cada marco tem critério de pronto **testável**.

| Marco | Entrega | Pronto quando | Godot |
| :--- | :--- | :--- | :--- |
| **M0 — Andaime** | repo + CMake + Qt6 + framework de teste | `cmake --build` verde nos 2 OS; 1 teste dummy no CI | intacto |
| **M1 — Janela+loop+sprite** | window + render2d (1 sprite) + loop fixo | sprite a 60fps estável nos 2 OS; teste de acumulador de delta | intacto |
| **M2 — Input** | input → ações; porta `input_remap` | move com WASD+gamepad; remap persiste; testes input verdes | intacto |
| **M3 — Lógica pura portada** | porta save + i18n + knowledge + templates | **~390 testes C++ verdes**; oráculo de save byte-a-byte; paridade i18n | intacto (paralelo) |
| **M4 — Cena top-down** | tilemap + colisão de grid + câmera fixa | anda em mapa real, colide, câmera clampada; sem Godot no binário | paralelo |
| **M5 — Combate (portado) + tela de batalha** | PORTAR motor `turn_combat` (preservado) + `BattleScreen` estilo Pokémon (apresentação) | overworld→tela de batalha→resultado→volta; **~200 testes de combate portados verdes** | paralelo |
| **M6 — Áudio** | platform/audio + 1 música + SFX | música em loop, SFX em hit, fade entre telas | paralelo |
| **M7 — Paridade jogável** | loop completo na engine nova | playthrough ~5min 100% sem Godot | redundante |
| **M8 — Decommission (RF-2.1)** | apagar game/, .csproj, project.godot, addons, .mcp.json | repo compila/roda sem nenhum bit de Godot/C# | **APAGADO** |

Ordem real sugerida: **M0 → (M1 ‖ M3) → M2 → M4 → M5 → M6 → M7 → M8**. M3 (lógica pura) entra cedo e em paralelo — menor risco, maior valor, testável headless desde o dia 1. Gate anti-big-bang: nenhum marco apaga Godot antes de M7 provar paridade.

---

## 5. Render

- **2D-only sobre Qt RHI:** sprite/tilemap batcher, câmera **ortográfica fixa**, Y-sort. Câmera fixa elimina frustum culling, LOD, occlusion — ganho de simplicidade do pivot.
- **UI / menus / telas de batalha Pokémon em Qt Quick (QML):** menus de ação, caixas de texto, barras de HP, transições. Divisão limpa: **RHI = pixels do mundo, Quick = chrome/menus**.
- **Pipeline de arte (decisão por asset, eixo RF-5):** pixel à mão (estilo Zelda SNES) e/ou modelar em 3D no Blender → bake para sprite (estilo Stardew/DKC). A engine não muda entre os dois.
- Tudo atrás de `IRenderer` — se um dia for HD-2D ou 3D real, a fundação serve (híbrido).

---

## 6. Build / Test / CI

- **Toolchain:** CMake + presets + Qt6 LTS + vcpkg (Qt, OpenSSL). MSVC ou MinGW no Windows; GCC/Clang no Linux.
- **Testes:** Catch2 v3 para `core/`+`domain/` (a spec executável portada, headless) + Qt Test para `platform/` (window/input/audio). Ambos via `ctest`.
- **CI Forgejo Actions:** matriz **Linux + Windows** desde M0. `clang-format --dry-run --Werror` (lint) + `cmake --build` + `ctest`. Cache de Qt no runner.
- **Simplificação:** morre o `aot_check` e todos os erratums de AOT do .NET (ADR-002-ERRATUM-AOT) — C++ é AOT por natureza.

---

## 7. Licença (DECIDIDA pelo líder 2026-06-21; fecha o RF-9, ver ADR-005)

**Modelo de negócio (decidido 2026-06-21):** o jogo é **freeware** (grátis) + **doação opcional** ("buy me a coffee and some AI tokens", PayPal donate (merchant `9XNZQ4RND67KL`, BRL), nunca obrigatória) + **código aberto sob GPLv3**.

**Decisões fechadas (detalhe em [ADR-005](../adr/ADR-005-license-gpl3-assets-ccbysa.md)):**
- **Código = GPLv3.** Migrado de AGPL-3.0 (a cláusula de rede §13 da AGPL é inócua num jogo desktop single-player sem servidor; GPLv3 dá o mesmo copyleft prático e é a escolha convencional). A dúvida "AGPL vs GPL" está resolvida: é GPLv3.
- **Assets** (arte, música, lore in-game) = **CC-BY-SA 4.0**, separada do código. Fronteira detalhada em `ASSETS-LICENSE.md`.
- **Livros-companheiros** (Vol 1 bíblia + Vol 2 antologia, em `docs/book/`) = **direitos reservados**, obra literária à parte. A lore no jogo (CC-BY-SA) NÃO estende essa licença ao texto dos livros.
- **Static-link do Qt liberado.** Com o jogo todo em GPLv3, o link (inclusive estático) com Qt LGPL/GPL é permitido **sem custo nem licença comercial do Qt**. O link dinâmico LGPL deixa de ser obrigatório: vira escolha técnica de empacotamento, não de licença.
- O **EULA proprietário / código fechado** que o F2-LEG.1 previa fica **OBSOLETO**: não há mais intenção de fechar o código.
- Atribuição de terceiros (Qt, Godot, addons, fontes, OpenSSL) em `THIRD-PARTY-LICENSES.md` (incluir no pacote de release).

**Nota:** releases já publicadas sob AGPL-3.0 permanecem AGPL-3.0; a migração para GPLv3 vale daqui para frente. Titular único do copyright (solo indie) preserva a liberdade de relicenciar versões futuras.

- **Distribuição:** freeware self-hosted (itch.io / Codeberg releases) não tem atrito; lojas com DRM proprietário (Steamworks) teriam, mas Steam já está fora do G1 (CUT.10).

---

## 8. Fora do escopo deste design (outros eixos do pivot)

Brainstorms separados, cada um com seu spec:
- **RF-4** apresentação visual do combate (tela de batalha estilo Pokémon). **Sistema de combate preservado e portado** — este eixo é só render/UI da batalha, não redesign.
- **RF-3** câmera fixa top-down (detalhe de feel, zoom, follow, clamp).
- **RF-5** estilo de arte (pixel à mão vs 3D-bake por asset; novo style-guide).
- **RF-8** áudio: camada própria **sobre Qt Multimedia** (não do zero) — música adaptativa/SFX/mixagem/fade. Este design prevê o *hook* em `platform/audio`; a camada in-house é detalhada em eixo próprio.
- **RF-6** reclassificação de porte (scale-up → grande; recalibrar a constelação via /bigtech).
- **RF-9** auditoria de licença: **FECHADO** em 2026-06-21 (ver §7 e ADR-005): código GPLv3, assets CC-BY-SA 4.0, livros reservados, doação PayPal donate (merchant `9XNZQ4RND67KL`, BRL).

---

## 9. Riscos

| # | Risco | Mitigação |
| :--- | :--- | :--- |
| R1 | Qt RHI é API semi-privada (estabiliza por versão) | encapsular atrás de `IRenderer`; pin do Qt LTS; trocar backend vira 1 arquivo |
| R2 | Pipeline 3D→sprite vira projeto paralelo e atrasa a slice | YAGNI: pixel à mão na vertical slice; bake só quando um asset concreto exigir volume |
| R3 | Transição mundo↔batalha engasga na iGPU | pré-carregar atlas de batalha; transição com fade; um contexto RHI vivo (não recriar device) |
| R-macro | Escopo de engine própria multiplica o cronograma | mitigado por: híbrido enxuto + 2D-only + ~390 testes/lógica portável + marcos testáveis + Godot vivo até M8 |

---

*Documento de design do sub-projeto "engine" do pivot. Os demais sub-projetos (RF-3..RF-9) terão specs próprios. Implementação só começa após revisão do líder + plano de implementação (writing-plans).*

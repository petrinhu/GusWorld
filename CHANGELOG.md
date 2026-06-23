# Changelog

Todas as mudanças notáveis deste projeto são documentadas neste arquivo.

O formato é baseado em [Keep a Changelog](https://keepachangelog.com/pt-BR/1.1.0/) (adaptado a um jogo solo indie em desenvolvimento, sem release pública ainda) e o projeto adere a [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

O destaque deste changelog são os **pivôs de stack**: decisões one-way door que reescreveram a base técnica. Cada pivô tem um ADR canônico em [`docs/tech/adr/`](docs/tech/adr/), linkado abaixo. Este arquivo é o índice cronológico; não duplica o conteúdo dos ADRs.

Datas em ISO 8601 (`AAAA-MM-DD`). Idioma: pt-br.

---

## Não lançado

O jogo está em desenvolvimento (vertical slice da Fase 2). Nenhuma versão foi publicada. O trabalho está agrupado por marco do board de migração ([TODO.md](TODO.md)) e pelos pivôs de stack.

### Pivôs de stack (one-way doors)

Três viradas de base técnica até hoje. Cada uma reescreveu fundação; a última é a mais recente e a mais relevante.

- **Fase 1 (deep-lore) para Fase 2 (engine), 2026-05-19, [ADR-001](docs/tech/adr/ADR-001-pivot-lore-to-engine.md).**
  Pausa da produção de deep-lore como gate de cronograma; deep-lore restante segue em paralelo orgânico (não bloqueia código). Começa a construção da engine + vertical slice.

- **Godot 4 + C# .NET 8 AOT, 2026-05-19, [ADR-002](docs/tech/adr/ADR-002-csharp-aot-over-gdscript.md).**
  Linguagem primária deixou de ser GDScript e virou C# .NET 8 (AOT), por critério de máxima performance em máquinas modestas. (Stack posteriormente abandonado, ver abaixo.)

- **Godot/C# para C++20 + engine própria (Qt6), 2026-06-21, `docs/tech/pivot/engine-design.md`.**
  Decisão do líder supremo: trocar Godot por engine própria em C++20, migração faseada anti big-bang (Godot vivo como referência de leitura até o decommission no M8). Arquitetura em 4 camadas (`core`/`domain` POCO puro + `platform`/`app` na fronteira). Câmera fixa top-down, combate por turnos estilo Pokémon, visual estilo Stardew. Licença migrada de AGPL-3.0 para GPLv3 ([ADR-005](docs/tech/adr/ADR-005-license-gpl3-assets-ccbysa.md)).

- **RE-PIVOT: Qt6 para SDL3 + RmlUi + miniaudio, 2026-06-22, [ADR-008](docs/tech/adr/ADR-008-repivot-qt-to-sdl3.md) (o mais recente e mais importante).**
  A camada de plataforma deixa de ser Qt6 e passa a ser SDL3 (janela, loop, input, gamepad), com **RmlUi** (HTML/CSS-like) para a UI do jogador e **miniaudio** para áudio. Motivos: o Qt RHI é API semi-privada e isso é risco (some com `SDL_Renderer`); gamepad nativo de classe AAA (o Qt6 removeu o QtGamepad); binário e deploy ~10x menores (licença zlib); portabilidade para mobile e console (Qt não vai para console). Custo baixo porque a lógica pura (`core`/`domain`, ~590 testes auditados) não muda: só a fronteira `platform/` + a casca `app/` são reescritas. A invariante das 4 camadas passa a proibir Qt **e** SDL em `core`/`domain`.

### M0: andaime da engine

- Repositório C++20 + CMake + CMakePresets + framework de teste (Catch2 + Qt Test) em `GusEngine/`.
- Build Linux verde, suíte `ctest` passando, CI Forgejo escrito com gate da invariante das 4 camadas.
- `.clangd` apontando `compile_commands.json`; guard de TDD test-first para C++ via hook `PreToolUse`.

### M1: janela + loop + sprite

- Janela + loop de tempo fixo (acumulador 60Hz com alpha de interpolação e clamp anti spiral-of-death) + `render2d` + ponte de input.
- Boneco anda no mapa (WASD/setas), corre com Shift, desliza nas paredes (corner-correction estilo Stardew), câmera presa ao mapa.
- Fix: pé colado na parede (foot-anchor automático) e diagonal travando no último eixo.
- **Fase 1 do re-pivot SDL ([ADR-008](docs/tech/adr/ADR-008-repivot-qt-to-sdl3.md)) FEITA:** a fronteira do M1 foi reescrita de Qt6 para SDL3 (janela + `render2d` via `SDL_Renderer` + input com gamepad), o sprite do Cauã reintegrado, smoke headless via `SDL_VIDEODRIVER=dummy`. Backend Qt aposentado.

### M2: input (lógica feita)

- Lógica pura de remapeamento portada (`ActionRegistry`/`InputBinding` C# para POCO C++).
- Persistência de controles em TDD: `controls.json` com serializer/parser JSON próprio (zero dependência), `hash128` (SHA-256 truncado), diff/restore e sanitização de perfil multi-player.
- Save subiu para V4 (backup de controles + slot-id selado + KDF), com três camadas anti-tamper. Ver [ADR-007](docs/tech/adr/ADR-007-controls-json-hash128-save-v4.md).
- Pendente: backend de evento da plataforma + I/O em disco + janela de aviso do diff.

### M3: lógica pura portada (auditado, verde)

- Subsistemas portados de C# para POCO C++ puro: save, i18n, progression (knowledge), templates.
- Cripto própria SHA-256/HMAC validada contra vetores FIPS 180-4 e RFC 4231. Ver [ADR-006](docs/tech/adr/ADR-006-crypto-hmac-formato-domain.md).
- Save com migrators forward-only (V1 para V2 para V3), oráculo semântico (roundtrip + tamper + determinismo).
- Hardening do decoder contra pré-alocação guiada por count + 33 testes de fuzzing.
- Auditado pelo `internal-auditor`: 0 críticos, 0 importantes (dossiê `docs/auditoria/AUDIT-M3-2026-06-22/`). 174 testes verdes.

### M4: cena top-down (lógica feita)

- Lógica pura de espaço em `core/spatial`: `TileGrid`, colisão AABB que desliza na grade (resolução por eixo, feel Zelda/Stardew), clamp de câmera ao mapa.
- Pendente: a metade visual (tilemap render na plataforma + integração de cena no app), que nasce em SDL na Fase 2 do re-pivot.

### M5: combate portado + tela de batalha

- Motor `turn_combat` portado de C# para C++20 em TDD (preservado, não descartado): `CombatStateMachine`, `ComboTable`, brains de inimigo, RNG injetável, atores/filas, subsistema de ambiente (`EnvironmentModifier`, ver [ADR-004](docs/tech/adr/ADR-004-environment-modifier-contract.md)).
- Property-based + fuzz das 9 invariantes do motor.
- **M5-DMG:** evolução da fórmula de dano §11 (decisão do líder 2026-06-22): canal de crit/falha (5% piso) sobre a variância de Knowledge. Auditada pelo `internal-auditor`: 0 críticos (dossiê `docs/auditoria/AUDIT-M5-MOTOR-2026-06-22/`).
- Pendente: a `BattleScreen` (apresentação estilo Pokémon), que entra na Fase 3 do re-pivot (UI em RmlUi).

### Bibliotecas e dependências

- **32 bibliotecas C++ vendorizadas** em `GusEngine/third_party/` (filosofia zero-dep): header-only de licenças permissivas (MIT/Boost/zlib/Apache/PD), incluindo miniaudio, PCG, stb, fmt, glm, EnTT, Box2D.
- SDL3 e RmlUi entram via FetchContent/submodule com pin de versão (não são header-only); miniaudio aposenta a camada de áudio sobre Qt Multimedia que estava planejada.

### Pipeline de arte por IA

- Pipeline de produção assistido por IA, do lore ao sprite: prompts derivados do lore canônico alimentam geração de imagem 2D (nano banana / Grok), Tripo3D para image-to-3D quando precisa de volume, e **PixelLab (Pro)** para sprites multi-direção + animação. O jogo é 2D em runtime; o 3D é só ferramenta de produção.
- **Party + 16 personagens secundários** gerados.
- O jogador deixou de ser retângulo-placeholder e virou **sprite animado do Gus** no overworld (walk de 7 frames + idle com respiração procedural).
- **Viewer de animação** na engine (`gusworld_app --anim-preview`) + plano de animação canonizado ([`docs/design/mecanicas/animation-plan.md`](docs/design/mecanicas/animation-plan.md)).
- Esquema de locomoção/animação canonizado: 3 poses de walk em ping-pong, 4 direções únicas (sem flip, Pillar 3), timing por distância. Ver [`docs/design/mecanicas/locomotion.md`](docs/design/mecanicas/locomotion.md).

### Mecânicas de design

- **Carga do Aparato** (a "stamina" re-enquadrada por pillar-check) canonizada, números aprovados pelo líder 2026-06-23. Não é stamina física (vetada pelo anti-pillar souls-like); é a Carga do Tavus-Drive (o poder vem do hardware, Pillar 3), e o overflow é pago pelo corpo do Gus (ofegância, Pillar 4). Regra de ouro: a Carga **nunca** trava o deslocamento básico. Números em Fibonacci (max 89, drain 8/s, regen 13/s parado). Ver [`docs/design/mecanicas/stamina.md`](docs/design/mecanicas/stamina.md).

### Legal

- Licença do código migrada de AGPL-3.0 para **GPLv3**; assets sob **CC-BY-SA 4.0**; livros Vol1/Vol2 com direitos reservados (obra à parte). Modelo freeware + doação opcional. Ver [ADR-005](docs/tech/adr/ADR-005-license-gpl3-assets-ccbysa.md).

---

## Era de design (Fase 1, concluída)

Antes do código, ~365k palavras de deep-lore canônico foram produzidas: Era 1 §§1-10 (~318k), R2 Facções (7 docs, ~22k), R3 Settings (8 docs, ~25k), além dos Blocos F/G/H/I. Base canônica em [`docs/narrative/`](docs/narrative/), [`sinopse.md`](sinopse.md), [CHARS.md](CHARS.md) e [PLACES.md](PLACES.md). O gate desta fase está registrado em [ADR-001](docs/tech/adr/ADR-001-pivot-lore-to-engine.md).

---

## Notas de versionamento

- Versões pré-`v1.0.0` (`v0.x.x`) podem invalidar saves sem migrator (rebuild aceitável durante prototipagem). A partir de `v1.0.0`, migrators forward-only são **obrigatórios** (ver [CONTRACT.md](CONTRACT.md) §7).
- Bumps de versão acompanham:
  - **MAJOR** (`v1.x.x` para `v2.0.0`): quebras de save format sem migrator, mudança de pillar, redesign de mecânica core.
  - **MINOR** (`v1.0.x` para `v1.1.0`): nova feature, nova mecânica, nova área, novo companion.
  - **PATCH** (`v1.0.0` para `v1.0.1`): bugfix, balance tweak, hotfix de localização.

---

## Marcos planejados (não-versionados ainda)

| Marco | Versão alvo | Descrição |
|---|---|---|
| Vertical Slice | `v0.1.0` | 5-10min coeso jogável: cidade explora + 1 combate turn-based + 1 puzzle Gambito + save funcional |
| Perf validada | `v0.1.1` | 60fps confirmado em profiler real no hardware-floor (iGPU) |
| Playtest | `v0.2.0` | Time-to-fun validado com o time de playtest N=3 |
| Build distribuível | `v0.3.0` | `.tar.gz` Linux + `.zip` Windows rodando em máquina limpa |
| Produção | `v0.5.x - v0.9.x` | Conteúdo completo + balance + a11y gates v1.0.0 |
| Beta | `v0.9.x` | Beta fechada + bug bash + localização en-intl |
| Release | `v1.0.0` | Ship Linux + Windows. Vol 1 livro lore-bible + Vol 2 antologia narrativa. |

# Changelog

Todas as mudanças notáveis deste projeto são documentadas neste arquivo.

O formato é baseado em [Keep a Changelog](https://keepachangelog.com/pt-BR/1.1.0/) (adaptado a um jogo solo indie em desenvolvimento, sem release pública ainda) e o projeto adere a [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

O destaque deste changelog são os **pivôs de stack**: decisões one-way door que reescreveram a base técnica. Cada pivô tem um ADR canônico em [`docs/tech/adr/`](docs/tech/adr/), linkado abaixo. Este arquivo é o índice cronológico; não duplica o conteúdo dos ADRs.

Datas em ISO 8601 (`AAAA-MM-DD`). Idioma: pt-br.

---

## Não lançado

O jogo está em desenvolvimento (vertical slice da Fase 2). Nenhuma versão foi publicada. O trabalho está agrupado por marco do board de migração ([TODO.md](TODO.md)) e pelos pivôs de stack.

**Início do projeto: 2026-05-15** (primeiro commit `97ed2fe`, Fase 1 concepção + deep-lore Bloco G).

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
- **BattleScreen cockpit "Tático" entregue** via glintfx (embed mode): paridade visual real (gradientes, glow, `border-radius`, molduras nativos) + dados vivos por data-model (HP, verbo, alvo, log de batalha, retrato que segue o ator ativo, label do inimigo). Aprovado ao vivo pelo criador. Ver a seção de UI/HUD abaixo e [ADR-010](docs/tech/adr/ADR-010-adopt-glintfx-embed-mode.md).

### UI/HUD: adoção do glintfx (embed mode), 2026-07-01

- **Adotado o glintfx via embed mode (`glintfx::UiLayer`) como motor de UI/HUD**, no lugar do backend RmlUi vendorizado à mão (`RmlUi_Renderer_GL3` + `RmlUi_Platform_SDL` + `rmlui_hud`). O glintfx (MPL-2.0, repo do próprio criador) embrulha RmlUi 6.3 + backend GL3 + efeitos data-driven atrás de uma fachada limpa. Decisão canônica em [ADR-010](docs/tech/adr/ADR-010-adopt-glintfx-embed-mode.md) (two-way door no backend de UI).
- Execução faseada F0→F3: F0 alinhou o RmlUi ao SHA `2cd2886` (6.3) do glintfx; F1 provou o smoke do embed mode; F2 portou o cockpit "Tático" com paridade visual + dados vivos; **F3 removeu o backend RmlUi vendorizado e estendeu o GATE das 4 camadas** para proibir `<glintfx` em `core/`+`domain/`.
- Consumido via CMake FetchContent (pin `v0.2.4`, `GLINTFX_BACKEND_GLFW=OFF`, embed-only sem GLFW, honrando o [ADR-008](docs/tech/adr/ADR-008-repivot-qt-to-sdl3.md)/SDL3). A v0.2.4 carrega a UA-stylesheet do RmlUi.
- Preservado: SDL3 dono de janela/loop/input/gamepad/contexto GL; a arena (`Render2dGl3`) intocada; a ordem de composição arena → UI → swap; `core/`+`domain/` POCO (~1013 testes) intactos.
- **Rodada de polish do cockpit (2026-07-01, aprovada ao vivo pelo criador):** o brasão da abertura passou a usar o glyph Vetor-Dragão (`resources/images/vance_dragon_glyph.png`, agora versionado por whitelist no `.gitignore`) recortado em medalhão circular via par máscara/filho (o RmlUi não tem `border-radius:50%` nem recorta o próprio decorator do elemento), no lugar do antigo monograma "V"; os anéis do brasão foram para aço/gunmetal neutro (`#8a94a8`/`#6f7c96`/`#5d6a86`) no lugar de latão/cyan, para harmonizar com o glyph vermelho e o fundo azul. Fixes de layout no mesmo passe: nome/role do ator centrados sob a moldura do retrato, log de combate enxuto em estilo terminal (`atacante -> alvo -dano`) no lugar da narração verbosa, barra de HP com largura contida, fila CTB sem a caixa de fundo (compõe direto sobre a vinheta da arena) e topo do gradiente do painel escurecido para não formar barra clara na borda.

### M6: camada de áudio (miniaudio), 2026-07-03

- **AudioEngine sobre miniaudio** entregue em 4 fases (plano fechado em brainstorm Caetano/CTO + líder via `AskUserQuestion`, [ADR-011](docs/tech/adr/ADR-011-m6-audio-onda1-plano.md)): F1 alicerce (device real + null-device + mixer mínimo música/SFX); F2 kit provisório CC0 curado pela bíblia de leitmotivs (2 hits digitais A/B + 1 faixa de cidade em loop, proveniência registrada em `assets/AUDIO_KIT_PROVISORIO.md`); F3 SFX no hit ancorado no evento de contato real do `BattleAnimDirector` (party e inimigo disparam pelo mesmo `play_sfx`), aprovado ao vivo pelo líder ("o som está em sincronia"); F4 música em loop com fade-in/fade-out nativos do miniaudio, aprovado ao vivo ("AMEI").
- `domain/` permanece POCO puro (a FSM de combate nunca chama áudio); disparo por chamada direta da tela no ponto de contato, sem event bus (infraestrutura que o porte solo não pedia ainda).
- Feedback de timbre do hit registrado como não bloqueante, para a onda de áudio dedicada futura ("parece raio laser de filme, não dano contuso").
- **Crossfade cidade↔arena entregue via `M7-COSTURA` Inc.2** (`maestro_logic.hpp::crossfade_music`), fechando o critério pendente "fade entre telas", que dependia do loop de cena overworld↔batalha existir. Faixa de batalha `Arena_GusWorld.mp3` gerada pelo próprio líder no Suno, com prompt ancorado na bíblia de leitmotivs.
- **Validado ao vivo pelo líder** ("ABSURDO DE BOM"): o crossfade audível cidade↔arena, nos 2 sentidos, foi aprovado. M6 fecha.
- Backlog não bloqueante para a onda de áudio dedicada: `AUDIO-LOOP-CROSSFADE` (loop sistêmico sem costurinha audível na faixa gerada por IA), timbre do hit, ducking e os 5 buses completos.

### M7: paridade jogável, 2026-07-21

- **Costura overworld↔batalha** (`M7-COSTURA`, plano em [ADR-012](docs/tech/adr/ADR-012-m7-paridade-jogavel-plano.md)): um `Maestro` leve guarda o estado sobrevivente à troca de cena (posição do Gus, música corrente, save) e faz o swap cidade↔batalha; 1 inimigo fixo no mapa dispara o combate por colisão; vitória volta ao ponto de origem. Auditado internamente pelo `internal-auditor` (dossiê `docs/auditoria/AUDIT-M7-COSTURA-2026-07-04/`, 0 crítico, 2 importantes tratados à parte). Incrementos: fade preto seguido de uma sequência de frames pré-renderizada ("boot pixelizado") na transição de tela (o glitch procedural foi vetado ao vivo pelo líder, "pareceu bug"); posse do `AudioEngine` sobe da `battle_preview` pro `Maestro`; flavor da derrota (o Gus reboota, não morre, eco do Pillar "magia é software", setup do Dragon Victory).
- **Menu de sistema, pausa e config de som** (`MENU-PAUSA-CONFIG-SOM`): árvore hierárquica de 7 telas (Continuar/Salvar/Configurações/Sair), sliders de volume de música e SFX, hover e clique com som próprio. Aprovado ao vivo pelo líder ("tudo funcionando, sem bugs ou alterações perceptíveis").
- **Diálogo e NPC** (`M7-DIALOGO`, arquitetura em [ADR-014](docs/tech/adr/ADR-014-dialogue-runtime-poco.md)): runtime POCO C++20 em `domain/dialogue/` (grafo, nós, escolhas, flags, formato de autoria em texto próprio, zero-dep), NPC Seu Bertoldo Caím interagível no mapa real via gatilho ancorado nos pés (`feet_trigger_aabb`), apresentação nos 2 registros canônicos (caixa quente com retrato e tela-terminal), flag `npc_intro.*` persistida no save V4 sem bump de schema. Testado ao vivo pelo líder ("ok"): diálogo completo, escolha muda flag, flag sobrevive a save e load.
- **UI de salvar e carregar** (`SAVE-LOAD-UI`): tela de título no boot (Novo Jogo, Continuar, Sair), 6 slots manuais mais 1 Auto, autosave ao entrar e sair de área e após vitória em combate, avisos de save corrompido ou de versão incompatível e de controles diferentes do save. Auditado internamente (1 achado crítico corrigido: sobrescrita silenciosa por cima de um save corrompido) e confirmado pelo `qa-engineer` em verificação adversarial com execução real, não só leitura.
- **M7 fechado pelo playthrough ao vivo do líder com o Gus, 2026-07-21** (`cafd14c`): o loop completo (andar, falar com o NPC Bertoldo, entrar em combate até Victory, autosave, load) rodou 100% na engine C++20/SDL3+glintfx, sem nenhuma janela do Godot aberta, zero erro (saída 0).
- **3 feedbacks de polish do playtest viraram follow-up sem bloquear o marco:** `M7-FB1`, exploit de canto no tunneling da colisão, endereçado no mesmo dia com sweep sub-pixel e o guard `TUNNELING-CLAMP-GUARD` (`cc8c326`, `03b59f4`, `c9cbba9`); `M7-FB2`, a janela maximizada invade a barra de tarefas; decisão do líder foi não abrir issue no SDL e resolver na futura migração para o App mode do glintfx, que já corrigiu o problema no upstream (v0.16.0); `M7-FB3`, o menu inicial deve ganhar arte de fundo própria em vez da cena congelada, pendente de asset.

### M8: decommission Godot/C#, 2026-07-22

- **Godot/C# apagados do repo: repo compila e roda sem nenhum bit do stack antigo.** Faseado anti big-bang em 3 fases + 1 correção, todas validadas por build+`ctest` verdes:
  - **F1** (`912f9c3`): dados VIVOS movidos de `game/` para `resources/` via `git mv` (preserva histórico), `resources/translations/` (fonte do i18n e do gate de paridade do CI) e `resources/dialogues/npc_intro_bertoldo.dlg.txt`.
  - **F2** (`4760659`): submódulo `engine/` (foundation C# legada, repo próprio `petrinhu/gus_dragon-engine`) removido, junto com `.gitmodules` e `global.json` (o pin do .NET SDK 8 que só existia por causa dele).
  - **F3** (`c59fa0c`): `game/` morto apagado, 172 arquivos, 20816 linhas (addon dialogue_manager, 30 scripts C# incluindo o `SaveManager.cs` legado, tools, tests, `CombatScene.tscn`, `project.godot`, `Game.csproj`, `GusWorld.sln`). Prova: build a partir de diretório zerado, 1044/1044 alvos.
  - **Correção** (`c81b5d0`): limpeza física de `game/` (30M), `.mcp.json` e `.git/modules/engine`, que ainda sobravam no disco fora do índice.
  - Tag `pre-m8-godot-legacy` preserva o legado por registro (recuperação: `git checkout pre-m8-godot-legacy -- game/`).
  - Gate de build Windows do M8 já estava PRÉ-CUMPRIDO desde 2026-07-14 (WIN-CROSS-VALIDATE); ficou verde de novo na F1.
  - Decisão do líder pendente FORA deste repo: arquivar read-only o `petrinhu/gus_dragon-engine` no Codeberg.

### Bibliotecas e dependências

- **32 bibliotecas C++ vendorizadas** em `GusEngine/third_party/` (filosofia zero-dep): header-only de licenças permissivas (MIT/Boost/zlib/Apache/PD), incluindo miniaudio, PCG, stb, fmt, glm, EnTT, Box2D.
- SDL3 e **glintfx** (que traz o RmlUi 6.3 embrulhado) entram via FetchContent com pin de versão (não são header-only); miniaudio aposenta a camada de áudio sobre Qt Multimedia que estava planejada.

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


---

## Migrado do TODO.md em 2026-06-25 (enxugamento: TODO so com tabelas)

### Preambulo e atualizacoes narrativas (estavam no topo do TODO)

Tabela canônica de pendências e planejamento. Atualizar via skill `/tab_pendencias`.

**Reorganização 2026-05-29 (Cosmo/COO + squad C-level):** 9 colunas (adicionada **Onda**). Linhas em ordem topológica + WSJF. ~85 gaps da auditoria multidisciplinar inseridos. Status/Estado Auditado atualizados pelos vereditos da auditoria empírica de disco. Ondas W0–W7 (paralelizáveis dentro da onda); concluídos/CUT = Onda `—`.

**Auditoria de execução Cosmo/COO 2026-06-04:** 13 sprints (W2 fechado + W3) auditados contra disco. **Zero ✅ refutado** nesta sessão (motor real, não casca); **577 testes verdes confirmados empiricamente** (main thread rodou `dotnet test`); game compila. 1 rebaixamento: F2-AU.3 ✅→🔍 (`.tres`+AutoLoad não validáveis headless, validar no editor). W0/W1/W2 = 100%; W3 ~85% (poço de motor puro SECOU); W4/W5/W6/W7 ~0%. **Risco ativo: motor MUITO à frente da integração jogável (R-02 parcial) — nenhuma cena jogável existe. Próximo passo de maior valor = F2-G.5 (tela de combate) no EDITOR com o criador, NÃO mais motor solo.** Itens solo-able restantes são poucos (F2-QA.4 playtest plan, F2-S.12a paridade i18n, F2-D.8 doc, F2-LEG.3 SCA); FsCheck/extras de motor = adiar (anti-OE).

**Reorg 2026-06-07 (`--reorder`, thread direta — anti-OE: atualização de status, sem mudança de dependência ou WSJF):** **F2-G.5** polido nesta sessão (fix mouse-filter que bloqueava cartas; tooltips de cartas + 8 ações; HP barra vermelha + Mana barra azul; indicador de status sob o nome; nomes da party Title-Case via 5 chaves `ACTOR_*`; auto-target quando sobra 1 inimigo) — falta só validação interativa no editor. **F2-E.6 passou de 🎨 Pendente design → 🟡 Parcial:** fundação DialogueManager implementada (plugin v3.10.4 MIT + `DialogueService` 461 linhas + `.dialogue` stub Bertoldo; escopo "só fundação" escolhido pelo criador; build limpo). Sub-pendências realocadas ao lar natural: balloon UI→**G.7**, world-state ao vivo→**G.8**, AOT-do-plugin→**CI.7**, SCA-do-plugin→**LEG.3**. Novo item de polish reportado pelo criador: **F2-G.5-UX** (GambitPredict sem feedback visível). Ordem/ondas inalteradas. Próximo de maior valor (W4): validar G.5 no editor, depois **F2-G.EXPLORE** (movimento/interação) ou **F2-G.7** (diálogo na tela, já com a fundação E.6 pronta).

**⚠ PIVOT DE STACK — DECISÃO DO LÍDER SUPREMO (2026-06-21, ULTRA-PRIORITÁRIO):** o criador decidiu refatorar o projeto para nova base técnica e de design. **O QUÊ está decidido (one-way doors do líder); o COMO sai de brainstorm bigtech colaborativo (RF-7).** Resumo: stack **C++/Qt6** + **engine própria** (jogo + áudio/música in-house, fim do Godot), câmera **fixa top-down** (Zelda ALttP / Stardew), combate **por turnos estilo Pokémon**, estilo visual Stardew, porte → **grande**. **Consequência:** o roadmap Godot/C# abaixo (W0–W7, ADR-002 C#/AOT, ADR-003 DialogueManager, pillar câmera 3/4, combate CTB/cartas/Gambito) fica **SOB REVISÃO / SUSPENSO** pendente do brainstorm — NÃO apagar nem reordenar até a estratégia de migração ser decidida (o que se reaproveita vs reescreve). Os RF-* abaixo só registram o escopo; ordem/prioridade/dependências finas saem de RF-7.

**Atualização 2026-06-23 (re-pivot SDL EM EXECUÇÃO + trilha de arte):** o pivot RF-* avançou e SUPEROU a primeira intenção. O brainstorm RF-7 levou a **C++20 + Qt6** e, ao testar o M1 na prática, a um **RE-PIVOT de Qt6 → SDL3 + RmlUi + miniaudio (ADR-008)** (motivos: risco do Qt RHI semi-privado, gamepad nativo, binário ~10x menor, portabilidade console). **Estado real:** RF-1 (stack) e RF-2 (engine própria) **EM EXECUÇÃO** — `GusEngine/` em 4 camadas (core/domain POCO + platform/app SDL); board M0–M9 com M0/M2/M3/M4-lógica/M5-motor feitos e **M1 (camada visual) ENTREGUE em SDL** (janela + loop + render2d + input + gamepad + sprite animado). **824 testes**, pushado até `6b2bba9`. **Novos itens desta onda** (a sequenciar num `/tab_pendencias --reorder` futuro): pipeline de arte PixelLab Pro (party + 16 secundários gerados, ~76 na fila), **demo Gus jogável** (walk/idle/respiração), **viewer de animação** (`--anim-preview`), **mecânica de Carga do aparato** canonizada (`docs/design/mecanicas/stamina.md`, projetada por lead-game-designer + economy-designer). Pendências pequenas da Carga: cabear "Carga 0 corta o sprint" (gancho 1 linha) + decidir drain por input vs pós-colisão. Caminho e histórico canônicos: ver **ROADMAP.md** + **CHANGELOG.md**. Re-sequenciamento fino do board pós-SDL fica pra sessão dedicada de `/tab_pendencias`.

**Atualização 2026-06-23 (parte 2), trilhas paralelas (narrativa/docs/RAG, NÃO bloqueiam o board de código):** sessão longa pós-M1-SDL. (1) **Conlang "Sylvarin" criada** (brainstorm colaborativo com o criador): 4 docs canônicos em `docs/narrative/lingua/` (00-arquitetura/espinha, 01-fonologia, 02-lexico-semente de 13 raízes, 03-gramatica-nucleo). Sabor élfico + português forte, gramática rígida (Pillar 2), mutação consonantal como assinatura, escrita cifrada da Era 1, padroes numericos recorrentes nas contagens; as raízes explicam nomes já canon (Sylvarin, Selve Sombria, Neo-Sylvania, Vyrdragon). Pendente: mutações nasal/mista, deriva histórica Era 1 para Era 3, sistema de escrita cripto-glifo. (2) **RAG aumentado:** baixados 79 arquivos de língua élfica de Tolkien (`resources/livros/elvish/`, gitignored) e criado um **2º índice ISOLADO `rag_elvish` (1.989 chunks)** sem tocar o principal (163.443). Incidente resolvido: um `rag ingest` no índice principal removeu os 306 livros (corpus-fonte fora do disco), RESTAURADO via versão do LanceDB (v582). (3) **Docs higienizados pro SDL3 (2ª passada):** ~10 docs que ainda citavam Godot/C#/Qt como atual (blockout, knowledge-gates, style-guide, ui-spec, leitmotivs, dialogue-tree, libs-vendoring, runtime-hardening, deep/_INDEX) + bibliografia nova (`docs/narrative/bibliografia-rag.md`). (4) **README** higienizado + seção **Agradecimentos** (pessoas, IAs com URLs, autores do corpus com link, ferramentas FOSS) + parâmetros do RAG; **descrição do repo Codeberg** atualizada (Godot/Fase 1 para C++20+SDL3/Fase 2). Pushado até `13bdbf2`. **Board técnico (M0-M9) inalterado**; re-sequenciamento fino pós-SDL segue pendente de `/tab_pendencias --reorder` dedicado.

**Atualização 2026-06-23 (parte 3), LOOP JOGAVEL (M4) FECHADO + constelacao ativada:** o M4 visual fechou e foi VALIDADO pelo lider no display. Cidade Distritos Inferiores carregada do formato de mapa `.gmap` binario proprio SELADO (HMAC-SHA256 + UUID anti map-swap; review do security-engineer = adequado a ameaca, 0 criticos) + compilador CSV->.gmap + camera com zoom (43px/tile, follow+clamp) + walk polido (cadencia relativa ao tile, respiracao so-bob, histerese anti deslize-no-spam). **M4 = checkmark.** Commits `90ffa6c` (loop) + `0f178be` (binding UUID). 900 testes verdes. **Constelacao bigtech ATIVADA** (Cosimo deu o mapa: 3 C-levels CEO/CTO/CPO + game-producer; regua de 3 niveis: thread direto pra ajuste fechado, game-producer pra 2+ frentes, C-level + AskUserQuestion pra one-way doors). **Proxima frente: M5 BattleScreen** (motor turn_combat ja portado/auditado; falta a apresentacao), a comecar por brainstorm da tela. WIP=2 frentes de codigo. Higiene Qt->SDL: M4/M6 feitos; resto do `--reorder` pendente.

### ## Notas

- **One-way doors (W0)** DEVEM ir ao criador supremo via AskUserQuestion (opção recomendada primeiro), conforme regra canônica inquebrável. Nenhuma foi decidida pelos agents — apenas ordenadas e marcada a reversibilidade.
- **3 arestas de dependência reais AUSENTES do TODO original** (gravadas agora): (1) F2-G.5 depende de F2-E.10 + F2-E.10b, não só F2-E.5 (= F2-PROD.5); (2) F2-E.11 depende de F2-E.5b (status inertes), não só F2-E.5; (3) F2-M.4 depende fisicamente de export templates (dir vazia) + export_presets.cfg (ausente) + SDK pin.
- **Caminho crítico técnico do VS** (mais longo até jogável, antes invisível): F2-E.3.PARTY → F2-E.10-CONTRACT → F2-E.10 → F2-E.10b → F2-G.5/G.9.
- **Anti-over-engineering (porte solo):** a engine-foundation está madura/testada (combate 133 testes, save HMAC, scene, input 37 actions, i18n, câmera). O caminho crítico AGORA é Game+Arte, não foundation. Cortado/adiado: §18 catálogo completo (→2-3 arenas), música 3 estados (→2), FsCheck/lockfile/ICU plural/SCA pesado.
- **F1.11** (`docs/tech/*`): 🟡⚠ — gerado em modo autônomo pré-reforma, aguarda revisão criador. **F1.12** (GDD): ✅ F1.12-REFRESH aplicado 2026-06-02; gdd.md v0.2 canônico.
- **Cortes G1 (`CUT.*`)** = decisões one-way doors. ✅ CUT-RECONCILE 2026-06-02: GDD §9 fonte primária (CUT.1..17); CUT.* no TODO rastreia internamente.
- **Disciplina antes de release** (memória `project_qa_deploy_disciplina`): AUD zerado + assinatura nominal em deploy irreversível (sub-fase 5.3: 48h + 30d offline).
- **F1-DL.REFAC (débito literário)**: prose §§6-8 prioriza densidade técnica obsessiva. Refactor antes de F5-BK.1. Não bloqueia código.
- **Padroes numericos descontinuados em MÉTRICAS DE PROCESSO** (palavras/queries) mas **MANTIDOS no TEXTO** (datações/contagens/idades/dimensões/compassos); a camada velada do conteudo segue mantida. Detalhe so em `docs/_secret/` + memorias privadas.

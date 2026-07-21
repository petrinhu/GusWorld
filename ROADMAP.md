# Roadmap

O caminho do GusWorld, da lore à release. Este arquivo é o mapa de alto nível; o detalhe vive em outros docs (linkados, não duplicados):

- Board executável com critérios de saída testáveis: [TODO.md](TODO.md).
- Design da engine do pivot: [`docs/tech/pivot/engine-design.md`](docs/tech/pivot/engine-design.md).
- Histórico do que já mudou: [CHANGELOG.md](CHANGELOG.md).
- Decisões one-way door: [`docs/tech/adr/`](docs/tech/adr/).

Legenda: ✅ feito / auditado · 🔍 entregue, aguarda verificação · 🔄 em andamento · ⏳ pendente.

Idioma: pt-br.

---

## Fase 1: deep-lore (concluída)

✅ Worldbuilding canônico antes do código: ~365k palavras (Era 1 §§1-10, R2 Facções, R3 Settings, Blocos F/G/H/I), pillars, personagens, settings, timeline. Base em [`docs/narrative/`](docs/narrative/), [`sinopse.md`](sinopse.md), [CHARS.md](CHARS.md), [PLACES.md](PLACES.md).

A pausa da deep-lore como gate, com retomada paralela orgânica, está em [ADR-001](docs/tech/adr/ADR-001-pivot-lore-to-engine.md). O deep-lore restante (F1-DL.4-9 + REFAC) não bloqueia o código; entra entre os steps técnicos.

---

## Fase 2: vertical slice (em andamento)

Meta: vertical slice jogável (4-6 meses), um recorte vertical que prova o jogo de ponta a ponta.

Escopo do slice:
- 1 área pequena de cidade (overworld explorável).
- 1 encontro de combate turn-based.
- 1 puzzle Vetor do Gambito (ver [`docs/design/mecanicas/puzzle-gambito.md`](docs/design/mecanicas/puzzle-gambito.md)).
- Engine modular reutilizável + save funcional.

### Board de marcos (M0-M9)

A migração é faseada anti big-bang: cada marco fecha pelo seu critério de saída testável, e o Godot legado só é apagado no M8, depois que a engine nova provar paridade jogável (M7). Critérios de saída completos em [TODO.md](TODO.md).

| Marco | Status | O que é |
|---|---|---|
| M0 Andaime | 🔍 | Repo C++ + CMake + presets + framework de teste. Build Linux verde + `ctest` passando |
| M1 Janela + loop + sprite | ✅ | Janela + loop de tempo fixo + render2d + input. Boneco anda no mapa, câmera presa ao mapa. Validado no display do líder (2026-06-23). Fronteira reescrita em SDL3 (ver re-pivot abaixo) |
| M2 Input | ✅ | Eventos para ações lógicas + input_remap + persistência de controles + save V4 ([ADR-007](docs/tech/adr/ADR-007-controls-json-hash128-save-v4.md)). Validado ao vivo pelo líder de ponta a ponta (2026-07-07) |
| M3 Lógica pura portada | ✅ | Save + i18n + progression + templates em POCO C++ puro. 174 testes verdes, cripto bate vetores FIPS/RFC ([ADR-006](docs/tech/adr/ADR-006-crypto-hmac-formato-domain.md)), auditado |
| M4 Cena top-down | ✅ | Loop jogável fechado e validado ao vivo pelo líder (2026-06-23): tilemap + colisão de grade + clamp de câmera + render na plataforma |
| M5 Combate + tela de batalha | ✅ | Motor `turn_combat` portado inteiro (~200 testes de combate) + BattleScreen cockpit "Tático" entregue via glintfx (paridade visual + dados vivos + rodada de polish, [ADR-010](docs/tech/adr/ADR-010-adopt-glintfx-embed-mode.md)). Saída "overworld → batalha → resultado → volta" validada ao vivo pelo líder (fechado 2026-07-07) |
| M6 Áudio | ✅ | Camada de áudio (miniaudio) + música + SFX + fade entre telas. Crossfade cidade↔arena validado ao vivo pelo líder (concluído 2026-07-03) |
| M7 Paridade jogável | ✅ | **FECHADO 2026-07-21 (playthrough ao vivo, Gus).** Loop completo (andar, NPC/Bertoldo, combate→Victory, autosave, load) rodou 100% na engine C++/SDL3+glintfx, sem Godot, zero erro (exit 0). 3 feedbacks de polish do playtest viraram follow-up na INBOX (M7-FB1 corner-exploit test, M7-FB2 maximize/taskbar→glintfx, M7-FB3 menu inicial com fundo próprio) — não são paridade, não bloqueiam. Destrava M8/M9 + integração glintfx |
| M8 Decommission | ⏳ | Apagar Godot + C# + addons. Repo compila e roda sem nenhum bit do stack antigo. Gate de build Windows já PRÉ-CUMPRIDO (CI Windows real MSVC verde desde 2026-07-14) |
| M9 Higienização | ⏳ | Limpar a árvore pós-porte, remover resíduo do stack antigo, normalizar `GusEngine/` |

Ordem real de execução: M0, depois M1 e M3 em paralelo, depois M2, M4, M5, M6, M7, M8, M9. **M0-M7 entregues — M7 (paridade) FECHADO 2026-07-21 pelo playthrough ao vivo (Gus); M8/M9 destravados.**

### Onda paralela: motor de cartas techMagic (não bloqueia o M7)

Onda `CARDS`, paralela ao board M0-M9 (não faz parte da fila de marcos, roda em paralelo desde 2026-07-14). Executor de conjuros `techMagic` data-driven ([ADR-016](docs/tech/adr/ADR-016-techmagic-effect-engine-data-driven.md)): MVP dos 5 steps entregue (records/enums, dispatcher de hooks, ledger cross-ator, catálogo de 8 cartas, RepeatLastAction) + EffectKinds adicionais entregues desde então (ChainDamage/Tesla, DelayAction/Einstein, o "Balde B" Faraday/Newton/Gödel). Cada EffectKind novo é verificado adversarialmente (mutation testing por `qa-engineer`, revisor distinto do implementador) antes de fechar. Próximo da fila: DamageQuantize/Planck. Detalhe completo em [ADR-016](docs/tech/adr/ADR-016-techmagic-effect-engine-data-driven.md), [`docs/design/mecanicas/cartas-technomagik.md`](docs/design/mecanicas/cartas-technomagik.md) e [TODO.md](TODO.md) (linha `CARD-ENGINE-MANIFESTO`). Paralelo a essa onda: a estética visual de "terminal" para logs de combate e telas de sistema (design fechado, implementação em curso).

### Re-pivot da plataforma: Qt6 para SDL3 (3 fases)

A camada de plataforma deixou de ser Qt6 e passou a ser SDL3 + RmlUi + miniaudio ([ADR-008](docs/tech/adr/ADR-008-repivot-qt-to-sdl3.md)). A lógica pura (`core`/`domain`) não muda; só a fronteira `platform/` + a casca `app/` são reescritas. A UI/HUD passou a ser servida pelo glintfx (embed mode), que embrulha o RmlUi 6.3 + backend GL3; o backend vendorizado à mão foi aposentado ([ADR-010](docs/tech/adr/ADR-010-adopt-glintfx-embed-mode.md)). As fases do re-pivot atravessam os marcos do board:

| Fase do re-pivot | Status | Escopo | Marco do board |
|---|---|---|---|
| Fase 1 (fronteira M1) | ✅ | platform/window + render2d (`SDL_Renderer`) + input com gamepad; sprite do Cauã reintegrado; smoke headless `SDL_VIDEODRIVER=dummy`; backend Qt aposentado | M1 |
| Fase 2 (metades visuais) | ✅ | A metade visual de M2 (input/IO) e M4 (tilemap render) nasceu em SDL | M2, M4 |
| Fase 3 (UI do jogador) | 🔄 | Menus, batalha estilo Pokémon, diálogo, inventário via glintfx (embed mode, [ADR-010](docs/tech/adr/ADR-010-adopt-glintfx-embed-mode.md)). Entregues: cockpit "Tático" da batalha, menu de pausa + config (áudio/controles), tela de save/load com avisos, diálogo (caixa quente + tela-terminal), tela de dificuldade. Em curso: estética "terminal" para logs de combate, targeting de aliado nas cartas do Balde B | M5+ |

---

## Trilha de arte (paralela)

Produção de assets assistida por IA, do lore ao sprite (pipeline detalhado no [README](README.md)). Roda em paralelo ao código; o slice usa placeholder-first e troca por arte final conforme ela fica pronta.

| Item | Status | O que é |
|---|---|---|
| Prompts canônicos | 🔄 | Prompts visuais derivados do lore para cada personagem |
| Party + secundários | 🔍 | Party + 16 personagens secundários gerados |
| Sprite jogável do Gus | 🔍 | Gus no overworld (walk de 7 frames + idle com respiração procedural), no lugar do retângulo |
| Esquema de locomoção/animação | ✅ | 3 poses de walk em ping-pong, 4 direções únicas (sem flip, Pillar 3). Ver [`docs/design/mecanicas/locomotion.md`](docs/design/mecanicas/locomotion.md) e [`docs/design/mecanicas/animation-plan.md`](docs/design/mecanicas/animation-plan.md) |
| Viewer de animação | ✅ | `gusworld_app --anim-preview` para inspecionar ciclos na engine |
| Tilesets + cenário cidade | ⏳ | Arte da área de cidade do slice (estilo Stardew) |
| Sprites de combate | ⏳ | Apresentação da batalha estilo Pokémon |

---

## Pós-vertical slice

Depois que o slice provar o jogo e a engine atingir paridade jogável (M7) e decommission (M8/M9), o foco vira conteúdo e release. Mapeamento de marco para versão em [CHANGELOG.md](CHANGELOG.md) (seção "Marcos planejados").

- **Mecânicas pós-VS:** scan ambiental consumindo Carga, Celula de Pulso, foreshadow da sabotagem do Dante, escopo completo de combate. Ver [`docs/design/mecanicas/stamina.md`](docs/design/mecanicas/stamina.md).
- **Produção (`v0.5.x`-`v0.9.x`):** conteúdo completo, balanceamento, gates de a11y para v1.0.0.
- **Beta (`v0.9.x`):** beta fechada, bug bash, localização en-intl (dev em pt-br; tradução pós-1.0).
- **Release (`v1.0.0`):** Linux + Windows. Vol 1 (livro lore-bible) + Vol 2 (antologia narrativa).
- **Wiki + doc para iniciante:** uma wiki inicial ("starter") já foi publicada em 2026-07-14 nos dois remotos (Codeberg, bilíngue EN/PT, foco técnico/contribuidor; GitHub, PT-br, foco leigo/iniciante). A versão completa e ainda mais didática, cobrindo tudo em detalhe (derivada de `docs/`, não duplica), fica para depois da tag de versão.

---

## Decisões one-way door (ADRs)

As viradas de stack e os contratos irreversíveis estão registrados em [`docs/tech/adr/`](docs/tech/adr/):

| ADR | Decisão |
|---|---|
| [ADR-001](docs/tech/adr/ADR-001-pivot-lore-to-engine.md) | Pivot Fase 1 (lore) para Fase 2 (engine) |
| [ADR-002](docs/tech/adr/ADR-002-csharp-aot-over-gdscript.md) | GDScript para C# .NET 8 AOT (depois superado) |
| [ADR-003](docs/tech/adr/ADR-003-dialogue-library.md) | Biblioteca de diálogo (Godot) — **SUPERADO pela ADR-014** |
| [ADR-004](docs/tech/adr/ADR-004-environment-modifier-contract.md) | Contrato do `EnvironmentModifier` (§18 combate) |
| [ADR-005](docs/tech/adr/ADR-005-license-gpl3-assets-ccbysa.md) | GPLv3 (código) + CC-BY-SA 4.0 (assets) |
| [ADR-006](docs/tech/adr/ADR-006-crypto-hmac-formato-domain.md) | Cripto HMAC-SHA256 + formato de serialização do domain |
| [ADR-007](docs/tech/adr/ADR-007-controls-json-hash128-save-v4.md) | Persistência de controles + save V4 anti-tamper |
| [ADR-008](docs/tech/adr/ADR-008-repivot-qt-to-sdl3.md) | Re-pivot Qt6 para SDL3 + RmlUi + miniaudio |
| [ADR-009](docs/tech/adr/ADR-009-rmlui.md) | Adoção do RmlUi (UI HTML/CSS-like) |
| [ADR-010](docs/tech/adr/ADR-010-adopt-glintfx-embed-mode.md) | Adotar glintfx (embed mode) como motor de UI/HUD; aposentar o backend RmlUi vendorizado |
| [ADR-011](docs/tech/adr/ADR-011-m6-audio-onda1-plano.md) | Plano da onda 1 de áudio (miniaudio) do M6 |
| [ADR-012](docs/tech/adr/ADR-012-m7-paridade-jogavel-plano.md) | Plano de paridade jogável do M7 |
| [ADR-013](docs/tech/adr/ADR-013-asset-source-vfs-fase1.md) | Asset source VFS fase 1 (porteiro de assets) |
| [ADR-014](docs/tech/adr/ADR-014-dialogue-runtime-poco.md) | Runtime de diálogo POCO C++20 (supersede ADR-003) |
| [ADR-015](docs/tech/adr/ADR-015-save-security-v2-offline.md) | **Save security v2 offline (AEAD/Monocypher/machine-bind/anti-rollback)** — o mais recente e sensível |
| [ADR-016](docs/tech/adr/ADR-016-techmagic-effect-engine-data-driven.md) | **Motor de efeitos das cartas = executor data-driven `techMagic` (não VM)** — desbloqueia as cartas especiais (2026-07-14) |


---

## Migrado do TODO.md em 2026-06-25

## Ondas de execução (Cosmo/COO 2026-05-29)

- **W0 — Salvaguarda + One-Way Doors + Higiene.** Backup off-site (submodule-first), decisões irreversíveis em lote via AskUserQuestion ao criador (renderer, hardware-floor, AOT-erratum, lib diálogo, contrato .gdt, modelo schema, licença, placeholder-first, escopo §18), reconciliação doc↔código. Maioria é decisão/edição barata, não código. Saída: zero one-way-door aberto + remoto com backup + canon não-mente.
- **W1 — Fundação barata + re-baseline de escopo.** Folhas POCO testáveis (coverlet, save HMAC tests, MathHelpers, localization/input, fix parser i18n), GDD refresh + cortes reconciliados, roadmap VS, fechar trackers de auditoria, C-EX §10 restante. Paralelizável, sem one-way-door.
- **W2 — Design-spec do VS.** Specs que destravam produção (balance prototype com números reais, puzzle Gambito, Knowledge curve, core loop fora de combate, onboarding, economia mínima, dialogue tree blueprint, ludonarrative check) + plano de produção com exit-criteria + RAID + mapear aresta F2-E.10→G.5. Spec PRIMEIRO, impl depois.
- **W3 — Cadeia de dados do combate + status + diálogo + áudio-bus.** TemplateSerializer → CharacterRepository, wiring de status inertes, ambientes (escopo cortado 2-3 arenas) + TDD, source-gen JSON, blockout, spike de arte (lateral), audio bus + AudioDirector. Caminho crítico técnico do VS.
- **W4 — Loop jogável placeholder-first.** Movimento/interação/triggers, combate jogável + HUD com cápsulas, puzzle, diálogo 1 NPC, save/load, inventário/deck, quest, loot, SFX/hooks, concepts do VS + validações de arte. Saída: VS end-to-end JOGÁVEL com grayboxing = base de F2-M.3 cedo.
- **W5 — Arte no caminho crítico (substitui placeholders) + áudio mínimo.** Shader-gate (outline+toon) ANTES de modelar Gus, modelo+rig+locomotion+inimigo+anims, atlas + import Nearest, demais shaders, art sheets, música adaptive 2 estados. NÃO bloquear fun-loop atrás disto.
- **W6 — CI + segurança + build (depois do runner; não bloquear VS).** Runner → workflow enxuto → test job + gitleaks + CVE scan + i18n lint, export templates + presets + PoC AOT + wrappers. Export local manual primeiro. Diferidos anti-OE: FsCheck, lockfile, ICU plural.
- **W7 — Milestone + produção + legal-release.** VS coeso + perf + playtest instrumentado + build distribuível + NOTICE/atribuição + marcador revisão ADR-001.
- **W7+ — Pós-VS (F3-F5 + livro).** Alpha/Beta/Gold + age rating + auditoria final + tradução/publicação do livro.
- **`—` — Concluído / Decisão tomada / CUT.** Sem onda de execução.

## Roadmap VS — Now / Next / Later

**Atualizado 2026-07-21 (M7 FECHADO pelo playthrough ao vivo; mirror+Wiki publicados; ondas `CARDS`/`CARDS-HW` em curso; sincronizado com [TODO.md](TODO.md)).** M0-M7 entregues — o M7 (paridade jogável, fecha o board da migração) foi validado ao vivo (loop completo sem Godot, zero erro). Em paralelo ao board, a onda `CARDS` (motor de cartas techMagic) avança um `EffectKind` por vez.

| Now (CONCLUÍDO) | Next (próxima onda) | Later |
|---|---|---|
| **M0-M7 motor+plataforma portados** (save/i18n/progression/templates/combat/crypto/áudio) em C++20+SDL3; M3, M5-DMG, M6 e **M7 (paridade jogável)** validados ao vivo | **M7 FECHADO 2026-07-21** (playthrough ao vivo; loop completo sem Godot, zero erro); 3 feedbacks de polish na INBOX (M7-FB1/2/3) | **M8 Decommission** (apagar Godot/C#/addons; gate de build Windows já pré-cumprido) + **M9 Higienização** — DESTRAVADOS |
| **M7-DIALOGO** (runtime de diálogo POCO, [ADR-014](docs/tech/adr/ADR-014-dialogue-runtime-poco.md)) + **DIALOGO-TERMINAL** (tela-terminal/caixa-quente) entregues e testados ao vivo | **Onda `CARDS`** (motor de cartas techMagic, [ADR-016](docs/tech/adr/ADR-016-techmagic-effect-engine-data-driven.md)): próximo EffectKind = DamageQuantize/Planck; resíduos de UI (targeting de aliado do Balde B) | **ARTE-DIAGONAL-8DIR** (decisão 4-dir vs 8-dir pendente) |
| **Mirror GitHub + Wiki (GitHub e Codeberg) + AI-DISCLOSURE.md** publicados (2026-07-14); CI Windows real (MSVC) verde | **Estética "terminal"** para logs de combate e telas de sistema (design fechado, implementação em curso) | Sistema de deck/mão (brainstorm inicial, ainda sem decisão fechada) |
| **CARD-ENGINE-MANIFESTO em curso:** ChainDamage/Tesla, DelayAction/Einstein e o "Balde B" (Faraday/Newton/Gödel) entregues e verificados adversarialmente | **CARTAS-BALANCEAMENTO:** statlines finais das 20 cartas dos mestres + playtest N=3 dos valores exatos | **Conlang Sylvarin** (paralelo orgânico): mutações, deriva histórica, escrita cifrada |

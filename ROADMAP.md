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
| M1 Janela + loop + sprite | 🔍 | Janela + loop de tempo fixo + render2d + input. Boneco anda no mapa, câmera presa ao mapa. Fronteira já reescrita em SDL3 (ver re-pivot abaixo) |
| M2 Input | 🔍 | Eventos para ações lógicas + input_remap + persistência de controles + save V4 ([ADR-007](docs/tech/adr/ADR-007-controls-json-hash128-save-v4.md)). Falta backend de evento + I/O em disco |
| M3 Lógica pura portada | ✅ | Save + i18n + progression + templates em POCO C++ puro. 174 testes verdes, cripto bate vetores FIPS/RFC ([ADR-006](docs/tech/adr/ADR-006-crypto-hmac-formato-domain.md)), auditado |
| M4 Cena top-down | 🔍 | Tilemap + colisão de grade + clamp de câmera (lógica pura feita). Falta a metade visual (tilemap render na plataforma) |
| M5 Combate + tela de batalha | 🔄 | Motor `turn_combat` portado e endurecido (fórmula de dano §11 evoluída e auditada, ver M5-DMG). BattleScreen cockpit "Tático" entregue via glintfx (paridade visual + dados vivos, [ADR-010](docs/tech/adr/ADR-010-adopt-glintfx-embed-mode.md)) |
| M6 Áudio | ⏳ | Camada de áudio (miniaudio) + música + SFX + fade entre telas |
| M7 Paridade jogável | ⏳ | Loop completo (andar, NPC, combate, save, carregar) 100% na engine nova, sem Godot |
| M8 Decommission | ⏳ | Apagar Godot + C# + addons. Repo compila e roda sem nenhum bit do stack antigo |
| M9 Higienização | ⏳ | Limpar a árvore pós-porte, remover resíduo do stack antigo, normalizar `GusEngine/` |

Ordem real de execução: M0, depois M1 e M3 em paralelo, depois M2, M4, M5, M6, M7, M8, M9.

### Re-pivot da plataforma: Qt6 para SDL3 (3 fases)

A camada de plataforma deixou de ser Qt6 e passou a ser SDL3 + RmlUi + miniaudio ([ADR-008](docs/tech/adr/ADR-008-repivot-qt-to-sdl3.md)). A lógica pura (`core`/`domain`) não muda; só a fronteira `platform/` + a casca `app/` são reescritas. A UI/HUD passou a ser servida pelo glintfx (embed mode), que embrulha o RmlUi 6.3 + backend GL3; o backend vendorizado à mão foi aposentado ([ADR-010](docs/tech/adr/ADR-010-adopt-glintfx-embed-mode.md)). As fases do re-pivot atravessam os marcos do board:

| Fase do re-pivot | Status | Escopo | Marco do board |
|---|---|---|---|
| Fase 1 (fronteira M1) | ✅ | platform/window + render2d (`SDL_Renderer`) + input com gamepad; sprite do Cauã reintegrado; smoke headless `SDL_VIDEODRIVER=dummy`; backend Qt aposentado | M1 |
| Fase 2 (metades visuais) | 🔄 | A metade visual de M2 (input/IO) e M4 (tilemap render) nasce em SDL | M2, M4 |
| Fase 3 (UI do jogador) | 🔄 | Menus, batalha estilo Pokémon, diálogo, inventário via glintfx (embed mode, [ADR-010](docs/tech/adr/ADR-010-adopt-glintfx-embed-mode.md)); cockpit "Tático" da batalha entregue | M5+ |

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
- **Wiki + doc para iniciante:** após a tag de versão, Wiki nativa do repo + documentação `.md` didática para iniciante em computação (derivada de `docs/`, não duplica).

---

## Decisões one-way door (ADRs)

As viradas de stack e os contratos irreversíveis estão registrados em [`docs/tech/adr/`](docs/tech/adr/):

| ADR | Decisão |
|---|---|
| [ADR-001](docs/tech/adr/ADR-001-pivot-lore-to-engine.md) | Pivot Fase 1 (lore) para Fase 2 (engine) |
| [ADR-002](docs/tech/adr/ADR-002-csharp-aot-over-gdscript.md) | GDScript para C# .NET 8 AOT (depois superado) |
| [ADR-003](docs/tech/adr/ADR-003-dialogue-library.md) | Biblioteca de diálogo (contexto Godot, sob revisão pós-pivot) |
| [ADR-004](docs/tech/adr/ADR-004-environment-modifier-contract.md) | Contrato do `EnvironmentModifier` (§18 combate) |
| [ADR-005](docs/tech/adr/ADR-005-license-gpl3-assets-ccbysa.md) | GPLv3 (código) + CC-BY-SA 4.0 (assets) |
| [ADR-006](docs/tech/adr/ADR-006-crypto-hmac-formato-domain.md) | Cripto HMAC-SHA256 + formato de serialização do domain |
| [ADR-007](docs/tech/adr/ADR-007-controls-json-hash128-save-v4.md) | Persistência de controles + save V4 anti-tamper |
| [ADR-008](docs/tech/adr/ADR-008-repivot-qt-to-sdl3.md) | Re-pivot Qt6 para SDL3 + RmlUi + miniaudio |
| [ADR-010](docs/tech/adr/ADR-010-adopt-glintfx-embed-mode.md) | Adotar glintfx (embed mode) como motor de UI/HUD; aposentar o backend RmlUi vendorizado |


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

**Atualizado 2026-06-23 (pos-pivot SDL3, board M0-M9; game-producer).** O LOOP JOGAVEL (M4) fechou e foi validado no display; o proximo pilar e o COMBATE (M5 BattleScreen). WIP=2 frentes de codigo (conlang fora do WIP; sprites bloqueados nao contam).

| Now (CONCLUIDO) | Next (proxima onda) | Later |
|---|---|---|
| **M0-M5 motor portado** C++/SDL (save/i18n/progression/templates/combat/crypto); M3 e M5-DMG auditados | **M5 BattleScreen** (motor turn_combat ja portado/auditado; falta a apresentacao): comecar pelo BRAINSTORM da tela (layout, posicoes, UI de cartas, transicao overworld->batalha->volta, feel Pokemon) | **Tiled** + pipeline de mapas (brainstorm lore -> blockout level-designer -> .tmj -> compilador -> .gmap) |
| **M1 visual** (janela+loop+sprite SDL) validado | Trilha de ARTE em paralelo (a 2a vaga do WIP): re-arte do walk com pe-plantado (zera foot-slide) + **ARTE-RESP-4DIR** (respiracao nas 4 direcoes) | **M6 audio** (miniaudio), M7 paridade jogavel, M8 decommission Godot/C#, M9 higiene |
| **M4 loop jogavel** (cidade do .gmap selado HMAC+UUID + camera/zoom + walk polido + anti-deslize) | (Sprites: **76 secundarios** bloqueados pela API PixelLab/500; re-rodar em lote quando estabilizar) | **Conlang Sylvarin** (paralelo organico): mutacoes, deriva historica, escrita cifrada |
| Mecanica **Carga do Aparato** canonizada (numeros) | **COMBATE-AUTOKILL** (INBOX, parametros fechados): canonizar no combat.md DENTRO do M5 | **ARTE-DIAGONAL-8DIR** (decisao 4-dir vs 8-dir) |

# TODO — GusWorld

Backlog canônico do projeto. Atualizar via skill `/tab_pendencias` ou manualmente. Status: `[ ]` pendente, `[~]` em andamento, `[x]` feito, `[-]` cancelado.

## Fase 1 — Concepção (concluída)

- [x] Pillars + GDD 1-page (`docs/design/`)
- [x] Lore bible + arco principal + party (`docs/narrative/`)
- [x] Style guide 3D low-poly (`docs/art/`)
- [x] Arquitetura Godot 4 modular + build pipeline (`docs/tech/`)

## Fase 2 — Implementação (próxima)

### Setup

- [ ] Instalar Godot 4 (versão LTS estável)
- [ ] Criar `/game/project.godot` inicial (Godot 4, GDScript, forward+ renderer)
- [ ] `git init` no projeto, primeiro commit baseline
- [ ] Criar repo Forgejo `gusworld` (privado inicial)
- [ ] Decidir: `gusworld-engine` como repo separado ou submódulo? (ver `docs/tech/architecture.md`)
- [ ] Setup `.gitignore` Godot (excluir `.import/`, `*.translation`, etc.)
- [ ] Setup Forgejo Actions runner local (se ainda não existir no vault)

### Engine — módulos prioritários (G1 vertical slice)

- [ ] `engine/orbital_camera/` — câmera 3/4 com rotação livre + zoom + collision-aware (alvo: prototype rodando primeiro)
- [ ] `engine/event_bus/` — autoload de signals globais
- [ ] `engine/save_system/` — JSON versionado `save_version: 1` + migrators stub
- [ ] `engine/scene_manager/` — fade in/out + scene loading async
- [ ] `engine/turn_based_combat/` — initiative queue + action resolution + state machine
- [ ] `engine/dialogue_system/` — Ink ou DialogueManager (decidir) + branching base
- [ ] `engine/input_remap/` — controles remappáveis (acessibilidade D1)

### Game — vertical slice scope

- [ ] Blockout 1 área pequena Distritos Inferiores (cidade ato 1, ~5min explore)
- [ ] Modelo low-poly Gus (3k-4.5k tris, 512² atlas) — silhouette test em 8 ângulos
- [ ] Locomotion básico Gus (idle / walk / run / interagir)
- [ ] 1 inimigo comum (800-1.5k tris)
- [ ] Combate turn-based: 1 encontro 1v1 com Compilação de Deck Rúnico (3-5 cartas básicas)
- [ ] Puzzle único Vetor do Gambito (predição trajetória, mini-board holográfico)
- [ ] Dialogue 1 NPC introdutório
- [ ] Save/load funcional
- [ ] HUD básico (HP, action points, deck)

### Arte vertical slice

- [ ] Atlas gradient cidade (256² ou 512², paleta ato 1)
- [ ] Shader outline inverted-hull reutilizável
- [ ] Shader holograma (cartas + UI rúnica)
- [ ] Material toon Gus
- [ ] 5-10 props modulares cidade (tile-based)

### Áudio vertical slice

- [ ] 1 track ambient cidade noturna (pós-punk + drone)
- [ ] SFX base: footstep, card-play, hit, scan-óculos, UI confirm/cancel
- [ ] Audio bus setup Godot (master / music / sfx / ui / vo)

### Build/CI

- [ ] Wrapper shell `scripts/build_linux.sh` + `scripts/build_windows.sh`
- [ ] Forgejo Actions YAML inicial: lint + import + export Linux/Win em PR
- [ ] Versioning convention (`v0.0.x` durante VS, `v0.1.0` no VS done)

### Milestone gate — Vertical Slice (alvo: ~4-6 meses)

Pra fechar VS:
- [ ] Tudo acima funcional end-to-end (5-10min gameplay coeso)
- [ ] 60fps @ 1080p em GTX 1060+
- [ ] 5 playtesters externos: time-to-fun ≤ 5min (métrica de sucesso do GDD)
- [ ] Build Linux + Windows distribuível (.tar.gz / .zip)
- [ ] Revisão style guide com dados reais de perf

## Fases futuras (placeholder)

- **Alpha** (~8m): todas features VS expandidas + ato 2 Selve em blockout
- **Beta** (~10m): content-complete, balance pass, localização base
- **Gold/RC** (~11-12m): cert (Steam manifest), age rating IARC, no critical bug
- **Release** + post-launch patch plan

## Cortes confirmados (G1 — ver `docs/design/gdd.md` §9)

Não fazer em v1:
- Multiplayer / co-op
- Open-world / mundo persistente
- Crafting system / economia complexa
- VO (voice acting)
- Mocap
- DDA (dynamic difficulty)
- Mod support / in-game editor
- Achievements / leaderboards (post-launch talvez)
- Localização além de pt-br no v1 (estrutura prevista, conteúdo pós)
- Console cert (Steam Deck verified vem depois)

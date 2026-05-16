# GusWorld — Arquitetura Técnica

> **Escopo:** projeto solo indie G1, 6-12 meses, single-player puro, PC (Linux + Windows). Godot 4.x.
> **Princípio diretor:** simplicidade agressiva. Cada decisão pondera *custo de iteração solo* contra *reaproveitamento futuro da engine modular*.

---

## 1. Engine e linguagem

### 1.1. Decisão: **Godot 4.3+ LTS** (estável, .NET opcional desativado em G1)

- 3D razoável (3/4 isométrica com `Camera3D` orbital cabe naturalmente).
- Scene/Node + signals = boa fit pra turn-based e dialogue.
- Export Linux + Windows nativo, sem royalties, sem account vendor lock-in.
- Comunidade GDScript ampla, plugins de TCG/dialogue/save já existem como referência.

**Recusado:** Unity (custo licenciamento/runtime-fee + churn de management). Unreal (overkill 3D AAA, GDScript-equivalente Blueprints prende menos solo, mas peso de build + asset pipeline é incompatível com G1 solo). Custom engine (anti-objetivo crítico — ver §10).

### 1.2. Linguagem: **GDScript** (decisão SIM)

| Critério | GDScript | C# |
|---|---|---|
| Iteração (hot reload, REPL editor) | **forte** | médio |
| Tooling Godot nativo (signals, editor hints) | **first-class** | second-class |
| Perf bruta | médio | forte |
| Portabilidade futura (sair de Godot) | fraca | média |
| Curva solo | **mínima** | exige .NET SDK + IDE separada |
| Tamanho de export | **menor** | +30-80MB runtime |

**Decisão:** GDScript pra 100% do gameplay G1. C# **não** entra em G1.

**Reserva de exceção:** se profiler mostrar hotspot real (ex.: A* pathfinding em mapa grande, simulação de fauna da Selve com N>1000 entidades), extrair como **GDExtension em C++** (não C#). Justificativa: C++ via GDExtension já é stack default do usuário (Qt23) — coerência cognitiva > poliglota. C# traria runtime sem ganho proporcional.

---

## 2. Separação engine/game

### 2.1. Critério de divisão

> *Engine = "se eu fizesse outro RPG turn-based amanhã, copio isto sem mudar".*
> *Game = "se eu mudar de jogo, jogo isto fora".*

### 2.2. Conteúdo de `/engine/` (reutilizável)

| Módulo | Responsabilidade |
|---|---|
| `orbital_camera` | Camera3D 3/4 isométrica-livre, rotação orbital (8 ou 16 stops, ou contínua), zoom, edge-pan, follow target opcional |
| `turn_combat` | State machine de turnos (Initiative → ActionSelect → Resolve → End), action queue, hooks de IA |
| `party` | Roster de personagens, stats genéricas (HP/MP/AP/buffs), equipamento slots agnósticos |
| `card_engine` | Deck/hand/discard/exile, custo, efeito como `Resource` com `apply(target, context)`, combos por tags |
| `dialogue` | Grafo (nós + escolhas + condições), parser de arquivo (ver §6), variáveis de sessão, hook localização |
| `save_system` | Snapshot serializável, slots, autosave, versionamento + migração (ver §5) |
| `scene_router` | Stack de cenas, transições, persistência de estado entre mapas |
| `audio_director` | Buses Master/Music/SFX/Voice/UI, ducking, crossfade entre tracks |
| `input_remap` | Bind table editável runtime, gamepad+kbm, persiste em `user://input.cfg` |
| `localization` | Wrapper sobre `tr()` + reload runtime + pluralização (ver §6) |
| `event_bus` | Signal global tipado pra eventos cross-system (sem virar god-object — limite ~20 sinais) |
| `puzzle_kit` | Primitivas: trigger, switch, gate, sequência, validador genérico (puzzles específicos ficam em `/game/`) |

### 2.3. Conteúdo de `/game/` (game-specific GusWorld)

- Lore, characters (Gus, NPCs, fauna da Selve), levels, mapas, scripts de quest.
- Conteúdo de diálogo, conteúdo de cartas (Código de Raiz, Pulso Elétrico, etc.).
- Balance data (`.tres` Resources), tabelas de loot, currency.
- Arte/áudio importados (sources brutos ficam em `/assets/`).
- Skins de UI (estética ciber-gótica + Selve).
- Mecânicas-âncora específicas: Sintonização Ortodôntica, Compilação Rúnica, Vetores do Gambito — implementadas **sobre** as primitivas da engine, não dentro dela.

---

## 3. Estratégia de reaproveitamento

### 3.1. Avaliação das opções

| Opção | Prós | Contras | Veredito G1 |
|---|---|---|---|
| Submodule git | Versão independente, fácil bump | Submodule é frágil em workflow solo, fácil esquecer push | **SIM (escolha)** |
| Godot addon (plugin.cfg em `addons/`) | Drag-and-drop entre projetos | Cada addon é silo, dependências entre addons viram dor | Sim **dentro** do submodule |
| GDExtension C++ | Perf, portável binário | Build cross-platform vira projeto em si | Só sob pressão de perf |
| Cópia manual | Zero overhead inicial | Drift entre projetos garantido | NÃO |
| Asset Library publish | Comunidade, versionamento | Burocracia, exige API estável | NÃO em G1 |

### 3.2. Decisão concreta

**Layout:**
- `/engine/` é um **repositório git separado** (`gusworld-engine` ou nome neutro tipo `vector-engine`), incluído em `gusworld` via **git submodule**.
- Dentro de `/engine/` cada módulo é um **Godot addon** (`engine/addons/<modulo>/plugin.cfg`). Isso garante que o projeto Godot do game (`/game/project.godot`) os ative individualmente via Project Settings → Plugins.
- Symlink ou path config: `/game/addons/` aponta pra `/engine/addons/` (symlink em dev; cópia no CI build se necessário).

**Razão:** solo G1 não quer maintainence overhead de 12 repos. 1 repo `engine` + 1 repo `game` = balance. Addons internos viram standalone naturalmente quando maduros.

**Versionamento da engine:** semver simples no submodule. Game cravado em commit hash. Bump deliberado.

---

## 4. Sistemas core (visão detalhada)

### 4.1. Camera orbital 3/4

- `Camera3D` filha de `SpringArm3D` filho de `Node3D` (pivot).
- Rotação orbital: input `camera_rotate_left/right` aplica yaw em stops de 45° (ou contínuo via hold + mouse drag).
- Zoom: spring length 6.0 → 20.0, smoothstep.
- Tilt fixo isométrico (~30-35°) com override opcional pra cutscene.
- Follow: target = `Node3D` exportado; tween de lerp pra evitar jitter.
- API: `set_target(node)`, `snap_to(angle)`, `shake(intensity, duration)`.

### 4.2. Turn-based combat

- State machine explícita: `RoundStart → TurnStart → ActionSelect (player ou AI) → ActionResolve → TurnEnd → (próximo) → RoundEnd`.
- Initiative: array ordenado por stat `speed` + tiebreaker determinístico.
- Action = `Resource` com `cost`, `range`, `targets`, `effect_script` (callable).
- AI: behavior tree leve ou utility-based (1 arquivo por archetype de inimigo).
- **Hook para Vetores do Gambito:** modo "preview" que executa simulação dry-run do próximo turno do inimigo e mostra trajetória holográfica.

### 4.3. Deck/cards system

- `CardResource`: id, name, cost (AP/MP), tags (`raiz`, `eletrico`, `gambito`...), effect_chain.
- Deck = `Array[CardResource]` + RNG seeded por save.
- Hand/Discard/Exile como `Array`. Operations: `draw(n)`, `play(card, target)`, `discard(n)`, `mill(n)`.
- **Compilação Rúnica:** combo detector — se hand contém cards com tags compatíveis, expõe ação "Compile" que gera card temporário com efeito merge. Tabela de combos em `/game/data/runic_combos.tres`.

### 4.4. Dialogue

- Grafo nodes-and-edges. Arquivo `.dialogue` (texto custom) compilado em `Resource` no import.
- Suporta: choices, conditions (`{has_item:tactical_glasses}`), variable set/get, jump, localized string key.
- Renderer = cena UI no `/game/` (engine só fornece o runtime do grafo).

### 4.5. Party management

- G1: party pequena (1-3 personagens). Não generalizar pra 6+ — over-engineering.
- Roster fora de combate vs active em combate.
- Stats: HP, MP, AP, Defense, Speed, status effects (array de `Resource` com duração).

### 4.6. Save/load

Ver §5 (detalhe).

### 4.7. Inventory

- Slot-less com stack quantity (simples G1). Categoria: card, key_item, consumable, equipment.
- Item = `Resource`. Inventory = `Dictionary[item_id, quantity]`.
- Equipment slots: glasses, ortho_matrix, drive — fixos canônicos do Gus (não generalizar).

### 4.8. Puzzle hooks

- Primitivas da engine: `Trigger`, `Switch`, `Gate`, `Sequence`, `Validator`.
- Puzzles específicos (sintonização de frequências, decode rúnico) ficam em `/game/scenes/puzzles/`.

---

## 5. Save format

### 5.1. Decisão: **JSON** (escolha SIM)

| Critério | JSON | Binary `Resource` (.tres/.res) |
|---|---|---|
| Debuggable a olho nu | **sim** | não |
| Diff em git (saves de teste) | **sim** | ruim |
| Forward-compat manual | **controlado** | depende de Godot |
| Tamanho | maior | menor |
| Velocidade | suficiente | mais rápido |
| Tampering por jogador | trivial | um pouco mais difícil |

**Veredito G1:** JSON. Single-player puro, anti-tampering é não-objetivo. Debug solo agradece JSON legível.

### 5.2. Schema D1

```json
{
  "save_version": 1,
  "engine_version": "0.1.0",
  "game_version": "0.1.0",
  "created_at": "2026-05-15T14:00:00Z",
  "playtime_seconds": 0,
  "scene": "res://game/scenes/levels/distrito_inferior.tscn",
  "player": { "position": [0,0,0], "stats": {...} },
  "party": [...],
  "inventory": {...},
  "deck": [...],
  "flags": {...},
  "quests": {...}
}
```

### 5.3. Versionamento e migração

- `save_version: 1` desde D1. **Inegociável.**
- Loader: `SaveSystem.load(path)` → detecta `save_version` → aplica chain de migrators (`migrate_1_to_2`, `migrate_2_to_3`...) → retorna struct atual.
- Migrators são funções puras em `/engine/addons/save_system/migrations/`.
- Forward-compat: campos desconhecidos em saves antigos = ignorados sem crash. Campos faltantes em saves antigos = preenchidos com defaults documentados.
- Slot layout: `user://saves/slot_N.json` + `user://saves/autosave.json` + `user://saves/quicksave.json`.

---

## 6. Localização

### 6.1. Decisão: **Godot `tr()` + CSV** (escolha SIM)

- v1 só pt-br, mas D1 com infra. Custo de retrofitar localização depois é alto.
- CSV (`localization/strings.csv`) com colunas `key, pt_BR, en_US, ...` — Godot importa nativo.
- Toda string visível ao jogador passa por `tr("KEY_ID")`. Sem string literal em UI.
- Pluralização: Godot 4 suporta `tr_n("KEY", "KEY_PLURAL", count)`. Usar.
- Formatação: `tr("KEY").format({"name": "Gus"})`. **Não** ICU MessageFormat (overhead pra solo).

**Recusado:** sistema próprio (NIH). ICU MessageFormat (gramáticas slavas não são problema G1, pt-br + en-US bastam).

### 6.2. Convenção de chaves

- `UPPER_SNAKE_CASE` com namespace: `UI_MAIN_MENU_PLAY`, `DIALOGUE_GUS_INTRO_01`, `ITEM_CARD_RAIZ_NAME`, `ITEM_CARD_RAIZ_DESC`.
- Diálogos: chaves geradas automaticamente pelo compilador de `.dialogue`.

---

## 7. Build pipeline

### 7.1. Targets

| Target | Formato | Signing |
|---|---|---|
| Linux x86_64 | `.tar.gz` contendo binário + `.pck` | não em G1 |
| Linux x86_64 | AppImage (stretch goal) | não em G1 |
| Windows x86_64 | `.zip` contendo `.exe` + `.pck` | **não em G1** (cert custa ~$300/ano; Defender SmartScreen vai reclamar; aceito) |

**Flatpak/Snap:** anti-objetivo G1. Avaliar pós-1.0.

### 7.2. Templates de export

- `export_presets.cfg` versionado no git (sem secrets).
- Templates oficiais Godot baixados via `godot --headless --download-export-templates` na primeira run de CI.
- Encryption key: **não** em G1 (custo > benefício pra single-player indie).

Detalhes de comandos em [`build.md`](./build.md).

---

## 8. Branching Forgejo

### 8.1. Modelo: **Trunk-based simplificado** (solo G1 não precisa GitFlow)

- `main` — sempre buildável. Tag `v0.X.Y` em milestones.
- `dev` — integração diária. Pode ter WIP. Merge em `main` em milestone.
- `feat/<short-name>` — feature de mais de 2 dias. Merge em `dev` via PR (mesmo solo, força reflexão).
- `fix/<short-name>` — hotfix em `main`, cherry-pick pra `dev`.
- Tags: `engine-v0.X.Y` no repo `engine`, `game-v0.X.Y` no repo `game`.

### 8.2. Commits

- Conventional Commits (`feat:`, `fix:`, `refactor:`, `docs:`, `chore:`).
- pt-br no corpo OK; tipo em inglês.
- Sem `--no-verify`, sem `--force` em `main`.

### 8.3. CI

- Forgejo Actions (esqueleto em [`build.md`](./build.md)).
- Job mínimo: lint GDScript (`gdtoolkit`) + import headless + export Linux/Windows + upload artifact.

---

## 9. Estrutura final do repo

```
gusworld/                          # repo "game" (root)
├── CLAUDE.md
├── README.md
├── LICENSE
├── sinopse.md
├── .gitignore
├── .gitmodules                    # aponta engine/ → repo gusworld-engine
│
├── engine/                        # SUBMODULE → gusworld-engine.git
│   ├── README.md
│   ├── VERSION                    # semver da engine
│   ├── addons/
│   │   ├── orbital_camera/
│   │   │   ├── plugin.cfg
│   │   │   ├── plugin.gd
│   │   │   ├── orbital_camera.gd
│   │   │   └── README.md
│   │   ├── turn_combat/
│   │   ├── party/
│   │   ├── card_engine/
│   │   ├── dialogue/
│   │   ├── save_system/
│   │   │   ├── plugin.cfg
│   │   │   ├── save_system.gd
│   │   │   └── migrations/
│   │   ├── scene_router/
│   │   ├── audio_director/
│   │   ├── input_remap/
│   │   ├── localization/
│   │   ├── event_bus/
│   │   └── puzzle_kit/
│   └── tests/                     # GUT (Godot Unit Test) suite da engine
│
├── game/                          # projeto Godot principal
│   ├── project.godot
│   ├── icon.svg
│   ├── addons/                    # symlink → ../engine/addons/
│   ├── scenes/
│   │   ├── main.tscn
│   │   ├── levels/
│   │   │   ├── distrito_inferior.tscn
│   │   │   └── selve_sombria_01.tscn
│   │   ├── characters/
│   │   │   ├── gus.tscn
│   │   │   └── npcs/
│   │   ├── ui/
│   │   ├── combat/
│   │   ├── puzzles/
│   │   └── cutscenes/
│   ├── scripts/                   # game-specific GDScript
│   │   ├── globals/
│   │   ├── characters/
│   │   ├── combat/                # subclasses dos hooks da engine
│   │   └── puzzles/
│   ├── data/                      # .tres / .json de balance, cartas, combos
│   │   ├── cards/
│   │   ├── enemies/
│   │   ├── items/
│   │   └── runic_combos.tres
│   ├── dialogue/                  # arquivos .dialogue source
│   ├── localization/
│   │   └── strings.csv
│   ├── shaders/
│   ├── art/                       # arte importada (final), references em /assets/
│   ├── audio/                     # áudio importado (final)
│   ├── fonts/
│   ├── tests/                     # GUT tests do game
│   └── export_presets.cfg
│
├── assets/                        # sources brutos (FORA do project.godot)
│   ├── models/                    # .blend, .fbx originais
│   ├── sprites/                   # .aseprite, .psd
│   ├── textures/                  # PSD/Krita sources
│   ├── music/                     # .wav/FLAC masters, projetos DAW
│   ├── sfx/                       # masters
│   └── references/                # mood boards
│
├── docs/
│   ├── tech/
│   │   ├── architecture.md        # este arquivo
│   │   ├── engine-modules.md
│   │   └── build.md
│   ├── design/
│   ├── narrative/
│   ├── art/
│   ├── audio/
│   └── production/
│
├── build/                         # outputs (gitignored)
│   ├── linux/
│   │   └── v0.1.0/
│   └── windows/
│       └── v0.1.0/
│
├── tools/                         # scripts CLI auxiliares
│   ├── export_linux.sh
│   ├── export_windows.sh
│   └── version_bump.sh
│
└── .forgejo/
    └── workflows/
        └── ci.yml
```

`.gitignore` deve cobrir: `build/`, `.import/`, `*.translation`, `export_presets.cfg.user`, `*.pck` em build, `user://` (não é commitado por natureza), `assets/` opcionalmente em LFS ou em repo separado.

**Decisão sobre `/assets/`:** sources brutos podem ficar em **git-lfs** OU em **repo separado** (`gusworld-assets`). G1 recomendação: **git-lfs no mesmo repo** até cruzar ~5GB. Acima disso, separar.

---

## 10. Anti-objetivos G1

Lista do que **NÃO** se faz em G1. Cada um destes mata escopo solo de 6-12 meses.

- **Custom engine.** Godot resolve. Ponto.
- **Multiplayer netcode.** Single-player puro. Sem `MultiplayerSpawner`, sem `RPC`, sem rollback.
- **Mod support / Lua scripting / runtime plugins.** Carga de design API + sandboxing + suporte de comunidade. Não em G1.
- **In-game level editor.** Editor do Godot basta.
- **Cutscene engine complexa.** Timeline + AnimationPlayer cobre G1.
- **Photorealistic 3D / PBR pesado.** Estética estilizada ciber-gótica + Selve cabe em low-poly + post-processing + emissive. Não chasing fidelidade.
- **Voice acting.** Texto + bleeps estilo Undertale/Banjo. Voz é pós-1.0 (se).
- **Console ports (Switch/PS/Xbox).** Devkit + cert + bizdev: fora de G1 solo.
- **Mobile.** Touch UI + perf + storefront. Não.
- **Achievements/Steam integration na primeira build.** Steamworks API entra **depois** de demo jogável.
- **Save em cloud / cross-save.** Local file only.
- **Localização além de pt-br no v1.** Infra D1, conteúdo pt-br only. en-US é stretch pós-demo.
- **Anti-cheat / anti-tamper.** Single-player. Não.
- **Procedural generation de levels.** Levels handcrafted. Procedural é tentação solo-killer.
- **Day-one DLC / season pass.** Foco no jogo base.
- **Marketplace de cartas / economia online.** Single-player.
- **Sistema de mods workshop.** Pós-1.0 se tração.
- **Sistema de partículas custom / GPU compute.** `GPUParticles3D` nativo basta.
- **Refactor da engine sem teste.** Cada módulo da engine precisa pelo menos 1 GUT test antes de virar dependência de outro módulo.

---

## 11. Trade-offs aceitos explícitos

- **GDScript over C#:** sacrifica perf marginal e portabilidade fora-de-Godot pra ganhar 30-50% velocidade de iteração solo. Aceito.
- **JSON save over binary:** sacrifica tamanho de arquivo e dificuldade-de-tampering. Ganha debuggabilidade. Aceito (single-player).
- **Engine como submodule com addons internos** vs publicar em Asset Library: sacrifica "discoverabilidade". Ganha velocidade de iteração + zero burocracia de release. Aceito até pós-1.0.
- **Sem code signing Windows G1:** SmartScreen warning na primeira execução. Aceito (jogadores indie estão acostumados; cert custa mais que o demo provavelmente fatura).
- **Sem cloud save:** se o disco do jogador morre, save perde. Aceito (mitigado por slot múltiplo + autosave + quicksave).
- **Symlink `game/addons → engine/addons`:** Windows requer dev mode ou admin pra symlink. Aceito (dev primário é Linux; build Windows é só export, não exige symlink em runtime do jogo final).

---

## 12. Próximos passos sugeridos

1. Criar repo `gusworld-engine` no Forgejo, init com README + `addons/` vazio.
2. Adicionar submodule no `gusworld` apontando pra ele.
3. Primeiro módulo a implementar: `orbital_camera` (validação rápida da câmera 3/4).
4. Segundo: `save_system` com schema v1 + 1 migrator dummy (validar pipeline de migração antes de ter saves reais).
5. Terceiro: `turn_combat` esqueleto com 2 personagens dummy.
6. Setup CI Forgejo Actions com job de export Linux (Windows depois).

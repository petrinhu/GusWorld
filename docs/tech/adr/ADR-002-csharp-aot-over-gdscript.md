# ADR-002: Mudança de GDScript para C# .NET AOT como linguagem primária

| | |
|---|---|
| **Status** | Accepted |
| **Date** | 2026-05-19 |
| **Decisor** | petrinhu (criador supremo) |
| **Reversibilidade** | One-way door massivo. Reverter exige rewrite paralelo (~2-4 semanas). |
| **Substitui** | CLAUDE.md decisão "Engine: Godot 4 + GDScript. C# rejeitado pra G1" |

## Contexto

ADR-001 (pivot Fase 1 para Fase 2) canonizou Godot 4 + GDScript como stack default. Decisão original assumiu que **iteração rápida solo > performance bruta**, dado escopo turn-based + área pequena.

Em 2026-05-19, durante setup F2-S.11 + F2-E.2, criador supremo levantou questão de **máxima performance em máquinas fracas** (Steam Deck, GTX 1050, laptops 2017+). Reabriu decisão GDScript vs alternativas compiladas.

Análise pragmática das opções:

| Opção | Perf (vs GDScript) | Trade-off |
|---|---|---|
| GDScript puro | 1× (baseline) | Interpretado bytecode VM. Iteração rápida (hot-reload no editor). |
| GDExtension C++ módulo-a-módulo | ~10-20× hot paths | Build cross-platform overhead. Sem hot-reload. Solo: lento. |
| C# .NET AOT | ~3-5× hot paths | Compilação nativa AOT. Iteração via dotnet build (alguns segundos). Reload via Godot detect. |
| Rewrite engine Unity IL2CPP | ~3-5× hot paths | One-way door catastrófico. Decisão Godot 4 ratificada. |

Critério decisor:
- Performance bruta superior a GDScript em 100% do código (não só hot paths)
- Compilação nativa garantida (não interpretado runtime)
- Iteração ainda aceitável solo (build dotnet ~5-15s)
- Ecosystem Godot 4 maduro pra C# desde 4.0 (atual 4.6.1 com .NET 8 LTS suportado)
- Tooling profissional disponível (Visual Studio Code + Omnisharp, JetBrains Rider, dotnet CLI)

## Decisão

**Migrar de GDScript para C# .NET AOT como linguagem primária canon.**

### Diretrizes

1. **Todo código gameplay + engine MUST ser C# .NET 8** (LTS atual, suporte AOT estável Godot 4.4+).
2. **GDScript MAY ser usado em** tooling auxiliar não-críticos (ex: `validate_autoloads.gd`, scripts editor-only).
3. **AOT (Ahead-of-Time) compilation MUST estar habilitado** em export builds release (não JIT runtime).
4. **Build target:** .NET 8 LTS (suportado até 2026-11) + Godot 4.4+ Mono.
5. **Resources e `.tres` files permanecem** como cobertos por Godot core (não-código).
6. **MD files** (translations, lore, docs) permanecem MD.
7. **Hot paths** (turn resolve, save serialization, AI behavior) MUST atingir < 50% de frame budget em GTX 1050 target.

### Implicações imediatas

#### Migração de código existente (F2-S.MIG, sessão dedicada futura)

GDScript existente a reescrever em C#:

| Arquivo GDScript | Arquivo C# alvo |
|---|---|
| `engine/event_bus/event_bus.gd` | `engine/event_bus/EventBus.cs` |
| `engine/event_bus/test_event_bus.gd` | `engine/event_bus/EventBusTests.cs` |
| `engine/localization/localization.gd` | `engine/localization/Localization.cs` |
| `engine/localization/md_translation_loader.gd` | `engine/localization/MdTranslationLoader.cs` |
| `game/tools/validate_autoloads.gd` | manter GDScript (tool standalone) |

#### Estrutura .csproj

Solo G1 G2 G3:
```
gusworld/
├── game/
│   ├── project.godot       (ativar Mono + .NET 8)
│   ├── GusWorld.csproj     (csproj raiz do game project)
│   └── GusWorld.sln        (solution include csproj)
├── engine/                 (módulos engine; cada um pode ter sub-csproj OU agrupado)
│   └── *.cs                (mantido em diretórios por módulo)
└── *.cs.uid                (Godot rastreia)
```

Detalhe definitivo de .sln + .csproj structure em revisão de architecture.md.

#### CI + build

- Forgejo Actions runner MUST instalar .NET 8 SDK + Mono runtime Godot.
- Build pipeline: `dotnet restore` → `dotnet build -c Release` → `godot --export-release "Linux/X11"`.
- AOT compilation step durante export (configurar export_presets.cfg).
- CI image base: `mcr.microsoft.com/dotnet/sdk:8.0` + Godot Mono binary.

#### Custo estimado migração

- Rewrite event_bus + localization + tests: ~4-8h.
- Setup .csproj + .sln + dotnet workflow: ~2-4h.
- CI pipeline atualização: ~2-4h.
- Documentação update (CONTRACT, TESTES, architecture, engine-modules, build, README, CLAUDE): ~4-8h.
- Aprendizado dotnet + Godot C# (se necessário): ~variável.

**Total estimado: 12-24h trabalho focado solo.**

#### Sem hot-reload nativo igual GDScript

Iteração em C#:
1. Editor Godot atualiza referência via auto-recompile (Godot 4 Mono integration).
2. Mudança trivial em método: ~3-8s recompile + reload.
3. Mudança que toca scene script signature: full reload editor possivelmente.

Comparado a GDScript reload instantâneo, isso é overhead, mas aceitável.

## Alternativas consideradas

### A. Manter GDScript (status quo)

**Pros:**
- Iteração mais rápida.
- Stack canonizado funcionando (F2-E.2 + F2-S.11 done).
- Comunidade Godot indie usa massivamente.
- Performance suficiente pra escopo turn-based GusWorld.

**Cons:**
- Performance ~3-5× pior que C# AOT em CPU-bound code.
- Interpretado, não satisfaz critério "compilado nativo" do criador supremo.

**Decidida:** REJEITADA. Critério "máxima performance em máquinas fracas" prevalece sobre conforto de iteração.

### B. GDExtension C++ módulo-a-módulo

**Pros:**
- Performance máxima possível (10-20× GDScript em hot paths).

**Cons:**
- Build cross-platform pesado (Windows + Linux + Steam Deck).
- Iteração muito lenta (recompile + reimport addon).
- Solo prohibitive: cada mudança = pipeline complexo.
- Sem hot-reload prático.

**Decidida:** REJEITADA. Trade-off iteração vs perf desfavorável solo.

### C. C# .NET AOT (escolhida)

**Pros:**
- Compilação nativa AOT (atende critério "compilado").
- Performance 3-5× GDScript.
- Iteração aceitável (~5-15s rebuild via dotnet).
- Tooling profissional maduro.
- Ecosystem Godot 4 estável (4.0+ Mono integration; AOT estável 4.4+).
- Suporte LTS .NET 8 até 2026-11.

**Cons:**
- Aprendizado C# + dotnet workflow.
- CI pipeline mais pesado (.NET SDK ~600MB).
- Rewrite código atual (~12-24h custo).
- Sem hot-reload instantâneo GDScript.

**Decidida:** ACEITA.

### D. Rewrite engine (Unity / Unreal)

**Pros:**
- IL2CPP / C++ nativo.

**Cons:**
- One-way door catastrófico.
- ADR-001 + decisão Godot 4 ratificada.
- Custo proibitivo (~6+ meses migração).

**Decidida:** REJEITADA (não considerada formalmente).

## Consequências

### Positivas

- Stack profissional padrão indústria (C# .NET 8 maduro).
- Performance superior em todas categorias de código, não só hot paths.
- Tooling pro (Rider, VS Code Omnisharp, dotnet CLI).
- Static typing forte previne classes de bugs comuns em GDScript.
- AOT garante deploy sem dependência runtime JIT.
- Ecosystem NuGet rich (libraries data, math, etc se necessário).

### Negativas

- Iteração ~10× mais lenta que GDScript hot-reload.
- Curva aprendizado se solo dev não dominava C#.
- CI overhead (.NET SDK pesado).
- Lock-in .NET 8 LTS (próximo LTS .NET 10 em 2027-11).
- GDScript ecosystem Godot indie (GUT addon, DialogueManager addon) menos integrado.

### Mitigações

- Pair-programming com Claude pra acelerar aprendizado dotnet workflow.
- Build incremental dotnet pra reduzir rebuild time.
- Cache .NET SDK em CI.
- GDScript permitido pra editor-only tools (validate_autoloads, build scripts).
- Templates `dotnet new` agilizam scaffolding.

## Ações imediatas

1. ✅ Este ADR criado e canonizado.
2. ⏳ Atualizar `CLAUDE.md` decisões fechadas (GDScript → C# .NET AOT).
3. ⏳ Atualizar `CONTRACT.md` §2 scopes + §4 DoD.
4. ⏳ Atualizar `TESTES.md` (GUT GDScript → NUnit/xUnit C#).
5. ⏳ Atualizar `docs/tech/architecture.md` (tooling, build flow, .csproj structure).
6. ⏳ Atualizar `docs/tech/engine-modules.md` (C# classes em vez de GDScript).
7. ⏳ Atualizar `docs/tech/build.md` (dotnet restore + build + AOT export).
8. ⏳ Atualizar `README.md` (tech stack C#).
9. ⏳ Atualizar `CHANGELOG.md` Unreleased.
10. ⏳ Criar task F2-S.MIG (sessão dedicada migração).
11. ⏳ Fetch + memorizar `reference_godot_csharp.md` (Godot C# docs chave).

## Cross-refs

- `ADR-001-pivot-lore-to-engine.md` (pivot precedente).
- `CONTRACT.md` §1 princípios (single-dev sustainability + anti over-engineering).
- `CONTRACT.md` §5 performance budget (GTX 1050 target reforça motivação).
- `docs/tech/architecture.md` (revisão obrigatória).
- Godot docs: <https://docs.godotengine.org/en/stable/tutorials/scripting/c_sharp/index.html>

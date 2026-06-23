# Build Pipeline, GusWorld G1

> **STATUS: SUPERADO (2026-06-23).** Este documento descreve o build do stack **Godot 4 Mono + C# .NET 8 AOT** (`dotnet restore/build/format/test`, `export_presets.cfg`, `godot --headless --export-release`, export templates, AOT), **aposentado** pelo pivot para C++/Qt6 e depois pelo [ADR-008](adr/ADR-008-repivot-qt-to-sdl3.md) (engine própria C++20 + SDL3). **NÃO é mais o pipeline canônico.** O build atual usa **CMake + Ninja + CMakePresets** (`cmake --preset` + `cmake --build` + `ctest`), com SDL3 + RmlUi via FetchContent (pin de versão) e Catch2 nos testes; smoke da plataforma headless com `SDL_VIDEODRIVER=dummy`. Comandos de referência atuais estão no [README](../../README.md) (seção Build / Run) e o design da engine em [`docs/tech/pivot/engine-design.md`](pivot/engine-design.md). Este arquivo fica como **registro histórico de leitura** até o decommission no marco M8; não editar como se fosse vigente. Re-derivar o pipeline (CMake local + CI Forgejo) para o stack atual é decisão de produção do criador.
>
> **Escopo (histórico):** build local + CI Forgejo Actions. Linux primário (dev Fedora 44), Windows via export Godot cross-platform. C# .NET 8 AOT.

---

## §1. Pré-requisitos

### §1.1. SDK + tooling obrigatórios

| Ferramenta | Versão | Install Fedora 44 |
|---|---|---|
| **Godot 4.6+ Mono** | 4.6.1 ✅ | Download oficial + extract `~/.local/bin/godot-mono/` (Flatpak versão regular NÃO inclui Mono; baixar Mono específico) |
| **.NET SDK 8** | 8.0.x LTS | `sudo dnf install dotnet-sdk-8.0` |
| **git** | 2.40+ | `sudo dnf install git` |
| **dotnet format** | included | `dotnet tool install -g dotnet-format` (se não built-in) |
| **dotnet-coverage** | optional | `dotnet tool install -g dotnet-coverage` |
| **reportgenerator** | optional | `dotnet tool install -g dotnet-reportgenerator-globaltool` |
| **xUnit** | latest | Via NuGet em `tests/*.csproj` |
| **Roslyn analyzers** | latest-recommended | Via `<AnalysisLevel>` em `.csproj` |

### §1.2. Validação setup

```bash
godot --version              # 4.6.1.stable.mono ou superior
dotnet --version             # 8.0.x
dotnet workload list         # confirmar nenhum workload faltando
```

### §1.3. Godot Mono export templates

Templates pra export multi-plataforma:

```bash
# Via Godot editor:
# Project > Export... > Manage Export Templates > Download

# OU via CLI (Linux):
godot --headless --download-export-templates
```

Templates ficam em `~/.local/share/godot/export_templates/4.6.x.stable.mono/`.

---

## §2. Export presets (`game/export_presets.cfg`)

Configuração canônica pós-ADR-002 (AOT habilitado):

```ini
[preset.0]
name="Linux/X11"
platform="Linux/X11"
runnable=true
export_filter="all_resources"
export_path="../build/linux/gusworld.x86_64"
custom_features=""
export_files=PoolStringArray()

[preset.0.options]
custom_template/debug=""
custom_template/release=""
dotnet/include_scripts_content=false
dotnet/include_debug_symbols=false
dotnet/embed_build_outputs=true
dotnet/use_aot=true             ; AOT obrigatório (ADR-002)

[preset.1]
name="Windows Desktop"
platform="Windows Desktop"
runnable=true
export_path="../build/windows/gusworld.exe"

[preset.1.options]
dotnet/include_scripts_content=false
dotnet/include_debug_symbols=false
dotnet/embed_build_outputs=true
dotnet/use_aot=true             ; AOT obrigatório
```

Detalhes AOT em §7.

---

## §3. Comandos concretos (build local)

### §3.1. Linux (dev box Fedora)

Fluxo canon ADR-002 batch 8:

```bash
# 1. Restore dependências NuGet
cd /home/petrus/IDrive/Documentos/projetos_claudebrain/Projects/gusworld
dotnet restore

# 2. Build C# Release com warnings as errors
dotnet build -c Release /warnaserror

# 3. Validar lint + format
dotnet format --verify-no-changes

# 4. Rodar testes
dotnet test

# 5. Import Godot (primeira vez ou após mudança .tscn/.tres)
godot --headless --path ./game --import

# 6. Export Linux release
godot --headless --path ./game --export-release "Linux/X11" ../build/linux/gusworld.x86_64
```

### §3.2. Windows (cross-compile a partir Linux)

```bash
# Pré-requisito: rcedit ou wine + rcedit pra ícone (opcional G1)
# Sem signing G1.

godot --headless --path ./game --export-release "Windows Desktop" ../build/windows/gusworld.exe
```

Output: `build/windows/gusworld.exe` + DLLs runtime .NET 8.

### §3.3. Wrappers shell (`scripts/`)

Wrappers ergonômicos em `scripts/` na raiz (canon pós-F2-CI.1, 2026-05-30). `game/tools/` é reservado para ferramentas headless Godot (ValidateAutoloads, TestCombatIntegration).

#### `scripts/build_linux.sh`
```bash
#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."

VERSION=$(cat game/VERSION)
echo "Building GusWorld $VERSION (Linux)"

dotnet restore
dotnet format --verify-no-changes
dotnet build -c Release /warnaserror
dotnet test --no-build -c Release

godot --headless --path ./game --import
mkdir -p build/linux
godot --headless --path ./game --export-release "Linux/X11" "$(pwd)/build/linux/gusworld.x86_64"

echo "Build done: build/linux/gusworld.x86_64"
```

#### `scripts/build_windows.sh`
```bash
#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."

VERSION=$(cat game/VERSION)
echo "Building GusWorld $VERSION (Windows)"

mkdir -p build/windows
godot --headless --path ./game --export-release "Windows Desktop" "$(pwd)/build/windows/gusworld.exe"

echo "Build done: build/windows/gusworld.exe"
```

#### `scripts/version_bump.sh`
```bash
#!/usr/bin/env bash
# Usage: scripts/version_bump.sh 0.1.0
set -euo pipefail
NEW_VERSION="$1"
echo "$NEW_VERSION" > game/VERSION
git add game/VERSION
echo "Bumped to $NEW_VERSION (not committed)"
```

Wrappers usam `set -euo pipefail` (disciplina shell). Os arquivos shell são gerados quando F2-CI.1 for implementado.

---

## §4. CI, Forgejo Actions (esqueleto)

### §4.1. Image base

**Image canon (ADR-002 batch 4):** `mcr.microsoft.com/dotnet/sdk:8.0` + Godot Mono binary cached entre runs.

### §4.2. Workflow file `.forgejo/workflows/ci.yml`

```yaml
name: CI
on:
  push:
    branches: [main]
  pull_request:
    branches: [main]

env:
  GODOT_VERSION: "4.6.1"
  DOTNET_VERSION: "8.0.x"

jobs:
  lint:
    runs-on: docker
    container:
      image: mcr.microsoft.com/dotnet/sdk:8.0
    steps:
      - uses: actions/checkout@v4
        with:
          submodules: recursive
      - name: dotnet format check
        run: dotnet format --verify-no-changes
      - name: dotnet build (warnings as errors)
        run: dotnet build -c Release /warnaserror

  test:
    runs-on: docker
    container:
      image: mcr.microsoft.com/dotnet/sdk:8.0
    needs: lint
    steps:
      - uses: actions/checkout@v4
        with:
          submodules: recursive
      - name: dotnet test
        run: dotnet test --no-build -c Release /p:CollectCoverage=true
      - name: Upload coverage
        uses: actions/upload-artifact@v3
        with:
          name: coverage
          path: '**/coverage.opencover.xml'
          retention-days: 30

  build-linux:
    runs-on: docker
    container:
      image: mcr.microsoft.com/dotnet/sdk:8.0
    needs: [lint, test]
    steps:
      - uses: actions/checkout@v4
        with:
          submodules: recursive
      - name: Cache Godot binary
        uses: actions/cache@v4
        with:
          path: ~/godot
          key: godot-${{ env.GODOT_VERSION }}-mono-linux
      - name: Install Godot Mono (cache miss)
        run: |
          if [ ! -f ~/godot/godot ]; then
            mkdir -p ~/godot
            wget -q "https://github.com/godotengine/godot/releases/download/${{ env.GODOT_VERSION }}-stable/Godot_v${{ env.GODOT_VERSION }}-stable_mono_linux_x86_64.zip" -O /tmp/godot.zip
            unzip -q /tmp/godot.zip -d ~/godot
            mv ~/godot/Godot_v*/* ~/godot/
            chmod +x ~/godot/Godot_v*_mono_linux.x86_64
          fi
      - name: Build Linux
        run: |
          dotnet restore
          dotnet build -c Release
          ~/godot/Godot_v*_mono_linux.x86_64 --headless --path ./game --import
          mkdir -p build/linux
          ~/godot/Godot_v*_mono_linux.x86_64 --headless --path ./game --export-release "Linux/X11" "$(pwd)/build/linux/gusworld.x86_64"
      - name: Upload Linux artifact
        uses: actions/upload-artifact@v3
        with:
          name: gusworld-linux
          path: build/linux/
          retention-days: 30

  build-windows:
    runs-on: docker
    container:
      image: mcr.microsoft.com/dotnet/sdk:8.0
    needs: [lint, test]
    steps:
      # Similar a build-linux mas com export "Windows Desktop"
      # Detalhes idem
      - run: echo "Windows export step here"

  aot_check:
    runs-on: docker
    container:
      image: mcr.microsoft.com/dotnet/sdk:8.0
    needs: lint
    steps:
      - uses: actions/checkout@v4
        with:
          submodules: recursive
      - name: dotnet publish AOT validation
        run: |
          dotnet publish -c Release --self-contained -r linux-x64 /p:PublishAot=true
          # Exit non-zero se trim/AOT warnings encontrados
```

### §4.3. Release job (manual trigger)

```yaml
release:
  runs-on: docker
  if: startsWith(github.ref, 'refs/tags/v')
  needs: [build-linux, build-windows]
  steps:
    - name: Download artifacts
      uses: actions/download-artifact@v3
    - name: Create release archives
      run: |
        tar -czf gusworld-linux-$(cat game/VERSION).tar.gz -C build/linux .
        cd build/windows && zip -r ../../gusworld-windows-$(cat game/VERSION).zip .
    - name: Create Forgejo release
      run: |
        # TODO: usar API Forgejo /api/v1/repos/{owner}/{repo}/releases
        echo "Forgejo release API call here"
```

---

## §5. Estrutura `/build/`

```
build/
├── linux/
│   ├── gusworld.x86_64       (binário export)
│   ├── data_GusWorld_linux.x86_64/  (assets packed)
│   └── *.so                  (libs Godot + .NET runtime AOT)
├── windows/
│   ├── gusworld.exe          (binário export)
│   ├── data_GusWorld_windows/
│   └── *.dll                 (libs)
├── v0.1.0/                   (snapshots versionados)
│   ├── linux/
│   ├── windows/
│   └── CHANGELOG-v0.1.0.md
└── .gitignore                (build/ gitignored exceto README)
```

---

## §6. Checklist pré-release manual (G1)

10 itens canon antes de bumpar `v0.X.Y`:

- [ ] `dotnet format --verify-no-changes` exit 0.
- [ ] `dotnet build /warnaserror` exit 0.
- [ ] `dotnet test` 100% pass.
- [ ] `dotnet publish -c Release --self-contained` zero trim/AOT warnings.
- [ ] `godot --headless --path ./game --import` zero errors.
- [ ] Export Linux + Windows roda em VM clean (smoke test).
- [ ] Save format compat: load save da versão anterior funciona (se aplicável; pre-v1.0.0 MAY invalidar).
- [ ] CHANGELOG.md atualizado com entry pra nova versão.
- [ ] `game/VERSION` bumpado.
- [ ] Tag git criada `v0.X.Y` + commit + push origin tag.

---

## §7. AOT compilation, detalhes canon (ADR-002)

### §7.1. Configuração export_presets.cfg

AOT habilitado obrigatório em release + dev (ADR-002 batch 1):

```ini
[preset.0.options]
dotnet/use_aot=true
dotnet/embed_build_outputs=true
dotnet/include_debug_symbols=false  ; release
```

Em dev preset (futuro):
```ini
[preset.2]
name="Linux Dev (AOT)"
...
[preset.2.options]
dotnet/use_aot=true
dotnet/include_debug_symbols=true   ; dev mantém symbols pra debug
```

### §7.2. AOT-compatibility rules

Cross-ref CONTRACT.md §4 DoD + architecture.md §12.2.

- ❌ Zero `dynamic` keyword.
- ❌ Reflection runtime não-trimming-friendly.
- ✅ Source generators (System.Text.Json, MessagePack source-gen).
- ✅ `[DynamicallyAccessedMembers]` quando reflection necessária.
- ✅ AOT-friendly libraries (verify before adicionar NuGet package).

### §7.3. Validation step CI

```bash
dotnet publish -c Release --self-contained -r linux-x64 /p:PublishAot=true \
  /warnaserror /p:TrimmerSingleWarn=false
# Exit non-zero se algum trim/AOT warning emitido
```

### §7.4. Tamanho binário esperado

| Plataforma | Tamanho esperado |
|---|---|
| Linux x86_64 | ~80-120MB (binário + .NET AOT runtime + assets) |
| Windows x64 | ~70-100MB |

Comparação: GDScript build ~40-60MB. Trade-off aceito por ADR-002.

### §7.5. Limitações AOT conhecidas

- Reflection runtime limitada (use `[DynamicallyAccessedMembers]`).
- `dynamic` keyword não funciona.
- Type/method generation runtime restrita.
- Plugins/mods dinâmicos (futuro) exigem workarounds.

---

## §8. Cross-refs

- `docs/tech/architecture.md` (§7 CI/CD overview).
- `docs/tech/engine-modules.md` (módulos detalhados).
- `docs/tech/adr/ADR-002-csharp-aot-over-gdscript.md` (canon stack).
- `CONTRACT.md` §3 branching + §4 DoD + §5 perf budget.
- `TESTES.md` T+A sections.
- Memory `reference_godot_csharp.md`.

---

**Última revisão:** 2026-05-19. Pós-ADR-002 canonization. Revisão obrigatória em F2-M.1 (CI pipeline real validado).

# GusWorld — Build Pipeline

> Build local + CI Forgejo. Linux primário (dev box), Windows via export Godot cross-platform.

---

## 1. Pré-requisitos

- Godot 4.3+ (stable). Binário headless em `$PATH` como `godot` (ou ajustar variável).
- Export templates instalados:
  ```bash
  godot --headless --download-export-templates
  ```
  ou colocados manualmente em `~/.local/share/godot/export_templates/4.3.stable/`.
- Submodule `engine/` inicializado:
  ```bash
  git submodule update --init --recursive
  ```
- Symlink `game/addons/` (uma vez, manual):
  ```bash
  ln -s ../engine/addons /home/petrus/IDrive/Documentos/projetos_claudebrain/Projects/gusworld/game/addons
  ```

---

## 2. Export presets (`game/export_presets.cfg`)

Dois presets versionados:

| Preset | Plataforma | Output |
|---|---|---|
| `Linux/X11` | linux/x86_64 | `build/linux/v<X.Y.Z>/gusworld.x86_64` + `gusworld.pck` |
| `Windows Desktop` | windows/x86_64 | `build/windows/v<X.Y.Z>/gusworld.exe` + `gusworld.pck` |

Encryption key: **vazio** em G1.
Embed PCK: **não** (PCK separado facilita patch e debug).

---

## 3. Comandos concretos

### 3.1. Linux

```bash
# variáveis
PROJECT_ROOT=/home/petrus/IDrive/Documentos/projetos_claudebrain/Projects/gusworld
VERSION=$(cat "$PROJECT_ROOT/game/VERSION")   # ex: 0.1.0
OUT_LINUX="$PROJECT_ROOT/build/linux/v${VERSION}"

mkdir -p "$OUT_LINUX"

godot --headless \
      --path "$PROJECT_ROOT/game" \
      --export-release "Linux/X11" \
      "$OUT_LINUX/gusworld.x86_64"

# empacotar
cd "$OUT_LINUX"
tar czf "gusworld-linux-x86_64-v${VERSION}.tar.gz" gusworld.x86_64 gusworld.pck
sha256sum "gusworld-linux-x86_64-v${VERSION}.tar.gz" > SHA256SUMS
```

### 3.2. Windows (cross-compile a partir de Linux)

```bash
OUT_WIN="$PROJECT_ROOT/build/windows/v${VERSION}"
mkdir -p "$OUT_WIN"

godot --headless \
      --path "$PROJECT_ROOT/game" \
      --export-release "Windows Desktop" \
      "$OUT_WIN/gusworld.exe"

cd "$OUT_WIN"
zip "gusworld-windows-x86_64-v${VERSION}.zip" gusworld.exe gusworld.pck
sha256sum "gusworld-windows-x86_64-v${VERSION}.zip" >> SHA256SUMS
```

**Sem signing em G1.** Windows Defender SmartScreen vai exibir warning na primeira execução — aceito.

### 3.3. Wrappers shell (`tools/`)

`tools/export_linux.sh`:
```bash
#!/usr/bin/env bash
set -euo pipefail
PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
VERSION="$(cat "$PROJECT_ROOT/game/VERSION")"
OUT="$PROJECT_ROOT/build/linux/v${VERSION}"
mkdir -p "$OUT"
godot --headless --path "$PROJECT_ROOT/game" \
      --export-release "Linux/X11" "$OUT/gusworld.x86_64"
( cd "$OUT" && tar czf "gusworld-linux-x86_64-v${VERSION}.tar.gz" gusworld.x86_64 gusworld.pck && sha256sum "gusworld-linux-x86_64-v${VERSION}.tar.gz" > SHA256SUMS )
echo "OK: $OUT"
```

`tools/export_windows.sh` (análogo).

`tools/version_bump.sh`:
```bash
#!/usr/bin/env bash
set -euo pipefail
PART="${1:-patch}"   # major|minor|patch
PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
FILE="$PROJECT_ROOT/game/VERSION"
IFS=. read -r MA MI PA < "$FILE"
case "$PART" in
  major) MA=$((MA+1)); MI=0; PA=0 ;;
  minor) MI=$((MI+1)); PA=0 ;;
  patch) PA=$((PA+1)) ;;
esac
echo "${MA}.${MI}.${PA}" > "$FILE"
cat "$FILE"
```

---

## 4. CI — Forgejo Actions (esqueleto)

`/.forgejo/workflows/ci.yml`:

```yaml
name: ci

on:
  push:
    branches: [main, dev]
  pull_request:
    branches: [main, dev]
  workflow_dispatch:
    inputs:
      release:
        description: "Build release artifacts"
        required: false
        default: "false"

env:
  GODOT_VERSION: "4.3.stable"

jobs:

  lint:
    runs-on: docker
    container: barichello/godot-ci:4.3
    steps:
      - uses: actions/checkout@v4
        with:
          submodules: recursive
      - name: Setup gdtoolkit
        run: pip install --no-cache-dir gdtoolkit==4.*
      - name: Format check
        run: gdformat --check game/scripts engine/addons
      - name: Lint
        run: gdlint game/scripts engine/addons

  test:
    runs-on: docker
    container: barichello/godot-ci:4.3
    needs: lint
    steps:
      - uses: actions/checkout@v4
        with:
          submodules: recursive
      - name: Setup addons symlink
        run: ln -s ../engine/addons game/addons
      - name: Import assets headless
        run: godot --headless --path game --import || true
      - name: GUT tests
        run: godot --headless --path game -s addons/gut/gut_cmdln.gd -gdir=res://tests -gexit

  build-linux:
    runs-on: docker
    container: barichello/godot-ci:4.3
    needs: test
    steps:
      - uses: actions/checkout@v4
        with:
          submodules: recursive
      - name: Setup addons symlink
        run: ln -s ../engine/addons game/addons
      - name: Import
        run: godot --headless --path game --import || true
      - name: Export Linux
        run: |
          VERSION="$(cat game/VERSION)"
          mkdir -p build/linux/v${VERSION}
          godot --headless --path game \
                --export-release "Linux/X11" \
                build/linux/v${VERSION}/gusworld.x86_64
          ( cd build/linux/v${VERSION} && \
            tar czf gusworld-linux-x86_64-v${VERSION}.tar.gz gusworld.x86_64 gusworld.pck && \
            sha256sum gusworld-linux-x86_64-v${VERSION}.tar.gz > SHA256SUMS )
      - uses: actions/upload-artifact@v3
        with:
          name: gusworld-linux
          path: build/linux/

  build-windows:
    runs-on: docker
    container: barichello/godot-ci:4.3
    needs: test
    steps:
      - uses: actions/checkout@v4
        with:
          submodules: recursive
      - name: Setup addons symlink
        run: ln -s ../engine/addons game/addons
      - name: Import
        run: godot --headless --path game --import || true
      - name: Export Windows
        run: |
          VERSION="$(cat game/VERSION)"
          mkdir -p build/windows/v${VERSION}
          godot --headless --path game \
                --export-release "Windows Desktop" \
                build/windows/v${VERSION}/gusworld.exe
          ( cd build/windows/v${VERSION} && \
            zip gusworld-windows-x86_64-v${VERSION}.zip gusworld.exe gusworld.pck && \
            sha256sum gusworld-windows-x86_64-v${VERSION}.zip >> SHA256SUMS )
      - uses: actions/upload-artifact@v3
        with:
          name: gusworld-windows
          path: build/windows/

  release:
    runs-on: docker
    needs: [build-linux, build-windows]
    if: startsWith(github.ref, 'refs/tags/v')
    steps:
      - uses: actions/checkout@v4
      - name: Download artifacts
        uses: actions/download-artifact@v3
      - name: Create Forgejo release
        run: |
          echo "TODO: usar API Forgejo (forgejo-cli ou curl) pra criar release"
          echo "Tag: ${GITHUB_REF#refs/tags/}"
          # curl -X POST -H "Authorization: token $FORGEJO_TOKEN" \
          #   "$FORGEJO_API/repos/$REPO/releases" -d @payload.json
```

**Notas G1:**
- Imagem `barichello/godot-ci:4.3` é referência comunidade — substituir por imagem própria mantida se Forgejo runner tiver registry interno.
- Jobs `build-*` rodam em paralelo após `test`. Pull único de checkout.
- Release só dispara em tag `v*`. Manual via `workflow_dispatch` opcional.
- Cache de templates de export: adicionar step `actions/cache` apontando pra `~/.local/share/godot/export_templates/` em ronda 2 (otimização, não bloqueador).

---

## 5. Estrutura `/build/`

```
build/
├── linux/
│   ├── v0.1.0/
│   │   ├── gusworld.x86_64
│   │   ├── gusworld.pck
│   │   ├── gusworld-linux-x86_64-v0.1.0.tar.gz
│   │   └── SHA256SUMS
│   └── v0.1.1/
│       └── ...
└── windows/
    ├── v0.1.0/
    │   ├── gusworld.exe
    │   ├── gusworld.pck
    │   ├── gusworld-windows-x86_64-v0.1.0.zip
    │   └── SHA256SUMS
    └── v0.1.1/
        └── ...
```

- Todo conteúdo de `/build/` é **gitignored**.
- Versão lida de `game/VERSION` (single source of truth) — incrementada via `tools/version_bump.sh`.
- Manter pelo menos 3 versões anteriores localmente; mais antigas → arquivar/deletar.
- SHA256SUMS por release pra verificação manual.

---

## 6. Checklist pré-release manual (G1)

Antes de tag `v0.X.Y` em `main`:

- [ ] `game/VERSION` bumped e commitado.
- [ ] CHANGELOG.md atualizado (Keep a Changelog format).
- [ ] `gdformat --check` + `gdlint` limpo.
- [ ] Testes GUT verdes.
- [ ] Build Linux roda em máquina limpa (não dev box).
- [ ] Build Windows roda em VM Windows ou hardware real.
- [ ] Save de versão anterior carrega (migração testada).
- [ ] Tag anotada: `git tag -a v0.X.Y -m "..."` + push.
- [ ] CI passa e gera artifacts.
- [ ] Release Forgejo criado com binários + SHA256SUMS + changelog.

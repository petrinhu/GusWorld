#!/usr/bin/env bash
# linuxci_container.sh - CI-POLICY-LOCAL-FIRST: gemeo Linux do winbuild_container.sh.
#
# Reproduz o job "Linux (gcc, Release)" do .forgejo/workflows/ci.yml DENTRO de um
# container efemero fedora:44, para uso enquanto o Codeberg (e o CI Forgejo dele)
# esta fora do ar. Mesma regra suprema do irmao Windows: host limpo, nada sai do
# container.
#
#   - UM UNICO `docker run --rm`: dnf, configure, build, ctest, gates - tudo
#     acontece dentro do container, que morre ao sair (--rm).
#   - O repo entra READ-ONLY (:ro,Z) - o build NAO pode escrever no working tree
#     do host. Como CMake precisa escrever (build/, _deps/), o script COPIA o repo
#     pra um dir gravavel dentro do proprio container (/build, filesystem efemero)
#     logo no inicio, e builda de la. Nao ha bind mount de saida: nenhum artefato
#     volta pro host.
#   - TMPDIR do build tambem fica dentro do container (/build/tmp), nao usa o /tmp
#     do host.
#
# Estagios (espelham .forgejo/workflows/ci.yml job "linux" + tools/check.sh):
#   1) DNF_DEPS        - toolchain + deps de build do SDL3/RmlUi/glintfx (GL/X11/etc)
#   2) ARCH_GATE_PRE   - gate de arquitetura (roda cedo, antes de gastar build - mesma
#                        ordem do ci.yml: falha rapido se Qt/SDL/RmlUi/glintfx vazou
#                        pra core/ ou domain/)
#   3) CONFIGURE       - cmake --preset linux-release
#   4) BUILD           - cmake --build --preset linux-release
#   5) CTEST           - ctest --preset linux-release
#   6) SMOKE           - gusworld_app --smoke com drivers dummy do SDL (mesma logica
#                        do step "Smoke" do ci.yml / estagio SMOKE do check.sh)
#   7) I18N_PARITY     - tools/i18n_parity.py (paridade estrutural de chaves)
#
# Cada estagio imprime seu proprio "<ESTAGIO>=<exitcode>" no stdout; o script
# acumula um veredito final e sai != 0 se qualquer estagio falhou (nao para no
# primeiro erro depois do build, para o relatorio sair completo - mesmo espirito
# do tools/check.sh).
#
# Uso: bash tools/linuxci_container.sh
#   (roda a partir de qualquer cwd; resolve a raiz do repo sozinho)
#
# Runtime: docker (podman tambem serve, trocar o binario abaixo se preciso).
#
# -j4 fixo (nao -j$(nproc)): mesma cautela de OOM do winbuild_container.sh - nao
# rodar este script em paralelo com outro build pesado no host.
#
# Imagem PINADA em fedora:44 (a mais atual disponivel localmente no momento desta
# onda) - NUNCA fedora:latest (reprodutibilidade; anti-pattern "latest tag").
#
# DEPS DNF (achadas por tentativa: primeira rodada falhou faltando X e a lista foi
# completada iterativamente ate configure+build passarem - ver comentario abaixo
# da secao "dnf install" para o detalhe de cada pacote):
#   - toolchain: gcc-c++ cmake ninja-build git python3 which
#   - SDL3 (compilado do fonte via FetchContent) precisa dos -devel de
#     X11/Wayland/GL/áudio/inputs, equivalente Fedora da lista -dev do ci.yml:
#     libX11-devel libXext-devel libXrandr-devel libXcursor-devel libXi-devel
#     libXfixes-devel libXScrnSaver-devel libXtst-devel libxkbcommon-devel
#     wayland-devel wayland-protocols-devel libdecor-devel
#     mesa-libGL-devel mesa-libEGL-devel libdrm-devel mesa-libgbm-devel
#     pulseaudio-libs-devel alsa-lib-devel dbus-devel
#   - RmlUi usa find_package(Freetype) (NAO FetchContent) -> freetype-devel
#     obrigatorio no Fedora (o CI Ubuntu ja tinha isso via libfreetype-dev
#     transitivo de outro pacote; no Fedora e explicito).

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO="$(cd "$SCRIPT_DIR/.." && pwd)"

IMAGE="fedora:44"

echo "=== CI-POLICY-LOCAL-FIRST: Linux CI local em container efemero ==="
echo "repo (mount :ro): $REPO"
echo "imagem: $IMAGE"

docker run --rm -v "$REPO:/work:ro,Z" -w /work "$IMAGE" bash -lc '
  set -uo pipefail
  RC=0

  echo "--- [1/7] DNF_DEPS: toolchain + deps de build SDL3/RmlUi/glintfx ---"
  dnf -y install -q \
    gcc-c++ cmake ninja-build git python3 which \
    libX11-devel libXext-devel libXrandr-devel libXcursor-devel libXi-devel \
    libXfixes-devel libXScrnSaver-devel libXtst-devel libxkbcommon-devel \
    wayland-devel wayland-protocols-devel libdecor-devel \
    mesa-libGL-devel mesa-libEGL-devel libdrm-devel mesa-libgbm-devel \
    pulseaudio-libs-devel alsa-lib-devel dbus-devel \
    freetype-devel
  DNF_DEPS=$?
  echo "DNF_DEPS=$DNF_DEPS"
  [ "$DNF_DEPS" = "0" ] || RC=1

  # Copia pra um dir gravavel dentro do CONTAINER (efemero; nao e bind mount do
  # host). O mount /work e :ro de proposito - se algo tentar escrever nele, tem
  # que falhar aqui, nao silenciosamente no host.
  echo "--- copiando repo (ro) para /build (rw, efemero no container), sem build/ do host ---"
  # NAO usar `cp -a /work /build` puro: o host pode ter (e teve, na 1a prova desta
  # onda) um GusEngine/build/ de varios GB (builds locais + _deps de FetchContent
  # ja resolvidos) que nao serve pra nada aqui - alem de desperdicar minutos de
  # copia, ele carrega um CMakeCache.txt apontando pro path DO HOST, e o cmake
  # recusa reconfigurar (path da cache != path atual dentro do container). Um
  # runner de CI de verdade tambem comecaria sem build/ preexistente. `tar` com
  # --exclude evita copiar esse peso (nao depende de rsync estar no dnf install).
  mkdir -p /build
  tar -C /work --exclude=GusEngine/build -cf - . | tar -C /build -xf -
  export TMPDIR=/build/tmp
  mkdir -p "$TMPDIR"
  cd /build/GusEngine

  echo "--- [2/7] ARCH_GATE_PRE: gate de arquitetura (core/domain sem Qt/SDL/RmlUi/glintfx) ---"
  ARCH_GATE_PRE=0
  if grep -rnE "#[[:space:]]*include[[:space:]]*<(Q[A-Za-z]|SDL|RmlUi|glintfx)" core domain; then
    echo "GATE(arch): include de Qt, SDL, RmlUi ou glintfx encontrado em core/ ou domain/."
    ARCH_GATE_PRE=1
  elif grep -rnE "\\b(Rml|glintfx)::" core domain; then
    echo "GATE(arch): namespace Rml:: ou glintfx:: encontrado em core/ ou domain/."
    ARCH_GATE_PRE=1
  else
    echo "OK: nenhum include/namespace Qt/SDL/RmlUi/glintfx em core/ ou domain/."
  fi
  echo "ARCH_GATE_PRE=$ARCH_GATE_PRE"
  [ "$ARCH_GATE_PRE" = "0" ] || RC=1

  echo "--- [3/7] CONFIGURE: cmake --preset linux-release ---"
  CONFIGURE=0
  if [ "$DNF_DEPS" = "0" ]; then
    cmake --preset linux-release
    CONFIGURE=$?
  else
    echo "(pulado: DNF_DEPS falhou)"
    CONFIGURE=1
  fi
  echo "CONFIGURE=$CONFIGURE"
  [ "$CONFIGURE" = "0" ] || RC=1

  echo "--- [4/7] BUILD: cmake --build --preset linux-release (-j4) ---"
  BUILD=0
  if [ "$CONFIGURE" = "0" ]; then
    nice -n19 cmake --build --preset linux-release -- -j4
    BUILD=$?
  else
    echo "(pulado: CONFIGURE falhou)"
    BUILD=1
  fi
  echo "BUILD=$BUILD"
  [ "$BUILD" = "0" ] || RC=1

  echo "--- [5/7] CTEST: ctest --preset linux-release ---"
  CTEST=0
  if [ "$BUILD" = "0" ]; then
    ctest --preset linux-release
    CTEST=$?
  else
    echo "(pulado: BUILD falhou)"
    CTEST=1
  fi
  echo "CTEST=$CTEST"
  [ "$CTEST" = "0" ] || RC=1

  echo "--- [6/7] SMOKE: gusworld_app --smoke (SDL dummy, timeout 60s) ---"
  SMOKE=0
  APP_BIN=./build/linux-release/app/gusworld_app
  if [ "$BUILD" = "0" ] && [ -x "$APP_BIN" ]; then
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy timeout 60 "$APP_BIN" --smoke
    SMOKE=$?
  else
    echo "(pulado: BUILD falhou ou binario ausente)"
    SMOKE=1
  fi
  echo "SMOKE=$SMOKE"
  [ "$SMOKE" = "0" ] || RC=1

  echo "--- [7/7] I18N_PARITY: tools/i18n_parity.py ---"
  cd /build
  python3 tools/i18n_parity.py
  I18N_PARITY=$?
  echo "I18N_PARITY=$I18N_PARITY"
  [ "$I18N_PARITY" = "0" ] || RC=1

  echo "=== RESUMO ==="
  echo "DNF_DEPS=$DNF_DEPS CONFIGURE=$CONFIGURE BUILD=$BUILD CTEST=$CTEST ARCH_GATE_PRE=$ARCH_GATE_PRE SMOKE=$SMOKE I18N_PARITY=$I18N_PARITY"
  exit $RC
'
STATUS=$?
echo "=== container encerrado (--rm); host segue limpo, nenhum artefato persistido ==="
exit $STATUS

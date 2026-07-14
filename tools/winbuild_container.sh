#!/usr/bin/env bash
# winbuild_container.sh - valida cross-compile Windows x86_64 (MinGW-w64) do
# GusEngine DENTRO de um container efemero, sem tocar o host (WIN-CROSS-VALIDATE).
#
# REGRA SUPREMA (ordem do lider): host limpo, nada de lixo Windows sai do
# container. Por isso:
#   - UM UNICO `docker run --rm`: tudo (apt, FreeType cross, configure, build)
#     acontece dentro do container, que morre ao sair (--rm).
#   - O repo entra READ-ONLY (:ro,Z) - CMake NAO escreve dentro do source tree;
#     se algo tentar, o container falha em vez de sujar o working tree.
#   - O binary dir E TODO artefato Windows (.exe/.dll/.a) vivem em /tmp DO
#     CONTAINER (nao e bind mount - e o filesystem efemero do proprio
#     container), NUNCA num path do host nem do scratchpad.
#   - Nao ha `-v .../out` nem `docker cp`: os .exe sao so listados (`file` +
#     `sha256sum`) no stdout do container como prova, e morrem com o --rm.
#
# Uso: bash tools/winbuild_container.sh
#   (roda a partir de qualquer cwd; resolve a raiz do repo sozinho)
#
# Runtime observado: docker (podman tambem serve, trocar o binario abaixo se
# preciso - mesma sintaxe de --rm/-v/-w usada aqui).
#
# -j4 fixo (nao -j$(nproc)): a maquina tem swap sob pressao noutras sessoes;
# nice -n19 pra nao competir por CPU com o resto do host. UM container por vez
# (nao rodar este script em paralelo com outro build pesado).

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO="$(cd "$SCRIPT_DIR/.." && pwd)"

IMAGE="ghcr.io/catthehacker/ubuntu:act-latest"

echo "=== WIN-CROSS-VALIDATE: cross-build Windows x86_64 em container efemero ==="
echo "repo (mount :ro): $REPO"
echo "imagem: $IMAGE"

docker run --rm -v "$REPO:/work:ro,Z" -w /work "$IMAGE" bash -lc '
  set -euxo pipefail
  export DEBIAN_FRONTEND=noninteractive

  echo "--- apt-get: toolchain MinGW-w64 + build deps ---"
  apt-get update -qq
  apt-get install -y -qq \
    g++-mingw-w64-x86-64-posix gcc-mingw-w64-x86-64-posix \
    binutils-mingw-w64-x86-64 cmake ninja-build git ca-certificates

  echo "--- GATE: versao do cross-compiler (C++20 exige gcc >= 13) ---"
  x86_64-w64-mingw32-g++-posix --version
  # -dumpversion no wrapper -posix devolve "13-posix" (nao um numero puro tipo
  # gcc normal) - corta no "-" ANTES do "." (senao MAJOR vira "13-posix" e o
  # teste numerico abaixo quebra silenciosamente, como aconteceu na 1a prova
  # manual WIN-CROSS-VALIDATE 2026-07-14: `[: 13-posix: integer expression
  # expected`, gate nao abortou por engano).
  MAJOR=$(x86_64-w64-mingw32-g++-posix -dumpversion | cut -d- -f1 | cut -d. -f1)
  if [ "$MAJOR" -lt 13 ]; then
    echo "ABORT: x86_64-w64-mingw32-g++-posix major=$MAJOR < 13 (C++20 nao garantido)."
    exit 97
  fi

  echo "--- FreeType cross from-source (Ubuntu nao empacota freetype-mingw) ---"
  git clone --depth 1 --branch VER-2-13-3 \
    https://gitlab.freedesktop.org/freetype/freetype.git /tmp/ft
  cmake -S /tmp/ft -B /tmp/ftbuild -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE=/work/GusEngine/cmake/toolchains/mingw-w64.cmake \
    -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF \
    -DFT_DISABLE_ZLIB=ON -DFT_DISABLE_BZIP2=ON -DFT_DISABLE_PNG=ON \
    -DFT_DISABLE_HARFBUZZ=ON -DFT_DISABLE_BROTLI=ON \
    -DCMAKE_INSTALL_PREFIX=/tmp/winprefix
  nice -n19 cmake --build /tmp/ftbuild -j4
  cmake --install /tmp/ftbuild

  echo "--- GusEngine: configure + build (windows-mingw, binary dir em /tmp) ---"
  export WINDEPS_PREFIX=/tmp/winprefix
  cmake -S /work/GusEngine -B /tmp/winbuild -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE=/work/GusEngine/cmake/toolchains/mingw-w64.cmake \
    -DCMAKE_BUILD_TYPE=Release
  nice -n19 cmake --build /tmp/winbuild -j4

  echo "=== ARTEFATOS WINDOWS (ficam NO container, morrem no --rm) ==="
  find /tmp/winbuild -name "*.exe" -exec file {} \; -exec sha256sum {} \;
'
echo "=== container encerrado (--rm); host segue limpo, nenhum artefato Windows persistido ==="

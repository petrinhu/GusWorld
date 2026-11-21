#!/bin/bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Dynamic steps of the local CI mirror (TODO item C7, GW-35). They run only inside the CI
# image, never on the host. mirror.sh passes this file's text to `bash -c` in the container
# and streams a snapshot of the repository (a git repository with one commit) on stdin.
# Each step prints one line: "STEP <gate> <name> <rc>". The verdict is the host's, from rc.
#
# Inline commands here mirror the workflow `run:` blocks that have no script under
# tools/ci/ (the test count of build.yml, the sanitizer configure line, the commit-gate
# tree checks). Any change in a workflow must be copied here by hand: that is a known
# drift risk, noted in tools/ci/local/ and the C7 report.
set -u

# The image runs as the unprivileged user builder (WORKDIR /home/builder), so the tree goes
# under $HOME. --no-same-owner keeps the tar from asking for chown, which builder cannot do.
work="$HOME/gw-mirror"
mkdir -p "$work/rt" && cd "$work" || { echo "STEP setup tree 1"; exit 1; }
tar --no-same-owner -xf - || { echo "STEP setup extract 1"; exit 1; }
export RUNNER_TEMP="$work/rt"
export CC="${CC:-cc}" CXX="${CXX:-c++}" CMAKE_GENERATOR=Ninja

# step <gate> <name> <command...>: runs the command, logs it, prints the STEP line.
step() {
  local gate="$1" name="$2" rc=0 log
  shift 2
  log="$RUNNER_TEMP/$gate.$name.log"
  "$@" > "$log" 2>&1 || rc=$?
  echo "STEP $gate $name $rc"
  if [ "$rc" -ne 0 ]; then
    echo "--- last lines of $gate/$name:"
    tail -n 40 "$log"
  fi
}

# --- Gate 1, zero warnings: build.yml ---
step build configure cmake -S . -B build -DGUSWORLD_WERROR=ON "-DCMAKE_CXX_COMPILER=$CXX"
step build compile cmake --build build -j2

# Inline count (build.yml): ctest -N exits 0 with zero tests, so the value decides.
count_tests() {
  ctest --test-dir build -N > "$RUNNER_TEMP/inventory.txt" 2>&1 || { cat "$RUNNER_TEMP/inventory.txt"; return 1; }
  cat "$RUNNER_TEMP/inventory.txt"
  local total
  total="$(sed -n 's/^Total Tests: //p' "$RUNNER_TEMP/inventory.txt")"
  case "$total" in
    ''|*[!0-9]*) echo "FAIL: the test count is missing or not a number"; return 1 ;;
  esac
  [ "$total" -ne 0 ] || { echo "FAIL: zero tests registered"; return 1; }
  echo "registered tests: $total"
}
step build count count_tests
step build ctest ctest --test-dir build --output-on-failure

# --- Gate 2, sanitizers: sanitizers.yml ---
# The image lists its packages in /etc/gusworld/packages.txt, and libasan and libubsan are
# not among them (sanitizers.yml installs them with dnf, which needs network the container
# does not have). Without the runtime the gate is NOT RUN, never REPROVA: the host reads the
# NOTRUN line.
if rpm -q libasan libubsan > "$RUNNER_TEMP/san-runtime.txt" 2>&1; then
  step sanitizadores configure cmake -S . -B build-sanitizers -G Ninja \
    "-DCMAKE_CXX_COMPILER=$CXX" \
    "-DCMAKE_CXX_FLAGS=-fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer -g" \
    "-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined" \
    -DGUSWORLD_SANITIZER_FIXTURES=ON
  step sanitizadores build cmake --build build-sanitizers -j2
  step sanitizadores inventory bash -c 'ctest --test-dir build-sanitizers -N > "$RUNNER_TEMP/san-inventory.txt" 2>&1 && bash tools/ci/sanitizers/judge_inventory.sh "$RUNNER_TEMP/san-inventory.txt"'
  step sanitizadores ctest ctest --test-dir build-sanitizers --output-on-failure
else
  echo "NOTRUN sanitizadores runtime não executado: falta libasan/libubsan na imagem (sem rede, e instalar exige a ordem do líder, GW-28)"
fi

# --- Gate 5, commit message gate suite and mutation: commit-gate.yml ---
step gancho-commit suite-verde bash tools/ci/commit_gate/run_suite.sh tools/security/test_commit_gate.py "$RUNNER_TEMP/green.log" green
step gancho-commit extrai bash tools/ci/commit_gate/extract_copy.sh "$RUNNER_TEMP/gate-copy"
step gancho-commit muta python3 -I tools/ci/commit_gate/mutate.py "$RUNNER_TEMP/gate-copy/tools/security/commit_gate.py"
step gancho-commit sonda python3 -I tools/ci/commit_gate/probe.py "$RUNNER_TEMP/gate-copy/tools/security"
step gancho-commit prova-vermelha bash tools/ci/commit_gate/run_suite.sh "$RUNNER_TEMP/gate-copy/tools/security/test_commit_gate.py" "$RUNNER_TEMP/red.log" red:test_remove_acento_e_caixa
step gancho-commit arquivos-intactos bash -c 'git status --porcelain -- tools/security/commit_gate.py tools/security/test_commit_gate.py && git diff --exit-code -- tools/security/commit_gate.py tools/security/test_commit_gate.py'

# --- Layers, self-test of the gate: layers.yml (the --build half runs on the host) ---
step camadas selftest python3 tools/ci/check_layers.py --selftest

echo "CONTAINER STEPS DONE"

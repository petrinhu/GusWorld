# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Shared helpers for the invoker tests. Sourced by test_*.sh, which run inside
# the container (L-35). Every assertion counts one case. suite_done exits 1 when
# a case failed and also when no case ran at all, so an empty suite is red.

TEST_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"
REPO="$(cd "$TEST_DIR/../../.." && pwd -P)"
TDD_DIR="$REPO/tools/tdd"
FIXTURES="$TEST_DIR/fixtures"
CASES=0
FAILS=0

pass_case() {
    CASES=$((CASES + 1))
    echo "ok    $1"
}

fail_case() {
    CASES=$((CASES + 1))
    FAILS=$((FAILS + 1))
    echo "FALHA $1${2:+: $2}"
}

expect_eq() {
    if [ "$2" = "$3" ]; then
        pass_case "$1"
    else
        fail_case "$1" "esperado [$2], veio [$3]"
    fi
}

# expect_rc NAME WANT COMMAND...: the exit code must be exactly WANT.
expect_rc() {
    local name="$1" want="$2"
    shift 2
    "$@" >/dev/null 2>&1
    expect_eq "$name" "$want" "$?"
}

# expect_fail NAME COMMAND...: a real non-zero exit passes. Exit 127 (command not
# found) never passes, so a missing subject cannot pass a "must refuse" check.
expect_fail() {
    local name="$1"
    shift
    "$@" >/dev/null 2>&1
    local got=$?
    if [ "$got" -eq 127 ]; then
        fail_case "$name" "rc 127: comando ausente, nada foi testado"
    elif [ "$got" -ne 0 ]; then
        pass_case "$name"
    else
        fail_case "$name" "esperado rc diferente de zero, veio 0"
    fi
}

# require_file NAME PATH: the subject under test must exist before its checks run.
require_file() {
    if [ -f "$2" ]; then
        pass_case "$1"
    else
        fail_case "$1" "arquivo ausente: $2"
    fi
}

# require_function NAME FUNCTION: a sourced helper must define the function.
require_function() {
    if [ "$(type -t "$2")" = function ]; then
        pass_case "$1"
    else
        fail_case "$1" "funcao ausente: $2"
    fi
}

suite_done() {
    echo "casos=$CASES falhas=$FAILS"
    if [ "$CASES" -eq 0 ]; then
        echo "FALHA zero casos executados"
        exit 1
    fi
    [ "$FAILS" -eq 0 ] || exit 1
    exit 0
}

# make_fixture: a copy of the invoker tree in a scratch git repository, with a
# fake resource gate (the real one reads the host cgroup, absent in the container).
# Prints the fixture root. The fixture plants a secret path under docs/_secret.
make_fixture() {
    local fx
    fx="$(mktemp -d /tmp/tdd-fixture-XXXXXX)"
    mkdir -p "$fx/tools/ci/env" "$fx/tools/container" "$fx/tools/tdd" "$fx/core" "$fx/docs/_secret"
    cp -a "$REPO/tools/tdd/." "$fx/tools/tdd/"
    cp -a "$REPO/tools/container/." "$fx/tools/container/"
    cp -a "$REPO/tools/ci/env/." "$fx/tools/ci/env/"
    cp -a "$REPO/tools/ci/cmake_floor.sh" "$fx/tools/ci/cmake_floor.sh"
    cp "$FIXTURES/fake_gate.sh" "$fx/tools/container/resource_gate.sh"
    chmod +x "$fx/tools/container/resource_gate.sh"
    printf 'cmake_minimum_required(VERSION 3.20)\n' > "$fx/CMakeLists.txt"
    printf 'int main() { return 0; }\n' > "$fx/core/a.cpp"
    printf 'segredo de teste\n' > "$fx/docs/_secret/x"
    git -C "$fx" init -q
    git -C "$fx" add -A
    printf '%s\n' "$fx"
}

# make_fakebin WITH_DOCKER: a directory with a sentinel ctest, and optionally a fake docker.
# Prints the directory. The sentinel records its call in $SENTINEL_FILE.
make_fakebin() {
    local bin
    bin="$(mktemp -d /tmp/tdd-bin-XXXXXX)"
    cp "$FIXTURES/sentinel_ctest.sh" "$bin/ctest"
    chmod +x "$bin/ctest"
    if [ "$1" = yes ]; then
        cp "$FIXTURES/fake_docker.sh" "$bin/docker"
        chmod +x "$bin/docker"
    fi
    printf '%s\n' "$bin"
}

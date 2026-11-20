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

# Digest of the image the fake docker reports. Its tag is the hash of the current
# context plus this digest (env_hash.sh, A4), so the tag is computed, not typed.
TEST_DIGEST="sha256:a7d72ba2745b100a3351087472e5ee7b2450af7cf331e7673007e3817612839c"

# fixture_tag FX: the tag that the fixture's env_hash.sh gives for TEST_DIGEST.
fixture_tag() {
    printf 'gusworld-ci-fedora:%s\n' "$("$1/tools/container/env_hash.sh" "$TEST_DIGEST" | cut -c1-12)"
}

# setup_fake_image FX: the fake docker reports one local image, labelled with
# TEST_DIGEST and tagged with the tag that this context gives.
setup_fake_image() {
    export FAKE_IMAGE_LS
    FAKE_IMAGE_LS="$(fixture_tag "$1")"
    export FAKE_IMAGE_DIGEST="$TEST_DIGEST"
}

# make_mini_tree KIND DIR: a tiny CMake project with the helpers in_container.sh reads.
# KIND selects the defect: green, zero (no add_test), badcompile, failtest (test
# returns 1), badcmake (CMakeLists that does not parse), slow (a test that sleeps 3 s,
# so a budget of 1 s cuts it while the default 89 s lets it pass).
make_mini_tree() {
    local kind="$1" dir="$2" cml src
    mkdir -p "$dir/tools/tdd"
    cp "$TDD_DIR/xdg_check.sh" "$TDD_DIR/ctest_total.sh" "$TDD_DIR/touch_changed.sh" "$dir/tools/tdd/"
    chmod 755 "$dir/tools/tdd/ctest_total.sh" "$dir/tools/tdd/touch_changed.sh"
    cml="$dir/CMakeLists.txt"
    src="$dir/t.cpp"
    if [ "$kind" = badcmake ]; then
        printf 'this is not cmake(\n' > "$cml"
    else
        {
            printf 'cmake_minimum_required(VERSION 3.20)\n'
            printf 'project(mini CXX)\n'
            printf 'enable_testing()\n'
            printf 'add_executable(t t.cpp)\n'
            case "$kind" in
                zero) ;;
                failtest) printf 'add_test(NAME f COMMAND t)\n' ;;
                slow) printf 'add_test(NAME s COMMAND ${CMAKE_COMMAND} -E sleep 3)\n' ;;
                *) printf 'add_test(NAME ok COMMAND t)\n' ;;
            esac
        } > "$cml"
    fi
    case "$kind" in
        badcompile) printf 'int main( {\n' > "$src" ;;
        failtest) printf 'int main() { return 1; }\n' > "$src" ;;
        *) printf 'int main() { return 0; }\n' > "$src" ;;
    esac
}

# ic_run WORK TREE: streams TREE into in_container.sh as the tar would, in WORK.
# Sets IC_RC, IC_SECS and IC_OUT (the output file).
ic_run() {
    local work="$1" tree="$2" start
    IC_OUT="$work/out.log"
    tar -C "$tree" -cf "$work/in.tar" .
    start="$(date +%s)"
    TDD_WORK="$work" bash "$TDD_DIR/in_container.sh" < "$work/in.tar" > "$IC_OUT" 2>&1
    IC_RC=$?
    IC_SECS=$(( $(date +%s) - start ))
}

# ic_prime WORK TREE GENERATOR: the warm-up. A cold run extracts TREE (it must answer
# 3), then the tree is configured with GENERATOR, as a warm container would be.
ic_prime() {
    local work="$1" tree="$2" gen="$3"
    ic_run "$work" "$tree"
    cmake -S "$work/src" -B "$work/build" -G "$gen" > "$work/configure.log" 2>&1
}

# verdict_of FILE: the last "tdd-verdict:" value in an output file.
verdict_of() {
    sed -n 's/^tdd-verdict: //p' "$1" | sed -n '$p'
}

# new_work: a fresh work directory for in_container.sh (the container's /work).
new_work() {
    mktemp -d /tmp/tdd-work-XXXXXX
}

# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Runs inside gusworld-tdd-<hash16>, with the work tree on stdin as a tar. The host
# sends this file INLINE (bash -c "$(cat ...)"), so the first lines that run are
# always the host version, never a stale copy extracted by an earlier run (ADR-026).
# The helpers are read only after the swap, from the tree just extracted.
#
# Verdict: the last line is "tdd-verdict: N" and the exit code is N.
#   0  green
#   1  red: a compile error with its source location, or a failing test (ctest 8)
#   3  infrastructure: lock, extraction, cold or foreign cache, CMake regeneration
#      error, unparsed ctest total, timeout, any unexpected exit code
# The host maps 0 and 1 to the guard and anything else to 126.
# Before the verdict, "tdd-tempo: N" gives the seconds spent after the lock, which is
# the clock the 89 s budget bounds (C8 measurement, fatia 4).
# GUSWORLD_TDD_BUDGET, passed by run_suite.sh, can only SHORTEN that budget: a positive
# integer below 89. Any other value is ignored, so it never loosens the guard.
set -u

W="${TDD_WORK:-/work}"
BUILD_BUDGET=89
LOCK_WAIT=34
me="$$"

if [[ "${GUSWORLD_TDD_BUDGET:-}" =~ ^[1-9][0-9]*$ ]] && (( GUSWORLD_TDD_BUDGET < BUILD_BUDGET )); then
    BUILD_BUDGET=$GUSWORLD_TDD_BUDGET
fi

finish() {
    if [ -n "${start:-}" ]; then
        printf 'tdd-tempo: %s\n' "$(( $(date +%s) - start ))"
    fi
    printf 'tdd-verdict: %s\n' "$1"
    exit "$1"
}

# The lock is a symlink whose target is the pid of its holder. ln -s is atomic and
# fails when the name exists, and the pid is in place from the first instant the lock
# is visible, so no waiter ever sees a lock without an owner.
lock="$W/.tdd.lock"

lock_is_stale() {
    local holder
    if [ -L "$lock" ]; then
        holder="$(readlink "$lock")"
        case "$holder" in
            ''|*[!0-9]*) return 0 ;;
        esac
        kill -0 "$holder" 2>/dev/null && return 1
        return 0
    fi
    # Gone already: nothing to expire, retry at once. Anything else here is an
    # entry with no pid in it, which no run of this script creates: stale.
    [ -e "$lock" ] && return 0
    return 1
}

# Expiry by atomic move: the stale entry is renamed to a name only this run uses, and
# checked again there. If the parked entry turns out to belong to a live holder (a
# new lock raced in), it is put back with ln -s, which never overwrites.
expire_stale() {
    local parked="$W/.tdd.lock.expired.$me" holder
    mv -T "$lock" "$parked" 2>/dev/null || return 0
    if [ -L "$parked" ]; then
        holder="$(readlink "$parked")"
        case "$holder" in
            ''|*[!0-9]*) ;;
            *)
                if kill -0 "$holder" 2>/dev/null; then
                    ln -s "$holder" "$lock" 2>/dev/null && rm -f "$parked"
                    return 0
                fi
                ;;
        esac
    fi
    rm -rf "$parked"
    return 0
}

release_lock() {
    if [ "$(readlink "$lock" 2>/dev/null)" = "$me" ]; then
        rm -f "$lock"
    fi
}

deadline=$(( $(date +%s) + LOCK_WAIT ))
until ln -s "$me" "$lock" 2>/dev/null; do
    if lock_is_stale; then
        expire_stale
    fi
    if [ "$(date +%s)" -ge "$deadline" ]; then
        echo "tdd: trava ocupada por mais de ${LOCK_WAIT} s" >&2
        finish 3
    fi
    sleep 0.5
done
trap release_lock EXIT

# The build budget starts after the lock: build and ctest share the same 89 s.
start="$(date +%s)"
budget_left() {
    echo $(( BUILD_BUDGET - ($(date +%s) - start) ))
}

# Extract beside the live tree, then swap. A failed extraction never touches src.
rm -rf "$W/src.new" || finish 3
mkdir -p "$W/src.new" || finish 3
# The extraction is inside the budget too (review MÉDIA-3): what is left of the 89 s bounds it.
left="$(budget_left)"
[ "$left" -gt 0 ] || { echo "tdd: estouro de prazo antes da extracao" >&2; finish 3; }
timeout "$left" tar -x -f - -C "$W/src.new"
rc=$?
case "$rc" in
    0) ;;
    124|137) echo "tdd: estouro de prazo na extracao" >&2; finish 3 ;;
    *) echo "tdd: extracao da arvore falhou" >&2; finish 3 ;;
esac
rm -rf "$W/src.old" || finish 3
if [ -d "$W/src" ]; then
    mv "$W/src" "$W/src.old" || finish 3
fi
mv "$W/src.new" "$W/src" || finish 3
rm -rf "$W/src.old"

# tar keeps host mtimes: touch the files whose content changed since the last run, or
# the build would skip them (a false green). The manifest lives beside src, not in it.
# The touch runs inside the same budget (review MÉDIA-3), with what is left of it.
left="$(budget_left)"
[ "$left" -gt 0 ] || { echo "tdd: estouro de prazo antes da deteccao de mudanca" >&2; finish 3; }
timeout "$left" "$W/src/tools/tdd/touch_changed.sh" "$W/src" "$W/manifest.sha256"
rc=$?
case "$rc" in
    0) ;;
    124|137) echo "tdd: estouro de prazo na deteccao de mudanca" >&2; finish 3 ;;
    *) echo "tdd: deteccao de mudanca falhou" >&2; finish 3 ;;
esac

# shellcheck source=xdg_check.sh
. "$W/src/tools/tdd/xdg_check.sh" || finish 3

export XDG_RUNTIME_DIR="$W/xdg-runtime"
mkdir -p "$XDG_RUNTIME_DIR" && chmod 700 "$XDG_RUNTIME_DIR" || finish 3
xdg_runtime_ok "$XDG_RUNTIME_DIR" || { echo "tdd: XDG_RUNTIME_DIR sem modo 700 ou de outro dono" >&2; finish 3; }

# A cold container has no cache of this tree, and a cache made by another generator
# is not the one this guard reads: both are refused rather than rebuilt here.
cache="$W/build/CMakeCache.txt"
if ! grep -qxF "CMAKE_HOME_DIRECTORY:INTERNAL=$W/src" "$cache" 2>/dev/null; then
    echo "tdd: contêiner frio, sem CMakeCache apontando para $W/src" >&2
    finish 3
fi
if ! grep -qxF "CMAKE_GENERATOR:INTERNAL=Ninja" "$cache" 2>/dev/null; then
    echo "tdd: CMakeCache nao foi gerado com Ninja" >&2
    finish 3
fi

# Compile errors are recognised by their marker, not by the exit code, which is 1
# for many other failures too (a missing build.ninja, a CMakeLists that does not parse).
# Each failure is classified from the build output.
build_log="$W/build.log"
classify_build_failure() {
    if grep -qE "rebuilding 'build.ninja'|loading 'build.ninja'|CMake Error" "$build_log"; then
        echo "tdd: erro de regeneracao do CMake" >&2
        finish 3
    fi
    if grep -q '^FAILED: ' "$build_log" \
        && grep -qE '\.(c|cc|cpp|cxx|h|hpp):[0-9]+:[0-9]+: (fatal )?error:' "$build_log"; then
        echo "tdd: falha de compilacao" >&2
        finish 1
    fi
    echo "tdd: build sem veredito de compilacao" >&2
    finish 3
}

left="$(budget_left)"
[ "$left" -gt 0 ] || { echo "tdd: estouro de prazo antes do build" >&2; finish 3; }
timeout "$left" cmake --build "$W/build" > "$build_log" 2>&1
rc=$?
cat "$build_log"
if [ "$rc" -ne 0 ]; then
    if [ "$rc" -eq 124 ]; then
        echo "tdd: estouro de prazo no build" >&2
        finish 3
    fi
    classify_build_failure
fi

left="$(budget_left)"
[ "$left" -gt 0 ] || { echo "tdd: estouro de prazo antes da listagem de testes" >&2; finish 3; }
timeout "$left" ctest --test-dir "$W/build" -N > "$W/ctest-list.txt" 2>&1 \
    || { echo "tdd: ctest -N falhou" >&2; finish 3; }
if ! "$W/src/tools/tdd/ctest_total.sh" < "$W/ctest-list.txt" > /dev/null; then
    echo "tdd: ctest -N sem total positivo" >&2
    finish 3
fi

left="$(budget_left)"
[ "$left" -gt 0 ] || { echo "tdd: estouro de prazo antes da suite" >&2; finish 3; }
timeout "$left" ctest --test-dir "$W/build" --output-on-failure
rc=$?
case "$rc" in
    0) finish 0 ;;
    8) finish 1 ;;
    124|137) echo "tdd: estouro de prazo na suite" >&2; finish 3 ;;
    *) echo "tdd: ctest sem veredito de teste (rc $rc)" >&2; finish 3 ;;
esac

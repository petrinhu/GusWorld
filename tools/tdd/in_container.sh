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
#   1  red: the build failed to compile, or a test failed
#   3  infrastructure: lock, extraction, cold container, timeout, unreadable total
# The host maps 0 and 1 to the guard and anything else to 126.
set -u

W="${TDD_WORK:-/work}"
BUILD_BUDGET=89
LOCK_WAIT=34

finish() {
    printf 'tdd-verdict: %s\n' "$1"
    exit "$1"
}

# Lock by mkdir, which is atomic. A holder that is no longer alive leaves a stale
# lock, and it is removed. The wait is bounded, and the deadline is checked on every
# round, including the stale-lock round, so no path can spin forever.
lock="$W/.tdd.lock"
deadline=$(( $(date +%s) + LOCK_WAIT ))
while ! mkdir "$lock" 2>/dev/null; do
    holder="$(cat "$lock/pid" 2>/dev/null)"
    if [ -n "$holder" ] && ! kill -0 "$holder" 2>/dev/null; then
        rm -rf "$lock"
    fi
    if [ "$(date +%s)" -ge "$deadline" ]; then
        echo "tdd: trava ocupada por mais de ${LOCK_WAIT} s" >&2
        finish 3
    fi
    sleep 0.5
done
echo "$$" > "$lock/pid" || finish 3
trap 'rm -rf "$lock"' EXIT

# The build budget starts after the lock: build and ctest share the same 89 s.
start="$(date +%s)"
budget_left() {
    echo $(( BUILD_BUDGET - ($(date +%s) - start) ))
}

# Extract beside the live tree, then swap. A failed extraction never touches src.
rm -rf "$W/src.new" || finish 3
mkdir -p "$W/src.new" || finish 3
tar -x -f - -C "$W/src.new" || { echo "tdd: extracao da arvore falhou" >&2; finish 3; }
rm -rf "$W/src.old" || finish 3
if [ -d "$W/src" ]; then
    mv "$W/src" "$W/src.old" || finish 3
fi
mv "$W/src.new" "$W/src" || finish 3
rm -rf "$W/src.old"

# shellcheck source=xdg_check.sh
. "$W/src/tools/tdd/xdg_check.sh" || finish 3

export XDG_RUNTIME_DIR="$W/xdg-runtime"
mkdir -p "$XDG_RUNTIME_DIR" && chmod 700 "$XDG_RUNTIME_DIR" || finish 3
xdg_runtime_ok "$XDG_RUNTIME_DIR" || { echo "tdd: XDG_RUNTIME_DIR sem modo 700" >&2; finish 3; }

# A cold container has no cache of this tree: refuse rather than configure.
if ! grep -qxF "CMAKE_HOME_DIRECTORY:INTERNAL=$W/src" "$W/build/CMakeCache.txt" 2>/dev/null; then
    echo "tdd: contêiner frio, sem CMakeCache apontando para $W/src" >&2
    finish 3
fi

left="$(budget_left)"
[ "$left" -gt 0 ] || finish 3
timeout "$left" cmake --build "$W/build"
rc=$?
case "$rc" in
    0) ;;
    1) echo "tdd: falha de compilacao" >&2; finish 1 ;;
    *) echo "tdd: build sem veredito de compilacao (rc $rc)" >&2; finish 3 ;;
esac

left="$(budget_left)"
[ "$left" -gt 0 ] || finish 3
timeout "$left" ctest --test-dir "$W/build" -N > "$W/ctest-list.txt" 2>&1 || finish 3
if ! "$W/src/tools/tdd/ctest_total.sh" < "$W/ctest-list.txt" > /dev/null; then
    echo "tdd: ctest -N sem total positivo" >&2
    finish 3
fi

left="$(budget_left)"
[ "$left" -gt 0 ] || finish 3
timeout "$left" ctest --test-dir "$W/build" --output-on-failure
rc=$?
case "$rc" in
    0) finish 0 ;;
    124|137) echo "tdd: estouro de prazo na suite" >&2; finish 3 ;;
    *) finish 1 ;;
esac

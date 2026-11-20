# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# The inner verdict, EXECUTED inside the container (L-35) on a tiny CMake project.
# Each case warms a work dir (cold run, then configure with -G Ninja) and then runs
# the case. Every verdict is read from the output and checked by value.
. "$(dirname "${BASH_SOURCE[0]}")/lib.sh"

IC="$TDD_DIR/in_container.sh"
require_file "in_container.sh existe" "$IC"
expect_rc "cmake disponivel no contêiner" 0 command -v cmake
expect_rc "ninja disponivel no contêiner" 0 command -v ninja

TREES="$(mktemp -d /tmp/tdd-trees-XXXXXX)"
for kind in green zero badcompile failtest badcmake; do
    make_mini_tree "$kind" "$TREES/$kind"
done

# 1. Everything green: build, ctest -N with a value, ctest run.
W="$(new_work)"
ic_prime "$W" "$TREES/green" Ninja
ic_run "$W" "$TREES/green"
expect_eq "tudo verde: rc 0" 0 "$IC_RC"
expect_eq "tudo verde: veredito 0 impresso" 0 "$(verdict_of "$IC_OUT")"

# 2. Zero tests: ctest -N reports a total of 0, which is refused by value (E2).
W="$(new_work)"
ic_prime "$W" "$TREES/green" Ninja
ic_run "$W" "$TREES/zero"
expect_eq "zero testes: rc 3" 3 "$IC_RC"

# 3. build.ninja missing: a regeneration error, not a red build (E7a).
W="$(new_work)"
ic_prime "$W" "$TREES/green" Ninja
rm -f "$W/build/build.ninja"
ic_run "$W" "$TREES/green"
expect_eq "build.ninja ausente: rc 3" 3 "$IC_RC"

# 4. CMakeLists that does not parse: the regeneration fails during the build (E10).
W="$(new_work)"
ic_prime "$W" "$TREES/green" Ninja
ic_run "$W" "$TREES/badcmake"
expect_eq "CMakeLists invalido: rc 3" 3 "$IC_RC"

# 5. A real compile error in a source file is red (E7b).
W="$(new_work)"
ic_prime "$W" "$TREES/green" Ninja
ic_run "$W" "$TREES/badcompile"
expect_eq "erro real de compilacao: rc 1" 1 "$IC_RC"

# 6. A failing test (ctest exit 8) is red (E11).
W="$(new_work)"
ic_prime "$W" "$TREES/green" Ninja
ic_run "$W" "$TREES/failtest"
expect_eq "teste falho: rc 1" 1 "$IC_RC"

# 7. A cache configured with another generator is refused (MÉDIA-4).
W="$(new_work)"
ic_prime "$W" "$TREES/green" "Unix Makefiles"
ic_run "$W" "$TREES/green"
expect_eq "cache com outro gerador: rc 3" 3 "$IC_RC"

# 8. Locks. A lock whose holder is dead expires at once and the run goes on.
W="$(new_work)"
ic_prime "$W" "$TREES/green" Ninja
done_pid=$(bash -c 'exit 0 & echo $!; wait')
ln -s "$done_pid" "$W/.tdd.lock"
ic_run "$W" "$TREES/green"
expect_eq "trava com dono morto expira: rc 0" 0 "$IC_RC"
expect_eq "trava com dono morto expira em pouco tempo" 0 "$(( IC_SECS > 30 ? 1 : 0 ))"

# 9. A lock directory without a pid is stale and expires too.
W="$(new_work)"
ic_prime "$W" "$TREES/green" Ninja
mkdir "$W/.tdd.lock"
ic_run "$W" "$TREES/green"
expect_eq "trava sem pid expira: rc 0" 0 "$IC_RC"

# 10. A live holder is respected: the run gives up after the wait, with 3 (E4).
W="$(new_work)"
ic_prime "$W" "$TREES/green" Ninja
sleep 300 &
live=$!
ln -s "$live" "$W/.tdd.lock"
ic_run "$W" "$TREES/green"
kill "$live" 2>/dev/null
expect_eq "trava viva: rc 3" 3 "$IC_RC"
expect_eq "trava viva: espera limitada a 34 s" 1 "$(( IC_SECS >= 30 && IC_SECS <= 45 ? 1 : 0 ))"

suite_done

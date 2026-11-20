# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# prepare.sh warms the long-lived container (ADR-026, decisions 3 and 6). A missing
# container is created with the hardening of decision 7 and nothing else. An existing
# one is reused only when container_ready.sh approves it. /work is given to the
# builder by docker cp, never by root. The tree enters through in_container.sh, which
# answers 3 on a cold cache; then the tree is configured with Ninja and built once.
# A fake docker answers every call and logs it; each flag is checked by its text.
. "$(dirname "${BASH_SOURCE[0]}")/lib.sh"

PREP="$TDD_DIR/prepare.sh"
require_file "prepare.sh existe" "$PREP"
[ -f "$PREP" ] || suite_done

FAKE_PREP="$FIXTURES/fake_docker_prepare.sh"
require_file "fixture do docker falso de prepare existe" "$FAKE_PREP"
[ -f "$FAKE_PREP" ] || suite_done

GOOD_HOSTCONF="none|0|0|false|4294967296|[ALL]|[no-new-privileges]|builder"

# count REGEX: how many log lines match. Prints 0 when none does.
count() {
    local n
    n="$(grep -cE -- "$1" "$LOG")"
    printf '%s' "${n:-0}"
}

# line_of REGEX: the first log line that matches, or nothing.
line_of() {
    grep -nE -- "$1" "$LOG" | head -n 1 | cut -d: -f1
}

# expect_in NAME NEEDLE TEXT: TEXT contains NEEDLE.
expect_in() {
    if printf '%s\n' "$3" | grep -qF -- "$2"; then
        pass_case "$1"
    else
        fail_case "$1" "nao contem [$2]"
    fi
}

# expect_absent NAME REGEX TEXT: TEXT does not match REGEX.
expect_absent() {
    if printf '%s\n' "$3" | grep -qE -- "$2"; then
        fail_case "$1" "contem o padrao [$2]"
    else
        pass_case "$1"
    fi
}

# fresh: a new fixture tree, a directory with the fake docker, and the answers of a
# cold host: no container, /work absent, a cold cache, every command green, and a
# verdict 3 from the cold in_container run.
fresh() {
    FX="$(make_fixture)"
    BIN="$(mktemp -d /tmp/tdd-prep-XXXXXX)"
    cp "$FAKE_PREP" "$BIN/docker"
    chmod +x "$BIN/docker"
    LOG="$BIN/docker.log"
    OUT="$BIN/out.txt"
    : > "$LOG"
    setup_fake_image "$FX"
    export FAKE_DOCKER_LOG="$LOG" FAKE_DOCKER_STDIN="$BIN/exec-stdin.tar"
    export FAKE_DOCKER_INFO_RC=0 FAKE_IMAGE_RC=0 FAKE_IMAGE_ID=imagemfake
    export FAKE_CTR_RC=1 FAKE_CTR_IMAGE=sha256:imagemfake FAKE_CTR_HOSTCONF="$GOOD_HOSTCONF"
    export FAKE_CREATE_RC=0 FAKE_START_RC=0 FAKE_CP_RC=0
    export FAKE_WORK_RC=1 FAKE_WORK_W_RC=0 FAKE_CACHE_RC=1
    export FAKE_CMAKE_RC=0 FAKE_BUILD_RC=0 FAKE_GATE_RC=0
    export FAKE_EXEC_OUT="tdd-verdict: 3" FAKE_EXEC_RC=3
}

# run_prepare: one run of prepare.sh of the fixture against the fake docker. Sets RC.
run_prepare() {
    env PATH="$BIN:$PATH" bash "$FX/tools/tdd/prepare.sh" < /dev/null > "$OUT" 2>&1
    RC=$?
}

# A. Docker absent: refused with a reason, nothing created.
fresh
NODOCK="$(make_fakebin no)"
expect_fail "premissa: docker ausente no PATH do teste" \
    env PATH="$NODOCK:/usr/bin:/bin" sh -c 'command -v docker'
env PATH="$NODOCK:/usr/bin:/bin" bash "$FX/tools/tdd/prepare.sh" < /dev/null > "$OUT" 2>&1
RC=$?
expect_eq "sem docker prepare sai 1" 1 "$RC"
expect_in "sem docker o motivo cita docker" "docker" "$(cat "$OUT")"

# B. A stale image is refused before any container is touched.
fresh
export FAKE_IMAGE_LS=""
run_prepare
expect_eq "imagem defasada: rc 1" 1 "$RC"
expect_eq "imagem defasada: nenhum create" 0 "$(count '^create ')"

# C. An existing container that container_ready refuses is refused, not replaced.
fresh
export FAKE_CTR_RC=0 FAKE_CTR_HOSTCONF="none|1|0|false|4294967296|[ALL]|[no-new-privileges]|builder"
run_prepare
expect_eq "contêiner reprovado: rc 1" 1 "$RC"
expect_in "contêiner reprovado: motivo vem do container_ready" "diverge" "$(cat "$OUT")"
expect_eq "contêiner reprovado: nenhum create" 0 "$(count '^create ')"
expect_eq "contêiner reprovado: nenhum docker exec" 0 "$(count '^exec ')"

# D. A missing container is created with every flag of ADR-026, decision 7, and no mount.
fresh
run_prepare
CREATE="$(grep -E '^create ' "$LOG")"
expect_eq "criacao feita uma vez" 1 "$(count '^create ')"
for flag in "--network none" "--cap-drop ALL" "--security-opt no-new-privileges" \
    "--user builder" "--memory 4g" "--memory-swap 4g" "--pids-limit 233" "--cpus 4" \
    "--pull never" "sleep infinity" "sha256:imagemfake"; do
    expect_in "criacao leva: $flag" "$flag" "$CREATE"
done
expect_absent "criacao sem -v, --volume, --mount, --privileged nem --rm" \
    '(^| )(-v|--volume|--mount|--privileged|--rm)( |$)' "$CREATE"
expect_eq "nenhum docker run" 0 "$(count '^run ')"

# E. Order: create, start, copy /work, warm the tree, configure, build.
fresh
run_prepare
n_create="$(line_of '^create ')"
n_start="$(line_of '^start ')"
n_cp="$(line_of '^cp ')"
n_warm="$(line_of '^exec -i ')"
n_cfg="$(line_of '^exec [^ ]+ cmake -S ')"
n_build="$(line_of '^exec [^ ]+ cmake --build ')"
order_ok=1
prev=0
for n in "$n_create" "$n_start" "$n_cp" "$n_warm" "$n_cfg" "$n_build"; do
    case "$n" in
        ''|*[!0-9]*) order_ok=0; break ;;
    esac
    if [ "$n" -le "$prev" ]; then
        order_ok=0
        break
    fi
    prev=$n
done
expect_eq "ordem: criar, iniciar, copiar, aquecer, configurar, construir" 1 "$order_ok"

# F. Cold happy path: configure with Ninja, build once, and say it is ready.
fresh
run_prepare
expect_eq "caminho frio: rc 0" 0 "$RC"
expect_eq "configura com Ninja em /work/src e /work/build" 1 \
    "$(count '^exec [^ ]+ cmake -S /work/src -B /work/build -G Ninja$')"
expect_eq "constroi uma vez com paralelismo de 4" 1 \
    "$(count '^exec [^ ]+ cmake --build /work/build --parallel 4$')"
expect_in "termina dizendo que ficou pronto" "pronto" "$(cat "$OUT")"

# G. A cold run that does not answer 3 is refused: the cache was not cold after all.
fresh
export FAKE_EXEC_OUT="tdd-verdict: 0" FAKE_EXEC_RC=0
run_prepare
expect_eq "aquecimento frio com veredito 0 recusa: rc 1" 1 "$RC"
expect_eq "nenhum cmake depois do aquecimento inesperado" 0 "$(count 'cmake -S ')"

# H. A warm container is reused: no create, no copy, no configure.
fresh
export FAKE_CTR_RC=0 FAKE_WORK_RC=0 FAKE_CACHE_RC=0 FAKE_EXEC_OUT="tdd-verdict: 0" FAKE_EXEC_RC=0
run_prepare
expect_eq "reaproveitado e aquecido: rc 0" 0 "$RC"
expect_eq "reaproveitado: nenhum create" 0 "$(count '^create ')"
expect_eq "reaproveitado com /work: nenhum cp" 0 "$(count '^cp ')"
expect_eq "reaproveitado com cache: nenhum cmake -S" 0 "$(count 'cmake -S ')"
expect_eq "reaproveitado: aquece uma vez" 1 "$(count '^exec -i ')"

# I. A reused container without /work gets /work by docker cp, then goes on.
fresh
export FAKE_CTR_RC=0 FAKE_WORK_RC=1
run_prepare
expect_eq "reaproveitado sem /work: rc 0" 0 "$RC"
expect_eq "reaproveitado sem /work: uma copia de /work" 1 "$(count '^cp ')"

# J. Each failing step refuses with its own reason.
fresh
export FAKE_CTR_RC=1 FAKE_CREATE_RC=1
run_prepare
expect_eq "create falha: rc 1" 1 "$RC"
expect_in "create falha: motivo cita create" "create" "$(cat "$OUT")"

fresh
export FAKE_CMAKE_RC=1
run_prepare
expect_eq "configuracao falha: rc 1" 1 "$RC"
expect_in "configuracao falha: motivo cita configur" "configur" "$(cat "$OUT")"

fresh
export FAKE_BUILD_RC=2
run_prepare
expect_eq "build frio falha: rc 1" 1 "$RC"
expect_in "build frio falha: motivo cita build" "build" "$(cat "$OUT")"

# K. The resource gate refuses before any container is created (L-11, ADR-026).
fresh
export FAKE_GATE_RC=1
run_prepare
expect_eq "portao de recurso reprova: rc 1" 1 "$RC"
expect_in "portao de recurso reprova: motivo cita o portao" "portao de recurso" "$(cat "$OUT")"
expect_eq "portao de recurso reprova: nenhum create" 0 "$(count '^create ')"

suite_done

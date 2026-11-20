# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Each refusal (gate, image, container, container configuration) answers 126 with
# a reason, and docker exec is never called. The fake docker log proves the second
# part: a refused run leaves no "exec" line.
. "$(dirname "${BASH_SOURCE[0]}")/lib.sh"

RUN="$TDD_DIR/run_suite.sh"
require_file "run_suite.sh existe" "$RUN"
[ -f "$RUN" ] || suite_done

FX="$(make_fixture)"
BIN="$(make_fakebin yes)"
SENT="$BIN/sentinel-ran"
export SENTINEL_FILE="$SENT"
export FAKE_DOCKER_STDIN="$BIN/exec-stdin.tar"
export PATH="$BIN:$PATH"
state_hash="$(printf '%s' "$FX" | sha256sum | cut -c1-16)"
reason="/var/tmp/gusworld-tdd/$state_hash/last-reason"

# run_refusal LOG VAR=VALUE...: one run of the invoker with those variables set.
# Sets RR_RC. A fresh log per case, so the exec count is per case.
run_refusal() {
    local log="$1"
    shift
    env FAKE_DOCKER_LOG="$log" FAKE_EXEC_RC=0 FAKE_EXEC_OUT="tdd-verdict: 0" \
        "$@" bash "$FX/tools/tdd/run_suite.sh" >/dev/null 2>&1
    RR_RC=$?
}

# exec_calls LOG: how many docker exec lines the fake docker logged.
exec_calls() {
    grep -c '^exec' "$1" 2>/dev/null || true
}

setup_fake_image "$FX"

# Control: with no fault injected the invoker reaches docker exec and answers 0. Without
# this, every "exec never called" below could pass for the wrong reason (refused early).
LOG="$BIN/control.log"
run_refusal "$LOG"
expect_eq "controle sem falha: rc 0" 0 "$RR_RC"
expect_eq "controle sem falha: docker exec chamado uma vez" 1 "$(exec_calls "$LOG")"

LOG="$BIN/gate.log"
run_refusal "$LOG" FAKE_GATE_RC=1
expect_eq "gate reprova: rc 126" 126 "$RR_RC"
expect_eq "gate reprova: docker exec nunca chamado" 0 "$(exec_calls "$LOG")"

LOG="$BIN/stale.log"
run_refusal "$LOG" FAKE_IMAGE_LS="gusworld-ci-fedora:000000000000"
expect_eq "imagem defasada: rc 126" 126 "$RR_RC"
expect_rc "imagem defasada: motivo diz defasada" 0 grep -q 'defasada' "$reason"
expect_eq "imagem defasada: docker exec nunca chamado" 0 "$(exec_calls "$LOG")"

LOG="$BIN/imgrc.log"
run_refusal "$LOG" FAKE_IMAGE_RC=1
expect_eq "imagem com inspect falho: rc 126" 126 "$RR_RC"
expect_eq "imagem com inspect falho: docker exec nunca chamado" 0 "$(exec_calls "$LOG")"

LOG="$BIN/ctrrc.log"
run_refusal "$LOG" FAKE_CTR_RC=1
expect_eq "contêiner ausente (frio): rc 126" 126 "$RR_RC"
expect_rc "contêiner ausente: motivo diz frio" 0 grep -q 'frio' "$reason"
expect_eq "contêiner ausente: docker exec nunca chamado" 0 "$(exec_calls "$LOG")"

LOG="$BIN/ctrimg.log"
run_refusal "$LOG" FAKE_CTR_IMAGE="sha256:outra"
expect_eq "contêiner de outra imagem: rc 126" 126 "$RR_RC"
expect_eq "contêiner de outra imagem: docker exec nunca chamado" 0 "$(exec_calls "$LOG")"

LOG="$BIN/bind.log"
run_refusal "$LOG" FAKE_CTR_HOSTCONF="none|1|0|false|4294967296|4294967296|233|4000000000|[ALL]|0|[no-new-privileges]|builder||private|||0|0"
expect_eq "contêiner com bind: rc 126" 126 "$RR_RC"
expect_rc "contêiner com bind: motivo cita HostConfig" 0 grep -q 'HostConfig' "$reason"
expect_eq "contêiner com bind: docker exec nunca chamado" 0 "$(exec_calls "$LOG")"

suite_done

# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# A signal to the invoker is a "no run": SIGTERM, SIGINT or SIGHUP leave with 126 and
# a reason, never with the signal's own status (143), which the guard would read as red.
. "$(dirname "${BASH_SOURCE[0]}")/lib.sh"

RUN="$TDD_DIR/run_suite.sh"
require_file "run_suite.sh existe" "$RUN"
[ -f "$RUN" ] || suite_done

FX="$(make_fixture)"
BIN="$(make_fakebin yes)"
setup_fake_image "$FX"
export FAKE_DOCKER_LOG="$BIN/docker.log"
export FAKE_DOCKER_STDIN="$BIN/exec-stdin.tar"
export SENTINEL_FILE="$BIN/sentinel-ran"
export PATH="$BIN:$PATH"
# The fake exec sleeps, so the invoker is still waiting on docker when the signal comes.
export FAKE_EXEC_SLEEP=30
export FAKE_EXEC_RC=0
export FAKE_EXEC_OUT="tdd-verdict: 0"

state_hash="$(printf '%s' "$FX" | sha256sum | cut -c1-16)"
reason="/var/tmp/gusworld-tdd/$state_hash/last-reason"

bash "$FX/tools/tdd/run_suite.sh" >/dev/null 2>&1 &
pid=$!
sleep 4
# Control: the signal must land while docker exec is in flight. A run refused before
# that point would also answer 126, and the test would prove nothing.
expect_rc "sinal chegou com docker exec em curso" 0 grep -q '^exec' "$FAKE_DOCKER_LOG"
kill -TERM "$pid" 2>/dev/null
wait "$pid"
rc=$?
expect_eq "SIGTERM no invocador vira 126" 126 "$rc"
expect_rc "motivo registra o sinal" 0 grep -q 'sinal' "$reason"

suite_done

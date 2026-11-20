# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Case (a) end to end and the verdict rule. A fake docker answers "exec" with an
# exit code and a verdict line. Only a verdict line that matches the exit code, with
# 0 or 1, becomes 0 or 1. Anything else, including a bare exit 1 from a missing
# container, becomes 126. The tar that reaches docker exec is checked too.
. "$(dirname "${BASH_SOURCE[0]}")/lib.sh"

RUN="$TDD_DIR/run_suite.sh"
require_file "run_suite.sh existe" "$RUN"

FX="$(make_fixture)"
BIN="$(make_fakebin yes)"
if [ ! -f "$RUN" ]; then
    fail_case "run_suite presente antes dos casos do veredito"
    suite_done
fi
SENT="$BIN/sentinel-ran"
export SENTINEL_FILE="$SENT"
export FAKE_DOCKER_LOG="$BIN/docker.log"
setup_fake_image "$FX"
export FAKE_DOCKER_STDIN="$BIN/exec-stdin.tar"
export PATH="$BIN:$PATH"

# run_case EXEC_RC EXEC_OUT: runs the invoker once against the fake docker.
run_case() {
    FAKE_EXEC_RC="$1" FAKE_EXEC_OUT="$2" bash "$FX/tools/tdd/run_suite.sh" >/dev/null 2>&1
}

run_case 0 "tdd-verdict: 0"
expect_rc "verde do contêiner vira 0" 0 run_case 0 "tdd-verdict: 0"
expect_rc "vermelho do contêiner vira 1" 1 run_case 1 "tdd-verdict: 1"
expect_rc "docker exec 1 sem veredito vira 126" 126 run_case 1 ""
expect_rc "veredito 3 vira 126" 126 run_case 3 "tdd-verdict: 3"
expect_rc "morto por sinal sem veredito vira 126" 126 run_case 137 ""
expect_rc "veredito divergente do rc vira 126" 126 run_case 0 "tdd-verdict: 1"
expect_rc "rc 2 com veredito 2 vira 126" 126 run_case 2 "tdd-verdict: 2"

# The tar handed to docker exec: the invoker tree, no secret area.
run_case 0 "tdd-verdict: 0"
expect_rc "tar entregue ao docker exec e legivel" 0 tar -tf "$FAKE_DOCKER_STDIN"
tar -tf "$FAKE_DOCKER_STDIN" > "$BIN/exec-names.txt" 2>/dev/null
expect_eq "tar entregue tem o run_suite" 1 "$(grep -c '^tools/tdd/run_suite.sh$' "$BIN/exec-names.txt")"
expect_eq "tar entregue sem docs/_secret" 0 "$(grep -c '^docs/' "$BIN/exec-names.txt")"

# The reason file is rewritten on every run, green included.
state_hash="$(printf '%s' "$FX" | sha256sum | cut -c1-16)"
reason="/var/tmp/gusworld-tdd/$state_hash/last-reason"
run_case 1 "tdd-verdict: 1"
expect_rc "motivo do vermelho gravado" 0 grep -q 'vermelho' "$reason"
run_case 0 "tdd-verdict: 0"
expect_rc "motivo do verde sobrescreve o anterior" 0 grep -q 'verde' "$reason"

expect_rc "ctest local nunca rodou" 1 test -e "$SENT"

# The budget variable of the host reaches docker exec only when the host sets it (it
# can only shorten the budget, ADR-026 decision 5). A fresh log per case.
BUDGET_LOG="$BIN/budget.log"
rm -f "$BUDGET_LOG"
env FAKE_DOCKER_LOG="$BUDGET_LOG" GUSWORLD_TDD_BUDGET=1 FAKE_EXEC_RC=0 FAKE_EXEC_OUT="tdd-verdict: 0" \
    bash "$FX/tools/tdd/run_suite.sh" >/dev/null 2>&1
expect_eq "prazo do host chega ao docker exec" 1 "$(grep -c -- '-e GUSWORLD_TDD_BUDGET=1' "$BUDGET_LOG")"

rm -f "$BUDGET_LOG"
env -u GUSWORLD_TDD_BUDGET FAKE_DOCKER_LOG="$BUDGET_LOG" FAKE_EXEC_RC=0 FAKE_EXEC_OUT="tdd-verdict: 0" \
    bash "$FX/tools/tdd/run_suite.sh" >/dev/null 2>&1
expect_eq "sem prazo no host nada e repassado" 0 "$(grep -c 'GUSWORLD_TDD_BUDGET' "$BUDGET_LOG")"

# A verdict 3 keeps the reason of the infrastructure: the deadline line reaches last-reason.
PRAZO_OUT="$(printf 'tdd: estouro de prazo na suite\ntdd-verdict: 3')"
expect_rc "veredito 3 por prazo vira 126" 126 run_case 3 "$PRAZO_OUT"
expect_rc "motivo do prazo chega ao last-reason" 0 grep -q 'estouro de prazo na suite' "$reason"

suite_done

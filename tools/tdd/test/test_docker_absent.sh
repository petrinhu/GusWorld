# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Case (c): with docker absent the invoker answers 126 with a reason, and the
# local ctest never runs. PATH is explicit: no docker on it, sentinel ctest first.
. "$(dirname "${BASH_SOURCE[0]}")/lib.sh"

RUN="$TDD_DIR/run_suite.sh"
require_file "run_suite.sh existe" "$RUN"

FX="$(make_fixture)"
BIN="$(make_fakebin no)"
# A missing run_suite would also leave the sentinel absent; the checks below
# would then pass for the wrong reason, so they run only on a present subject.
if [ ! -f "$RUN" ]; then
    fail_case "run_suite presente antes dos casos de ausencia do docker"
    suite_done
fi
SENT="$BIN/sentinel-ran"
export SENTINEL_FILE="$SENT"

# Premise: under this PATH docker really is absent, so the case means something.
expect_fail "premissa: docker ausente no PATH do teste" \
    env PATH="$BIN:/usr/bin:/bin" sh -c 'command -v docker'

expect_rc "sem docker o invocador sai 126" 126 \
    env PATH="$BIN:/usr/bin:/bin" bash "$FX/tools/tdd/run_suite.sh"
expect_rc "ctest local nunca rodou" 1 test -e "$SENT"

state_hash="$(printf '%s' "$FX" | sha256sum | cut -c1-16)"
reason="/var/tmp/gusworld-tdd/$state_hash/last-reason"
expect_rc "motivo gravado menciona docker" 0 grep -q 'docker' "$reason"

suite_done

#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Text tests of the guard verdict: 18 fabricated logs, with no secret and no scan.
# Single responsibility: assert each judge outcome. It calls the same guard.sh that expect.sh
# calls for the red proofs and the real scan, found beside this file.
# Usage: verdict_cases.sh   (reads RUNNER_TEMP, which the CI runner always sets)
set -eu -o pipefail

here="$(cd "$(dirname "$0")" && pwd)"
guard="$here/guard.sh"
fx="$RUNNER_TEMP/fixtures"
mkdir -p "$fx"
printf 'INF 3 commits scanned.\nINF no leaks found\n' > "$fx/clean.log"
printf 'INF 1 commit scanned.\nINF no leaks found\n' > "$fx/singular.log"
printf 'INF 0 commits scanned.\nINF no leaks found\n' > "$fx/zero.log"
printf 'INF commits scanned.\nINF no leaks found\n' > "$fx/garbled.log"
printf 'INF 2 commits scanned.\n' > "$fx/no-verdict.log"
printf 'INF 1 commits scanned.\nWRN leaks found: 1\n' > "$fx/leak.log"
printf 'ERR stderr is not empty\nINF 1 commits scanned.\n' > "$fx/abort.log"
printf 'INF 3 commits scanned.\nINF no leaks found\nWRN leaks found: 2\n' > "$fx/leak-count.log"
printf 'INF 0 commits scanned.\nWRN leaks found: 1\n' > "$fx/zero-leak.log"
: > "$fx/empty.log"
printf 'ERR stderr is not empty\n' > "$fx/stderr-only.log"
printf 'INF 2 commits scanned.\nINF 5 commits scanned.\n' > "$fx/two-counts.log"
printf 'WRN leaks found: 1\n' > "$fx/leak-only.log"
check() {
  # check <name> <exit-code> <log> <expected judge status> <expected verdict>
  local name="$1" rc="$2" log="$3" want_status="$4" want_verdict="$5"
  local out status=0 got
  out=$(bash "$guard" judge "$rc" "$log") || status=$?
  got=$(sed -n 's/^GUARD_VERDICT=//p' <<<"$out")
  echo "$name: judge status $status, verdict $got"
  # The verdict is compared by the whole line, never by a fragment of it.
  if [ "$status" != "$want_status" ] || ! grep -qx "GUARD_VERDICT=$want_verdict" <<<"$out"; then
    echo "TEXT TEST FAILED: $name"
    exit 1
  fi
}
check clean-exit0 0 "$fx/clean.log" 0 accept
check singular-exit0 0 "$fx/singular.log" 0 accept
check zero-exit0 0 "$fx/zero.log" 1 zero
check garbled-exit0 0 "$fx/garbled.log" 1 unreadable
check no-verdict-exit0 0 "$fx/no-verdict.log" 1 format-unknown
check leak-exit1 1 "$fx/leak.log" 1 leak
check error-exit1 1 "$fx/clean.log" 1 tool-error
check abort-exit1 1 "$fx/abort.log" 1 escalate
check flag-exit126 126 "$fx/clean.log" 1 tool-error
check died-exit139 139 "$fx/clean.log" 1 tool-died
check odd-exit2 2 "$fx/clean.log" 1 unknown-code
check leak-with-count-exit0 0 "$fx/leak-count.log" 1 leak
check zero-with-leak-exit0 0 "$fx/zero-leak.log" 1 zero
check empty-exit0 0 "$fx/empty.log" 1 unreadable
check empty-exit1 1 "$fx/empty.log" 1 tool-error
check stderr-only-exit0 0 "$fx/stderr-only.log" 1 escalate
check two-counts-exit0 0 "$fx/two-counts.log" 1 unreadable
check leak-only-exit1 1 "$fx/leak-only.log" 1 leak
echo "ALL TEXT TESTS PASSED"

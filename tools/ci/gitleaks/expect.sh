#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Asserts one verdict of the secret gate. Used by both red proofs and by the real scan,
# so every step of gitleaks.yml calls the same path and judges nothing itself.
# Single responsibility: compare the outcome of guard.sh with the expected one. The verdict
# is defined only in guard.sh; this file reads it and never redefines it.
# Usage: expect.sh <expected-verdict> <expected-count|any> <repo> <log>
#   expected-verdict: accept, zero or leak.
# Prints the log and the guard output, then exits 0 only when the verdict matches, the count
# matches (or is any), and the exit status of the guard is the one the verdict needs.
set -eu

want="${1:?expected verdict missing}"
count_want="${2:?expected count missing, a number or any}"
repo="${3:?repo missing}"
log="${4:?log missing}"

here="$(cd "$(dirname "$0")" && pwd)"
gate=0
out=$(bash "$here/guard.sh" scan "$repo" "$log") || gate=$?
cat "$log"
printf '%s\n' "$out"

got=$(sed -n 's/^GUARD_VERDICT=//p' <<<"$out")
reason=$(sed -n 's/^GUARD_REASON=//p' <<<"$out")
count=$(sed -n 's/^GUARD_COUNT=//p' <<<"$out")
fail() {
  echo "EXPECT FAILED: $1; verdict ${got:-none}, reason ${reason:-none}"
  exit 1
}

# The verdict line is compared whole, never by a fragment of it.
grep -Fqx "GUARD_VERDICT=$want" <<<"$out" || fail "wanted verdict $want"

# guard.sh exits 0 only on accept, so the exit status must follow the verdict.
status_want=1
if [ "$want" = accept ]; then status_want=0; fi
[ "$gate" = "$status_want" ] || fail "guard exit status is $gate, wanted $status_want"

# A leak is a real finding: GitLeaks exits 1 on it, so the red proof checks the exit code too.
if [ "$want" = leak ]; then
  grep -Fqx 'GUARD_RC=1' <<<"$out" || fail "the exit code is not 1"
fi

# A fixed count is checked here. With "any", the floor "above zero" is already the accept
# rule of guard.sh, which refuses accept on a count of zero or unreadable; it is not repeated.
if [ "$count_want" != any ]; then
  grep -Fqx "GUARD_COUNT=$count_want" <<<"$out" || fail "wanted commit count $count_want"
fi

if [ "$want" = accept ]; then
  echo "REAL SCAN ACCEPTED: $count commits scanned, above zero"
else
  echo "EXPECT PASSED: $want"
fi

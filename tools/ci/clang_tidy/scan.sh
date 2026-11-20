#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Runs clang-tidy in the three modes of the gate, then judges the run with judge.sh.
# The one place where the clang-tidy command line lives: the real gate and both red
# proofs are different calls to this script.
#
# Why the job is written this way:
#   No CMake integration of the tool. The CMake documentation says a warning is reported
#   when the tool reports a problem, and it does not say whether that stops the build. A
#   script that calls clang-tidy directly controls the exit code itself.
#   --warnings-as-errors is on the COMMAND LINE only. The .clang-tidy file never sets
#   WarningsAsErrors, so the editor of a developer keeps warnings as warnings.
#   The exit code of clang-tidy is NOT in its documentation (the page is silent). It is
#   MEASURED here and printed. Nothing here assumes 1 or 0 for a finding.
#   One clang-tidy process per file, so each file has its own exit code and the count of
#   analysed files is real (L-36 of the global laws). That is 11 processes for the tree
#   and 2 for the fixtures. 13 in total, accepted by L-11 of the global laws for 11 files.
#   The fixture is analysed WITHOUT the database, with its own flags after "--", so the
#   compile database cannot hide it.
#
# Usage: scan.sh real <log-file> <records-file> <build-dir> <sources-list-file>
#        scan.sh with <log-file> <fixture>
#        scan.sh without <log-file> <fixture>
# Exit code is judge.sh's: 0 only when the verdict of the mode holds.
set -eu -o pipefail

mode="${1:?mode missing: real, with or without}"
log="${2:?log path missing}"

here="$(cd "$(dirname "$0")" && pwd)"

# The gate on the real tree: one clang-tidy process per file, each with its own exit code,
# written to records. Warnings are promoted to errors on the command line only. The loop
# keeps going after a failure, so the counts are complete (L-36).
run_real() {
  local records build sources src rc
  records="${3:?records path missing}"
  build="${4:?build directory missing}"
  sources="${5:?sources list missing}"
  : > "$records"
  : > "$log"
  while IFS= read -r src; do
    rc=0
    clang-tidy -p "$build" --warnings-as-errors='*' "$src" >> "$log" 2>&1 || rc=$?
    echo "exit ${rc} ${src}" >> "$records"
  done < "$sources"
  cat "$log"
  bash "$here/judge.sh" real "$records" "$log" "$sources"
}

# Red proofs. The fixture has one deliberate defect, outside the compile database, with its
# own flags after "--". WITH the option the gate must exit non-zero AND print the check id in
# brackets.
# The exit code is printed on its own line, because it is measured here and not documented.
# WITHOUT the option that is the trap: the same defect and no --warnings-as-errors, and
# clang-tidy must exit 0 WITH the finding in the log. That is the blind gate the option
# exists to prevent. The documentation does not promise exit 0 here, so the measured code is
# printed and the job decides by it.
run_proof() {
  local fixture rc
  fixture="${3:?fixture missing}"
  rc=0
  if [ "$mode" = "with" ]; then
    clang-tidy --warnings-as-errors='*' "$fixture" -- -std=c++23 > "$log" 2>&1 || rc=$?
  else
    clang-tidy "$fixture" -- -std=c++23 > "$log" 2>&1 || rc=$?
  fi
  cat "$log"
  bash "$here/judge.sh" "$mode" "$rc" "$log"
}

case "$mode" in
  real) run_real "$@" ;;
  with|without) run_proof "$@" ;;
  *) echo "FAIL: unknown mode ${mode}"; exit 1 ;;
esac

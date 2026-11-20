#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Runs the commit-gate self-test once and judges its verdict. One script serves both proofs:
#   green       the intact tree: exit 0, every declared case ran, the OK line is printed.
#   red:<case>  the mutated copy: exit 1 (not an interpreter death), the named case FAILs,
#               the FAILED summary is printed, every declared case ran.
# The judgement "ran equals declared" lives in judge_sweep, the only copy of it.
# Usage: run_suite.sh <suite-file> <log-file> green|red:<case-name>
# Exit 0 on a green verdict or a detected mutant, 1 otherwise, 2 on bad usage.
set -eu -o pipefail

suite="${1:?suite file missing}"
log="${2:?log file missing}"
mode="${3:?mode missing: green or red:<case-name>}"

# Judges one sweep: the cases the run reported against the cases declared in the file.
# A zero, an unreadable count or any difference is a broken sweep, never a pass.
# Usage: judge_sweep <ran> <declared>
judge_sweep() {
  local ran="$1" declared="$2"
  case "$ran" in ''|*[!0-9]*) echo "run count unreadable: broken sweep, not a verdict"; return 1 ;; esac
  case "$declared" in ''|*[!0-9]*) echo "declared count unreadable: broken sweep"; return 1 ;; esac
  if [ "$declared" -le 0 ] || [ "$ran" -le 0 ]; then
    echo "zero cases ran or declared: broken sweep, not a pass"
    return 1
  fi
  if [ "$ran" -ne "$declared" ]; then
    echo "ran $ran of $declared declared cases: a case is not collected"
    return 1
  fi
  return 0
}

run_green() {
  local rc=0 ran declared
  python3 -I "$suite" -v > "$log" 2>&1 || rc=$?
  cat "$log"
  echo "exit code on the intact tree: $rc"
  if [ "$rc" -ge 128 ]; then
    echo "interpreter died with code $rc: not a test verdict"
    exit 1
  fi
  if [ "$rc" -ne 0 ]; then
    echo "intact tree is red: the gate or its self-test is broken"
    exit 1
  fi
  ran=$(sed -n 's/^Ran \([0-9][0-9]*\) tests.*/\1/p' "$log")
  echo "tests run on the intact tree: ${ran:-none}"
  declared=$(grep -c '^\s*def test_' "$suite" || true)
  echo "cases declared in the file: ${declared:-unreadable}"
  if ! judge_sweep "$ran" "$declared"; then
    exit 1
  fi
  grep -qx 'OK' "$log" || { echo "no OK line in the log"; exit 1; }
}

run_red() {
  local case_name="$1" rc=0 ran declared
  python3 -I "$suite" -v > "$log" 2>&1 || rc=$?
  cat "$log"
  echo "exit code on the mutated copy: $rc"
  if [ "$rc" -ge 128 ]; then
    echo "interpreter died with code $rc: not a detection"
    exit 1
  fi
  if [ "$rc" -eq 0 ]; then
    echo "mutant SURVIVED: the suite passed on the mutated copy"
    exit 1
  fi
  if ! grep -q "^FAIL: $case_name (" "$log"; then
    echo "exit $rc, but the named case did not FAIL: a crash or an import error, not a detection"
    exit 1
  fi
  grep -q '^FAILED (failures=' "$log" || { echo "no FAILED summary line"; exit 1; }
  ran=$(sed -n 's/^Ran \([0-9][0-9]*\) tests.*/\1/p' "$log")
  declared=$(grep -c '^\s*def test_' "$suite" || true)
  echo "cases ran on the mutated copy: ${ran:-none}, declared in the copy: ${declared:-unreadable}"
  if ! judge_sweep "$ran" "$declared"; then
    exit 1
  fi
  echo "PROVED: the suite reds on the mutation, by the name $case_name"
}

case "$mode" in
  green) run_green ;;
  red:?*) run_red "${mode#red:}" ;;
  *) echo "usage: run_suite.sh <suite> <log> green|red:<case-name>" >&2; exit 2 ;;
esac

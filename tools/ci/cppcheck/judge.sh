#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Judges one cppcheck run. Single responsibility: exit code and counts to a verdict.
# Usage: judge.sh <mode> <exit-code> <log-file> <target>
#   real:     exit 0, and the Checking lines equal the sources in the list (target)
#   with:     exit 1 and at least one uninitvar finding (red proof, by code and by id)
#   without:  exit 0 with an uninitvar finding (the trap: blind without the option)
# Exit 0 only when the verdict of the mode holds.
set -eu

mode="${1:?mode missing}"
rc="${2:?exit code missing}"
log="${3:?log path missing}"
target="${4:?target missing}"

judge_real() {
  local found analysed
  found="$(wc -l < "$target")"
  analysed="$(grep -c '^Checking ' "$log")" || true
  echo "exit code: ${rc}"
  echo "encontrados: ${found}, analisados (Checking lines): ${analysed:-<unreadable>}"
  case "$rc" in
    0) ;;
    1)
      echo "FAIL: cppcheck exit 1, a finding or an invalid argument (read the log above)"
      exit 1
      ;;
    126)
      echo "FAIL: exit 126, cppcheck is not executable or the option is unknown"
      exit 1
      ;;
    *)
      if [ "$rc" -ge 128 ]; then
        echo "FAIL: exit ${rc}, cppcheck died by signal $((rc - 128)), not a finding"
      else
        echo "FAIL: unexpected exit ${rc}"
      fi
      exit 1
      ;;
  esac
  case "$analysed" in
    ''|*[!0-9]*)
      echo "FAIL: the analysed count is missing or not a number"
      exit 1
      ;;
  esac
  if [ "$analysed" -eq 0 ]; then
    echo "FAIL: zero files analysed, a broken scan (L-36)"
    exit 1
  fi
  if [ "$analysed" -ne "$found" ]; then
    echo "FAIL: analysed ${analysed} differs from found ${found}, coverage lost"
    exit 1
  fi
  echo "PASS: all ${found} sources analysed, no finding"
}

judge_proof() {
  local hits label want_rc want_text pass_text
  hits="$(grep -c 'uninitvar' "$log")" || true
  if [ "$mode" = "with" ]; then
    label="WITH --error-exitcode=1"
    want_rc=1
    want_text="expected exit 1 and an uninitvar finding"
    pass_text="PASS: the red proof is red, by code 1 and by the finding id"
  else
    label="WITHOUT --error-exitcode"
    want_rc=0
    want_text="expected exit 0 with an uninitvar finding"
    pass_text="PASS: without the option the finding is there and the exit is 0, the trap is real"
  fi
  echo "${label}: exit ${rc}, uninitvar lines: ${hits:-<unreadable>}"
  if [ "$rc" -ge 128 ]; then
    echo "NOTE: exit ${rc} is death by signal, not a finding"
  fi
  case "$hits" in
    ''|*[!0-9]*)
      echo "FAIL: the uninitvar count is missing or not a number"
      exit 1
      ;;
  esac
  if [ "$rc" -ne "$want_rc" ] || [ "$hits" -lt 1 ]; then
    echo "FAIL: ${want_text}, got exit ${rc} and ${hits}"
    exit 1
  fi
  echo "$pass_text"
}

case "$mode" in
  real) judge_real ;;
  with|without) judge_proof ;;
  *) echo "FAIL: unknown mode ${mode}"; exit 1 ;;
esac

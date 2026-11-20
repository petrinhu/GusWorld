#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Judges one clang-tidy run. Single responsibility: counts and exit codes to a verdict.
# One function for the real gate, one for the two red proofs (with and without the option).
#
# Why the verdict is written this way:
#   The exit code of clang-tidy is NOT in its documentation. Nothing here assumes 1 or 0
#   for a finding: the code is measured and printed.
#   Exit 1 is also what a parser or header error can return, so exit 1 alone never proves a
#   finding: the log must also carry the check id in brackets, like uninitvar in cppcheck.
#   Exit 126 (not executable) and 128 and above (death by signal, L-49 of the global laws)
#   are named apart from a finding, because they need opposite fixes.
#   A "fatal error" or "file not found" line is a parser or header problem, not a finding,
#   and is counted and printed on its own line.
#
# Usage: judge.sh real <records-file> <log-file> <sources-list-file>
#        judge.sh with|without <exit-code> <log-file>
# Exit 0 only when the verdict of the mode holds.
set -eu -o pipefail

judge_real() {
  local found analysed falharam parser stray
  found="$(wc -l < "$sources")"
  analysed="$(wc -l < "$records")"
  falharam="$(grep -cv '^exit 0 ' "$records")" || true
  parser="$(grep -cE 'fatal error|file not found' "$log")" || true
  stray="$(grep -E ' (warning|error): ' "$log" | grep -vc '\[')" || true
  echo "encontrados: ${found}, analisados (linhas em records): ${analysed}, falharam (exit diferente de 0): ${falharam:-<unreadable>}"
  echo "erro de parser ou header, nao achado (fatal error, file not found): ${parser:-<unreadable>}"
  echo "avisos ou erros sem id de verificacao entre colchetes: ${stray:-<unreadable>}"
  case "$found" in
    ''|*[!0-9]*)
      echo "FAIL: the source count is missing or not a number"
      exit 1
      ;;
  esac
  if [ "$found" -eq 0 ]; then
    echo "FAIL: zero sources found"
    exit 1
  fi
  if [ "$analysed" -ne "$found" ]; then
    echo "FAIL: analysed ${analysed} differs from found ${found}, coverage lost"
    exit 1
  fi
  if [ "${falharam:-1}" -ne 0 ]; then
    echo "FAIL: files with a non-zero exit, listed below (1 is a finding or a parser error, read the log; 126 or 128 and above is a broken tool, not a finding)"
    grep -v '^exit 0 ' "$records"
    exit 1
  fi
  if [ "${stray:-1}" -ne 0 ] || [ "${parser:-1}" -ne 0 ]; then
    echo "FAIL: the log has a parser or header error, or a message without a check id"
    exit 1
  fi
  echo "PASS: all ${found} sources analysed, no finding"
}

# The two red proofs share the same parse and the same 126 and 128 and above treatment.
# Only the expected condition differs: WITH needs a non-zero exit, WITHOUT needs exit 0.
judge_proof() {
  local hits parser label flag
  if [ "$mode" = "with" ]; then
    label="WITH"
    flag="com"
  else
    label="WITHOUT"
    flag="sem"
  fi
  hits="$(grep -c 'bugprone-macro-parentheses' "$log")" || true
  parser="$(grep -cE 'fatal error|file not found' "$log")" || true
  echo "CODIGO MEDIDO, ${flag} --warnings-as-errors: ${rc}"
  echo "${label}: exit ${rc}, bugprone-macro-parentheses lines: ${hits:-<unreadable>}, parser errors: ${parser:-<unreadable>}"
  case "$hits" in
    ''|*[!0-9]*)
      echo "FAIL: the check count is missing or not a number"
      exit 1
      ;;
  esac
  if [ "$rc" -eq 126 ]; then
    echo "FAIL: exit 126, clang-tidy is not executable"
    exit 1
  fi
  if [ "$rc" -ge 128 ]; then
    echo "FAIL: exit ${rc} is death by signal ($((rc - 128))), not a finding"
    exit 1
  fi
  if [ "$mode" = "with" ]; then
    if [ "$rc" -eq 0 ] || [ "$hits" -lt 1 ]; then
      echo "FAIL: expected a non-zero exit and the check id, got exit ${rc} and ${hits}"
      exit 1
    fi
    if [ "${parser:-1}" -ne 0 ]; then
      echo "FAIL: the non-zero exit comes with a parser or header error, not with the finding"
      exit 1
    fi
    echo "PASS: the red proof is red, by a non-zero exit and by the check id"
  else
    if [ "$rc" -ne 0 ] || [ "$hits" -lt 1 ]; then
      echo "FAIL: expected exit 0 with the check id, got exit ${rc} and ${hits}"
      exit 1
    fi
    if [ "${parser:-1}" -ne 0 ]; then
      echo "FAIL: a parser or header error in the trap case, the premise of this proof is broken"
      exit 1
    fi
    echo "PASS: without the option the finding is there and the exit is 0, the trap is real"
  fi
}

mode="${1:?mode missing: real, with or without}"
case "$mode" in
  real)
    records="${2:?records path missing}"
    log="${3:?log path missing}"
    sources="${4:?sources list missing}"
    judge_real
    ;;
  with|without)
    rc="${2:?exit code missing}"
    log="${3:?log path missing}"
    judge_proof
    ;;
  *) echo "FAIL: unknown mode ${mode}"; exit 1 ;;
esac

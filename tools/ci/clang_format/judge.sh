#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Judges a list of sources against the project policy (.clang-format, TODO item C11).
# Single responsibility: files to a verdict. One clang-format process per file, so a
# crash in one file cannot hide the others (L-36, batch tools).
#
# Why the flags are written this way:
#   --dry-run only prints the changes. Without -Werror it exits 0 even when a file needs
#   reformatting, so the gate would be green with a violation inside. -Werror turns each
#   change into an error and makes the exit code non-zero. Required.
#   --style=file is explicit: the policy comes from .clang-format, never from the default.
#   Exit 1 is also what an invalid argument or a missing .clang-format returns, so the
#   log is read for the id clang-format-violations too. Exit alone never proves a finding.
#   The floor of the tool version is declared here: an old clang-format is not the policy's
#   tool, and the verdict says so instead of judging with it.
#
# Usage: judge.sh <sources-list-file> <log-file>
# Exit 0 only when every listed file is analysed and passes.
set -eu -o pipefail

list="${1:?sources list path missing}"
log="${2:?log path missing}"
floor=18

command -v clang-format >/dev/null 2>&1 || { echo "FAIL: clang-format is not installed"; exit 1; }

version_line="$(clang-format --version)"
echo "${version_line}"
major="$(printf '%s\n' "$version_line" | sed -n 's/.*version \([0-9][0-9]*\).*/\1/p')"
case "$major" in
  ''|*[!0-9]*)
    echo "FAIL: the clang-format major version could not be read"
    exit 1
    ;;
esac
if [ "$major" -lt "$floor" ]; then
  echo "FAIL: clang-format ${major} is below the declared floor ${floor}"
  exit 1
fi

found="$(wc -l < "$list")"
analysed=0
falharam=0
: > "$log"
while IFS= read -r file || [ -n "$file" ]; do
  analysed=$((analysed + 1))
  rc=0
  clang-format --style=file --dry-run -Werror "$file" >> "$log" 2>&1 || rc=$?
  echo "exit ${rc}: ${file}" >> "$log"
  if [ "$rc" -ne 0 ]; then
    falharam=$((falharam + 1))
  fi
done < "$list"

violacoes="$(grep -c 'clang-format-violations' "$log")" || true
cat "$log"
echo "encontrados: ${found}, analisados: ${analysed}, falharam: ${falharam}, violacoes com id: ${violacoes:-<unreadable>}"

case "$found" in
  ''|*[!0-9]*)
    echo "FAIL: the source count is missing or not a number"
    exit 1
    ;;
esac
if [ "$found" -eq 0 ]; then
  echo "FAIL: zero files to judge, a broken scan (L-36)"
  exit 1
fi
if [ "$analysed" -ne "$found" ]; then
  echo "FAIL: analysed ${analysed} differs from found ${found}, coverage lost"
  exit 1
fi
if [ "$falharam" -ne 0 ]; then
  echo "FAIL: ${falharam} file(s) out of the policy or with a tool error (read the log above)"
  exit 1
fi
echo "PASS: all ${found} sources analysed, every one in the policy"

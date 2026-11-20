#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Single definition of the scan and of the verdict. Every step calls this file.
#   guard.sh scan <repo> <log>        runs gitleaks, then judges the outcome
#   guard.sh judge <exit-code> <log>  judges an outcome already recorded
# Prints GUARD_RC, GUARD_COUNT, GUARD_VERDICT and GUARD_REASON. Exit 0 only on accept.
set -u

judge() {
  local rc="$1" log="$2" count_lines count="" verdict reason
  count_lines=$(grep -cE '[0-9]+ commits? scanned' "$log" || true)
  if [ "$count_lines" = "1" ]; then
    count=$(grep -oE '[0-9]+ commits? scanned' "$log" | grep -oE '^[0-9]+' || true)
  fi
  case "$count" in ''|*[!0-9]*) count="" ;; esac
  if grep -q 'stderr is not empty' "$log"; then
    verdict="escalate"
    reason="stderr is not empty: gitleaks aborted, issue 2129 family. Escalate, do not migrate to stdin"
  elif [ "$rc" = "0" ]; then
    if [ -z "$count" ]; then
      verdict="unreadable"; reason="exit 0 but the commit count is missing or not a number"
    elif [ "$count" -eq 0 ]; then
      verdict="zero"; reason="exit 0 with zero commits scanned: broken sweep, not a clean tree"
    elif grep -qE 'leaks found: [0-9]+' "$log"; then
      verdict="leak"; reason="exit 0 but the log reports leaks found: a real finding"
    elif ! grep -q 'no leaks found' "$log"; then
      verdict="format-unknown"; reason="exit 0 without the no leaks found line"
    else
      verdict="accept"; reason="commit count above zero and no leaks found"
    fi
  elif [ "$rc" = "1" ]; then
    if grep -qE 'leaks found: [0-9]+' "$log"; then
      verdict="leak"; reason="real finding: leaks found"
    else
      verdict="tool-error"; reason="exit 1 without a leaks found line: tool error, not a verdict"
    fi
  elif [ "$rc" = "126" ]; then
    verdict="tool-error"; reason="exit 126: unknown flag or not executable"
  elif [ "$rc" -ge 128 ]; then
    verdict="tool-died"; reason="exit $rc: the tool died on a signal, not a verdict"
  else
    verdict="unknown-code"; reason="exit $rc: unexpected code, not a verdict"
  fi
  printf 'GUARD_RC=%s\nGUARD_COUNT=%s\nGUARD_VERDICT=%s\nGUARD_REASON=%s\n' \
    "$rc" "${count:-unreadable}" "$verdict" "$reason"
  [ "$verdict" = "accept" ]
}

scan() {
  local repo="$1" log="$2" rc=0
  # The one and only scan command. Every step reaches gitleaks through this file.
  gitleaks git --redact "$repo" > "$log" 2>&1 || rc=$?
  judge "$rc" "$log"
}

case "${1:-}" in
  scan) scan "${2:?repo missing}" "${3:?log missing}" ;;
  judge) judge "${2:?exit code missing}" "${3:?log missing}" ;;
  *) echo "usage: guard.sh scan <repo> <log> | judge <exit-code> <log>" >&2; exit 2 ;;
esac

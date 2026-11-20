#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Allowlist gate: counts the entries of an allowlist file and compares the count with the
# declared one. Single responsibility: count and compare. It never edits the file.
# Usage: allowlist_gate.sh <allowlist-file> <declared-count>
# Exit 0 only when the counts are equal. Entries are printed as path and rule, never whole.
set -eu -o pipefail

file="${1:?allowlist file missing}"
declared="${2:?declared count missing}"

# Counts the entries: non-comment, non-blank lines. Entries are never printed whole.
count_entries() {
  local file="$1" n
  if [ ! -f "$file" ]; then echo "allowlist file is missing: $file" >&2; return 1; fi
  n=$(grep -cvE '^[[:space:]]*(#|$)' "$file" || true)
  case "$n" in ''|*[!0-9]*) echo "allowlist count is unreadable" >&2; return 1 ;; esac
  printf '%s\n' "$n"
}
# Prints the path and the rule of each entry (never the commit, never the line),
# then compares the count with the declared one.
allowlist_gate() {
  local file="$1" declared="$2" n
  n=$(count_entries "$file") || return 1
  grep -vE '^[[:space:]]*(#|$)' "$file" | awk -F: '{ print "entry: path=" $(NF-2) " rule=" $(NF-1) }' || true
  if [ "$n" != "$declared" ]; then
    echo "ALLOWLIST GATE REJECTED: $n entries in $file, declared $declared" >&2
    return 1
  fi
  echo "allowlist gate: $n entries, declared $declared, equal"
}

allowlist_gate "$file" "$declared"

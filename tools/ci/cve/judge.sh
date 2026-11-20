#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Judges one grype run. Single responsibility: map the outcome to one verdict.
# Usage: judge.sh <exit-code> <grype.json> <grype.err> <expected-distro>
# Prints exactly one line, VERDICT=<value>. Exit 0 only on "accept".
#
# Order matters: the exit-code class first, then the package floor, then the distro,
# then the finding. A zero package count must report "zero", not "distro".
set -u

rc="${1:?exit code missing}"
json="${2:?json path missing}"
log="${3:?log path missing}"
want="${4:?expected distro missing}"

# The package count is in the INFO line "gathered packages" (grype -v). Format not yet measured.
pkgs=$(grep -E 'gathered packages' "$log" 2>/dev/null | head -n 1 \
  | grep -oE 'packages[=:][ "]*[0-9]+' | grep -oE '[0-9]+$' | head -n 1 || true)
# The distro is the first "name" inside the "distro" object of the JSON.
distro=$(tr -d ' \n' < "$json" 2>/dev/null \
  | grep -oE '"distro":\{"name":"[^"]*"' | head -n 1 \
  | sed 's/.*"name":"//; s/"$//' || true)

case "$rc" in
  ''|*[!0-9]*) verdict="unknown-code" ;;
  *)
    if [ "$rc" -ge 128 ]; then
      verdict="tool-died"
    elif [ "$rc" = "126" ] || [ "$rc" = "127" ]; then
      verdict="tool-error"
    elif [ "$rc" = "1" ]; then
      verdict="tool-error"
    elif [ -z "$pkgs" ]; then
      verdict="unreadable"
    elif [ "$pkgs" -eq 0 ]; then
      verdict="zero"
    elif [ -z "$distro" ]; then
      verdict="distro-unknown"
    elif [ "$distro" != "$want" ]; then
      verdict="distro-mismatch"
    elif [ "$rc" = "2" ]; then
      verdict="finding"
    elif [ "$rc" = "0" ]; then
      verdict="accept"
    else
      verdict="unknown-code"
    fi
    ;;
esac

printf 'VERDICT=%s\n' "$verdict"
[ "$verdict" = "accept" ]

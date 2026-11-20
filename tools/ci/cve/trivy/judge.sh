#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Judges one trivy run. Single responsibility: map the outcome to one verdict.
# Usage: judge.sh <exit-code> <trivy.json> <trivy.err> <expected-distro>
# Prints exactly one line, VERDICT=<value>. Exit 0 only on "accept".
#
# Same order as ../judge.sh (grype): exit-code class, then the package floor, then the distro,
# then the finding. Packages and OS family come from the JSON through jq (the package list
# of --list-all-pkgs, and Metadata.OS.Family), never from the log text. Unreadable JSON, or
# no jq, gives "unreadable": the gate fails closed and never guesses.
# The log argument is kept so the signature matches ../judge.sh (grype). This judge does not read it.
set -u

rc="${1:?exit code missing}"
json="${2:?json path missing}"
want="${4:?expected distro missing}"

pkgs=""
distro=""
if command -v jq >/dev/null 2>&1 && jq -e . "$json" >/dev/null 2>&1; then
  pkgs=$(jq -r '[.Results[]?.Packages[]?] | length' "$json" 2>/dev/null || true)
  distro=$(jq -r '.Metadata.OS.Family // empty' "$json" 2>/dev/null || true)
fi
case "$pkgs" in
  ''|*[!0-9]*) pkgs="" ;;
esac

case "$rc" in
  ''|*[!0-9]*) verdict="unknown-code" ;;
  *)
    if [ "$rc" -ge 128 ]; then
      verdict="tool-died"
    elif [ "$rc" = "126" ] || [ "$rc" = "127" ] || [ "$rc" = "1" ]; then
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

#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Prints the summary of one grype run, before the verdict: the package count from the log,
# the recognized distro, and the CVE count by severity, split into matched and ignored by
# --only-fixed. The full list goes to the artifact, not to the log.
# Single responsibility: print. It never judges and never fails the gate.
# Usage: report.sh <grype.json> <grype.err>
set -u

json="${1:?json path missing}"
log="${2:?log path missing}"

echo "--- grype report ---"
line=$(grep -E 'gathered packages' "$log" 2>/dev/null | head -n 1 || true)
echo "${line:-packages: the gathered packages line was not found in the log}"

if ! command -v jq >/dev/null 2>&1; then
  echo "report: jq is missing, no severity summary"
  exit 0
fi
if ! jq -e . "$json" >/dev/null 2>&1; then
  echo "report: the JSON is unreadable, no severity summary"
  exit 0
fi
# Matched entries carry .vulnerability. Ignored entries carry the match under .match or flat.
jq -r '
  "distro: \(.distro.name // "none") \(.distro.version // "")",
  "matched, by severity:",
  ((.matches // []) | map(.vulnerability.severity // "unknown") | group_by(.) | map("  \(.[0]): \(length)") | .[]),
  "ignored by --only-fixed, by severity:",
  ((.ignoredMatches // []) | map((.vulnerability.severity // .match.vulnerability.severity) // "unknown") | group_by(.) | map("  \(.[0]): \(length)") | .[])
' "$json" || echo "report: the summary failed"
exit 0

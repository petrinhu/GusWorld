#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Prints the summary of one trivy run, before the verdict: the OS family, the package count,
# the scanned targets (to check the excludes), the CRITICAL matches, and the data source IDs
# that produced those matches. Single responsibility: print. It never judges and never fails.
# Usage: report.sh <trivy.json> <trivy.err>
set -u

json="${1:?json path missing}"
log="${2:?log path missing}"

echo "--- trivy report ---"
if ! command -v jq >/dev/null 2>&1; then
  echo "report: jq is missing, no summary"
  exit 0
fi
if ! jq -e . "$json" >/dev/null 2>&1; then
  echo "report: the JSON is unreadable, no summary; first log lines follow"
  head -n 5 "$log" 2>/dev/null || true
  exit 0
fi
jq -r '
  "os: \(.Metadata.OS.Family // "none") \(.Metadata.OS.Name // "")",
  "packages: \([.Results[]?.Packages[]?] | length)",
  "targets: \([.Results[]?.Target // empty] | unique | join(", "))",
  "matched, by severity:",
  ([.Results[]?.Vulnerabilities[]?.Severity // "unknown"] | group_by(.) | map("  \(.[0]): \(length)") | .[]),
  "data source IDs of the matches:",
  ([.Results[]?.Vulnerabilities[]?.DataSource.ID // "none"] | unique | map("  \(.)") | .[])
' "$json" || echo "report: the summary failed"
exit 0

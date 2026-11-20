#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Asserts that allowlist_gate.sh rejects a given allowlist file. Single responsibility: the
# red proof of the allowlist gate. It runs before the real file is trusted, so the comparison
# is shown to be able to fail. It never counts or compares by itself.
# Usage: expect_allowlist_rejects.sh <allowlist-file> <declared-count>
# Exit 0 only when the gate exits 1 AND prints its rejection line.
set -eu

file="${1:?allowlist file missing}"
declared="${2:?declared count missing}"

here="$(cd "$(dirname "$0")" && pwd)"
status=0
out=$(bash "$here/allowlist_gate.sh" "$file" "$declared" 2>&1) || status=$?
printf '%s\n' "$out"
if [ "$status" != "1" ]; then
  echo "EXPECT FAILED: the allowlist gate exit status is $status, wanted 1"
  exit 1
fi
case "$out" in *REJECTED*) ;; *) echo "EXPECT FAILED: no rejection line"; exit 1 ;; esac
echo "EXPECT PASSED: the allowlist gate rejected the file"

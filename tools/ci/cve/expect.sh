#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Asserts one verdict. Used by the proof of strength (finding, zero) and by the real
# scan (accept), so every step of cve.yml runs the same path and judges nothing itself.
# Usage: expect.sh <expected-verdict> <expected-distro> <out-dir> <grype-target> [extra grype options...]
# Prints the scan output, then exits 0 only if the verdict equals the expected one.
set -u

want="${1:?expected verdict missing}"
distro="${2:?expected distro missing}"
out="${3:?out dir missing}"
target="${4:?grype target missing}"
shift 4

here="$(cd "$(dirname "$0")" && pwd)"
text=$(bash "$here/scan.sh" "$distro" "$out" "$target" "$@") || true
printf '%s\n' "$text"
got=$(printf '%s\n' "$text" | sed -n 's/^VERDICT=//p' | tail -n 1)
if [ "$got" != "$want" ]; then
  echo "EXPECT FAILED: wanted $want, got ${got:-none}"
  exit 1
fi
echo "EXPECT PASSED: $want"

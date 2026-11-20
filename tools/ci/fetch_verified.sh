#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Downloads one URL into one destination file and checks its SHA-256 before anything
# else may use it. Single responsibility: fetch and verify. It never extracts and
# never touches PATH; each caller keeps its own extraction and placement.
# Usage: fetch_verified.sh <url> <sha256> <destination-file>
# Exit 0 only when the digest matches. On a download error or a digest mismatch the
# destination is removed and the script exits 1, so an unverified file never stays behind.
set -euo pipefail

url="${1:?url missing}"
expected="${2:?sha256 missing}"
dest="${3:?destination file missing}"

fail() {
  echo "FAIL: $1" >&2
  rm -f -- "$dest"
  exit 1
}

if [[ ! "$expected" =~ ^[0-9a-f]{64}$ ]]; then
  echo "FAIL: sha256 malformed, expected 64 lowercase hex digits: $expected" >&2
  exit 1
fi

if ! curl -fsSL -o "$dest" "$url"; then
  fail "download failed: $url"
fi

if ! printf '%s  %s\n' "$expected" "$dest" | sha256sum -c --status -; then
  fail "sha256 mismatch for $dest, expected $expected"
fi

echo "OK: sha256 $expected verified for $dest ($url)"

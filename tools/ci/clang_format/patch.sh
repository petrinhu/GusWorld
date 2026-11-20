#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Produces the reformatting patch of the clang-format gate (TODO item C11). Single
# responsibility: list in, patch out. It rewrites the files in place with the project
# policy (.clang-format) and writes the working-tree diff as a binary-safe patch.
# It does not judge: the verdict is judge.sh's, and this patch never turns a red gate
# green. The patch is applied in a slice of its own, never by this script.
#
# Usage: patch.sh <sources-list-file> <patch-output-file>
set -eu -o pipefail

list="${1:?sources list path missing}"
out="${2:?patch output path missing}"

command -v clang-format >/dev/null 2>&1 || { echo "FAIL: clang-format is not installed"; exit 1; }

found="$(wc -l < "$list")"
case "$found" in
  ''|*[!0-9]*)
    echo "FAIL: the source count is missing or not a number"
    exit 1
    ;;
esac
if [ "$found" -eq 0 ]; then
  echo "FAIL: zero files to reformat, a broken scan (L-36)"
  exit 1
fi

while IFS= read -r file || [ -n "$file" ]; do
  clang-format --style=file -i "$file"
done < "$list"

git diff --binary > "$out"

if [ ! -s "$out" ]; then
  echo "PATCH: empty, the tree is in the policy"
  exit 0
fi
files="$(git diff --name-only | wc -l)"
echo "PATCH: ${files} file(s) reformatted"
echo "PATCH: $(wc -l < "$out") line(s) in ${out}"

#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Lists every .cpp and .hpp of the tree, fixtures excluded, into the file given as argument.
# Single responsibility: build the source list. Zero is a failure: an empty scan is a
# broken scan, not a clean tree (L-36).
# Why the headers are listed: cppcheck analyses a header only when it is passed in the
# list or reached by an #include from a listed source. A header under <layer>/include/ that
# no source includes would pass in silence without this, so the headers are input too.
# Usage: enumerate.sh <sources-list-file>
set -eu -o pipefail

list="${1:?sources list path missing}"

find . \( -name '*.cpp' -o -name '*.hpp' \) -not -path './.git/*' -not -path './tests/fixtures/*' \
  | sort > "$list"
found="$(wc -l < "$list")"
echo "encontrados (.cpp e .hpp fora dos fixtures): ${found}"
cat "$list"
case "$found" in
  ''|*[!0-9]*)
    echo "FAIL: the source count is missing or not a number"
    exit 1
    ;;
esac
if [ "$found" -eq 0 ]; then
  echo "FAIL: zero sources found"
  exit 1
fi

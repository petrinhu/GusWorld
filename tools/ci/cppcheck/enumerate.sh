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

# Self-check, the one that catches a dropped header. judge.sh counts found and analysed from
# the same list, so a header missing from the list would pass green there. Here every header
# under <layer>/include/ must be in the list. It applies only when such a header exists.
# Layer list: the same five names as .clang-tidy (HeaderFilterRegex) and scan.sh (-I loop).
layer_dirs=()
for layer in core content domain app present; do
  if [ -d "$layer/include" ]; then
    layer_dirs+=("./$layer/include")
  fi
done
if [ "${#layer_dirs[@]}" -gt 0 ]; then
  layer_headers="$(find "${layer_dirs[@]}" -name '*.hpp' | wc -l)"
  missing="$(comm -23 <(find "${layer_dirs[@]}" -name '*.hpp' | sort) <(sort "$list"))"
  echo "cabecalhos sob <camada>/include/: ${layer_headers}"
  if [ -n "$missing" ]; then
    echo "FAIL: headers under a layer's include/ are missing from the list:"
    echo "$missing"
    exit 1
  fi
fi
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

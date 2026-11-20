#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Lists every C and C++ source of the tree into the file given as argument, for the
# clang-format gate (TODO item C11). Single responsibility: build the list.
# Zero is a failure: an empty scan is a broken scan, not a clean tree (L-36).
#
# Pruned on purpose: .git, build directories (build, build*) and third-party trees
# (third_party, vendor, external). Fixtures are NOT pruned: they are C++ too, and the
# policy covers the whole tree. Symlinks are skipped (-type f does not follow them).
# Usage: enumerate.sh <sources-list-file>
set -eu -o pipefail

list="${1:?sources list path missing}"

find . \
  \( -type d \( -name .git -o -name 'build*' -o -name third_party -o -name vendor -o -name external \) -prune \) \
  -o -type f \( -name '*.c' -o -name '*.cc' -o -name '*.cpp' -o -name '*.cxx' \
     -o -name '*.h' -o -name '*.hh' -o -name '*.hpp' -o -name '*.hxx' \) -print \
  | sort > "$list"

found="$(wc -l < "$list")"
echo "encontrados (fontes C e C++ da arvore): ${found}"
cat "$list"
case "$found" in
  ''|*[!0-9]*)
    echo "FAIL: the source count is missing or not a number"
    exit 1
    ;;
esac
if [ "$found" -eq 0 ]; then
  echo "FAIL: zero sources found, a broken scan (L-36)"
  exit 1
fi

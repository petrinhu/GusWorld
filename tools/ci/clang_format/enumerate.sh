#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Lists every C and C++ source of the tree into the file given as argument, for the
# clang-format gate (TODO item C11). Single responsibility: build the list. Zero is a
# failure: an empty scan is a broken scan, not a clean tree (L-36).
#
# The inventory is the one of git: `git ls-files`, which lists what is TRACKED, not what
# is on disk. Build output and ignored trees are never tracked, so no directory is pruned
# by name (a name prune would also hide content/builders/). The suffix list is closed and
# case-sensitive: .C is C++ and .c is C. framework/ is excluded by explicit path: it is
# where the GlintFx submodule goes. Without git, or outside a work tree, the script fails
# closed.
#
# Usage: enumerate.sh <sources-list-file>
set -eu -o pipefail

list="${1:?sources list path missing}"
here="$(cd "$(dirname "$0")" && pwd)"
root="$(cd "$here/../../.." && pwd)"
cd "$root"

command -v git >/dev/null 2>&1 || { echo "FAIL: git is not installed, the inventory cannot be read"; exit 1; }
git rev-parse --is-inside-work-tree >/dev/null 2>&1 || { echo "FAIL: not a git work tree, the inventory cannot be read"; exit 1; }

# git ls-files runs alone under set -e, so a git failure aborts here. The grep stages are
# allowed to match nothing: the zero check below is the one that reprova.
tracked="$(git ls-files)"
printf '%s\n' "$tracked" \
  | grep -E '\.(c|cc|cpp|cxx|C|c\+\+|h|hh|hpp|hxx|h\+\+|ipp|inl|tpp|cppm|ixx|inc)$' \
  | grep -v '^framework/' \
  | sort > "$list" || true

found="$(wc -l < "$list")"
echo "encontrados (fontes C e C++ rastreados pelo git): ${found}"
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

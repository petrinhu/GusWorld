#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Compile database against the tree, in both directions. Single responsibility: the set
# of files in the database equals the set of files in the tree.
#
# Why this check exists:
#   clang-tidy does NOT fail on a file that is missing from the database: the
#   documentation is silent on that case, and it falls back to default flags, so a
#   file left out would be analysed in silence. The database must match the tree
#   exactly, in both directions, before the gate runs.
#
# Must run from the repository root: the "./" prefix of the sources list is rewritten
# with the physical path of the current directory (pwd -P).
# Usage: check_db.sh <compile-commands.json> <sources-list-file> <scratch-dir>
set -eu -o pipefail
export LC_ALL=C

db="${1:?compile database path missing}"
sources="${2:?sources list path missing}"
scratch="${3:?scratch directory missing}"

db_files="${scratch}/db-files.txt"
expected_files="${scratch}/expected-files.txt"

grep -o '"file": *"[^"]*"' "$db" | sed 's/^"file": *"//; s/"$//' | sort -u \
  > "$db_files"
sed "s#^\./#$(pwd -P)/#" "$sources" | sort -u \
  > "$expected_files"
expected="$(wc -l < "$expected_files")"
in_db="$(wc -l < "$db_files")"
missing="$(comm -23 "$expected_files" "$db_files" | wc -l)"
extra="$(comm -13 "$expected_files" "$db_files" | wc -l)"
echo "na arvore: ${expected}, na receita: ${in_db}, faltando na receita: ${missing}, sobrando na receita: ${extra}"
if [ "$missing" -ne 0 ] || [ "$extra" -ne 0 ]; then
  echo "FAIL: the compile database does not match the tree"
  comm -3 "$expected_files" "$db_files"
  exit 1
fi

#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Builds the test tar from a NUL-separated list (ADR-026, decision 2). The tar is
# written to a file, never piped, so its status is the status of tar itself. It is
# read back and its entry count must equal the list, or the run is refused.
# usage: make_tar.sh <repo_root> <list_nul> <out_tar>
set -uo pipefail

here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"
root="${1:?usage: make_tar.sh <repo_root> <list_nul> <out_tar>}"
list="${2:?usage: make_tar.sh <repo_root> <list_nul> <out_tar>}"
out="${3:?usage: make_tar.sh <repo_root> <list_nul> <out_tar>}"

# shellcheck source=secret_areas.sh
. "$here/secret_areas.sh"

if [ ! -s "$list" ]; then
    echo "make_tar: lista vazia ou ausente, abortado" >&2
    exit 1
fi

# Second lock, on the list itself: every entry is checked again before tar runs.
while IFS= read -r -d '' rel; do
    if is_secret_path "$rel"; then
        echo "make_tar: caminho de area cifrada: $rel, abortado" >&2
        exit 1
    fi
done < "$list"

if ! tar -C "$root" --null -T "$list" -cf "$out"; then
    echo "make_tar: tar falhou" >&2
    exit 1
fi

names="$out.names"
if ! tar -tf "$out" > "$names"; then
    echo "make_tar: tar gerado nao pode ser lido" >&2
    exit 1
fi

expected="$(tr -cd '\0' < "$list" | wc -c)"
got="$(wc -l < "$names")"
if [ "$expected" != "$got" ]; then
    echo "make_tar: tar tem $got entradas, a lista tem $expected" >&2
    exit 1
fi

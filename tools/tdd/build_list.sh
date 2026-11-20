#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Writes the NUL-separated list of files for the test tar: git ls-files over the
# pathspecs of build_paths.txt, minus the tracked files deleted from the tree.
# Fails closed (ADR-026): a git-crypt path in the listing aborts, and so does an
# empty list. Without pathspecs, git ls-files would list the whole repository, so
# an empty whitelist is refused too.
# usage: build_list.sh <repo_root> <out_list>
set -uo pipefail

here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"
root="${1:?usage: build_list.sh <repo_root> <out_list>}"
out="${2:?usage: build_list.sh <repo_root> <out_list>}"

# shellcheck source=secret_areas.sh
. "$here/secret_areas.sh"

pathspecs=()
while IFS= read -r line; do
    case "$line" in
        ''|'#'*) continue ;;
    esac
    pathspecs+=("$line")
done < "$here/build_paths.txt"

if [ "${#pathspecs[@]}" -eq 0 ]; then
    echo "build_list: lista branca vazia ou ausente, abortado" >&2
    exit 1
fi

# The raw listing stays beside the output and is overwritten on every run.
raw="$out.raw"
cd "$root" || exit 1
if ! git ls-files -z -co --exclude-standard -- "${pathspecs[@]}" > "$raw"; then
    echo "build_list: git ls-files falhou" >&2
    exit 1
fi

# Written beside the output and renamed only on success, so a refused run leaves
# no list under the real name.
partial="$out.partial"
: > "$partial" || exit 1
count=0
while IFS= read -r -d '' rel; do
    if is_secret_path "$rel"; then
        echo "build_list: caminho de area cifrada na lista: $rel, abortado" >&2
        exit 1
    fi
    if [ ! -e "$rel" ] && [ ! -L "$rel" ]; then
        continue
    fi
    printf '%s\0' "$rel" >> "$partial"
    count=$((count + 1))
done < "$raw"

if [ "$count" -eq 0 ]; then
    echo "build_list: lista vazia, abortado" >&2
    exit 1
fi
mv "$partial" "$out" || exit 1

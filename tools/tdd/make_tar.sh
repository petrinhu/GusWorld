#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Builds the test tar from a NUL-separated list (ADR-026, decision 2). The tar is
# written under a partial name, read back, and its entry count must equal the list.
# Only then is it renamed to the real name. A failure at any step therefore leaves no
# tar under the real name, without deleting anything (MÉDIA-6). The tar is written to
# a file, never piped, so its status is the status of tar itself.
#
# Declared limits (review of 2026-11-20, open items in the INBOX of TODO.md):
#   (a) The entry-count check between tar and list (the "expected" against "got"
#       comparison, currently at line 49) has no test of its own. Mutant M7, which
#       removes it, survives: no test passes a list with a parent directory such as
#       docs or core, so the count mismatch is never exercised.
#   (b) On refusal, the partial files (<out>.partial and <out>.partial.names) stay
#       on disk under /var/tmp, owned by the same user on the same machine as the
#       decrypted tree. They can hold secret content. build_list.sh cannot reach this
#       case, because it emits only files of the whitelist; the residue is still real
#       and is removed by the open item, not by this script.
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

partial="$out.partial"
names="$out.partial.names"

if ! tar -C "$root" --null -T "$list" -cf "$partial"; then
    echo "make_tar: tar falhou" >&2
    exit 1
fi

if ! tar -tf "$partial" > "$names"; then
    echo "make_tar: tar gerado nao pode ser lido" >&2
    exit 1
fi

expected="$(tr -cd '\0' < "$list" | wc -c)"
got="$(wc -l < "$names")"
if [ "$expected" != "$got" ]; then
    echo "make_tar: tar tem $got entradas, a lista tem $expected" >&2
    exit 1
fi

mv "$partial" "$out" || exit 1

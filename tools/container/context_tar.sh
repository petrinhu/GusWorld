#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Writes the build context to stdout as a tar that holds exactly the files of
# context_list.sh, flat, at the root. Nothing else from the tree is archived.
set -euo pipefail

here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=context_list.sh
. "$here/context_list.sh"

args=()
for rel in "${CONTEXT_FILES[@]}"; do
    args+=(-C "$REPO_ROOT/$(dirname "$rel")" "$(basename "$rel")")
done
tar -cf - "${args[@]}"

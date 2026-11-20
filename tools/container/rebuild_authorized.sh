#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Authorization guard for every script in this directory that pulls or builds
# (build_image.sh and base_probe.sh call it first, A1). It reads the COMMITTED
# decisions table, git show HEAD, never the working tree, and looks for a line
# that starts with today's rebuild authorization:
#   | DD/MM/AAAA | rebuild da imagem gusworld-ci-fedora autorizado |
# Exit 0 when that line is committed. Exit 1 otherwise, with the refusal message.
set -euo pipefail

here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
root="$(cd "$here/../.." && pwd)"

today="$(date +%d/%m/%Y)"
prefix="| ${today} | rebuild da imagem gusworld-ci-fedora autorizado |"

decisions="$(git -C "$root" show HEAD:docs/decisoes-do-lider.md 2>/dev/null)" || decisions=""

if printf '%s\n' "$decisions" | awk -v p="$prefix" 'index($0, p) == 1 { found = 1 } END { exit !found }'; then
    exit 0
fi

echo "REBUILD NOT AUTHORIZED: the leader must authorize today by AskUserQuestion, recorded in docs/decisoes-do-lider.md" >&2
exit 1

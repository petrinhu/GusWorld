#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Checks the state directory of the guard, and creates it private when it is missing
# (review of the notice.py finding, 2026-11-20). The path has a predictable name, so any
# local user can create it first. Accepted only when it is a real directory (never a
# link), owned by the current uid, and with no write bit for group or others. Its parent
# is checked the same way, and created private when missing. The reason of a refusal goes
# to stderr, and the callers turn it into a no-run (126) or a refusal (1).
# usage: state_dir.sh <state_dir>
set -uo pipefail

# check_private_dir PATH: creates PATH private when it is missing, then checks it.
check_private_dir() {
    local p="$1" uid mode
    if [ -L "$p" ]; then
        echo "state_dir: $p e link simbolico, recusado" >&2
        return 1
    fi
    if [ ! -e "$p" ]; then
        mkdir -m 0700 -- "$p" 2>/dev/null || {
            echo "state_dir: nao foi possivel criar $p" >&2
            return 1
        }
    fi
    if [ ! -d "$p" ]; then
        echo "state_dir: $p nao e diretorio" >&2
        return 1
    fi
    uid="$(stat -c '%u' -- "$p")" || return 1
    mode="$(stat -c '%a' -- "$p")" || return 1
    if [ "$uid" != "$(id -u)" ]; then
        echo "state_dir: $p nao pertence ao uid $(id -u)" >&2
        return 1
    fi
    if [ $(( 0$mode & 022 )) -ne 0 ]; then
        echo "state_dir: $p e gravavel por grupo ou outros (modo $mode)" >&2
        return 1
    fi
    return 0
}

dir="${1:?usage: state_dir.sh <state_dir>}"
parent="$(dirname -- "$dir")"

check_private_dir "$parent" || exit 1
check_private_dir "$dir" || exit 1
exit 0

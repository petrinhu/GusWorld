# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Sourced by in_container.sh. Defines xdg_runtime_ok DIR: the directory must exist,
# be mode 700 and belong to the current user (L-35: the test process gets its own
# runtime dir, not the session's). Runs nothing on its own.

xdg_runtime_ok() {
    local dir="$1" mode owner
    [ -d "$dir" ] || return 1
    mode="$(stat -c '%a' "$dir")" || return 1
    owner="$(stat -c '%u' "$dir")" || return 1
    [ "$mode" = 700 ] && [ "$owner" = "$(id -u)" ]
}

#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Takes one of three heavy-work slots (L-11 global, item 2). A slot is a flock on
# /var/tmp/gusworld-heavy-slot-<n>.lock, held for as long as the shell that took
# it lives. Any script that takes the same slot files is excluded while it runs;
# today build_image.sh is the only one in this tree that takes them, so the slots
# guard against two builds of this image, not against other heavy work on the host.
# Sourced by build_image.sh and prepare.sh. It defines two functions and runs nothing
# on its own.
#
# /var/tmp is writable by every local user, so the lock path has a predictable name.
# Before the slot is opened, the lock is refused when it is a symbolic link, when it is
# not a regular file, or when its owner is not the current uid. A refusal stops the
# whole take: a planted lock is not skipped over to the next slot. Review of the
# heavy_slot finding of 2026-11-20 (TODO.md, INBOX).

# check_slot_lock PATH: 0 when PATH may be opened as a slot lock, 1 with the reason on
# stderr otherwise. take_heavy_slot creates a missing PATH with O_EXCL before this check, so
# a missing PATH here only means that create failed; exec then fails as well.
check_slot_lock() {
    local p="$1" uid
    if [ -L "$p" ]; then
        echo "heavy_slot: REFUSED, ${p} is a symbolic link" >&2
        return 1
    fi
    if [ -e "$p" ]; then
        if [ ! -f "$p" ]; then
            echo "heavy_slot: REFUSED, ${p} is not a regular file" >&2
            return 1
        fi
        uid="$(stat -c '%u' -- "$p")" || {
            echo "heavy_slot: REFUSED, cannot read the owner of ${p}" >&2
            return 1
        }
        if [ "$uid" != "$(id -u)" ]; then
            echo "heavy_slot: REFUSED, ${p} belongs to uid ${uid}, not to uid $(id -u)" >&2
            return 1
        fi
    fi
    return 0
}

take_heavy_slot() {
    local n lock
    for n in 1 2 3; do
        lock="/var/tmp/gusworld-heavy-slot-${n}.lock"
        # O_EXCL: the lock is created only when nothing is at the path. A symbolic link or
        # a file of another owner makes this create fail, and check_slot_lock refuses it.
        # Once the lock exists, exec only opens it, so no create race is left after the check.
        (set -C; : > "$lock") 2>/dev/null || true
        check_slot_lock "$lock" || return 1
        exec 9>"$lock"
        if flock -n 9; then
            echo "heavy_slot: slot ${n} of 3 taken" >&2
            return 0
        fi
    done
    echo "heavy_slot: ABORTED, all three heavy-work slots are busy" >&2
    return 1
}

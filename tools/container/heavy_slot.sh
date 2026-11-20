#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Takes one of three heavy-work slots (L-11 global, item 2). A slot is a flock on
# /var/tmp/gusworld-heavy-slot-<n>.lock, held for as long as the shell that took
# it lives. Any script that takes the same slot files is excluded while it runs;
# today build_image.sh is the only one in this tree that takes them, so the slots
# guard against two builds of this image, not against other heavy work on the host.
# Sourced by build_image.sh. It defines one function and runs nothing on its own.

take_heavy_slot() {
    local n
    for n in 1 2 3; do
        exec 9>"/var/tmp/gusworld-heavy-slot-${n}.lock"
        if flock -n 9; then
            echo "heavy_slot: slot ${n} of 3 taken" >&2
            return 0
        fi
    done
    echo "heavy_slot: ABORTED, all three heavy-work slots are busy" >&2
    return 1
}

#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Driver of the invoker tests. Runs inside the container (GW-35), never on the host.
# Each test_*.sh runs in its own bash process. The driver exits 1 when a file
# fails and when there is no test file at all.
set -u

here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"
status=0
ran=0
for f in "$here"/test_*.sh; do
    if [ ! -e "$f" ]; then
        echo "FALHA nenhum arquivo test_*.sh encontrado"
        exit 1
    fi
    ran=$((ran + 1))
    echo "== $(basename "$f")"
    if ! bash "$f"; then
        echo "FALHA arquivo $(basename "$f")"
        status=1
    fi
done

[ "$ran" -gt 0 ] || exit 1
exit "$status"

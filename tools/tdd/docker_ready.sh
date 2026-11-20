#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Checks that the docker client exists and that the daemon answers. Prints the
# reason on failure. Both checks are bounded, so a hung daemon cannot hang the guard.
if ! command -v docker >/dev/null 2>&1; then
    echo "docker ausente no PATH"
    exit 1
fi
if ! timeout 15 docker info >/dev/null 2>&1; then
    echo "daemon do docker inacessivel"
    exit 1
fi

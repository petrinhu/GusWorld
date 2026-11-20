#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Checks the long-lived test container: it must exist, be created from the expected
# image id, and be running. A stopped container is started with docker start, which
# never downloads anything (ADR-026). A missing container is a cold container: the
# guard answers 126 and does not create one here.
# usage: container_ready.sh <container_name> <image_id>
set -uo pipefail

ctr="${1:?usage: container_ready.sh <container_name> <image_id>}"
want="${2:?usage: container_ready.sh <container_name> <image_id>}"

have="$(timeout 20 docker container inspect --format '{{.Image}}' "$ctr" 2>/dev/null)" || {
    echo "contêiner $ctr ausente (contêiner frio)"
    exit 1
}
if [ "$have" != "$want" ]; then
    echo "contêiner $ctr foi criado de outra imagem"
    exit 1
fi

running="$(timeout 20 docker container inspect --format '{{.State.Running}}' "$ctr" 2>/dev/null)" || {
    echo "contêiner $ctr não pôde ser inspecionado"
    exit 1
}
if [ "$running" != true ]; then
    if ! timeout 60 docker start "$ctr" >/dev/null 2>&1; then
        echo "falha no docker start de $ctr"
        exit 1
    fi
fi

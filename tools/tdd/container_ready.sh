#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Checks the long-lived test container: it must exist, be created from the expected
# image id, be running, and carry the hardening of ADR-026 (decision 7). A stopped
# container is started with docker start, which never downloads anything. A missing
# container is a cold container: refused here, never created here.
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

# One read of the whole hardening, in this order:
# network|binds|mounts|privileged|memory|capdrop|secopt|user
hc="$(timeout 20 docker container inspect --format \
    '{{.HostConfig.NetworkMode}}|{{len .HostConfig.Binds}}|{{len .Mounts}}|{{.HostConfig.Privileged}}|{{.HostConfig.Memory}}|{{.HostConfig.CapDrop}}|{{.HostConfig.SecurityOpt}}|{{.Config.User}}' \
    "$ctr" 2>/dev/null)" || {
    echo "contêiner $ctr: HostConfig não pôde ser lido"
    exit 1
}
IFS='|' read -r net binds mounts priv mem capdrop secopt user <<< "$hc"

bad=""
[ "$net" = none ] || bad="$bad rede=$net"
[ "$binds" = 0 ] || bad="$bad binds=$binds"
[ "$mounts" = 0 ] || bad="$bad mounts=$mounts"
[ "$priv" = false ] || bad="$bad privileged=$priv"
case "$mem" in
    ''|0|*[!0-9]*) bad="$bad memoria=$mem" ;;
esac
case "$capdrop" in
    *ALL*) ;;
    *) bad="$bad capdrop=$capdrop" ;;
esac
case "$secopt" in
    *no-new-privileges*) ;;
    *) bad="$bad secopt=$secopt" ;;
esac
[ "$user" = builder ] || bad="$bad user=$user"

if [ -n "$bad" ]; then
    echo "contêiner $ctr diverge do HostConfig esperado:$bad"
    exit 1
fi

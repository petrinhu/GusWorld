#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Checks the long-lived test container: it must exist, be created from the expected
# image id, and carry the hardening of ADR-026 (decision 7) with the exact values of
# container_spec.sh. The HostConfig is read and judged BEFORE any docker start (review
# MÉDIA-2): a divergent stopped container is refused and never started. A stopped but
# conforming container is started with docker start, which never downloads anything.
# A missing container is a cold container: refused here, never created here.
# usage: container_ready.sh <container_name> <image_id>
set -uo pipefail

here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"
# shellcheck source=container_spec.sh
. "$here/container_spec.sh"

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

# One read of the whole hardening, in this order (18 fields, '|' separated):
#  0 network  1 binds  2 mounts  3 privileged  4 memory  5 memswap  6 pids  7 nanocpus
#  8 capdrop  9 capadd(count)  10 secopt  11 user  12 pidmode  13 ipcmode  14 utsmode
#  15 usernsmode  16 devices  17 tmpfs
# Devices and Tmpfs are read through index: docker's template errors on a missing key.
hc="$(timeout 20 docker container inspect --format \
    '{{.HostConfig.NetworkMode}}|{{if index .HostConfig "Binds"}}{{len (index .HostConfig "Binds")}}{{else}}0{{end}}|{{if .Mounts}}{{len .Mounts}}{{else}}0{{end}}|{{.HostConfig.Privileged}}|{{.HostConfig.Memory}}|{{.HostConfig.MemorySwap}}|{{.HostConfig.PidsLimit}}|{{.HostConfig.NanoCpus}}|{{.HostConfig.CapDrop}}|{{if index .HostConfig "CapAdd"}}{{len (index .HostConfig "CapAdd")}}{{else}}0{{end}}|{{.HostConfig.SecurityOpt}}|{{.Config.User}}|{{.HostConfig.PidMode}}|{{.HostConfig.IpcMode}}|{{.HostConfig.UTSMode}}|{{.HostConfig.UsernsMode}}|{{if index .HostConfig "Devices"}}{{len (index .HostConfig "Devices")}}{{else}}0{{end}}|{{if index .HostConfig "Tmpfs"}}{{len (index .HostConfig "Tmpfs")}}{{else}}0{{end}}' \
    "$ctr" 2>/dev/null)" || {
    echo "contêiner $ctr: HostConfig não pôde ser lido"
    exit 1
}
IFS='|' read -r net binds mounts priv mem memswap pids nano capdrop capadd secopt user \
    pidmode ipcmode utsmode usernsmode devices tmpfs <<< "$hc"

bad=""
[ "$net" = "$SPEC_NETWORK" ] || bad="$bad rede=$net"
[ "$binds" = 0 ] || bad="$bad binds=$binds"
[ "$mounts" = 0 ] || bad="$bad mounts=$mounts"
[ "$priv" = false ] || bad="$bad privileged=$priv"
[ "$mem" = "$SPEC_MEMORY_BYTES" ] || bad="$bad memoria=$mem"
[ "$memswap" = "$SPEC_MEMORY_SWAP_BYTES" ] || bad="$bad memswap=$memswap"
[ "$pids" = "$SPEC_PIDS" ] || bad="$bad pids=$pids"
[ "$nano" = "$SPEC_NANOCPUS" ] || bad="$bad cpus=$nano"
case "$capdrop" in
    *ALL*) ;;
    *) bad="$bad capdrop=$capdrop" ;;
esac
[ "$capadd" = 0 ] || bad="$bad capadd=$capadd"
case "$secopt" in
    *no-new-privileges*) ;;
    *) bad="$bad secopt=$secopt" ;;
esac
[ "$user" = "$SPEC_USER" ] || bad="$bad user=$user"
case "$pidmode" in ''|private) ;; *) bad="$bad pidmode=$pidmode" ;; esac
case "$ipcmode" in ''|private) ;; *) bad="$bad ipcmode=$ipcmode" ;; esac
case "$utsmode" in ''|private) ;; *) bad="$bad utsmode=$utsmode" ;; esac
case "$usernsmode" in ''|private) ;; *) bad="$bad usernsmode=$usernsmode" ;; esac
[ "$devices" = 0 ] || bad="$bad devices=$devices"
[ "$tmpfs" = 0 ] || bad="$bad tmpfs=$tmpfs"

if [ -n "$bad" ]; then
    echo "contêiner $ctr diverge do HostConfig esperado:$bad"
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

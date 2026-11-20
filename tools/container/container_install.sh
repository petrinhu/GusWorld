#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Builds one image by install-and-commit, under a hard resource limit (L-11 global,
# item 2(b), decided by the leader on 20/11/2026: "Contêiner limitado + commit").
#   docker run --memory 4g --memory-swap 4g --pids-limit 233 --cpus 4
#              --security-opt no-new-privileges <base@digest>
# The context arrives as a tar on stdin. The container runs install_toolchain.sh,
# then docker commit writes the tag with the USER, WORKDIR and LABEL changes.
# The network is open ONLY in this install step. The temporary container is
# removed by a trap on EXIT. Names carry the PID; leftovers from an earlier run
# with the same prefix are removed before the new run starts (A11).
# Usage: container_install.sh <base@digest> <base digest> <VERSION_ID> <tag>
set -euo pipefail

base_pinned="${1:?base@digest}"
base_digest="${2:?base digest}"
version_id="${3:?VERSION_ID}"
tag="${4:?tag}"

here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
name_prefix="gusworld-ci-build-"
name="${name_prefix}$$"

leftover_ids="$(docker ps -aq --filter "name=^${name_prefix}")"
if [ -n "$leftover_ids" ]; then
    echo "container_install: removing leftovers from earlier runs" >&2
    docker rm -f $leftover_ids >/dev/null
fi

trap 'docker rm -f "$name" >/dev/null 2>&1 || true' EXIT

"$here/context_tar.sh" | docker run -i --name "$name" \
    --memory 4g --memory-swap 4g --pids-limit 233 --cpus 4 \
    --security-opt no-new-privileges \
    -e "BASE_VERSION_ID=${version_id}" \
    "$base_pinned" \
    bash -c 'command -v tar >/dev/null 2>&1 || dnf -y install tar; mkdir -p /opt/gusworld-ci && tar -x -C /opt/gusworld-ci && bash /opt/gusworld-ci/install_toolchain.sh'

docker commit \
    --change 'USER builder' \
    --change 'WORKDIR /home/builder' \
    --change "LABEL org.gusworld.base.digest=${base_digest}" \
    --change "LABEL org.gusworld.base.version_id=${version_id}" \
    "$name" "$tag" >/dev/null

echo "container_install: committed ${tag} from ${name}"

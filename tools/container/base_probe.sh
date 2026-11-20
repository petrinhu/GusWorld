#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Pulls the base image and prints what the build must pin. The digest comes from
# the "Digest:" line of the pull output (A8), and the base is then addressed by
# that digest. VERSION_ID is read through a created container that is never
# started, so no code of the base runs.
# Guard first (A1): rebuild_authorized.sh must pass before anything is pulled.
# Output on stdout, one key=value per line:
#   BASE_PINNED=fedora@sha256:...   BASE_DIGEST=sha256:...   BASE_VERSION_ID=45
set -euo pipefail

here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
"$here/rebuild_authorized.sh"

base_ref="${BASE_REF:-fedora:latest}"
base_repo="${base_ref%%:*}"

pull_out="$(docker pull "$base_ref" 2>&1)" || { echo "$pull_out" >&2; exit 1; }
echo "$pull_out" >&2

digest="$(printf '%s\n' "$pull_out" | sed -n 's/^Digest: //p' | head -n 1)"
[[ "$digest" =~ ^sha256:[0-9a-f]{64}$ ]] || { echo "base_probe: no Digest line in the pull output" >&2; exit 1; }
pinned="${base_repo}@${digest}"

probe="gusworld-base-probe-$$"
docker create --name "$probe" "$pinned" >/dev/null
trap 'docker rm -f "$probe" >/dev/null 2>&1 || true' EXIT

# docker cp -L follows the /etc/os-release symlink and copies the real file.
version_id="$(docker cp -L "$probe:/etc/os-release" - | tar -xO os-release \
    | sed -n 's/^VERSION_ID=//p' | tr -d '"')"
[ -n "$version_id" ] || { echo "base_probe: no VERSION_ID in $pinned" >&2; exit 1; }

echo "BASE_PINNED=${pinned}"
echo "BASE_DIGEST=${digest}"
echo "BASE_VERSION_ID=${version_id}"

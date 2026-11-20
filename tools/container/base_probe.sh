#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Pulls the base image and prints what the build must pin: the base reference
# with its digest, and the VERSION_ID of that base. The base is only inspected
# through a created container that is never started, so no code of the base runs.
# Output on stdout, one key=value per line:
#   BASE_PINNED=fedora@sha256:...   BASE_DIGEST=sha256:...   BASE_VERSION_ID=45
set -euo pipefail

base_ref="${BASE_REF:-fedora:latest}"
base_repo="${base_ref%%:*}"

docker pull "$base_ref" >&2

digest="$(docker image inspect --format '{{range .RepoDigests}}{{println .}}{{end}}' "$base_ref" \
    | sed -n "s/^${base_repo}@//p" | head -n 1)"
[ -n "$digest" ] || { echo "base_probe: no repo digest for $base_ref" >&2; exit 1; }

probe="gusworld-base-probe-$$"
docker create --name "$probe" "$base_ref" >/dev/null
trap 'docker rm -f "$probe" >/dev/null 2>&1 || true' EXIT

# docker cp -L follows the /etc/os-release symlink and copies the real file.
version_id="$(docker cp -L "$probe:/etc/os-release" - | tar -xO os-release \
    | sed -n 's/^VERSION_ID=//p' | tr -d '"')"
[ -n "$version_id" ] || { echo "base_probe: no VERSION_ID in $base_ref" >&2; exit 1; }

echo "BASE_PINNED=${base_repo}@${digest}"
echo "BASE_DIGEST=${digest}"
echo "BASE_VERSION_ID=${version_id}"

#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Prints the facts of a built image without starting it: id, size, the base
# labels, and the rpm list that the Containerfile wrote at build time.
# Uses docker create, docker cp and docker rm only.
set -euo pipefail

tag="${1:?usage: report_image.sh <tag>}"
probe="gusworld-ci-report-$$"

docker create --name "$probe" "$tag" >/dev/null
trap 'docker rm -f "$probe" >/dev/null 2>&1 || true' EXIT

docker image inspect --format 'tag_id={{.Id}}
size_bytes={{.Size}}
base_digest={{index .Config.Labels "org.gusworld.base.digest"}}
base_version_id={{index .Config.Labels "org.gusworld.base.version_id"}}' "$tag"

echo "rpm -q (written at build time):"
docker cp "$probe:/etc/gusworld/packages.txt" - | tar -xO packages.txt

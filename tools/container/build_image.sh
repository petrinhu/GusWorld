#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Builds gusworld-ci-fedora:<sha12> and nothing else. Each step is a script:
#   1. resource_gate.sh  measures memory, load and pids now (L-11 global)
#   2. heavy_slot.sh     takes one of three heavy-work locks (L-11 global)
#   3. base_probe.sh     pulls the base, pins its digest, reads its VERSION_ID
#   4. env_hash.sh       the single source of the tag
#   5. context_tar.sh    streams the four context files to docker build
# The full build log goes to BUILD_LOG (default: a new file in /var/tmp), so a
# failed build can be shown later. The log is evidence; keep it.
set -euo pipefail

here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
export TMPDIR="${TMPDIR:-/var/tmp}"
started="$(date +%s)"

if ! "$here/resource_gate.sh"; then
    echo "build_image: ABORTED, the resource gate refused; nothing was built" >&2
    exit 1
fi

# shellcheck source=heavy_slot.sh
. "$here/heavy_slot.sh"
take_heavy_slot

probe="$("$here/base_probe.sh")"
base_pinned="$(printf '%s\n' "$probe" | sed -n 's/^BASE_PINNED=//p')"
base_digest="$(printf '%s\n' "$probe" | sed -n 's/^BASE_DIGEST=//p')"
base_version_id="$(printf '%s\n' "$probe" | sed -n 's/^BASE_VERSION_ID=//p')"
echo "build_image: base ${base_pinned}, VERSION_ID ${base_version_id}" >&2

hash="$("$here/env_hash.sh")"
tag="gusworld-ci-fedora:${hash:0:12}"
log="${BUILD_LOG:-$(mktemp /var/tmp/gusworld-ci-fedora-build-XXXXXX.log)}"
echo "build_image: tag ${tag}, log ${log}" >&2

if "$here/context_tar.sh" | nice -n 5 docker build --progress=plain --pull \
        -t "$tag" \
        --build-arg "BASE_IMAGE=${base_pinned}" \
        --build-arg "BASE_DIGEST=${base_digest}" \
        --build-arg "BASE_VERSION_ID=${base_version_id}" \
        -f Containerfile.fedora - > "$log" 2>&1; then
    echo "build_image: build OK" >&2
else
    status=$?
    echo "build_image: BUILD FAILED, status ${status}, log ${log}" >&2
    exit "$status"
fi

"$here/report_image.sh" "$tag"
echo "build_image: wall time $(( $(date +%s) - started )) s"

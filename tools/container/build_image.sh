#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Builds gusworld-ci-fedora:<sha12> and nothing else. Steps, in this order:
#   1. rebuild_authorized.sh  the committed leader authorization for today (A1)
#   2. heavy_slot.sh          takes one of three heavy-work locks (L-11 global)
#   3. resource_gate.sh       measures memory, load and pids, AFTER the lock (A5)
#   4. base_probe.sh          pulls the base, pins its digest from the pull output
#   5. env_hash.sh            the tag: context files plus the base digest (A4)
#   6. refuses to overwrite an existing tag (A4)
#   7. container_install.sh   docker run under --memory/--pids-limit, then commit
# Limit of L-11 global item 2(b): enforced by container_install.sh, with
# --memory 4g --pids-limit 233 --cpus 4, decided by the leader on 20/11/2026.
# The full log goes to BUILD_LOG (default: a new file in /var/tmp); keep it.
set -euo pipefail

here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
export TMPDIR="${TMPDIR:-/var/tmp}"

"$here/rebuild_authorized.sh"

started="$(date +%s)"

# shellcheck source=heavy_slot.sh
. "$here/heavy_slot.sh"
take_heavy_slot

if ! "$here/resource_gate.sh"; then
    echo "build_image: ABORTED, the resource gate refused; nothing was built" >&2
    exit 1
fi

probe="$("$here/base_probe.sh")"
base_pinned="$(printf '%s\n' "$probe" | sed -n 's/^BASE_PINNED=//p')"
base_digest="$(printf '%s\n' "$probe" | sed -n 's/^BASE_DIGEST=//p')"
base_version_id="$(printf '%s\n' "$probe" | sed -n 's/^BASE_VERSION_ID=//p')"
echo "build_image: base ${base_pinned}, VERSION_ID ${base_version_id}" >&2

hash="$("$here/env_hash.sh" "$base_digest")"
tag="gusworld-ci-fedora:${hash:0:12}"
if docker image inspect "$tag" >/dev/null 2>&1; then
    echo "build_image: REFUSED, tag ${tag} already exists; it is never overwritten" >&2
    exit 1
fi

log="${BUILD_LOG:-$(mktemp /var/tmp/gusworld-ci-fedora-build-XXXXXX.log)}"
echo "build_image: tag ${tag}, log ${log}" >&2

if nice -n 5 "$here/container_install.sh" "$base_pinned" "$base_digest" "$base_version_id" "$tag" > "$log" 2>&1; then
    echo "build_image: build OK" >&2
else
    status=$?
    echo "build_image: BUILD FAILED, status ${status}, log ${log}" >&2
    exit "$status"
fi

"$here/report_image.sh" "$tag"
echo "build_image: wall time $(( $(date +%s) - started )) s"

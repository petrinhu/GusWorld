#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Host side of the invoker tests. It only launches: the tests run in a fresh
# container of the image (L-35). The tree goes in as a tar file, written first and
# fed by "<", so a failed tar stops here. No volume is mounted (ADR-026, decision 7).
set -euo pipefail

here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"
root="$(cd "$here/../../.." && pwd -P)"
hash="$("$root/tools/container/env_hash.sh")"
tag="gusworld-ci-fedora:${hash:0:12}"

out_dir=/var/tmp/gusworld-tdd-test
mkdir -p "$out_dir"
tar_file="$out_dir/test-tree.tar"
(cd "$root" && tar -c -f "$tar_file" tools/tdd tools/container tools/ci/env tools/ci/cmake_floor.sh)
tar -t -f "$tar_file" > /dev/null

docker run --rm -i --pull never \
    --network none \
    --memory 2g --pids-limit 233 --cpus 4 \
    --cap-drop ALL --security-opt no-new-privileges \
    --user builder \
    "$tag" \
    bash -c 'd="$(mktemp -d)" && tar -x -f - -C "$d" && exec bash "$d/tools/tdd/test/run_tests.sh"' \
    < "$tar_file"

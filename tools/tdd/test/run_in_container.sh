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
# The image is the local one whose label digest matches the current context, the same
# choice the guard makes (image_select.sh). No rebuild happens here.
image="$("$root/tools/tdd/image_select.sh" "$root" | sed -n 's/^IMAGE_REF=//p')"
[ -n "$image" ] || { echo "run_in_container: nenhuma imagem atual, rode build_image.sh" >&2; exit 1; }

out_dir=/var/tmp/gusworld-tdd-test
mkdir -p "$out_dir"
tar_file="$out_dir/test-tree.tar"
(cd "$root" && tar -c -f "$tar_file" tools/tdd tools/container tools/ci/env tools/ci/cmake_floor.sh .claude/settings.json)
tar -t -f "$tar_file" > /dev/null

docker run --rm -i --pull never \
    --network none \
    --memory 2g --pids-limit 233 --cpus 4 \
    --cap-drop ALL --security-opt no-new-privileges \
    --user builder \
    "$image" \
    bash -c 'd="$(mktemp -d)" && tar -x -f - -C "$d" && exec bash "$d/tools/tdd/test/run_tests.sh"' \
    < "$tar_file"

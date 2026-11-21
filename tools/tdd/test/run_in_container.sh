#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Host side of the invoker tests. It only launches: the tests run in a fresh
# container of the image (GW-35). The tree goes in as a tar file, written first and
# fed by "<", so a failed tar stops here. No volume is mounted (ADR-026, decision 7).
set -euo pipefail

here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"
root="$(cd "$here/../../.." && pwd -P)"
# The image is the local one whose label digest matches the current context, the same
# choice the guard makes (image_select.sh). No rebuild happens here.
image="$("$root/tools/tdd/image_select.sh" "$root" | sed -n 's/^IMAGE_REF=//p')"
[ -n "$image" ] || { echo "run_in_container: nenhuma imagem atual, rode build_image.sh" >&2; exit 1; }

# A private, fresh work dir per run: mktemp creates it with a random name and mode 700.
# No fixed path under /var/tmp, so no symlink planted there can redirect the copies or
# the cleanup. The cleanup is confined to the exact variable, with ${out_dir:?}.
out_dir="$(mktemp -d "${XDG_RUNTIME_DIR:-/tmp}/gusworld-tdd-test-XXXXXX")"
trap 'rm -rf -- "${out_dir:?}"' EXIT
tar_file="$out_dir/test-tree.tar"
# The bigtech plugin's hooks travel as a copy under plugin/hooks (the contract test loads
# the real tdd_guard from there, GW-35). The copy comes from the SAME resolver the notice
# uses (review F2: marketplace directory first, installPath as fallback), so the tests run
# the plugin Claude Code runs, not a stale cache. Nothing under ~/.claude is written.
plugin_src="$(python3 -B -c 'import sys; sys.path.insert(0, sys.argv[1]); import notice; print(notice.plugin_hooks_dir())' "$root/tools/tdd")" \
    || { echo "run_in_container: plugin nao resolvido pelo notice.py" >&2; exit 1; }
mkdir -p "$out_dir/plugin"
cp -a "$plugin_src" "$out_dir/plugin/hooks"
# The log names the exact bytes that enter the container (sha256 of each .py copied).
sha256sum "$out_dir"/plugin/hooks/*.py | sed 's/^/run_in_container: plugin sha256 /' >&2
(cd "$root" && tar -c -f "$tar_file" tools/tdd tools/container tools/ci/env tools/ci/cmake_floor.sh .claude/settings.json .claude/tdd-guard.json -C "$out_dir" plugin)
tar -t -f "$tar_file" > /dev/null

# The pids limit of the test container (C8 fatia 7). The default is 60. GUSWORLD_TEST_PIDS
# can only TIGHTEN it: an integer from 1 to 60. Any other value keeps the default.
pids_limit=60
case "${GUSWORLD_TEST_PIDS:-}" in
    ""|*[!0-9]*) ;;
    *)
        if [ "$GUSWORLD_TEST_PIDS" -ge 1 ] && [ "$GUSWORLD_TEST_PIDS" -le 60 ]; then
            pids_limit="$GUSWORLD_TEST_PIDS"
        fi
        ;;
esac
# The resource gate of the L-11 global limits, before each container (12 GiB free).
GATE_MIN_MEM_KB=12582912 "$root/tools/container/resource_gate.sh" >/dev/null 2>&1 \
    || { echo "run_in_container: portao de recurso reprovou (GATE_MIN_MEM_KB=12582912)" >&2; exit 1; }
echo "run_in_container: pids-limit $pids_limit" >&2
name="gusworld-tdd-test-$$"
echo "run_in_container: contêiner $name" >&2

docker run --rm -i --pull never --name "$name" \
    --network none \
    --memory 2g --pids-limit "$pids_limit" --cpus 4 \
    --cap-drop ALL --security-opt no-new-privileges \
    --user builder \
    "$image" \
    bash -c 'd="$(mktemp -d)" && tar -x -f - -C "$d" && exec bash "$d/tools/tdd/test/run_tests.sh"' \
    < "$tar_file"

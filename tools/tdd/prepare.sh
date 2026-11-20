#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Warms the long-lived test container gusworld-tdd-<hash16> (ADR-026, decisions 3 and 6),
# so the guard answers from a warm cache. Its work, in order:
#   1. docker and the image: docker_ready.sh and image_select.sh, refused when either fails.
#   2. the resource gate of the L-11 global limits, before any heavy step.
#   3. a heavy-work slot (tools/container/heavy_slot.sh), held until this script exits.
#   4. the container. A missing one is created with the hardening of decision 7, and
#      started. An existing one is reused only when container_ready.sh approves it;
#      otherwise it is refused with that reason. It is never removed nor replaced.
#   5. /work, when absent: docker cp puts it in, owned by the builder. Root never runs
#      inside the container.
#   6. the tree: build_list.sh and make_tar.sh, then in_container.sh inline. On a cold
#      cache it answers 3 after extracting the tree and writing the change manifest:
#      that is the expected answer here. On a warm cache it answers 0 or 1, and it is done.
#   7. cold only: cmake -S /work/src -B /work/build -G Ninja, then one build, timed as
#      information. Four jobs, the --cpus quota, so the build fits the 4 GiB limit.
# The container is left running: stopping it is the caller's decision.
# usage: prepare.sh [<repo_root>]
set -uo pipefail

here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"
root="${1:-$(cd "$here/../.." && pwd -P)}"
h16="$(printf '%s' "$root" | sha256sum | cut -c1-16)"
ctr="gusworld-tdd-$h16"
state="/var/tmp/gusworld-tdd/$h16"
list="$state/prepare.list"
tar_file="$state/prepare.tar"
seed="$state/seed"
work_tar="$state/work.tar"
cold_jobs=4

refuse() {
    echo "prepare: recusado, $1" >&2
    exit 1
}

# detail_of TEXT: the last "tdd:" line of an in_container.sh run, which says why it failed.
detail_of() {
    printf '%s\n' "$1" | sed -n 's/^tdd: //p' | sed -n '$p'
}

mkdir -p "$state" || refuse "estado $state inacessivel"

msg="$("$here/docker_ready.sh" 2>&1)" || refuse "docker: $msg"

msg="$("$root/tools/container/resource_gate.sh" 2>&1)" || refuse "portao de recurso reprovou: $msg"

sel="$("$here/image_select.sh" "$root" 2>&1)" || refuse "$sel"
image_id="$(printf '%s\n' "$sel" | sed -n 's/^IMAGE_ID=//p')"
[ -n "$image_id" ] || refuse "selecao de imagem sem id"

# shellcheck source=../container/heavy_slot.sh
. "$root/tools/container/heavy_slot.sh" || refuse "heavy_slot.sh ilegivel"
take_heavy_slot || refuse "slots de trabalho pesado ocupados"

if timeout 20 docker container inspect --format '{{.Id}}' "$ctr" >/dev/null 2>&1; then
    msg="$("$here/container_ready.sh" "$ctr" "$image_id" 2>&1)" \
        || refuse "contêiner $ctr existente reprovado: $msg"
    echo "prepare: contêiner $ctr reaproveitado, aprovado pelo container_ready"
else
    timeout 60 docker create --name "$ctr" --pull never --network none \
        --cap-drop ALL --security-opt no-new-privileges --user builder \
        --memory 4g --memory-swap 4g --pids-limit 233 --cpus 4 \
        "$image_id" sleep infinity >/dev/null 2>&1 || refuse "docker create falhou para $ctr"
    timeout 60 docker start "$ctr" >/dev/null 2>&1 || refuse "docker start falhou para $ctr"
    echo "prepare: contêiner $ctr criado da imagem $image_id"
fi

# ensure_work: /work exists and the builder can write in it. Its owner is the uid that
# the container reports for the builder, never a number typed here.
ensure_work() {
    if timeout 20 docker exec "$ctr" test -d /work >/dev/null 2>&1; then
        return 0
    fi
    local uid gid
    uid="$(timeout 20 docker exec "$ctr" id -u)" || refuse "id -u no contêiner falhou"
    gid="$(timeout 20 docker exec "$ctr" id -g)" || refuse "id -g no contêiner falhou"
    mkdir -p "$seed/work" || refuse "seed de /work inacessivel"
    tar --owner="$uid" --group="$gid" -C "$seed" -cf "$work_tar" work || refuse "tar de /work falhou"
    timeout 60 docker cp - "$ctr:/" < "$work_tar" || refuse "docker cp de /work falhou"
    timeout 20 docker exec "$ctr" test -w /work >/dev/null 2>&1 \
        || refuse "/work sem escrita para o builder"
}
ensure_work

cache_before=no
if timeout 20 docker exec "$ctr" test -f /work/build/CMakeCache.txt >/dev/null 2>&1; then
    cache_before=yes
fi

"$here/build_list.sh" "$root" "$list" || refuse "lista da arvore falhou"
timeout --kill-after=5 123 "$here/make_tar.sh" "$root" "$list" "$tar_file" \
    || refuse "tar da arvore falhou"

script="$(cat "$here/in_container.sh")" || refuse "in_container.sh ilegivel"
warm_out="$(timeout --kill-after=5 123 docker exec -i "$ctr" bash -c "$script" < "$tar_file" 2>&1)"
verdict="$(printf '%s\n' "$warm_out" | sed -n 's/^tdd-verdict: //p' | sed -n '$p')"
det="$(detail_of "$warm_out")"

if [ "$cache_before" = no ]; then
    [ "$verdict" = 3 ] || refuse "aquecimento frio devolveu veredito ${verdict:-nenhum}, esperado 3${det:+: $det}"

    cfg_out="$(timeout 300 docker exec "$ctr" cmake -S /work/src -B /work/build -G Ninja 2>&1)" || {
        printf '%s\n' "$cfg_out" >&2
        refuse "configuracao (cmake -S) falhou"
    }

    start_build="$(date +%s)"
    build_out="$(timeout 1800 docker exec "$ctr" cmake --build /work/build --parallel "$cold_jobs" 2>&1)" || {
        printf '%s\n' "$build_out" >&2
        refuse "build frio falhou"
    }
    echo "prepare: build frio em $(( $(date +%s) - start_build )) s, medido como informacao"
else
    case "$verdict" in
        0|1) ;;
        *) refuse "aquecimento com cache devolveu veredito ${verdict:-nenhum}${det:+: $det}" ;;
    esac
fi

echo "prepare: pronto, contêiner $ctr aquecido (veredito ${verdict})"

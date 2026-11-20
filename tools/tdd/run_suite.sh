#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# test_command of the TDD guard (ADR-026). Runs the suite in the long-lived container
# gusworld-tdd-<hash16> and exits with exactly one verdict: 0 green, 1 red, or 126
# no run. Every other outcome is 126, so the guard never reads an infrastructure
# failure as red: no docker, gate refused, no image, cold container, a docker exec
# without a matching verdict line, container exit 3, or a signal.
# Each run overwrites last-reason, green included.
set -u

here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"
root="$(cd "$here/../.." && pwd -P)"
h16="$(printf '%s' "$root" | sha256sum | cut -c1-16)"
ctr="gusworld-tdd-$h16"
state="/var/tmp/gusworld-tdd/$h16"
reason_file="$state/last-reason"
log="$state/last.log"
list="$state/src.list"
tar_file="$state/src.tar"

mkdir -p "$state" || exit 126
: > "$log" || exit 126

record() {
    printf '%s\n' "$1" > "$reason_file"
}

refuse() {
    record "sem execucao: $1"
    echo "tdd run_suite: sem execucao, $1" >&2
    exit 126
}

msg="$("$here/docker_ready.sh" 2>&1)" || refuse "docker: $msg"

msg="$("$root/tools/container/resource_gate.sh" 2>&1)" || refuse "portao de recurso reprovou: $msg"

hash="$("$root/tools/container/env_hash.sh" 2>/dev/null)" || refuse "env_hash falhou"
tag="gusworld-ci-fedora:${hash:0:12}"
image_id="$(timeout 20 docker image inspect --format '{{.Id}}' "$tag" 2>/dev/null)"
[ -n "$image_id" ] || refuse "imagem $tag ausente, rode tools/container/build_image.sh"

msg="$("$here/container_ready.sh" "$ctr" "$image_id" 2>&1)" || refuse "$msg"

msg="$("$here/build_list.sh" "$root" "$list" 2>&1)" || refuse "lista da arvore: $msg"
msg="$("$here/make_tar.sh" "$root" "$list" "$tar_file" 2>&1)" || refuse "tar da arvore: $msg"

# The script goes inline, so it is the host version. The tar is the stdin.
script="$(cat "$here/in_container.sh")" || refuse "in_container.sh ilegivel"
docker exec -i "$ctr" bash -c "$script" < "$tar_file" > "$log" 2>&1
rc=$?

# docker exec exits 1 by itself when the container vanished. Only a verdict line
# that matches the exit code, with 0 or 1, counts as an answer.
verdict="$(sed -n 's/^tdd-verdict: //p' "$log" | sed -n '$p')"
case "$verdict" in
    0|1|3) ;;
    *) refuse "contêiner sem veredito (docker exec rc $rc)" ;;
esac
[ "$verdict" = "$rc" ] || refuse "veredito $verdict diverge do rc $rc"

code="$("$here/map_exit.sh" "$rc")"
case "$code" in
    0) record "verde: suite passou (veredito 0)" ;;
    1) record "vermelho: build ou testes falharam (veredito 1)" ;;
    *) refuse "infraestrutura, veredito $rc" ;;
esac

cat "$log"
exit "$code"

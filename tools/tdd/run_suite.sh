# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# test_command of the TDD guard (ADR-026). Runs the suite in the long-lived container
# gusworld-tdd-<hash16> and exits with exactly one verdict: 0 green, 1 red, or 126
# no run. Every other outcome is 126, so the guard never reads an infrastructure
# failure as red: no docker, gate refused, no current image, cold or foreign container,
# a docker exec without a matching verdict line, container exit 3, a bounded timeout,
# or a signal to this script.
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

# The state directory must be private to this uid, not a link, not writable by others
# (state_dir.sh, review of the notice.py finding). Anything else is no run: 126.
"$here/state_dir.sh" "$state" || exit 126

# No state file may be a link: a write through a link would reach its target. Checked
# before any write, so a planted link refuses the run and touches nothing.
for f in "$reason_file" "$log" "$list" "$tar_file"; do
    if [ -L "$f" ]; then
        echo "tdd run_suite: sem execucao, $f e link simbolico, recusado" >&2
        exit 126
    fi
done

: > "$log" || exit 126

record() {
    printf '%s\n' "$1" > "$reason_file"
}

refuse() {
    record "sem execucao: $1"
    echo "tdd run_suite: sem execucao, $1" >&2
    exit 126
}

# A signal is a "no run" too. It must not leave with 143 (the signal's own status),
# which the guard would read as red. The docker exec child is stopped with it.
exec_pid=""
on_signal() {
    if [ -n "$exec_pid" ]; then
        kill "$exec_pid" 2>/dev/null
    fi
    refuse "sinal recebido durante a execucao"
}
trap on_signal TERM INT HUP

msg="$("$here/docker_ready.sh" 2>&1)" || refuse "docker: $msg"

msg="$("$root/tools/container/resource_gate.sh" 2>&1)" || refuse "portao de recurso reprovou: $msg"

# The image is the local one whose label digest matches the current context (MÉDIA-8).
sel="$("$here/image_select.sh" "$root" 2>&1)" || refuse "$sel"
image_ref="$(printf '%s\n' "$sel" | sed -n 's/^IMAGE_REF=//p')"
image_id="$(printf '%s\n' "$sel" | sed -n 's/^IMAGE_ID=//p')"
[ -n "$image_id" ] || refuse "seleção de imagem sem id"

msg="$("$here/container_ready.sh" "$ctr" "$image_id" 2>&1)" || refuse "$msg"

msg="$("$here/build_list.sh" "$root" "$list" 2>&1)" || refuse "lista da arvore: $msg"

# Bounded by 123 s with a 5 s kill grace (MÉDIA-10). A timeout is a no run.
msg="$(timeout --kill-after=5 123 "$here/make_tar.sh" "$root" "$list" "$tar_file" 2>&1)"
rc=$?
case "$rc" in
    0) ;;
    124|137) refuse "prazo estourado ao montar o tar" ;;
    *) refuse "tar da arvore: $msg" ;;
esac

# The script goes inline, so it is the host version. The tar is the stdin. The host
# budget, when set, reaches the container: in_container.sh only lets it shorten the run.
exec_env=()
if [ -n "${GUSWORLD_TDD_BUDGET:-}" ]; then
    exec_env=(-e "GUSWORLD_TDD_BUDGET=$GUSWORLD_TDD_BUDGET")
fi
script="$(cat "$here/in_container.sh")" || refuse "in_container.sh ilegivel"
timeout --kill-after=5 123 docker exec -i "${exec_env[@]}" "$ctr" bash -c "$script" < "$tar_file" > "$log" 2>&1 &
exec_pid=$!
wait "$exec_pid"
rc=$?
exec_pid=""
case "$rc" in
    124|137) refuse "prazo estourado no docker exec" ;;
esac

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
    *)
        # The last "tdd:" line of the container says why: deadline, cold cache, lock.
        detail="$(sed -n 's/^tdd: //p' "$log" | sed -n '$p')"
        refuse "infraestrutura, veredito $rc${detail:+: $detail}"
        ;;
esac

cat "$log"
exit "$code"

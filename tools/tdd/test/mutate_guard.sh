#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Mutation proof of the TDD guard (ADR-026, C8 fatia 6, L-19 N2, L-27). Host side: it
# only prepares trees and launches one fresh container per case. Each tree comes from the
# COMMITTED HEAD (git archive), gets at most one mutant (mutants.py), and is checked by
# hash: the blob of HEAD must differ from the mutated file, and the tar that enters the
# container must carry the mutated bytes. The live tree is never edited. The tests run
# in the container (L-35), without volume and without network, under 80 pids and 2 GiB.
# usage: mutate_guard.sh baseline M1 [M2 ... M7]
# M8 (undo a fix of R2) is not applicable: R2 has no fix yet.
set -u

here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"
root="$(cd "$here/../../.." && pwd -P)"
suites="tools/tdd/test/test_guard_proof.sh tools/tdd/test/test_notice_settings.sh tools/tdd/test/test_notice_contract.sh"

if [ -n "$(git -C "$root" status --porcelain -- tools/tdd .claude/settings.json)" ]; then
    echo "recusado: tools/tdd ou .claude/settings.json com alteracao nao commitada (L-27)" >&2
    exit 1
fi
head="$(git -C "$root" rev-parse HEAD)" || exit 1
work="$(mktemp -d "${XDG_RUNTIME_DIR:-/tmp}/gusworld-mut-XXXXXX")" || exit 1
trap 'rm -rf -- "${work:?}"' EXIT
plugin_src="$HOME/.claude/plugins/cache/petrinhu/bigtech/0.2.0/hooks"

# run_case ID: one tree, one container, one log. Returns non-zero on any refusal.
run_case() {
    local id="$1" tree="$work/$1" tar_file="$work/$1.tar" log="$work/$1.log" rel
    mkdir -p "$tree" || return 1
    git -C "$root" archive "$head" tools/tdd .claude/settings.json | tar -x -C "$tree" || return 1
    mkdir -p "$tree/plugin" && cp -a "$plugin_src" "$tree/plugin/hooks" || return 1
    if [ "$id" != baseline ]; then
        python3 -I "$here/mutants.py" apply "$tree" "$id" || { echo "caso $id: mutante nao aplicado" >&2; return 1; }
    fi
    # Blob check: the baseline copy equals HEAD, and a mutant copy differs from HEAD in
    # exactly one of the two files (the one the mutant names).
    local differs=0
    for rel in tools/tdd/notice.py .claude/settings.json; do
        local head_blob copy_blob
        head_blob="$(git -C "$root" rev-parse "$head:$rel")" || return 1
        copy_blob="$(git hash-object -- "$tree/$rel")" || return 1
        [ "$head_blob" = "$copy_blob" ] || differs=$((differs + 1))
    done
    if [ "$id" = baseline ] && [ "$differs" -ne 0 ]; then
        echo "caso baseline: copia diverge do blob de HEAD" >&2; return 1
    fi
    if [ "$id" != baseline ] && [ "$differs" -ne 1 ]; then
        echo "caso $id: $differs arquivo(s) diferem do HEAD, esperado 1" >&2; return 1
    fi
    tar -C "$tree" -cf "$tar_file" . || return 1
    # The bytes that enter the container are the bytes of the copy.
    for rel in tools/tdd/notice.py .claude/settings.json; do
        if [ "$(tar -xOf "$tar_file" "./$rel" | sha256sum)" != "$(sha256sum < "$tree/$rel")" ]; then
            echo "caso $id: o tar nao carrega o arquivo mutado ($rel)" >&2; return 1
        fi
    done
    GATE_MIN_MEM_KB=12582912 "$root/tools/container/resource_gate.sh" >/dev/null 2>&1 \
        || { echo "caso $id: portao de recurso reprovou (GATE_MIN_MEM_KB=12582912)" >&2; return 1; }
    local sel image
    sel="$("$root/tools/tdd/image_select.sh" "$root")" || { echo "caso $id: imagem defasada" >&2; return 1; }
    image="$(printf '%s\n' "$sel" | sed -n 's/^IMAGE_REF=//p')"
    [ -n "$image" ] || { echo "caso $id: sem IMAGE_REF" >&2; return 1; }
    local cname="gusworld-mut-$id-$$"
    docker run --rm -i --pull never --name "$cname" \
        --network none \
        --memory 2g --memory-swap 2g --pids-limit 80 \
        --cap-drop ALL --security-opt no-new-privileges \
        --user builder \
        "$image" \
        bash -c 'd="$(mktemp -d)" && tar -x -f - -C "$d" && cd "$d" && for t in '"$suites"'; do echo "== $t"; bash "$t" || echo "FALHA arquivo $t"; done' \
        < "$tar_file" > "$log" 2>&1 &
    local run_pid=$!
    sample_pids "$cname" "$work/$id.pids"
    wait "$run_pid"
    echo "caso $id: contêiner saiu com $?, log em $log"
    echo "caso $id: pids $(cat "$work/$id.pids")"
    return 0
}

# sample_pids NAME OUT: while the container runs, keeps the highest pids.peak and the last
# pids.events read from its cgroup v2 (found through /proc/<pid>/cgroup). The cgroup
# vanishes with the container, so the last read before that is the final one.
sample_pids() {
    local name="$1" out="$2" pid cg dir peak=0 events="(nao lido)" p seen=0 waited=0
    : > "$out"
    while :; do
        pid="$(docker inspect -f '{{.State.Pid}}' "$name" 2>/dev/null)" || {
            # Not started yet: wait up to 30 s for it. Once seen, its absence means it ended.
            if [ "$seen" -eq 1 ] || [ "$waited" -ge 600 ]; then break; fi
            waited=$((waited + 1)); sleep 0.05; continue
        }
        seen=1
        if [ -n "$pid" ] && [ "$pid" != 0 ]; then
            cg="$(awk -F: '$1 == "0" { print $3 }' "/proc/$pid/cgroup" 2>/dev/null)"
            dir="/sys/fs/cgroup${cg}"
            if [ -r "$dir/pids.peak" ]; then
                p="$(cat "$dir/pids.peak")"
                [ "$p" -gt "$peak" ] 2>/dev/null && peak="$p"
                events="$(tr '\n' ' ' < "$dir/pids.events" 2>/dev/null)"
            fi
        fi
        sleep 0.05
    done
    printf 'pids.peak=%s pids.events=[%s] (limite 80)\n' "$peak" "$events" > "$out"
}

run_case baseline || { echo "linha de base nao executada: nada a comparar" >&2; exit 1; }
base_log="$work/baseline.log"
if ! grep -q '^casos=' "$base_log"; then
    echo "linha de base sem linhas casos=: inconclusiva" >&2
    exit 1
fi
echo "== linha de base (FALHA esperada no R2 enquanto o defeito existir)"
grep -E '^(ok|FALHA) ' "$base_log"

status=0
for id in "$@"; do
    if [ "$id" = baseline ]; then
        continue
    fi
    if ! run_case "$id"; then
        echo "caso $id: nao executado"
        status=1
        continue
    fi
    echo "== $id"
    python3 -I "$here/mutants.py" compare "$base_log" "$work/$id.log" 3
    rc=$?
    [ "$rc" -eq 0 ] || status=1
done
exit "$status"

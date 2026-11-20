# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# The long-lived container must have the hardening of ADR-026 (decision 7), with the
# exact values of container_spec.sh, the single source of the spec. The HostConfig is
# read BEFORE any docker start, so a divergent stopped container is never started
# (MÉDIA-7 and MÉDIA-2 of the review of 2026-11-20). Any divergence is a refusal.
. "$(dirname "${BASH_SOURCE[0]}")/lib.sh"

CR="$TDD_DIR/container_ready.sh"
SPEC="$TDD_DIR/container_spec.sh"
require_file "container_ready.sh existe" "$CR"
require_file "container_spec.sh existe" "$SPEC"
[ -f "$CR" ] && [ -f "$SPEC" ] || suite_done
. "$SPEC"

BIN="$(make_fakebin yes)"
export FAKE_DOCKER_LOG="$BIN/docker.log"
export FAKE_DOCKER_STDIN="$BIN/stdin.tar"
export PATH="$BIN:$PATH"
CTR="gusworld-tdd-0123456789abcdef"

# The HostConfig as container_ready.sh reads it, field by field (18 fields):
#  0 network  1 binds  2 mounts  3 privileged  4 memory  5 memswap  6 pids  7 nanocpus
#  8 capdrop  9 capadd  10 secopt  11 user  12 pidmode  13 ipcmode  14 utsmode
#  15 usernsmode  16 devices  17 tmpfs
HC_GOOD=(none 0 0 false "$SPEC_MEMORY_BYTES" "$SPEC_MEMORY_SWAP_BYTES" "$SPEC_PIDS" \
    "$SPEC_NANOCPUS" "[ALL]" 0 "[no-new-privileges]" builder "" private "" "" 0 0)

# hc_with INDEX VALUE: the good HostConfig with one field replaced, joined by '|'.
hc_with() {
    local -a a=("${HC_GOOD[@]}")
    a[$1]="$2"
    local IFS='|'
    printf '%s' "${a[*]}"
}

# check NAME WANT HOSTCONF [RUNNING]: container_ready.sh must answer WANT.
check() {
    expect_rc "$1" "$2" env FAKE_CTR_HOSTCONF="$3" FAKE_CTR_RUNNING="${4:-true}" \
        bash "$CR" "$CTR" sha256:imagemfake
}

GOOD="$(hc_with 0 none)"
check "configuracao endurecida passa" 0 "$GOOD"
check "rede liberada reprova" 1 "$(hc_with 0 host)"
check "bind de host reprova" 1 "$(hc_with 1 1)"
check "volume montado reprova" 1 "$(hc_with 2 1)"
check "privilegiado reprova" 1 "$(hc_with 3 true)"
check "sem limite de memoria reprova" 1 "$(hc_with 4 0)"
check "memoria de 64g reprova" 1 "$(hc_with 4 68719476736)"
check "swap de 64g reprova" 1 "$(hc_with 5 68719476736)"
check "pids errado reprova" 1 "$(hc_with 6 100)"
check "2 CPUs em vez de 4 reprova" 1 "$(hc_with 7 2000000000)"
check "capabilities nao descartadas reprovam" 1 "$(hc_with 8 "[]")"
check "CapAdd nao vazio reprova" 1 "$(hc_with 9 1)"
check "sem no-new-privileges reprova" 1 "$(hc_with 10 "[]")"
check "usuario root reprova" 1 "$(hc_with 11 root)"
check "PidMode host reprova" 1 "$(hc_with 12 host)"
check "IpcMode host reprova" 1 "$(hc_with 13 host)"
check "UTSMode host reprova" 1 "$(hc_with 14 host)"
check "UsernsMode host reprova" 1 "$(hc_with 15 host)"
check "PidMode private passa" 0 "$(hc_with 12 private)"
check "device montado reprova" 1 "$(hc_with 16 1)"
check "tmpfs montado reprova" 1 "$(hc_with 17 1)"
check "parado e endurecido: inicia" 0 "$GOOD" false

# A divergent container that is stopped is refused and never started.
: > "$FAKE_DOCKER_LOG"
check "parado e divergente reprova" 1 "$(hc_with 4 68719476736)" false
expect_eq "parado e divergente: nenhum docker start" 0 "$(grep -c '^start ' "$FAKE_DOCKER_LOG")"

# A good stopped container is started only after its HostConfig passed.
: > "$FAKE_DOCKER_LOG"
check "parado e endurecido: inicia depois da conferencia" 0 "$GOOD" false
expect_eq "parado e endurecido: um docker start" 1 "$(grep -c '^start ' "$FAKE_DOCKER_LOG")"

expect_rc "outra imagem reprova (imagem)" 1 env FAKE_CTR_HOSTCONF="$GOOD" bash "$CR" "$CTR" sha256:outra

suite_done

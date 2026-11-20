# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# The long-lived container must have the hardening of ADR-026 (decision 7): no
# network, no bind and no mount, not privileged, a memory limit, every capability
# dropped, no-new-privileges, and user builder. Any divergence is a refusal (MÉDIA-7).
. "$(dirname "${BASH_SOURCE[0]}")/lib.sh"

CR="$TDD_DIR/container_ready.sh"
require_file "container_ready.sh existe" "$CR"
[ -f "$CR" ] || suite_done

BIN="$(make_fakebin yes)"
export FAKE_DOCKER_LOG="$BIN/docker.log"
export FAKE_DOCKER_STDIN="$BIN/stdin.tar"
export PATH="$BIN:$PATH"
CTR="gusworld-tdd-0123456789abcdef"

# The expected HostConfig, as the template in container_ready.sh reads it:
# network|binds|mounts|privileged|memory|capdrop|secopt|user
GOOD="none|0|0|false|2147483648|[ALL]|[no-new-privileges]|builder"

expect_rc "configuracao endurecida passa" 0 env FAKE_CTR_HOSTCONF="$GOOD" bash "$CR" "$CTR" sha256:imagemfake
expect_rc "rede liberada reprova" 1 env FAKE_CTR_HOSTCONF="host|0|0|false|2147483648|[ALL]|[no-new-privileges]|builder" bash "$CR" "$CTR" sha256:imagemfake
expect_rc "bind de host reprova" 1 env FAKE_CTR_HOSTCONF="none|1|0|false|2147483648|[ALL]|[no-new-privileges]|builder" bash "$CR" "$CTR" sha256:imagemfake
expect_rc "volume montado reprova" 1 env FAKE_CTR_HOSTCONF="none|0|1|false|2147483648|[ALL]|[no-new-privileges]|builder" bash "$CR" "$CTR" sha256:imagemfake
expect_rc "privilegiado reprova" 1 env FAKE_CTR_HOSTCONF="none|0|0|true|2147483648|[ALL]|[no-new-privileges]|builder" bash "$CR" "$CTR" sha256:imagemfake
expect_rc "sem limite de memoria reprova" 1 env FAKE_CTR_HOSTCONF="none|0|0|false|0|[ALL]|[no-new-privileges]|builder" bash "$CR" "$CTR" sha256:imagemfake
expect_rc "capabilities nao descartadas reprovam" 1 env FAKE_CTR_HOSTCONF="none|0|0|false|2147483648|[]|[no-new-privileges]|builder" bash "$CR" "$CTR" sha256:imagemfake
expect_rc "sem no-new-privileges reprova" 1 env FAKE_CTR_HOSTCONF="none|0|0|false|2147483648|[ALL]|[]|builder" bash "$CR" "$CTR" sha256:imagemfake
expect_rc "usuario root reprova" 1 env FAKE_CTR_HOSTCONF="none|0|0|false|2147483648|[ALL]|[no-new-privileges]|root" bash "$CR" "$CTR" sha256:imagemfake
expect_rc "outra imagem reprova" 1 env FAKE_CTR_HOSTCONF="$GOOD" bash "$CR" "$CTR" sha256:outra

suite_done

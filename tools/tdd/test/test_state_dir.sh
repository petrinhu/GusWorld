# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# The state directory /var/tmp/gusworld-tdd/<hash16> is written by run_suite.sh and
# prepare.sh, and its name is predictable. Any local user can create that name first, so
# the state directory must be a real directory of the current uid with no write bit for
# group or others (review of the notice.py finding, 2026-11-20). A link is never a state
# file: writes through a symlink are refused, and the target is left alone.
. "$(dirname "${BASH_SOURCE[0]}")/lib.sh"

SD="$TDD_DIR/state_dir.sh"
require_file "state_dir.sh existe" "$SD"
[ -f "$SD" ] || suite_done

T="$(mktemp -d /tmp/tdd-sd-XXXXXX)"

# A. A missing directory is created private, with its parent, and accepted.
expect_rc "caminho novo e criado e aceito" 0 "$SD" "$T/novo"
expect_eq "caminho novo sai com modo 700" 700 "$(stat -c '%a' "$T/novo" 2>/dev/null)"
expect_rc "caminho novo aninhado e criado" 0 "$SD" "$T/base/estado"
expect_eq "pai criado com modo 700" 700 "$(stat -c '%a' "$T/base" 2>/dev/null)"

# B. A directory another local user can write to is refused.
mkdir -p "$T/aberto" && chmod 777 "$T/aberto"
expect_rc "diretorio 0777 reprova" 1 "$SD" "$T/aberto"
mkdir -p "$T/grupo" && chmod 770 "$T/grupo"
expect_rc "diretorio 0770 reprova (escrita de grupo)" 1 "$SD" "$T/grupo"
mkdir -p "$T/sticky" && chmod 1777 "$T/sticky"
expect_rc "diretorio 1777 reprova (sticky, escrita para todos)" 1 "$SD" "$T/sticky"

# C. A private directory of this uid is accepted; group read is not write.
mkdir -p "$T/privado" && chmod 700 "$T/privado"
expect_rc "diretorio 0700 do dono passa" 0 "$SD" "$T/privado"
mkdir -p "$T/leitura" && chmod 750 "$T/leitura"
expect_rc "diretorio 0750 do dono passa (sem escrita de grupo)" 0 "$SD" "$T/leitura"

# D. A symlink in place of the state directory is refused, and its target is untouched.
mkdir -p "$T/alvo" && chmod 700 "$T/alvo"
ln -s "$T/alvo" "$T/link"
expect_rc "caminho que e link simbolico reprova" 1 "$SD" "$T/link"
expect_eq "alvo do link intocado" 0 "$(find "$T/alvo" -mindepth 1 | wc -l)"

# E. A regular file in place of the directory is refused.
: > "$T/arquivo"
expect_rc "arquivo no lugar do diretorio reprova" 1 "$SD" "$T/arquivo"

# F. A symlink as the parent (the shared base) is refused.
ln -s "$T/alvo" "$T/base_link"
expect_rc "base que e link simbolico reprova" 1 "$SD" "$T/base_link/estado"
expect_eq "alvo do link de base intocado" 0 "$(find "$T/alvo" -mindepth 1 | wc -l)"

# G-I. run_suite.sh, the writer of the state files. Each case is refused with 126 before
# docker exec, and the target of a planted link keeps its content (review of 2026-11-20).
RUN="$TDD_DIR/run_suite.sh"
require_file "run_suite.sh existe" "$RUN"
FX="$(make_fixture)"
BIN="$(make_fakebin yes)"
export PATH="$BIN:$PATH"
setup_fake_image "$FX"
export FAKE_EXEC_RC=0 FAKE_EXEC_OUT="tdd-verdict: 0"
export FAKE_DOCKER_STDIN="$BIN/stdin.tar"
RH="$(printf '%s' "$FX" | sha256sum | cut -c1-16)"
RSTATE="/var/tmp/gusworld-tdd/$RH"

# G. The state directory is writable by others (0777): refused, no exec.
LOGG="$BIN/state.log"
mkdir -p "$RSTATE" && chmod 777 "$RSTATE"
env FAKE_DOCKER_LOG="$LOGG" bash "$FX/tools/tdd/run_suite.sh" >/dev/null 2>&1
expect_eq "estado 0777: run_suite rc 126" 126 "$?"
expect_eq "estado 0777: docker exec nunca chamado" 0 "$(grep -c '^exec ' "$LOGG" 2>/dev/null || echo 0)"

# H. last-reason is a symlink to a victim file: refused, the victim untouched, no exec.
FX2="$(make_fixture)"
setup_fake_image "$FX2"
RH2="$(printf '%s' "$FX2" | sha256sum | cut -c1-16)"
RSTATE2="/var/tmp/gusworld-tdd/$RH2"
mkdir -p "$RSTATE2" && chmod 700 "$RSTATE2"
printf 'precioso\n' > "$T/victim-reason"
ln -s "$T/victim-reason" "$RSTATE2/last-reason"
LOGH="$BIN/reason-link.log"
env FAKE_DOCKER_LOG="$LOGH" bash "$FX2/tools/tdd/run_suite.sh" >/dev/null 2>&1
expect_eq "last-reason link: run_suite rc 126" 126 "$?"
expect_eq "last-reason link: alvo intacto" "precioso" "$(cat "$T/victim-reason")"
expect_eq "last-reason link: docker exec nunca chamado" 0 "$(grep -c '^exec ' "$LOGH" 2>/dev/null || echo 0)"

# I. last.log is a symlink to a victim file: refused, the victim untouched, no exec.
FX3="$(make_fixture)"
setup_fake_image "$FX3"
RH3="$(printf '%s' "$FX3" | sha256sum | cut -c1-16)"
RSTATE3="/var/tmp/gusworld-tdd/$RH3"
mkdir -p "$RSTATE3" && chmod 700 "$RSTATE3"
printf 'precioso\n' > "$T/victim-log"
ln -s "$T/victim-log" "$RSTATE3/last.log"
LOGI="$BIN/log-link.log"
env FAKE_DOCKER_LOG="$LOGI" bash "$FX3/tools/tdd/run_suite.sh" >/dev/null 2>&1
expect_eq "last.log link: run_suite rc 126" 126 "$?"
expect_eq "last.log link: alvo intacto" "precioso" "$(cat "$T/victim-log")"
expect_eq "last.log link: docker exec nunca chamado" 0 "$(grep -c '^exec ' "$LOGI" 2>/dev/null || echo 0)"

suite_done

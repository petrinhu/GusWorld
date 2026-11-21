# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Case (d): the XDG_RUNTIME_DIR check refuses any directory that is not mode 700
# owned by the current user (GW-35: the test process gets its own runtime dir).
. "$(dirname "${BASH_SOURCE[0]}")/lib.sh"

XDG_LIB="$TDD_DIR/xdg_check.sh"
require_file "xdg_check.sh existe" "$XDG_LIB"
. "$XDG_LIB"
require_function "xdg_runtime_ok definida" xdg_runtime_ok

base="$(mktemp -d /tmp/tdd-xdg-XXXXXX)"
mkdir "$base/sem-chmod"
chmod 755 "$base/sem-chmod"
mkdir "$base/com-chmod"
chmod 700 "$base/com-chmod"

expect_fail "diretorio 755 sem chmod 700 reprova" xdg_runtime_ok "$base/sem-chmod"
expect_rc "diretorio 700 do dono passa" 0 xdg_runtime_ok "$base/com-chmod"
expect_fail "diretorio inexistente reprova" xdg_runtime_ok "$base/nao-existe"

# Right mode, wrong owner: the current uid is faked to 4242, the directory belongs to
# builder (10001), so the check must refuse it (M8).
fakeid="$(mktemp -d /tmp/tdd-id-XXXXXX)"
printf '#!/usr/bin/env bash\necho 4242\n' > "$fakeid/id"
chmod +x "$fakeid/id"
expect_rc "dono errado com modo 700 reprova" 1 \
    env PATH="$fakeid:$PATH" bash -c ". '$XDG_LIB'; xdg_runtime_ok '$base/com-chmod'"

suite_done

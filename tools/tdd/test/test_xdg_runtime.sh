# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Case (d): the XDG_RUNTIME_DIR check refuses any directory that is not mode 700
# owned by the current user (L-35: the test process gets its own runtime dir).
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

suite_done

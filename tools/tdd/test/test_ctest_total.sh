# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# The total of "ctest -N" is read as a VALUE. Exit 0 alone proves nothing: ctest
# -N with no tests exits 0 and prints "Total Tests: 0" (CLAUDE.md, Comandos).
. "$(dirname "${BASH_SOURCE[0]}")/lib.sh"

CT="$TDD_DIR/ctest_total.sh"
require_file "ctest_total.sh existe" "$CT"

expect_rc "Total Tests: 10 passa" 0 "$CT" <<< "Total Tests: 10"
expect_eq "Total Tests: 10 imprime 10" 10 "$("$CT" <<< "Total Tests: 10")"
expect_rc "Total Tests: 0 reprova" 1 "$CT" <<< "Total Tests: 0"
expect_rc "saida sem total reprova" 1 "$CT" <<< "Test project /work/build"
expect_rc "entrada vazia reprova" 1 "$CT" <<< ""

suite_done

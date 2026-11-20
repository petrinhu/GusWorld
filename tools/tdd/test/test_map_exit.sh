# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Case (a): the exit mapping. Only 0 and 1 pass through; every other code is 126.
. "$(dirname "${BASH_SOURCE[0]}")/lib.sh"

MAP="$TDD_DIR/map_exit.sh"
require_file "map_exit.sh existe" "$MAP"

expect_eq "rc 0 continua 0" 0 "$("$MAP" 0)"
expect_eq "rc 1 continua 1" 1 "$("$MAP" 1)"
for code in 2 3 9 124 125 127 137 255; do
    expect_eq "rc $code vira 126" 126 "$("$MAP" "$code")"
done
expect_eq "sem argumento vira 126" 126 "$("$MAP")"
expect_eq "texto vira 126" 126 "$("$MAP" abc)"

suite_done

#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Reads "ctest -N" output on stdin and checks the VALUE of "Total Tests:", not the
# exit code: ctest -N with no tests exits 0 and prints "Total Tests: 0" (CLAUDE.md).
# Prints the total and exits 0 when it is a positive integer; exits 1 otherwise.
total="$(sed -n 's/^Total Tests: *\([0-9][0-9]*\)[[:space:]]*$/\1/p' | sed -n '1p')"
case "$total" in
    ''|*[!0-9]*) exit 1 ;;
esac
[ "$total" -gt 0 ] || exit 1
echo "$total"

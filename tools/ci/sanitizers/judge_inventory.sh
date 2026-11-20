#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Judges the ctest inventory of the sanitizers build. Single responsibility: the count
# of registered tests and the three red proofs, from the inventory to a verdict.
#
# Why the verdict is written this way:
#   ctest -N exits 0 even with zero tests, so the count is read from the value, and
#   zero fails on purpose: an empty scan is a broken scan (L-36).
#   The three red proofs must be registered by name. Without them the suite runs under
#   the sanitizers and the gate can pass with no red proof (L-36).
#
# Usage: judge_inventory.sh <inventory>
#   <inventory> is the output of "ctest -N", written by the workflow before this call.
# Exit 0 only when the count is above zero and all three proofs are registered.
set -eu

inventory="${1:?inventory path missing}"

total="$(sed -n 's/^Total Tests: //p' "$inventory")"
echo "registered tests: ${total:-<unreadable>}"
case "$total" in
  ''|*[!0-9]*)
    echo "FAIL: the test count is missing or not a number"
    exit 1
    ;;
esac
if [ "$total" -eq 0 ]; then
  echo "FAIL: zero tests registered"
  exit 1
fi
# The three red proofs must be registered by name. Without them the suite
# runs under the sanitizers and the gate can pass with no red proof (L-36).
proofs=0
for name in sanitizer_heap_buffer_overflow_is_detected \
            sanitizer_signed_overflow_is_detected \
            sanitizer_leak_is_detected; do
  found="$(grep -c ": $name\$" "$inventory" || true)"
  if [ "${found:-0}" -eq 1 ]; then
    proofs=$((proofs + 1))
    echo "proof found: $name"
  else
    echo "proof MISSING: $name (matches: ${found:-0})"
  fi
done
echo "proof tests found: $proofs of 3"
if [ "$proofs" -ne 3 ]; then
  echo "FAIL: a proof test is not registered"
  exit 1
fi

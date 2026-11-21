#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Runs cppcheck once with the gate policy, prints the log, then judges it with judge.sh.
# The one place where the cppcheck command line lives: the real gate and both red proofs
# share it. Policy: --std=c++23 always, and --error-exitcode=1 in every mode except
# "without". Without the option cppcheck exits 0 even when it reports findings.
# Usage: scan.sh <mode> <log-file> <target>
#   real:     target is the sources list file, passed as --file-list (the gate on the tree)
#   with:     target is one .cpp, with the option (red proof)
#   without:  target is one .cpp, without the option (the trap)
# Exit code is judge.sh's: 0 only when the verdict of the mode holds.
set -eu

mode="${1:?mode missing: real, with or without}"
log="${2:?log path missing}"
target="${3:?target missing}"

here="$(cd "$(dirname "$0")" && pwd)"

# Include path of each layer that exists. A source reaches its headers through these
# directories only, so without them the #include lines do not resolve and cppcheck loses
# the header bodies. Relative paths: this script runs from the repository root.
# The layer list is the same as in .clang-tidy and in enumerate.sh; change all three together.
includes=()
for layer in core content domain app present; do
  if [ -d "$layer/include" ]; then
    includes+=("-I$layer/include")
  fi
done

case "$mode" in
  real) inputs=(--file-list="$target") ;;
  with|without) inputs=("$target") ;;
  *) echo "FAIL: unknown mode ${mode}"; exit 1 ;;
esac
case "$mode" in
  without) policy=() ;;
  *) policy=(--error-exitcode=1) ;;
esac

# rc is taken with "|| rc=$?", so the real exit code is kept under set -e.
rc=0
cppcheck --std=c++23 "${includes[@]}" "${policy[@]}" "${inputs[@]}" > "$log" 2>&1 || rc=$?
cat "$log"
bash "$here/judge.sh" "$mode" "$rc" "$log" "$target"

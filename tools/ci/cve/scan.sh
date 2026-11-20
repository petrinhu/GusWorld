#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Runs grype once with the gate policy, prints the report, then judges with judge.sh.
# The one place where the grype command line lives: the proof and the real scan share it.
# Policy: --only-fixed (a CVE with no published fix never fails the gate) and
# --fail-on critical (only critical severity fails the gate).
# Usage: scan.sh <expected-distro> <out-dir> <grype-target> [extra grype options...]
# Prints the report, then exactly one line VERDICT=<value> from judge.sh.
# Exit 0 only on accept.
set -u

want="${1:?expected distro missing}"
out="${2:?out dir missing}"
target="${3:?grype target missing}"
shift 3

here="$(cd "$(dirname "$0")" && pwd)"
mkdir -p "$out"
rc=0
# JSON goes to stdout, the INFO log (-v) to stderr. judge.sh reads the exit code.
grype "$target" -v -o json --only-fixed --fail-on critical "$@" > "$out/grype.json" 2> "$out/grype.err" || rc=$?
bash "$here/report.sh" "$out/grype.json" "$out/grype.err"
bash "$here/judge.sh" "$rc" "$out/grype.json" "$out/grype.err" "$want"

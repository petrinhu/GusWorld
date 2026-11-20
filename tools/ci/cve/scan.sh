#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Runs grype once and judges the outcome with judge.sh. The one place where the grype
# command line lives, so the proof of strength and the real scan cannot drift apart.
# Usage: scan.sh <expected-distro> <out-dir> <grype-target> [grype options...]
# Prints exactly one line, VERDICT=<value>, from judge.sh. Exit 0 only on "accept".
set -u

want="${1:?expected distro missing}"
out="${2:?out dir missing}"
target="${3:?grype target missing}"
shift 3

here="$(cd "$(dirname "$0")" && pwd)"
mkdir -p "$out"
rc=0
# JSON goes to stdout, the INFO log (-v) to stderr. The verdict is the exit code, read by judge.sh.
grype "$target" -v -o json "$@" > "$out/grype.json" 2> "$out/grype.err" || rc=$?
bash "$here/judge.sh" "$rc" "$out/grype.json" "$out/grype.err" "$want"

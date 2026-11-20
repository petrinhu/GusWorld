#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Runs trivy once with the gate policy, prints the report, then judges with judge.sh.
# The one place where the trivy command line lives: the proof and the real scan share it.
# Same interface as ../scan.sh (grype), so expect.sh calls either one through CVE_SCAN.
#
# Policy: --severity CRITICAL, --ignore-unfixed (no published fix never fails the gate),
# --exit-code 2 (only a critical fixed finding fails it), --scanners vuln (secrets and
# misconfigurations are not this gate's business, and they would raise the exit code too).
# Targets: registry:REF runs "trivy image" straight from the registry, because the CI
# container has no docker daemon; dir:PATH runs "trivy rootfs".
# --exclude PATTERN follows the grype convention ('./proc/**'). trivy takes --skip-dirs, and
# the docs do not confirm whether it matches relative or absolute paths, so both spellings
# are passed. A spelling that matches nothing is harmless. Which one matched is for the log.
#
# Usage: scan.sh <expected-distro> <out-dir> <target> [--exclude PATTERN ...]
# Needs TRIVY_CACHE_DIR in the environment (the workflow sets it).
# Prints the report, then exactly one line VERDICT=<value> from judge.sh. Exit 0 only on accept.
set -u

want="${1:?expected distro missing}"
out="${2:?out dir missing}"
target="${3:?target missing}"
shift 3

here="$(cd "$(dirname "$0")" && pwd)"
cache="${TRIVY_CACHE_DIR:?TRIVY_CACHE_DIR missing}"

case "$target" in
  registry:*) sub=(image --image-src remote); ref="${target#registry:}" ;;
  dir:*)      sub=(rootfs); ref="${target#dir:}" ;;
  *) echo "trivy scan: unknown target scheme: $target"; exit 1 ;;
esac

skip=()
while [ "$#" -gt 0 ]; do
  case "$1" in
    --exclude)
      pattern="${2:?exclude pattern missing}"
      pattern="${pattern%/\*\*}"
      skip+=(--skip-dirs "$pattern" --skip-dirs "/${pattern#./}")
      shift 2 ;;
    *) echo "trivy scan: unsupported option: $1"; exit 1 ;;
  esac
done

mkdir -p "$out"
rc=0
trivy "${sub[@]}" \
  --scanners vuln --severity CRITICAL --ignore-unfixed --exit-code 2 --list-all-pkgs \
  --cache-dir "$cache" --format json --output "$out/trivy.json" \
  "${skip[@]}" "$ref" > "$out/trivy.out" 2> "$out/trivy.err" || rc=$?
echo "trivy exit code: $rc"
bash "$here/report.sh" "$out/trivy.json" "$out/trivy.err"
bash "$here/judge.sh" "$rc" "$out/trivy.json" "$out/trivy.err" "$want"

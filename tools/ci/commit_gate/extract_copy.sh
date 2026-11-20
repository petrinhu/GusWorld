#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Extracts the gate and its self-test from the committed blob into a scratch copy, then
# proves that each copied file equals its blob. Single responsibility: extraction and the
# blob check. It never mutates anything; the mutation belongs to mutate.py.
# Usage: extract_copy.sh <copy-dir>   (run from the repository root)
set -eu -o pipefail

copy="${1:?copy directory missing}"

mkdir -p "$copy"
git archive HEAD tools/security/commit_gate.py tools/security/test_commit_gate.py | tar -x -C "$copy"
for f in commit_gate.py test_commit_gate.py; do
  blob=$(git show "HEAD:tools/security/$f" | sha256sum)
  copied=$(sha256sum "$copy/tools/security/$f")
  if [ "${blob%% *}" != "${copied%% *}" ]; then
    echo "copy of $f differs from the committed blob"
    exit 1
  fi
  echo "copy equals blob: $f ${blob%% *}"
done

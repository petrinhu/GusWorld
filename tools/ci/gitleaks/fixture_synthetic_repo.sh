#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Builds the synthetic repository of the red proof, half 1: one commit with one fabricated key.
# Single responsibility: build the fixture. It never scans and never judges.
# Usage: fixture_synthetic_repo.sh <new-repo-dir>
# The directory must not exist yet. The key is generated at runtime, printed nowhere, and
# written only into this disposable repository.
set -eu

repo="${1:?new repository directory missing}"
mkdir "$repo"
git init -q -b main "$repo"
# Generated here, never printed, never committed to this repository.
key=$(python3 -c 'import secrets; print("AKIA" + "".join(secrets.choice("ABCDEFGHIJKLMNOPQRSTUVWXYZ234567") for _ in range(16)))')
if [ "${#key}" -ne 20 ]; then echo "synthetic key has the wrong length"; exit 1; fi
printf 'aws_access_key_id = %s\n' "$key" > "$repo/fixture.txt"
git -C "$repo" add fixture.txt
git -C "$repo" -c user.name=ci -c user.email=ci@example.invalid commit -q -m "test: synthetic fixture for the red proof"

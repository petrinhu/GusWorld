#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Asserts that the ID in /etc/os-release is the one the matrix names. Single responsibility:
# compare one value and fail closed. Without this, a job named for one distro could scan another
# image without any step noticing, because report.sh only prints the os-release.
# Usage: assert_os_id.sh <expected-id>
# Exit 0 only when the ID equals the expected one. ID_LIKE is not read: only ID names the image.
set -u

want="${1:?expected os id missing}"

if [ ! -r /etc/os-release ]; then
  echo "OS ID MISSING: /etc/os-release is not readable"
  exit 1
fi
got=$(sed -n 's/^ID=//p' /etc/os-release | head -n 1 | tr -d '"')
if [ -z "$got" ]; then
  echo "OS ID MISSING: no ID line in /etc/os-release"
  exit 1
fi
if [ "$got" != "$want" ]; then
  echo "OS ID MISMATCH: wanted $want, got $got"
  exit 1
fi
echo "OS ID OK: $got"

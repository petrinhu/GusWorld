#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# CMake floor check shared by the CI jobs (build, clang-tidy, cppcheck,
# sanitizers, layers). It reads the version of the cmake on PATH and fails
# when that version cannot be read or is below 4.1.
#
# The 4.1 floor comes from cmake_minimum_required(VERSION 4.1) in
# CMakeLists.txt, line 2. If that line changes, the literal below must change
# with it.
#
# The version is read from a file, not from a pipe, so a failing cmake cannot
# hide behind the sed that follows it. The file cmake_version.txt is written to
# the current directory, which is the repository root in every caller.
#
# Verdicts: the success line "CMake <version> meets the 4.1 floor" with exit 0,
# or a FAIL line with exit 1. Nothing else is printed as a verdict.
#
# Declared limit: this floor has never been seen rejecting a version in CI. The
# check was moved here from copies that already existed, without a new proof of
# its own (project owner's decision, 20/11/2026: "Mover sem prova nova").
#
# Needs bash. Callers run it as `bash tools/ci/cmake_floor.sh`.
set -euo pipefail
cmake --version > cmake_version.txt
version="$(sed -n '1s/^cmake version //p' cmake_version.txt)"
echo "CMake read: ${version:-<unreadable>}"
case "$version" in
  ''|*[!0-9.]*)
    echo "FAIL: could not read the CMake version"
    exit 1
    ;;
esac
if printf '%s\n%s\n' 4.1 "$version" | sort -V -C; then
  echo "CMake ${version} meets the 4.1 floor"
else
  echo "FAIL: CMake ${version} is below the 4.1 floor"
  exit 1
fi

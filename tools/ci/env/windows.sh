#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Setup for the Windows target (windows-latest, MSVC) of CI jobs. It installs
# nothing: the runner image already ships Visual Studio, CMake and Git
# Bash. It does two things and nothing else:
#   1. export CMAKE_CONFIG_TYPE=Debug to the next steps via $GITHUB_ENV, the
#      configuration that cmake --build and ctest use with a multi-config generator;
#   2. print the cmake and git versions, so the CI log proves what ran.
#
# It does not source _common.sh: that script fixes cc/c++ and Ninja, and the
# Windows job uses the default Visual Studio generator.

set -eu

{
    echo "CMAKE_CONFIG_TYPE=Debug"
} >> "$GITHUB_ENV"

cmake --version
git --version

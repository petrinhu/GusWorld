#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Setup for the Fedora target (primary, L-09) of the build job in CI.
# Three responsibilities and nothing else:
#   1. install both compilers, GCC and Clang, from the distro repository;
#   2. export CMAKE_GENERATOR=Ninja to the next steps via $GITHUB_ENV;
#   3. print the tool versions, so the CI log proves what actually ran.
#
# The compiler is not chosen here. The workflow matrix sets CC and CXX for each
# entry (GCC or Clang), and this script only installs the packages. Running it
# alone therefore needs CC and CXX already set in the environment.
#
# Measured 11/10/2026 in fedora:latest (Fedora 44, local cache, no pull):
# dnf provides cmake 4.3.0, above the 4.1 floor, so no SHA-pinned download
# is needed here (that path exists in layers.yml only because the bare
# ubuntu-latest runner ships cmake 3.31.6). The image ships WITHOUT git and
# WITHOUT python3. The workflow installs git before its checkout, so the
# checkout is a real git clone. Without git, actions/checkout downloads a
# tarball through the REST API instead, and that path failed with 5xx from
# GitHub three times in each of two CI runs (run 38196743681), so it is not
# used. git stays in the package list below, so this script is complete when
# it runs alone too.
# python3 is left out on purpose: no CMakeLists.txt in this tree calls it.
#
# No hash of this file is compared anywhere: the workflow runs the copy from
# its own checkout, so such a comparison would only compare the file with itself.

set -eu

dnf -y install gcc-c++ clang cmake ninja-build git

{
    echo "CMAKE_GENERATOR=Ninja"
} >> "$GITHUB_ENV"

cat /etc/os-release
"$CC" --version
"$CXX" --version
cmake --version
ninja --version
git --version

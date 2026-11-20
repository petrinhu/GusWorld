#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Setup for the Fedora target (primary, L-09) of CI jobs.
# Three responsibilities and nothing else:
#   1. install both compilers, GCC and Clang, from the distro repository;
#   2. export CC, CXX and CMAKE_GENERATOR=Ninja to the next steps via $GITHUB_ENV;
#   3. print the tool versions, so the CI log proves what actually ran.
#
# The compiler comes from the workflow matrix when the workflow has one (the
# build job sets CC and CXX per entry, GCC or Clang). A workflow without a
# matrix gets the default: cc and c++, the compilers this script used before
# the matrix existed. Set CC and CXX in the environment to override it.
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

CC="${CC:-cc}"
CXX="${CXX:-c++}"

dnf -y install gcc-c++ clang cmake ninja-build git

{
    echo "CC=${CC}"
    echo "CXX=${CXX}"
    echo "CMAKE_GENERATOR=Ninja"
} >> "$GITHUB_ENV"

cat /etc/os-release
"$CC" --version
"$CXX" --version
cmake --version
ninja --version
git --version

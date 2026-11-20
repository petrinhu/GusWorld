#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Setup for the Ubuntu target (ubuntu:latest) of CI jobs.
# Three responsibilities and nothing else:
#   1. install the toolchain from the distro repository (g++ and clang);
#   2. export CC, CXX and CMAKE_GENERATOR=Ninja to the next steps via $GITHUB_ENV;
#   3. print the tool versions, so the CI log proves what actually ran.
#
# The compiler comes from the workflow matrix when the workflow has one (the
# build job sets CC and CXX per entry, GCC or Clang). A workflow without a
# matrix gets the default: cc and c++. Set CC and CXX in the environment to
# override it.
#
# Expected, not measured yet: ubuntu:latest is Ubuntu 26.04 (resolute), and
# its apt repository ships cmake 4.2.3, above the 4.1 floor, so no SHA-pinned
# download is needed. The CI log of the Ubuntu job is the measurement.
# The image ships WITHOUT git. The workflow installs git before its checkout,
# so the checkout is a real git clone. git stays in the package list below,
# so this script is complete when it runs alone too.
#
# No hash of this file is compared anywhere: the workflow runs the copy from
# its own checkout, so such a comparison would only compare the file with itself.

set -eu

CC="${CC:-cc}"
CXX="${CXX:-c++}"

apt-get update
DEBIAN_FRONTEND=noninteractive apt-get install -y g++ clang cmake ninja-build git

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

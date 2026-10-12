#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Setup for the Fedora target (primary, L-09) of the build job in CI.
# Three responsibilities and nothing else:
#   1. install the toolchain from the distro repository;
#   2. export CC, CXX and GUSWORLD_PREP_SHA to the next steps via $GITHUB_ENV;
#   3. print the tool versions, so the CI log proves what actually ran.
#
# Measured 11/10/2026 in fedora:latest (Fedora 44, local cache, no pull):
# dnf provides cmake 4.3.0, above the 4.1 floor, so no SHA-pinned download
# is needed here (that path exists in layers.yml only because the bare
# ubuntu-latest runner ships cmake 3.31.6). The image ships WITHOUT git and
# WITHOUT python3: the bootstrap checkout therefore falls back to the REST
# API by design, and git is installed here, before the real checkout.
# python3 is left out on purpose: no CMakeLists.txt in this tree calls it.
#
# GUSWORLD_PREP_SHA is the sha256 of this very file. The workflow compares
# it against the real file after the real checkout, which proves that the
# script that ran (from the bootstrap checkout) is byte-for-byte the one the
# repository has.

set -eu

cc_bin="cc"
cxx_bin="c++"

dnf -y install gcc-c++ cmake ninja-build git

{
    echo "CC=${cc_bin}"
    echo "CXX=${cxx_bin}"
    echo "GUSWORLD_PREP_SHA=$(sha256sum "$0" | awk '{print $1}')"
} >> "$GITHUB_ENV"

"$cc_bin" --version
"$cxx_bin" --version
cmake --version
ninja --version
git --version

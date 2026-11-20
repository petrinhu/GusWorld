#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Setup for the Fedora target (primary, L-09) of CI jobs: installs the
# toolchain, then hands off to _common.sh for the CI environment.
#
# Measured 20/11/2026 in fedora:latest (Fedora 45, cmake 4.3.0 from dnf):
# dnf provides cmake 4.3.0, above the 4.1 floor, so no SHA-pinned download
# is needed here (that path exists in layers.yml only because the bare
# ubuntu-latest runner ships cmake 3.31.6). The image ships WITHOUT git and
# WITHOUT python3. The workflow installs git before its checkout, so the
# checkout is a real git clone. Without git, actions/checkout downloads a
# tarball through the REST API instead, and that path failed with 5xx from
# GitHub three times in each of two CI runs (run 38196743681), so it is not
# used. git stays in the package list below, so the install is complete when
# this script runs alone. The rest writes to $GITHUB_ENV and assumes Actions.
# python3 is not installed here. install_toolchain.sh installs python3-pyyaml, which
# brings python3, in a later step of the same container install.
#
# No hash of this file is compared anywhere: the workflow runs the copy from
# its own checkout, so such a comparison would only compare the file with itself.

set -eu

dnf -y install gcc-c++ clang cmake ninja-build git

. "$(dirname "$0")/_common.sh"

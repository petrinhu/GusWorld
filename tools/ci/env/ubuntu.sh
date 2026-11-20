#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Setup for the Ubuntu target (ubuntu:latest) of CI jobs: installs the
# toolchain, then hands off to _common.sh for the CI environment.
#
# Measured 20/11/2026 in ubuntu:latest (Ubuntu 26.04, cmake 4.2.3 from apt),
# above the 4.1 floor, so no SHA-pinned download is needed.
# The image ships WITHOUT git. The workflow installs git before its checkout,
# so the checkout is a real git clone. git stays in the package list below,
# so the install is complete when this script runs alone. The rest writes to
# $GITHUB_ENV and assumes Actions.
#
# No hash of this file is compared anywhere: the workflow runs the copy from
# its own checkout, so such a comparison would only compare the file with itself.

set -eu

apt-get update
DEBIAN_FRONTEND=noninteractive apt-get install -y g++ clang cmake ninja-build git

. "$(dirname "$0")/_common.sh"

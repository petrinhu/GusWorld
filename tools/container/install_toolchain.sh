#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# The recipe of gusworld-ci-fedora. It runs INSIDE the container that
# container_install.sh starts, as root, with network, and only once. The context
# is unpacked in /opt/gusworld-ci before this runs. The container is committed
# afterwards, so nothing here is left running.
set -euo pipefail

ctx=/opt/gusworld-ci

if [ "${BASE_VERSION_ID:-unknown}" != "$(. /etc/os-release && echo "$VERSION_ID")" ]; then
    echo "install_toolchain: base VERSION_ID is not $BASE_VERSION_ID, refusing" >&2
    exit 1
fi

mkdir -p /etc/gusworld
GITHUB_ENV=/etc/gusworld/ci.env bash "$ctx/fedora.sh"
dnf clean all

( mkdir -p /tmp/floor && cd /tmp/floor && bash "$ctx/cmake_floor.sh" )
rm -rf /tmp/floor

# python3-pyyaml pulls python3 in; tar and coreutils are named, not assumed.
dnf -y install tar coreutils python3-pyyaml
dnf clean all

useradd --create-home --uid 10001 --shell /bin/bash builder

# rpm -q exits non-zero when any package is missing, which fails the install.
rpm -q tar coreutils python3-pyyaml cmake gcc-c++ ninja-build clang git > /etc/gusworld/packages.txt
cat /etc/gusworld/packages.txt

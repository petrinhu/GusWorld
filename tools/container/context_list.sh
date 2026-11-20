#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# The only place where the build context of gusworld-ci-fedora is written down.
# Sourced by env_hash.sh and context_tar.sh; it runs nothing on its own.
# Each entry is a repo-relative path. Inside the context tar every file keeps its
# basename, at the root of the tar; the container unpacks it into /opt/gusworld-ci.

TOOLS_CONTAINER_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$TOOLS_CONTAINER_DIR/../.." && pwd)"

CONTEXT_FILES=(
    tools/container/install_toolchain.sh
    tools/ci/env/fedora.sh
    tools/ci/env/_common.sh
    tools/ci/cmake_floor.sh
)

#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# CVE gate prerequisites for the Fedora target: jq, which report.sh reads. curl is already in
# fedora:latest (the grype install passes on it). Kept apart from tools/ci/env/, which is the
# build toolchain shared with build.yml, so this gate does not change the build environment.
set -eu

dnf -y install jq

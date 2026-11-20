#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# CVE gate prerequisites for the Ubuntu target: curl and ca-certificates (fetch_verified.sh
# needs them to download grype and trivy over https) and jq (report.sh). ubuntu:latest ships
# without them. Kept apart from tools/ci/env/, which is the shared build toolchain.
set -eu

export DEBIAN_FRONTEND=noninteractive
apt-get update
apt-get install -y curl ca-certificates jq

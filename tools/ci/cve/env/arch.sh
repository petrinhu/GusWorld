#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# CVE gate prerequisites for the Arch target (archlinux:latest): curl and ca-certificates
# (fetch_verified.sh needs them to download grype over https) and jq (report.sh). The image
# may ship without the CA bundle, so both are named here, not inferred from pacman's own
# dependencies. Kept apart from tools/ci/env/, which is the shared build toolchain.
# A full sync (-Syu), never -Sy alone: a partial upgrade breaks Arch.
set -eu

pacman -Syu --noconfirm --needed curl ca-certificates jq

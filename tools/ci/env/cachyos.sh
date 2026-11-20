#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Setup for the CachyOS target (cachyos/cachyos:latest) of CI jobs: installs the
# toolchain, then hands off to _common.sh for the CI environment.
#
# CachyOS is its own target, not a renamed Arch: its own image and its own job.
# The install is a full sync (-Syu), never -Sy alone: a partial upgrade breaks
# the distro. git is in the package list below, so the install is complete when
# this script runs alone. The rest writes to $GITHUB_ENV and assumes Actions.

set -eu

pacman -Syu --noconfirm --needed gcc clang cmake ninja git

. "$(dirname "$0")/_common.sh"

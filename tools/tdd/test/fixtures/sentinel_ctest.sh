#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Sentinel ctest placed first on PATH in the invoker tests. Any call leaves a
# file behind, so a test can prove the local ctest never ran.
: > "${SENTINEL_FILE:?}"
exit 0

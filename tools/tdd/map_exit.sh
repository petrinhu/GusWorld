#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Maps an exit code to the guard verdict (ADR-026, decision 2): 0 stays 0, 1 stays
# 1, and every other code, including none, becomes 126 (no run). The guard reads
# only 0 and 1 as a result, and 126 as "did not run".
case "${1:-}" in
    0) echo 0 ;;
    1) echo 1 ;;
    *) echo 126 ;;
esac

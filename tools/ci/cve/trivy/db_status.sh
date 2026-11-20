#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Prints what trivy says about its vulnerability database: the trivy version and the cache
# metadata. The metadata records version and dates, and the docs do not list the sources the
# database was built from, so the sources are read from the matches in report.sh instead.
# Single responsibility: print. It never fails the gate.
set -u

cache="${TRIVY_CACHE_DIR:?TRIVY_CACHE_DIR missing}"
echo "--- trivy version ---"
trivy --version || echo "trivy version failed"
echo "--- trivy db metadata ($cache/db/metadata.json) ---"
cat "$cache/db/metadata.json" 2>/dev/null || echo "metadata.json not found"
exit 0

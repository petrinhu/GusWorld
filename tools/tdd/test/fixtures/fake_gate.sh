#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Stands in for tools/container/resource_gate.sh inside the test fixture only.
# The real gate reads the host cgroup, which does not exist in the test container.
echo "gate fake: passa"
exit "${FAKE_GATE_RC:-0}"

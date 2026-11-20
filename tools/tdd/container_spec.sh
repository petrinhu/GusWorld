# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# The single source of the long-lived test container's spec (ADR-026, decision 7; review
# MÉDIA-2). prepare.sh builds the container from these values, container_ready.sh checks
# the HostConfig against them, and the tests read them. Sourced, it runs nothing.
# The bytes and nanocpus are what docker inspect reports; the flag forms are what
# docker create takes. Both come from here, so no other file repeats a number.

SPEC_MEMORY="4g"
SPEC_MEMORY_BYTES=4294967296
SPEC_MEMORY_SWAP="4g"
SPEC_MEMORY_SWAP_BYTES=4294967296
SPEC_PIDS=233
SPEC_CPUS=4
SPEC_NANOCPUS=4000000000
SPEC_NETWORK=none
SPEC_USER=builder

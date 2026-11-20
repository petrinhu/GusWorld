#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Measures the machine at this instant and prints one line per check. Exit 0 only
# when every check passes. The defaults are the L-11 global limits:
#   MemAvailable >= 10 GiB, 1-minute load <= 14, pids.current + budget <= 3200.
# The GATE_* variables can only TIGHTEN a limit, never loosen it. They exist so a
# negative control can be run with a stricter threshold than the default.
set -uo pipefail

default_min_mem_kb=10485760
default_max_load1=14
default_pids_ceiling=3200
default_pids_budget=200

min_mem_kb=$default_min_mem_kb
if [[ "${GATE_MIN_MEM_KB:-}" =~ ^[0-9]+$ ]] && (( GATE_MIN_MEM_KB > min_mem_kb )); then
    min_mem_kb=$GATE_MIN_MEM_KB
fi

max_load1=$default_max_load1
if [[ "${GATE_MAX_LOAD1:-}" =~ ^[0-9]+(\.[0-9]+)?$ ]] \
    && awk -v a="$GATE_MAX_LOAD1" -v b="$max_load1" 'BEGIN { exit !(a + 0 < b + 0) }'; then
    max_load1=$GATE_MAX_LOAD1
fi

pids_ceiling=$default_pids_ceiling
if [[ "${GATE_PIDS_CEILING:-}" =~ ^[0-9]+$ ]] && (( GATE_PIDS_CEILING < pids_ceiling )); then
    pids_ceiling=$GATE_PIDS_CEILING
fi

pids_budget=$default_pids_budget
if [[ "${GATE_PIDS_BUDGET:-}" =~ ^[0-9]+$ ]] && (( GATE_PIDS_BUDGET > pids_budget )); then
    pids_budget=$GATE_PIDS_BUDGET
fi

pids_file="/sys/fs/cgroup/user.slice/user-$(id -u).slice/pids.current"
mem_kb="$(awk '/^MemAvailable:/ { print $2 }' /proc/meminfo)"
load1="$(awk '{ print $1 }' /proc/loadavg)"
pids="$(cat "$pids_file" 2>/dev/null)"

status=0

verdict=FAIL
if [[ "$mem_kb" =~ ^[0-9]+$ ]] && (( mem_kb >= min_mem_kb )); then verdict=ok; fi
[ "$verdict" = ok ] || status=1
echo "gate memavailable_kb=${mem_kb:-unreadable} min_kb=${min_mem_kb} verdict=${verdict}"

verdict=FAIL
if awk -v l="$load1" -v m="$max_load1" 'BEGIN { exit !(l + 0 <= m + 0) }'; then verdict=ok; fi
[ "$verdict" = ok ] || status=1
echo "gate load1=${load1} max=${max_load1} verdict=${verdict}"

verdict=FAIL
if [[ "$pids" =~ ^[0-9]+$ ]] && (( pids + pids_budget <= pids_ceiling )); then verdict=ok; fi
[ "$verdict" = ok ] || status=1
echo "gate pids_current=${pids:-unreadable} budget=${pids_budget} ceiling=${pids_ceiling} verdict=${verdict}"

exit "$status"

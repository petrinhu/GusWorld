#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Edge of the freshness check of the notice (C8 fatia 7, achado 2 of the CTO). A last-reason
# written up to FRESH_TOLERANCE seconds before ts is the fresh one of the run (the runner
# stamps ts after run_suite.sh exits). Written 5 s before ts, it belongs to an earlier run.
# Calls fresh_enough directly, as the notice imports it: stdlib only, no subprocess.
. "$(dirname "${BASH_SOURCE[0]}")/lib.sh"

NOTICE="$TDD_DIR/notice.py"
require_file "notice.py existe" "$NOTICE"
if [ "$FAILS" -gt 0 ]; then
    suite_done
fi

# fresh OFFSET: fresh_enough(ts + OFFSET, {"ts": ts}), printed as True or False.
fresh() {
    python3 -I - "$NOTICE" "$1" <<'PYEOF' 2>/dev/null || echo ERRO
import importlib.util
import sys

spec = importlib.util.spec_from_file_location("notice", sys.argv[1])
notice = importlib.util.module_from_spec(spec)
spec.loader.exec_module(notice)
ts = 1800000000
print("True" if notice.fresh_enough(ts + int(sys.argv[2]), {"ts": ts}) else "False")
PYEOF
}

expect_eq "last-reason com mtime ts-1 e fresco (borda da tolerancia)" "True" "$(fresh -1)"
expect_eq "last-reason com mtime ts-2 e fresco (limite da tolerancia)" "True" "$(fresh -2)"
expect_eq "last-reason com mtime ts-3 e antigo, fora da tolerancia" "False" "$(fresh -3)"
expect_eq "last-reason com mtime ts-5 e antigo, de outra execucao" "False" "$(fresh -5)"
expect_eq "last-reason com mtime igual a ts e fresco" "True" "$(fresh 0)"

suite_done

#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Config of the TDD guard after the virada (C8 fatia 7, ADR-026 decisions 2, 5 and 9).
# The guard runs `bash tools/tdd/run_suite.sh`, its timeout stays under the ceiling of the
# plugin's PostToolUse hook, and it classifies the production and the test files of this
# project as the game needs. Every check reads the config as shipped in the container
# (REPO/.claude/tdd-guard.json), and the classification runs through the plugin's own
# tdd_common, never a copy of its rules typed here.
. "$(dirname "${BASH_SOURCE[0]}")/lib.sh"

CONFIG="$REPO/.claude/tdd-guard.json"
HOOKS_JSON="$REPO/plugin/hooks/hooks.json"
require_file "tdd-guard.json do projeto existe" "$CONFIG"
require_file "hooks.json do plugin existe (copia do contêiner)" "$HOOKS_JSON"
require_file "tdd_common.py do plugin existe (copia do contêiner)" "$REPO/plugin/hooks/tdd_common.py"
require_file "run_suite.sh existe" "$TDD_DIR/run_suite.sh"
if [ "$FAILS" -gt 0 ]; then
    suite_done
fi

# facts: one "key=value" line per fact, read by the checks below.
FACTS="$(python3 -I - "$CONFIG" "$HOOKS_JSON" "$REPO/plugin/hooks" "$REPO" <<'PY'
import json
import os
import sys

config_path, hooks_path, hooks_dir, repo = sys.argv[1:5]
sys.path.insert(0, hooks_dir)
import tdd_common as c  # noqa: E402

with open(config_path, encoding="utf-8") as f:
    cfg = c.resolve_config(json.load(f))
with open(hooks_path, encoding="utf-8") as f:
    hooks = json.load(f)

ceiling = None
for group in hooks["hooks"]["PostToolUse"]:
    for hook in group["hooks"]:
        if any("tdd_runner.py" in str(a) for a in hook.get("args", [])):
            ceiling = hook["timeout"]

print("enabled=" + json.dumps(cfg.get("enabled")))
print("test_command=" + str(cfg.get("test_command")))
print("timeout_sec=" + str(cfg.get("timeout_sec")))
print("ceiling=" + str(ceiling))
print("test_glob_cmake=" + json.dumps("tests/**/CMakeLists.txt" in cfg.get("test_globs", [])))
print("run_suite_exists=" + json.dumps(os.path.isfile(os.path.join(repo, "tools/tdd/run_suite.sh"))))
for rel in ["core/a.cpp", "app/a.cpp", "domain/a.hpp"]:
    print("prod:" + rel + "=" + c.classify(rel, cfg))
for rel in ["tests/harness/t.cpp", "tests/selftest/t.cpp", "tests/CMakeLists.txt",
            "tests/fixtures/werror/CMakeLists.txt", "tests/fixtures/sanitizers/x.cpp"]:
    print("test:" + rel + "=" + c.classify(rel, cfg))
print("excl:tools/tdd/run_suite.sh=" + c.classify("tools/tdd/run_suite.sh", cfg))
print("excl:build/x.cpp=" + c.classify("build/x.cpp", cfg))
PY
)" || { echo "FALHA nao foi possivel ler a config"; suite_done; }

fact() {
    printf '%s\n' "$FACTS" | awk -v k="$1" 'index($0, k "=") == 1 { v = substr($0, length(k) + 2) } END { print v }'
}

expect_eq "guard ligado" "true" "$(fact enabled)"
expect_eq "test_command roda run_suite.sh" "bash tools/tdd/run_suite.sh" "$(fact test_command)"
expect_eq "run_suite.sh existe onde o test_command aponta" "true" "$(fact run_suite_exists)"
expect_eq "timeout_sec e 144 s (ADR-026, decisao 5)" "144" "$(fact timeout_sec)"
expect_eq "teto do hook do plugin lido do hooks.json" "180" "$(fact ceiling)"
if [ "$(fact timeout_sec)" -lt "$(fact ceiling)" ] 2>/dev/null; then
    pass_case "timeout_sec fica abaixo do teto do hooks.json"
else
    fail_case "timeout_sec fica abaixo do teto do hooks.json" "timeout [$(fact timeout_sec)] teto [$(fact ceiling)]"
fi
expect_eq "globo de CMakeLists de teste em test_globs" "true" "$(fact test_glob_cmake)"

expect_eq "producao em core/ e classificada como producao" "production" "$(fact prod:core/a.cpp)"
expect_eq "producao em app/ e classificada como producao" "production" "$(fact prod:app/a.cpp)"
expect_eq "cabecalho de producao em domain/ e classificado como producao" "production" "$(fact prod:domain/a.hpp)"
expect_eq "teste em tests/harness e classificado como teste" "test" "$(fact test:tests/harness/t.cpp)"
expect_eq "teste em tests/selftest e classificado como teste" "test" "$(fact test:tests/selftest/t.cpp)"
expect_eq "CMakeLists de teste em tests/ e classificado como teste" "test" "$(fact test:tests/CMakeLists.txt)"
expect_eq "CMakeLists de fixture de teste e classificado como teste" "test" "$(fact test:tests/fixtures/werror/CMakeLists.txt)"
expect_eq "fonte de fixture em tests/ e classificada como teste (sem tests/** o mutante sobrevive)" "test" "$(fact test:tests/fixtures/sanitizers/x.cpp)"
expect_eq "tools/ e excluido do guard" "excluded" "$(fact excl:tools/tdd/run_suite.sh)"
expect_eq "build/ e excluido do guard" "excluded" "$(fact excl:build/x.cpp)"

suite_done

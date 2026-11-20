#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# The notice hook is registered in the project settings, after the existing hooks,
# which must still be there. The file reaches the container through run_in_container.sh.
# Moved out of test_notice.sh (review A8), so the settings check has its own file.
. "$(dirname "${BASH_SOURCE[0]}")/lib.sh"

SETTINGS="$REPO/.claude/settings.json"
require_file "settings.json do projeto existe" "$SETTINGS"
if python3 - "$SETTINGS" <<'PY' >/dev/null 2>&1
import json
import sys

with open(sys.argv[1], encoding="utf-8") as f:
    hooks = json.load(f)["hooks"]
pre = hooks["PreToolUse"]
assert len(pre) == 1
assert pre[0]["matcher"] == "Write|Edit|MultiEdit"
cmds = [h["command"] for h in pre[0]["hooks"]]
assert len(cmds) == 1
assert "tools/tdd/notice.py" in cmds[0]
assert cmds[0].endswith("|| true")
assert 0 < pre[0]["hooks"][0]["timeout"] <= 10
assert "SessionStart" in hooks and "UserPromptSubmit" in hooks
assert any("github-gossips" in h["command"] for h in hooks["SessionStart"][0]["hooks"])
PY
then
    pass_case "PreToolUse de notice.py registrado e hooks antigos preservados"
else
    fail_case "PreToolUse de notice.py registrado e hooks antigos preservados"
fi

suite_done

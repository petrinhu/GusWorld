#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Proof of the visible-warning channel (ADR-026, decision 8). Runs inside the
# container (L-35) with a fake HOME and a fake guard state. Every case checks the exit
# code and the stdout together, so a missing notice.py (exit 2, empty stdout) cannot
# pass a "stays mute" case, and a JSON case reads the decoded fields, not raw bytes.
. "$(dirname "${BASH_SOURCE[0]}")/lib.sh"

NOTICE="$TDD_DIR/notice.py"
require_file "notice.py existe" "$NOTICE"

CLEAN=()
CASE_ROOT=""
CASE_DIGEST=""
CASE_STATE=""
CASE_REASON=""
RC=""

# new_case: a fresh project root with its own notice.py, a fake HOME, and the digest
# that notice.py derives from the root (the same digest as the guard's state folder).
new_case() {
    local fx
    fx="$(mktemp -d /tmp/tdd-notice-XXXXXX)"
    fx="$(cd "$fx" && pwd -P)"
    CLEAN+=("$fx")
    mkdir -p "$fx/tools/tdd" "$fx/home"
    cp "$NOTICE" "$fx/tools/tdd/notice.py" 2>/dev/null || true
    CASE_ROOT="$fx"
    CASE_DIGEST="$(printf '%s' "$fx" | sha256sum | cut -c1-16)"
    CLEAN+=("/var/tmp/gusworld-tdd/$CASE_DIGEST")
    CASE_STATE="$fx/home/.claude/state/tdd-guard/$CASE_DIGEST/last-run.json"
    CASE_REASON="/var/tmp/gusworld-tdd/$CASE_DIGEST/last-reason"
    export HOME="$fx/home"
}

# write_state TEXT: the guard's last-run.json, written as given.
write_state() {
    mkdir -p "$(dirname "$CASE_STATE")"
    printf '%s' "$1" > "$CASE_STATE"
}

# write_reason TEXT: the last-reason file that run_suite.sh writes.
write_reason() {
    mkdir -p "$(dirname "$CASE_REASON")"
    printf '%s' "$1" > "$CASE_REASON"
}

# run_notice: runs the hook the way Claude Code does (a PreToolUse payload on stdin).
# Stdout goes to $CASE_ROOT/out.txt and the exit code to RC.
run_notice() {
    echo '{"tool_name":"Edit","tool_input":{"file_path":"core/a.cpp"}}' |
        (cd "$CASE_ROOT" && python3 tools/tdd/notice.py) > "$CASE_ROOT/out.txt" 2> "$CASE_ROOT/err.txt"
    RC=$?
}

# expect_mute NAME: exit 0 and an empty stdout, in one comparison.
expect_mute() {
    run_notice
    expect_eq "$1" "rc0|" "rc$RC|$(cat "$CASE_ROOT/out.txt")"
}

# expect_text NAME TEXT: exit 0, and the stdout is the PreToolUse JSON whose
# systemMessage and additionalContext are exactly TEXT.
expect_text() {
    run_notice
    if [ "$RC" -eq 0 ] && python3 - "$CASE_ROOT/out.txt" "$2" <<'PY' >/dev/null 2>&1
import json
import sys

with open(sys.argv[1], encoding="utf-8") as f:
    data = json.load(f)
want = sys.argv[2]
hso = data["hookSpecificOutput"]
assert hso["hookEventName"] == "PreToolUse"
assert data["systemMessage"] == want
assert hso["additionalContext"] == want
assert set(hso) == {"hookEventName", "additionalContext"}
PY
    then
        pass_case "$1"
    else
        fail_case "$1" "rc $RC, saida: $(head -c 300 "$CASE_ROOT/out.txt")"
    fi
}

# expect_notice NAME REASON: the "liberada sem teste" text, with REASON as the motive.
expect_notice() {
    expect_text "$1" "TDD guard não rodou (falha de infraestrutura): $2. A edição foi liberada sem teste."
}

# BLOCK_JSON: the text for an unreadable state that the guard reads as no state (it
# blocks production edits until the runner writes the state again).
BLOCK_JSON="TDD guard sem estado legível: as edições de produção ficam bloqueadas até o runner gravar o estado de novo (JSON inválido)."

# Case 1: ran=false with a last-reason from run_suite.sh. The "sem execucao: " prefix
# is the invoker's own; the motive is what follows it.
new_case
write_state '{"ran": false, "reason": "comando nao executavel (exit 126)"}'
write_reason 'sem execucao: docker: daemon fora do ar'
expect_notice "ran=false gera JSON com o motivo do run_suite" "docker: daemon fora do ar"

# Case 2: ran=true stays mute, with exit 0.
new_case
write_state '{"ran": true, "has_red": true, "exit_code": 1}'
expect_mute "ran=true fica mudo"

# Case 3: no state at all stays mute, with exit 0.
new_case
expect_mute "sem estado fica mudo"

# Case 4: a state that is not valid JSON. The guard reads it as no state and blocks
# production edits, so the notice says so (the blocking text, not "liberada").
new_case
write_state '{"ran": tru'
expect_text "JSON invalido bloqueia: texto de bloqueio" "$BLOCK_JSON"

# Case 4b: bytes that are not UTF-8 are the same case: the guard reads them as no state.
new_case
mkdir -p "$(dirname "$CASE_STATE")"
printf '\377\376{' > "$CASE_STATE"
expect_text "UTF-8 invalido bloqueia: texto de bloqueio" "$BLOCK_JSON"

# Case 5: a state that is valid JSON but not an object. The guard fails open on it
# (an AttributeError inside it, caught), so the notice keeps the "liberada" text.
new_case
write_state '[1, 2]'
expect_notice "JSON que nao e objeto avisa como liberada" "estado ilegível"

# Case 6: a state path that cannot be read as a file (a directory). The guard fails
# open here (WARN_STATE_IO), so the notice keeps the "liberada" text and never blocks.
new_case
mkdir -p "$CASE_STATE"
expect_notice "estado que nao le como arquivo avisa como liberada e sai com 0" "estado ilegível"

# Case 7: quotes, accents, and a backslash in the reason survive the JSON encoding.
new_case
write_state '{"ran": false}'
write_reason 'sem execucao: "lei" é ação \ fim'
expect_notice "motivo com aspas, acentos e barra sai escapado" '"lei" é ação \ fim'

# Case 8: no last-reason, so the motive comes from the state's own reason field.
new_case
write_state '{"ran": false, "reason": "comando nao executavel (exit 126)"}'
expect_notice "sem last-reason usa o reason do estado" "comando nao executavel (exit 126)"

# Case 9: a stale last-reason (green, from an earlier run) does not supply the motive
# of a timeout. The state's reason wins when last-reason is not a no-run record.
new_case
write_state '{"ran": false, "reason": "timeout"}'
write_reason 'verde: suite passou (veredito 0)'
expect_notice "last-reason antigo nao vence o motivo do estado" "timeout"

# Case 10: ran=false with no reason anywhere names the motive as unknown.
new_case
write_state '{"ran": false}'
expect_notice "ran=false sem motivo vira desconhecido" "desconhecido"

# Case 11: a multi-line reason is collapsed to one line.
new_case
write_state '{"ran": false}'
write_reason 'sem execucao: docker: linha um
linha dois'
expect_notice "motivo de varias linhas vira uma linha" "docker: linha um linha dois"

# Case 12: a state without the ran key counts as not run (the guard reads it the same
# way), so it warns.
new_case
write_state '{"reason": "timeout"}'
expect_notice "estado sem a chave ran avisa, como o guard" "timeout"

# Case 13: the hook is registered in the project settings, after the existing hooks,
# which must still be there. The file reaches the container through run_in_container.sh.
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

for d in "${CLEAN[@]}"; do
    rm -rf -- "$d"
done

suite_done

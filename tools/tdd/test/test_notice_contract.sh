#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Contract test of the notice against the REAL bigtech plugin (review item A1 and A2): the
# notice must mirror the guard's own answer. The plugin's hooks come as a copy under
# $REPO/plugin/hooks (run_in_container.sh), so the real tdd_guard.evaluate runs here.
# If the copy is missing, the case fails: a run without the real plugin is "nao
# executado" and never counts as green (L-35).
. "$(dirname "${BASH_SOURCE[0]}")/lib.sh"

NOTICE="$TDD_DIR/notice.py"
require_file "notice.py existe" "$NOTICE"
PLUGIN="$REPO/plugin"
require_file "plugin real (copia do contêiner) existe" "$PLUGIN/hooks/tdd_guard.py"

FX="$(mktemp -d /tmp/tdd-contract-XXXXXX)"
FX="$(cd "$FX" && pwd -P)"
PROJ="$FX/proj"
mkdir -p "$PROJ/.claude" "$PROJ/core" "$PROJ/docs" "$PROJ/tools/tdd" "$FX/home/.claude/plugins"
printf '%s\n' '{"enabled": true, "production_globs": ["core/**"], "test_globs": ["tests/**"], "exclude_globs": ["docs/**", "tools/**"], "test_command": "true"}' > "$PROJ/.claude/tdd-guard.json"
printf '%s' "{\"version\": 2, \"plugins\": {\"bigtech@petrinhu\": [{\"scope\": \"user\", \"installPath\": \"$PLUGIN\"}]}}" > "$FX/home/.claude/plugins/installed_plugins.json"
export HOME="$FX/home"
DIGEST="$(printf '%s' "$PROJ" | sha256sum | cut -c1-16)"
STATE="$FX/home/.claude/state/tdd-guard/$DIGEST/last-run.json"
REASON="/var/tmp/gusworld-tdd/$DIGEST/last-reason"
NO_RUN="TDD guard não rodou (falha de infraestrutura): docker: daemon fora do ar. A edição foi liberada sem teste."

# run_notice REL [VAR=VAL...]: the hook for the project file PROJ/REL, with optional
# environment for the guard. Stdout to out.txt, stderr to err.txt, exit code to RC.
run_notice() {
    local rel="$1"
    shift
    printf '{"tool_name":"Edit","tool_input":{"file_path":"%s"},"cwd":"%s"}' \
        "$PROJ/$rel" "$PROJ" |
        env "$@" python3 "$NOTICE" > "$FX/out.txt" 2> "$FX/err.txt"
    RC=$?
}

# mute_case NAME REL [VAR=VAL...]: exit 0 and an empty stdout.
mute_case() {
    local name="$1" rel="$2"
    shift 2
    run_notice "$rel" "$@"
    local got="rc$RC|$(cat "$FX/out.txt")"
    if [ "$got" = "rc0|" ]; then
        pass_case "$name"
    else
        fail_case "$name" "esperado [rc0|], veio [$got], stderr: $(head -c 300 "$FX/err.txt")"
    fi
}

# text_case NAME TEXT REL [VAR=VAL...]: exit 0, and the JSON with TEXT in both fields.
text_case() {
    local name="$1" text="$2" rel="$3"
    shift 3
    run_notice "$rel" "$@"
    if [ "$RC" -eq 0 ] && python3 - "$FX/out.txt" "$text" <<'PY' >/dev/null 2>&1
import json
import sys

with open(sys.argv[1], encoding="utf-8") as f:
    data = json.load(f)
hso = data["hookSpecificOutput"]
assert hso["hookEventName"] == "PreToolUse"
assert data["systemMessage"] == sys.argv[2]
assert hso["additionalContext"] == sys.argv[2]
PY
    then
        pass_case "$name"
    else
        fail_case "$name" "rc $RC, saida: $(head -c 300 "$FX/out.txt"), stderr: $(head -c 300 "$FX/err.txt")"
    fi
}

mkdir -p "$(dirname "$REASON")"
printf '%s' 'sem execucao: docker: daemon fora do ar' > "$REASON"

# set_state CONTENT: the guard's state file, with its folder created first. (Before this
# fix the folder was missing, the write failed, and the mute cases passed for free.)
set_state() {
    mkdir -p "$(dirname "$STATE")"
    printf '%s' "$1" > "$STATE"
}

# state_is CONTENT: the state file exists and holds exactly CONTENT.
state_is() {
    [ -f "$STATE" ] && [ "$(cat "$STATE")" = "$1" ]
}

# mute_with_state NAME REL CONTENT [VAR=VAL...]: a mute case runs only after its state
# precondition holds, so a missing or wrong file can never make the case pass by itself.
mute_with_state() {
    local name="$1" rel="$2" content="$3"
    shift 3
    if state_is "$content"; then
        mute_case "$name" "$rel" "$@"
    else
        fail_case "$name" "precondicao: estado [$content] nao gravado em $STATE"
    fi
}

RUN_STATE='{"ran": false, "reason": "timeout", "ts": 0}'

# Control first: a production file with ran=false must warn with the real guard. The
# state precondition is checked before the warning, and only then do the mute cases
# below prove anything.
set_state "$RUN_STATE"
if state_is "$RUN_STATE"; then
    text_case "controle: producao com ran=false avisa com o motivo do run_suite" "$NO_RUN" core/a.cpp
else
    fail_case "controle: estado ran=false gravado antes do aviso" "estado nao gravado em $STATE"
fi

# The guard's own silences: the notice mirrors them and stays mute. Each runs with the
# ran=false state present, so the silence is the guard's classification, not a missing file.
mute_with_state "TDD_GUARD=off fica mudo" core/a.cpp "$RUN_STATE" TDD_GUARD=off
mute_with_state "TDD_PHASE=refactor fica mudo" core/a.cpp "$RUN_STATE" TDD_PHASE=refactor
mute_with_state "edicao em docs/ fica muda" docs/x.md "$RUN_STATE"
mute_with_state "edicao em tools/ fica muda" tools/tdd/x.py "$RUN_STATE"

# The guard blocks production without a state, so the notice stays mute and does not
# repeat the block (the guard speaks for itself, exit 2). The precondition is that the
# state file does not exist.
rm -f -- "$STATE"
if [ -e "$STATE" ]; then
    fail_case "producao sem estado: precondicao, arquivo nao deve existir" "$STATE existe"
else
    mute_case "producao sem estado: guard bloqueia, notice mudo" core/a.cpp
fi

set_state 'null'
mute_with_state "estado null com producao: guard bloqueia, notice mudo" core/a.cpp 'null'

set_state '{"ran": true, "has_red": true, "exit_code": 1}'
mute_with_state "ran=true com vermelho: guard deixa passar, notice mudo" core/a.cpp \
    '{"ran": true, "has_red": true, "exit_code": 1}'

# F2: the notice resolves the plugin as Claude Code loads it. The marketplace petrinhu is a
# `directory` source whose installLocation is a symlink, and the plugin loads in place, so
# the marketplace copy is the one that runs. HOME2 also holds a STALE installPath copy that
# answers "block" (code 2) to every production edit: a warning proves the stale copy was not
# the one used.
MKT="$FX/mkt"
MKT_LINK="$FX/mkt-link"
HOME2="$FX/home2"
STALE="$FX/stale"
mkdir -p "$MKT/.claude-plugin" "$MKT/hooks" "$STALE/hooks" "$HOME2/.claude/plugins" "$HOME2/.claude/state"
cp "$PLUGIN/hooks/"*.py "$MKT/hooks/"
printf '%s' '{"plugins": [{"name": "bigtech", "source": "./"}]}' > "$MKT/.claude-plugin/marketplace.json"
ln -s "$MKT" "$MKT_LINK"
cp "$PLUGIN/hooks/tdd_common.py" "$STALE/hooks/tdd_common.py"
printf '%s\n' 'def evaluate(data, env):' '    return 2, "copia antiga"' > "$STALE/hooks/tdd_guard.py"
printf '%s' "{\"version\": 2, \"plugins\": {\"bigtech@petrinhu\": [{\"scope\": \"user\", \"installPath\": \"$STALE\"}]}}" > "$HOME2/.claude/plugins/installed_plugins.json"
printf '%s' "{\"petrinhu\": {\"source\": {\"source\": \"directory\", \"path\": \"$MKT\"}, \"installLocation\": \"$MKT_LINK\"}}" > "$HOME2/.claude/plugins/known_marketplaces.json"
mkdir -p "$HOME2/.claude/state/tdd-guard/$DIGEST"
printf '%s' '{"ran": false, "reason": "timeout", "ts": 0}' > "$HOME2/.claude/state/tdd-guard/$DIGEST/last-run.json"

resolved="$(env HOME="$HOME2" python3 -B -c 'import sys; sys.path.insert(0, sys.argv[1]); import notice; print(notice.plugin_hooks_dir())' "$TDD_DIR" 2>/dev/null)"
expect_eq "F2: resolvedor segue o symlink do marketplace petrinhu" "$(realpath "$MKT")/hooks" "$resolved"
text_case "F2: marketplace directory vence o installPath antigo, que bloquearia" "$NO_RUN" core/a.cpp HOME="$HOME2"

rm -rf -- "$FX" "/var/tmp/gusworld-tdd/$DIGEST"
suite_done

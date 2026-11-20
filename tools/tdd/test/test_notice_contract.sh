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
printf '%s' '{"ran": false, "reason": "timeout", "ts": 0}' > "$STATE"
printf '%s' 'sem execucao: docker: daemon fora do ar' > "$REASON"

# Control first: a production file with ran=false must warn with the real guard. Only
# then do the mute cases below prove anything.
text_case "controle: producao com ran=false avisa com o motivo do run_suite" "$NO_RUN" core/a.cpp

# The guard's own silences: the notice mirrors them and stays mute.
mute_case "TDD_GUARD=off fica mudo" core/a.cpp TDD_GUARD=off
mute_case "TDD_PHASE=refactor fica mudo" core/a.cpp TDD_PHASE=refactor
mute_case "edicao em docs/ fica muda" docs/x.md
mute_case "edicao em tools/ fica muda" tools/tdd/x.py

# The guard blocks production without a state, so the notice stays mute and does not
# repeat the block (the guard speaks for itself, exit 2).
rm -f -- "$STATE"
mute_case "producao sem estado: guard bloqueia, notice mudo" core/a.cpp

printf '%s' 'null' > "$STATE"
mute_case "estado null com producao: guard bloqueia, notice mudo" core/a.cpp

printf '%s' '{"ran": true, "has_red": true, "exit_code": 1}' > "$STATE"
mute_case "ran=true com vermelho: guard deixa passar, notice mudo" core/a.cpp

rm -rf -- "$FX" "/var/tmp/gusworld-tdd/$DIGEST"
suite_done

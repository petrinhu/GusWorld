#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Notice of the visible-warning channel (ADR-026, decision 8), with a FAKE bigtech
# plugin: its tdd_guard.evaluate answers by FAKE_GUARD_MODE, while tdd_common is the
# real one (copied from the plugin under plugin/hooks). The real guard is covered by
# test_notice_contract.sh. Runs inside the container (L-35) with a fake HOME.
#
# Every case checks the exit code and the stdout together. A "mute" case is only
# trusted after a control on the same fixture has warned, so a silent notice cannot
# pass by itself.
. "$(dirname "${BASH_SOURCE[0]}")/lib.sh"

NOTICE="$TDD_DIR/notice.py"
require_file "notice.py existe" "$NOTICE"
PLUGIN_TDD_COMMON="$REPO/plugin/hooks/tdd_common.py"
require_file "tdd_common.py do plugin existe (copia do contêiner)" "$PLUGIN_TDD_COMMON"

PLUGIN_TEXT="plugin bigtech não encontrado ou ilegível: o guard de TDD pode não estar rodando"
INTERNAL_TEXT="aviso do guard falhou internamente"

CLEAN=()
CASE_ROOT=""
CASE_DIGEST=""
CASE_STATE=""
CASE_REASON=""
CASE_FX=""
RC=""

# new_case: a fresh fixture. proj/ is the project (with .claude/tdd-guard.json), home/ is
# the fake HOME, plugin/ is the fake plugin with the real tdd_common and a fake
# tdd_guard that answers by FAKE_GUARD_MODE. The registry points at plugin/.
new_case() {
    local fx
    fx="$(mktemp -d /tmp/tdd-notice-XXXXXX)"
    fx="$(cd "$fx" && pwd -P)"
    CLEAN+=("$fx")
    CASE_FX="$fx"
    mkdir -p "$fx/proj/.claude" "$fx/proj/core" "$fx/home/.claude/plugins" "$fx/plugin/hooks"
    printf '%s\n' '{"enabled": true, "production_globs": ["core/**"], "test_globs": ["tests/**"], "exclude_globs": ["docs/**", "tools/**"], "test_command": "true"}' > "$fx/proj/.claude/tdd-guard.json"
    cp "$PLUGIN_TDD_COMMON" "$fx/plugin/hooks/tdd_common.py"
    cat > "$fx/plugin/hooks/tdd_guard.py" <<'PY'
# Fake guard for the notice tests. The answer depends only on FAKE_GUARD_MODE.
def evaluate(data, env):
    mode = env.get("FAKE_GUARD_MODE", "warn")
    if mode == "warn":
        return 0, "TDD guard: a ultima execucao de teste nao rodou (motivo: fake)."
    if mode == "silent":
        return 0, ""
    if mode == "block":
        return 2, "TDD guard: bloqueio fake."
    if mode == "io":
        return 0, "TDD guard: falha de I/O ao ler o estado (boom)."
    raise RuntimeError("guard falhou de proposito")
PY
    printf '%s' "{\"version\": 2, \"plugins\": {\"bigtech@petrinhu\": [{\"scope\": \"user\", \"installPath\": \"$fx/plugin\"}]}}" > "$fx/home/.claude/plugins/installed_plugins.json"
    CASE_ROOT="$fx/proj"
    CASE_DIGEST="$(printf '%s' "$CASE_ROOT" | sha256sum | cut -c1-16)"
    CLEAN+=("/var/tmp/gusworld-tdd/$CASE_DIGEST")
    CASE_STATE="$fx/home/.claude/state/tdd-guard/$CASE_DIGEST/last-run.json"
    CASE_REASON="/var/tmp/gusworld-tdd/$CASE_DIGEST/last-reason"
    export HOME="$fx/home"
    export FAKE_GUARD_MODE=warn
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

# run_notice: runs the hook the way Claude Code does, a PreToolUse payload on stdin for
# a production file of the fixture project. Stdout goes to out.txt, stderr to err.txt.
run_notice() {
    printf '{"tool_name":"Edit","tool_input":{"file_path":"%s"},"cwd":"%s"}' \
        "$CASE_ROOT/core/a.cpp" "$CASE_ROOT" |
        python3 "$NOTICE" > "$CASE_FX/out.txt" 2> "$CASE_FX/err.txt"
    RC=$?
}

# expect_mute NAME: exit 0 and an empty stdout, in one comparison.
expect_mute() {
    run_notice
    local got="rc$RC|$(cat "$CASE_FX/out.txt")"
    if [ "$got" = "rc0|" ]; then
        pass_case "$1"
    else
        fail_case "$1" "esperado [rc0|], veio [$got], stderr: $(head -c 300 "$CASE_FX/err.txt")"
    fi
}

# expect_text NAME TEXT: exit 0, and the stdout is the PreToolUse JSON whose
# systemMessage and additionalContext are exactly TEXT.
expect_text() {
    run_notice
    if [ "$RC" -eq 0 ] && python3 - "$CASE_FX/out.txt" "$2" <<'PY' >/dev/null 2>&1
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
        fail_case "$1" "rc $RC, saida: $(head -c 300 "$CASE_FX/out.txt"), stderr: $(head -c 300 "$CASE_FX/err.txt")"
    fi
}

# expect_notice NAME REASON: the "liberada sem teste" text, with REASON as the motive.
expect_notice() {
    expect_text "$1" "TDD guard não rodou (falha de infraestrutura): $2. A edição foi liberada sem teste."
}

# Case 1: the guard warns and the last-reason is fresh (no state ts newer than it): its
# motive from run_suite.sh is shown.
new_case
write_state '{"ran": false, "reason": "x", "ts": 0}'
write_reason 'sem execucao: docker: daemon fora do ar'
expect_notice "aviso com last-reason fresco mostra o motivo do run_suite" "docker: daemon fora do ar"

# Case 2: control on one digest. The guard warns, then goes silent, then blocks. Silent
# and blocking answers must be mute only after the warning on the same digest.
new_case
write_state '{"ran": false, "reason": "timeout", "ts": 0}'
expect_notice "controle: modo warn avisa" "timeout"
export FAKE_GUARD_MODE=silent
expect_mute "modo silent do guard fica mudo"
export FAKE_GUARD_MODE=block
expect_mute "guard bloqueando (code 2) deixa o notice mudo"
export FAKE_GUARD_MODE=warn

# Case 3: the same control with a state that is absent or null. The real guard blocks
# these (no state), so the notice stays mute. Here the fake answers block for them.
new_case
write_state '{"ran": false, "reason": "timeout", "ts": 0}'
expect_notice "controle: ran=false avisa antes de remover o estado" "timeout"
rm -f -- "$CASE_STATE"
export FAKE_GUARD_MODE=block
expect_mute "sem estado, guard bloqueia, notice mudo"
export FAKE_GUARD_MODE=warn

# Case 4: a last-reason OLDER than the state's ts is ignored. The state's reason wins.
new_case
write_state '{"ran": false, "reason": "timeout", "ts": 4102444800}'
write_reason 'sem execucao: docker: X'
expect_notice "last-reason mais antigo que o estado e ignorado" "timeout"

# Case 5: no last-reason: the state's own reason is the motive.
new_case
write_state '{"ran": false, "reason": "comando nao executavel (exit 126)", "ts": 0}'
expect_notice "sem last-reason usa o reason do estado" "comando nao executavel (exit 126)"

# Case 6: no reason anywhere, the motive is named as unknown.
new_case
write_state '{"ran": false, "ts": 0}'
expect_notice "sem motivo em lugar nenhum vira desconhecido" "desconhecido"

# Case 7: the guard warns about an I/O failure with no state: its message is the motive
# (without the final period, which the notice adds back).
new_case
export FAKE_GUARD_MODE=io
expect_notice "aviso de I/O usa a mensagem do guard como motivo" "TDD guard: falha de I/O ao ler o estado (boom)"
export FAKE_GUARD_MODE=warn

# Case 8: the guard raises. The notice does not stay silent: it says it failed inside.
new_case
export FAKE_GUARD_MODE=raise
expect_text "excecao do guard vira aviso interno, nunca mudo" "$INTERNAL_TEXT"
export FAKE_GUARD_MODE=warn

# Case 9: the plugin is not registered. The plugin text is shown, always.
new_case
rm -f -- "$CASE_FX/home/.claude/plugins/installed_plugins.json"
expect_text "plugin sem registro avisa com o texto do plugin" "$PLUGIN_TEXT"

# Case 10: the registry is not valid JSON. The plugin text is shown.
new_case
printf '%s' '{' > "$CASE_FX/home/.claude/plugins/installed_plugins.json"
expect_text "registro ilegivel avisa com o texto do plugin" "$PLUGIN_TEXT"

# Case 11: the plugin directory is writable by others. It is not trusted, so the plugin
# text is shown.
new_case
chmod 0777 "$CASE_FX/plugin"
expect_text "plugin gravavel por outros avisa com o texto do plugin" "$PLUGIN_TEXT"

# Case 12: last-reason is a symlink. Its target is never read: the motive is the fixed
# text, and the notice still warns.
new_case
write_state '{"ran": false, "ts": 0}'
printf '%s' 'sem execucao: alvo de ataque' > "$CASE_FX/target.txt"
mkdir -p "$(dirname "$CASE_REASON")"
ln -s "$CASE_FX/target.txt" "$CASE_REASON"
expect_notice "last-reason simbolico vira motivo indisponivel e ainda avisa" "motivo indisponível"

# Case 13: the digest folder is writable by group or others. The reason is not trusted,
# and the notice still warns. (The owner check, uid against getuid(), is not testable
# here: the container runs as builder and cannot create a file of another owner.)
new_case
write_state '{"ran": false, "ts": 0}'
write_reason 'sem execucao: docker: daemon fora do ar'
chmod 0777 "$(dirname "$CASE_REASON")"
expect_notice "pasta do digest gravavel por outros vira motivo indisponivel" "motivo indisponível"

# Case 14: the reason file itself is writable by group or others.
new_case
write_state '{"ran": false, "ts": 0}'
write_reason 'sem execucao: docker: daemon fora do ar'
chmod 0666 "$CASE_REASON"
expect_notice "arquivo gravavel por outros vira motivo indisponivel" "motivo indisponível"

# Case 15: a motive longer than 200 characters is cut to 200, and the notice still warns.
new_case
write_state '{"ran": false, "ts": 0}'
write_reason "sem execucao: $(printf 'a%.0s' $(seq 1 250))"
expect_notice "motivo com mais de 200 caracteres sai com 200" "$(printf 'a%.0s' $(seq 1 200))"

# Case 16: ANSI escape sequences and control characters are removed.
new_case
write_state '{"ran": false, "ts": 0}'
write_reason "$(printf 'sem execucao: \033[31mdocker\033[0m\007 falhou\000 aqui')"
expect_notice "controle e ANSI saem do motivo" "docker falhou aqui"

# Case 17: quotes, accents, and a backslash survive the JSON encoding.
new_case
write_state '{"ran": false, "ts": 0}'
write_reason 'sem execucao: "lei" é ação \ fim'
expect_notice "motivo com aspas, acentos e barra sai escapado" '"lei" é ação \ fim'

# Case 18: a multi-line motive is collapsed to one line.
new_case
write_state '{"ran": false, "ts": 0}'
write_reason 'sem execucao: docker: linha um
linha dois'
expect_notice "motivo de varias linhas vira uma linha" "docker: linha um linha dois"

for d in "${CLEAN[@]}"; do
    rm -rf -- "$d"
done

suite_done

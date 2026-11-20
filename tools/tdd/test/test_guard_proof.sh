#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Proof that the TDD guard bites (ADR-026, C8 fatia 6, camada N2). It runs the real
# bigtech plugin (tdd_guard.py, tdd_runner.py and tdd_common.py, copied under
# $REPO/plugin/hooks), the literal hook command of .claude/settings.json, and a fixture
# project whose test_command is a sentinel: exit $(cat verdict). The fixture runs no code
# of the game. HOME is fake, so the real ~/.claude is never read or written.
# Runs inside the container (L-35). mutate_guard.sh runs it again on mutated copies.
# Every case prints "ok" or "FALHA" with its name, and a mutant is killed by a name that
# passed on the baseline and failed (or vanished) on the copy. Case names carry no colon.
. "$(dirname "${BASH_SOURCE[0]}")/lib.sh"

PLUGIN_HOOKS="$REPO/plugin/hooks"
SETTINGS="$REPO/.claude/settings.json"
require_file "guard real do plugin existe (copia do contêiner)" "$PLUGIN_HOOKS/tdd_guard.py"
require_file "runner real do plugin existe (copia do contêiner)" "$PLUGIN_HOOKS/tdd_runner.py"
require_file "tdd_common real do plugin existe (copia do contêiner)" "$PLUGIN_HOOKS/tdd_common.py"
require_file "notice.py existe" "$TDD_DIR/notice.py"
require_file "settings do projeto existe" "$SETTINGS"
if [ "$FAILS" -gt 0 ]; then
    suite_done
fi

FX="$(mktemp -d /tmp/tdd-proof-XXXXXX)"
FX="$(cd "$FX" && pwd -P)"
PROJ="$FX/proj"
mkdir -p "$PROJ/.claude" "$PROJ/core" "$PROJ/tools/tdd" "$FX/empty" "$FX/home/.claude/plugins"
printf '%s\n' '{"enabled": true, "production_globs": ["core/**"], "test_globs": ["tests/**"], "exclude_globs": ["docs/**", "tools/**", ".claude/**"], "test_command": "exit $(cat verdict)", "timeout_sec": 30}' > "$PROJ/.claude/tdd-guard.json"
cp "$TDD_DIR/notice.py" "$PROJ/tools/tdd/notice.py"
printf '%s' "{\"version\": 2, \"plugins\": {\"bigtech@petrinhu\": [{\"scope\": \"user\", \"installPath\": \"$REPO/plugin\"}]}}" > "$FX/home/.claude/plugins/installed_plugins.json"
export HOME="$FX/home"

DIGEST="$(printf '%s' "$PROJ" | sha256sum | cut -c1-16)"
STATE="$HOME/.claude/state/tdd-guard/$DIGEST/last-run.json"
REASON_DIR="/var/tmp/gusworld-tdd/$DIGEST"
REASON="$REASON_DIR/last-reason"
NO_RUN_126="TDD guard não rodou (falha de infraestrutura): comando nao executavel (exit 126). A edição foi liberada sem teste."

# The literal hook command, read from the project settings and never retyped.
HOOK_CMD="$(python3 -I - "$SETTINGS" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as f:
    print(json.load(f)["hooks"]["PreToolUse"][0]["hooks"][0]["command"], end="")
PY
)"
case "$HOOK_CMD" in
    *tools/tdd/notice.py*) pass_case "comando literal do hook extraido do settings" ;;
    *) fail_case "comando literal do hook extraido do settings" "veio [$HOOK_CMD]" ;;
esac
case "$HOOK_CMD" in
    *'|| true') pass_case "comando literal do hook termina com || true" ;;
    *) fail_case "comando literal do hook termina com || true" "veio [$HOOK_CMD]" ;;
esac

# payload REL: the PreToolUse payload of a Write to PROJ/REL, with the session cwd.
payload() {
    printf '{"tool_name":"Write","tool_input":{"file_path":"%s"},"cwd":"%s"}' "$PROJ/$1" "$PROJ"
}

# verdict_set N: the sentinel's answer, read by the test_command as exit $(cat verdict).
verdict_set() {
    printf '%s' "$1" > "$PROJ/verdict"
}

# guard_on REL: the real guard's answer for a Write of PROJ/REL. Sets G_RC and G_ERR.
# The guard speaks on two channels: stderr when it blocks (exit 2), and stdout as a JSON
# warning when it fails open with exit 0 (tdd_guard.py 2.0.0, emite_aviso). G_ERR holds both,
# so a check reads the message whichever channel the guard used.
guard_on() {
    payload "$1" | env -u TDD_GUARD -u TDD_PHASE python3 -I "$PLUGIN_HOOKS/tdd_guard.py" \
        > "$FX/guard.out" 2> "$FX/guard.err"
    G_RC=$?
    G_ERR="$(cat "$FX/guard.err" "$FX/guard.out")"
}

# runner_on REL: the real runner runs the sentinel and writes last-run.json.
runner_on() {
    payload "$1" | env -u TDD_GUARD python3 -I "$PLUGIN_HOOKS/tdd_runner.py" \
        > "$FX/runner.out" 2> "$FX/runner.err"
}

# notice_on REL [DIR]: the literal hook command, as Claude Code runs it, with
# CLAUDE_PROJECT_DIR set to DIR (default: the fixture project). Sets N_RC and N_OUT.
notice_on() {
    payload "$1" | env CLAUDE_PROJECT_DIR="${2:-$PROJ}" bash -c "$HOOK_CMD" \
        > "$FX/notice.out" 2> "$FX/notice.err"
    N_RC=$?
    N_OUT="$(cat "$FX/notice.out")"
}

# notice_is FILE TEXT: the JSON of the notice has PreToolUse and TEXT in both fields.
notice_is() {
    python3 -I - "$1" "$2" <<'PY' >/dev/null 2>&1
import json
import sys

with open(sys.argv[1], encoding="utf-8") as f:
    data = json.load(f)
hso = data["hookSpecificOutput"]
assert hso["hookEventName"] == "PreToolUse"
assert data["systemMessage"] == sys.argv[2]
assert hso["additionalContext"] == sys.argv[2]
PY
}

# state_field KEY: the value of KEY in last-run.json, as JSON; AUSENTE when the key or
# the file is missing, ILEGIVEL when the file cannot be read.
state_field() {
    python3 -I - "$STATE" "$1" <<'PY' 2>/dev/null || echo ILEGIVEL
import json
import sys

try:
    with open(sys.argv[1], encoding="utf-8") as f:
        data = json.load(f)
    print(json.dumps(data.get(sys.argv[2], "AUSENTE")))
except (OSError, ValueError):
    print("ILEGIVEL")
PY
}

# has TEXT NEEDLE: true when TEXT contains NEEDLE as plain text.
has() {
    case "$1" in
        *"$2"*) return 0 ;;
        *) return 1 ;;
    esac
}

# measure PASSO: what the guard, the state and the notice answered at this step.
measure() {
    local st="AUSENTE"
    if [ -e "$STATE" ]; then
        st="$(head -c 300 "$STATE" 2>/dev/null || echo ILEGIVEL)"
    fi
    echo "medida $1: guard_rc=$G_RC guard_err=[$(printf '%s' "$G_ERR" | head -c 200)] state=[$st] aviso_rc=$N_RC aviso=[$(printf '%s' "$N_OUT" | head -c 300)]"
}

# --- Passo 1: estado ausente. O guard bloqueia (2) e o aviso fica mudo. ---
rm -f -- "$STATE"
verdict_set 1
guard_on core/a.cpp
notice_on core/a.cpp
measure passo1
if [ ! -e "$STATE" ]; then
    pass_case "precondicao: estado ausente antes do passo 1"
else
    fail_case "precondicao: estado ausente antes do passo 1" "estado existe em $STATE"
fi
expect_eq "estado ausente: guard bloqueia com codigo 2" 2 "$G_RC"
if has "$G_ERR" "nenhuma execucao"; then
    pass_case "estado ausente: guard diz que nenhuma execucao foi registrada"
else
    fail_case "estado ausente: guard diz que nenhuma execucao foi registrada" "stderr [$G_ERR]"
fi
expect_eq "estado ausente: aviso mudo com saida vazia" "0|" "$N_RC|$N_OUT"

# --- Passo 2: sentinela 1 (vermelho). O runner grava has_red; o guard libera; aviso mudo. ---
verdict_set 1
runner_on core/a.cpp
guard_on core/a.cpp
notice_on core/a.cpp
measure passo2
expect_eq "sentinela 1: estado registra ran verdadeiro" true "$(state_field ran)"
expect_eq "sentinela 1: estado registra has_red verdadeiro" true "$(state_field has_red)"
expect_eq "sentinela 1: estado registra exit_code 1" 1 "$(state_field exit_code)"
expect_eq "sentinela 1: guard libera com codigo 0" 0 "$G_RC"
expect_eq "sentinela 1: guard sem mensagem no stderr" "" "$G_ERR"
expect_eq "sentinela 1: aviso mudo com saida vazia" "0|" "$N_RC|$N_OUT"

# --- Passo 3: sentinela 0 (verde). O guard bloqueia (2) com a mensagem de tudo verde. ---
verdict_set 0
runner_on core/a.cpp
guard_on core/a.cpp
notice_on core/a.cpp
measure passo3
expect_eq "sentinela 0: estado registra has_red falso" false "$(state_field has_red)"
expect_eq "sentinela 0: estado registra exit_code 0" 0 "$(state_field exit_code)"
expect_eq "sentinela 0: guard bloqueia com codigo 2" 2 "$G_RC"
if has "$G_ERR" "toda verde"; then
    pass_case "sentinela 0: guard diz que a suite esta toda verde"
else
    fail_case "sentinela 0: guard diz que a suite esta toda verde" "stderr [$G_ERR]"
fi
expect_eq "sentinela 0: aviso mudo, o guard ja fala" "0|" "$N_RC|$N_OUT"

# --- Passo 4: sentinela 126 (nao rodou). O guard libera (0) e o aviso CHEGA ao agente. ---
verdict_set 126
runner_on core/a.cpp
guard_on core/a.cpp
notice_on core/a.cpp
measure passo4
expect_eq "sentinela 126: estado registra ran falso" false "$(state_field ran)"
expect_eq "sentinela 126: estado registra o motivo do sentinela" '"comando nao executavel (exit 126)"' "$(state_field reason)"
expect_eq "sentinela 126: guard libera com codigo 0" 0 "$G_RC"
if has "$G_ERR" "nao rodou"; then
    pass_case "sentinela 126: guard avisa que a ultima execucao nao rodou"
else
    fail_case "sentinela 126: guard avisa que a ultima execucao nao rodou" "stderr [$G_ERR]"
fi
expect_eq "sentinela 126: aviso sai com codigo 0" 0 "$N_RC"
if notice_is "$FX/notice.out" "$NO_RUN_126"; then
    pass_case "sentinela 126: aviso traz additionalContext e systemMessage com o motivo do estado"
else
    fail_case "sentinela 126: aviso traz additionalContext e systemMessage com o motivo do estado" "saida [$N_OUT]"
fi

# --- Passo 5: last-reason antigo e recente contra o ts do estado (M7). ---
TS="$(state_field ts)"
mkdir -p -m 700 "$REASON_DIR"
if [[ "$TS" =~ ^[0-9]+$ ]]; then
    pass_case "precondicao: estado do passo 4 tem ts numerico"
else
    fail_case "precondicao: estado do passo 4 tem ts numerico" "ts [$TS]"
fi
printf '%s' 'sem execucao: docker: daemon fora do ar' > "$REASON"
touch -d "@$((TS - 100))" -- "$REASON"
notice_on core/a.cpp
measure passo5-velho
if has "$N_OUT" "comando nao executavel (exit 126)" && ! has "$N_OUT" "daemon fora do ar"; then
    pass_case "last-reason mais velho que o ts e ignorado: vale o motivo do estado"
else
    fail_case "last-reason mais velho que o ts e ignorado: vale o motivo do estado" "saida [$N_OUT]"
fi
touch -d "@$((TS + 5))" -- "$REASON"
notice_on core/a.cpp
measure passo5-novo
if has "$N_OUT" "daemon fora do ar" && ! has "$N_OUT" "comando nao executavel"; then
    pass_case "last-reason igual ou mais novo que o ts vale como motivo"
else
    fail_case "last-reason igual ou mais novo que o ts vale como motivo" "saida [$N_OUT]"
fi

# --- Passo 6 (R2): estado ilegivel e last-reason antigo. Defeito conhecido: o aviso deve
# mostrar a mensagem de I/O do guard, e NAO o motivo antigo. Sai vermelho no HEAD atual.
chmod 000 "$STATE"
if [ "$(id -u)" -ne 0 ] && [ ! -r "$STATE" ]; then
    pass_case "R2 precondicao: estado ilegivel para o usuario do teste"
else
    fail_case "R2 precondicao: estado ilegivel para o usuario do teste" "uid $(id -u) ou estado legivel"
fi
printf '%s' 'sem execucao: docker: daemon fora do ar' > "$REASON"
touch -d "@$((TS + 5))" -- "$REASON"
guard_on core/a.cpp
expect_eq "R2 precondicao: guard abre com codigo 0 diante de estado ilegivel" 0 "$G_RC"
if has "$G_ERR" "falha de I/O ao ler o estado"; then
    pass_case "R2 precondicao: guard relata a falha de I/O"
else
    fail_case "R2 precondicao: guard relata a falha de I/O" "stderr [$G_ERR]"
fi
notice_on core/a.cpp
measure passo6-R2
expect_eq "R2: aviso sai com codigo 0" 0 "$N_RC"
if has "$N_OUT" "falha de I/O ao ler o estado"; then
    pass_case "R2: aviso mostra a mensagem de I/O do guard"
else
    fail_case "R2: aviso mostra a mensagem de I/O do guard" "saida [$N_OUT]"
fi
if has "$N_OUT" "daemon fora do ar"; then
    fail_case "R2: aviso nao repete o motivo antigo do last-reason" "saida [$N_OUT]"
else
    pass_case "R2: aviso nao repete o motivo antigo do last-reason"
fi
chmod 600 "$STATE" 2>/dev/null

# --- Passo 7 (M5): sem notice.py, o comando literal sai com 0 e nao imprime nada. ---
notice_on core/a.cpp "$FX/empty"
measure passo7-sem-notice
expect_eq "notice.py ausente: comando literal sai com codigo 0" 0 "$N_RC"
expect_eq "notice.py ausente: comando literal nao imprime nada" "" "$N_OUT"

chmod 600 "$STATE" 2>/dev/null
rm -rf -- "$FX" "$REASON_DIR"
suite_done

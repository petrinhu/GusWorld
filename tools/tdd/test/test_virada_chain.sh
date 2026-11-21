#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# The chain of the virada (C8 fatia 7, ADR-026 decision 8 and criteria a, b and c). The real
# runner of the bigtech plugin (tdd_runner.py) calls run_suite.sh in a fixture project that
# carries the shipped .claude/tdd-guard.json; the guard and the notice then read the state.
#   (a) run_suite.sh, called by the runner, records ran=true with the suite's verdict.
#       The container's answer comes from fake_docker: the chain is real, the verdict is not.
#   (b) without docker, or with a cold container, run_suite.sh answers 126 and the runner
#       records ran=false with a reason.
#   (c) in that case the notice shows the motive of run_suite.sh (last-reason), not a
#       generic text.
# Runs inside the container (L-35). The container has no docker, so the docker of each case
# is the fake one. HOME is fake, and every case has its own fixture and its own state.
. "$(dirname "${BASH_SOURCE[0]}")/lib.sh"

CONFIG="$REPO/.claude/tdd-guard.json"
RUNNER="$REPO/plugin/hooks/tdd_runner.py"
require_file "tdd-guard.json do projeto existe" "$CONFIG"
require_file "runner real do plugin existe (copia do contêiner)" "$RUNNER"
require_file "plugin real (copia do contêiner) existe" "$REPO/plugin/hooks/tdd_guard.py"
if [ "$FAILS" -gt 0 ]; then
    suite_done
fi

CLEAN=()
cleanup() {
    local d
    for d in "${CLEAN[@]}"; do
        rm -rf -- "$d"
    done
}
trap cleanup EXIT

# new_case INFO_RC CTR_RC: a fixture with the shipped config, a fake HOME with the plugin
# registered, and a bin dir that holds only the fake docker. Sets FX, STATE, REASON, CTR.
new_case() {
    local bin
    FX="$(make_fixture)"
    FX="$(cd "$FX" && pwd -P)"
    CLEAN+=("$FX")
    mkdir -p "$FX/.claude" "$FX/home/.claude/plugins"
    cp "$CONFIG" "$FX/.claude/tdd-guard.json"
    printf '%s' "{\"version\": 2, \"plugins\": {\"bigtech@petrinhu\": [{\"scope\": \"user\", \"installPath\": \"$REPO/plugin\"}]}}" \
        > "$FX/home/.claude/plugins/installed_plugins.json"
    export HOME="$FX/home"
    bin="$(mktemp -d /tmp/tdd-virada-bin-XXXXXX)"
    CLEAN+=("$bin")
    cp "$FIXTURES/fake_docker.sh" "$bin/docker"
    chmod +x "$bin/docker"
    export PATH="$bin:$PATH"
    export FAKE_DOCKER_LOG="$bin/docker.log"
    export FAKE_DOCKER_STDIN="$bin/exec-stdin.tar"
    export FAKE_DOCKER_INFO_RC="$1"
    export FAKE_CTR_RC="$2"
    export FAKE_EXEC_RC=0
    export FAKE_EXEC_OUT="tdd-verdict: 0"
    setup_fake_image "$FX"
    local digest
    digest="$(printf '%s' "$FX" | sha256sum | cut -c1-16)"
    CTR="gusworld-tdd-$digest"
    STATE="$HOME/.claude/state/tdd-guard/$digest/last-run.json"
    REASON="/var/tmp/gusworld-tdd/$digest/last-reason"
    CLEAN+=("/var/tmp/gusworld-tdd/$digest")
}

# run_runner: the real runner on a production file of the fixture, as the hook gives it.
run_runner() {
    printf '{"tool_name":"Edit","tool_input":{"file_path":"%s"},"cwd":"%s"}' \
        "$FX/core/a.cpp" "$FX" \
        | env -u TDD_GUARD python3 -I "$RUNNER" > "$FX/runner.out" 2> "$FX/runner.err"
}

# run_notice: the notice of the fixture for the same production file. Sets N_TEXT to the
# systemMessage, or to MUDO when the notice stays silent.
run_notice() {
    printf '{"tool_name":"Edit","tool_input":{"file_path":"%s"},"cwd":"%s"}' \
        "$FX/core/a.cpp" "$FX" \
        | python3 "$FX/tools/tdd/notice.py" > "$FX/notice.out" 2> "$FX/notice.err"
    N_TEXT="$(python3 -I - "$FX/notice.out" <<'PY' 2>/dev/null || echo MUDO
import json
import sys

with open(sys.argv[1], encoding="utf-8") as f:
    raw = f.read()
if not raw.strip():
    print("MUDO")
else:
    print(json.loads(raw)["systemMessage"])
PY
)"
}

# state_field KEY: the value of KEY in the runner's last-run.json, as JSON.
state_field() {
    python3 -I - "$STATE" "$1" <<'PY' 2>/dev/null || echo ILEGIVEL
import json
import sys

try:
    with open(sys.argv[1], encoding="utf-8") as f:
        print(json.dumps(json.load(f).get(sys.argv[2], "AUSENTE")))
except FileNotFoundError:
    print("AUSENTE")
PY
}

NO_RUN_PREFIX="TDD guard não rodou (falha de infraestrutura): "
NO_RUN_SUFFIX=". A edição foi liberada sem teste."

# Case (a): the suite passes through the real chain.
new_case 0 0
run_runner
expect_eq "(a) runner gravou ran=true" "true" "$(state_field ran)"
expect_eq "(a) runner gravou o exit_code do veredito verde" "0" "$(state_field exit_code)"
expect_eq "(a) runner gravou has_red=false no verde" "false" "$(state_field has_red)"
expect_eq "(a) runner gravou o test_command do projeto" '"bash tools/tdd/run_suite.sh"' "$(state_field command)"

new_case 0 0
export FAKE_EXEC_RC=1 FAKE_EXEC_OUT="tdd-verdict: 1"
run_runner
expect_eq "(a) vermelho do contêiner chega ao runner como has_red=true" "true" "$(state_field has_red)"
expect_eq "(a) vermelho do contêiner chega ao runner com exit_code 1" "1" "$(state_field exit_code)"

# Case (b1): the docker daemon is absent. run_suite.sh answers 126 and the runner says so.
new_case 1 0
expect_rc "(b) sem daemon, run_suite.sh responde 126" 126 bash "$FX/tools/tdd/run_suite.sh"
run_runner
expect_eq "(b) sem daemon, runner gravou ran=false" "false" "$(state_field ran)"
expect_eq "(b) sem daemon, runner gravou o motivo de 126" '"comando nao executavel (exit 126)"' "$(state_field reason)"

# Case (c1): the same run, read by the notice. The motive is run_suite's own text.
run_notice
expect_eq "(c) sem daemon, aviso mostra o motivo do run_suite (last-reason)" \
    "${NO_RUN_PREFIX}docker: daemon do docker inacessivel${NO_RUN_SUFFIX}" "$N_TEXT"

# Case (b2): a cold container. run_suite.sh answers 126 with the container's reason.
new_case 0 1
expect_rc "(b) contêiner frio, run_suite.sh responde 126" 126 bash "$FX/tools/tdd/run_suite.sh"
run_runner
expect_eq "(b) contêiner frio, runner gravou ran=false" "false" "$(state_field ran)"

# Case (c2): the notice shows the container's reason, not a generic one.
run_notice
expect_eq "(c) contêiner frio, aviso mostra o motivo do contêiner" \
    "${NO_RUN_PREFIX}contêiner ${CTR} ausente (contêiner frio)${NO_RUN_SUFFIX}" "$N_TEXT"

suite_done

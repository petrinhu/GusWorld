#!/usr/bin/env bash
# tools/crash_journal_check.sh
#
# Entregavel 1 (defense-in-depth #1) do par anti-crash/UAF do GusWorld.
# REATIVO e barato: verifica se algum dos NOSSOS binarios (gusengine_*_tests,
# gusworld_app) coredumpou numa janela curta e recente, lendo o journal do
# systemd via `coredumpctl`. Roda no pre-commit LOCAL do repo
# (.git/hooks/pre-commit), que o shim global core.hooksPath
# (~/.claude/githooks/pre-commit -> _chain.sh) encadeia automaticamente.
#
# NUNCA bloqueia o commit: so avisa (WARN alto) e SEMPRE sai 0. Nasceu de um
# UAF real (dominio combate) que a suite verde escondeu e que outra sessao
# so pegou revirando coredump a mao depois do fato - este hook automatiza
# esse "revirar" e poe o aviso na cara de quem esta commitando.
#
# --- por que so avisa, nunca bloqueia --------------------------------------
# Um coredump recente pode ser de OUTRO trabalho concorrente na mesma
# maquina (outra sessao, outro binario que por acaso bate no regex, um
# crash-test proposital que ja foi corrigido mas ainda nao foi limpo do
# journal). Bloquear o commit por causa disso e falso-positivo caro; avisar
# alto e deixar o humano/agente decidir e o trade-off certo pra um hook de
# pre-commit (o gate PESADO e de verdade e o ASan no pre-push, entregavel 2).
#
# --- por que a janela e curta E ancorada -----------------------------------
# SINCE = o mais recente entre (a) o timestamp do commit anterior e (b) HA 15
# MINUTOS. Ou seja: nunca olhamos mais de 15 min pro passado (evita pegar
# coredump de uma sessao de horas atras, ja investigado/corrigido, e
# reabrir alarme falso toda vez que alguem commita), e se os commits estao
# vindo rapido (< 15 min entre eles) a janela fica ainda mais apertada
# (so desde o ultimo commit) - o que e exatamente o intervalo em que um
# crash NOVO, ainda nao visto, teria acontecido.
#
# --- escapes p/ testes-de-crash intencionais --------------------------------
# Provocamos crash de proposito sob ASan pra provar que a suite pega o bug
# (esse e literalmente o objetivo do entregavel 2). Isso NAO pode virar
# falso-alarme no commit seguinte que registra o teste corrigido:
#   - env GUSWORLD_SKIP_CRASH_CHECK=1              (uma execucao)
#   - marker file .git/.gusworld_skip_crash_check  (ate remover; pratico
#     pra uma sessao inteira de autoria de teste-de-crash). Fica dentro de
#     .git/ de proposito: nunca versionado, nunca vaza pro commit.
#
# --- cross-platform (DOCUMENTADO aqui, NAO implementado nesta rodada) ------
# So Linux + systemd-coredump por ora (unico ambiente de dev/CI atual do
# projeto). Gemeos locais para quando/se portarmos:
#   - macOS: crash reports ficam em
#     ~/Library/Logs/DiagnosticReports/<binario>-<data>-<hash>.ips (ou
#     /Library/Logs/DiagnosticReports/ pra processos de sistema). Listar por
#     mtime + nome do binario e o equivalente a `coredumpctl list --since`;
#     nao ha journal centralizado, e arquivo por crash.
#   - Windows: Windows Error Reporting (WER). Equivalente via PowerShell:
#     `Get-WinEvent -LogName Application -FilterXPath
#     "*[System[Provider[@Name='Application Error'] and TimeCreated[timediff(@SystemTime) <= 900000]]]"`
#     (janela de 900000ms = 15min, mesmo cap desta versao Linux), ou
#     inspecionar %LOCALAPPDATA%\CrashDumps se o app tiver LocalDumps
#     habilitado no registro.
#   Se algum dia isso for implementado, o padrao e um script irmao
#   (tools/crash_journal_check_macos.sh / _windows.ps1) com a MESMA logica
#   (janela curta ancorada, warn-only, escape via env/marker) - nao inflar
#   este arquivo com branches de OS.
#
# Uso: chamado sem argumentos pelo hook. Pode ser rodado a mao pra
# depurar (idempotente, so leitura).

set -uo pipefail
# SEM -e de proposito: este script so pode terminar atraves dos `exit 0`
# explicitos abaixo. Um hook de aviso que morre cedo por causa de um
# comando auxiliar falhando (coredumpctl ausente, jq com bug, etc.) e pior
# que um hook que so deixa de avisar - nunca pode travar o commit.

ROOT="$(git rev-parse --show-toplevel 2>/dev/null)" || exit 0
MARKER="$ROOT/.git/.gusworld_skip_crash_check"

# --- escapes -----------------------------------------------------------
if [ "${GUSWORLD_SKIP_CRASH_CHECK:-0}" = "1" ]; then
    exit 0
fi
if [ -e "$MARKER" ]; then
    exit 0
fi

# --- so faz sentido no Linux com systemd-coredump ativo -----------------
if [ "$(uname -s)" != "Linux" ]; then
    exit 0
fi
if ! command -v coredumpctl >/dev/null 2>&1; then
    exit 0
fi
if [ ! -d /run/systemd/system ]; then
    # systemd nao e o init deste host (container minimal, WSL1, etc.): sem
    # journal de coredump pra consultar.
    exit 0
fi

# --- janela curta e ancorada ---------------------------------------------
NOW_EPOCH=$(date +%s)
CAP_EPOCH=$((NOW_EPOCH - 900)) # 15 min, teto duro

LAST_COMMIT_EPOCH=$(git -C "$ROOT" log -1 --format=%ct 2>/dev/null)
LAST_COMMIT_EPOCH=${LAST_COMMIT_EPOCH:-0}

if [ "$LAST_COMMIT_EPOCH" -gt "$CAP_EPOCH" ] 2>/dev/null; then
    SINCE_EPOCH=$LAST_COMMIT_EPOCH
else
    SINCE_EPOCH=$CAP_EPOCH
fi
SINCE_ISO=$(date -d "@$SINCE_EPOCH" '+%Y-%m-%d %H:%M:%S' 2>/dev/null) || exit 0

# --- nossos binarios -------------------------------------------------------
# gusengine_domain_tests, gusengine_app_tests (Catch2), gusworld_app (jogo).
# O regex casa qualquer build dir (linux-release, asan, glintfx-on, etc.) -
# filtramos pelo NOME do binario, nao pelo caminho.
PATTERN='gusengine_[a-z_]*_tests|gusworld_app'

# --- consulta: jq > python3 > texto simples, na ordem de robustez ----------
RAW_JSON=$(coredumpctl list --json=short --no-legend --since="$SINCE_ISO" 2>/dev/null)
RC=$?
if [ $RC -ne 0 ] || [ -z "$RAW_JSON" ]; then
    RAW_JSON="[]"
fi

HITS=""
MODE="raw"
if command -v jq >/dev/null 2>&1; then
    MODE="jq"
    HITS=$(printf '%s' "$RAW_JSON" | jq -r --arg pat "$PATTERN" \
        '.[]? | select((.exe // "") | test($pat)) | "\(.pid)\t\(.sig)\t\(.exe)"' 2>/dev/null)
elif command -v python3 >/dev/null 2>&1; then
    MODE="python3"
    HITS=$(printf '%s' "$RAW_JSON" | python3 -c '
import sys, json, re
pat = re.compile(r"gusengine_[a-z_]*_tests|gusworld_app")
try:
    data = json.load(sys.stdin)
except Exception:
    data = []
for e in data:
    exe = e.get("exe") or ""
    if pat.search(exe):
        print("%s\t%s\t%s" % (e.get("pid", "?"), e.get("sig", "?"), exe))
' 2>/dev/null)
else
    MODE="raw"
    HITS=$(coredumpctl list --no-legend --since="$SINCE_ISO" 2>/dev/null | grep -E "$PATTERN" || true)
fi

[ -z "$HITS" ] && exit 0

# --- achou: avisa ALTO, NAO bloqueia --------------------------------------
echo "" >&2
echo "############################################################" >&2
echo "# AVISO (nao bloqueia o commit): coredump recente de um     #" >&2
echo "# binario NOSSO desde $SINCE_ISO                            #" >&2
echo "############################################################" >&2
if [ "$MODE" = "raw" ]; then
    echo "$HITS" | sed 's/^/  /' >&2
else
    echo "$HITS" | while IFS=$'\t' read -r pid sig exe; do
        printf '  PID %s  sig=%s  %s\n' "${pid:-?}" "${sig:-?}" "${exe:-?}" >&2
    done
fi
echo "" >&2
echo "Investigue antes do proximo push:" >&2
echo "  coredumpctl gdb <PID>" >&2
echo "  (ou: coredumpctl list --since=\"$SINCE_ISO\")" >&2
echo "" >&2
echo "Testando crash de proposito (ASan etc)? Suprima este aviso com:" >&2
echo "  GUSWORLD_SKIP_CRASH_CHECK=1 git commit ..." >&2
echo "  ou: touch \"$MARKER\"   (lembre de remover depois)" >&2
echo "" >&2

exit 0

#!/usr/bin/env bash
# F4-2.5 - roteiro DETERMINISTICO ponta-a-ponta do input novo (glintfx::App real ->
# GlintfxInput -> InputMapper -> movimento observavel), com ISOLAMENTO OBRIGATORIO
# da sessao viva do lider (ver o brief da fatia / docs/window-modes.md). NUNCA
# injeta tecla nenhuma sem antes PROVAR o isolamento (passo 5) - se a prova falhar,
# PARA e sai sem tocar em xdotool key nenhum.
#
# Isolamento: Xvfb PROPRIO (display livre, escolhido abaixo) + XDG_RUNTIME_DIR
# PROPRIO (dir fresco, chmod 700) SO no processo do probe (o Xvfb em si nao precisa
# de XDG_RUNTIME_DIR nenhum - e um servidor X, nao um cliente Wayland) +
# XDG_SESSION_TYPE=x11 (a variavel que o GLFW honra pra nao tentar Wayland) +
# WAYLAND_DISPLAY explicitamente REMOVIDO do ambiente do probe.
#
# Uso:
#   ./run_e2e.sh                              # build (se preciso) + roda o roteiro
#   GUSWORLD_E2E_SKIP_BUILD=1 ./run_e2e.sh    # pula o cmake --build (probe ja compilado)
#
# Saida: 0 se a sequencia de EDGE do probe.log bater EXATAMENTE com expected_edges.txt
# (na ordem, campo a campo), 1 em qualquer outro caso (falha de isolamento, timeout,
# mismatch de sequencia). Nunca decide por "olho" - so por diff de texto.

set -u
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
GUSENGINE_ROOT="$SCRIPT_DIR/../../.."
BUILD_DIR="$GUSENGINE_ROOT/build/glintfx_input_e2e_scratch"
PROBE_BIN="$BUILD_DIR/glintfx_input_e2e_probe"
EXPECTED_FILE="$SCRIPT_DIR/expected_edges.txt"

fail() {
    echo "FAIL: $*" >&2
    exit 1
}

# --- 0) build (a menos que pulado) -----------------------------------------------
if [ "${GUSWORLD_E2E_SKIP_BUILD:-0}" != "1" ]; then
    export TMPDIR=/var/tmp
    cmake -S "$SCRIPT_DIR" -B "$BUILD_DIR" -G Ninja >/var/tmp/gusworld_e2e_cmake_configure.log 2>&1 \
        || { cat /var/tmp/gusworld_e2e_cmake_configure.log >&2; fail "cmake configure falhou"; }
    cmake --build "$BUILD_DIR" -j"$(nproc)" \
        || fail "cmake --build falhou"
fi
[ -x "$PROBE_BIN" ] || fail "binario nao encontrado: $PROBE_BIN"

# --- 1) escolher display X11 livre + Xvfb PROPRIO ---------------------------------
XN=""
for cand in $(seq 180 220); do
    if [ ! -e "/tmp/.X11-unix/X$cand" ]; then
        XN="$cand"
        break
    fi
done
[ -n "$XN" ] || fail "nenhum display X11 livre entre :180 e :220"

RUN_ID="$$_$(date +%s)"
LOG_ROOT="/var/tmp/gusworld_glintfx_input_e2e_run/$RUN_ID"
mkdir -p "$LOG_ROOT" || fail "mkdir $LOG_ROOT"
XVFB_LOG="$LOG_ROOT/xvfb.log"

Xvfb ":$XN" -screen 0 1024x768x24 >"$XVFB_LOG" 2>&1 &
XVFB_PID=$!
disown "$XVFB_PID" 2>/dev/null || true

cleanup() {
    kill -9 "$XVFB_PID" >/dev/null 2>&1 || true
    if [ -n "${PROBE_PID:-}" ]; then
        kill -9 "$PROBE_PID" >/dev/null 2>&1 || true
    fi
}
trap cleanup EXIT

for _ in $(seq 1 30); do
    [ -S "/tmp/.X11-unix/X$XN" ] && break
    sleep 0.2
done
[ -S "/tmp/.X11-unix/X$XN" ] || fail "Xvfb :$XN nao subiu (ver $XVFB_LOG)"
echo "OK: Xvfb isolado em :$XN (pid=$XVFB_PID)"

# --- 2) XDG_RUNTIME_DIR PROPRIO, SO pro processo do probe -------------------------
RUNDIR="$(mktemp -d /var/tmp/gusworld_e2e_xdgrun.XXXXXX)" || fail "mktemp XDG_RUNTIME_DIR"
chmod 700 "$RUNDIR"

LOG_DIR="$LOG_ROOT/probe"
mkdir -p "$LOG_DIR" || fail "mkdir $LOG_DIR"

# --- 3) sobe o probe, ISOLADO --------------------------------------------------
env -u WAYLAND_DISPLAY \
    XDG_RUNTIME_DIR="$RUNDIR" \
    XDG_SESSION_TYPE=x11 \
    DISPLAY=":$XN" \
    GUSWORLD_E2E_SECONDS="${GUSWORLD_E2E_SECONDS:-60}" \
    GUSWORLD_E2E_LOGDIR="$LOG_DIR" \
    "$PROBE_BIN" >"$LOG_DIR/stdout.log" 2>&1 &
PROBE_PID=$!
disown "$PROBE_PID" 2>/dev/null || true
echo "OK: probe subiu pid=$PROBE_PID (XDG_RUNTIME_DIR=$RUNDIR DISPLAY=:$XN)"

for _ in $(seq 1 50); do
    [ -f "$LOG_DIR/READY" ] && break
    kill -0 "$PROBE_PID" 2>/dev/null || fail "probe morreu antes do READY (ver $LOG_DIR/stdout.log e $LOG_DIR/probe.log)"
    sleep 0.2
done
[ -f "$LOG_DIR/READY" ] || fail "probe nunca escreveu READY em $LOG_DIR"
echo "OK: probe pronto (READY)"

# --- 4) PROVA DE ISOLAMENTO (item 5 do brief, OBRIGATORIA antes de QUALQUER tecla) -
WAYLAND_FD_COUNT="$(lsof -p "$PROBE_PID" 2>/dev/null | grep -c libwayland)"
echo "PROVA-1: lsof -p $PROBE_PID | grep -c libwayland = $WAYLAND_FD_COUNT (precisa ser 0)"

# Layout REAL de `ss -xp` (verificado empiricamente nesta fatia, cat -A + awk
# NF - 9 campos sempre, mesmo com o nome do path ausente):
#   u_str ESTAB 0 0  <local_addr>  <local_inode>  <peer_addr>  <peer_inode>  users:(...)
#     $1    $2  $3 $4    $5           $6              $7          $8            $9
# No lado CLIENTE (nosso probe, socket anonimo/connect()), <local_addr>=* - so o
# <peer_inode> ($8, aqui NF-1 com NF=9) identifica a quem ele fala. No lado
# SERVIDOR (Xvfb, socket NOMEADO/bind()), <local_addr>=@/tmp/.X11-unix/X<N> e
# <local_inode> ($6, aqui NF-3) e o numero que o peer_inode do cliente precisa bater.
SS_PROBE_LINE="$(ss -xp 2>/dev/null | grep -F "\"glintfx_input_e\",pid=$PROBE_PID")"
PROBE_PEER_INODE=""
if [ -n "$SS_PROBE_LINE" ]; then
    PROBE_PEER_INODE="$(echo "$SS_PROBE_LINE" | awk '{print $(NF-1)}')"
fi

XN_LOCAL_INODE=""
SS_XN_LINE="$(ss -xp 2>/dev/null | grep -F "@/tmp/.X11-unix/X$XN ")"
if [ -n "$SS_XN_LINE" ]; then
    XN_LOCAL_INODE="$(echo "$SS_XN_LINE" | awk '{print $(NF-3)}')"
fi

X0_LOCAL_INODES="$(ss -xp 2>/dev/null | grep -F "@/tmp/.X11-unix/X0 " | awk '{print $(NF-3)}')"
X0_DANGER="nao"
if [ -n "$PROBE_PEER_INODE" ]; then
    for _x0i in $X0_LOCAL_INODES; do
        [ "$_x0i" = "$PROBE_PEER_INODE" ] && X0_DANGER="SIM-PERIGO"
    done
fi

PEER_OK="nao"
[ -n "$PROBE_PEER_INODE" ] && [ -n "$XN_LOCAL_INODE" ] && [ "$PROBE_PEER_INODE" = "$XN_LOCAL_INODE" ] && PEER_OK="SIM"

echo "PROVA-2: probe_peer_inode=$PROBE_PEER_INODE; X${XN}_local_inode=$XN_LOCAL_INODE; peer==X$XN? $PEER_OK"
echo "PROVA-2b: probe_peer_inode bate com algum socket bindado em @/tmp/.X11-unix/X0 (sessao viva)? $X0_DANGER"

if [ "$WAYLAND_FD_COUNT" != "0" ] || [ "$PEER_OK" != "SIM" ] || [ "$X0_DANGER" != "nao" ]; then
    echo "ss -xp bruto (debug):" >&2
    ss -xp 2>/dev/null | grep -F "$PROBE_PID" >&2
    kill -9 "$PROBE_PID" 2>/dev/null
    fail "PROVA DE ISOLAMENTO FALHOU - nenhuma tecla foi injetada. Ver PROVA-1/2/2b acima."
fi
echo "OK: isolamento provado (libwayland=0, peer=:$XN, sem peer em :0) - prosseguindo com o roteiro."

# --- 5) foco + roteiro determinístico (10 eventos xdotool = 10 EDGE esperadas) ----
WIN="$(DISPLAY=":$XN" xdotool search --name "glintfx_input_e2e_probe" 2>/dev/null | tail -1)"
[ -n "$WIN" ] || fail "janela do probe nao encontrada via xdotool search em :$XN"
DISPLAY=":$XN" xdotool windowfocus --sync "$WIN" || fail "windowfocus falhou"
FOCUSED="$(DISPLAY=":$XN" xdotool getwindowfocus 2>/dev/null)"
[ "$FOCUSED" = "$WIN" ] || fail "janela nao ganhou foco (getwindowfocus=$FOCUSED, esperado=$WIN)"
echo "OK: janela $WIN focada em :$XN"

DWELL=0.35
step() {
    DISPLAY=":$XN" xdotool "$@" || fail "xdotool $* falhou"
    sleep "$DWELL"
}

echo "--- roteiro (10 eventos) ---"
step keydown w
step keydown d
step keyup w
step keydown Shift_L
step keyup d
step keyup Shift_L
step keydown s
step keydown a
step keyup s
step keyup a
echo "OK: roteiro injetado"

# pede pro probe sair (Q, MESMO caminho de producao dentro do probe - edge-detect
# manual, ver glintfx_input_e2e_probe.cpp).
DISPLAY=":$XN" xdotool keydown q
sleep 0.2
DISPLAY=":$XN" xdotool keyup q

for _ in $(seq 1 30); do
    kill -0 "$PROBE_PID" 2>/dev/null || break
    sleep 0.2
done
if kill -0 "$PROBE_PID" 2>/dev/null; then
    echo "AVISO: probe nao saiu sozinho apos Q - matando (kill -9)." >&2
    kill -9 "$PROBE_PID" 2>/dev/null
fi

# --- 6) verificacao POR SCRIPT (nao por olho) -------------------------------------
ACTUAL_FILE="$LOG_DIR/actual_edges.txt"
grep '^EDGE ' "$LOG_DIR/probe.log" \
    | sed -E 's/.*dx=(-?[0-9]+) dy=(-?[0-9]+) run=([01]).*/\1 \2 \3/' \
    > "$ACTUAL_FILE"

EXPECTED_CLEAN="$LOG_DIR/expected_clean.txt"
grep -vE '^\s*#' "$EXPECTED_FILE" | grep -vE '^\s*$' > "$EXPECTED_CLEAN"

echo "--- esperado ---"
cat "$EXPECTED_CLEAN"
echo "--- observado (probe.log) ---"
cat "$ACTUAL_FILE"

if diff -u "$EXPECTED_CLEAN" "$ACTUAL_FILE" >"$LOG_DIR/diff.txt" 2>&1; then
    echo "PASS: sequencia de EDGE bate EXATAMENTE com expected_edges.txt ($LOG_DIR)"
    exit 0
else
    echo "--- diff (esperado vs observado) ---"
    cat "$LOG_DIR/diff.txt"
    fail "sequencia de EDGE NAO bate com expected_edges.txt ($LOG_DIR)"
fi

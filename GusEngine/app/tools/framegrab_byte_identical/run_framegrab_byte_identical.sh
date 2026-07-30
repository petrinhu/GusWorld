#!/usr/bin/env bash
# FRAMEGRAB-7-SITIOS - roda o framegrab_byte_identical_probe com ISOLAMENTO OBRIGATORIO
# da sessao viva do lider. Receita COPIADA INTEIRA de
# app/tools/framegrab_ordem/run_framegrab.sh (a invocacao canonica da casa pra este tipo
# de probe grafico) e so entao adaptada pro nome do binario/saida - copiar metade e como
# um agente daqui abriu janela na sessao viva por 2 minutos.
#
# Uso:
#   ./run_framegrab_byte_identical.sh                                # build (se preciso) + roda
#   GUSWORLD_FRAMEGRAB_BI_SKIP_BUILD=1 ./run_framegrab_byte_identical.sh
# Saida: os artefatos em $GUSWORLD_FRAMEGRAB_BI_OUT (default abaixo) + o stdout do probe.

set -u
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
GUSENGINE_ROOT="$SCRIPT_DIR/../../.."
BUILD_DIR="$GUSENGINE_ROOT/build/framegrab_byte_identical_scratch"
PROBE_BIN="$BUILD_DIR/framegrab_byte_identical_probe"
OUT_DIR="${GUSWORLD_FRAMEGRAB_BI_OUT:-/var/tmp/builds/claude-1000/framegrab-byte-identical/out}"

fail() {
    echo "FAIL: $*" >&2
    exit 1
}

# --- 0) build (a menos que pulado) - SEMPRE sob o lock de build da maquina + TMPDIR em
#        disco real (/tmp e tmpfs e o linker estoura nele) --------------------------------
if [ "${GUSWORLD_FRAMEGRAB_BI_SKIP_BUILD:-0}" != "1" ]; then
    export TMPDIR=/var/tmp
    flock /var/tmp/gusworld-build.lock -c \
        "cmake -S '$SCRIPT_DIR' -B '$BUILD_DIR' -G Ninja" \
        >/var/tmp/gusworld_framegrab_bi_cmake_configure.log 2>&1 \
        || { cat /var/tmp/gusworld_framegrab_bi_cmake_configure.log >&2; fail "cmake configure falhou"; }
    flock /var/tmp/gusworld-build.lock -c \
        "cmake --build '$BUILD_DIR' -j$(nproc)" \
        || fail "cmake --build falhou"
fi
[ -x "$PROBE_BIN" ] || fail "binario nao encontrado: $PROBE_BIN"

mkdir -p "$OUT_DIR" || fail "mkdir $OUT_DIR"

# --- 1) escolher display X11 livre + Xvfb PROPRIO ---------------------------------
XN=""
for cand in $(seq 220 260); do
    if [ ! -e "/tmp/.X11-unix/X$cand" ]; then
        XN="$cand"
        break
    fi
done
[ -n "$XN" ] || fail "nenhum display X11 livre entre :220 e :260"

RUN_ID="$$_$(date +%s)"
LOG_ROOT="/var/tmp/gusworld_framegrab_bi_run/$RUN_ID"
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
RUNDIR="$(mktemp -d /var/tmp/gusworld_framegrab_bi_xdgrun.XXXXXX)" || fail "mktemp XDG_RUNTIME_DIR"
chmod 700 "$RUNDIR"

# --- 3) sobe o probe, ISOLADO --------------------------------------------------
env -u WAYLAND_DISPLAY \
    XDG_RUNTIME_DIR="$RUNDIR" \
    XDG_SESSION_TYPE=x11 \
    DISPLAY=":$XN" \
    GUSWORLD_FRAMEGRAB_BI_OUT="$OUT_DIR" \
    "$PROBE_BIN" >"$LOG_ROOT/stdout.log" 2>&1 &
PROBE_PID=$!
echo "OK: probe subiu pid=$PROBE_PID (XDG_RUNTIME_DIR=$RUNDIR DISPLAY=:$XN)"

# --- 4) PROVA DE ISOLAMENTO, antes de confiar em resultado nenhum ------------------
WAYLAND_FD_COUNT="$(lsof -p "$PROBE_PID" 2>/dev/null | grep -c libwayland)"
echo "PROVA-1: lsof -p $PROBE_PID | grep -c libwayland = $WAYLAND_FD_COUNT (precisa ser 0)"

SS_PROBE_LINE="$(ss -xp 2>/dev/null | grep -F "pid=$PROBE_PID")"
PROBE_PEER_INODE=""
if [ -n "$SS_PROBE_LINE" ]; then
    PROBE_PEER_INODE="$(echo "$SS_PROBE_LINE" | awk '{print $(NF-1)}' | head -1)"
fi
X0_LOCAL_INODES="$(ss -xp 2>/dev/null | grep -F "@/tmp/.X11-unix/X0 " | awk '{print $(NF-3)}')"
X0_DANGER="nao"
if [ -n "$PROBE_PEER_INODE" ]; then
    for _x0i in $X0_LOCAL_INODES; do
        [ "$_x0i" = "$PROBE_PEER_INODE" ] && X0_DANGER="SIM-PERIGO"
    done
fi
echo "PROVA-2: probe_peer_inode=$PROBE_PEER_INODE"
echo "PROVA-2b: peer bate com socket bindado em @/tmp/.X11-unix/X0 (sessao viva)? $X0_DANGER"

if [ "$WAYLAND_FD_COUNT" != "0" ] || [ "$X0_DANGER" != "nao" ]; then
    ss -xp 2>/dev/null | grep -F "$PROBE_PID" >&2
    kill -9 "$PROBE_PID" 2>/dev/null
    fail "PROVA DE ISOLAMENTO FALHOU - ver PROVA-1/2b acima."
fi
echo "OK: isolamento provado (libwayland=0, sem peer em :0)"

# --- 5) espera o probe terminar (com teto de tempo) --------------------------------
RC=""
for _ in $(seq 1 150); do
    if ! kill -0 "$PROBE_PID" 2>/dev/null; then
        wait "$PROBE_PID"
        RC=$?
        break
    fi
    sleep 0.2
done
if [ -z "$RC" ]; then
    kill -9 "$PROBE_PID" 2>/dev/null
    cat "$LOG_ROOT/stdout.log" >&2
    fail "probe nao terminou em 30s - matado"
fi

echo "--- stdout do probe (rc=$RC) ---"
cat "$LOG_ROOT/stdout.log"
echo "--- artefatos em $OUT_DIR ---"
ls -l "$OUT_DIR"
echo "log completo: $LOG_ROOT/stdout.log"
exit "$RC"

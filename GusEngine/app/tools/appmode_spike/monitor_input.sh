#!/usr/bin/env bash
# app/tools/appmode_spike/monitor_input.sh (F3-SPIKE-1, scratch, NAO commitado).
#
# Instrumentacao de diagnostico pro ROTEIRO LIVE de winmodes (RUNBOOK.md deste
# diretorio) - NAO roda nada sozinho, so OBSERVA: snapshot inicial do estado de
# input do sistema + 3 capturas continuas em background (journal do compositor,
# journal do kernel, /proc/interrupts filtrado) durante a janela em que o lider
# aperta F1/F2/F3 ao vivo. Se o compositor "engasgar" (o incidente documentado
# pelo glintfx: rajada de troca de modo mata o roteamento de input do KDE/Wayland
# ate reboot), a evidencia ja esta gravada em disco ANTES do travamento visivel -
# ver a tabela de triagem no RUNBOOK.md.
#
# Uso:
#   ./monitor_input.sh <dir_de_log>
# (o RUNBOOK.md instrui a usar o MESMO dir_de_log que o probe.log/metrics.json da
# rodada, ex.: /var/tmp/gusworld_spike_appmode/<timestamp>/)
#
# Ctrl+C mata os 3 processos em background (trap) e sai.

set -uo pipefail

if [[ $# -lt 1 ]]; then
    echo "uso: $0 <dir_de_log>" >&2
    exit 1
fi

LOG_DIR="$1"
mkdir -p "$LOG_DIR"

echo "monitor_input.sh: gravando em $LOG_DIR"

# --- (1) snapshot inicial (1x, antes de qualquer transicao de modo) -----------
{
    echo "=== snapshot inicial ==="
    echo "data: $(date -Is)"
    echo "XDG_SESSION_TYPE: ${XDG_SESSION_TYPE:-<vazio>}"
    echo
    echo "--- /proc/bus/input/devices (integral) ---"
    cat /proc/bus/input/devices 2>&1
    echo
    echo "--- libinput list-devices (tolera EACCES - roda sem root; alguns"
    echo "    /dev/input/eventN podem recusar leitura, ver a saida abaixo) ---"
    libinput list-devices 2>&1
} > "$LOG_DIR/monitor_snapshot_inicial.log"
echo "monitor_input.sh: snapshot inicial gravado em monitor_snapshot_inicial.log"

# --- (2)-(4) 3 capturas continuas em background, arquivos SEPARADOS ----------
# stdbuf -oL: forca output line-buffered (mesmo espirito do fflush por-linha do
# probe.log) - sem isto, journalctl/o loop de /proc/interrupts podem ficar
# bufferizados em bloco e o tail -f do lider (ou desta sessao, revisando depois)
# ver dados atrasados/ausentes ate o buffer encher.

# (2) journal do COMPOSITOR (KDE/Wayland) - o processo cujo travamento de
# roteamento de input e o proprio incidente documentado que motiva as grades de
# seguranca.
stdbuf -oL journalctl --user -u plasma-kwin_wayland.service -f -o short-iso-precise \
    > "$LOG_DIR/monitor_journal_kwin.log" 2>&1 &
PID_KWIN=$!

# (3) journal do KERNEL - "mais fundo" que o compositor; se o kwin morre mas o
# kernel segue mudo tambem sobre o teclado/mouse, aponta pra uma falha ainda mais
# funda (driver/hardware), ver a tabela de triagem do RUNBOOK.md.
stdbuf -oL journalctl -k -f -o short-iso-precise \
    > "$LOG_DIR/monitor_journal_kernel.log" 2>&1 &
PID_KERNEL=$!

# (4) loop de 1s de /proc/interrupts (linhas dos controladores de teclado/mouse/
# touchpad reais desta maquina) + diff de /proc/bus/input/devices SO quando ele
# muda (um dispositivo sumindo do arquivo durante o roteiro = sinal forte de
# falha de driver, nao so de roteamento do compositor).
{
    PREV_DEVICES="$LOG_DIR/.monitor_devices_prev.tmp"
    cp /proc/bus/input/devices "$PREV_DEVICES" 2>/dev/null || true
    ITER=0
    while true; do
        echo "=== $(date -Is) ==="
        grep -E 'i8042|i2c_designware|ELAN|SYNA|touchpad' /proc/interrupts 2>&1
        if ! diff -q /proc/bus/input/devices "$PREV_DEVICES" >/dev/null 2>&1; then
            echo "--- /proc/bus/input/devices MUDOU (diff abaixo) ---"
            diff "$PREV_DEVICES" /proc/bus/input/devices 2>&1
            cp /proc/bus/input/devices "$PREV_DEVICES" 2>/dev/null || true
        fi
        ITER=$((ITER + 1))
        if (( ITER % 5 == 0 )); then
            sync
        fi
        sleep 1
    done
} 2>&1 | stdbuf -oL cat > "$LOG_DIR/monitor_interrupts_devices.log" &
PID_IRQ=$!

echo "monitor_input.sh: capturas rodando em background (PIDs kwin=$PID_KWIN" \
     "kernel=$PID_KERNEL irq=$PID_IRQ). Ctrl+C encerra os 3."

# --- trap: mata os 3 filhos no Ctrl+C ----------------------------------------
cleanup() {
    echo
    echo "monitor_input.sh: encerrando (Ctrl+C) - matando PIDs $PID_KWIN $PID_KERNEL $PID_IRQ"
    kill "$PID_KWIN" "$PID_KERNEL" "$PID_IRQ" 2>/dev/null || true
    wait "$PID_KWIN" "$PID_KERNEL" "$PID_IRQ" 2>/dev/null || true
    exit 0
}
trap cleanup INT TERM

wait

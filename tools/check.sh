#!/usr/bin/env bash
# check.sh - build + smoke + gate + suite do GusEngine, num comando.
#
# Espelha o "=== build + smoke + gate + suite ===" do PokemonTCGViewer,
# adaptado a stack do GusWorld (C++20 + CMake + Ninja + Catch2 + SDL3).
# Roda a cada mudanca de codigo (via hook PostToolUse) e tambem a mao.
#
# POS REPIVOT ADR-008: a camada de plataforma e SDL3 (nao mais Qt). O smoke roda o
# app em modo headless com os drivers DUMMY do SDL (sem display/audio), e o gate de
# arquitetura audita que core/ e domain/ nao incluem Qt NEM SDL.
#
# ESTAGIOS (cada um imprime seu exit; o script sai != 0 se QUALQUER um falhar):
#   BUILD : cmake --build (incremental; no-op ~0.02s, rebuild real ~1.5s)
#   SMOKE : roda o gusworld_app --smoke (N ticks + 1 render headless, sai 0)
#   GATE  : (a) arquitetura - core/ e domain/ NAO incluem Qt NEM SDL
#           (b) paridade i18n - tabela por locale (tools/i18n_parity.py)
#   SUITE : ctest (resumo "100% tests passed ... out of N")
#
# Idempotente. Reaproveita o build dir existente (so reconfigura no 1o uso).
# So Linux por enquanto (preset linux-release; Windows e fase posterior).
#
# Variaveis de ambiente uteis:
#   CHECK_QUIET=1 silencia o log de build/ctest (so mostra os marcadores e a
#                 tabela); usado pelo hook para nao poluir o transcript.
#   CHECK_MIN_I18N=<n>  alem da estrutura, exige cobertura de conteudo >= n%.

set -euo pipefail

# Raiz do repo = pai de tools/ (resolve symlink do proprio script).
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
ENGINE="$ROOT/GusEngine"
PRESET="linux-release"
APP_BIN="$ENGINE/build/$PRESET/app/gusworld_app"

QUIET="${CHECK_QUIET:-0}"
MIN_I18N="${CHECK_MIN_I18N:-}"

# Redireciona stdout de comandos verbosos quando QUIET=1.
run_log() {
    if [ "$QUIET" = "1" ]; then "$@" >/dev/null; else "$@"; fi
}

echo "=== build + smoke + gate + suite ==="

# ---------------------------------------------------------------- BUILD
# Reconfigura so se o build dir ainda nao existe (1a vez). Depois e incremental.
if [ ! -f "$ENGINE/build/$PRESET/build.ninja" ] \
   && [ ! -f "$ENGINE/build/$PRESET/Makefile" ]; then
    run_log cmake --preset "$PRESET" -S "$ENGINE" || true
fi
set +e
( cd "$ENGINE" && run_log cmake --build --preset "$PRESET" )
BUILD=$?
set -e
echo "BUILD=$BUILD"
if [ "$BUILD" != "0" ]; then
    # build quebrado: smoke/suite nao fazem sentido; gate de arquitetura ainda
    # roda (e estatico) para ja apontar se a causa foi Qt vazando em core/domain.
    echo "(build falhou; pulando SMOKE e SUITE)"
fi

# ---------------------------------------------------------------- SMOKE
# O app ABRE JANELA no modo normal (entraria no loop e travaria o check sem
# display). Por isso o smoke roda o modo HEADLESS do app:
#   --smoke  => inicializa tudo, roda N ticks do loop logico + 1 render OFFSCREEN
#              (Render2dSdl em modo headless, sem GPU/display), imprime resumo e
#              sai 0.
# SDL_VIDEODRIVER=dummy / SDL_AUDIODRIVER=dummy => o SDL nao tenta abrir display
# nem audio (vale no CI sem X/Wayland). timeout como cinto de seguranca: se algum
# dia o smoke travar, falha rapido em vez de pendurar o hook/CI.
SMOKE=0
if [ "$BUILD" = "0" ]; then
    if [ -x "$APP_BIN" ]; then
        set +e
        SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
            run_log timeout 60 "$APP_BIN" --smoke
        SMOKE=$?
        set -e
    else
        echo "(smoke: $APP_BIN inexistente)"
        SMOKE=1
    fi
    echo "SMOKE=$SMOKE"
fi

# ---------------------------------------------------------------- GATE
# (a) Arquitetura: core/ e domain/ sao POCO; nenhum #include <Q...> NEM <SDL...>
#     (ADR-008). Mesma logica do .forgejo/workflows/ci.yml (sincronia de proposito).
GATE_ARCH=0
if grep -rnE '#[[:space:]]*include[[:space:]]*<(Q[A-Za-z]|SDL)' \
        "$ENGINE/core" "$ENGINE/domain" 2>/dev/null; then
    echo "GATE(arch): include de Qt ou SDL encontrado em core/ ou domain/ (POCO)."
    GATE_ARCH=1
else
    [ "$QUIET" = "1" ] || echo "GATE(arch): OK (sem Qt nem SDL em core/ ou domain/)."
fi

# (b) Paridade i18n: tabela por locale (faltando/extra/dup reprovam; % so exibe).
set +e
if [ -n "$MIN_I18N" ]; then
    python3 "$ROOT/tools/i18n_parity.py" --min "$MIN_I18N"
else
    python3 "$ROOT/tools/i18n_parity.py"
fi
GATE_I18N=$?
set -e

GATE=0
[ "$GATE_ARCH" = "0" ] && [ "$GATE_I18N" = "0" ] || GATE=1
echo "GATE=$GATE"

# ---------------------------------------------------------------- SUITE
SUITE=0
if [ "$BUILD" = "0" ]; then
    set +e
    # outputOnFailure ja vem do testPreset; aqui so capturamos o resumo.
    ( cd "$ENGINE" && ctest --preset "$PRESET" 2>&1 ) | tee /tmp/.gusworld_ctest.$$ \
        | grep -E "tests passed|tests failed|Total Test" || true
    SUITE=${PIPESTATUS[0]}
    set -e
    gio trash /tmp/.gusworld_ctest.$$ 2>/dev/null || true
fi
echo "SUITE=$SUITE"

# ---------------------------------------------------------------- VEREDITO
RC=0
[ "$BUILD" = "0" ] || RC=1
[ "$SMOKE" = "0" ] || RC=1
[ "$GATE"  = "0" ] || RC=1
[ "$SUITE" = "0" ] || RC=1
echo "=== resultado: $([ $RC = 0 ] && echo OK || echo FALHOU) (rc=$RC) ==="
exit $RC

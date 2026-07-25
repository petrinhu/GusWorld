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
# (a) Arquitetura: core/ e domain/ sao POCO; nenhum #include <Q...>, <SDL...>, <RmlUi...>
#     NEM <glintfx...> (ADR-008 + ADR-009 + ADR-010 F3: SDL/RmlUi/glintfx vivem SO em
#     platform/ + app/). O match tambem pega os namespaces Rml:: e glintfx:: vazando pra
#     core/domain. Mesma logica do .forgejo/workflows/ci.yml (sincronia de proposito).
GATE_ARCH=0
if grep -rnE '#[[:space:]]*include[[:space:]]*<(Q[A-Za-z]|SDL|RmlUi|glintfx)' \
        "$ENGINE/core" "$ENGINE/domain" 2>/dev/null; then
    echo "GATE(arch): include de Qt, SDL, RmlUi ou glintfx encontrado em core/ ou domain/ (POCO)."
    GATE_ARCH=1
elif grep -rnE '\b(Rml|glintfx)::' "$ENGINE/core" "$ENGINE/domain" 2>/dev/null; then
    echo "GATE(arch): uso do namespace Rml:: ou glintfx:: encontrado em core/ ou domain/ (POCO)."
    GATE_ARCH=1
else
    [ "$QUIET" = "1" ] || \
        echo "GATE(arch): OK (sem Qt, SDL, RmlUi nem glintfx em core/ ou domain/)."
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
# Log completo do ctest vai pra um caminho FIXO dentro do build dir (sobrescrito a
# cada rodada, nao precisa de remocao): antes ia pra /tmp/.gusworld_ctest.$$ e era
# descartado via `gio trash` a cada execucao do hook PostToolUse - centenas de
# arquivos por sessao de trabalho poluindo o Lixo do usuario (AC-E6).
CTEST_LOG="$ENGINE/build/$PRESET/last_ctest.log"
SUITE=0
if [ "$BUILD" = "0" ]; then
    set +e
    # XDG_RUNTIME_DIR ISOLADO (2026-07-24, achado do glintfx via bus - o furo que sobrou
    # no 1o conserto): forcar SDL_VIDEODRIVER=x11 e unsetar WAYLAND_DISPLAY **NAO ISOLA**.
    # `wl_display_connect(NULL)` - que QUALQUER dependencia do processo pode chamar, nao so
    # o nosso codigo - cai no nome EMBUTIDO "wayland-0" e resolve dentro do
    # $XDG_RUNTIME_DIR; unsetar a variavel de backend so tira o OVERRIDE, o fallback pelo
    # nome fixo continua achando o socket da sessao VIVA do usuario (eles levaram a mordida:
    # janela na sessao real por 2-3 min com o display aninhado ja provado). O conserto que
    # fecha e apontar o XDG_RUNTIME_DIR pra um dir PROPRIO e VAZIO (sem socket wayland-0 pra
    # o fallback encontrar), so no processo sob teste. Fica dentro do build dir (descartavel,
    # nunca contem socket) em vez de mktemp, o que evita de quebra a armadilha de limpeza que
    # eles relataram (trap ... RETURN nao dispara quando `set -e` aborta pela falha do
    # proprio comando - justamente o caminho de teste FALHANDO).
    _suite_xdg="$ENGINE/build/$PRESET/.suite_xdg_runtime"
    mkdir -p "$_suite_xdg" && chmod 700 "$_suite_xdg"

    # DISPLAY ISOLADO (2026-07-24, F4-GL-TESTS-SILENT-DEGRADE / achado do QA F4-1b.4):
    # alguns testes de interacao (app/tests/*_interaction_test) abrem contexto GL REAL
    # via glintfx::UiLayer. Sem isolar, o ctest herda o DISPLAY=:0 / WAYLAND_DISPLAY da
    # sessao VIVA do usuario e os testes GL rodam NELA (janelas HIDDEN, mas mesma
    # superficie que feedback_nunca_stress_janela_sessao_viva proibe em absoluto).
    # Solucao: subir um Xvfb dedicado num display LIVRE e forcar o SDL a usar X11 nele
    # (env -u WAYLAND_DISPLAY tira o wayland-0 do caminho; SDL_VIDEODRIVER=x11 impede
    # o SDL de escolher wayland). Sem Xvfb no host -> SDL_VIDEODRIVER=offscreen (os
    # testes GL degradam pra 0 assercoes, mas NAO tocam a sessao do usuario). SMOKE ja
    # usa dummy; aqui o GL precisa rodar de verdade, por isso Xvfb (framebuffer real),
    # nao dummy. GUSWORLD_SUITE_DISPLAY=:N sobrepoe (ex.: CI que ja tem Xvfb proprio).
    _suite_xvfb_pid=""
    _suite_disp="${GUSWORLD_SUITE_DISPLAY:-}"
    if [ -z "$_suite_disp" ] && command -v Xvfb >/dev/null 2>&1; then
        for _n in 99 100 101 102 103 104 105 106 107 108; do
            [ -e "/tmp/.X11-unix/X$_n" ] || { _suite_disp=":$_n"; break; }
        done
        if [ -n "$_suite_disp" ]; then
            Xvfb "$_suite_disp" -screen 0 1024x768x24 >/dev/null 2>&1 &
            _suite_xvfb_pid=$!
            sleep 1
        fi
    fi
    # FIX (2026-07-14): capturar o exit do ctest LOGO APOS o pipe ctest|tee (o `|| true`
    # de um grep depois resetava PIPESTATUS e mascarava falha -> verde cego).
    if [ -n "$_suite_disp" ]; then
        ( cd "$ENGINE" && env -u WAYLAND_DISPLAY DISPLAY="$_suite_disp" SDL_VIDEODRIVER=x11 \
            XDG_RUNTIME_DIR="$_suite_xdg" XDG_SESSION_TYPE=x11 \
            DBUS_SESSION_BUS_ADDRESS="unix:path=$_suite_xdg/no-dbus" \
            ctest --preset "$PRESET" 2>&1 ) | tee "$CTEST_LOG" >/dev/null
    else
        # sem display isolado: NUNCA rodar GL no :0 vivo -> offscreen (GL degrada).
        # O XDG_RUNTIME_DIR proprio vai TAMBEM aqui: sem ele, uma dependencia que chame
        # wl_display_connect(NULL) ainda acharia o wayland-0 da sessao viva mesmo com o
        # DISPLAY vazio (o fallback e por nome fixo dentro do runtime dir, nao por DISPLAY).
        ( cd "$ENGINE" && env -u WAYLAND_DISPLAY DISPLAY= SDL_VIDEODRIVER=offscreen \
            XDG_RUNTIME_DIR="$_suite_xdg" XDG_SESSION_TYPE=x11 \
            DBUS_SESSION_BUS_ADDRESS="unix:path=$_suite_xdg/no-dbus" \
            ctest --preset "$PRESET" 2>&1 ) | tee "$CTEST_LOG" >/dev/null
    fi
    SUITE=${PIPESTATUS[0]}
    [ -n "$_suite_xvfb_pid" ] && kill "$_suite_xvfb_pid" 2>/dev/null
    grep -E "tests passed|tests failed|Total Test" "$CTEST_LOG" || true
    set -e
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

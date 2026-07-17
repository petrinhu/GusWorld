#!/usr/bin/env bash
# tools/asan_gate.sh
#
# Entregavel 2 (defense-in-depth #2) do par anti-crash/UAF do GusWorld.
# PROATIVO e pesado: compila os testes de dominio (deck/save/combat/etc, o
# POCO puro de GusEngine/domain) sob AddressSanitizer + UndefinedBehavior-
# Sanitizer e roda a suite. Se o ASan/UBSan acusar erro OU algum teste
# falhar, o push e BLOQUEADO (exit != 0). Isto teria pego o UAF real que deu
# origem a este par de scripts ANTES de qualquer coredump em producao/CI -
# a suite normal (sem sanitizer) passou verde e escondeu o bug.
#
# Roda no pre-push LOCAL do repo (.git/hooks/pre-push), que o shim global
# core.hooksPath (~/.claude/githooks/pre-push -> _chain.sh) encadeia. Fica
# no PRE-PUSH (nao pre-commit) de proposito: e pesado (build C++ + suite),
# e a politica local-first do projeto (feedback_ci_policy_local_first) e
# jobs pesados fora do loop de commit, dentro do loop de push - o commit
# continua rapido, quem empurra pro remoto paga o preco do gate real.
#
# Escape (documentado, NAO magico): `git push --no-verify` pula TODOS os
# hooks de pre-push, este incluso. Uso excepcional, nunca default.
#
# --- relacao com o job "asan" do CI (.forgejo/workflows/ci.yml) -----------
# O CI ja tem um job "asan" completo (core+domain+platform+app, runner
# codeberg-medium, ~30min timeout) que roda em PR/push pro remoto. ESTE
# script NAO substitui aquele - e um pre-flight LOCAL e mais estreito (so
# domain/, so o subconjunto do -R) pra pegar o mesmo tipo de bug ANTES de
# gastar minuto de CI/round-trip de PR. Os dois sao defense-in-depth entre
# si tambem: se alguem pular este gate (--no-verify) ou ele tiver um
# falso-negativo por causa do -R estreito, o CI completo ainda pega.
#
# --- por que um build dir PROPRIO (build/asan-gate), separado do
#     build/asan manual -----------------------------------------------------
# GusEngine/build/asan e um build ASan feito A MAO por quem esta caçando um
# bug especifico (config atual: -fsanitize=address,undefined
# -fno-omit-frame-pointer -g -O2, CMAKE_BUILD_TYPE=Debug, GLINTFX_SANITIZE=
# OFF - conferido no CMakeCache.txt de hoje). Um gate automatico que roda
# a cada push NAO PODE compartilhar esse diretorio: dois `ninja` escrevendo
# no mesmo build dir ao mesmo tempo corrompem o build (exatamente a razao
# pela qual esta sessao esta PROIBIDA de tocar em build/asan agora - outro
# agente esta compilando la). Por isso este script usa
# GusEngine/build/asan-gate, configuravel via GUSWORLD_ASAN_BUILD_DIR se um
# dia for preciso isolar ainda mais (ex: rodar 2 gates em paralelo).
#
# --- por que so o alvo gusengine_domain_tests -----------------------------
# domain/ e POCO puro (NAO linka SDL3/RmlUi/glintfx - gate de arquitetura
# do check.sh garante isso). `cmake --build --target gusengine_domain_tests`
# builda so core+domain+Catch2, pulando platform/app/glintfx/RmlUi (que sao
# a maior parte do tempo de build do projeto). Mantem o gate do pre-push
# rapido o bastante pra nao virar habito de pular.
#
# --- por que TMPDIR=/var/tmp -----------------------------------------------
# Nesta maquina /tmp e tmpfs (RAM). Um build C++ com ASan + o link do
# binario de testes (ja tem ~190MB no build manual de hoje) pode encher o
# tmpfs e falhar com "no space on device" no `ld` (que usa $TMPDIR). Builda
# em disco de verdade: build/asan-gate fica dentro do repo (disco real,
# nao tmpfs) e TMPDIR aponta pra /var/tmp (idem).
#
# --- por que halt_on_error=1 em ASAN/UBSAN_OPTIONS -------------------------
# Por padrao o UBSan (diferente do ASan) so IMPRIME o diagnostico e
# CONTINUA rodando - o processo pode terminar com exit 0 mesmo tendo
# reportado um erro. Isso mataria o proposito do gate (silenciosamente
# verde de novo, a mesma classe de falha que originou este script). Forcamos
# halt_on_error=1 nos dois sanitizers pra garantir que QUALQUER diagnostico
# vira exit != 0.
#
# --- por que checar "zero testes casaram" como FALHA -----------------------
# Um regex de -R que por engano casa 0 testes faria o ctest "passar" sem
# checar NADA - um gate mudo e pior que nenhum gate (falso verde). Se o
# filtro nao casar nenhum teste, tratamos como FALHA (nao como skip),
# forcando quem mexeu no regex a perceber.
#
# Variaveis de ambiente uteis:
#   GUSWORLD_ASAN_BUILD_DIR   build dir do gate (default: GusEngine/build/asan-gate)
#   GUSWORLD_ASAN_TEST_REGEX  regex do `ctest -R` (default: deck|save|combat|invariants|lifetime)
#   GUSWORLD_ASAN_SKIP=1      pula o gate por completo COM AVISO (emergencia;
#                             preferir `git push --no-verify`, que fica no
#                             log do git em vez de uma env silenciosa)

set -euo pipefail

# ---------------------------------------------------------------- resolucao
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
ENGINE="$ROOT/GusEngine"
BUILD_DIR="${GUSWORLD_ASAN_BUILD_DIR:-$ENGINE/build/asan-gate}"
TEST_DIR="$BUILD_DIR/domain/tests"
TEST_REGEX="${GUSWORLD_ASAN_TEST_REGEX:-deck|save|combat|invariants|lifetime}"

echo "=== asan gate (pre-push, GusEngine/domain sob ASan+UBSan) ==="

# ---------------------------------------------------------------- escape
if [ "${GUSWORLD_ASAN_SKIP:-0}" = "1" ]; then
    echo "AVISO: GUSWORLD_ASAN_SKIP=1 - gate ASan pulado. Prefira 'git push"
    echo "--no-verify' (fica no historico do git) a esta env pra pular so"
    echo "uma vez de proposito."
    exit 0
fi

# ---------------------------------------------------------------- toolchain
# Ferramenta ausente = SKIP com aviso, nunca bloqueio falso (maquina sem
# toolchain C++ nao e o mesmo problema que um bug real). Ferramenta
# PRESENTE que reporta erro (compile ou runtime) e bloqueio de verdade.
MISSING=""
for tool in cmake ninja; do
    command -v "$tool" >/dev/null 2>&1 || MISSING="$MISSING $tool"
done
CXX_BIN="${CXX:-}"
if [ -z "$CXX_BIN" ]; then
    command -v c++ >/dev/null 2>&1 || command -v g++ >/dev/null 2>&1 \
        || command -v clang++ >/dev/null 2>&1 || MISSING="$MISSING (c++|g++|clang++)"
fi
if [ -n "$MISSING" ]; then
    echo "AVISO: ferramenta(s) ausente(s):$MISSING - gate ASan pulado (nao bloqueia)."
    echo "Instale cmake+ninja+um compilador C++20 pra habilitar o gate de verdade."
    exit 0
fi

# ---------------------------------------------------------------- ambiente
# tmpfs cheio derruba o link do ASan; disco de verdade evita isso (ver nota
# no topo do arquivo).
export TMPDIR=/var/tmp

# halt_on_error=1: garante exit != 0 em QUALQUER diagnostico ASan/UBSan
# (ver nota no topo). detect_leaks=1 liga LeakSanitizer junto (parte do
# runtime do ASan em Linux); print_stacktrace=1 ajuda o triage do UBSan.
export ASAN_OPTIONS="halt_on_error=1:detect_leaks=1:${ASAN_OPTIONS:-}"
export UBSAN_OPTIONS="halt_on_error=1:print_stacktrace=1:${UBSAN_OPTIONS:-}"

# ---------------------------------------------------------------- configure
# Idempotente (mesmo padrao do tools/check.sh): so reconfigura se o build
# dir ainda nao existe. Flags espelham o build ASan manual de referencia
# (GusEngine/build/asan/CMakeCache.txt, conferido em 2026-07-16):
# CMAKE_BUILD_TYPE=Debug, -fsanitize=address,undefined -fno-omit-frame-
# pointer -g -O2 no compilador, mesma sanitizacao no linker,
# GLINTFX_SANITIZE=OFF (nao sanitiza a lib vendorizada - ruido/tempo, e
# domain/ nem linka ela).
if [ ! -f "$BUILD_DIR/build.ninja" ]; then
    echo "(configurando $BUILD_DIR pela 1a vez)"
    cmake -S "$ENGINE" -B "$BUILD_DIR" -G Ninja \
        -DCMAKE_BUILD_TYPE=Debug \
        -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer -g -O2" \
        -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address,undefined" \
        -DGLINTFX_SANITIZE=OFF
fi

# ---------------------------------------------------------------- build
set +e
cmake --build "$BUILD_DIR" --target gusengine_domain_tests
BUILD_RC=$?
set -e
echo "BUILD=$BUILD_RC"
if [ "$BUILD_RC" != "0" ]; then
    echo "=== asan gate FALHOU: build de gusengine_domain_tests quebrou. ==="
    exit 1
fi

# ---------------------------------------------------------------- suite
if [ ! -d "$TEST_DIR" ]; then
    echo "=== asan gate FALHOU: $TEST_DIR nao existe apos o build (layout mudou?). ==="
    exit 1
fi

set +e
CTEST_OUT="$(ctest --test-dir "$TEST_DIR" -R "$TEST_REGEX" --output-on-failure 2>&1)"
SUITE_RC=$?
set -e
echo "$CTEST_OUT"
echo "SUITE=$SUITE_RC"

if [ "$SUITE_RC" != "0" ]; then
    echo "=== asan gate FALHOU: ASan/UBSan ou teste(s) reportaram erro. ==="
    echo "=== Push bloqueado. Corrija antes de 'git push' de novo, ou use ==="
    echo "=== 'git push --no-verify' se souber exatamente o que esta fazendo. ==="
    exit 1
fi

# Regex que casa 0 testes = gate mudo = falso-verde. Trata como falha (ver
# nota no topo do arquivo).
if printf '%s' "$CTEST_OUT" | grep -qE 'No tests were found|Total Tests: 0'; then
    echo "=== asan gate FALHOU: o regex '-R $TEST_REGEX' nao casou NENHUM teste."
    echo "=== Isso e um gate mudo (checa nada, 'passa' sempre). Ajuste"
    echo "=== GUSWORLD_ASAN_TEST_REGEX ou o regex default no script."
    exit 1
fi

echo "=== asan gate OK ==="
exit 0

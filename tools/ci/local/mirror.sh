#!/bin/sh
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Local mirror of the CI gates (TODO item C7, GW-19, GW-35). Runs on this machine the same
# tools/ci/<gate>/ scripts that .github/workflows/*.yml call, in the same order and with
# the same arguments. The verdict of each gate comes from the script the workflow calls;
# this file only orchestrates. Values the workflows declare (the gitleaks pin, the
# allowlist count) are read from the snapshot's workflow, never typed again here.
#
# Where each gate runs:
#   host          text and static analysis, on a snapshot of HEAD (see below)
#   container     dynamic work (build, ctest, sanitizers, gate suites): only with
#                 --container, only in the image GUSWORLD_CI_IMAGE names, and never with
#                 a host volume (GW-35). Without --container those gates are NOT RUN.
#   NOT RUN       the gate needs network, a missing tool, or a run that was not
#                 authorized here. The reason is printed. NOT RUN is never green.
#
# Snapshot: `git archive HEAD` without the git-crypt areas listed in .gitattributes,
# unpacked into a throwaway directory and committed there, so the scripts judge HEAD and
# not the working tree. The git-crypt areas are never read, copied or put in a tar.
#
# The hook half (b) of the commit gate is tools/ci/local/check_hook.sh, run on the real
# clone, because the hook lives in the clone and not in HEAD.
#
# The work directory is printed at the end and is NOT removed: remove it by hand.
#
# Declared limits:
#   - Only GCC (CXX defaults to c++) is run here. Clang, Ubuntu, Arch, CachyOS and Windows
#     are not run by this mirror; they stay with the CI.
#   - Portão 4 (secret scan) is NOT RUN here: the history holds encrypted docs/_secret
#     objects the brief forbids reading. No weaker scan takes its place.
#   - cve (network), versao-tag (no tag at HEAD) and sanitizadores (CI only, leader's
#     decision of 21/11/2026) are NOT RUN by design, listed on every run, and do not change
#     the exit code.
#
# Usage: sh tools/ci/local/mirror.sh [--container]
# Exit: 0 every gate PASSA (or only the fixed NOT RUN above); 1 some gate REPROVA;
#       2 some other gate NOT RUN or PARCIAL.
set -u

usage() {
  cat <<'EOF'
usage: sh tools/ci/local/mirror.sh [--container]
  --container   also run the dynamic gates in GUSWORLD_CI_IMAGE (default
                gusworld-ci-fedora:7ca9869fd9d8). Needs the owner's go-ahead (GW-35).
EOF
}

want_container=0
for arg in "$@"; do
  case "$arg" in
    --container) want_container=1 ;;
    -h|--help) usage; exit 0 ;;
    *) echo "mirror.sh: unknown argument: $arg" >&2; usage >&2; exit 2 ;;
  esac
done

root=$(git rev-parse --show-toplevel 2>/dev/null) || { echo "mirror.sh: not inside a git work tree" >&2; exit 2; }

tmp_base=${TMPDIR:-/var/tmp}
[ -d "$tmp_base" ] || tmp_base=/tmp
W=$(mktemp -d "$tmp_base/gw-mirror.XXXXXX") || { echo "mirror.sh: cannot create the work directory" >&2; exit 2; }
src="$W/src"
RUNNER_TEMP="$W/rt"
LOGS="$W/logs"
VERD="$W/verdicts.txt"
mkdir -p "$src" "$RUNNER_TEMP" "$LOGS" || exit 2
: > "$VERD"
export RUNNER_TEMP

# Snapshot of HEAD without the git-crypt areas (the exclusions mirror .gitattributes).
git -C "$root" archive --format=tar HEAD -- . \
  ':(exclude)docs/_secret' \
  ':(exclude)docs/book' \
  ':(exclude)docs/narrative/deep/stinger' \
  ':(exclude)tools/security/termos_proibidos.txt' \
  | tar -x -C "$src" || { echo "mirror.sh: snapshot of HEAD failed" >&2; exit 2; }

cd "$src" || exit 2
git init -q . || { echo "mirror.sh: cannot init the snapshot repository" >&2; exit 2; }
git add -A || { echo "mirror.sh: cannot stage the snapshot" >&2; exit 2; }
# core.hooksPath points to a missing directory, so no hook of the machine runs on the snapshot.
git -c core.hooksPath="$W/no-hooks" -c user.name=gw-mirror -c user.email=mirror@localhost \
  -c commit.gpgsign=false commit -q -m "snapshot of HEAD for the local CI mirror" \
  || { echo "mirror.sh: cannot commit the snapshot" >&2; exit 2; }

GATES="build sanitizadores estatica-cppcheck estatica-clang-tidy segredo segredo-autotestes gancho-commit formatacao piso-cmake camadas lei-zero versao-tag cve"

title_of() {
  case "$1" in
    build) echo "portão 1, zero aviso e testes (build.yml)" ;;
    sanitizadores) echo "portão 2, sanitizadores (sanitizers.yml)" ;;
    estatica-cppcheck) echo "portão 3, análise estática cppcheck (cppcheck.yml)" ;;
    estatica-clang-tidy) echo "portão 3, análise estática clang-tidy (clang-tidy.yml)" ;;
    segredo) echo "portão 4, varredura de segredo com histórico (gitleaks.yml)" ;;
    segredo-autotestes) echo "autotestes do guard de segredo e da lista de exceções (gitleaks.yml)" ;;
    gancho-commit) echo "portão 5, mensagem de commit, suíte e gancho (commit-gate.yml)" ;;
    formatacao) echo "formatação clang-format (clang-format.yml)" ;;
    piso-cmake) echo "piso do CMake 4.1 (cmake_floor.sh, em cada job)" ;;
    camadas) echo "camadas do código (layers.yml)" ;;
    lei-zero) echo "LEI ZERO (lei-zero.yml)" ;;
    versao-tag) echo "tag casa com a versão (version-tag.yml)" ;;
    cve) echo "portão de CVE (cve.yml)" ;;
    *) echo "$1" ;;
  esac
}

# record <gate> <step> <status> <detail>. Status: PASSA, REPROVA or NÃO EXECUTADO.
record() {
  printf '%s|%s|%s|%s\n' "$1" "$2" "$3" "$4" >> "$VERD"
}

# step <gate> <name> <command...>: runs the command in the snapshot, keeps its log, judges by rc.
step() {
  gate=$1; name=$2; shift 2
  log="$LOGS/$gate.$name.log"
  rc=0
  "$@" > "$log" 2>&1 || rc=$?
  if [ "$rc" -eq 0 ]; then
    record "$gate" "$name" "PASSA" "rc=0"
    echo "  [PASSA] $gate / $name"
  else
    record "$gate" "$name" "REPROVA" "rc=$rc"
    echo "  [REPROVA] $gate / $name (rc=$rc, log: $log)"
    tail -n 15 "$log" | sed 's/^/      | /'
  fi
}

# not_run <gate> <name> <reason>
not_run() {
  record "$1" "$2" "NÃO EXECUTADO" "$3"
  echo "  [NÃO EXECUTADO] $1 / $2: $3"
}

# The container gates. Their steps come from container_steps.sh, so they are NOT RUN here.
container_not_run() {
  reason="$1"
  not_run build compilacao "$reason"
  not_run build contagem "$reason"
  not_run build ctest "$reason"
  not_run gancho-commit suite-e-mutacao "$reason"
  not_run camadas selftest "$reason"
}

run_container() {
  img=${GUSWORLD_CI_IMAGE:-gusworld-ci-fedora:7ca9869fd9d8}
  if command -v podman >/dev/null 2>&1; then
    rt=podman
  elif command -v docker >/dev/null 2>&1; then
    rt=docker
  else
    container_not_run "não executado: falta o runtime de contêiner (podman ou docker)"
    return 0
  fi
  if ! "$rt" image inspect "$img" >/dev/null 2>&1; then
    container_not_run "não executado: falta a imagem $img"
    return 0
  fi
  steps="$root/tools/ci/local/container_steps.sh"
  if [ ! -f "$steps" ]; then
    container_not_run "não executado: falta $steps"
    return 0
  fi
  clog="$LOGS/container.log"
  rc=0
  # One heavy-work slot (L-11 global), held from before docker run until the file descriptor
  # is closed below. The same take that tools/tdd/prepare.sh makes.
  # shellcheck source=../../container/heavy_slot.sh
  . "$root/tools/container/heavy_slot.sh" || { container_not_run "não executado: heavy_slot.sh ilegível"; return 0; }
  take_heavy_slot || { container_not_run "não executado: slots de trabalho pesado ocupados"; return 0; }
  # The tree enters by stdin, as a tar of the snapshot. No host volume. Flags as GW-35 asks.
  tar -C "$src" -cf - . | "$rt" run --rm -i \
    --pull never --network none --cap-drop ALL --pids-limit 80 --memory 4g \
    "$img" bash -c "$(cat "$steps")" > "$clog" 2>&1 || rc=$?
  exec 9>&-
  echo "  heavy slot solto"
  echo "  contêiner ($img) saiu com rc=$rc, log: $clog"
  grep '^STEP ' "$clog" | while read -r tag gate name code; do
    if [ "$code" = 0 ]; then
      record "$gate" "$name" "PASSA" "rc=0 no contêiner"
      echo "  [PASSA] $gate / $name (contêiner)"
    else
      record "$gate" "$name" "REPROVA" "rc=$code no contêiner"
      echo "  [REPROVA] $gate / $name (contêiner, rc=$code, log: $clog)"
    fi
  done
  grep '^NOTRUN ' "$clog" | while read -r tag gate name reason; do
    not_run "$gate" "$name" "$reason"
  done
  if ! grep -q '^CONTAINER STEPS DONE' "$clog"; then
    container_not_run "não executado: o contêiner não chegou ao fim (rc=$rc, log: $clog)"
  fi
}

run_host_gates() {
  cd "$src" || return 1
  echo "== sanitizadores (fixo, só no CI remoto)"
  not_run sanitizadores fixo "sanitizadores só no CI remoto, por decisão do líder de 21/11/2026"

  echo "== piso-cmake"
  step piso-cmake piso bash tools/ci/cmake_floor.sh

  echo "== formatacao"
  step formatacao enumerar bash tools/ci/clang_format/enumerate.sh "$RUNNER_TEMP/clang-sources.txt"
  step formatacao prova bash tools/ci/clang_format/proof.sh "$RUNNER_TEMP/clang-format-proof"
  step formatacao juiz bash tools/ci/clang_format/judge.sh "$RUNNER_TEMP/clang-sources.txt" "$RUNNER_TEMP/clang-format-real.log"

  echo "== estatica-cppcheck"
  step estatica-cppcheck configure sh -c 'cmake -S . -B "$RUNNER_TEMP/cppcheck-build" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON && test -s "$RUNNER_TEMP/cppcheck-build/compile_commands.json"'
  step estatica-cppcheck enumerar bash tools/ci/cppcheck/enumerate.sh "$RUNNER_TEMP/cc-sources.txt"
  step estatica-cppcheck arvore bash tools/ci/cppcheck/scan.sh real "$RUNNER_TEMP/cppcheck-real.log" "$RUNNER_TEMP/cc-sources.txt"
  step estatica-cppcheck prova-com bash tools/ci/cppcheck/scan.sh with "$RUNNER_TEMP/cppcheck-with.log" tests/fixtures/cppcheck/cppcheck_fixture.cpp
  step estatica-cppcheck prova-cabecalho bash tools/ci/cppcheck/scan.sh with "$RUNNER_TEMP/cppcheck-cc-header-with.log" tests/fixtures/cppcheck/core/include/gusworld/core/header_probe.hpp
  step estatica-cppcheck prova-sem-opcao bash tools/ci/cppcheck/scan.sh without "$RUNNER_TEMP/cppcheck-without.log" tests/fixtures/cppcheck/cppcheck_fixture.cpp

  echo "== estatica-clang-tidy"
  step estatica-clang-tidy configure sh -c 'cmake -S . -B "$RUNNER_TEMP/clang-tidy-build" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DGUSWORLD_PRESENT=ON -DCMAKE_CXX_SCAN_FOR_MODULES=OFF && test -s "$RUNNER_TEMP/clang-tidy-build/compile_commands.json"'
  step estatica-clang-tidy enumerar bash tools/ci/clang_tidy/enumerate.sh "$RUNNER_TEMP/tidy-sources.txt"
  step estatica-clang-tidy receita bash tools/ci/clang_tidy/check_db.sh "$RUNNER_TEMP/clang-tidy-build/compile_commands.json" "$RUNNER_TEMP/tidy-sources.txt" "$RUNNER_TEMP"
  step estatica-clang-tidy arvore bash tools/ci/clang_tidy/scan.sh real "$RUNNER_TEMP/tidy-real.log" "$RUNNER_TEMP/tidy-records.txt" "$RUNNER_TEMP/clang-tidy-build" "$RUNNER_TEMP/tidy-sources.txt"
  step estatica-clang-tidy prova-com bash tools/ci/clang_tidy/scan.sh with "$RUNNER_TEMP/tidy-with.log" tests/fixtures/clang_tidy/clang_tidy_fixture.cpp
  step estatica-clang-tidy prova-cabecalho bash tools/ci/clang_tidy/scan.sh with "$RUNNER_TEMP/tidy-header-with.log" tests/fixtures/clang_tidy/header_fixture.cpp
  step estatica-clang-tidy prova-sem-opcao bash tools/ci/clang_tidy/scan.sh without "$RUNNER_TEMP/tidy-without.log" tests/fixtures/clang_tidy/clang_tidy_fixture.cpp

  echo "== segredo"
  pin=$(sed -n 's/^ *GITLEAKS_VERSION: "\([0-9.]*\)".*/\1/p' .github/workflows/gitleaks.yml | head -n 1)
  ALLOW_N=$(sed -n 's/^ *GITLEAKS_ALLOWLIST_ENTRIES: "\([0-9][0-9]*\)".*/\1/p' .github/workflows/gitleaks.yml | head -n 1)
  export ALLOW_N
  have=$(gitleaks version 2>/dev/null | head -n 1) || have=""
  if [ -z "$have" ]; then
    not_run segredo ferramenta "não executado: falta o gitleaks no PATH"
  elif [ "$have" != "$pin" ]; then
    not_run segredo ferramenta "não executado: gitleaks local $have difere do fixado no CI ($pin)"
  else
    # Self-tests of the guard and the allowlist. They are not the scan: the scan is below.
    step segredo-autotestes bash tools/ci/gitleaks/verdict_cases.sh
    step segredo-autotestes prova-vermelha-1 sh -c 'set -e; repo="$RUNNER_TEMP/red-synthetic"; log="$RUNNER_TEMP/red-synthetic.log"; bash tools/ci/gitleaks/fixture_synthetic_repo.sh "$repo"; bash tools/ci/gitleaks/expect.sh leak 1 "$repo" "$log"'
    step segredo-autotestes prova-vermelha-2 sh -c 'set -e; repo="$RUNNER_TEMP/red-empty"; mkdir "$repo"; git init -q -b main "$repo"; bash tools/ci/gitleaks/expect.sh zero any "$repo" "$RUNNER_TEMP/red-empty.log"'
    step segredo-autotestes prova-lista-vermelha sh -c 'set -e; fake="$RUNNER_TEMP/allowlist-fake"; printf "%s\n" "# fabricated for the red proof, no secret" "fake/one.txt:fake-rule:1" "fake/two.txt:fake-rule:2" > "$fake"; bash tools/ci/gitleaks/expect_allowlist_rejects.sh "$fake" "$ALLOW_N"'
    step segredo-autotestes lista-de-excecoes bash tools/ci/gitleaks/allowlist_gate.sh .gitleaksignore "$ALLOW_N"
  fi
  # The portão 4 verdict is the history scan. It is never replaced by a weaker scan here.
  not_run segredo varredura-do-historico "vedado ao espelho: o histórico guarda objetos cifrados de docs/_secret, e o brief proíbe ler esses caminhos; roda no CI"

  echo "== gancho-commit (metade b, no clone real)"
  step gancho-commit gancho-instalado sh "$root/tools/ci/local/check_hook.sh" "$root"

  echo "== camadas"
  step camadas configure sh -c 'cmake -S . -B "$RUNNER_TEMP/layers-build" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON'
  step camadas arvore python3 tools/ci/check_layers.py --build "$RUNNER_TEMP/layers-build"

  echo "== lei-zero"
  step lei-zero divida python3 tools/security/lei_zero_guard.py
  up=$(git -C "$root" rev-parse --verify -q '@{upstream}' 2>/dev/null) || up=""
  if [ -z "$up" ]; then
    not_run lei-zero regressao "sem upstream: o CI compara com a base do push, e o espelho não tem essa base"
  elif [ -n "$(git -C "$root" diff --name-only "$up" HEAD -- docs/_secret docs/book docs/narrative/deep/stinger tools/security/termos_proibidos.txt)" ]; then
    not_run lei-zero regressao "vedado ao espelho: o intervalo toca área cifrada e o brief proíbe ler esses caminhos"
  else
    step lei-zero regressao sh -c 'cd "$1" && python3 "$2/tools/security/lei_zero_guard.py" --range "$3 HEAD"' sh "$root" "$src" "$up"
  fi

  echo "== versao-tag"
  tag=$(git -C "$root" for-each-ref --points-at HEAD --format='%(refname:short)' refs/tags 2>/dev/null | head -n 1)
  if [ -z "$tag" ]; then
    not_run versao-tag verificacao "sem tag em HEAD: o portão roda no push de tag"
  else
    step versao-tag verificacao python3 tools/ci/check_version_matches_tag.py "$tag"
  fi

  echo "== cve"
  not_run cve varredura "precisa de rede para atualizar o banco de vulnerabilidades (grype e trivy) e de imagens base; roda no CI"
}

echo "== espelho local do CI, snapshot em $src"
run_host_gates
cd "$src" || exit 2

echo "== contêineres (dinâmico, GW-35)"
if [ "$want_container" -eq 1 ]; then
  run_container
else
  container_not_run "não executado: contêiner não solicitado nesta execução (use --container, com o pode do líder)"
fi

# gate_verdict <gate>: REPROVA if any step fails; NÃO EXECUTADO if all are not run; PARCIAL if
# some are not run; PASSA if all pass. Prints nothing when the gate has no step.
gate_verdict() {
  awk -F'|' -v g="$1" '
    $1 == g {
      n++
      if ($3 == "REPROVA") f++
      else if ($3 == "NÃO EXECUTADO") x++
      else if ($3 == "PASSA") p++
    }
    END {
      if (n == 0) exit
      if (f > 0) print "REPROVA"
      else if (x == n) print "NÃO EXECUTADO"
      else if (x > 0) print "PARCIAL"
      else print "PASSA"
    }' "$VERD"
}

echo
echo "=== VEREDITO POR PORTÃO ==="
# Gates that cannot run here by design: network (cve) and the absence of a tag (versao-tag).
# They are listed on every run and do not make the exit code non-zero. Any other NÃO
# EXECUTADO or PARCIAL does.
FIXED_NOT_RUN="cve versao-tag sanitizadores"
not_passed=0
for g in $GATES; do
  v=$(gate_verdict "$g")
  # Coverage: a gate with no record at all is REPROVA, never silence (GW-19: a gate with no
  # step is a broken scan, L-36). The record makes the exit code non-zero below.
  if [ -z "$v" ]; then
    record "$g" "cobertura" "REPROVA" "portão sem nenhum registro: o espelho não rodou nenhum passo dele"
    v="REPROVA"
  fi
  printf '  %-20s %-14s %s\n' "$g" "$v" "$(title_of "$g")"
  case " $FIXED_NOT_RUN " in
    *" $g "*) [ "$v" = "PASSA" ] || [ "$v" = "NÃO EXECUTADO" ] || not_passed=$((not_passed + 1)) ;;
    *) [ "$v" = "PASSA" ] || not_passed=$((not_passed + 1)) ;;
  esac
done
echo
echo "NÃO EXECUTADO por desenho, fixo, em toda execução: cve (rede), versao-tag (sem tag em HEAD), sanitizadores (só no CI remoto, por decisão do líder de 21/11/2026). Não derruba a saída."
echo
echo "=== DETALHE DOS QUE NÃO PASSARAM ==="
awk -F'|' '$3 != "PASSA" { printf "  %s / %s: %s (%s)\n", $1, $2, $3, $4 }' "$VERD"
echo
echo "snapshot, logs e veredito: $W (não removido: apague à mão quando não precisar)"

# Any REPROVA line counts, including the container's preparation lines outside GATES.
if grep -q '|REPROVA|' "$VERD"; then
  echo "RESULTADO: REPROVA (há passo reprovado; veja o detalhe)"
  exit 1
fi
if [ "$not_passed" -gt 0 ]; then
  echo "RESULTADO: NÃO FECHA ($not_passed portão(ões) não executados ou parciais; não contam como verde)"
  exit 2
fi
echo "RESULTADO: PASSA (todos os portões executados e verdes)"
exit 0

#!/usr/bin/env bash
# SPDX-License-Identifier: Apache-2.0
# tools/install_hooks.sh
#
# CRASH-HOOKS (residuo fechado em 2026-08-07): instalador de bootstrap dos
# git hooks LOCAIS deste repo. Ate esta fatia, `tools/crash_journal_check.sh`,
# `tools/asan_gate.sh` e `tools/gitleaks_staged_check.sh` estavam commitados e
# FUNCIONANDO nesta maquina - mas so porque alguem escreveu `.git/hooks/
# pre-commit` e `.git/hooks/pre-push` A MAO. `.git/hooks/` NUNCA e versionado
# (e interno ao `.git/` de cada clone), entao um CLONE LIMPO (outra maquina,
# outro dev, CI local) nao ganhava hook nenhum - o furo que este script fecha.
#
# O QUE ESTE SCRIPT FAZ: escreve `.git/hooks/pre-commit` e `.git/hooks/
# pre-push` deste repositorio (LOCAL, nunca `core.hooksPath` global - ver nota
# abaixo) chamando os tres clientes que existem de fato hoje:
#   pre-commit -> tools/crash_journal_check.sh   (avisa, NUNCA bloqueia)
#             -> tools/gitleaks_staged_check.sh  (BLOQUEIA se achar segredo)
#   pre-push   -> tools/asan_gate.sh              (BLOQUEIA se ASan/UBSan/teste falhar)
#             -> git-lfs pre-push (se git-lfs estiver instalado - preserva o
#                filtro auto-gerado por `git lfs install`, na MESMA ordem que
#                o arquivo local ja tinha antes desta fatia: nosso bloco
#                primeiro, LFS por ultimo)
#
# POR QUE NAO MEXE em `core.hooksPath` (LEI desta fatia, nao negociavel):
# nesta maquina existe um shim GLOBAL (`~/.claude/githooks/*` -> `_chain.sh`)
# que delega para `.git/hooks/<nome>` de QUALQUER repo, entao os hooks
# instalados aqui rodam tanto nesta maquina (via o shim, `--git-common-dir`)
# quanto num clone comum sem shim nenhum (onde `core.hooksPath` fica no
# default e o proprio git ja olha `.git/hooks/` direto). Sobrescrever
# `core.hooksPath` global e fora do escopo deste script POR DESENHO - ele so
# escreve dentro do `.git/` deste repositorio.
#
# ARMADILHA JA MEDIDA NESTA CASA (ver ~/.claude/githooks/_chain.sh): com
# `core.hooksPath` GLOBAL setado, `git rev-parse --git-path hooks` devolve o
# dir GLOBAL, nao o local do repo - usar isso aqui reescreveria os hooks
# globais da maquina por engano. Este script usa `git rev-parse
# --git-common-dir` (correto tambem em worktrees) + "/hooks", a MESMA tecnica
# do `_chain.sh`, e NUNCA `--git-path`.
#
# IDEMPOTENTE: rodar de novo com o repo ja instalado e um no-op (mesmo
# conteudo, mesma permissao). Se `.git/hooks/<nome>` ja existir com conteudo
# DIFERENTE do gerado por este script (hook manual, de outra ferramenta, ou
# customizado), o arquivo antigo e preservado com sufixo
# `.pre-gusworld-hooks.bak` antes de ser substituido - nada e perdido em
# silencio.
#
# CROSS-PLATFORM: este script e bash/POSIX (Linux/macOS). O gemeo Windows
# (hooks .cmd/.ps1 ou WSL) continua DOCUMENTADO e NAO IMPLEMENTADO - residuo
# conhecido desde a criacao do par anti-crash (2026-07-16), sem mudanca nesta
# fatia (o projeto e Linux-first hoje; Windows e alvo pos-v1.0.0, ver
# CLAUDE.md). Em Windows nativo (fora de WSL/Git Bash) `.git/hooks/pre-commit`
# sem shebang interpretavel pelo Git for Windows exige o Git Bash instalado
# (que ja vem com a instalacao padrao do Git for Windows) - nao ha fallback
# .cmd/.ps1 aqui.
#
# Uso:
#   tools/install_hooks.sh            instala pre-commit + pre-push
#   tools/install_hooks.sh --dry-run  mostra o que seria escrito, nao escreve nada

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

DRY_RUN=0
for _arg in "$@"; do
    case "$_arg" in
        --dry-run) DRY_RUN=1 ;;
        -h|--help)
            echo "uso: install_hooks.sh [--dry-run]"
            exit 0 ;;
        *)
            echo "install_hooks.sh: argumento desconhecido '$_arg' (use --help)" >&2
            exit 2 ;;
    esac
done

# --git-common-dir, NUNCA --git-path (ver nota acima) - resolve o .git/ real
# mesmo com core.hooksPath global setado, e mesmo dentro de um worktree.
GIT_COMMON_DIR="$(cd "$ROOT" && git rev-parse --git-common-dir)"
case "$GIT_COMMON_DIR" in
    /*) : ;;
    *) GIT_COMMON_DIR="$ROOT/$GIT_COMMON_DIR" ;;
esac
HOOKS_DIR="$GIT_COMMON_DIR/hooks"

if [ ! -d "$HOOKS_DIR" ]; then
    echo "install_hooks.sh: FALHA - $HOOKS_DIR nao existe (repo git valido?)." >&2
    exit 1
fi

MARKER="# GUSWORLD-INSTALL-HOOKS: gerado por tools/install_hooks.sh, nao editar a mao (rode o script de novo)."

_write_hook() {
    local name="$1" content="$2"
    local target="$HOOKS_DIR/$name"

    if [ "$DRY_RUN" = "1" ]; then
        echo "--- $target (dry-run, nao escrito) ---"
        printf '%s\n' "$content"
        return 0
    fi

    if [ -f "$target" ] && [ "$(cat "$target")" != "$content" ]; then
        local backup="$target.pre-gusworld-hooks.bak"
        cp "$target" "$backup"
        echo "install_hooks.sh: $name existia com conteudo diferente - backup em $(basename "$backup")."
    fi

    printf '%s\n' "$content" > "$target"
    chmod +x "$target"
    echo "install_hooks.sh: $name instalado em $target"
}

# --- pre-commit --------------------------------------------------------
PRE_COMMIT_CONTENT=$(cat <<EOF
#!/bin/sh
# .git/hooks/pre-commit (LOCAL deste repo - gerado por tools/install_hooks.sh,
# NAO versionado; rode o script de novo apos clonar).
$MARKER
#
# Entregavel 1 do par anti-crash/UAF (defense-in-depth #1): checa o journal
# de coredump por crash recente de um binario NOSSO. REATIVO, barato, NUNCA
# bloqueia (so avisa). Ver tools/crash_journal_check.sh pra logica completa.
"\$(git rev-parse --show-toplevel)/tools/crash_journal_check.sh"

# Recomendacao no 3 do TST-8: secret-scan gitleaks SO nos arquivos staged
# (sub-segundo; a varredura de arvore leva ~47s e nao cabe num commit). Este
# BLOQUEIA - segredo commitado nao se desfaz sem reescrever historico. Sai 0
# em silencio se o repo nao tem .gitleaks.toml, e avisa+passa se o gitleaks
# nao estiver instalado. Ver tools/gitleaks_staged_check.sh.
"\$(git rev-parse --show-toplevel)/tools/gitleaks_staged_check.sh" || exit \$?

exit 0
EOF
)

# --- pre-push ------------------------------------------------------------
PRE_PUSH_CONTENT=$(cat <<EOF
#!/bin/sh
# .git/hooks/pre-push (LOCAL deste repo - gerado por tools/install_hooks.sh,
# NAO versionado; rode o script de novo apos clonar).
$MARKER
#
# ATENCAO: se \`git lfs install --force\` (ou um reclone com LFS) rodar depois,
# ele pode SOBRESCREVER este arquivo e apagar o bloco abaixo - rode
# tools/install_hooks.sh de novo pra restaurar (idempotente).

# --- GusWorld asan gate (entregavel 2 do par anti-crash/UAF) --------------
# PROATIVO e pesado: compila+roda a suite de dominio sob ASan/UBSan. Falha
# aqui BLOQUEIA o push (exit != 0 propaga). Ver tools/asan_gate.sh pra logica
# completa. Escape: \`git push --no-verify\`.
GUSWORLD_ROOT="\$(git rev-parse --show-toplevel)" || exit 0
"\$GUSWORLD_ROOT/tools/asan_gate.sh" || exit 1
# --- fim GusWorld asan gate -------------------------------------------------

command -v git-lfs >/dev/null 2>&1 || { printf >&2 "\n%s\n\n" "This repository is configured for Git LFS but 'git-lfs' was not found on your path. If you no longer wish to use Git LFS, remove this hook by deleting the 'pre-push' file in the hooks directory (set by 'core.hookspath'; usually '.git/hooks')."; exit 2; }
git lfs pre-push "\$@"
EOF
)

_write_hook "pre-commit" "$PRE_COMMIT_CONTENT"
_write_hook "pre-push" "$PRE_PUSH_CONTENT"

if [ "$DRY_RUN" != "1" ]; then
    echo "install_hooks.sh: OK. Clientes chamados: tools/crash_journal_check.sh,"
    echo "  tools/gitleaks_staged_check.sh (pre-commit); tools/asan_gate.sh + git-lfs"
    echo "  (pre-push). core.hooksPath NAO foi tocado."
fi

#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
"""
Instala os hooks locais do GusWorld em .git/hooks/ (ou onde
"git rev-parse --git-common-dir" apontar, o que cobre worktree).

So COPIA o conteudo de tools/git-hooks/<nome> pro destino (nunca
symlink, pra funcionar identico em Windows sem privilegio especial), e
marca o bit de execucao onde o sistema de arquivos suporta. Idempotente:
pode rodar de novo a qualquer momento pra reinstalar depois de editar o
hook fonte.

NAO mexe no core.hooksPath compartilhado (~/.claude/githooks) nem em
nenhum arquivo fora deste repositorio -- ver
docs/security/commit-gate.md, secao "Como isto convive com o hooksPath
global", pra entender por que isso e seguro.

Uso:
    python3 tools/git-hooks/install.py
"""
from __future__ import annotations

import os
import shutil
import stat
import subprocess
import sys
from pathlib import Path

HOOKS = ["commit-msg"]


def git_common_dir(repo_root: Path) -> Path:
    resultado = subprocess.run(
        ["git", "-C", str(repo_root), "rev-parse", "--git-common-dir"],
        capture_output=True,
        text=True,
        check=True,
    )
    caminho = Path(resultado.stdout.strip())
    if not caminho.is_absolute():
        caminho = repo_root / caminho
    return caminho


def main() -> int:
    repo_root = Path(__file__).resolve().parent.parent.parent
    src_dir = repo_root / "tools" / "git-hooks"

    try:
        dst_dir = git_common_dir(repo_root) / "hooks"
    except (subprocess.CalledProcessError, FileNotFoundError) as exc:
        print(f"install_hooks: nao consegui localizar o repositorio git: {exc}", file=sys.stderr)
        return 1

    dst_dir.mkdir(parents=True, exist_ok=True)

    for nome in HOOKS:
        src = src_dir / nome
        if not src.exists():
            print(f"install_hooks: fonte ausente, pulei: {src}", file=sys.stderr)
            continue
        dst = dst_dir / nome
        shutil.copyfile(src, dst)
        modo_atual = os.stat(dst).st_mode
        os.chmod(dst, modo_atual | stat.S_IEXEC | stat.S_IXGRP | stat.S_IXOTH)
        print(f"install_hooks: instalado {dst}")

    print(
        "install_hooks: pronto. O gate roda a partir do proximo "
        "'git commit' nesta copia de trabalho."
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())

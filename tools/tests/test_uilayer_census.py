#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
"""test_uilayer_census.py - prova de que o censo de "UiLayer"
(tools/uilayer_census.py) e DETERMINISTICO e imune ao furo que tornou o N=233
original nao-reproduzivel (UILAYER-EXCLUSIVIDADE, 2026-08-06/07): a raiz do
problema era o `grep` cru varrendo `GusEngine/build/` (diretorio gitignored
que pode conter multiplas copias vendorizadas do glintfx via FetchContent).

`test_build_dir_gitignorado_nao_entra_na_contagem` e a prova VERMELHO-ANTES:
um `grep -rn` cru sobre o disco contaria as ocorrencias plantadas dentro de
um `build/` de brinquedo; este script, restrito a `git ls-files`, nao conta.

Uso: `python3 -m pytest tools/tests/test_uilayer_census.py -q`
"""

import os
import subprocess
import sys

import pytest

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import uilayer_census as census_mod  # noqa: E402


def _run(cmd, cwd):
    subprocess.run(cmd, cwd=cwd, check=True, capture_output=True, text=True)


def _write(path: str, content: str) -> None:
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w", encoding="utf-8") as f:
        f.write(content)


def _init_repo(tmp_path):
    _run(["git", "init", "-q"], cwd=str(tmp_path))
    _run(["git", "config", "user.email", "t@t.t"], cwd=str(tmp_path))
    _run(["git", "config", "user.name", "t"], cwd=str(tmp_path))
    return str(tmp_path)


def test_build_dir_gitignorado_nao_entra_na_contagem(tmp_path, monkeypatch):
    """A prova central: `build/` com centenas de "UiLayer" plantadas fica de
    fora, porque nunca e `git add`-ado (o mesmo estado real de
    GusEngine/build/ nesta maquina)."""
    repo = _init_repo(tmp_path)
    _write(os.path.join(repo, "GusEngine", "app", "src", "real.cpp"),
           "std::optional<glintfx::UiLayer> ui_;\n")
    _write(os.path.join(repo, ".gitignore"), "GusEngine/build/\n")
    # o "vendored copy" gitignorado, com 5 ocorrencias - simula _deps/glintfx-*
    _write(
        os.path.join(repo, "GusEngine", "build", "linux-release", "_deps",
                      "glintfx-src", "glintfx", "ui_layer.hpp"),
        "UiLayer\n" * 5,
    )
    _run(["git", "add", "GusEngine/app", ".gitignore"], cwd=repo)
    _run(["git", "commit", "-q", "-m", "init"], cwd=repo)

    monkeypatch.chdir(repo)
    files = census_mod.tracked_files()
    assert files == [os.path.join("GusEngine", "app", "src", "real.cpp")]

    hits = census_mod.census()
    assert len(hits) == 1  # NAO 6 (1 real + 5 do build/ gitignorado)


def test_apenas_cpp_e_hpp_contam(tmp_path, monkeypatch):
    """Extensao fora do escopo declarado (.py, .md, .rml) nao entra, mesmo
    rastreada e mesmo citando "UiLayer"."""
    repo = _init_repo(tmp_path)
    _write(os.path.join(repo, "GusEngine", "app", "src", "real.cpp"),
           "glintfx::UiLayer ui;\n")
    _write(os.path.join(repo, "GusEngine", "app", "tools", "spike", "main.rml"),
           "<!-- UiLayer mencionado aqui -->\n")
    _write(os.path.join(repo, "docs", "tech", "nota.md"),
           "fala de UiLayer\n")
    _run(["git", "add", "-A"], cwd=repo)
    _run(["git", "commit", "-q", "-m", "init"], cwd=repo)

    monkeypatch.chdir(repo)
    files = census_mod.tracked_files()
    assert files == [os.path.join("GusEngine", "app", "src", "real.cpp")]
    assert len(census_mod.census()) == 1


def test_fora_de_gusengine_nao_conta(tmp_path, monkeypatch):
    """O escopo e GusEngine/ - um .cpp fora dela (ex.: tools/ do proprio
    repo) nao entra, mesmo rastreado."""
    repo = _init_repo(tmp_path)
    _write(os.path.join(repo, "GusEngine", "app", "src", "real.cpp"),
           "glintfx::UiLayer ui;\n")
    _write(os.path.join(repo, "tools", "algo.cpp"),
           "// nao e GusEngine: UiLayer\n")
    _run(["git", "add", "-A"], cwd=repo)
    _run(["git", "commit", "-q", "-m", "init"], cwd=repo)

    monkeypatch.chdir(repo)
    files = census_mod.tracked_files()
    assert files == [os.path.join("GusEngine", "app", "src", "real.cpp")]


def test_zero_arquivo_rastreado_reprova_sem_ok_vazio(tmp_path, monkeypatch):
    repo = _init_repo(tmp_path)
    _write(os.path.join(repo, "README.md"), "nada aqui\n")
    _run(["git", "add", "-A"], cwd=repo)
    _run(["git", "commit", "-q", "-m", "init"], cwd=repo)

    monkeypatch.chdir(repo)
    assert census_mod.main([]) == 1


def test_determinismo_duas_chamadas_mesmo_resultado(tmp_path, monkeypatch):
    """Mesma arvore, duas chamadas -> mesmo N (nenhum estado de sessao,
    nenhuma dependencia de qual build/scratch existe no disco)."""
    repo = _init_repo(tmp_path)
    _write(os.path.join(repo, "GusEngine", "app", "src", "a.cpp"),
           "glintfx::UiLayer ui;\nUiLayer de novo\n")
    _write(os.path.join(repo, "GusEngine", "app", "include", "b.hpp"),
           "UiLayer\n")
    _run(["git", "add", "-A"], cwd=repo)
    _run(["git", "commit", "-q", "-m", "init"], cwd=repo)

    monkeypatch.chdir(repo)
    n1 = len(census_mod.census())
    n2 = len(census_mod.census())
    assert n1 == n2 == 3


# ------------------------------------------------------ o repo REAL, de hoje


def test_repo_real_e_deterministico_em_duas_chamadas():
    n1 = len(census_mod.census())
    n2 = len(census_mod.census())
    assert n1 == n2 > 0


def test_repo_real_main_ok():
    assert census_mod.main([]) == 0


if __name__ == "__main__":
    sys.exit(pytest.main([__file__, "-v"]))

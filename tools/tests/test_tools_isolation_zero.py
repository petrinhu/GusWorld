#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
"""test_tools_isolation_zero.py - prova das 3 regras de gate da casa para o
GATE(tools-isolation) (FATIA1-LOG-CLOCK, 2026-08-06/07):

  (a) mede de verdade  -> test_add_subdirectory_tools_e_pego (sabotagem
      controlada: escreve `add_subdirectory(tools)` numa arvore de brinquedo
      e prova que o gate REPROVA) + test_comentario_nao_conta (prova que o
      parser nao falso-positiva em texto que so MENCIONA a chamada).
  (b) nasce verde       -> test_repo_real_nasce_verde (roda contra o
      GusEngine/ real de hoje).
  (c) esta LIGADO        -> test_check_sh_gates_ligados.py (fora deste
      arquivo) confere que tools/check.sh soma GATE_TOOLS_ISOLATION na
      composicao do veredito final.

Uso: `python3 -m pytest tools/tests/test_tools_isolation_zero.py -q`
"""

import os
import sys

import pytest

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import tools_isolation_zero as gate  # noqa: E402


def _write(path: str, content: str) -> None:
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w", encoding="utf-8") as f:
        f.write(content)


def _arvore(tmp_path, raiz_content: str, app_content: str):
    raiz = str(tmp_path / "CMakeLists.txt")
    app = str(tmp_path / "app" / "CMakeLists.txt")
    _write(raiz, raiz_content)
    _write(app, app_content)
    return raiz, app


def test_add_subdirectory_tools_e_pego(tmp_path, monkeypatch):
    """SABOTAGEM CONTROLADA: a raiz do plano (docs/tech/plano-camadas-sdl.md
    secao 8, item 0) e exatamente este cenario - o dia em que alguem linkar
    app/tools/ no build real. VERMELHO-ANTES: sem este gate, nada acusa."""
    raiz, app = _arvore(
        tmp_path,
        "add_subdirectory(core)\nadd_subdirectory(app)\n",
        "add_subdirectory(tests)\nadd_subdirectory(tools)\n",
    )
    monkeypatch.setattr(gate, "ROOT", str(tmp_path))
    monkeypatch.setattr(gate, "SCAN_FILES", [raiz, app])

    offenders = gate.find_offenders()
    assert len(offenders) == 1
    assert offenders[0][0] == os.path.join("app", "CMakeLists.txt")
    assert gate.main() == 1


def test_add_subdirectory_tools_na_raiz_tambem_e_pego(tmp_path, monkeypatch):
    """O escopo cobre OS DOIS arquivos, nao so app/CMakeLists.txt."""
    raiz, app = _arvore(
        tmp_path,
        "add_subdirectory(tools)\n",
        "add_subdirectory(tests)\n",
    )
    monkeypatch.setattr(gate, "ROOT", str(tmp_path))
    monkeypatch.setattr(gate, "SCAN_FILES", [raiz, app])

    assert gate.main() == 1


def test_comentario_nao_conta(tmp_path, monkeypatch):
    """A propria documentacao deste gate MENCIONA `add_subdirectory(tools)`
    como texto (neste docstring, no plano, no TODO.md) - um grep cru casaria
    essas linhas. O parser (strip_cmake_line_comments) tem de ignorar
    comentario de CMake."""
    raiz, app = _arvore(
        tmp_path,
        "add_subdirectory(core)\n",
        "# nota: nunca fazer add_subdirectory(tools) aqui, ver plano\n"
        "add_subdirectory(tests)\n",
    )
    monkeypatch.setattr(gate, "ROOT", str(tmp_path))
    monkeypatch.setattr(gate, "SCAN_FILES", [raiz, app])

    assert gate.find_offenders() == []
    assert gate.main() == 0


def test_subdiretorio_novo_sem_tools_nao_barra(tmp_path, monkeypatch):
    """CRESCIMENTO LEGITIMO: `add_subdirectory` de qualquer OUTRO nome
    (camada nova, teste novo) nao aciona o gate - ele so olha o nome exato
    "tools"."""
    raiz, app = _arvore(
        tmp_path,
        "add_subdirectory(core)\nadd_subdirectory(domain)\nadd_subdirectory(app)\n",
        "add_subdirectory(tests)\nadd_subdirectory(toolshed)\n",
    )
    monkeypatch.setattr(gate, "ROOT", str(tmp_path))
    monkeypatch.setattr(gate, "SCAN_FILES", [raiz, app])

    assert gate.find_offenders() == []
    assert gate.main() == 0


def test_quoted_tools_tambem_e_pego(tmp_path, monkeypatch):
    """`add_subdirectory("tools")` (aspas) e a mesma violacao."""
    raiz, app = _arvore(
        tmp_path,
        "add_subdirectory(core)\n",
        'add_subdirectory("tools")\n',
    )
    monkeypatch.setattr(gate, "ROOT", str(tmp_path))
    monkeypatch.setattr(gate, "SCAN_FILES", [raiz, app])

    assert len(gate.find_offenders()) == 1
    assert gate.main() == 1


def test_arquivo_do_escopo_ausente_reprova_sem_ok_vazio(tmp_path, monkeypatch):
    """Sentinela de escopo: se um dos dois arquivos do escopo sumir, o gate
    reprova em vez de dizer OK sem ter olhado pra ele (mesma regra de
    escopo-colapsou dos outros gates zero-tolerancia da casa)."""
    raiz = str(tmp_path / "CMakeLists.txt")
    _write(raiz, "add_subdirectory(core)\n")
    inexistente = str(tmp_path / "app" / "CMakeLists.txt")
    monkeypatch.setattr(gate, "ROOT", str(tmp_path))
    monkeypatch.setattr(gate, "SCAN_FILES", [raiz, inexistente])

    assert gate.main() == 1


# ------------------------------------------------------ o repo REAL, de hoje


def test_repo_real_nasce_verde():
    """(b) NASCE VERDE: hoje nenhum dos dois CMakeLists.txt reais tem
    add_subdirectory(tools) - medido tambem por
    `grep -n 'add_subdirectory(tools)' GusEngine/CMakeLists.txt
    GusEngine/app/CMakeLists.txt` (vazio)."""
    assert gate.main() == 0


def test_repo_real_escopo_e_os_dois_arquivos():
    assert [os.path.relpath(p, gate.ROOT) for p in gate.SCAN_FILES] == [
        os.path.join("GusEngine", "CMakeLists.txt"),
        os.path.join("GusEngine", "app", "CMakeLists.txt"),
    ]


if __name__ == "__main__":
    sys.exit(pytest.main([__file__, "-v"]))

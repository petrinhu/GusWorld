#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
"""test_fetchcontent_manifest.py - prova que o GATE(fetchcontent-manifest)
enxerga o GRAFO de add_subdirectory, nao um arquivo so (FURO 2 da auditoria dos
gates, GATES-HARDEN 2026-08-06).

Ate esta fatia o gate lia EXATAMENTE `GusEngine/CMakeLists.txt`. Como
`core/`, `domain/`, `platform/`, `app/`, `tests/` e `third_party/monocypher/`
sao `add_subdirectory` DELE, um `FetchContent_Declare` em qualquer um deles
entrava no build real e passava VERDE. Nao havia divida no dia da medicao; o
furo era prospectivo - e este teste e o que o transforma em impossivel.

`test_declare_em_subdiretorio_e_pego` e a prova VERMELHO-ANTES: a MESMA arvore
de brinquedo passa varrendo so a raiz e reprova varrendo o grafo.

Uso: `python3 -m pytest tools/tests/test_fetchcontent_manifest.py -q`
"""

import os
import sys

import pytest

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import fetchcontent_manifest as gate  # noqa: E402


def _write(path: str, content: str) -> None:
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w", encoding="utf-8") as f:
        f.write(content)


def _arvore(tmp_path, sub_content: str, raiz_extra: str = "") -> str:
    """Raiz com add_subdirectory(core) + um subdir standalone NAO alcancavel."""
    raiz = str(tmp_path / "CMakeLists.txt")
    _write(
        raiz,
        "FetchContent_Declare(SDL3 GIT_REPOSITORY x)\n"
        "add_subdirectory(core)\n" + raiz_extra,
    )
    _write(str(tmp_path / "core" / "CMakeLists.txt"), sub_content)
    _write(
        str(tmp_path / "tools" / "spike" / "CMakeLists.txt"),
        "FetchContent_Declare(imgui GIT_REPOSITORY x)\n",
    )
    return raiz


def test_declare_em_subdiretorio_e_pego(tmp_path, monkeypatch):
    """A prova do furo: dependencia nova num CMakeLists.txt de camada."""
    raiz = _arvore(tmp_path, "FetchContent_Declare(fmt GIT_REPOSITORY x)\n")
    monkeypatch.setattr(gate, "ROOT", str(tmp_path))
    monkeypatch.setattr(gate, "MANIFEST", raiz)

    # COMPORTAMENTO ANTIGO (so a raiz): `fmt` invisivel - passava verde.
    antigos = gate.find_declared_names([raiz])
    assert "fmt" not in antigos

    # COMPORTAMENTO NOVO (grafo): `fmt` aparece e o gate reprova.
    files = gate.collect_build_cmakelists(raiz)
    assert "fmt" in gate.find_declared_names(files)
    assert gate.main() == 1


def test_subdiretorio_standalone_fica_de_fora_por_construcao(tmp_path, monkeypatch):
    """`app/tools/*` e standalone: NAO e add_subdirectory de ninguem, entao e
    inalcancavel pelo walk. A exclusao correta e documentada continua valendo -
    agora por estrutura, nao por lista de exclusao escrita a mao."""
    raiz = _arvore(tmp_path, "# camada limpa\n")
    monkeypatch.setattr(gate, "ROOT", str(tmp_path))
    monkeypatch.setattr(gate, "MANIFEST", raiz)

    files = gate.collect_build_cmakelists(raiz)
    assert not any("spike" in f for f in files)
    assert "imgui" not in gate.find_declared_names(files)
    assert gate.main() == 0  # CRESCIMENTO LEGITIMO: o standalone nao barra nada


def test_add_subdirectory_em_comentario_nao_entra_no_walk(tmp_path, monkeypatch):
    """O CMakeLists.txt real DOCUMENTA `add_subdirectory` em comentario (linha
    611). Seguir comentario faria o walk inventar arquivo."""
    raiz = _arvore(
        tmp_path,
        "# camada limpa\n",
        raiz_extra="# nota: add_subdirectory(inexistente) so quando top-level\n",
    )
    monkeypatch.setattr(gate, "ROOT", str(tmp_path))
    monkeypatch.setattr(gate, "MANIFEST", raiz)
    files = gate.collect_build_cmakelists(raiz)
    assert not any("inexistente" in f for f in files)


def test_walk_nao_entra_em_loop(tmp_path, monkeypatch):
    """Ciclo A->B->A nao pode pendurar o gate."""
    _write(str(tmp_path / "CMakeLists.txt"), "add_subdirectory(a)\n")
    _write(str(tmp_path / "a" / "CMakeLists.txt"), "add_subdirectory(../a)\n")
    files = gate.collect_build_cmakelists(str(tmp_path / "CMakeLists.txt"))
    assert len(files) == 2


def test_subdiretorio_novo_entra_sozinho(tmp_path, monkeypatch):
    """CRESCIMENTO LEGITIMO: camada nova sem dependencia nova nao barra, e ja
    nasce coberta - sem ninguem lembrar de acrescentar nada ao script."""
    raiz = _arvore(tmp_path, "add_subdirectory(sub)\n")
    _write(str(tmp_path / "core" / "sub" / "CMakeLists.txt"), "# nada aqui\n")
    monkeypatch.setattr(gate, "ROOT", str(tmp_path))
    monkeypatch.setattr(gate, "MANIFEST", raiz)

    files = gate.collect_build_cmakelists(raiz)
    assert any(os.path.join("core", "sub") in f for f in files)
    assert gate.main() == 0


# ------------------------------------------------------ o repo REAL, de hoje


def test_repo_real_cobre_as_4_camadas():
    files = [os.path.relpath(f, gate.ROOT) for f in gate.collect_build_cmakelists()]
    for esperado in ("core", "domain", "platform", "app"):
        assert os.path.join("GusEngine", esperado, "CMakeLists.txt") in files, files
    # E NAO cobre os standalone de app/tools/ (exclusao por construcao).
    assert not any(os.path.join("app", "tools") in f for f in files), files


def test_repo_real_nasce_verde():
    assert gate.main() == 0


if __name__ == "__main__":
    sys.exit(pytest.main([__file__, "-v"]))

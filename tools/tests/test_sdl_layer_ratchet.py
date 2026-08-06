#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
"""test_sdl_layer_ratchet.py - prova que o escopo do GATE(sdl-ratchet) alcanca
`app/main.cpp` (FURO 1 da auditoria dos gates, GATES-HARDEN 2026-08-06).

O ratchet varria `app/src` + `app/include/gus/app` e nada mais. `app/main.cpp`
mora na RAIZ de app/ e e a UNICA fonte do executavel (`add_executable(
gusworld_app main.cpp)`) - ficava fora do gate, e carregava um
`#include <SDL3/SDL.h>` orfao que ninguem via. O include saiu no mesmo commit
(zero token SDL_* fora de comentario no arquivo), entao o teto 24 nao subiu.

`test_main_cpp_seria_infrator_se_voltasse_o_include` e a prova VERMELHO-ANTES:
ele reconstroi a versao antiga do arquivo (com o include) numa arvore de
brinquedo e confirma que o gate de HOJE a pegaria - o de ontem nao pegava.

Uso: `python3 -m pytest tools/tests/test_sdl_layer_ratchet.py -q`
"""

import os
import sys

import pytest

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import sdl_layer_ratchet as gate  # noqa: E402


def _write(path: str, content: str) -> None:
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w", encoding="utf-8") as f:
        f.write(content)


def _fake_app(tmp_path, main_content: str, src_content: str = "// nada\n"):
    app = str(tmp_path / "GusEngine" / "app")
    _write(os.path.join(app, "src", "limpo.cpp"), src_content)
    _write(os.path.join(app, "main.cpp"), main_content)
    return app


def _patch(monkeypatch, tmp_path, app, com_main_cpp: bool):
    monkeypatch.setattr(gate, "ROOT", str(tmp_path))
    monkeypatch.setattr(gate, "PRODUCTION_DIRS", [os.path.join(app, "src")])
    monkeypatch.setattr(
        gate,
        "PRODUCTION_FILES",
        [os.path.join(app, "main.cpp")] if com_main_cpp else [],
    )


def test_main_cpp_seria_infrator_se_voltasse_o_include(tmp_path, monkeypatch):
    """VERMELHO-ANTES/VERDE-DEPOIS na MESMA arvore, so mudando o escopo."""
    app = _fake_app(tmp_path, "#include <SDL3/SDL.h>\nint main(){return 0;}\n")

    # Escopo ANTIGO (so dirs): o include em main.cpp e INVISIVEL - este era o furo.
    _patch(monkeypatch, tmp_path, app, com_main_cpp=False)
    assert gate.find_offenders() == []

    # Escopo NOVO: o mesmo arquivo, na mesma arvore, agora e pego.
    _patch(monkeypatch, tmp_path, app, com_main_cpp=True)
    assert gate.find_offenders() == [os.path.join("GusEngine", "app", "main.cpp")]


def test_main_cpp_limpo_nao_conta(tmp_path, monkeypatch):
    """CRESCIMENTO LEGITIMO: main.cpp no escopo, sem SDL, nao mexe na contagem -
    e por isso o teto ficou 24, nao 25."""
    app = _fake_app(tmp_path, "// sem SDL nenhum\nint main(){return 0;}\n")
    _patch(monkeypatch, tmp_path, app, com_main_cpp=True)
    assert gate.find_offenders() == []


def test_mencao_a_sdl_em_comentario_nao_conta(tmp_path, monkeypatch):
    """O comentario que o proprio main.cpp ganhou (explicando por que o include
    saiu) MENCIONA `SDL3/SDL.h` e `SDL_*`. Um grep cru reprovaria justamente o
    arquivo consertado; o parser de caractere nao."""
    app = _fake_app(
        tmp_path,
        "// SEM `#include <SDL3/SDL.h>`: zero token SDL_Init aqui.\n"
        "int main(){return 0;}\n",
    )
    _patch(monkeypatch, tmp_path, app, com_main_cpp=True)
    assert gate.find_offenders() == []


def test_escopo_zero_reprova(tmp_path, monkeypatch):
    """Sentinela: escopo colapsado nao pode imprimir 'OK (no teto)' varrendo 0."""
    monkeypatch.setattr(gate, "ROOT", str(tmp_path))
    monkeypatch.setattr(gate, "PRODUCTION_DIRS", [str(tmp_path / "nao_existe")])
    monkeypatch.setattr(gate, "PRODUCTION_FILES", [])
    assert gate.main() == 1


# ------------------------------------------------------ o repo REAL, de hoje


def test_main_cpp_real_esta_no_escopo():
    varridos = {os.path.abspath(p) for p in gate.iter_production_files()}
    main_cpp = os.path.join(gate.APP, "main.cpp")
    assert os.path.isfile(main_cpp)
    assert os.path.abspath(main_cpp) in varridos


def test_teto_real_continua_valendo():
    """Nasce verde: com main.cpp DENTRO do escopo, a contagem real segue <= 24."""
    n = len(gate.find_offenders())
    assert n <= gate.CEILING, f"{n} > teto {gate.CEILING}"


if __name__ == "__main__":
    sys.exit(pytest.main([__file__, "-v"]))

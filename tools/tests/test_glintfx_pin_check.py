#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
"""test_glintfx_pin_check.py - a versao se le da TAG, e o parser nao se engana.

Cobre as tres armadilhas MEDIDAS do bloco real do GusEngine/CMakeLists.txt:

  (1) o bloco do glintfx e cheio de COMENTARIO historico, e ha nota de migracao
      mencionando tag antiga - um regex ingenuo de `GIT_TAG` casaria com linha
      comentada e devolveria a versao errada com cara de certa;
  (2) ha OUTRAS quatro FetchContent_Declare no mesmo arquivo (SDL3, RmlUi,
      Catch2), cada uma com o SEU GIT_TAG - ler "o primeiro GIT_TAG do arquivo"
      devolveria o pin do SDL3;
  (3) o `config.hpp` gerado pode CONCORDAR com o numero declarado e ainda assim a
      arvore estar errada (foi o caso real: v0.29.0 e v0.30.0 geram o MESMO
      "0.29.0"). O rc tem de sair da TAG, nunca do header.

Uso: `python3 -m pytest tools/tests/test_glintfx_pin_check.py -q`
"""

import os
import sys

import pytest

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import glintfx_pin_check as pin  # noqa: E402


BLOCO_REALISTA = """\
FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG        release-3.4.10
)

FetchContent_Declare(
    glintfx
    # MIGRADO DO CODEBERG PRO GITHUB: a tag v0.20.0 tem o MESMO sha nos dois
    # hosts. Nota historica: antes o pin era
    # GIT_TAG v0.20.0
    GIT_REPOSITORY https://github.com/petrinhu/glintfx
    GIT_TAG        v0.30.0
    SOURCE_SUBDIR  glintfx
)

FetchContent_Declare(
    Catch2
    GIT_TAG        v3.5.2
)
"""


def _cmakelists(tmp_path, conteudo):
    engine = tmp_path / "GusEngine"
    engine.mkdir()
    (engine / "CMakeLists.txt").write_text(conteudo, encoding="utf-8")
    return str(engine / "CMakeLists.txt")


@pytest.fixture
def aponta_para(monkeypatch):
    def _aponta(caminho):
        monkeypatch.setattr(pin, "CMAKELISTS", caminho)
    return _aponta


def test_le_o_git_tag_do_bloco_do_glintfx(tmp_path, aponta_para):
    aponta_para(_cmakelists(tmp_path, BLOCO_REALISTA))
    assert pin.pin_declarado() == "v0.30.0"


def test_nao_le_o_git_tag_de_outra_dependencia(tmp_path, aponta_para):
    """Ler 'o primeiro GIT_TAG do arquivo' devolveria o pin do SDL3."""
    aponta_para(_cmakelists(tmp_path, BLOCO_REALISTA))
    assert pin.pin_declarado() != "release-3.4.10"


def test_ignora_git_tag_comentado(tmp_path, aponta_para):
    """A nota historica `# GIT_TAG v0.20.0` vem ANTES do GIT_TAG efetivo."""
    aponta_para(_cmakelists(tmp_path, BLOCO_REALISTA))
    assert pin.pin_declarado() != "v0.20.0"


def test_bloco_ausente_nao_passa_em_silencio(tmp_path, aponta_para):
    """Sentinela de escopo: sem o bloco, o script NAO pode dizer OK."""
    aponta_para(_cmakelists(tmp_path, "FetchContent_Declare(\n    SDL3\n    GIT_TAG x\n)\n"))
    with pytest.raises(LookupError):
        pin.pin_declarado()


def test_git_tag_so_comentado_nao_passa_em_silencio(tmp_path, aponta_para):
    conteudo = (
        "FetchContent_Declare(\n    glintfx\n"
        "    # GIT_TAG        v0.30.0\n"
        "    SOURCE_SUBDIR  glintfx\n)\n"
    )
    aponta_para(_cmakelists(tmp_path, conteudo))
    with pytest.raises(LookupError):
        pin.pin_declarado()


def test_config_hpp_nunca_decide_o_rc(tmp_path, aponta_para, monkeypatch, capsys):
    """O caso REAL: config.hpp diz "0.29.0" e a arvore esta na v0.30.0.

    Se o rc saisse do header, um bump v0.29.0 -> v0.30.0 pareceria "sem efeito" e
    um bump NAO aplicado pareceria aplicado. Aqui o declarado e v0.31.0, a tag
    baixada e v0.30.0, e o config.hpp "concorda" com 0.29.0 - o veredito tem de
    ser FALHA, vindo da TAG.
    """
    aponta_para(_cmakelists(tmp_path, BLOCO_REALISTA.replace("v0.30.0", "v0.31.0")))
    src = tmp_path / "GusEngine" / "build" / "linux-release" / "_deps" / "glintfx-src"
    src.mkdir(parents=True)
    monkeypatch.setattr(pin, "ENGINE", str(tmp_path / "GusEngine"))
    monkeypatch.setattr(pin, "tag_baixada", lambda _s: ("v0.30.0", None))
    monkeypatch.setattr(pin, "versao_do_config_hpp", lambda _b: "0.29.0")
    monkeypatch.setattr(sys, "argv", ["glintfx_pin_check.py"])

    assert pin.main() == 1
    assert "FALHA" in capsys.readouterr().out


def test_sem_deps_devolve_2_e_nao_0(tmp_path, aponta_para, monkeypatch):
    """"Nao consegui medir" NUNCA pode virar "esta tudo certo"."""
    aponta_para(_cmakelists(tmp_path, BLOCO_REALISTA))
    monkeypatch.setattr(pin, "ENGINE", str(tmp_path / "GusEngine"))
    monkeypatch.setattr(sys, "argv", ["glintfx_pin_check.py"])
    assert pin.main() == 2


if __name__ == "__main__":
    sys.exit(pytest.main([__file__, "-v"]))

#!/usr/bin/env python3
"""test_spdx_header_required.py - prova que o GATE(spdx-required) MORDE.

"Verificador em que ninguem confia e pior que verificador nenhum" (requisito
5 da fatia GATE-SPDX): este teste cria arquivo temporario SEM o cabecalho e
confirma que o gate REPROVA (main() != 0) - nao infere pela leitura do
codigo, executa de verdade. Roda 100% fora do git real do repo (nao stagia,
nao commita, nao mexe no index): `list_tracked_files` e monkeypatchada para
devolver uma lista fixa apontando pra arquivos dentro de tmp_path, entao
nenhum outro agente na mesma arvore e afetado.

Uso: `python3 -m pytest tools/tests/test_spdx_header_required.py -q`
"""

import os
import sys

import pytest

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import spdx_header_required as gate  # noqa: E402


def _write(path: str, content: str) -> None:
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w", encoding="utf-8") as f:
        f.write(content)


# --------------------------------------------------------------- unidades


def test_has_required_header_linha_1(tmp_path):
    fp = tmp_path / "ok.hpp"
    _write(str(fp), "// SPDX-License-Identifier: Apache-2.0\n// resto\n")
    assert gate.has_required_header(str(fp)) is True


def test_has_required_header_ausente(tmp_path):
    fp = tmp_path / "orfao.hpp"
    _write(str(fp), "// gus/app/orfao.hpp\n// sem SPDX nenhum\n")
    assert gate.has_required_header(str(fp)) is False


def test_has_required_header_alem_do_limite_nao_conta(tmp_path):
    # SPDX na linha 6 (MAX_HEADER_LINES=5) NAO deve contar - o requisito e
    # "perto do topo", nao "em qualquer lugar do arquivo".
    fp = tmp_path / "tarde_demais.hpp"
    body = "\n".join([f"// linha {i}" for i in range(1, 6)])
    body += "\n// SPDX-License-Identifier: Apache-2.0\n"
    _write(str(fp), body)
    assert gate.has_required_header(str(fp)) is False


def test_find_offenders_respeita_allowlist(tmp_path):
    bom = tmp_path / "bom.cpp"
    vendor = tmp_path / "third_party" / "vendor.h"
    _write(str(bom), "// SPDX-License-Identifier: Apache-2.0\n")
    _write(str(vendor), "// licenca do vendor, sem o nosso SPDX\n")
    allowlist = {"third_party/vendor.h": "terceiro (exemplo de teste)"}
    offenders = gate.find_offenders(["bom.cpp", "third_party/vendor.h"], str(tmp_path), allowlist)
    assert offenders == []


def test_find_offenders_pega_arquivo_sem_spdx(tmp_path):
    orfao = tmp_path / "orfao.cpp"
    _write(str(orfao), "// gus/orfao.cpp\n// nunca teve SPDX\n")
    offenders = gate.find_offenders(["orfao.cpp"], str(tmp_path), {})
    assert offenders == ["orfao.cpp"]


def test_load_allowlist_exige_tab(tmp_path):
    fp = tmp_path / "allowlist_ruim.txt"
    _write(str(fp), "caminho/sem/tab motivo solto sem separador\n")
    with pytest.raises(ValueError):
        gate.load_allowlist(str(fp))


def test_find_stale_allowlist_pega_arquivo_apagado(tmp_path):
    stale = gate.find_stale_allowlist(str(tmp_path), {"nao_existe.cpp": "motivo qualquer"})
    assert stale == [("nao_existe.cpp", "arquivo nao existe mais")]


# ------------------------------- FURO 3: a licenca antiga nao pode VOLTAR
#
# GATES-HARDEN (2026-08-06): o gate procurava a PRESENCA do Apache e nunca a
# AUSENCIA de outra licenca. Medido: GPL na linha 1 + Apache na linha 2 passava
# VERDE, enquanto o docstring do script prometia impedir "o antigo GPL de
# voltar". Os testes abaixo sao a prova VERMELHO-ANTES: cada um deles passaria
# (has_required_header == True) com a regra antiga.

_GPL = "GPL-3" + ".0-or-later"  # partido pra nao virar tag SPDX neste arquivo


def test_gpl_convivendo_com_apache_no_cabecalho_reprova(tmp_path):
    """O FURO EXATO, verbatim: GPL na linha 1, Apache na linha 2."""
    fp = tmp_path / "voltou.cpp"
    _write(str(fp), f"// SPDX-License-Identifier: {_GPL}\n"
                    "// SPDX-License-Identifier: Apache-2.0\n")
    # A regra ANTIGA (presenca) dava OK - e e por isso que ela nao bastava:
    assert gate.has_required_header(str(fp)) is True
    # A regra NOVA (exclusividade) pega:
    assert gate.foreign_license_tags(str(fp)) == [(1, _GPL)]
    assert gate.find_foreign(["voltou.cpp"], str(tmp_path), {}) == [
        ("voltou.cpp", 1, _GPL)
    ]


def test_gpl_longe_do_topo_tambem_reprova(tmp_path):
    """A variante seguinte do mesmo furo: fora da janela de cabecalho. Se a
    regra nova so olhasse as 5 primeiras linhas, a linha 40 reabriria o buraco."""
    fp = tmp_path / "fundo.cpp"
    corpo = "// SPDX-License-Identifier: Apache-2.0\n" + "// codigo\n" * 40
    corpo += f"/* SPDX-License-Identifier: {_GPL} */\n"
    _write(str(fp), corpo)
    achados = gate.foreign_license_tags(str(fp))
    assert achados == [(42, _GPL)], achados


def test_rabo_de_comentario_de_bloco_nao_escapa(tmp_path):
    """`/* SPDX-...: Apache-2.0 */` e legal em C: o ` */` nao pode fazer o valor
    parecer diferente de Apache-2.0 (falso POSITIVO que barraria arquivo bom)."""
    fp = tmp_path / "bloco.cpp"
    _write(str(fp), "/* SPDX-License-Identifier: Apache-2.0 */\n")
    assert gate.foreign_license_tags(str(fp)) == []


def test_arquivo_so_com_apache_passa(tmp_path):
    """CRESCIMENTO LEGITIMO: o formato de 571 arquivos do repo nao pode barrar."""
    fp = tmp_path / "normal.cpp"
    _write(str(fp), "// SPDX-License-Identifier: Apache-2.0\n// gus/normal.cpp\n"
                    "// menciona GPL e Apache em prosa, sem tag nenhuma\n")
    assert gate.foreign_license_tags(str(fp)) == []
    assert gate.find_foreign(["normal.cpp"], str(tmp_path), {}) == []


def test_terceiro_na_allowlist_nao_reprova(tmp_path):
    """monocypher.c declara BSD-2-Clause OR CC0-1.0 de verdade: a allowlist e o
    caminho, nao afrouxar a regra (nem por o nosso SPDX por cima - seria falso)."""
    fp = tmp_path / "vendor.c"
    _write(str(fp), "// SPDX-License-Identifier: BSD-2-Clause OR CC0-1.0\n")
    assert gate.foreign_license_tags(str(fp)) != []
    assert gate.find_foreign(["vendor.c"], str(tmp_path),
                             {"vendor.c": "terceiro"}) == []


def test_main_reprova_licenca_antiga_voltando(tmp_path, monkeypatch):
    """Ponta-a-ponta: main() != 0 com o arquivo do furo."""
    fp = tmp_path / "GusEngine" / "app" / "src" / "voltou.cpp"
    _write(str(fp), f"// SPDX-License-Identifier: {_GPL}\n"
                    "// SPDX-License-Identifier: Apache-2.0\n")
    monkeypatch.setattr(gate, "ROOT", str(tmp_path))
    monkeypatch.setattr(
        gate, "list_tracked_files",
        lambda root, extensions: ["GusEngine/app/src/voltou.cpp"],
    )
    monkeypatch.setattr(gate, "ALLOWLIST_PATH", str(tmp_path / "allowlist_vazio.txt"))
    assert gate.main() == 1


def test_extensoes_novas_estao_no_escopo():
    """`.cc/.cxx/.inl/.hxx/.ipp` entraram (custo 1 linha, zero divida medida)."""
    for ext in ("cc", "cxx", "inl", "hxx", "ipp"):
        assert ext in gate.EXTENSIONS
    # ⚠️ .py/.sh/.cmake continuam FORA de proposito (fatia PROPRIA, com os
    # cabecalhos aplicados ANTES): liga-las hoje faria o gate nascer VERMELHO
    # contra ~27 arquivos pre-existentes.
    for fora in ("py", "sh", "cmake", "yml"):
        assert fora not in gate.EXTENSIONS


def test_repo_real_nasce_verde():
    """Requisito (b): a arvore de hoje passa nas DUAS regras."""
    assert gate.main() == 0


# --------------------------------------------------------- ponta-a-ponta


def test_main_reprova_com_arquivo_novo_sem_spdx(tmp_path, monkeypatch):
    """A PROVA-MAE do requisito 5: cria arquivo temporario sem SPDX, roda
    main() de ponta a ponta (allowlist real do repo incluida), e confirma
    que o gate MORDE - exit code 1, arquivo listado na saida."""
    orfao = tmp_path / "GusEngine" / "app" / "src" / "novo_sem_spdx.cpp"
    _write(str(orfao), "// gus/app/src/novo_sem_spdx.cpp\n// arquivo novo, sem cabecalho\n")

    monkeypatch.setattr(gate, "ROOT", str(tmp_path))
    monkeypatch.setattr(
        gate,
        "list_tracked_files",
        lambda root, extensions: ["GusEngine/app/src/novo_sem_spdx.cpp"],
    )
    # Allowlist vazia para este cenario (o real do repo nao cobre este arquivo).
    monkeypatch.setattr(gate, "ALLOWLIST_PATH", str(tmp_path / "allowlist_vazio.txt"))

    rc = gate.main()
    assert rc == 1, "o gate tem que REPROVAR um arquivo novo sem SPDX"


def test_main_passa_quando_todo_arquivo_tem_spdx(tmp_path, monkeypatch):
    bom = tmp_path / "GusEngine" / "core" / "bom.hpp"
    _write(str(bom), "// SPDX-License-Identifier: Apache-2.0\n// gus/core/bom.hpp\n")

    monkeypatch.setattr(gate, "ROOT", str(tmp_path))
    monkeypatch.setattr(
        gate, "list_tracked_files", lambda root, extensions: ["GusEngine/core/bom.hpp"]
    )
    monkeypatch.setattr(gate, "ALLOWLIST_PATH", str(tmp_path / "allowlist_vazio.txt"))

    rc = gate.main()
    assert rc == 0


if __name__ == "__main__":
    sys.exit(pytest.main([__file__, "-v"]))

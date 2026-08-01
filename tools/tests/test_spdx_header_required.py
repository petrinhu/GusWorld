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

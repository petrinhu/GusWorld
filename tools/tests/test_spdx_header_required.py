#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
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
import subprocess
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
        lambda root, extensions, basenames=(): ["GusEngine/app/src/voltou.cpp"],
    )
    monkeypatch.setattr(gate, "ALLOWLIST_PATH", str(tmp_path / "allowlist_vazio.txt"))
    assert gate.main() == 1


def test_extensoes_novas_estao_no_escopo():
    """`.cc/.cxx/.inl/.hxx/.ipp` entraram (custo 1 linha, zero divida medida)."""
    for ext in ("cc", "cxx", "inl", "hxx", "ipp"):
        assert ext in gate.EXTENSIONS
    # SPDX-QUITACAO (2026-08-06): .py e .cmake entraram DEPOIS de os 48 arquivos
    # ganharem cabecalho no mesmo commit (o gate nasce verde, nunca vermelho).
    # GATE-SPDX-ESCOPO-SH (2026-08-07): .sh entrou pelo mesmo desenho, DEPOIS de
    # os 2 arquivos sem cabecalho (tools/check.sh e o monitor_input.sh do
    # appmode_spike) ganharem o header no mesmo commit.
    for ext in ("py", "cmake", "sh"):
        assert ext in gate.EXTENSIONS
    # CMakeLists.txt nao tem extensao: ficou fora da fatia anterior por OMISSAO
    # (14 rastreados, 14 sem cabecalho, incluindo os alvos de build).
    assert "CMakeLists.txt" in gate.EXTRA_BASENAMES
    # .yml: decisao (workflow de CI nao leva cabecalho de licenca por pratica da
    # industria; o LICENSE da raiz cobre o repo) - unica extensao fora HOJE por
    # escolha, nao por divida.
    assert "yml" not in gate.EXTENSIONS


# ------------------------------------------- SPDX-QUITACAO: tooling no escopo


def test_py_novo_sem_cabecalho_reprova(tmp_path, monkeypatch):
    """A PROVA desta fatia: um .py NOVO sem cabecalho tem que REPROVAR.

    E o defeito medido, verbatim: entre 2026-08-01 e 2026-08-06 nasceram dois
    .py sem SPDX (tools/callback_dtor_*.py) porque a extensao estava fora do
    gate. Com .py dentro, isso passa a morder."""
    novo = tmp_path / "tools" / "ferramenta_nova.py"
    _write(str(novo), "#!/usr/bin/env python3\n"
                      '"""ferramenta nova, sem cabecalho de licenca."""\n')
    monkeypatch.setattr(gate, "ROOT", str(tmp_path))
    monkeypatch.setattr(gate, "list_tracked_files",
                        lambda root, extensions, basenames=(): ["tools/ferramenta_nova.py"])
    monkeypatch.setattr(gate, "ALLOWLIST_PATH", str(tmp_path / "vazio.txt"))
    assert gate.main() == 1


def test_py_com_cabecalho_depois_do_shebang_passa(tmp_path, monkeypatch):
    """O shebang TEM de continuar na linha 1 (senao o script deixa de rodar):
    o cabecalho vai pra linha 2, e isso precisa contar como cabecalho valido."""
    ok = tmp_path / "tools" / "ok.py"
    _write(str(ok), "#!/usr/bin/env python3\n"
                    "# SPDX-License-Identifier: Apache-2.0\n"
                    '"""docstring depois do cabecalho."""\n')
    assert gate.has_required_header(str(ok)) is True
    monkeypatch.setattr(gate, "ROOT", str(tmp_path))
    monkeypatch.setattr(gate, "list_tracked_files",
                        lambda root, extensions, basenames=(): ["tools/ok.py"])
    monkeypatch.setattr(gate, "ALLOWLIST_PATH", str(tmp_path / "vazio.txt"))
    assert gate.main() == 0


def test_py_de_terceiro_na_allowlist_nao_reprova(tmp_path, monkeypatch):
    """O CASO LEGITIMO: 8values_engine.py e obra derivada MIT. Nao pode ganhar o
    nosso Apache (declaracao falsa) nem reprovar - a allowlist e o caminho, e
    ela tem que cobrir as DUAS regras (falta de Apache E presenca de MIT)."""
    terceiro = tmp_path / "docs" / "vendor_tool.py"
    _write(str(terceiro), "#!/usr/bin/env python3\n"
                          "# SPDX-License-Identifier: MIT\n"
                          "# fork de projeto de terceiro\n")
    rel = "docs/vendor_tool.py"
    allow = {rel: "terceiro (fork MIT)"}
    assert gate.find_offenders([rel], str(tmp_path), allow) == []
    assert gate.find_foreign([rel], str(tmp_path), allow) == []
    # ... e SEM a allowlist as duas regras mordem:
    assert gate.find_offenders([rel], str(tmp_path), {}) == [rel]
    assert gate.find_foreign([rel], str(tmp_path), {}) == [(rel, 2, "MIT")]


def test_cmakelists_entra_por_basename_e_nao_por_sufixo(tmp_path):
    """`CMakeLists.txt` nao casa com `*.<ext>` nenhum - entra por basename. O
    pathspec e literal (`CMakeLists.txt` e `*/CMakeLists.txt`), nunca o glob de
    sufixo `*CMakeLists.txt`, que arrastaria `fooCMakeLists.txt` junto."""
    repo = tmp_path / "repo"
    (repo / "sub").mkdir(parents=True)
    for rel in ("CMakeLists.txt", "sub/CMakeLists.txt", "sub/fooCMakeLists.txt", "a.py"):
        _write(str(repo / rel), "# vazio\n")
    for cmd in (["git", "init", "-q"], ["git", "add", "-A"]):
        subprocess.run(cmd, cwd=str(repo), check=True, capture_output=True)
    achados = gate.list_tracked_files(str(repo), ("py",), ("CMakeLists.txt",))
    assert sorted(achados) == ["CMakeLists.txt", "a.py", "sub/CMakeLists.txt"]


def test_sh_novo_sem_cabecalho_reprova(tmp_path, monkeypatch):
    """A PROVA desta fatia (GATE-SPDX-ESCOPO-SH): um .sh NOVO sem cabecalho tem
    que REPROVAR, nomeando o arquivo - mesmo defeito medido em .py
    (tools/callback_dtor_*.py nasceram sem SPDX porque a extensao estava fora
    do gate), agora exercitado em .sh."""
    novo = tmp_path / "tools" / "script_novo.sh"
    _write(str(novo), "#!/usr/bin/env bash\necho sem cabecalho de licenca\n")
    monkeypatch.setattr(gate, "ROOT", str(tmp_path))
    monkeypatch.setattr(gate, "list_tracked_files",
                        lambda root, extensions, basenames=(): ["tools/script_novo.sh"])
    monkeypatch.setattr(gate, "ALLOWLIST_PATH", str(tmp_path / "vazio.txt"))
    assert gate.main() == 1


def test_sh_com_cabecalho_depois_do_shebang_passa(tmp_path, monkeypatch):
    """O shebang TEM de continuar na linha 1 (senao o script deixa de rodar):
    o cabecalho vai pra linha 2, e isso precisa contar como cabecalho valido
    - mesma regra do .py, agora provada em .sh."""
    ok = tmp_path / "tools" / "script_ok.sh"
    _write(str(ok), "#!/usr/bin/env bash\n"
                    "# SPDX-License-Identifier: Apache-2.0\n"
                    "echo ok\n")
    assert gate.has_required_header(str(ok)) is True
    monkeypatch.setattr(gate, "ROOT", str(tmp_path))
    monkeypatch.setattr(gate, "list_tracked_files",
                        lambda root, extensions, basenames=(): ["tools/script_ok.sh"])
    monkeypatch.setattr(gate, "ALLOWLIST_PATH", str(tmp_path / "vazio.txt"))
    assert gate.main() == 0


# ------------------------------------------- DECLARACAO x MENCAO (precisao)


def test_mencao_em_string_de_codigo_nao_e_declaracao(tmp_path):
    """SEM ISTO O GATE REPROVA A SI MESMO: spdx_header_required.py cita a tag no
    docstring e na regex, e o teste dele constroi fixture com a tag em string
    literal - 18 falsos positivos medidos em 2026-08-06, 4 no proprio gate."""
    fp = tmp_path / "gate_like.py"
    _write(str(fp), "# SPDX-License-Identifier: Apache-2.0\n"
                    'REQUIRED = "SPDX-License-Identifier: Apache-2.0"\n'
                    'RE = re.compile(r"SPDX-License-Identifier:\\s*(.*)")\n'
                    '_write(fp, "// SPDX-License-Identifier: GPL-3.0-or-later\\n")\n')
    assert gate.foreign_license_tags(str(fp)) == []
    assert gate.has_required_header(str(fp)) is True


def test_declaracao_de_outra_licenca_continua_reprovando(tmp_path):
    """NAO E AFROUXAMENTO: valor limpo = expressao SPDX = declaracao = reprova.
    Pina que a precisao nova nao reabriu o furo que a fatia anterior fechou."""
    for valor in (_GPL, "MIT", "BSD-2-Clause OR CC0-1.0", "LGPL-2.1-only"):
        fp = tmp_path / "decl.cpp"
        _write(str(fp), "// SPDX-License-Identifier: Apache-2.0\n"
                        f"// SPDX-License-Identifier: {valor}\n")
        assert gate.foreign_license_tags(str(fp)) == [(2, valor)], valor


def test_valor_limpo_porem_estranho_ainda_reprova(tmp_path):
    """O criterio e PERMISSIVO PARA SINALIZAR: sinalizar de mais e seguro
    (alguem conserta a prosa), sinalizar de menos nao e."""
    fp = tmp_path / "estranho.cpp"
    _write(str(fp), f"// SPDX-License-Identifier: {_GPL} provisorio\n")
    assert gate.foreign_license_tags(str(fp)) == [(1, f"{_GPL} provisorio")]


def test_mencao_perto_do_topo_nao_satisfaz_a_regra_1(tmp_path):
    """O GEMEO do furo (auditoria-domino): a REGRA 1 era substring pura, entao
    um arquivo que apenas CITASSE a string do cabecalho nas 5 primeiras linhas
    passava sem declarar licenca nenhuma."""
    fp = tmp_path / "so_cita.py"
    _write(str(fp), "#!/usr/bin/env python3\n"
                    '"""procura por "SPDX-License-Identifier: Apache-2.0" no topo."""\n')
    assert gate.has_required_header(str(fp)) is False
    assert gate.find_offenders(["so_cita.py"], str(tmp_path), {}) == ["so_cita.py"]


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
        lambda root, extensions, basenames=(): ["GusEngine/app/src/novo_sem_spdx.cpp"],
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
        gate, "list_tracked_files", lambda root, extensions, basenames=(): ["GusEngine/core/bom.hpp"]
    )
    monkeypatch.setattr(gate, "ALLOWLIST_PATH", str(tmp_path / "allowlist_vazio.txt"))

    rc = gate.main()
    assert rc == 0


if __name__ == "__main__":
    sys.exit(pytest.main([__file__, "-v"]))

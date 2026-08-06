#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
"""test_ci_roda_os_gates.py - o CI executa o estagio GATE de verdade.

O ACHADO QUE ORIGINOU ESTE TESTE (CI-RODA-OS-GATES, 2026-08-06): o
`.github/workflows/ci.yml` mencionava `tools/check.sh` QUATRO vezes, e as quatro
eram COMENTARIO - `grep -n "check.sh" ci.yml | grep -v "#"` devolvia ZERO linha.
O CI rodava 3 checagens (arquitetura por grep INLINE, i18n_parity, spdx) e os
outros 14 gates do estagio GATE so existiam na maquina local, via git hook. Um PR
podia entrar com sdl-ratchet, production-scope, fetchcontent-manifest,
callback-dtor-order, callback-dtor-disarm, decode-image-file-zero e
ui-log-key-only VERMELHOS sem nada barrar.

E o irmao exato do defeito que o test_check_sh_gates_ligados.py cobre: la o gate
rodava e o `rc` do check.sh ignorava; aqui o gate existe e o CI nunca o chama.
"Gate correto e desligado e gate que nao existe" - e ha mais de um jeito de
desligar um gate:
  (1) nao entrar na composicao do rc  -> test_check_sh_gates_ligados.py
  (2) o CI nao invocar o estagio      -> ESTE arquivo
  (3) o evento nunca chegar ao job    -> ESTE arquivo (filtro `paths:`)

O metodo e ENUMERAR, nao procurar: o conjunto de gates e fechado por construcao
(as variaveis GATE_* do check.sh), e o conjunto de entradas que eles leem fora de
GusEngine/ tambem. Busca dirigida encontra o que voce suspeita; quem esquece de
ligar um gate e, por definicao, quem nao suspeita daquele gate.

Uso: `python3 -m pytest tools/tests/test_ci_roda_os_gates.py -q`
"""

import os
import re
import sys

import pytest

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
CHECK_SH = os.path.join(ROOT, "tools", "check.sh")
CI_YML = os.path.join(ROOT, ".github", "workflows", "ci.yml")

# Modos de invocacao que cobrem o estagio GATE INTEIRO. Um modo novo que rode
# so um pedaco (ex.: `--only-arch`) tem de entrar aqui CONSCIENTEMENTE, no mesmo
# commit em que nascer - senao este teste reprova, que e o comportamento certo.
MODOS_QUE_COBREM_O_GATE = {"", "--gates-only"}

# Entradas que os gates leem FORA de GusEngine/, enumeradas dos proprios scripts
# (i18n_parity.py e ui_log_key_only.py -> resources/translations;
# spdx_header_required.py -> tools/spdx_allowlist.txt; os 14 scripts de gate e o
# tools/tests/ -> tools/). Sem estes prefixos no filtro `paths:`, um PR que mexa
# SO nelas nao dispara CI nenhum - inclusive um PR que AFROUXE um gate.
PREFIXOS_DE_ENTRADA_DOS_GATES = ("tools/", "resources/translations/")

_ASSIGN_RE = re.compile(r"^\s*(GATE_[A-Z0-9_]+)=", re.MULTILINE)


def _ler(caminho: str) -> str:
    with open(caminho, encoding="utf-8") as f:
        return f.read()


def _linhas_efetivas(texto: str):
    """Linhas do YAML SEM comentario: e a distincao exata que o achado explorou.

    Nao e um parser de YAML - e a mesma leitura que um `grep -v '#'` faz, so que
    tolerando comentario no fim da linha. Mencionar `check.sh` em comentario
    (havia 4) nao executa nada.
    """
    for bruta in texto.splitlines():
        sem_comentario = bruta.split("#", 1)[0]
        if sem_comentario.strip():
            yield sem_comentario


def _invocacoes_do_check_sh(texto_ci: str):
    """Toda invocacao efetiva de tools/check.sh no YAML, com os argumentos."""
    achadas = []
    for linha in _linhas_efetivas(texto_ci):
        m = re.search(r"(?:bash\s+|\./|\s|^)tools/check\.sh([^\n|&;]*)", linha)
        if m:
            achadas.append(m.group(1).strip())
    return achadas


# --------------------------------------------------------------------------
# (1) O CI chama o check.sh de verdade - nao em comentario.
# --------------------------------------------------------------------------
def test_ci_invoca_o_check_sh_fora_de_comentario():
    invocacoes = _invocacoes_do_check_sh(_ler(CI_YML))
    assert invocacoes, (
        "o .github/workflows/ci.yml NAO executa tools/check.sh em nenhuma linha "
        "efetiva (so em comentario, ou nem isso). Foi exatamente este o estado "
        "encontrado em 2026-08-06: 4 mencoes, 4 comentarios, 14 gates sem "
        "cobertura no CI. Acrescente um step `run: bash tools/check.sh "
        "--gates-only`."
    )


def test_o_modo_invocado_cobre_o_estagio_gate_inteiro():
    for args in _invocacoes_do_check_sh(_ler(CI_YML)):
        assert args in MODOS_QUE_COBREM_O_GATE, (
            f"o CI invoca `tools/check.sh {args}`, um modo que NAO consta da "
            f"lista de modos que cobrem o estagio GATE inteiro "
            f"({sorted(MODOS_QUE_COBREM_O_GATE)}). Se o modo e novo e legitimo, "
            "acrescente-o a MODOS_QUE_COBREM_O_GATE no mesmo commit - a lista "
            "existe pra obrigar a decisao a ser consciente."
        )


# --------------------------------------------------------------------------
# (2) Anti-duplicacao: o CI nao reimplementa gate que o check.sh ja compoe.
#     Duas implementacoes da mesma regra divergem, e a copia do YAML nao tem
#     dono - foi o caso do gate de arquitetura, que vivia como grep inline aqui
#     E como GATE(arch) no check.sh.
# --------------------------------------------------------------------------
def _gates_python_do_check_sh(texto_check: str):
    return set(re.findall(r'python3 "\$ROOT/tools/([a-z0-9_]+)\.py"', texto_check))


def test_sentinela_de_escopo_o_parser_enxerga_os_gates():
    """Se o parser quebrar, os testes abaixo passariam varrendo o vazio."""
    gates = _gates_python_do_check_sh(_ler(CHECK_SH))
    assert len(gates) >= 10, (
        f"so achei {len(gates)} gate(s) Python no tools/check.sh - o parser deve "
        "ter quebrado. Sentinela de escopo: nao passo em silencio."
    )
    variaveis = set(_ASSIGN_RE.findall(_ler(CHECK_SH)))
    assert len(variaveis) >= 10, (
        f"so achei {len(variaveis)} variavel(is) GATE_* no tools/check.sh - "
        "parser quebrado."
    )


def test_ci_nao_reimplementa_os_gates_do_check_sh():
    texto_ci = _ler(CI_YML)
    gates = _gates_python_do_check_sh(_ler(CHECK_SH))
    efetivo = "\n".join(_linhas_efetivas(texto_ci))
    duplicados = sorted(g for g in gates if f"tools/{g}.py" in efetivo)
    assert not duplicados, (
        "o ci.yml invoca diretamente gate(s) que o tools/check.sh ja compoe: "
        f"{duplicados}. Duas implementacoes/invocacoes da mesma regra divergem "
        "com o tempo. Deixe o step do check.sh --gates-only ser o unico dono."
    )


def test_ci_nao_tem_grep_de_arquitetura_inline():
    """O GATE(arch) do check.sh e a implementacao; o grep no YAML era a copia."""
    efetivo = "\n".join(_linhas_efetivas(_ler(CI_YML)))
    assert not re.search(r"grep[^\n]*GusEngine/core", efetivo), (
        "grep de arquitetura INLINE no ci.yml sobre GusEngine/core - e a copia "
        "que divergiu do GATE(arch) do tools/check.sh. Use o check.sh."
    )


# --------------------------------------------------------------------------
# (3) O evento chega ao job: o filtro `paths:` cobre as entradas dos gates.
# --------------------------------------------------------------------------
def _blocos_de_paths(texto_ci: str):
    blocos = re.findall(r"^\s+paths:\n((?:\s+- '[^']*'\n)+)", texto_ci, re.MULTILINE)
    return [set(re.findall(r"- '([^']*)'", b)) for b in blocos]


def test_paths_do_ci_cobrem_as_entradas_dos_gates():
    blocos = _blocos_de_paths(_ler(CI_YML))
    assert blocos, (
        "nao achei nenhum bloco `paths:` no ci.yml. Se o filtro foi removido de "
        "proposito (CI em todo push), ESTE teste tem de ser atualizado no mesmo "
        "commit - sem o bloco ele nao pode dizer OK."
    )
    for i, padroes in enumerate(blocos):
        for prefixo in PREFIXOS_DE_ENTRADA_DOS_GATES:
            coberto = any(p.startswith(prefixo) for p in padroes)
            assert coberto, (
                f"o bloco `paths:` #{i + 1} do ci.yml NAO cobre '{prefixo}**', "
                f"que e entrada lida pelos gates (padroes declarados: "
                f"{sorted(padroes)}). Sem isso, um PR que mexa SO nessa pasta - "
                "inclusive AFROUXANDO um gate ou quebrando a paridade i18n - nao "
                "dispara CI nenhum."
            )


if __name__ == "__main__":
    sys.exit(pytest.main([__file__, "-v"]))

#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
"""test_check_sh_gates_ligados.py - todo gate do check.sh esta LIGADO no rc.

REGRA (c) DA CASA: um gate so existe se (a) mede de verdade, (b) nasce verde na
arvore corrigida, e (c) esta LIGADO na condicao que faz o `rc` final virar 1.
"Gate correto e desligado e gate que nao existe".

POR QUE ESTE TESTE EXISTE: a regra (c) ja tinha sido esquecida no repo antes, e
foi esquecida DE NOVO na propria fatia GATES-HARDEN (2026-08-06) - o
GATE(tools-tests) foi escrito, invocado e teve o `rc` capturado em
`GATE_TOOLS_TESTS`, mas a variavel nao entrou na expressao de composicao. O
gate rodava, imprimia falha, e o `rc` do check.sh continuava 0. So apareceu no
mutation testing.

O conserto de PROCEDIMENTO nao e "prestar mais atencao", e ENUMERAR: o espaco
(as variaveis GATE_* do arquivo) e pequeno e fechado por construcao, entao se
enumera ele inteiro em vez de procurar dentro dele. Busca dirigida encontra o
que voce suspeita; enumeracao encontra o que voce nao sabia que devia
suspeitar - e quem esquece de ligar um gate e, por definicao, quem nao suspeita
daquele gate.

Uso: `python3 -m pytest tools/tests/test_check_sh_gates_ligados.py -q`
"""

import os
import re
import sys

import pytest

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
CHECK_SH = os.path.join(ROOT, "tools", "check.sh")

# Atribuicao de variavel de gate: `GATE_X=$?`, `GATE_X=0`, `    GATE_X=1`.
# `GATE=` (o acumulador) fica de fora - e o alvo, nao um gate.
_ASSIGN_RE = re.compile(r"^\s*(GATE_[A-Z0-9_]+)=", re.MULTILINE)


def _texto() -> str:
    with open(CHECK_SH, encoding="utf-8") as f:
        return f.read()


def _bloco_de_composicao(texto: str) -> str:
    """O trecho entre `GATE=0` e `echo "GATE=$GATE"` - a expressao que decide."""
    ini = texto.find("\nGATE=0\n")
    fim = texto.find('echo "GATE=$GATE"', ini if ini >= 0 else 0)
    assert ini >= 0 and fim > ini, (
        "nao achei o bloco de composicao do GATE em tools/check.sh. Se o formato "
        "mudou, ESTE teste tem de ser atualizado no mesmo commit - sem o bloco, "
        "ele nao pode dizer OK (seria um verificador varrendo o vazio)."
    )
    return texto[ini:fim]


def test_todo_gate_declarado_entra_na_composicao():
    """A prova: cada GATE_* atribuido no arquivo aparece na expressao final."""
    texto = _texto()
    declarados = sorted(set(_ASSIGN_RE.findall(texto)))
    assert len(declarados) >= 10, (
        f"so achei {len(declarados)} variavel(is) GATE_* - o parser deve ter "
        "quebrado; sentinela de escopo, nao passa em silencio."
    )

    bloco = _bloco_de_composicao(texto)
    desligados = [v for v in declarados if f'"${v}"' not in bloco]
    assert not desligados, (
        "gate(s) DESLIGADO(s) em tools/check.sh - rodam, podem imprimir FALHA, e "
        f"o rc do check.sh continua 0: {desligados}. Acrescente "
        '`&& [ "$VAR" = "0" ] \\` ao bloco `GATE=0 ... echo "GATE=$GATE"`.'
    )


def test_toda_variavel_da_composicao_e_realmente_atribuida():
    """O espelho: variavel na expressao sem ninguem atribuir vale sempre "" != 0
    sob `set -u`... ou pior, silenciosamente vazia. Pega rename pela metade."""
    texto = _texto()
    bloco = _bloco_de_composicao(texto)
    usadas = sorted(set(re.findall(r'"\$(GATE_[A-Z0-9_]+)"', bloco)))
    declaradas = set(_ASSIGN_RE.findall(texto))
    orfas = [v for v in usadas if v not in declaradas]
    assert not orfas, f"variavel usada na composicao e nunca atribuida: {orfas}"


def test_os_gates_desta_fatia_estao_ligados():
    """Ancora explicita nos dois gates que a fatia GATES-HARDEN acrescentou -
    o segundo e justamente o que nasceu desligado."""
    bloco = _bloco_de_composicao(_texto())
    for var in ("GATE_PRODUCTION_SCOPE", "GATE_TOOLS_TESTS"):
        assert f'"${var}"' in bloco, f"{var} fora da composicao do rc"


if __name__ == "__main__":
    sys.exit(pytest.main([__file__, "-v"]))

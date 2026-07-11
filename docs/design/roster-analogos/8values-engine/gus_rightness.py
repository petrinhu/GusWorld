#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
#
# gus_rightness.py -- GusWorld roster political scalar, camada v2 (ponderada)
# =============================================================================
#
# Contexto: ferramenta de design (nao codigo do jogo) que fica POR CIMA do
# fork 8values desta mesma pasta (`8values_engine.py`, MIT, ver
# `ATTRIBUTION.md` / `LICENSE_8VALUES_ORIGINAL.txt`). Usada para classificar
# o espectro politico das 21 figuras historicas do roster GusWorld
# (`docs/design/roster-analogos/`).
#
# RESTRICAO INVIOLAVEL: este arquivo NAO modifica `8values_engine.py` (fork
# fiel do 8values original -- a fidelidade numerica e parte da atribuicao ao
# autor original). Este script IMPORTA o modulo original (sem side effects
# no import: `8values_engine.py` so roda `main()` sob
# `if __name__ == "__main__"`) e REUTILIZA a funcao `score_answers()` dele
# para os 4 eixos -- zero reimplementacao da matematica de scoring, zero
# divergencia numerica possivel.
#
# DECISAO DO CRIADOR (via AskUserQuestion, 2026-07-11):
#   O escalar de "direita" usado ate aqui no roster (RIGHTNESS_V1, ja
#   calculado dentro do `8values_engine.py`) e so o eixo Economico
#   (Markets/Equality). O criador decidiu adicionar uma camada v2 PONDERADA,
#   por CIMA do fork (sem alterar o fork), que combina os 4 eixos do 8values
#   com pesos explicitos, derivados da ordem de importancia dos 8 valores no
#   lore do GusWorld:
#
#       Markets > Liberty > Authority > Equality > Tradition > Progress >
#       Nation > Peace
#
#   Polos de "direita" no GusWorld: Markets (economico), Liberty (civil),
#   Tradition (societal), Nation (diplomatico).
#
#   O eixo Economico pesa 2x o eixo Civil de proposito: e o UNICO eixo que
#   separa liberdade PROPRIETARIA (mercado, direita) de liberdade
#   ANARQUISTA/coletivista (o eixo Civil sozinho, sem o economico como
#   desempate, recompensaria "liberdade" em abstrato -- inclusive a de tipo
#   anarco-coletivista -- como se fosse direita, o que e um erro de
#   classificacao no eixo axiologico do GusWorld).
#
#   RIGHTNESS_V1 continua exposto (nao e substituido nem descontinuado) para
#   retrocompatibilidade com qualquer classificacao ja feita no roster;
#   RIGHTNESS_V2 e um refinamento aditivo, nao uma correcao do V1.
#
# O QUE ESTE ARQUIVO ADICIONA (alem do fork 8values):
#   - Import de `8values_engine.py` via `importlib.util` (mesma pasta), sem
#     poluir `sys.path` e sem executar `main()` do modulo original.
#   - RIGHTNESS_V1 = `result.rightness`, repassado tal e qual do engine
#     original (nenhum recalculo).
#   - RIGHTNESS_V2 = combinacao ponderada dos 4 eixos (formula na docstring
#     do modulo, pesos como constantes nomeadas abaixo).
#   - Relatorio legivel (4 eixos com os dois polos + rotulo do engine
#     original, ideologia mais proxima) + duas linhas parseaveis
#     `RIGHTNESS_V1=` / `RIGHTNESS_V2=`.
#
# Licenca deste arquivo: MIT (mesmos termos de `8values_engine.py`).

"""gus_rightness.py -- camada v2 (ponderada) do escalar de direita/esquerda
do roster GusWorld, por cima do fork 8values (`8values_engine.py`, mesma
pasta). NAO substitui o fork, NAO reimplementa a matematica de scoring dele
(importa e reusa `score_answers()` diretamente).

USO
---
    python3 gus_rightness.py <answers.json>

ou via stdin (mesmo padrao do `8values_engine.py`):

    cat answers.json | python3 gus_rightness.py

`<answers.json>` e EXATAMENTE o mesmo formato de input do
`8values_engine.py`: um array JSON com 70 tokens `sa`/`a`/`n`/`d`/`sd`
(Strongly Agree / Agree / Neutral / Disagree / Strongly Disagree), na ordem
1-70 de `PERGUNTAS.md`. Tambem aceito: `{"answers": [...]}`.

FORMULA
-------
RIGHTNESS_V1 (repassado do engine original, SEM mudanca nenhuma):

    RIGHTNESS_V1 = econ_markets_pct / 100

RIGHTNESS_V2 (nova, camada GusWorld v2 -- ver o bloco de decisao no topo do
arquivo para a justificativa completa dos pesos):

    RIGHTNESS_V2 = WEIGHT_ECONOMIC   * (econ_markets_pct   / 100)
                 + WEIGHT_CIVIL      * (govt_liberty_pct    / 100)
                 + WEIGHT_SOCIETAL   * (scty_tradition_pct  / 100)
                 + WEIGHT_DIPLOMATIC * (dipl_nation_pct     / 100)

Pesos (somam 1.0): WEIGHT_ECONOMIC=0.50, WEIGHT_CIVIL=0.25,
WEIGHT_SOCIETAL=0.15, WEIGHT_DIPLOMATIC=0.10.

Todos os quatro `*_pct` de entrada da formula acima vem PRONTOS do
`EightValuesResult` retornado por `8values_engine.score_answers()` -- ja
normalizados 0-100 e arredondados a 1 casa pelo engine original. Este
modulo nao refaz esse calculo, so combina os resultados prontos.

OUTPUT
------
Relatorio legivel (4 eixos com os dois polos e o rotulo do engine original,
ideologia mais proxima das 52 do 8values) e, ao final, duas linhas
parseaveis por scripts:

    RIGHTNESS_V1=0.NNN
    RIGHTNESS_V2=0.NNN
"""

from __future__ import annotations

import importlib.util
import json
import sys
from pathlib import Path
from types import ModuleType

# ---------------------------------------------------------------------------
# Pesos da camada v2 (constantes nomeadas -- decisao do criador 2026-07-11).
# Ver o bloco de decisao no cabecalho do arquivo para a justificativa
# completa. Resumo: ordem de importancia dos 8 valores no lore GusWorld e
#   Markets > Liberty > Authority > Equality > Tradition > Progress >
#   Nation > Peace
# Polos de "direita": Markets/Liberty/Tradition/Nation. Economico pesa 2x
# Civil de proposito -- e o unico eixo que separa liberdade PROPRIETARIA de
# liberdade ANARCO-COLETIVISTA (sem esse desempate o eixo Civil sozinho
# recompensaria liberdade em abstrato, mesmo a de tipo anarquista/coletivo,
# como se fosse direita).
# ---------------------------------------------------------------------------
WEIGHT_ECONOMIC: float = 0.50
WEIGHT_CIVIL: float = 0.25
WEIGHT_SOCIETAL: float = 0.15
WEIGHT_DIPLOMATIC: float = 0.10

_WEIGHT_SUM = WEIGHT_ECONOMIC + WEIGHT_CIVIL + WEIGHT_SOCIETAL + WEIGHT_DIPLOMATIC
assert abs(_WEIGHT_SUM - 1.0) < 1e-9, (
    f"pesos da camada v2 devem somar 1.0, somaram {_WEIGHT_SUM!r}"
)

_ENGINE_FILENAME = "8values_engine.py"


def _load_8values_engine() -> ModuleType:
    """Carrega `8values_engine.py` (mesma pasta) via `importlib.util`.

    Usa `spec_from_file_location` em vez de `import 8values_engine` porque
    o nome do arquivo comeca com digito (nao e um identificador Python
    valido para a sintaxe `import`). Isto NAO copia nem reescreve nenhuma
    linha do modulo original -- carrega o arquivo tal como esta em disco.

    Seguro porque o modulo original nao tem side effects em import: todo
    codigo que roda (ler stdin/argv, imprimir relatorio) fica atras de
    `if __name__ == "__main__":`.

    Registra o modulo em `sys.modules` ANTES de `exec_module` (pratica
    padrao do `importlib`, nao um hack): sem isso, `@dataclass` em
    `EightValuesResult` quebra em runtimes recentes (Python 3.14 resolve
    `cls.__module__` via `sys.modules.get(...)` internamente, e um modulo
    carregado so com `module_from_spec` + `exec_module`, sem passar por
    `sys.modules`, nao existe la ainda nesse momento).
    """
    engine_path = Path(__file__).resolve().parent / _ENGINE_FILENAME
    if not engine_path.is_file():
        raise FileNotFoundError(
            f"nao encontrei {_ENGINE_FILENAME} ao lado de {__file__} "
            "(gus_rightness.py exige o fork 8values na mesma pasta, "
            "e NAO reimplementa a matematica de scoring dele)"
        )
    module_name = "gusworld_8values_engine_v1"
    spec = importlib.util.spec_from_file_location(module_name, engine_path)
    if spec is None or spec.loader is None:
        raise ImportError(f"nao consegui montar o import spec de {engine_path}")
    module = importlib.util.module_from_spec(spec)
    sys.modules[module_name] = module
    try:
        spec.loader.exec_module(module)
    except BaseException:
        sys.modules.pop(module_name, None)
        raise
    return module


_engine = _load_8values_engine()


def _load_answers_from_text(text: str) -> list[str]:
    """Decodifica o mesmo formato de input aceito por `8values_engine.py`
    (lista JSON de 70 tokens, ou objeto `{"answers": [...]}`).

    Duplicada aqui (em vez de importada de `8values_engine._load_answers_
    from_text`, que e privada/underscore) de proposito: e SO decodificacao
    de formato JSON, ZERO matematica de scoring -- reimplementar isto nao
    cria risco de divergencia numerica. A funcao que de fato pontua
    (`score_answers`, com toda a formula/pesos/tabela de ideologias) e
    sempre importada do engine original, nunca reimplementada.
    """
    data = json.loads(text)
    if isinstance(data, dict):
        if "answers" not in data:
            raise ValueError('JSON object input must have an "answers" key')
        data = data["answers"]
    if not isinstance(data, list):
        raise ValueError(
            "input JSON must be a list of 70 answer tokens, or an object "
            'with an "answers" list'
        )
    return [str(x) for x in data]


def compute_rightness_v2(result: _engine.EightValuesResult) -> float:
    """RIGHTNESS_V2: combinacao ponderada dos 4 eixos ja calculados pelo
    engine original (`result` vem de `_engine.score_answers()`). Ver a
    docstring do modulo para a formula completa e o bloco de decisao no
    cabecalho do arquivo para a justificativa dos pesos.
    """
    v2 = (
        WEIGHT_ECONOMIC * (result.econ_markets_pct / 100.0)
        + WEIGHT_CIVIL * (result.govt_liberty_pct / 100.0)
        + WEIGHT_SOCIETAL * (result.scty_tradition_pct / 100.0)
        + WEIGHT_DIPLOMATIC * (result.dipl_nation_pct / 100.0)
    )
    return round(v2, 3)


def format_report(result: _engine.EightValuesResult, rightness_v2: float) -> str:
    lines = [
        "=== gus_rightness.py (camada v2 ponderada sobre o fork 8values) -- Report ===",
        "",
        f"Economic   (Equality <-> Markets):    "
        f"Equality {result.econ_equality_pct:5.1f}%  |  Markets   {result.econ_markets_pct:5.1f}%   "
        f"[{result.econ_label}]",
        f"Diplomatic (Peace    <-> Nation):     "
        f"Peace    {result.dipl_peace_pct:5.1f}%  |  Nation    {result.dipl_nation_pct:5.1f}%   "
        f"[{result.dipl_label}]",
        f"Civil/Govt (Liberty  <-> Authority):  "
        f"Liberty  {result.govt_liberty_pct:5.1f}%  |  Authority {result.govt_authority_pct:5.1f}%   "
        f"[{result.govt_label}]",
        f"Societal   (Progress <-> Tradition):  "
        f"Progress {result.scty_progress_pct:5.1f}%  |  Tradition {result.scty_tradition_pct:5.1f}%   "
        f"[{result.scty_label}]",
        "",
        f"Closest ideology (8values 52-entry table): {result.ideology}",
        "",
        f"RIGHTNESS_V1 (so eixo Economico -- repassado sem mudanca do "
        f"8values_engine.py): {result.rightness:.3f}",
        f"RIGHTNESS_V2 (ponderado -- {WEIGHT_ECONOMIC:.2f}*Markets + "
        f"{WEIGHT_CIVIL:.2f}*Liberty + {WEIGHT_SOCIETAL:.2f}*Tradition + "
        f"{WEIGHT_DIPLOMATIC:.2f}*Nation): {rightness_v2:.3f}",
        "",
        f"RIGHTNESS_V1={result.rightness:.3f}",
        f"RIGHTNESS_V2={rightness_v2:.3f}",
    ]
    return "\n".join(lines)


def main(argv: list[str]) -> int:
    if len(argv) > 1:
        path = argv[1]
        try:
            with open(path, "r", encoding="utf-8") as fh:
                text = fh.read()
        except OSError as exc:
            # Nota: 8values_engine.py original NAO trata este caso (deixa o
            # traceback subir); aqui blindamos porque este script tambem
            # roda como CLI standalone e o erro de "arquivo nao encontrado"
            # e comum ao testar varias figuras do roster em sequencia.
            print(f"error: nao consegui ler {path!r}: {exc}", file=sys.stderr)
            return 1
    else:
        text = sys.stdin.read()
        if not text.strip():
            print(
                "usage: gus_rightness.py <answers.json>   "
                "(ou pipe JSON via stdin)",
                file=sys.stderr,
            )
            return 2

    try:
        answers = _load_answers_from_text(text)
        result = _engine.score_answers(answers)
    except (ValueError, json.JSONDecodeError) as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 1

    rightness_v2 = compute_rightness_v2(result)
    print(format_report(result, rightness_v2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))

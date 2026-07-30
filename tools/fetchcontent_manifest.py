#!/usr/bin/env python3
"""fetchcontent_manifest.py - GATE(fetchcontent-manifest) do tools/check.sh.

LISTA FECHADA (nao ratchet, nao zero-tolerancia de token - de NOME): o unico
arquivo de manifesto e `GusEngine/CMakeLists.txt` (de proposito - os
CMakeLists.txt de `app/tools/*` sao standalone, mesma condicao estrutural
que ja exclui `app/tools/` do GATE(sdl-ratchet): eles reusam arvores JA
POPULADAS por `SOURCE_DIR` apontando pro `_deps` do build principal, nunca
fazem clone git novo, e nao linkam em `gusengine_app` - ver
tools/sdl_layer_ratchet.py). Medido em 2026-07-30
(docs/tech/plano-migracao-total.md secao 2.2): exatamente 4 nomes -
SDL3, RmlUi, glintfx, Catch2. Reprova se aparecer QUALQUER nome fora
desse conjunto.

Por que existe: uma dependencia pode entrar TRANSITIVAMENTE sem ninguem
decidir (o proprio miniaudio fez isso uma vez - saiu do nosso
`third_party/`, voltou por dentro do glintfx via GLINTFX_MODULE_AUDIO). Este
gate nao impede dependencia transitiva DENTRO de uma lib ja aprovada (isso e
decisao de quem mantem a lib); impede que ALGUEM aqui adicione um
`FetchContent_Declare` novo no NOSSO CMakeLists.txt sem essa decisao virar
uma linha no `ALLOWED` abaixo - a decisao passa a ser EXPLICITA por
construcao.

Parser: CMake nao tem `/* */`, so comentario de linha com `#` (ignorando a
sintaxe rara de bracket-comment `#[[ ]]`, ausente neste arquivo - conferido).
O strip abaixo e deliberadamente mais simples que strip_comments() de
tools/sdl_layer_ratchet.py (que e para C++): trunca cada LINHA no primeiro
`#` que nao esta dentro de uma string entre aspas duplas. Sem isso, os
proprios comentarios do CMakeLists.txt (que DOCUMENTAM
"FetchContent_Declare(RmlUi ...) dele/DELE", explicando a idempotencia entre
o nosso Declare e o do glintfx) casariam com a regex e o gate contaria RmlUi
2x - inofensivo aqui (RmlUi ja esta na lista), mas o mesmo padrao com um nome
comentado FORA da lista daria falso-positivo.
"""

import os
import re
import sys

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(SCRIPT_DIR)
MANIFEST = os.path.join(ROOT, "GusEngine", "CMakeLists.txt")

# TETO EXPLICITO. Mudar exige decisao registrada (commit citando o motivo,
# e idealmente o ADR/plano que aprovou a dependencia nova) - nunca so
# "adicionar aqui pra o gate passar".
ALLOWED = {"SDL3", "RmlUi", "glintfx", "Catch2"}

_DECLARE_RE = re.compile(r"FetchContent_Declare\s*\(\s*([A-Za-z0-9_]+)")


def strip_cmake_line_comments(text: str) -> str:
    """Trunca cada linha no primeiro `#` fora de string entre aspas duplas."""
    out_lines = []
    for line in text.splitlines():
        in_string = False
        cut = len(line)
        i = 0
        n = len(line)
        while i < n:
            c = line[i]
            if c == "\\" and in_string and i + 1 < n:
                i += 2
                continue
            if c == '"':
                in_string = not in_string
            elif c == "#" and not in_string:
                cut = i
                break
            i += 1
        out_lines.append(line[:cut])
    return "\n".join(out_lines)


def find_declared_names():
    if not os.path.isfile(MANIFEST):
        print(f"GATE(fetchcontent-manifest): FALHA - manifesto nao encontrado em "
              f"{os.path.relpath(MANIFEST, ROOT)}.")
        return None
    with open(MANIFEST, encoding="utf-8", errors="replace") as f:
        stripped = strip_cmake_line_comments(f.read())
    # dict preserva 1a ocorrencia (linha) de cada nome, pra mensagem de erro.
    names = {}
    for m in _DECLARE_RE.finditer(stripped):
        name = m.group(1)
        line_no = stripped.count("\n", 0, m.start()) + 1
        names.setdefault(name, line_no)
    return names


def main() -> int:
    names = find_declared_names()
    if names is None:
        return 1

    unexpected = sorted(n for n in names if n not in ALLOWED)
    missing = sorted(ALLOWED - set(names))

    if unexpected:
        print(f"GATE(fetchcontent-manifest): FALHA - {len(unexpected)} "
              f"FetchContent_Declare fora da lista fechada {sorted(ALLOWED)}.")
        for name in unexpected:
            print(f"    - {name} ({os.path.relpath(MANIFEST, ROOT)}:{names[name]})")
        print("  Dependencia nova entrando SEM decisao explicita registrada. "
              "Caminho certo: PRIMEIRO decidir (ADR/plano, dono da fronteira - "
              "docs/tech/glintfx-boundary.md se for algo que caberia no glintfx), "
              "SO ENTAO adicionar o nome a ALLOWED em tools/fetchcontent_manifest.py "
              "no MESMO commit que introduz o FetchContent_Declare.")
        return 1

    if missing:
        # Nao e FALHA por si so (a lista fechada e um teto, nao um piso
        # obrigatorio de presenca) - mas e sinal forte de manifesto
        # desatualizado (algo foi removido do CMakeLists.txt e ninguem
        # encolheu ALLOWED), entao avisa sem reprovar.
        print(f"GATE(fetchcontent-manifest): OK, mas {missing} esta em ALLOWED e "
              "NAO aparece mais no manifesto - se a remocao foi de proposito, "
              "encolha ALLOWED em tools/fetchcontent_manifest.py no mesmo commit.")
        return 0

    print(f"GATE(fetchcontent-manifest): OK ({len(names)} FetchContent_Declare, "
          f"todos na lista fechada {sorted(ALLOWED)}).")
    return 0


if __name__ == "__main__":
    sys.exit(main())

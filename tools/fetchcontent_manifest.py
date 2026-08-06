#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
"""fetchcontent_manifest.py - GATE(fetchcontent-manifest) do tools/check.sh.

LISTA FECHADA (nao ratchet, nao zero-tolerancia de token - de NOME): reprova
se aparecer QUALQUER nome de `FetchContent_Declare` fora do conjunto
{SDL3, RmlUi, glintfx, Catch2}, medido em 2026-07-30
(docs/tech/plano-migracao-total.md secao 2.2).

ESCOPO = O GRAFO DE `add_subdirectory`, NAO UM ARQUIVO SO (GATES-HARDEN,
2026-08-06). Ate esta fatia o gate lia EXATAMENTE `GusEngine/CMakeLists.txt`.
Mas `core/`, `domain/`, `platform/`, `app/`, `tests/` e
`third_party/monocypher/` sao `add_subdirectory` DELE (linhas 755 e 779-785),
logo os CMakeLists.txt deles fazem parte do MESMO build - e um
`FetchContent_Declare` em qualquer um passava VERDE (medido pela auditoria: F2
e F3 do relatorio). Nao havia divida no dia da medicao; o furo era
prospectivo, e o custo de fecha-lo e este walk.

O walk parte do manifesto raiz e segue cada `add_subdirectory(<dir>)` REAL
(fora de comentario) que exista no disco, recursivamente. Duas propriedades
que uma lista escrita a mao nao teria:
  - subdiretorio NOVO entra sozinho, sem ninguem lembrar de acrescentar aqui
    (era exatamente esse o modo de falha);
  - `app/tools/*` fica de fora POR CONSTRUCAO, nao por exclusao por nome:
    `app/CMakeLists.txt` so faz `add_subdirectory(tests)`, entao os projetos
    standalone de `app/tools/` sao INALCANCAVEIS pelo walk. Continua sendo a
    exclusao correta e documentada (eles reusam arvores JA POPULADAS por
    `SOURCE_DIR` apontando pro `_deps` do build principal, nunca clonam de
    novo, e nao linkam em `gusengine_app` - ver tools/sdl_layer_ratchet.py) -
    a diferenca e que agora ela decorre da estrutura, em vez de depender de
    alguem manter uma lista de exclusao.

FORA DO WALK, de proposito: `include(<arquivo>.cmake)`. Medido no mesmo dia -
o unico .cmake rastreado e `cmake/toolchains/mingw-w64.cmake`, que nao e
`include`-ado por CMakeLists nenhum (entra por `CMAKE_TOOLCHAIN_FILE` do
preset) e tem zero FetchContent. Seguir `include()` exigiria expandir
variavel de caminho (`${CMAKE_CURRENT_LIST_DIR}` etc.), o que este parser
deliberadamente nao faz. Se um dia nascer um `.cmake` proprio com
dependencia, ele precisa entrar no walk no MESMO commit.

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
# RAIZ do walk (ver "ESCOPO" no docstring) - nao mais o unico arquivo varrido.
MANIFEST = os.path.join(ROOT, "GusEngine", "CMakeLists.txt")

# TETO EXPLICITO. Mudar exige decisao registrada (commit citando o motivo,
# e idealmente o ADR/plano que aprovou a dependencia nova) - nunca so
# "adicionar aqui pra o gate passar".
ALLOWED = {"SDL3", "RmlUi", "glintfx", "Catch2"}

_DECLARE_RE = re.compile(r"FetchContent_Declare\s*\(\s*([A-Za-z0-9_]+)")
_SUBDIR_RE = re.compile(r"add_subdirectory\s*\(\s*([^\s)]+)")


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


def collect_build_cmakelists(root_manifest: str | None = None) -> list[str]:
    """Enumera os CMakeLists.txt do build REAL seguindo `add_subdirectory`.

    Comeca no manifesto raiz e desce recursivamente. Ignora `add_subdirectory`
    em comentario (o strip acima roda ANTES) e caminho que nao existe no disco
    ou que use variavel `${...}` (nao expandimos variavel - um caminho assim
    precisa entrar no walk a mao, no commit que o criar). Ordem = descoberta,
    determinista."""
    # `= None` e nao `= MANIFEST`: default de funcao e avaliado UMA VEZ, no
    # import - com o valor fixo, um teste (ou qualquer chamador) que troque
    # MANIFEST continuaria varrendo o caminho antigo, em silencio. Foi
    # exatamente assim que a 1a versao desta funcao nasceu, e o teste pegou.
    if root_manifest is None:
        root_manifest = MANIFEST
    found: list[str] = []
    seen: set[str] = set()
    pending = [os.path.abspath(root_manifest)]
    while pending:
        path = pending.pop(0)
        if path in seen or not os.path.isfile(path):
            continue
        seen.add(path)
        found.append(path)
        with open(path, encoding="utf-8", errors="replace") as f:
            stripped = strip_cmake_line_comments(f.read())
        here = os.path.dirname(path)
        for m in _SUBDIR_RE.finditer(stripped):
            sub = m.group(1).strip('"')
            if "${" in sub:
                continue
            child = os.path.abspath(os.path.join(here, sub, "CMakeLists.txt"))
            if child not in seen:
                pending.append(child)
    return found


def find_declared_names(files: list[str] | None = None):
    """{nome: "<caminho relativo>:<linha>"} da 1a ocorrencia de cada nome."""
    if files is None:
        files = collect_build_cmakelists()
    if not files:
        print(f"GATE(fetchcontent-manifest): FALHA - manifesto nao encontrado em "
              f"{os.path.relpath(MANIFEST, ROOT)}.")
        return None
    names = {}
    for path in files:
        with open(path, encoding="utf-8", errors="replace") as f:
            stripped = strip_cmake_line_comments(f.read())
        for m in _DECLARE_RE.finditer(stripped):
            name = m.group(1)
            line_no = stripped.count("\n", 0, m.start()) + 1
            names.setdefault(name, f"{os.path.relpath(path, ROOT)}:{line_no}")
    return names


def main() -> int:
    files = collect_build_cmakelists()
    names = find_declared_names(files)
    if names is None:
        return 1

    # DECLARA O QUE VARRE (mesma regra do GATE(spdx-required)): "OK" tem de
    # significar "varri estes N arquivos", nunca "nao achei onde procurei".
    print(f"GATE(fetchcontent-manifest): varrendo {len(files)} CMakeLists.txt do "
          f"grafo de add_subdirectory a partir de "
          f"{os.path.relpath(MANIFEST, ROOT)} "
          f"({', '.join(os.path.relpath(f, ROOT) for f in files)}).")

    unexpected = sorted(n for n in names if n not in ALLOWED)
    missing = sorted(ALLOWED - set(names))

    if unexpected:
        print(f"GATE(fetchcontent-manifest): FALHA - {len(unexpected)} "
              f"FetchContent_Declare fora da lista fechada {sorted(ALLOWED)}.")
        for name in unexpected:
            print(f"    - {name} ({names[name]})")
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
              "NAO aparece mais em nenhum CMakeLists.txt do build - se a remocao "
              "foi de proposito, encolha ALLOWED em tools/fetchcontent_manifest.py "
              "no mesmo commit.")
        return 0

    print(f"GATE(fetchcontent-manifest): OK ({len(names)} FetchContent_Declare em "
          f"{len(files)} CMakeLists.txt do build, todos na lista fechada "
          f"{sorted(ALLOWED)}).")
    return 0


if __name__ == "__main__":
    sys.exit(main())

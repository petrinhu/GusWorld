#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
"""tools_isolation_zero.py - GATE(tools-isolation) do tools/check.sh.

FATIA1-LOG-CLOCK (auditoria 2026-08-06): `GATE(tools-isolation)` era citado
como existente em TRES lugares (`TODO.md`, o docstring deste proprio modulo/
`sdl_layer_ratchet.py` e `docs/tech/plano-camadas-sdl.md`) mas nao estava
implementado em NENHUM ponto executavel - a fronteira `app/tools/` para `app/`
so era protegida ESTRUTURALMENTE (CMakeLists.txt separado), sem gate ativo.
Este arquivo fecha o furo: a partir de agora a citacao e verdadeira.

O QUE ESTE GATE PROTEGE (docs/tech/plano-camadas-sdl.md secao 6/8): `app/tools/`
tem `CMakeLists.txt` PROPRIO e STANDALONE (reusa arvores ja populadas por
`SOURCE_DIR` do `_deps` do build principal, nunca clona de novo) - NAO e
`add_subdirectory` de `GusEngine/app/CMakeLists.txt` nem de
`GusEngine/CMakeLists.txt`. Essa condicao estrutural e o que sustenta TRES
exececoes documentadas em outros gates/planos:
  - `tools/sdl_layer_ratchet.py` NAO conta arquivos de `app/tools/` como
    producao (nao compilam dentro de `gusengine_app`);
  - `tools/sdl_log_clock_zero.py` idem (mesmo escopo, mesmo motivo);
  - `tools/fetchcontent_manifest.py` so caminha o grafo REAL de
    `add_subdirectory` a partir do manifesto raiz - `app/tools/*` fica de fora
    "por construcao", nao por lista de exclusao.
No dia em que alguem linkar `app/tools/` em `gusengine_app` (via
`add_subdirectory(tools)` em qualquer um dos dois CMakeLists.txt abaixo), as
tres exececoes desmoronam ao mesmo tempo e NINGUEM percebe sem este gate - os
arquivos de `app/tools/` passariam a compilar dentro do binario shippado
enquanto tres gates diferentes continuam tratando-os como "fora do jogo".

ESCOPO (o mesmo da proposta original, docs/tech/plano-camadas-sdl.md secao 8,
item 0): `GusEngine/CMakeLists.txt` e `GusEngine/app/CMakeLists.txt` - os DOIS
unicos lugares de onde uma cadeia de `add_subdirectory` poderia alcancar
`app/tools/` (o proprio `app/tools/CMakeLists.txt` e standalone, nunca e
incluido por outro `add_subdirectory`; se um dia um `tools/CMakeLists.txt`
proprio de outra camada aparecer, ele entra no escopo abaixo no MESMO commit).

Parser: reusa `strip_cmake_line_comments` de `tools/fetchcontent_manifest.py`
(trunca cada linha no primeiro `#` fora de string) - a mesma razao de sempre:
um `grep` cru casaria a documentacao deste proprio docstring/comentarios que
MENCIONAM `add_subdirectory(tools)` como texto, nao como uso real (foi
exatamente essa armadilha que o GATE(arch) original caiu com Qt/SDL, ver
docs/tech/plano-camadas-sdl.md secao 1).

NASCE VERDE (sabotagem controlada, ver tools/tests/test_tools_isolation_zero.py):
hoje nenhum dos dois arquivos tem `add_subdirectory(tools)` real - medido com
`grep -n 'add_subdirectory(tools)' GusEngine/CMakeLists.txt
GusEngine/app/CMakeLists.txt` (vazio) e reconfirmado por este parser.
"""

import os
import re
import sys

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(SCRIPT_DIR)
sys.path.insert(0, SCRIPT_DIR)
from fetchcontent_manifest import strip_cmake_line_comments  # noqa: E402

ENGINE = os.path.join(ROOT, "GusEngine")

# OS DOIS UNICOS PONTOS de onde uma cadeia de add_subdirectory alcancaria
# app/tools/ hoje (ver docstring). Se um novo CMakeLists.txt nascer entre a
# raiz e app/tools/, ele entra aqui no MESMO commit.
SCAN_FILES = [
    os.path.join(ENGINE, "CMakeLists.txt"),
    os.path.join(ENGINE, "app", "CMakeLists.txt"),
]

# O NOME exato do subdiretorio proibido (app/tools/ e sempre referenciado
# como "tools" relativo ao CMakeLists.txt que o citaria).
FORBIDDEN_NAME = "tools"

_SUBDIR_RE = re.compile(r"add_subdirectory\s*\(\s*([^\s)]+)")


def find_offenders():
    """[(caminho relativo, linha, trecho)] de add_subdirectory(tools) REAL
    (fora de comentario) em qualquer arquivo de SCAN_FILES."""
    offenders = []
    for path in SCAN_FILES:
        if not os.path.isfile(path):
            continue
        with open(path, encoding="utf-8", errors="replace") as f:
            raw = f.read()
        stripped = strip_cmake_line_comments(raw)
        for m in _SUBDIR_RE.finditer(stripped):
            name = m.group(1).strip('"').strip("'")
            if name == FORBIDDEN_NAME:
                line_no = stripped.count("\n", 0, m.start()) + 1
                offenders.append((os.path.relpath(path, ROOT), line_no,
                                   m.group(0)))
    offenders.sort()
    return offenders


def main() -> int:
    scanned = [p for p in SCAN_FILES if os.path.isfile(p)]
    # SENTINELA DE ESCOPO (mesma regra dos demais gates zero-tolerancia): "OK"
    # so pode significar "varri estes arquivos", nunca "nao achei porque nao
    # olhei". Os dois arquivos SEMPRE existem no repo real (sao os
    # CMakeLists.txt raiz e de app/) - se sumirem, e o proprio build que
    # quebrou, e este gate reprova em vez de dizer OK vazio.
    if len(scanned) != len(SCAN_FILES):
        faltando = sorted(set(os.path.relpath(p, ROOT) for p in SCAN_FILES)
                           - set(os.path.relpath(p, ROOT) for p in scanned))
        print(f"GATE(tools-isolation): FALHA - {len(faltando)} arquivo(s) do "
              f"escopo nao encontrado(s): {faltando}. Este gate nao pode dizer "
              "OK sem ter olhado pra ambos.")
        return 1

    offenders = find_offenders()
    escopo_txt = ", ".join(os.path.relpath(p, ROOT) for p in SCAN_FILES)
    if offenders:
        print(f"GATE(tools-isolation): FALHA - {len(offenders)} "
              f"add_subdirectory({FORBIDDEN_NAME}) real encontrado ({escopo_txt}).")
        for path, line_no, trecho in offenders:
            print(f"    - {path}:{line_no}: {trecho}")
        print("  app/tools/ e STANDALONE de proposito (CMakeLists.txt proprio,")
        print("  reusa _deps ja populados, nao linka em gusengine_app). Linkar")
        print("  aqui invalida DE UMA VEZ tres exececoes documentadas:")
        print("  tools/sdl_layer_ratchet.py, tools/sdl_log_clock_zero.py e o")
        print("  walk de tools/fetchcontent_manifest.py (docs/tech/")
        print("  plano-camadas-sdl.md secao 6/8). Se a intencao e real,")
        print("  atualizar os tres gates + o plano no MESMO commit, nao so")
        print("  linkar tools/ em silencio.")
        return 1

    print(f"GATE(tools-isolation): OK (0 add_subdirectory({FORBIDDEN_NAME}) "
          f"em {len(scanned)} arquivo(s) varrido(s): {escopo_txt}).")
    return 0


if __name__ == "__main__":
    sys.exit(main())

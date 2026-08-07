#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
"""uilayer_census.py - conta reproduzivel de ocorrencias de "UiLayer" no
GusEngine (UILAYER-EXCLUSIVIDADE, auditoria 2026-08-06/07).

NAO E UM GATE de tools/check.sh (esta auditoria e read-only, nao tem
"defeito" pra reprovar) - e uma FERRAMENTA DE CENSO, commitada para que o
numero N citado em docs/tech/uilayer-exclusividade.md deixe de ser uma
afirmacao e passe a ser uma MEDIDA que qualquer um reproduz rodando este
script.

POR QUE O NUMERO ANTIGO (233) NAO ERA REPRODUZIVEL, causa raiz medida:

O documento original usou o comando `grep -rn "UiLayer" GusEngine/
--include='*.cpp' --include='*.hpp' | grep -v '/build'`. Nesta maquina, o
`grep` de um shell INTERATIVO do Claude Code e uma FUNCAO que embrulha
`ugrep --ignore-files` (respeita .gitignore automaticamente) - entao o
`GusEngine/build/` (gitignored) ja saia da varredura ANTES do `grep -v
'/build'` redundante entrar em acao, e o numero batia com o que da varrendo
so os arquivos RASTREADOS (confirmado: 233 = a contagem sobre o commit
287c29b via `git ls-files`).

Um auditor independente que rode o MESMO comando fora desse shell (script
Python via subprocess, `command grep`, `\\grep`, outro terminal, outra
maquina) usa o `grep` REAL (GNU grep), que NAO filtra `.gitignore` - e
`GusEngine/build/` nesta maquina de trabalho, num dado momento, pode conter
ATE OITO arvores de build/scratch diferentes (`linux-release`, `asan-gate`,
`caua_probe_scratch`, `city_audit_scratch`, `framegrab_byte_identical_scratch`,
`framegrab_ordem_scratch`, `npcdlg_scratch`, `scratchpad_shots`), cada uma com
sua PROPRIA copia vendorizada do glintfx via FetchContent
(`_deps/glintfx-src`, `_deps/glintfx-build`) - cada copia contribui centenas
de ocorrencias de "UiLayer" (medido: uma unica arvore de build somou 2186
so nela). O numero final depende de QUANTAS dessas arvores de scratch existem
no disco NO MOMENTO da medicao - uma quantidade efemera, especifica da
sessao, nunca canonica. Isso explica por que dois auditores honestos, no
mesmo repositorio, no mesmo dia, mediram 523 e 359: cada um tinha um
subconjunto diferente de diretorios de build/scratch no disco, e nenhum dos
dois numeros e "errado" para o que cada shell efetivamente varreu - o
metodo original e que dependia de estado nao-versionado.

O CONSERTO (o que este script faz, diferente do metodo original): varre
SOMENTE arquivos RASTREADOS pelo git (`git ls-files`), nunca o disco cru.
`GusEngine/build/` nunca esta em `git ls-files` (esta no `.gitignore`), entao
o numero independe de quantas arvores de build/scratch existem no disco, de
qual `grep` o chamador tem instalado, de alias de shell, e de qual maquina
roda - MESMO commit, MESMO numero, sempre.

ESCOPO (declarado aqui, nao em prosa externa): todo arquivo rastreado pelo
git cujo caminho comeca com `GusEngine/` e termina em `.cpp` ou `.hpp`
(mesma extensao-alvo do metodo original: `--include='*.cpp' --include='*.hpp'`).
Contagem = numero de LINHAS (nao de arquivos) que contem a substring literal
"UiLayer" em qualquer lugar (comentario, string, codigo real) - o mesmo
criterio bruto do `grep -n` original; a CLASSIFICACAO em classes a-e
(construcao/storage/referencia/comentario/assinatura) continua sendo o
trabalho manual documentado em docs/tech/uilayer-exclusividade.md secao 2,
que este script NAO reproduz (e julgamento de arquitetura, fora do escopo de
uma contagem mecanica).

Uso:
    python3 tools/uilayer_census.py            # so o total N
    python3 tools/uilayer_census.py --list      # + lista arquivo:linha completa
"""

import subprocess
import sys

SCRIPT_ARG_LIST = "--list"


def tracked_files():
    """git ls-files, restrito a GusEngine/*.cpp e GusEngine/*.hpp -
    DETERMINISTICO por construcao: git nunca lista o que esta gitignored,
    entao GusEngine/build/ (e qualquer arvore de scratch dentro dele) jamais
    entra aqui, nao importa quantas existam no disco no momento da chamada."""
    out = subprocess.run(
        ["git", "ls-files", "GusEngine/*.cpp", "GusEngine/*.hpp"],
        capture_output=True, text=True, check=True,
    )
    return [line for line in out.stdout.splitlines() if line]


def census():
    """[(caminho, numero_da_linha, texto_da_linha)] de toda ocorrencia da
    substring "UiLayer" nos arquivos rastreados do escopo."""
    hits = []
    for path in tracked_files():
        with open(path, encoding="utf-8", errors="replace") as f:
            for line_no, line in enumerate(f, start=1):
                if "UiLayer" in line:
                    hits.append((path, line_no, line.rstrip("\n")))
    return hits


def main(argv) -> int:
    files = tracked_files()
    if not files:
        # SENTINELA DE ESCOPO (mesma regra dos gates zero-tolerancia da casa):
        # zero arquivo rastreado seria a arvore inteira desaparecida - nunca
        # "OK silencioso".
        print("uilayer-census: FALHA - 0 arquivo GusEngine/*.cpp|*.hpp "
              "rastreado pelo git. Rode a partir da raiz do repo.")
        return 1

    hits = census()
    print(f"uilayer-census: N = {len(hits)} ocorrencia(s) de \"UiLayer\" em "
          f"{len(files)} arquivo(s) rastreado(s) pelo git "
          f"(escopo: GusEngine/*.cpp + GusEngine/*.hpp, via `git ls-files` - "
          f"GusEngine/build/ nunca entra, por construcao).")

    if SCRIPT_ARG_LIST in argv:
        for path, line_no, text in hits:
            print(f"    {path}:{line_no}:{text}")

    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))

#!/usr/bin/env python3
"""spdx_header_required.py - GATE(spdx-required) do tools/check.sh.

NAO EXISTIA NENHUM GATE DE LICENCA (2026-08-01): a onda LICENSE-APACHE
rotacionou 531 cabecalhos GPL-3.0-or-later -> Apache-2.0 (LIC-APACHE-3-SPDX),
mas nada no CI ou no check.sh garantia que o PROXIMO arquivo nasceria com
cabecalho, ou que os 3 orfaos achados pelo QA (ressalva nao-bloqueante da
onda: player_sprites_loader.hpp/.cpp e player_sprites_layout_test.cpp, que
NUNCA tiveram SPDX, pre-existentes a onda) ficassem cobertos por alguma
coisa. Este gate fecha a lacuna.

LICAO QUE DEFINE O DESENHO (veio do glintfx no mesmo dia, mesma auditoria):
"um verificador com ponto cego confirma o que se espera dele". O guard de
SPDX DELES passou verde com headers errados porque nao varria o diretorio
onde eles estavam, e so acrescentar o diretorio nao bastou (o find deles
casava por extensao; hook de git e sem extensao). Por isso:

  1. ENUMERA, NAO BUSCA: a lista de arquivos vem de `git ls-files` com
     PADROES EXPLICITOS de extensao (nunca find por glob de diretorio nem
     os.walk livre) - o espaco tem que ser fechado por construcao, e
     `git ls-files` tambem tem a vantagem de nunca listar untracked (um
     agente com trabalho em voo, ainda sem commit, nao entra no gate).
  2. DECLARA O QUE VARRE: a saida sempre imprime quantos arquivos varreu,
     quais extensoes e o caminho do allowlist. "OK" so pode significar
     "varri estes N", nunca "nao achei problema onde procurei".

ESCOPO (extensoes .cpp/.hpp/.h/.c, GusEngine/ + resto do repo): comeca
exatamente pelo que a onda rotacionou. Medido em 2026-08-01 (git grep -l
"SPDX-License-Identifier" -- '*.cpp' '*.hpp' '*.h' '*.c'): 532 de 538
arquivos ja tinham o cabecalho, e os 6 sem sao EXATAMENTE os 3 orfaos do QA
mais 3 vendors de terceiro (glad em platform/rmlui/, 2 headers do stb) - o
5o vendor de terceiro, monocypher.c/.h, ja carrega SPDX proprio (linha 10,
BSD-2-Clause OR CC0-1.0) e por isso NAO aparece como "sem SPDX" no grep, mas
segue na allowlist porque o gate exige espedificamente Apache-2.0, nao
"qualquer SPDX". Nao existe hoje nenhum .cpp/.hpp/.h/.c fora de GusEngine/,
entao o escopo "repo inteiro" e "GusEngine/" coincidem na pratica.

FORA DE ESCOPO, DE PROPOSITO (nao omissao - decisao registrada): .py/.sh/
.cmake/.yml. Medido no mesmo dia: 0/9 .sh, 0/1 .cmake, 0/3 .yml e 0/14 .py
NOSSOS (os 2 .py com SPDX sao os roster-analogos, MIT, de terceiro) tem
QUALQUER cabecalho de licenca hoje - ligar o gate nessas extensoes agora
faria ele nascer vermelho contra ~27 arquivos pre-existentes, violando a
regra de "nao nascer vermelho" (docs/tech, requisito 3 desta fatia). Se o
projeto decidir estender a rotacao a tooling/CI, a extensao deste gate e
trivial (so mexer em EXTENSIONS abaixo) - mas isso e uma fatia NOVA, com
cabecalhos aplicados ANTES do gate ligar, exatamente como esta fatia fez
para .cpp/.hpp/.h/.c.

ALLOWLIST (tools/spdx_allowlist.txt, arquivo VERSIONADO, uma entrada por
linha com motivo obrigatorio - nao lista embutida neste script): so cobre
arquivo de TERCEIRO, onde por o nosso SPDX Apache-2.0 seria declaracao de
licenca FALSA. RmlUi_Include_GL3.h e o caso que ja quase deu errado antes
(nome de arquivo enganoso, e' o header do glad vendorizado fora de
third_party/): o padrao de deteccao NAO pode ser dispensado por analise de
nome de arquivo, so pela allowlist explicita.
"""

from __future__ import annotations

import os
import subprocess
import sys

# Raiz do repo = pai de tools/ (resolve symlink do proprio script).
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(SCRIPT_DIR)

ALLOWLIST_PATH = os.path.join(SCRIPT_DIR, "spdx_allowlist.txt")

# Extensoes fechadas por construcao (ver docstring, secao ESCOPO). Cada
# entrada vira um pathspec `*.<ext>` passado ao `git ls-files` - nunca um
# glob de diretorio, nunca os.walk.
EXTENSIONS = ("cpp", "hpp", "h", "c")

REQUIRED = "SPDX-License-Identifier: Apache-2.0"

# O cabecalho vive na linha 1 nos 531 arquivos ja rotacionados (medido nesta
# mesma auditoria); tolera ate a linha 5 para nao reprovar por variacao
# cosmetica futura (ex.: um preambulo curto), sem abrir mao de checar perto
# do topo do arquivo (nao em qualquer lugar do arquivo).
MAX_HEADER_LINES = 5


def list_tracked_files(root: str, extensions: tuple[str, ...]) -> list[str]:
    """Enumera via `git ls-files`, nunca find/os.walk (ver docstring)."""
    patterns = [f"*.{ext}" for ext in extensions]
    out = subprocess.run(
        ["git", "-C", root, "ls-files", "--"] + patterns,
        capture_output=True,
        text=True,
        check=True,
    )
    return [line for line in out.stdout.splitlines() if line]


def load_allowlist(path: str) -> dict[str, str]:
    """<caminho>\\t<motivo> por linha; '#' e linha em branco sao comentario."""
    entries: dict[str, str] = {}
    if not os.path.isfile(path):
        return entries
    with open(path, encoding="utf-8") as f:
        for lineno, raw in enumerate(f, start=1):
            line = raw.rstrip("\n")
            if not line.strip() or line.lstrip().startswith("#"):
                continue
            if "\t" not in line:
                raise ValueError(
                    f"{path}:{lineno}: entrada sem TAB separando caminho e motivo: {line!r}"
                )
            rel, reason = line.split("\t", 1)
            rel = rel.strip()
            reason = reason.strip()
            if not rel or not reason:
                raise ValueError(f"{path}:{lineno}: caminho ou motivo vazio: {line!r}")
            entries[rel] = reason
    return entries


def has_required_header(fp: str) -> bool:
    try:
        with open(fp, encoding="utf-8", errors="replace") as f:
            for _ in range(MAX_HEADER_LINES):
                line = f.readline()
                if not line:
                    break
                if REQUIRED in line:
                    return True
    except OSError:
        return False
    return False


def find_offenders(files: list[str], root: str, allowlist: dict[str, str]) -> list[str]:
    offenders = []
    for rel in files:
        if rel in allowlist:
            continue
        fp = os.path.join(root, rel)
        if not has_required_header(fp):
            offenders.append(rel)
    offenders.sort()
    return offenders


def find_stale_allowlist(root: str, allowlist: dict[str, str]) -> list[tuple[str, str]]:
    """Entrada cujo arquivo sumiu do disco vira arraste de excecao morta."""
    stale = []
    for rel in sorted(allowlist):
        fp = os.path.join(root, rel)
        if not os.path.isfile(fp):
            stale.append((rel, "arquivo nao existe mais"))
    return stale


def main() -> int:
    allowlist = load_allowlist(ALLOWLIST_PATH)
    files = list_tracked_files(ROOT, EXTENSIONS)
    offenders = find_offenders(files, ROOT, allowlist)
    stale = find_stale_allowlist(ROOT, allowlist)

    ext_list = ", ".join(f".{e}" for e in EXTENSIONS)
    print(
        f"GATE(spdx-required): varrendo {len(files)} arquivo(s) rastreado(s) "
        f"({ext_list}) via git ls-files; allowlist = "
        f"{os.path.relpath(ALLOWLIST_PATH, ROOT)} ({len(allowlist)} entrada(s))."
    )

    ok = True

    if offenders:
        ok = False
        print(
            f"GATE(spdx-required): FALHA - {len(offenders)} arquivo(s) sem "
            f'"{REQUIRED}" nas primeiras {MAX_HEADER_LINES} linhas:'
        )
        for rel in offenders:
            print(f"    - {rel}")
        print(
            "  Se o arquivo e NOSSO: adicione a linha (ver qualquer arquivo em "
            "GusEngine/core/ como exemplo). Se e de TERCEIRO: documente o "
            f"motivo em {os.path.relpath(ALLOWLIST_PATH, ROOT)} (NUNCA marque "
            "arquivo de terceiro com o nosso SPDX - seria licenca falsa)."
        )

    if stale:
        ok = False
        print(
            f"GATE(spdx-required): FALHA - {len(stale)} entrada(s) da allowlist "
            "estao desatualizadas (arraste de excecao morta):"
        )
        for rel, reason in stale:
            print(f"    - {rel}: {reason}")
        print(
            f"  Remova a entrada de {os.path.relpath(ALLOWLIST_PATH, ROOT)} no "
            "mesmo commit que apagou/moveu o arquivo."
        )

    if not ok:
        return 1

    print(
        f"GATE(spdx-required): OK ({len(files)} arquivo(s) varrido(s), "
        f"{len(allowlist)} na allowlist auditada e sem entrada desatualizada, "
        f'0 sem "{REQUIRED}").'
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())

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

DUAS REGRAS, NAO UMA (GATES-HARDEN, 2026-08-06): ate esta fatia o gate so
procurava a PRESENCA do Apache-2.0 perto do topo, e nunca a AUSENCIA de outra
licenca - a auditoria mediu o furo: com `GPL-3.0-or-later` na linha 1 e
`Apache-2.0` na linha 2, o gate passava VERDE. O proprio docstring acima
promete impedir "o antigo GPL de voltar", e a promessa escrita era maior que o
comportamento. Agora:

  REGRA 1 (presenca): "SPDX-License-Identifier: Apache-2.0" nas primeiras
  MAX_HEADER_LINES linhas - o que ja existia.
  REGRA 2 (exclusividade): NENHUMA outra tag `SPDX-License-Identifier:` pode
  aparecer no arquivo INTEIRO, em nenhuma linha. Nao e o teto do cabecalho: a
  linha 40 tambem reprova - senao a proxima variante do mesmo furo (GPL longe
  do topo) reabriria o buraco que este trecho fecha.

A regra 2 vale para o arquivo inteiro porque a medicao permite: em 2026-08-06,
os 571 arquivos rastreados no escopo tinham exatamente 566 tags `Apache-2.0` e
2 `BSD-2-Clause OR CC0-1.0` (monocypher.c/.h, ja na allowlist) - nenhuma outra
string, em nenhuma linha. O gate nasce verde. Se um dia um arquivo NOSSO
precisar citar outra licenca de verdade (fixture de teste, por exemplo), o
caminho e a allowlist com motivo, nao afrouxar a regra.

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
import re
import subprocess
import sys

# Raiz do repo = pai de tools/ (resolve symlink do proprio script).
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(SCRIPT_DIR)

ALLOWLIST_PATH = os.path.join(SCRIPT_DIR, "spdx_allowlist.txt")

# Extensoes fechadas por construcao (ver docstring, secao ESCOPO). Cada
# entrada vira um pathspec `*.<ext>` passado ao `git ls-files` - nunca um
# glob de diretorio, nunca os.walk.
#
# GATES-HARDEN (2026-08-06): `.cc/.cxx/.inl/.hxx/.ipp` entraram porque o custo e
# uma linha e a medicao do dia deu ZERO arquivo rastreado com essas extensoes -
# o gate nasce verde e fecha a porta pro proximo `.inl` nascer sem cabecalho.
# ⚠️ `.py/.sh/.cmake/CMakeLists.txt` continuam FORA (ver "FORA DE ESCOPO" acima):
# ~27 arquivos pre-existentes sem cabecalho fariam o gate nascer VERMELHO. Isso e
# fatia PROPRIA, com os cabecalhos aplicados ANTES de a extensao entrar aqui.
EXTENSIONS = ("cpp", "hpp", "h", "c", "cc", "cxx", "inl", "hxx", "ipp")

REQUIRED = "SPDX-License-Identifier: Apache-2.0"

# REGRA 2 (exclusividade, ver docstring): captura o VALOR de qualquer tag SPDX
# no arquivo. O valor vai ate o fim da linha, com o rabo de comentario de bloco
# (` */`) e espaco a direita descartados - a forma `/* SPDX-...: X */` e legal
# em C e nao pode escapar da regra por causa do fechamento do comentario.
_SPDX_TAG_RE = re.compile(r"SPDX-License-Identifier:\s*(.*)")
_TAG_TAIL_RE = re.compile(r"\s*(?:\*/|-->|#>)\s*$")

REQUIRED_VALUE = "Apache-2.0"

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


def foreign_license_tags(fp: str) -> list[tuple[int, str]]:
    """REGRA 2: toda tag SPDX do arquivo INTEIRO cujo valor NAO e Apache-2.0.

    Devolve [(linha, valor)]. Lista vazia = so ha Apache-2.0 (ou nenhuma tag -
    a ausencia e problema da REGRA 1, nao desta). E o que impede a licenca
    antiga de VOLTAR: procurar so a presenca do Apache deixava passar um
    arquivo com GPL na linha 1 e Apache na linha 2 (furo medido)."""
    found: list[tuple[int, str]] = []
    try:
        with open(fp, encoding="utf-8", errors="replace") as f:
            for lineno, line in enumerate(f, start=1):
                m = _SPDX_TAG_RE.search(line)
                if not m:
                    continue
                value = _TAG_TAIL_RE.sub("", m.group(1)).strip()
                if value != REQUIRED_VALUE:
                    found.append((lineno, value))
    except OSError:
        return []
    return found


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


def find_foreign(files: list[str], root: str,
                 allowlist: dict[str, str]) -> list[tuple[str, int, str]]:
    out: list[tuple[str, int, str]] = []
    for rel in files:
        if rel in allowlist:
            continue
        for lineno, value in foreign_license_tags(os.path.join(root, rel)):
            out.append((rel, lineno, value))
    out.sort()
    return out


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
    foreign = find_foreign(files, ROOT, allowlist)
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

    if foreign:
        ok = False
        print(
            f"GATE(spdx-required): FALHA - {len(foreign)} tag(s) "
            "SPDX-License-Identifier com licenca DIFERENTE de "
            f"{REQUIRED_VALUE} (a licenca antiga nao pode voltar, nem "
            "convivendo com a nova no mesmo arquivo):"
        )
        for rel, lineno, value in foreign:
            print(f"    - {rel}:{lineno}: {value!r}")
        print(
            "  Um arquivo NOSSO declara Apache-2.0 e mais nada. Se o arquivo e de "
            f"TERCEIRO, ele vai pra {os.path.relpath(ALLOWLIST_PATH, ROOT)} com "
            "motivo (nunca ganha o nosso SPDX por cima da licenca dele)."
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
        f'0 sem "{REQUIRED}" perto do topo e 0 com tag SPDX de outra licenca '
        "em qualquer linha)."
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())

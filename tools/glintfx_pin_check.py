#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
"""glintfx_pin_check.py - a versao do glintfx se confere pela TAG, nao pelo config.hpp.

O DEFEITO QUE ESTE SCRIPT EXISTE PRA IMPEDIR (CI-RODA-OS-GATES, 2026-08-06): o
ritual de bump conferia a versao do glintfx lendo o `GLINTFX_VERSION` do
`config.hpp` GERADO. Esse header nao distingue v0.29.0 de v0.30.0 - a tag v0.30.0
deles NAO bumpou o `project(glintfx ... VERSION 0.29.0)` da linha 3 do CMakeLists
deles, e como o `config.hpp.in` injeta `@PROJECT_VERSION@`, o header gerado diz
"0.29.0" rodando a v0.30.0. Medido nesta arvore:

    config.hpp:164          -> #define GLINTFX_VERSION "0.29.0"   (MENTE)
    git describe --tags     -> v0.30.0                            (verdade)

Ja reportado a eles pelo bus. Mas o conserto do NOSSO lado nao e esperar eles
consertarem: e parar de perguntar pra uma fonte que nao sabe a resposta. A tag e
o que o nosso `GIT_TAG` pede, entao a tag e o que se confere.

E o mesmo erro de metodo que a casa ja catalogou em outras roupas: "introspeccao,
nunca grep em arquivo de build" e "a verificacao responde OUTRA pergunta". O
`config.hpp` responde "que numero os autores escreveram no project()?", que NAO e
"qual arvore esta no meu _deps?".

NAO e um gate do check.sh (nao entra no rc): e ferramenta do RITUAL DE BUMP,
chamada a mao quando o `GIT_TAG` do glintfx muda. Fica de fora do estagio GATE de
proposito - depende do `_deps/` populado, que so existe depois de configurar, e
reprovar a arvore de quem ainda nao configurou seria barulho, nao protecao.
Promove-la a gate e decisao do lider, nao deste script.

Uso:
    python3 tools/glintfx_pin_check.py [--preset linux-release]

Saida: 0 se a tag baixada casa com o GIT_TAG declarado; 1 se divergem; 2 se nao
deu pra medir (sem _deps, sem git). NUNCA 0 por nao ter conseguido olhar.
"""

import argparse
import os
import re
import subprocess
import sys

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(SCRIPT_DIR)
ENGINE = os.path.join(ROOT, "GusEngine")
CMAKELISTS = os.path.join(ENGINE, "CMakeLists.txt")


def pin_declarado() -> str:
    """O GIT_TAG do bloco FetchContent_Declare(glintfx ...) do nosso CMakeLists."""
    with open(CMAKELISTS, encoding="utf-8") as f:
        texto = f.read()
    # Recorta do `FetchContent_Declare(` cujo 1o argumento e `glintfx` ate o `)`
    # que fecha - assim um GIT_TAG de OUTRA dependencia (SDL3/RmlUi/Catch2) nunca
    # e lido por engano.
    m = re.search(
        r"FetchContent_Declare\(\s*\n\s*glintfx\b(?P<corpo>.*?)^\)",
        texto,
        re.DOTALL | re.MULTILINE,
    )
    if not m:
        raise LookupError(
            "nao achei o bloco FetchContent_Declare(glintfx ...) em "
            f"{CMAKELISTS}. Se o formato mudou, ESTE script tem de ser "
            "atualizado no mesmo commit - sem o bloco ele nao pode dizer OK."
        )
    corpo = m.group("corpo")
    # Ignora linha comentada: o bloco e cheio de comentario historico, e um
    # `# GIT_TAG v0.20.0` de nota de migracao casaria com um regex ingenuo.
    for linha in corpo.splitlines():
        sem_comentario = linha.split("#", 1)[0]
        t = re.search(r"\bGIT_TAG\s+(\S+)", sem_comentario)
        if t:
            return t.group(1)
    raise LookupError("bloco do glintfx achado, mas sem GIT_TAG efetivo (nao comentado).")


def tag_baixada(src: str):
    """A tag REAL da arvore em _deps/glintfx-src, lida do git - nao do config.hpp."""
    try:
        proc = subprocess.run(
            ["git", "-C", src, "describe", "--tags", "--exact-match"],
            capture_output=True, text=True, timeout=30,
        )
    except FileNotFoundError:
        return None, "binario 'git' nao encontrado no PATH."
    if proc.returncode != 0:
        # Sem tag exata (checkout por SHA, ou tag ausente): cai pro SHA, que
        # ainda e evidencia de verdade - so nao e um nome de versao.
        sha = subprocess.run(
            ["git", "-C", src, "rev-parse", "HEAD"],
            capture_output=True, text=True, timeout=30,
        )
        if sha.returncode == 0:
            return None, (
                f"HEAD de {src} nao esta numa tag exata (sha "
                f"{sha.stdout.strip()[:12]}); `describe --exact-match` disse: "
                f"{proc.stderr.strip()}"
            )
        return None, f"nao consegui ler o git de {src}: {proc.stderr.strip()}"
    return proc.stdout.strip(), None


def versao_do_config_hpp(build_dir: str):
    """So pra EXIBIR a divergencia. Este valor nunca decide o rc."""
    caminho = os.path.join(
        build_dir, "_deps", "glintfx-build", "include", "glintfx", "config.hpp"
    )
    try:
        with open(caminho, encoding="utf-8") as f:
            m = re.search(r'#define\s+GLINTFX_VERSION\s+"([^"]*)"', f.read())
            return m.group(1) if m else None
    except OSError:
        return None


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--preset", default="linux-release")
    args = ap.parse_args()

    build_dir = os.path.join(ENGINE, "build", args.preset)
    src = os.path.join(build_dir, "_deps", "glintfx-src")

    try:
        pin = pin_declarado()
    except LookupError as exc:
        print(f"GLINTFX-PIN: NAO VERIFICADO - {exc}")
        return 2

    if not os.path.isdir(src):
        print(f"GLINTFX-PIN: NAO VERIFICADO - {src} nao existe (arvore ainda nao "
              f"configurada com o preset '{args.preset}'?).")
        print(f"  Pin declarado no CMakeLists: {pin}. Rode `cmake --preset "
              f"{args.preset}` antes de conferir o bump.")
        return 2

    tag, erro = tag_baixada(src)
    cfg = versao_do_config_hpp(build_dir)

    if tag is None:
        print(f"GLINTFX-PIN: NAO VERIFICADO - {erro}")
        print(f"  Pin declarado: {pin}. Nao passo em silencio: sem a tag lida do "
              "git, nao ha o que comparar.")
        return 2

    print(f"GLINTFX-PIN: declarado no CMakeLists = {pin}")
    print(f"GLINTFX-PIN: tag REAL em _deps/glintfx-src = {tag}  (git describe "
          "--tags --exact-match)")
    if cfg is not None:
        marca = "" if f"v{cfg}" == tag else "   <-- DIVERGE DA TAG, e por isso NAO decide nada"
        print(f"GLINTFX-PIN: (informativo) GLINTFX_VERSION do config.hpp gerado = "
              f"{cfg}{marca}")

    if tag != pin:
        print(f"GLINTFX-PIN: FALHA - a arvore baixada esta em '{tag}', mas o "
              f"CMakeLists pede '{pin}'.")
        print("  O _deps NAO e repopulado sozinho quando o GIT_TAG muda: reconfigure "
              "(ou apague _deps/glintfx-*) e confira de novo ANTES de assinar o bump.")
        return 1

    print(f"GLINTFX-PIN: OK (arvore baixada e o pin declarado sao a MESMA tag: {tag}).")
    return 0


if __name__ == "__main__":
    sys.exit(main())

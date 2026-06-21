#!/usr/bin/env python3
"""TDD guard leve para C++ no GusEngine (reforco do ciclo test-first).

Bloqueia Write/Edit de um arquivo de IMPLEMENTACAO C++ dentro de GusEngine/
quando NENHUM arquivo de teste menciona o stem do arquivo. Heuristica
estrutural (nao roda build): cobra "escreva o teste primeiro".

Isencoes (permite sempre): arquivos de teste, GusEngine/app/* (stub/glue sem
logica de negocio), e qualquer coisa fora de GusEngine/. Editar impl que ja
tem teste passa (nao trava refactor).

Contrato de hook PreToolUse: le o tool-call em JSON no stdin; exit 0 = permite,
exit 2 + mensagem no stderr = bloqueia.
"""
import sys
import json
import re
import os
import pathlib


def allow():
    sys.exit(0)


def block(stem):
    msg = (
        "TDD guard (GusEngine): nenhum teste menciona '%s'. Escreva o teste "
        "Catch2 PRIMEIRO em GusEngine/.../tests/ (veja-o falhar, vermelho), e "
        "so entao implemente '%s'. Refactor de codigo ja coberto passa normal."
    ) % (stem, stem)
    print(msg, file=sys.stderr)
    sys.exit(2)


TEST_RE = re.compile(r"(^|/)(test_|[^/]*_test\.)")


def main():
    try:
        data = json.loads(sys.stdin.read())
    except Exception:
        allow()

    if data.get("tool_name") not in ("Write", "Edit", "MultiEdit"):
        allow()

    fp = (data.get("tool_input") or {}).get("file_path", "")
    if not fp:
        allow()

    p = fp.replace("\\", "/")
    if "GusEngine/" not in p:
        allow()
    if not re.search(r"\.(cpp|cc|cxx|hpp|hxx|h)$", p):
        allow()
    if "/tests/" in p or TEST_RE.search(p):
        allow()
    if re.search(r"/app/.*\.(cpp|hpp|h)$", p):
        allow()

    stem = pathlib.Path(p).stem
    ge = pathlib.Path(os.environ.get("CLAUDE_PROJECT_DIR", ".")) / "GusEngine"
    if not ge.is_dir():
        allow()

    for tf in ge.rglob("*"):
        sp = str(tf).replace("\\", "/")
        if "/build/" in sp or not tf.is_file():
            continue
        if "/tests/" in sp or TEST_RE.search(sp):
            try:
                if stem in tf.read_text(errors="ignore"):
                    allow()
            except Exception:
                pass

    block(stem)


main()

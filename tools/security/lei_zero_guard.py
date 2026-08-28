#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
"""Guard da LEI ZERO: o GusWorld liga em GlintFx e no sistema operacional. Mais nada.

Existe porque a LEI ZERO estava escrita e NADA a fazia cumprir: toda violacao
dependia de o lider notar e repetir a ordem. Este script tira essa carga dele.

Nao substitui julgamento -- ele acha NOME de dependencia proibida em arquivo
rastreado. Ferramenta de build (CMake, Ninja, ctest, clang-tidy) NAO e link e
nao entra aqui; asset licenciado (a fonte Pixel Operator Mono) tambem nao.

Uso:  python3 tools/security/lei_zero_guard.py [--quiet]
Saida: 0 = limpo | 2 = violacao encontrada | 1 = erro de execucao
"""
from __future__ import annotations
import re
import subprocess
import sys
from pathlib import Path

# Dependencias que a LEI ZERO proibe o executavel ligar. Nome canonico -> regex.
# So bibliotecas/engines que entrariam no binario ou o substituiriam.
PROIBIDOS = {
    "Qt":          r"\bQt[0-9]?\b|\bQt(Gamepad|RHI|Quick|Widgets|Core)\b",
    "SDL":         r"\bSDL[0-9]?\b|\bSDL_[A-Za-z]+\b",
    "RmlUi":       r"\bRmlUi\b|\bRCSS\b",
    "Godot":       r"\bGodot\b|\bGDScript\b",
    "Unity":       r"\bUnity\b",
    "Unreal":      r"\bUnreal\b",
    "Monocypher":  r"\bMonocypher\b",
    "OpenSSL":     r"\bOpenSSL\b",
    "libsodium":   r"\blibsodium\b",
    "Catch2":      r"\bCatch2\b",
    "GoogleTest":  r"\bGoogleTest\b|\bgtest\b",
    "doctest":     r"\bdoctest\b",
    "Dear ImGui":  r"\bImGui\b",
    "raylib":      r"\braylib\b",
    "bgfx":        r"\bbgfx\b",
    "GLFW":        r"\bGLFW\b",
    "EnTT":        r"\bEnTT\b",
    "fmt":         r"\bfmtlib\b|\bfmt::\b",
    "spdlog":      r"\bspdlog\b",
}

# Caminhos que legitimamente citam nome morto e NAO sao violacao:
# relatorio datado de processo (registro do que foi minerado/auditado), o
# proprio guard, e a lista de termos do outro portao.
# Documento que GOVERNA nao e documento que ENTREGA. Estes nomeiam a praga
# justamente para PROIBI-la, e por isso nao sao violacao -- a lei precisa poder
# dizer o nome do que ela bane. Limitacao declarada, nao silenciosa: o guard NAO
# cobre estes caminhos, e diz isso na saida.
ISENTOS = (
    "GODS_LAWS.md",            # a lei nomeia a stack morta para bani-la
    "TODO.md",                 # o backlog registra a ordem que a proibe
    "CLAUDE.md",               # indice de leis
    "docs/_processo/",         # relatorio datado: registro do que foi minerado
    "tools/security/",         # este guard e a lista do outro portao
)

def rastreados() -> list[str]:
    out = subprocess.run(["git", "ls-files", "-z"], capture_output=True, text=True, check=True)
    return [p for p in out.stdout.split("\0") if p]

def isento(caminho: str) -> bool:
    c = caminho.replace("\\", "/")
    return any(c.startswith(p) for p in ISENTOS)

def main() -> int:
    quiet = "--quiet" in sys.argv
    try:
        arquivos = rastreados()
    except Exception as e:                       # noqa: BLE001
        print(f"lei-zero-guard: nao consegui listar arquivos rastreados: {e}", file=sys.stderr)
        return 1

    achados: list[tuple[str, int, str, str]] = []
    varridos = 0
    for caminho in arquivos:
        if isento(caminho):
            continue
        p = Path(caminho)
        if not p.is_file():
            continue
        try:
            texto = p.read_text(encoding="utf-8")
        except (UnicodeDecodeError, OSError):
            continue                              # binario, cifrado ou ilegivel: fora do escopo
        varridos += 1
        for nome, padrao in PROIBIDOS.items():
            for m in re.finditer(padrao, texto):
                linha = texto.count("\n", 0, m.start()) + 1
                trecho = texto.splitlines()[linha - 1].strip()[:100]
                achados.append((caminho, linha, nome, trecho))

    if not quiet:
        print(f"lei-zero-guard: {varridos} arquivos de texto varridos, "
              f"{len(arquivos) - varridos} pulados (binarios, cifrados ou isentos).")
        print("lei-zero-guard: NAO cobre " + ", ".join(ISENTOS) +
              " -- governam, nao entregam.")
    if not achados:
        if not quiet:
            print("lei-zero-guard: LIMPO. Nenhuma dependencia proibida no corpus.")
        return 0

    porNome: dict[str, int] = {}
    for _, _, nome, _ in achados:
        porNome[nome] = porNome.get(nome, 0) + 1
    print(f"\nlei-zero-guard: {len(achados)} VIOLACOES da LEI ZERO, "
          f"em {len({a[0] for a in achados})} arquivos.\n", file=sys.stderr)
    for nome, n in sorted(porNome.items(), key=lambda kv: -kv[1]):
        print(f"  {nome:<12} {n}", file=sys.stderr)
    print("\nPrimeiras 25 ocorrencias:", file=sys.stderr)
    for caminho, linha, nome, trecho in achados[:25]:
        print(f"  {caminho}:{linha}  [{nome}]  {trecho}", file=sys.stderr)
    if len(achados) > 25:
        print(f"  ... e mais {len(achados) - 25}.", file=sys.stderr)
    print("\nA LEI ZERO diz: o GusWorld liga em GlintFx e no sistema operacional. "
          "Mais nada.", file=sys.stderr)
    return 2

if __name__ == "__main__":
    sys.exit(main())

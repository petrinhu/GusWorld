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


MSG_COMO_APAGAR = """
NAO reescreva a mencao. NAO troque por sinonimo. NAO deixe nota, aviso,
comentario, marcador nem bloco explicando que algo foi removido. O texto que
ficar tem de ler-se como se aquilo nunca tivesse existido (L-24).

Como apagar, conforme o caso:

  1. O ARQUIVO e a dependencia morta -> apague o arquivo inteiro (git rm).

  2. A PASSAGEM a descreve como stack -> apague a PASSAGEM inteira: a frase,
     o item de lista, a secao. Se sobrar frase truncada ou lista com buraco,
     voce cortou errado.

  3. A frase diz que NAO usamos -> apague a CLAUSULA inteira, nunca a palavra
     solta. De "sem rede, sem web, sem Qt, sem Godot" sobra "sem rede, sem
     web". Apagar so a palavra ja corrompeu "firmly" neste repositorio,
     comendo o "rml" do meio.

Depois de apagar, confira:  python3 tools/security/lei_zero_guard.py
"""

def rastreados() -> list[str]:
    out = subprocess.run(["git", "ls-files", "-z"], capture_output=True, text=True, check=True)
    return [p for p in out.stdout.split("\0") if p]

def isento(caminho: str) -> bool:
    c = caminho.replace("\\", "/")
    return any(c.startswith(p) for p in ISENTOS)

def linhas_adicionadas(intervalo: str | None = None) -> list[tuple[str, int, str]]:
    """Modo GATE: so o que este commit ACRESCENTA.

    Checar a arvore inteira travaria todo commit ate a limpeza terminar --
    inclusive os commits QUE FAZEM a limpeza. O gate morde a regressao: se o
    diff nao introduz nome proibido, passa, mesmo com divida antiga na arvore.
    """
    cmd = ["git", "diff", "--unified=0", "--no-color"]
    cmd += ["--cached"] if intervalo is None else intervalo.split()
    out = subprocess.run(cmd, capture_output=True, text=True, check=True)
    achados, caminho, linha = [], None, 0
    for l in out.stdout.split("\n"):
        if l.startswith("+++ b/"):
            caminho = l[6:]
        elif l.startswith("@@"):
            m = re.search(r"\+(\d+)", l)
            linha = int(m.group(1)) if m else 0
        elif l.startswith("+") and not l.startswith("+++") and caminho:
            achados.append((caminho, linha, l[1:]))
            linha += 1
    return achados


def main() -> int:
    quiet = "--quiet" in sys.argv
    intervalo = None
    for i, a in enumerate(sys.argv):
        if a == "--range" and i + 1 < len(sys.argv):
            intervalo = sys.argv[i + 1]
    if intervalo is not None or "--staged" in sys.argv:
        import os
        if str(os.environ.get("LEI_ZERO_GUARD", "")).lower() == "off":
            print("lei-zero-guard: DESLIGADO por LEI_ZERO_GUARD=off.", file=sys.stderr)
            return 0
        return gate(intervalo)
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



def gate(intervalo: str | None = None) -> int:
    """Portao de pre-commit: bloqueia commit que ACRESCENTA dependencia proibida."""
    try:
        adicionadas = linhas_adicionadas(intervalo)
    except Exception as e:                       # noqa: BLE001
        print(f"lei-zero-guard: nao consegui ler o diff: {e}", file=sys.stderr)
        return 1
    violacoes = []
    for caminho, linha, texto in adicionadas:
        if isento(caminho):
            continue
        for nome, padrao in PROIBIDOS.items():
            if re.search(padrao, texto):
                violacoes.append((caminho, linha, nome, texto.strip()[:100]))
    if not violacoes:
        return 0
    print("\n" + "=" * 68, file=sys.stderr)
    print("LEI ZERO: commit BLOQUEADO.", file=sys.stderr)
    print("=" * 68, file=sys.stderr)
    print("\nO GusWorld liga em GlintFx e no sistema operacional. Mais nada.\n",
          file=sys.stderr)
    print(f"Este commit ACRESCENTA {len(violacoes)} referencia(s) a dependencia proibida:\n",
          file=sys.stderr)
    for caminho, linha, nome, texto in violacoes[:15]:
        print(f"  {caminho}:{linha}  [{nome}]", file=sys.stderr)
        print(f"      {texto}", file=sys.stderr)
    if len(violacoes) > 15:
        print(f"  ... e mais {len(violacoes) - 15}.", file=sys.stderr)
    print("\n" + "-" * 68, file=sys.stderr)
    print("O QUE FAZER: APAGUE.  Ordem do lider, verbatim:", file=sys.stderr)
    print('  "e para APAGAR"   e   "Nao para colocar observacao"', file=sys.stderr)
    print("-" * 68, file=sys.stderr)
    print(MSG_COMO_APAGAR, file=sys.stderr)
    print("\nSe for citacao legitima (lei, backlog, relatorio de processo), o caminho",
          file=sys.stderr)
    print("ja deveria estar isento -- veja ISENTOS neste arquivo.", file=sys.stderr)
    print("Para uma excecao pontual e consciente:  LEI_ZERO_GUARD=off git commit ...\n",
          file=sys.stderr)
    return 2


if __name__ == "__main__":
    sys.exit(main())

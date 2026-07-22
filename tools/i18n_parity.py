#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Tabela de paridade i18n do GusWorld (estagio GATE do check.sh).

Espelha o cobertura_i18n.py do PokemonTCGViewer, adaptado ao formato de
catalogo proprio do GusWorld (Markdown "## CHAVE" + valor), nao .ts do Qt.

FONTE DA VERDADE
----------------
A regra canonica de parsing e de paridade vive no C++ POCO em
  GusEngine/domain/.../md_translation_loader.{hpp,cpp}
  GusEngine/domain/.../translation_parity_validator.{hpp,cpp}
e e exercitada pelo ctest (estagio SUITE). Este script replica a MESMA regra
em Python so para PRODUZIR A TABELA legivel por locale (o C++ valida; o Python
exibe), do mesmo jeito que o PTV usa Python puro para a tabela de cobertura.
Se a regra C++ mudar, o teste de drift (md_translation_loader_test) acende
vermelho no SUITE; mantenha as duas em sincronia.

REGRA DE PARSING (identica ao loader C++)
-----------------------------------------
- "## X" com X em UPPER_SNAKE (^[A-Z][A-Z0-9_]*$) e CHAVE;
- "## §1. ...", "### ..." e demais headers de secao NAO sao chave;
- valor = linhas seguintes ate a proxima "## ", linhas iniciadas por '#'
  filtradas, juntadas por '\n', com trim das pontas;
- chave duplicada = last-wins (mas a duplicata e reportada a parte).

O QUE A TABELA MOSTRA (por locale-alvo, vs pt_br como source)
------------------------------------------------------------
  locale  total  traduzido  %  faltando  extra
- total      : nº de chaves na source pt_br (universo).
- traduzido  : chaves do alvo com valor NAO-vazio (cobertura de conteudo).
- %          : traduzido / total.
- faltando   : chaves da source ausentes no alvo (quebra de paridade ESTRUTURAL).
- extra      : chaves do alvo ausentes na source (orfas).

EXIT CODE
---------
Por design do GusWorld (canon: en_intl tem valores VAZIOS ate pos-v1.0.0), o
gate falha em PARIDADE ESTRUTURAL e em DUPLICATA, mas NAO no % de conteudo.
  - faltando > 0  -> exit != 0  (chave da source nao existe no alvo)
  - extra > 0     -> exit != 0  (chave orfa no alvo)
  - duplicata > 0 -> exit != 0  (em qualquer locale, source ou alvo)
  - source com valor vazio > 0 -> exit != 0 (pt_br e lingua-fonte, MUST cheia)
  - cobertura de conteudo (%) so e EXIBIDA; nao reprova (limiar opcional --min).

USO
---
    python3 tools/i18n_parity.py
    python3 tools/i18n_parity.py --min 95      # tambem exige %>=95 no alvo
    python3 tools/i18n_parity.py --dir resources/translations --source pt_br
"""
from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

# Locales-alvo avaliados contra a source pt_br. en_intl e o unico hoje; novos
# locales entram aqui (mesma checagem de paridade/duplicata se aplica).
LOCALES_ALVO_PADRAO = ["en_intl"]
SOURCE_PADRAO = "pt_br"

# ^[A-Z][A-Z0-9_]*$  (regex do is_translation_key do C++, sem arrastar <regex>).
_CHAVE_RE = re.compile(r"^[A-Z][A-Z0-9_]*$")


def is_translation_key(s: str) -> bool:
    """True se 's' e UPPER_SNAKE_CASE. Espelha is_translation_key (C++)."""
    return bool(_CHAVE_RE.fullmatch(s))


def parse(conteudo: str) -> dict[str, str]:
    """Parseia catalogo .md -> {chave: valor}. Espelha parse() do loader C++.

    last-wins em chave duplicada; valor multi-linha juntado por '\n' com trim;
    headers de secao ("## §1.", "### ...") nao viram chave; linhas '#' dentro do
    valor sao filtradas.
    """
    catalogo: dict[str, str] = {}
    chave_atual: str | None = None
    buffer: list[str] = []

    def flush():
        nonlocal chave_atual, buffer
        if chave_atual is not None:
            catalogo[chave_atual] = "\n".join(buffer).strip()
        chave_atual = None
        buffer = []

    for linha in conteudo.splitlines():
        if linha.startswith("## "):
            candidato = linha[3:].strip()
            if is_translation_key(candidato):
                flush()
                chave_atual = candidato
                continue
            # header de secao ("## §1. Menu") -> fecha chave anterior, ignora
            flush()
            continue
        if linha.startswith("#"):
            # qualquer outra linha iniciada por '#' (inclui "### ...") nao entra
            # no valor; o C++ filtra linhas '#' do corpo.
            continue
        if chave_atual is not None:
            buffer.append(linha)
    flush()
    return catalogo


def chaves_duplicadas(conteudo: str) -> list[str]:
    """Chaves "## X" (UPPER_SNAKE) que aparecem mais de uma vez. Espelha
    find_duplicate_keys (C++). Ordenado, cada chave uma vez."""
    vistas: set[str] = set()
    dups: set[str] = set()
    for linha in conteudo.splitlines():
        if linha.startswith("## "):
            candidato = linha[3:].strip()
            if is_translation_key(candidato):
                if candidato in vistas:
                    dups.add(candidato)
                vistas.add(candidato)
    return sorted(dups)


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Tabela de paridade i18n do GusWorld (gate estrutural)."
    )
    parser.add_argument("--dir", default="resources/translations",
                        help="Diretorio dos catalogos .md.")
    parser.add_argument("--source", default=SOURCE_PADRAO,
                        help="Locale source (lingua-fonte). Padrao pt_br.")
    parser.add_argument("--alvos", nargs="*", default=LOCALES_ALVO_PADRAO,
                        help="Locales-alvo. Padrao: en_intl.")
    parser.add_argument("--min", type=float, default=None,
                        help="Se dado, exige cobertura de conteudo (%%) >= MIN "
                             "no alvo (alem da paridade estrutural). Padrao: so "
                             "estrutura reprova.")
    args = parser.parse_args()

    base = Path(args.dir)
    src_path = base / f"{args.source}.md"
    if not src_path.exists():
        print(f"ERRO: source nao encontrada em {src_path}", file=sys.stderr)
        return 2

    src_conteudo = src_path.read_text(encoding="utf-8")
    src = parse(src_conteudo)
    total = len(src)

    falhou = False

    # source: duplicata e valor-vazio sao proibidos (pt_br MUST estar cheia).
    dups_src = chaves_duplicadas(src_conteudo)
    vazias_src = sorted(k for k, v in src.items() if not v.strip())
    if dups_src:
        falhou = True
    if vazias_src:
        falhou = True

    print(f"source: {src_path}  ({total} chaves)")
    cab = f"{'locale':<10} {'total':>5} {'traduz':>6} {'%':>6} {'falta':>5} {'extra':>5}"
    print(cab)
    print("-" * len(cab))

    for loc in args.alvos:
        loc_path = base / f"{loc}.md"
        if not loc_path.exists():
            print(f"{loc:<10}  AUSENTE - {loc_path} nao encontrado")
            falhou = True
            continue
        loc_conteudo = loc_path.read_text(encoding="utf-8")
        alvo = parse(loc_conteudo)

        traduzido = sum(1 for k in src if alvo.get(k, "").strip())
        faltando = sorted(k for k in src if k not in alvo)   # paridade estrutural
        extra = sorted(k for k in alvo if k not in src)      # orfas
        dups_loc = chaves_duplicadas(loc_conteudo)
        pct = (traduzido / total * 100.0) if total else 0.0

        marcas = []
        if faltando:
            marcas.append(f"FALTA={len(faltando)}")
        if extra:
            marcas.append(f"EXTRA={len(extra)}")
        if dups_loc:
            marcas.append(f"DUP={len(dups_loc)}")
        if args.min is not None and pct < args.min:
            marcas.append(f"<{args.min:g}%")
        marca = ("  <- " + " ".join(marcas)) if marcas else ""

        print(f"{loc:<10} {total:>5} {traduzido:>6} {pct:>5.1f}% "
              f"{len(faltando):>5} {len(extra):>5}{marca}")

        if faltando or extra or dups_loc:
            falhou = True
        if args.min is not None and pct < args.min:
            falhou = True

    # rodape: so imprime detalhe quando ha problema (silencioso no caminho feliz).
    if dups_src:
        print(f"\nDUPLICATA em {args.source} ({len(dups_src)}): "
              f"{', '.join(dups_src[:10])}{'...' if len(dups_src) > 10 else ''}")
    if vazias_src:
        print(f"VALOR VAZIO em {args.source} (proibido na fonte, "
              f"{len(vazias_src)}): "
              f"{', '.join(vazias_src[:10])}{'...' if len(vazias_src) > 10 else ''}")

    return 1 if falhou else 0


if __name__ == "__main__":
    raise SystemExit(main())

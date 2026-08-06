#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
"""stb_image_zero.py - GATE(stbi-load-zero) do tools/check.sh.

ZERO-TOLERANCIA (nao ratchet) a `stbi_load`/`stbi_image_free`/inclusao direta
do header de DECODE `stb_image.h` em producao (core+domain+platform+app,
escopo de tools/production_scope.py). Habilitado pela Fatia S1 do
docs/tech/plano-migracao-total.md secao 5: `render2d_sdl.cpp` e
`render2d_gl3.cpp` trocaram `stbi_load()/stbi_image_free()` por
`glintfx::decode_image_file` (commit 9387155, STB-IMAGE-PLATFORM) - a mesma
troca que `app/src/app_icon.cpp` ja tinha feito antes (commit 46a12e3,
STB-IMAGE-APP). Medido no HEAD desta fatia: zero ocorrencia REAL das duas
funcoes na producao inteira - este gate CONGELA esse estado.

Reusa strip_comments() de tools/sdl_layer_ratchet.py (mesmo parser de
caractere, mesmo motivo: os proprios comentarios de app_icon.cpp/
render2d_sdl.cpp/render2d_gl3.cpp MENCIONAM "stbi_load" de proposito, pra
quem ler no futuro entender a troca - um grep ingenuo faria essas linhas de
comentario reprovarem, o oposto do que este gate deveria proteger).

⚠️ FORA DE ESCOPO DESTE GATE (de proposito, NAO e omissao): `stbi_write_png`
e o header `stb_image_write.h` (encode) NAO entram aqui. Eles continuam em
uso REAL em 5 arquivos de producao (title/difficulty/system_menu/
save_load_menu/battle_preview + sdl_window.cpp, ~8 call sites) esperando a
`IMG-ENCODE` (W21 do glintfx, 7 formatos disco+memoria) - incluir aqui
reprovaria o gate no dia em que ele nasce, sem nenhuma fatia real por tras
pra corrigir (o mesmo erro que a Fatia 1 de log-e-relogio evitou
deliberadamente deixando `SDL_Delay` de fora do GATE(log-clock-zero), ver
tools/sdl_log_clock_zero.py). Quando a IMG-ENCODE entregar e o cutover
matar o stbi_write de producao, adicionar o token/header a este gate no
MESMO commit que fizer a migracao - nao antes.

`stb_truetype.h`/`stbtt_*` (font_atlas.cpp, pipelines legadas gl3/sdl) e os
2 headers orfaos ja purgados na Fatia S2 (stb_rect_pack.h,
stb_image_resize2.h, commit 9b1d7df) tambem NAO entram - decode e a unica
categoria fechada por esta fatia.
"""

import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from sdl_layer_ratchet import strip_comments  # noqa: E402
from production_scope import iter_production_files  # noqa: E402

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

# `stbi_image_free` casaria com `\bstbi_load\w*\b`? Nao (prefixos diferentes) -
# listados os dois de proposito, mais o include direto do header de DECODE
# (nao o de encode nem o de truetype - ver docstring acima pro motivo do
# `stb_image.h` exato, sem casar `stb_image_write.h`).
_DEAD_TOKENS = ("stbi_load", "stbi_image_free")
_TOKEN_RE = re.compile(r"\b(?:" + "|".join(_DEAD_TOKENS) + r")\w*\b")
_INCLUDE_RE = re.compile(r'#\s*include\s*[<"]stb_image\.h[>"]')


def find_offenders():
    offenders = []
    for fp in iter_production_files():
        with open(fp, encoding="utf-8", errors="replace") as f:
            stripped = strip_comments(f.read())
        hits = sorted(set(_TOKEN_RE.findall(stripped)))
        if _INCLUDE_RE.search(stripped):
            hits.append('#include "stb_image.h"')
        if hits:
            offenders.append((os.path.relpath(fp, ROOT), hits))
    offenders.sort()
    return offenders


def main() -> int:
    offenders = find_offenders()
    if offenders:
        print(f"GATE(stbi-load-zero): FALHA - {len(offenders)} arquivo(s) ainda "
              "usam stbi_load/stbi_image_free ou incluem stb_image.h direto fora "
              "de comentario (zero-tolerancia, docs/tech/plano-migracao-total.md "
              "secao 5, Fatia S1).")
        for path, hits in offenders:
            print(f"    - {path}: {', '.join(hits)}")
        print("  Caminho certo: glintfx::decode_image_file (v0.25.0+) - ver a "
              "receita em app/src/app_icon.cpp (46a12e3) ou "
              "platform/src/render2d/render2d_{sdl,gl3}.cpp (9387155). Nao "
              "reintroduzir stbi_load/stbi_image_free em producao.")
        print("  stbi_write_png / stb_image_write.h NAO sao deste gate (bloqueado "
              "pela IMG-ENCODE, W21 do glintfx) - se o offender acima for stbi_write, "
              "o problema e outro (script com bug), reporte.")
        return 1
    print("GATE(stbi-load-zero): OK (zero uso real de stbi_load/stbi_image_free "
          "e zero include de stb_image.h em producao).")
    return 0


if __name__ == "__main__":
    sys.exit(main())

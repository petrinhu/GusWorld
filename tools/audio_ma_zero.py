#!/usr/bin/env python3
"""audio_ma_zero.py - GATE(audio-zero) do tools/check.sh.

ZERO-TOLERANCIA (nao ratchet) a `ma_*` (miniaudio DIRETO) e a inclusao de
`<miniaudio...>` em producao (core+domain+platform+app, escopo de
tools/production_scope.py). A migracao ja aconteceu em 2026-07-22 (commit
`d79d880`, `audio_engine.cpp` passou a consumir `<glintfx/audio.hpp>` em vez
de vendorizar miniaudio direto) - este gate nao MIGRA nada, so TRAVA o que
ja foi conquistado (docs/tech/plano-migracao-total.md secao 5, Fatia S3): o
miniaudio ja mudou de dono uma vez (saiu do nosso `third_party/`, voltou por
dentro do glintfx via GLINTFX_MODULE_AUDIO=ON) - sem trava, nada impede que
volte a ser vendorizado direto por engano no meio de uma fatia futura.

⚠️ ARMADILHA DE MEDICAO (por isso o parser, nao grep cru): as ~16 ocorrencias
de `ma_`/`miniaudio` no repo hoje sao TODAS comentario - os proprios
comentarios de audio_engine.hpp/.cpp e maestro_logic.cpp MENCIONAM `ma_sound_*`,
`ma_engine_*` etc. de proposito, documentando o que o glintfx faz por baixo
(ver docstring de tools/sdl_log_clock_zero.py pro mesmo raciocinio aplicado a
SDL_Log/SDL_GetTicksNS - um grep ingenuo reprovaria o gate no dia em que ele
nasce). Reusa strip_comments() de tools/sdl_layer_ratchet.py (mesmo parser de
caractere que sustenta os outros 2 gates de zero-tolerancia desta casa).

`\bma_[a-z0-9_]+\b` casa qualquer identificador `ma_*` (a API C do miniaudio
usa esse prefixo em TUDO - `ma_engine`, `ma_sound`, `ma_result`, etc.); o
`\b` no INICIO evita falso-positivo em substring de palavra comum
(`forma_x`, `sistema_y` teriam `ma_` no meio, nao no inicio de token - a
mesma licao de "grep -i 'eu ' casa dentro de 'seu'" aplicada a identificador
de codigo em vez de prosa).
"""

import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from sdl_layer_ratchet import strip_comments  # noqa: E402
from production_scope import iter_production_files  # noqa: E402

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

_TOKEN_RE = re.compile(r"\bma_[a-z0-9_]+\b")
_INCLUDE_RE = re.compile(r'#\s*include\s*[<"]miniaudio')


def find_offenders():
    offenders = []
    for fp in iter_production_files():
        with open(fp, encoding="utf-8", errors="replace") as f:
            stripped = strip_comments(f.read())
        hits = sorted(set(_TOKEN_RE.findall(stripped)))
        if _INCLUDE_RE.search(stripped):
            hits.append("#include <miniaudio...>")
        if hits:
            offenders.append((os.path.relpath(fp, ROOT), hits))
    offenders.sort()
    return offenders


def main() -> int:
    offenders = find_offenders()
    if offenders:
        print(f"GATE(audio-zero): FALHA - {len(offenders)} arquivo(s) ainda usam "
              "ma_* (miniaudio direto) ou incluem <miniaudio...> fora de comentario "
              "(zero-tolerancia, docs/tech/plano-migracao-total.md secao 5, "
              "Fatia S3; migracao original: commit d79d880, 2026-07-22).")
        for path, hits in offenders:
            print(f"    - {path}: {', '.join(hits)}")
        print("  Caminho certo: glintfx::Audio (GLINTFX_MODULE_AUDIO=ON) via a "
              "fachada platform/src/audio/audio_engine.cpp - nao vendorizar nem "
              "chamar miniaudio direto em producao. O miniaudio que sobra vive "
              "DENTRO do glintfx (_deps/glintfx-src/glintfx/third_party/miniaudio/), "
              "fora do nosso alcance de include.")
        return 1
    print("GATE(audio-zero): OK (zero uso real de ma_*/miniaudio em producao; "
          "audio servido inteiramente por glintfx::Audio).")
    return 0


if __name__ == "__main__":
    sys.exit(main())

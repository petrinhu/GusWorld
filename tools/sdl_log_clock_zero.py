#!/usr/bin/env python3
"""sdl_log_clock_zero.py - GATE(log-clock-zero) do tools/check.sh.

ZERO-TOLERANCIA (nao ratchet) pras DUAS categorias que a Fatia 1 do
docs/tech/plano-camadas-sdl.md matou de vez em GusEngine/app (producao: src/ +
include/gus/app/, EXCLUINDO app/tests/ e app/tools/ - MESMO escopo/exclusoes de
sdl_layer_ratchet.py, motivo identico): `SDL_Log` (~30 call sites antes desta
fatia, todos substituidos por glintfx::log/log_info/log_warn/log_error, FW-LOG)
e `SDL_GetTicksNS` (~17 call sites, substituidos por glintfx::monotonic_now_ns,
FW-CLOCK). A decisao ja foi tomada e nao ha transicao a acomodar (ao contrario
do ratchet por-arquivo, que ainda cobre categorias NAO fechadas) - por isso
zero-tolerancia direta, sem teto numerico: qualquer ocorrencia REAL (fora de
comentario/string) reprova na hora.

Reusa strip_comments/PRODUCTION_DIRS de sdl_layer_ratchet.py (mesmo parser de
caractere, mesmo motivo: bash+grep cru conta ocorrencia em comentario como uso
real - e a doc-comment de cada call site migrado MENCIONA o nome antigo de
proposito, pra quem ler no futuro entender a troca. Um grep ingenuo faria
exatamente essas linhas de comentario reprovarem, o oposto do que este gate
deveria proteger).

FORA DE ESCOPO DESTE GATE (de proposito, NAO e omissao): `SDL_Delay` continua
em uso real de producao (npc_dialogue_loop_gl.cpp, battle_preview.cpp,
system_menu_loop.cpp) - o plano (secao 8) cita as 3 categorias juntas como alvo
eventual da mesma zero-tolerancia, mas SDL_Delay NAO foi pedido nem migrado
nesta fatia (o glintfx nao entregou um FW-SLEEP/equivalente) - incluir aqui
faria este gate reprovar sempre, sem nenhuma fatia real pra corrigir. Quando
uma fatia futura matar SDL_Delay tambem, adicionar o token a `_DEAD_TOKENS`
abaixo no MESMO commit que fizer a migracao.
"""

import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from sdl_layer_ratchet import PRODUCTION_DIRS, strip_comments  # noqa: E402

# As categorias FECHADAS por esta fatia (log-e-relogio, 2026-07-30). Ver o
# docstring acima pro porque SDL_Delay NAO entra ainda.
_DEAD_TOKENS = ("SDL_Log", "SDL_GetTicksNS")
_TOKEN_RE = re.compile(r"\b(?:" + "|".join(_DEAD_TOKENS) + r")\b")


def find_offenders():
    offenders = []
    for base in PRODUCTION_DIRS:
        if not os.path.isdir(base):
            continue
        for dirpath, _dirnames, filenames in os.walk(base):
            for fn in filenames:
                if not (fn.endswith(".cpp") or fn.endswith(".hpp")):
                    continue
                fp = os.path.join(dirpath, fn)
                with open(fp, encoding="utf-8", errors="replace") as f:
                    stripped = strip_comments(f.read())
                matches = sorted(set(_TOKEN_RE.findall(stripped)))
                if matches:
                    offenders.append((os.path.relpath(fp, os.path.dirname(
                        os.path.dirname(os.path.abspath(__file__)))), matches))
    offenders.sort()
    return offenders


def main() -> int:
    offenders = find_offenders()
    if offenders:
        print(f"GATE(log-clock-zero): FALHA - {len(offenders)} arquivo(s) ainda "
              f"chamam {'/'.join(_DEAD_TOKENS)} fora de comentario (zero-tolerancia, "
              "docs/tech/plano-camadas-sdl.md secao 8).")
        for path, matches in offenders:
            print(f"    - {path}: {', '.join(matches)}")
        print("  Caminho certo: glintfx::log/log_info/log_warn/log_error (FW-LOG) e "
              "glintfx::monotonic_now_ns() (FW-CLOCK) - nao reintroduzir SDL_Log nem "
              "SDL_GetTicksNS em producao (app/src, app/include/gus/app).")
        return 1
    print(f"GATE(log-clock-zero): OK (zero uso real de {'/'.join(_DEAD_TOKENS)} em "
          "producao).")
    return 0


if __name__ == "__main__":
    sys.exit(main())

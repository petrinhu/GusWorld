#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
"""gl3_readbackbuffer_zero.py - GATE(gl3-readbackbuffer-zero) do tools/check.sh.

ZERO-TOLERANCIA (nao ratchet, com UMA excecao documentada) pra
gus::platform::rmlui::gl3_read_backbuffer_rgba (leitura crua de backbuffer via
glad) em GusEngine/app de PRODUCAO (src/ + include/gus/app/, EXCLUINDO
app/tests/ e app/tools/ - MESMO escopo/exclusoes de sdl_layer_ratchet.py,
motivo identico: app/tests/ e um binario ctest a parte, app/tools/ compila
standalone contra os .a, nenhum dos dois linka em gusengine_app).

FRAMEGRAB-7-SITIOS (2026-07-30): migrou os 7 sitios de captura de
gl3_read_backbuffer_rgba pra glintfx::UiLayer::capture_frame() -
battle_preview.cpp (4x: hover/anim/sprite-selftest + SMOKE VISUAL),
difficulty_menu_loop.cpp, title_menu_loop.cpp, save_load_menu_loop.cpp (1x
cada). Este gate CONGELA o resultado: qualquer chamada NOVA a
gl3_read_backbuffer_rgba em producao reprova na hora, exceto o unico site
documentado em ALLOWLIST abaixo.

ALLOWLIST (motivo por arquivo, nao generico): sdl_window.cpp:
capture_frame_to_png() roda ANTES de qualquer glintfx::UiLayer existir
naquele instante (o metodo e chamado no boot, antes da 1a tela criar sua
UiLayer) - capture_frame() e metodo de instancia de UiLayer, nao ha
instancia viva pra chamar. Sinalizado ao glintfx pelo bus (pedido de captura
livre de UiLayer, ver DOC-ANCORAS-MORTAS/FRAMEGRAB-7-SITIOS); ate a lacuna
fechar, este e o UNICO site fora do congelamento.

Reusa strip_comments/PRODUCTION_DIRS de sdl_layer_ratchet.py (mesmo parser de
caractere - bash+grep cru conta ocorrencia em comentario/doc-comment como uso
real, e cada site migrado TEM comentario mencionando o nome antigo de
proposito, pra quem ler no futuro entender a troca. Um grep ingenuo faria
essas linhas de comentario reprovarem, o oposto do que este gate deveria
proteger).
"""

import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from sdl_layer_ratchet import iter_production_files, strip_comments  # noqa: E402

# Raiz do repo = pai de tools/ (resolve symlink do proprio script).
_SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
_ROOT = os.path.dirname(_SCRIPT_DIR)

_TOKEN = "gl3_read_backbuffer_rgba"
_TOKEN_RE = re.compile(r"\b" + _TOKEN + r"\b")

# Caminho relativo a raiz do repo -> motivo (documentado tambem no docstring
# acima). Nao e uma lista de "arquivos a ignorar por conveniencia" - cada
# entrada e um site que PROVADAMENTE nao pode chamar capture_frame() hoje
# (sem UiLayer viva no instante da chamada).
ALLOWLIST = {
    "GusEngine/app/src/sdl_window.cpp": (
        "capture_frame_to_png() roda antes de qualquer glintfx::UiLayer "
        "existir (boot, antes da 1a tela); capture_frame() e metodo de "
        "instancia, sem instancia viva pra chamar. Lacuna sinalizada ao "
        "glintfx pelo bus (FRAMEGRAB-7-SITIOS)."
    ),
}


def find_offenders():
    # GATES-HARDEN (2026-08-06): iter_production_files() em vez do os.walk
    # proprio - o walk deixava `app/main.cpp` de fora (ver sdl_layer_ratchet.py).
    offenders = []
    for fp in iter_production_files():
        rel = os.path.relpath(fp, _ROOT)
        if rel in ALLOWLIST:
            continue
        with open(fp, encoding="utf-8", errors="replace") as f:
            stripped = strip_comments(f.read())
        count = len(_TOKEN_RE.findall(stripped))
        if count:
            offenders.append((rel, count))
    offenders.sort()
    return offenders


def check_allowlist_still_valid():
    """A allowlist so vale enquanto o arquivo (a) existir e (b) ainda tiver a
    chamada real - senao esta encobrindo um site que ja migrou ou sumiu, e
    devia sair da lista no mesmo commit."""
    stale = []
    for rel in sorted(ALLOWLIST):
        fp = os.path.join(_ROOT, rel)
        if not os.path.isfile(fp):
            stale.append((rel, "arquivo nao existe mais"))
            continue
        with open(fp, encoding="utf-8", errors="replace") as f:
            stripped = strip_comments(f.read())
        if not _TOKEN_RE.search(stripped):
            stale.append((rel, "arquivo nao chama mais " + _TOKEN))
    return stale


def main() -> int:
    offenders = find_offenders()
    stale = check_allowlist_still_valid()
    n_scanned = sum(1 for _ in iter_production_files())
    if n_scanned == 0:
        print("GATE(gl3-readbackbuffer-zero): FALHA - escopo colapsou pra ZERO "
              "arquivo de producao. Sem varrer nada, este gate nao pode dizer OK.")
        return 1
    ok = True

    if offenders:
        ok = False
        print(f"GATE(gl3-readbackbuffer-zero): FALHA - {len(offenders)} arquivo(s) de "
              f"producao fora da ALLOWLIST ainda chamam {_TOKEN} "
              "(zero-tolerancia pos FRAMEGRAB-7-SITIOS).")
        for path, count in offenders:
            print(f"    - {path}: {count} chamada(s)")
        print("  Caminho certo: glintfx::UiLayer::capture_frame() (ver "
              "battle_preview.cpp/difficulty_menu_loop.cpp/title_menu_loop.cpp/"
              "save_load_menu_loop.cpp). Se o site NAO pode migrar (sem UiLayer "
              "viva no instante da chamada), documente o motivo e adicione a "
              "ALLOWLIST deste script.")

    if stale:
        ok = False
        print(f"GATE(gl3-readbackbuffer-zero): FALHA - {len(stale)} entrada(s) da "
              "ALLOWLIST estao desatualizadas (arraste de excecao morta):")
        for path, reason in stale:
            print(f"    - {path}: {reason}")
        print("  Remova a entrada da ALLOWLIST (gl3_readbackbuffer_zero.py) no "
              "mesmo commit que migrou/apagou o site.")

    if not ok:
        return 1

    print(f"GATE(gl3-readbackbuffer-zero): OK (zero chamada de {_TOKEN} em "
          f"{n_scanned} arquivo(s) de producao varrido(s), fora da allowlist de "
          f"{len(ALLOWLIST)} site; allowlist auditada, sem entrada desatualizada).")
    return 0


if __name__ == "__main__":
    sys.exit(main())

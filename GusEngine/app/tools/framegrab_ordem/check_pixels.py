#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
"""FRAMEGRAB-ORDEM - verificador INDEPENDENTE das capturas do framegrab_ordem_probe.

Por que separado do probe: o programa que produz o artefato nao e testemunha confiavel
sobre ele. Este script le os .rgba crus do disco (o conteudo exato que
glintfx::UiLayer::capture_frame devolveu, sem passar por codec) e responde as 4 perguntas
do experimento por contagem de pixel.

O criterio de cada pergunta esta escrito ao lado da resposta - nada de "retornou ok",
nada de "as duas imagens diferem".

Em vez de PROCURAR a cor da UI (busca dirigida acha o que voce suspeita), cada regiao tem
seu HISTOGRAMA COMPLETO impresso: a enumeracao mostra o que ESTA la, nao so se o que eu
esperava esta la.

Uso:
  ./check_pixels.py [DIR_DOS_ARTEFATOS]
Saida: relatorio + exit 0 se a hipotese e CONFIRMADA por pixel, 1 se REFUTADA/inconclusiva.
"""

import os
import sys

import numpy as np

W, H = 960, 540

# MESMA geometria do framegrab_ordem_probe.cpp (coordenadas de tela, origem topo-esquerda).
YELLOW_RECT = (40, 40, 160, 80)     # x, y, w, h - fora do painel
GREEN_RECT = (300, 180, 360, 180)   # sob o painel
PANEL_RECT = (280, 160, 400, 220)   # o div da UI

BG = (0, 0, 255)
YELLOW = (255, 255, 0)
GREEN = (0, 255, 0)
MAGENTA = (255, 0, 255)
BLACK = (0, 0, 0)


def load(path):
    raw = np.fromfile(path, dtype=np.uint8)
    expect = W * H * 4
    if raw.size != expect:
        sys.exit(f"FAIL: {path} tem {raw.size} bytes, esperado {expect}")
    return raw.reshape(H, W, 4)[:, :, :3]  # descarta o alpha sintetico (255)


def sub(img, rect):
    x, y, w, h = rect
    return img[y:y + h, x:x + w]


def count_exact(img, rgb):
    return int(np.count_nonzero(np.all(img == np.array(rgb, dtype=np.uint8), axis=-1)))


def histogram(img, top=6):
    flat = img.reshape(-1, 3)
    colors, counts = np.unique(flat, axis=0, return_counts=True)
    order = np.argsort(-counts)
    out = []
    for i in order[:top]:
        out.append((tuple(int(v) for v in colors[i]), int(counts[i])))
    return out, len(colors)


def outside_panel_mask():
    m = np.ones((H, W), dtype=bool)
    x, y, w, h = PANEL_RECT
    m[y:y + h, x:x + w] = False
    return m


def fmt_hist(img, label):
    hist, n_distinct = histogram(img)
    parts = " | ".join(f"{c}={n}" for c, n in hist)
    return f"    {label}: {n_distinct} cor(es) distinta(s); top: {parts}"


def main():
    d = sys.argv[1] if len(sys.argv) > 1 else "/var/tmp/builds/claude-1000/framegrab-ordem/out"
    before = load(os.path.join(d, "before.rgba"))
    after = load(os.path.join(d, "after.rgba"))

    checks = []  # (nome, criterio, valor_medido, passou)

    print("=" * 78)
    print("FRAMEGRAB-ORDEM - verificacao independente por pixel")
    print(f"artefatos: {d}")
    print("=" * 78)

    # ---------------------------------------------------------------- ENUMERACAO
    print("\n[0] ENUMERACAO das regioes (o que ESTA la, nao o que eu procurava)")
    print("  ANTES do render():")
    print(fmt_hist(before, "imagem inteira"))
    print(fmt_hist(sub(before, GREEN_RECT), "regiao SOB o painel"))
    print(fmt_hist(sub(before, YELLOW_RECT), "bloco amarelo"))
    print(fmt_hist(sub(before, PANEL_RECT), "retangulo do painel"))
    print("  DEPOIS do render():")
    print(fmt_hist(after, "imagem inteira"))
    print(fmt_hist(sub(after, GREEN_RECT), "regiao SOB o painel"))
    print(fmt_hist(sub(after, YELLOW_RECT), "bloco amarelo"))
    print(fmt_hist(sub(after, PANEL_RECT), "retangulo do painel"))

    # ---------------------------------------------------- P1: o antes NAO tem a UI
    # Criterio: ZERO pixel da cor da UI na imagem INTEIRA do antes. Nao "na regiao do
    # painel" - na imagem inteira, senao a resposta seria so sobre onde eu olhei.
    mag_before_full = count_exact(before, MAGENTA)
    checks.append(("P1 o ANTES nao contem a UI",
                   "magenta na imagem INTEIRA == 0",
                   mag_before_full, mag_before_full == 0))

    # -------------------------------------- P1b: nem update() sozinho compoe a UI
    # Sem isto, o experimento provaria "antes de update()+render()" e eu estaria
    # atribuindo a render() um efeito que poderia ser de update().
    mid_path = os.path.join(d, "mid_after_update.rgba")
    if os.path.exists(mid_path):
        mid = load(mid_path)
        mag_mid = count_exact(mid, MAGENTA)
        mid_equals_before = bool(np.array_equal(mid, before))
        checks.append(("P1b update() sozinho nao compoe",
                       "magenta apos update() e ANTES de render() == 0",
                       mag_mid, mag_mid == 0))
        checks.append(("P1c a captura do meio == a de antes",
                       "buffer identico byte a byte (so render() muda o FBO)",
                       mid_equals_before, mid_equals_before))
    else:
        print(f"\nAVISO: {mid_path} ausente - P1b/P1c nao medidos")

    # ---------------------------------------------------- P2: o depois TEM a UI
    panel_after = sub(after, PANEL_RECT)
    panel_px = PANEL_RECT[2] * PANEL_RECT[3]
    mag_after_panel = count_exact(panel_after, MAGENTA)
    checks.append(("P2 o DEPOIS contem a UI",
                   f"magenta no retangulo do painel >= 99% de {panel_px}",
                   f"{mag_after_panel} ({100.0 * mag_after_panel / panel_px:.2f}%)",
                   mag_after_panel >= 0.99 * panel_px))

    # E o inverso, na MESMA regiao: o conteudo de jogo que estava ali sumiu.
    green_after_under = count_exact(sub(after, GREEN_RECT), GREEN)
    checks.append(("P2b a UI COBRIU o jogo",
                   "verde na regiao SOB o painel, no depois == 0",
                   green_after_under, green_after_under == 0))

    # ------------------------------- P3: o antes NAO esta vazio/preto - tem o JOGO
    # Esta e a pergunta que separa o teste de um falso positivo. Uma imagem toda preta
    # tambem "nao contem a UI". O criterio nao e "nao e preta": e que o conteudo de JOGO
    # esteja 100% presente EXATAMENTE onde a UI estaria.
    green_px = GREEN_RECT[2] * GREEN_RECT[3]
    green_before_under = count_exact(sub(before, GREEN_RECT), GREEN)
    checks.append(("P3 o ANTES contem o JOGO sob a UI",
                   f"verde na regiao SOB o painel == {green_px} (100%)",
                   green_before_under, green_before_under == green_px))

    yellow_px = YELLOW_RECT[2] * YELLOW_RECT[3]
    yellow_before = count_exact(sub(before, YELLOW_RECT), YELLOW)
    checks.append(("P3b o ANTES contem o JOGO fora da UI",
                   f"amarelo no bloco amarelo == {yellow_px} (100%)",
                   yellow_before, yellow_before == yellow_px))

    bg_before = count_exact(before, BG)
    bg_expected = W * H - green_px - yellow_px
    checks.append(("P3c o fundo do jogo esta inteiro",
                   f"azul no antes == {bg_expected} (tela - os 2 blocos)",
                   bg_before, bg_before == bg_expected))

    black_before = count_exact(before, BLACK)
    checks.append(("P3d o ANTES nao e preto/vazio",
                   "pixel preto puro no antes == 0",
                   black_before, black_before == 0))

    # Orientacao: o bloco amarelo e assimetrico na vertical. Se a imagem viesse invertida,
    # o amarelo apareceria na regiao espelhada.
    yx, yy, yw, yh = YELLOW_RECT
    mirrored = (yx, H - (yy + yh), yw, yh)
    yellow_mirror = count_exact(sub(before, mirrored), YELLOW)
    checks.append(("P3e orientacao (linha 0 = topo)",
                   "amarelo na regiao ESPELHADA == 0",
                   yellow_mirror, yellow_mirror == 0))

    # ------------------- P4: nao e imagem velha/trocada - o resto do quadro e identico
    mask_out = outside_panel_mask()
    diff_any = np.any(before != after, axis=-1)
    diff_outside = int(np.count_nonzero(diff_any & mask_out))
    diff_inside = int(np.count_nonzero(diff_any & ~mask_out))
    checks.append(("P4 antes/depois sao o MESMO quadro",
                   "pixels diferentes FORA do retangulo do painel == 0",
                   diff_outside, diff_outside == 0))
    checks.append(("P4b a unica mudanca e a UI",
                   f"pixels diferentes DENTRO do painel >= 99% de {panel_px}",
                   f"{diff_inside} ({100.0 * diff_inside / panel_px:.2f}%)",
                   diff_inside >= 0.99 * panel_px))

    # ------------------------------------------------ FASE B (observacao, sem criterio)
    stale_path = os.path.join(d, "stale_noclear.rgba")
    stale_note = None
    if os.path.exists(stale_path):
        stale = load(stale_path)
        mag_stale = count_exact(stale, MAGENTA)
        stale_note = (mag_stale, histogram(stale)[0])

    # ------------------------------------------------------------------- RELATORIO
    print("\n[1] CRITERIOS")
    all_ok = True
    for name, crit, val, ok in checks:
        all_ok = all_ok and ok
        print(f"  {'PASS' if ok else 'FALHA'}  {name}")
        print(f"        criterio: {crit}")
        print(f"        medido  : {val}")

    if stale_note is not None:
        print("\n[2] FASE B (observacao, nao criterio) - quadro sem clear e sem desenho")
        print(f"  magenta (UI) no back buffer nao-limpo: {stale_note[0]}")
        print("  top cores: " + " | ".join(f"{c}={n}" for c, n in stale_note[1]))

    print("\n" + "=" * 78)
    if all_ok:
        print("VEREDITO: CONFIRMADA - capture_frame() antes de render() devolve o quadro")
        print("do host COM o conteudo de jogo e SEM a UI; depois de render(), COM a UI.")
    else:
        print("VEREDITO: NAO CONFIRMADA - ver as linhas FALHA acima.")
    print("=" * 78)
    return 0 if all_ok else 1


if __name__ == "__main__":
    sys.exit(main())

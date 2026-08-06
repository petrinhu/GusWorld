#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
"""Gera os PDFs de "perfil canonico" de personagem, no template do Brunus.

Fonte: os contos canonicos em docs/narrative/characters/*-conto.md (formato
`# Titulo` + `*dedicatoria*` + `---` + prosa) e o sprite do personagem.
Pipeline: markdown -> HTML5/CSS -> Chromium headless --print-to-pdf.

O template NAO foi chutado: as cores, o gradiente e as reguas foram AMOSTRADAS do
PDF original `Brunus_Vetorial_Solveckt.pdf` (pdftoppm 150/200dpi + `magick
%[pixel:p{x,y}]`), por isso os valores aparecem literais aqui com o registro de onde
sairam. Se um dia o template mudar, re-amostrar em vez de adivinhar.

Uso:
    python3 tools/build_perfil_pdf.py              # gera todos
    python3 tools/build_perfil_pdf.py gus yakov    # gera so os citados

Requisitos: chromium-browser, ImageMagick (`magick`) para o pre-processo do sprite
(remocao de fundo branco), pdfinfo para a conferencia. Nenhum acesso a rede.
"""
import base64
import html
import pathlib
import re
import subprocess
import sys

RAIZ = pathlib.Path(__file__).resolve().parent.parent
CONTOS = RAIZ / "docs/narrative/characters"
SPRITES = RAIZ / "resources/sprites/personagens_inspirados"
TMP = pathlib.Path("/var/tmp/gusworld_perfil_pdf")

# slug -> conto, sprite de origem, pasta destino, nome do PDF, nome canonico,
#         epiteto de rodape, altura do sprite em mm
FICHAS = {
    "gus": dict(
        conto="gus-conto.md", sprite="gus/gus_conceito.png", pasta="gus",
        pdf="Gustaf_Gus_Vector_Tavus_Vance.pdf",
        nome='Gustaf "Gus" Vector Tavus Vance',
        epiteto="o menino que resolve por lógica", altura_mm=62),
    "pyotor": dict(
        conto="pyotor-vance-conto.md", sprite="pyotor_vance/front.png",
        pasta="pyotor_vance", pdf="Pyotor_Vance.pdf", nome="Pyotor Vance",
        epiteto="médico itinerante que cura uma pessoa de cada vez", altura_mm=62),
    "yakov": dict(
        conto="yakov-vance-conto.md", sprite="yakov/yakov.png", pasta="yakov",
        pdf="Yakov_Vance.pdf", nome="Yakov Vance",
        epiteto="engenheiro e geólogo que escuta a terra antes de cavar",
        altura_mm=62),
    # O do Brunus JA EXISTE e nao deve ser sobrescrito (ordem do lider 2026-07-25).
    # Fica aqui comentado como registro de que o template veio dele:
    # "brunus": conto="brunus-vetorial-conto.md", sprite="brunus_vetorial/brunus_chibi_3x.png"
}

CSS = """
@page { size: A4; margin: 0; }
html, body { margin: 0; padding: 0; }
body {
  background: #f4f1ea;                 /* corpo, amostrado: srgb(244,241,234) */
  font-family: Georgia, "DejaVu Serif", serif;
  color: #2c2a27;
  -webkit-print-color-adjust: exact;
  print-color-adjust: exact;
}
.hero {
  /* gradiente amostrado na coluna x=30 do original: topo srgb(16,19,32) -> base
     srgb(28,36,57); a divisoria e o cyan srgb(34,211,238) */
  background: linear-gradient(180deg, #101320 0%, #1c2439 100%);
  border-bottom: 2.2pt solid #22d3ee;
  text-align: center;
  padding: 16mm 18mm 9mm 18mm;
}
.hero img {
  height: SPRITE_Hmm; width: auto;
  image-rendering: pixelated;          /* sprite: sem interpolacao borrada */
  display: block; margin: 0 auto 9mm auto;
}
.kicker {
  font-family: "DejaVu Sans", Verdana, sans-serif;
  font-size: 7.6pt; letter-spacing: 0.30em; text-transform: uppercase;
  color: #67e8f9; margin: 0 0 3.5mm 0;
}
h1 {
  font-weight: 400; font-size: 25pt; line-height: 1.15;
  color: #f6f3ec; margin: 0 0 3mm 0; letter-spacing: 0.005em;
}
.nome-hero { font-style: italic; font-size: 11.5pt; color: #b6aca0; margin: 0; }
.corpo { padding: 11mm 20mm 14mm 20mm; }
/* .corpo p (classe+elemento) tem especificidade MAIOR que .dedicatoria (so classe):
   sem o seletor casado abaixo, o justify/text-indent do paragrafo vence e joga a
   dedicatoria pra esquerda com recuo. */
.corpo p.dedicatoria {
  font-style: italic; font-size: 10.5pt; text-align: center; text-indent: 0;
  color: #624f3c;                      /* sepia amostrado: srgb(98,79,60) */
  margin: 0 0 9mm 0;
}
.regua { border: 0; border-top: 0.6pt solid #cfc6b6; width: 58%; margin: 0 auto 8mm auto; }
.corpo p {
  text-align: justify; text-indent: 5.2mm; font-size: 10.4pt; line-height: 1.62;
  margin: 0; orphans: 2; widows: 2; hyphens: auto;
}
.corpo p:first-of-type { text-indent: 0; }
/* Rodape: no original NAO e italico serif, e MONOSPACE espacado, com regua clara
   acima e o nome de quem inspirou em NEGRITO azul-petroleo (amostrado: texto
   srgb(128,115,93), negrito srgb(44,88,116), regua srgb(235,230,221)). */
.rodape {
  margin-top: 13mm; padding-top: 6mm; border-top: 0.6pt solid #ebe6dd;
  text-align: center;
  font-family: "DejaVu Sans Mono", "Liberation Mono", monospace;
  font-size: 8.1pt; letter-spacing: 0.055em; line-height: 1.95;
  color: #80735d; page-break-inside: avoid;
}
.rodape .selo { display: block; }
.rodape b { color: #2c5874; font-weight: 700; }
"""

PAGINA = """<!doctype html>
<html lang="pt-br"><head><meta charset="utf-8"><title>{titulo}</title>
<style>{css}</style></head><body>
<header class="hero">
  <img src="data:image/png;base64,{sprite_b64}" alt="{nome_plain}">
  <p class="kicker">GusWorld &middot; Personagem Can&ocirc;nico</p>
  <h1>{titulo}</h1>
  <p class="nome-hero">{nome}</p>
</header>
<main class="corpo">
  <p class="dedicatoria">{dedicatoria}</p>
  <hr class="regua">
  {paragrafos}
  <div class="rodape">
    <span class="selo">{nome} &middot; {epiteto}</span>
    {inspiracao}GusWorld &middot; 2026
  </div>
</main>
</body></html>
"""


def le_conto(caminho):
    """Formato canonico (igual brunus-vetorial-conto.md): `# Titulo`, `*dedic*`,
    `---`, prosa em paragrafos separados por linha vazia."""
    txt = caminho.read_text(encoding="utf-8")
    m_tit = re.search(r"^#\s+(.+)$", txt, re.M)
    m_ded = re.search(r"^\*(.+)\*$", txt, re.M)
    if not m_tit:
        raise ValueError(f"{caminho.name}: sem `# Titulo` na primeira linha")
    corpo = txt[m_ded.end():] if m_ded else txt[m_tit.end():]
    corpo = re.sub(r"^\s*(---|\*\*\*|___)\s*$", "", corpo, flags=re.M).strip()
    paras = [p.strip().replace("\n", " ") for p in corpo.split("\n\n") if p.strip()]
    return m_tit.group(1).strip(), (m_ded.group(1).strip() if m_ded else ""), paras


def tipografia(s):
    """Escapa HTML e converte aspas retas em curvas (o original usa curvas)."""
    s = html.escape(s, quote=False)
    out, aberta = [], True
    for ch in s:
        if ch == '"':
            out.append("&ldquo;" if aberta else "&rdquo;")
            aberta = not aberta
        else:
            out.append(ch)
    return "".join(out)


def prepara_sprite(origem, destino):
    """Sprite tem que ficar TRANSPARENTE pra assentar no hero escuro. Alguns vem com
    fundo branco opaco: floodfill a partir da borda (nao troca-branco-global, que
    comeria dentes/reflexos internos). Reduz pra ~900px de altura: 62mm a 300dpi
    pede ~732px, entao 900 cobre impressao com folga sem inflar o PDF."""
    alfa_min = subprocess.run(
        ["magick", str(origem), "-format", "%[fx:minima.a]", "info:"],
        capture_output=True, text=True).stdout.strip()
    cmd = ["magick", str(origem)]
    if alfa_min != "0":                       # opaco -> remover o fundo
        cmd += ["-alpha", "set", "-bordercolor", "white", "-border", "1",
                "-fuzz", "10%", "-fill", "none", "-draw", "alpha 0,0 floodfill",
                "-shave", "1x1", "-trim", "+repage"]
    cmd += ["-resize", "x900>", "-strip", str(destino)]
    subprocess.run(cmd, check=True, capture_output=True)


def main():
    alvos = sys.argv[1:] or list(FICHAS)
    desconhecidos = [a for a in alvos if a not in FICHAS]
    if desconhecidos:
        sys.exit(f"slug desconhecido: {', '.join(desconhecidos)}\n"
                 f"validos: {', '.join(FICHAS)}")
    TMP.mkdir(parents=True, exist_ok=True)
    falhas = []

    for slug in alvos:
        f = FICHAS[slug]
        conto, sprite_src = CONTOS / f["conto"], SPRITES / f["sprite"]
        for p in (conto, sprite_src):
            if not p.exists():
                falhas.append(f"{slug}: nao existe {p}")
        if falhas:
            continue

        titulo, dedic, paras = le_conto(conto)
        sprite = TMP / f"{slug}_sprite.png"
        prepara_sprite(sprite_src, sprite)

        # o rodape do original repete "inspirado por <nome>" a partir da dedicatoria
        inspirador = dedic[5:].split(",")[0].strip() if dedic.lower().startswith("para ") else ""
        inspiracao = (f"inspirado por <b>{tipografia(inspirador)}</b> &middot; "
                      if inspirador else "")

        pagina = PAGINA.format(
            css=CSS.replace("SPRITE_H", str(f["altura_mm"])),
            sprite_b64=base64.b64encode(sprite.read_bytes()).decode(),
            titulo=tipografia(titulo), nome=tipografia(f["nome"]),
            nome_plain=html.escape(f["nome"].replace('"', "")),
            dedicatoria=tipografia(dedic), epiteto=tipografia(f["epiteto"]),
            inspiracao=inspiracao,
            paragrafos="\n  ".join(f"<p>{tipografia(p)}</p>" for p in paras))
        html_path = TMP / f"{slug}.html"
        html_path.write_text(pagina, encoding="utf-8")

        destino = SPRITES / f["pasta"] / f["pdf"]
        destino.parent.mkdir(parents=True, exist_ok=True)
        proc = subprocess.run(
            ["chromium-browser", "--headless", "--disable-gpu", "--no-sandbox",
             f"--user-data-dir={TMP}/.chromium_{slug}",
             "--no-pdf-header-footer", f"--print-to-pdf={destino}",
             f"file://{html_path}"],
            capture_output=True, text=True, timeout=180)
        if not destino.exists() or destino.stat().st_size < 5000:
            falhas.append(f"{slug}: chromium nao gerou o PDF\n{proc.stderr[-400:]}")
            continue
        info = subprocess.run(["pdfinfo", str(destino)], capture_output=True,
                              text=True).stdout
        npag = next((l.split(":")[1].strip() for l in info.split("\n")
                     if l.startswith("Pages")), "?")
        print(f"OK {slug}: {destino.relative_to(RAIZ)} "
              f"({destino.stat().st_size // 1024} KB, {npag} pag, {len(paras)} paragrafos)")

    if falhas:
        print("\nFALHAS:", *falhas, sep="\n - ")
        sys.exit(1)


if __name__ == "__main__":
    main()

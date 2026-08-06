#!/usr/bin/env python3
"""decode_image_file_zero.py - GATE(decode-image-file-zero) do tools/check.sh.

ZERO-TOLERANCIA (nao ratchet, allowlist VAZIA) a `glintfx::decode_image_file`
em producao (core+domain+platform+app, escopo de tools/production_scope.py).
O substituto e `glintfx::decode_png_file` (glintfx v0.30.0+,
glintfx/include/glintfx/image.hpp:691) - MESMO tipo de retorno
(DecodedImagePixels), MESMO contrato fail-high (`ok == false` e o unico sinal
de falha), MESMO alpha STRAIGHT nao-premultiplicado, MESMO `noexcept`: e
drop-in, uma palavra de diferenca no call site.

POR QUE (PNG-DECODE-ADOPT, 2026-08-06): `decode_image_file()` faz dispatch
PNG/JPG/TGA pelo sniffing do stb_image, que e FROUXO na perna TGA (o formato
nao tem numero magico - um `image_type` plausivel mais um `bits_per_pixel`
plausivel ja bastam, e ele NAO exige que o dado de pixel prometido pelo
header exista de fato). Medido pela auditoria independente da W1, e
reconferido nesta fatia: um header TGA de 18 bytes declarando 4096x4096 a 32
bpp, seguido de 64 bytes de ruido - 82 bytes no total, com nome `.png` -
decodifica "COM SUCESSO" (`ok == true`!) numa imagem RGBA8 4096x4096 toda
alpha-zero, 67.108.864 bytes alocados a partir de 82 bytes de entrada:
amplificacao de ~818.000x, a partir de arquivo. Nao e crash nem `ok ==
false`: e um handle VALIDO, entao um chamador que so testa "o handle e
valido" nao percebe nada. `decode_png_file()` checa os 8 bytes da assinatura
PNG (RFC 2083/ISO 15948) ANTES de o buffer alcancar o stb_image, e RECUSA o
forjado (provado por teste: GusEngine/platform/tests/decode_png_file_test.cpp).

Adotar `decode_png_file()` NAO e contornar lacuna do glintfx (o que a lei da
casa proibe, CLAUDE.md 2026-07-29) - e o OPOSTO: e adotar a API que a lib ja
expoe exatamente pra este fim, no pin que ja usamos. O proprio comentario
deles diz que a funcao existe "PRECISAMENTE pra dar a um chamador que so quer
PNG uma forma de fechar este buraco especifico no proprio call site dele".

A TROCA E SEGURA porque o pipeline de asset do jogo e SO-PNG, e isso foi
MEDIDO (nao presumido) em 2026-08-06:
  - resources/ + assets/ no disco: 1283 `.png`, 4 `.gif`, ZERO tga/bmp/jpg;
  - os 4 `.gif` tem ZERO referencia em codigo (`git grep .gif` no GusEngine
    volta vazio) - e `decode_image_file` nem decodifica GIF, entao nao ha
    nem caminho hipotetico perdido;
  - literais de extensao de imagem em producao, fora de comentario: 103 de
    103 sao `.png` (`asset_paths.hpp`: 18 de 18).
Se algum dia o jogo passar a carregar JPG/TGA de verdade, este gate reprova
na hora - e ai a decisao (voltar ao dispatch geral pra AQUELE call site, ou
pedir uma irma so-JPG ao glintfx) e explicita, com o custo de seguranca na
mesa, em vez de acontecer por acidente.

⚠️ FORA DE ESCOPO DESTE GATE (de proposito, NAO e omissao):
  - `decode_image_memory` (o irmao de buffer-em-memoria, image.hpp:226) NAO
    entra: o glintfx NAO expoe `decode_png_memory`, entao nao ha substituto
    pra oferecer - proibir seria fechar um caminho sem alternativa. Hoje ele
    tem ZERO uso em producao, entao incluir tambem nao protegeria nada. Se
    algum dia surgir uso real E o glintfx entregar a irma so-PNG de memoria,
    somar o token AQUI no MESMO commit da migracao, nao antes. (Mesma
    disciplina que deixou `stbi_write` de fora do GATE(stbi-load-zero) e
    `SDL_Delay` de fora do GATE(log-clock-zero): gate nao nasce vermelho sem
    fatia real por tras.)
  - `Draw2d::load_texture` do glintfx continua multi-formato POR DENTRO -
    lacuna DELES, ja reportada pelo bus. Nao e nossa pra contornar, e nao da
    pra ver daqui por token.

Reusa strip_comments() de tools/sdl_layer_ratchet.py (mesmo parser de
caractere, mesmo motivo do irmao stb_image_zero.py: os proprios comentarios
de app_icon.cpp/render2d_sdl.cpp/render2d_gl3.cpp MENCIONAM
"decode_image_file" de proposito, explicando a troca pra quem ler no futuro -
um grep ingenuo faria essas linhas reprovarem, o oposto do que este gate
deveria proteger).
"""

import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from sdl_layer_ratchet import strip_comments  # noqa: E402
from production_scope import iter_production_files  # noqa: E402

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

# `\b` nas duas pontas: nao casa um identificador MAIOR que apenas contenha o
# nome (ex.: um futuro `decode_image_file_ex`), que seria outra funcao e outra
# decisao. Casa `glintfx::decode_image_file(...)` e o nome nu (using-declaration).
_TOKEN_RE = re.compile(r"\bdecode_image_file\b")

# Sentinela de ESCOPO (regra (a) do desenho de gate da casa: gate que nao MEDE
# nao aprova). Sem isto, um rename de diretorio ou um bug em production_scope.py
# faria iter_production_files() render ZERO arquivo - e o gate imprimiria "OK"
# com folego total, um falso verde silencioso. Medido em 2026-08-06: 326 arquivos
# de producao. O piso e folgado de proposito (so pega colapso de escopo, nunca
# remocao legitima de arquivo).
_MIN_SCANNED_FILES = 200


def find_offenders():
    offenders = []
    scanned = 0
    for fp in iter_production_files():
        scanned += 1
        with open(fp, encoding="utf-8", errors="replace") as f:
            stripped = strip_comments(f.read())
        hits = len(_TOKEN_RE.findall(stripped))
        if hits:
            offenders.append((os.path.relpath(fp, ROOT), hits))
    offenders.sort()
    return offenders, scanned


def main() -> int:
    offenders, scanned = find_offenders()

    if scanned < _MIN_SCANNED_FILES:
        print(f"GATE(decode-image-file-zero): FALHA - escopo de producao "
              f"colapsou ({scanned} arquivo(s) varrido(s), piso "
              f"{_MIN_SCANNED_FILES}). O gate NAO mediu nada; um 'OK' aqui "
              "seria falso verde. Conferir tools/production_scope.py (dir "
              "renomeado/movido?).")
        return 1

    if offenders:
        total = sum(n for _, n in offenders)
        print(f"GATE(decode-image-file-zero): FALHA - {len(offenders)} "
              f"arquivo(s) / {total} chamada(s) de glintfx::decode_image_file "
              "em producao fora de comentario (zero-tolerancia, allowlist "
              "vazia, PNG-DECODE-ADOPT).")
        for path, n in offenders:
            print(f"    - {path}: {n} ocorrencia(s)")
        print("  Caminho certo: glintfx::decode_png_file (v0.30.0+, mesmo "
              "DecodedImagePixels, mesmo contrato fail-high, mesmo alpha "
              "straight - drop-in). Motivo: decode_image_file aceita 82 bytes "
              "de TGA forjado com nome .png e aloca 64 MB (amplificacao "
              "~818.000x); decode_png_file recusa pela assinatura PNG. Ver o "
              "docstring deste arquivo e "
              "GusEngine/platform/tests/decode_png_file_test.cpp.")
        print("  Precisa MESMO carregar JPG/TGA num call site novo? Entao a "
              "excecao e uma DECISAO (do lider), com o custo de seguranca na "
              "mesa - nao um allowlist silencioso aqui.")
        return 1

    print(f"GATE(decode-image-file-zero): OK (zero chamada real de "
          f"glintfx::decode_image_file em {scanned} arquivos de producao; "
          "decode PNG so via decode_png_file).")
    return 0


if __name__ == "__main__":
    sys.exit(main())

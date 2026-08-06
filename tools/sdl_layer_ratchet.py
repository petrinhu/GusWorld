#!/usr/bin/env python3
"""sdl_layer_ratchet.py - GATE(sdl-ratchet) do tools/check.sh.

Conta quantos arquivos de PRODUCAO em GusEngine/app (src/ + include/gus/app/,
EXCLUINDO app/tests/ e app/tools/) tocam SDL3 (`#include <SDL3/...>` ou
qualquer token `SDL_*`) FORA de comentario/string, e reprova se o numero
SUBIU acima do teto abaixo.

Por que nao bash+grep: `grep -c 'SDL_'` conta ocorrencia dentro de comentario
como se fosse uso real - e essa mentira e exatamente o que fez o
GATE(arch) original nunca cobrir app/ (docs/tech/plano-camadas-sdl.md, secao 1).
Um parser de caractere (nao regex ingenua) e o preco de um gate honesto.

Escopo (por que so producao, ver docs/tech/plano-camadas-sdl.md secao 6):
  - app/tests/  fica de fora: e um binario ctest a parte, nao linka em
    gusworld_app; os testes de interacao usam SDL_Event de proposito, e essa
    exigencia esta amarrada a decisao pendente da secao 5 do plano, nao a
    higiene de producao.
  - app/tools/  fica de fora: `app/CMakeLists.txt` so faz
    `add_subdirectory(tests)` - tools/ tem CMakeLists.txt PROPRIO e
    standalone que importa os .a ja compilados (nao compila dentro de
    gusengine_app). E a mesma condicao estrutural que sustenta o
    GATE(tools-isolation) em check.sh: se um dia alguem linkar tools/ em
    gusengine_app, o arquivo deixa de estar coberto pela exececao e passa a
    contar contra este ratchet como qualquer arquivo novo de producao.

O teto so pode CAIR (cada fatia que reduz producao real atualiza o numero
aqui, no mesmo commit que faz a reducao) - nunca subir por excecao pontual.
"""

import os
import re
import sys

# Raiz do repo = pai de tools/ (resolve symlink do proprio script).
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(SCRIPT_DIR)
APP = os.path.join(ROOT, "GusEngine", "app")

PRODUCTION_DIRS = [
    os.path.join(APP, "src"),
    os.path.join(APP, "include", "gus", "app"),
]

# GATES-HARDEN (2026-08-06): os dois dirs acima nao alcancam `app/main.cpp` - o
# unico .cpp solto na RAIZ de app/ e a UNICA fonte do executavel
# (`app/CMakeLists.txt`: add_executable(gusworld_app main.cpp)). Ele era ponto
# cego de TODOS os gates de camada (deste ratchet, do log-clock-zero, do
# gl3-readbackbuffer-zero e do production_scope.py), e a auditoria mediu o custo:
# carregava um `#include <SDL3/SDL.h>` ORFAO que ninguem via. O include saiu no
# MESMO commit que este arquivo entrou aqui (a medicao deu ZERO token `SDL_*`
# fora de comentario nele), por isso o teto continua 24 e o gate nasce verde -
# nao foi teto subido por conveniencia.
#
# Quem consome este escopo NAO deve iterar PRODUCTION_DIRS na mao: use
# iter_production_files() abaixo, senao o ponto cego volta a existir num gate de
# cada vez (foi exatamente como ele sobreviveu ate hoje).
PRODUCTION_FILES = [
    os.path.join(APP, "main.cpp"),
]

# Mesma nota de production_scope.py: extensao e filtro barato, nao criterio de
# autoria. Medido em 2026-08-06: zero arquivo com estas extensoes dentro do
# escopo de app/ hoje, entao acrescenta-las nao move nenhuma contagem - so
# fecha a porta pra um `.h`/`.inl` futuro entrar invisivel.
CODE_EXTENSIONS = (".cpp", ".hpp", ".h", ".c", ".cc", ".cxx", ".inl", ".hxx", ".ipp")

# TETO DE NAO-REGRESSAO. Medido em 2026-07-29 (docs/tech/plano-camadas-sdl.md
# secao 2): 32 arquivos de producao tocavam SDL3 fora de comentario. Cada
# fatia que reduz esse numero de verdade ATUALIZA o valor abaixo no mesmo
# commit - senao o gate para de proteger o terreno reconquistado.
#
# 32 -> 24 (M9-CAMADAS-SDL Fatia 0 + Fatia 2, mesmo dia): Fatia 0 apagou
# run_title/system_menu_loop_owning_gl (codigo morto, nao mudou a contagem de
# ARQUIVO - os 2 arquivos ainda tocam SDL via gl_current). Fatia 2 tirou
# SDL_Keycode do TIPO das 4 telas de menu puro (title/difficulty/system/
# save_load_menu.hpp+.cpp, 8 arquivos) trocando por glintfx::Key - a traducao
# ficou 100% nos 4 *_loop.cpp (que continuam contando, corretamente).
#
# 24 -> 24, SEM MUDANCA (Fatia 1, log-e-relogio, 2026-07-30): SDL_Log
# (~30 call sites) e SDL_GetTicksNS (~17 call sites) saíram por completo de
# producao (glintfx::log*/FW-LOG, glintfx::monotonic_now_ns/FW-CLOCK) - medido
# ANTES e DEPOIS desta fatia, os mesmos 24 arquivos continuam na lista porque
# TODOS OS 24, sem excecao, ja tinham OUTRO uso real de SDL3 (SDL_Window*,
# SDL_GLContext, SDL_Event, SDL_GL_*, etc.) alem de log/relogio - nenhum
# arquivo tinha log/relogio como seu UNICO motivo de aparecer aqui. Este
# ratchet e por-ARQUIVO (nao por-token), entao uma categoria inteira pode
# fechar sem mover este numero - a prova real da fatia esta no
# GATE(log-clock-zero) novo (tools/sdl_log_clock_zero.py), zero-tolerancia
# fechada pras duas categorias, chamado por tools/check.sh logo apos este
# gate. NAO baixar CEILING sem medir de novo - um teto abaixo de 24 aqui
# reprovaria hoje por engano (os 24 arquivos continuam legitimamente na
# lista).
CEILING = 24

SDL_TOKEN_RE = re.compile(r"\bSDL_[A-Za-z0-9_]*\b")
SDL_INCLUDE_RE = re.compile(r'#\s*include\s*[<"]SDL3?/')


def strip_comments(text: str) -> str:
    """Remove comentarios // e /* */, preservando o conteudo de strings e
    chars (parser de caractere - a mesma tecnica do measure_sdl.py que
    fundamentou a medicao do plano, nao regex ingenua que confunde `//`
    dentro de string com comentario)."""
    out = []
    i, n = 0, len(text)
    in_line_comment = False
    in_block_comment = False
    in_string = False
    in_char = False
    while i < n:
        c = text[i]
        c2 = text[i + 1] if i + 1 < n else ""
        if in_line_comment:
            if c == "\n":
                in_line_comment = False
                out.append(c)
            i += 1
            continue
        if in_block_comment:
            if c == "*" and c2 == "/":
                in_block_comment = False
                i += 2
                continue
            if c == "\n":
                out.append("\n")
            i += 1
            continue
        if in_string:
            out.append(c)
            if c == "\\" and i + 1 < n:
                out.append(text[i + 1])
                i += 2
                continue
            if c == '"':
                in_string = False
            i += 1
            continue
        if in_char:
            out.append(c)
            if c == "\\" and i + 1 < n:
                out.append(text[i + 1])
                i += 2
                continue
            if c == "'":
                in_char = False
            i += 1
            continue
        if c == "/" and c2 == "/":
            in_line_comment = True
            i += 2
            continue
        if c == "/" and c2 == "*":
            in_block_comment = True
            i += 2
            continue
        if c == '"':
            in_string = True
            out.append(c)
            i += 1
            continue
        if c == "'":
            in_char = True
            out.append(c)
            i += 1
            continue
        out.append(c)
        i += 1
    return "".join(out)


def iter_production_files():
    """Gera caminhos absolutos de TODO arquivo de producao de app/ - os dirs de
    PRODUCTION_DIRS mais os arquivos soltos de PRODUCTION_FILES.

    Ponto unico de verdade do escopo de app/: este ratchet, o
    GATE(log-clock-zero) e o GATE(gl3-readbackbuffer-zero) chamam esta funcao em
    vez de repetir o os.walk - se um dia entrar outro arquivo solto, os tres
    passam a ve-lo no mesmo commit."""
    for base in PRODUCTION_DIRS:
        if not os.path.isdir(base):
            continue
        for dirpath, _dirnames, filenames in os.walk(base):
            for fn in sorted(filenames):
                if fn.endswith(CODE_EXTENSIONS):
                    yield os.path.join(dirpath, fn)
    for fp in PRODUCTION_FILES:
        if os.path.isfile(fp):
            yield fp


def file_touches_sdl(path: str) -> bool:
    with open(path, encoding="utf-8", errors="replace") as f:
        text = f.read()
    stripped = strip_comments(text)
    if SDL_INCLUDE_RE.search(stripped):
        return True
    return SDL_TOKEN_RE.search(stripped) is not None


def find_offenders():
    offenders = []
    for fp in iter_production_files():
        if file_touches_sdl(fp):
            offenders.append(os.path.relpath(fp, ROOT))
    offenders.sort()
    return offenders


def main() -> int:
    offenders = find_offenders()
    n = len(offenders)
    # DECLARA O QUE VARRE (mesma regra do GATE(spdx-required)): "OK" so pode
    # significar "varri estes N", nunca "nao achei problema onde procurei". Se o
    # escopo colapsar (dir renomeado, arvore mexida), o numero cai a vista em vez
    # de o gate imprimir um OK vazio.
    n_scanned = sum(1 for _ in iter_production_files())
    if n_scanned == 0:
        print("GATE(sdl-ratchet): FALHA - escopo colapsou pra ZERO arquivo "
              "(app/src + app/include/gus/app + app/main.cpp). O gate nao pode "
              "dizer OK sem ter olhado pra nada.")
        return 1
    print(f"GATE(sdl-ratchet): {n} arquivo(s) de producao em app/ tocam SDL3 "
          f"(teto={CEILING}, escopo: app/src + app/include/gus/app + app/main.cpp, "
          f"excluindo app/tests/ e app/tools/; {n_scanned} arquivo(s) varrido(s)).")
    if n > CEILING:
        print(f"GATE(sdl-ratchet): FALHA - {n} > teto {CEILING}.")
        print("  Isso quebra o contrato de 4 camadas: platform/ e a UNICA")
        print("  fronteira que deveria tocar bibliotecas externas (CLAUDE.md).")
        print(f"  Arquivos infratores ({n}):")
        for off in offenders:
            print(f"    - {off}")
        print("  Caminho certo: mover a logica para platform/, ou, se faltar")
        print("  algo generico, PEDIR ao glintfx (docs/tech/glintfx-boundary.md)")
        print("  - nao reimplementar SDL em app/ nem contornar por dentro.")
        print("  Se voce baixou esse numero de proposito (fatia concluida),")
        print(f"  atualize CEILING neste script (agora {CEILING}) no MESMO commit.")
        return 1
    if n < CEILING:
        print(f"GATE(sdl-ratchet): OK, e {CEILING - n} abaixo do teto -- "
              f"considere baixar CEILING para {n} no proximo commit que tocar isto.")
    else:
        print("GATE(sdl-ratchet): OK (no teto).")
    return 0


if __name__ == "__main__":
    sys.exit(main())

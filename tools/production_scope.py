#!/usr/bin/env python3
"""production_scope.py - lista canonica de arquivos de PRODUCAO nas 4
camadas (core/domain/platform/app), para os gates CROSS-CAMADA de
zero-tolerancia de tools/check.sh (stb_image_zero.py, audio_ma_zero.py,
decode_image_file_zero.py) - e GATE(production-scope) por si so, que audita
que essa lista nao ficou para tras do disco.

NAO e um parser: o parser de comentario honesto continua sendo
strip_comments() de tools/sdl_layer_ratchet.py - importado por quem
precisar, nunca reescrito (a licao registrada no docstring daquele
arquivo: bash+grep cru conta ocorrencia em comentario como uso real).

Escopo EXCLUI, por construcao (a lista aponta direto pros dirs de codigo,
nao pro dir do layer inteiro), sem precisar de exclusao por nome:
  - domain/tests/, platform/tests/, app/tests/ - binarios ctest a parte,
    nao linkam em gusengine_app.
  - app/tools/ - CMakeLists.txt PROPRIO e standalone (mesma condicao
    estrutural documentada em sdl_layer_ratchet.py); se um dia passar a
    linkar em gusengine_app, precisa entrar aqui.
  - platform/rmlui/ ENTRA (e producao real, linkada no binario - e o
    vendor do glad/RmlUi_Include_GL3.h), mesmo nao sendo alvo de
    stb_image/miniaudio; incluir custa nada e evita um "esqueceram de
    olhar ali" no futuro.

core/ nao tem subdir de testes proprio (testado indiretamente via
domain/platform/app); nada a excluir alem do que ja fica de fora por a
lista so apontar pra src/ e include/.

ARQUIVO SOLTO NA RAIZ DA CAMADA (GATES-HARDEN, 2026-08-06): a lista acima
so aponta pra DIRETORIOS, e por isso deixava de fora `GusEngine/app/main.cpp`
- o unico .cpp solto na raiz de uma camada, e a UNICA fonte do executavel
(`app/CMakeLists.txt`, add_executable(gusworld_app main.cpp)). Ele nao estava
em NENHUM gate de camada: nem aqui, nem no ratchet de SDL. A auditoria mediu
o custo real disso: o arquivo carregava um `#include <SDL3/SDL.h>` orfao (zero
token SDL_* fora de comentario) que nenhum gate via. O include saiu no mesmo
commit e o arquivo entrou em ALL_PRODUCTION_FILES abaixo.

AUTO-AUDITORIA (GATES-HARDEN, requisito 4 da fatia): ALL_PRODUCTION_DIRS e
lista ESCRITA A MAO - uma 5a camada nasceria fora dela e os 3 gates
zero-tolerancia diriam "OK" sem nunca ter olhado pra ela ("um verificador com
ponto cego confirma o que se espera dele"). audit_scope() abaixo ENUMERA o
disco (`GusEngine/*/src`, `GusEngine/*/include`, e os arquivos de codigo
soltos na raiz de cada camada) e REPROVA se algo estiver fora da lista. E o
mesmo desenho do GATE(spdx-required), que enumera por `git ls-files` em vez
de procurar onde ja se sabe.
"""

from __future__ import annotations

import glob
import os
import sys

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(SCRIPT_DIR)
ENGINE = os.path.join(ROOT, "GusEngine")

ALL_PRODUCTION_DIRS = [
    os.path.join(ENGINE, "core", "src"),
    os.path.join(ENGINE, "core", "include"),
    os.path.join(ENGINE, "domain", "src"),
    os.path.join(ENGINE, "domain", "include"),
    os.path.join(ENGINE, "platform", "src"),
    os.path.join(ENGINE, "platform", "include"),
    os.path.join(ENGINE, "platform", "rmlui"),
    os.path.join(ENGINE, "app", "src"),
    os.path.join(ENGINE, "app", "include", "gus", "app"),
]

# Arquivos de producao que NAO moram em nenhum dos dirs acima (ver o bloco
# "ARQUIVO SOLTO NA RAIZ DA CAMADA" no docstring). Hoje: exatamente 1.
ALL_PRODUCTION_FILES = [
    os.path.join(ENGINE, "app", "main.cpp"),
]

# GATES-HARDEN: `.c` e as variantes `.cc/.cxx/.inl/.hxx/.ipp` entraram aqui
# porque o custo e uma linha e a medicao do dia (2026-08-06) deu ZERO arquivo
# novo dentro do escopo - o gate nasce verde. Antes, um `glad.c` largado em
# platform/rmlui/ ou um `.inl` com stbi_load seriam invisiveis pros 3 gates
# zero-tolerancia. Extensao de arquivo NAO e o criterio de "e nosso codigo?";
# e so o filtro barato pra nao abrir .png/.md.
CODE_EXTENSIONS = (".cpp", ".hpp", ".h", ".c", ".cc", ".cxx", ".inl", ".hxx", ".ipp")


def iter_production_files():
    """Gera caminhos absolutos de todo arquivo de codigo dentro do escopo."""
    for base in ALL_PRODUCTION_DIRS:
        if not os.path.isdir(base):
            continue
        for dirpath, _dirnames, filenames in os.walk(base):
            for fn in filenames:
                if fn.endswith(CODE_EXTENSIONS):
                    yield os.path.join(dirpath, fn)
    for fp in ALL_PRODUCTION_FILES:
        if os.path.isfile(fp):
            yield fp


def _covered_by_dir(path: str) -> bool:
    """True se `path` esta DENTRO de (ou E) algum dir de ALL_PRODUCTION_DIRS.

    `app/include` conta como coberto por `app/include/gus/app`: o que importa
    e existir varredura DENTRO dele, nao a raiz inteira estar na lista (o
    resto de app/include/ e header publico de outras camadas ja varrido pelo
    dir da camada dona)."""
    norm = os.path.normpath(path)
    for base in ALL_PRODUCTION_DIRS:
        b = os.path.normpath(base)
        if norm == b or b.startswith(norm + os.sep) or norm.startswith(b + os.sep):
            return True
    return False


def audit_scope() -> list[str]:
    """Enumera o disco e devolve os problemas de COBERTURA (lista vazia = ok).

    Nao mede conteudo de arquivo nenhum - so responde "a lista escrita a mao
    ainda descreve o que existe?". Ver o bloco AUTO-AUDITORIA no docstring."""
    problems: list[str] = []

    if not os.path.isdir(ENGINE):
        return [f"GusEngine/ nao existe em {ENGINE} - escopo colapsado, "
                "nao posso auditar nada."]

    # (1) toda camada com src/ no disco tem de estar coberta.
    layer_srcs = sorted(glob.glob(os.path.join(ENGINE, "*", "src")))
    if not layer_srcs:
        problems.append(
            "nenhum GusEngine/*/src encontrado no disco - ou a arvore mudou de "
            "forma, ou a auditoria esta olhando o lugar errado. Nos dois casos "
            "este gate NAO pode dizer OK."
        )
    for src in layer_srcs:
        if not os.path.isdir(src):
            continue
        if not _covered_by_dir(src):
            problems.append(
                f"camada NAO coberta: {os.path.relpath(src, ROOT)} existe no disco "
                "e nao esta em ALL_PRODUCTION_DIRS"
            )

    # (2) o include/ de cada camada com src/ tambem.
    for src in layer_srcs:
        inc = os.path.join(os.path.dirname(src), "include")
        if os.path.isdir(inc) and not _covered_by_dir(inc):
            problems.append(
                f"include de camada NAO coberto: {os.path.relpath(inc, ROOT)} existe "
                "no disco e nao esta em ALL_PRODUCTION_DIRS"
            )

    # (3) arquivo de codigo SOLTO na raiz de uma camada (o caso do main.cpp).
    known = {os.path.normpath(p) for p in ALL_PRODUCTION_FILES}
    for src in layer_srcs:
        layer = os.path.dirname(src)
        for fn in sorted(os.listdir(layer)):
            fp = os.path.join(layer, fn)
            if not os.path.isfile(fp) or not fn.endswith(CODE_EXTENSIONS):
                continue
            if os.path.normpath(fp) not in known:
                problems.append(
                    f"arquivo solto NAO coberto: {os.path.relpath(fp, ROOT)} esta na "
                    "raiz da camada (fora de src/ e include/) e nao esta em "
                    "ALL_PRODUCTION_FILES"
                )

    # (4) entrada morta em ALL_PRODUCTION_FILES (arraste de excecao).
    for fp in ALL_PRODUCTION_FILES:
        if not os.path.isfile(fp):
            problems.append(
                f"entrada desatualizada: {os.path.relpath(fp, ROOT)} esta em "
                "ALL_PRODUCTION_FILES e nao existe mais no disco"
            )

    return problems


def main() -> int:
    problems = audit_scope()
    n_dirs = len(ALL_PRODUCTION_DIRS)
    n_files = len(ALL_PRODUCTION_FILES)
    n_scanned = sum(1 for _ in iter_production_files())

    if problems:
        print(
            f"GATE(production-scope): FALHA - {len(problems)} lacuna(s) de cobertura "
            "entre o disco e a lista de tools/production_scope.py:"
        )
        for p in problems:
            print(f"    - {p}")
        print(
            "  Uma camada/arquivo fora da lista e PONTO CEGO dos gates "
            "zero-tolerancia (stb_image_zero, audio_ma_zero, decode_image_file_zero): "
            "eles diriam OK sem nunca ter olhado pra ela. Acrescente o caminho a "
            "ALL_PRODUCTION_DIRS/ALL_PRODUCTION_FILES no MESMO commit que criou a "
            "camada/arquivo."
        )
        return 1

    print(
        f"GATE(production-scope): OK ({n_dirs} dir(s) + {n_files} arquivo(s) solto(s) "
        f"na lista, {n_scanned} arquivo(s) de producao no escopo; disco enumerado "
        "(GusEngine/*/src, */include e raiz de cada camada) sem lacuna nem entrada "
        "desatualizada)."
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())

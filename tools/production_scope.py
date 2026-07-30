#!/usr/bin/env python3
"""production_scope.py - lista canonica de diretorios de PRODUCAO nas 4
camadas (core/domain/platform/app), para os gates CROSS-CAMADA de
zero-tolerancia de tools/check.sh (stbi_image_zero.py, audio_ma_zero.py).

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
"""

import os

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

CODE_EXTENSIONS = (".cpp", ".hpp", ".h")


def iter_production_files():
    """Gera caminhos absolutos de todo .cpp/.hpp/.h dentro do escopo acima."""
    for base in ALL_PRODUCTION_DIRS:
        if not os.path.isdir(base):
            continue
        for dirpath, _dirnames, filenames in os.walk(base):
            for fn in filenames:
                if fn.endswith(CODE_EXTENSIONS):
                    yield os.path.join(dirpath, fn)
